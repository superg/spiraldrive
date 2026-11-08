use anyhow::{Context, Result, ensure};
use serde::{
    Deserialize, Deserializer,
    de::{MapAccess, Visitor, value::MapAccessDeserializer},
};
use sha1::{Digest, Sha1};
use std::{
    fmt,
    marker::PhantomData,
    ops::Range,
    path::{Path, PathBuf},
};

use crate::payload::Payload;

const IMAGE_SIZE: usize = 0x80000;

struct ImageLayout {
    payload_base: Option<u32>,
    vendor_specific: Option<usize>,
    revisions: &'static [usize],
    checksum: usize,
    checksum_range: Range<usize>,
    checksum_bias: u32,
    erased: &'static [Range<usize>],
}

// Derived struct deserializers also accept sequences. Require JSON objects,
// then let Serde reject duplicate, missing, and unknown fields.
struct Object<T>(T);

impl<'de, T: Deserialize<'de>> Deserialize<'de> for Object<T> {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> std::result::Result<Self, D::Error> {
        struct ObjectVisitor<T>(PhantomData<T>);
        impl<'de, T: Deserialize<'de>> Visitor<'de> for ObjectVisitor<T> {
            type Value = Object<T>;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("an object")
            }

            fn visit_map<M: MapAccess<'de>>(
                self,
                map: M,
            ) -> std::result::Result<Self::Value, M::Error> {
                T::deserialize(MapAccessDeserializer::new(map)).map(Object)
            }
        }
        deserializer.deserialize_map(ObjectVisitor(PhantomData))
    }
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct DescriptionFields {
    sha1: String,
    revision: String,
    patches: Vec<Object<PatchFields>>,
    payload: Option<Object<PayloadFields>>,
    vendor_specific: Option<String>,
    // Default permits omission, while String rejects explicit null/comments
    // of other types. Renaming keeps this a recognized, duplicate-checked key.
    #[serde(default, rename = "comment")]
    _comment: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct PatchFields {
    offset: String,
    value: Option<String>,
    symbol: Option<String>,
    size: Option<usize>,
    #[serde(default, rename = "comment")]
    _comment: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct PayloadFields {
    hex: PathBuf,
    map: PathBuf,
}

enum PatchValue {
    Bytes(Vec<u8>),
    Symbol { name: String, size: usize },
}

struct Patch {
    offset: usize,
    value: PatchValue,
}

pub struct Description {
    sha1: String,
    revision: String,
    patches: Vec<Patch>,
    payload: Option<PayloadFields>,
    vendor_specific: Option<String>,
}

impl Description {
    pub fn revision(&self) -> &str {
        &self.revision
    }

    pub fn patch_count(&self) -> usize {
        self.patches.len()
    }

    pub fn payload_paths(&self, description_path: &Path) -> Option<[PathBuf; 2]> {
        let base = description_path.parent().unwrap_or(Path::new("."));
        self.payload
            .as_ref()
            .map(|files| [base.join(&files.hex), base.join(&files.map)])
    }
}

pub struct PatchedImage {
    pub bytes: Vec<u8>,
    pub checksum: u16,
}

fn parse_offset(text: &str) -> Result<usize> {
    let digits = text
        .strip_prefix("0x")
        .or_else(|| text.strip_prefix("0X"))
        .context("offset must be a hexadecimal string starting with 0x")?;
    ensure!(
        !digits.is_empty() && digits.bytes().all(|b| b.is_ascii_hexdigit()),
        "invalid hexadecimal offset"
    );
    usize::from_str_radix(digits, 16).context("offset is too large")
}

fn parse_bytes(text: &str) -> Result<Vec<u8>> {
    let mut bytes = Vec::new();
    for token in text.split_ascii_whitespace() {
        ensure!(
            token.len() == 2 && token.bytes().all(|b| b.is_ascii_hexdigit()),
            "expected whitespace-separated hexadecimal byte pairs"
        );
        bytes.push(u8::from_str_radix(token, 16)?);
    }
    ensure!(!bytes.is_empty(), "byte sequence must not be empty");
    Ok(bytes)
}

pub fn parse_description(bytes: &[u8]) -> Result<Description> {
    // Match the former parser's support for a UTF-8 BOM.
    let bytes = bytes.strip_prefix(b"\xef\xbb\xbf").unwrap_or(bytes);
    let Object(raw): Object<DescriptionFields> = serde_json::from_slice(bytes)?;
    ensure!(
        raw.sha1.len() == 40 && raw.sha1.bytes().all(|b| b.is_ascii_hexdigit()),
        "sha1 must contain exactly 40 hexadecimal characters"
    );
    ensure!(
        raw.revision.len() == 4 && raw.revision.bytes().all(|b| (0x20..=0x7e).contains(&b)),
        "revision must contain exactly four printable ASCII bytes"
    );
    let mut patches = Vec::with_capacity(raw.patches.len());
    for (index, Object(entry)) in raw.patches.into_iter().enumerate() {
        let number = index + 1;
        let offset =
            parse_offset(&entry.offset).with_context(|| format!("patch {number} offset"))?;
        let value = match (entry.value, entry.symbol, entry.size) {
            (Some(value), None, None) => PatchValue::Bytes(
                parse_bytes(&value).with_context(|| format!("patch {number} value"))?,
            ),
            (None, Some(name), Some(size @ (3 | 4))) => {
                ensure!(
                    !name.is_empty() && !name.bytes().any(|b| b.is_ascii_whitespace()),
                    "patch {number}: invalid symbol name"
                );
                ensure!(
                    raw.payload.is_some(),
                    "patch {number}: symbol requires a payload map"
                );
                PatchValue::Symbol { name, size }
            }
            _ => anyhow::bail!("patch {number}: provide value, or symbol with size 3 or 4"),
        };
        patches.push(Patch { offset, value });
    }
    if let Some(Object(files)) = &raw.payload {
        ensure!(
            !files.hex.as_os_str().is_empty() && !files.map.as_os_str().is_empty(),
            "payload HEX and map paths must not be empty"
        );
    }
    if let Some(text) = &raw.vendor_specific {
        ensure!(
            text.len() <= 20 && text.bytes().all(|b| (0x20..=0x7e).contains(&b)),
            "vendor_specific must contain at most 20 printable ASCII bytes"
        );
    }
    Ok(Description {
        sha1: raw.sha1.to_ascii_lowercase(),
        revision: raw.revision,
        patches,
        payload: raw.payload.map(|Object(files)| files),
        vendor_specific: raw.vendor_specific,
    })
}

fn checksum(image: &[u8], layout: &ImageLayout) -> u16 {
    let sum = image[layout.checksum_range.clone()]
        .iter()
        .fold(layout.checksum_bias, |sum, &byte| sum + u32::from(byte));
    (sum & 0xffff) as u16
}

fn validate_image(image: &[u8]) -> Result<ImageLayout> {
    ensure!(
        image.len() == IMAGE_SIZE,
        "firmware must be exactly 0x80000 bytes (512 KiB)"
    );
    let york = image.starts_with(b"PLEXTOR CD-R   PX-W4824A")
        && image[0x30..].starts_with(b"PLEXTOR YORK");
    let mont = image.starts_with(b"PLEXTOR CD-R   PX-W5224A")
        && image[0x30..].starts_with(b"PLEXTOR MONT");
    let winc = image[0x10000..].starts_with(b"PLEXTOR CD-R   PX-W2410A")
        && image[0x10030..].starts_with(b"PLEXTOR WINCHES");
    let seat = image[0x10000..].starts_with(b"PLEXTOR CD-R   PX-W1610A")
        && image[0x10030..].starts_with(b"PLEXTOR SEATTLE");
    let otta = image[0x10000..].starts_with(b"PLEXTOR CD-R   PX-S88T")
        && image[0x10030..].starts_with(b"PLEXTOR OTTAWA");
    let munc = image[0x5bd20..].starts_with(b"PLEXTOR \0CD-R   PX-W8432T\0")
        && image[0x5ff1c..].starts_with(b"PLEXTOR MUNCHEN");
    let layout = if york || mont {
        ImageLayout {
            payload_base: Some(0xf80000),
            vendor_specific: Some(0x1c),
            revisions: &[0x18],
            checksum: 0x79ffe,
            checksum_range: 0..0x79ffe,
            checksum_bias: 0x8000,
            erased: &[0x70000..0x78000, 0x7a000..IMAGE_SIZE],
        }
    } else if winc || seat || otta {
        ImageLayout {
            payload_base: None,
            vendor_specific: Some(0x1001c),
            revisions: &[0x10018],
            checksum: 0x77ffe,
            checksum_range: 0..0x77ffe,
            checksum_bias: 0,
            erased: &[0..0x10000, 0x78000..IMAGE_SIZE],
        }
    } else if munc {
        ImageLayout {
            payload_base: None,
            vendor_specific: None,
            revisions: &[0x5b99a, 0x5bd3a],
            checksum: 0x5fffe,
            checksum_range: 0x10000..0x5fffe,
            checksum_bias: 0,
            erased: &[0..0x10000, 0x60000..IMAGE_SIZE],
        }
    } else {
        anyhow::bail!(
            "unsupported firmware identification: expected PX-W4824A/YORK, PX-W5224A/MONT, PX-W2410A/WINCHES, PX-W1610A/SEATTLE, PX-W8432T/MUNCHEN, or PX-S88T/OTTAWA"
        );
    };
    for range in layout.erased {
        ensure!(
            image[range.clone()].iter().all(|&b| b == 0xff),
            "firmware erased region {:#x}..{:#x} is not all ff",
            range.start,
            range.end - 1
        );
    }
    let stored = u16::from_be_bytes([image[layout.checksum], image[layout.checksum + 1]]);
    let calculated = checksum(image, &layout);
    ensure!(
        stored == calculated,
        "firmware checksum mismatch: stored {stored:#06x}, calculated {calculated:#06x}"
    );
    Ok(layout)
}

pub fn patch_image(
    original: &[u8],
    description: &Description,
    payload: Option<&Payload>,
) -> Result<PatchedImage> {
    let actual_sha1: String = Sha1::digest(original)
        .iter()
        .map(|byte| format!("{byte:02x}"))
        .collect();
    ensure!(
        actual_sha1 == description.sha1,
        "firmware SHA-1 mismatch: expected {}, got {actual_sha1}",
        description.sha1
    );
    let layout = validate_image(original)?;
    ensure!(
        description.payload.is_some() == payload.is_some(),
        "payload inputs do not match the description"
    );
    let mut reserved_ranges = layout.erased.to_vec();
    reserved_ranges.push(0..layout.checksum_range.start);
    reserved_ranges.push(layout.checksum..layout.checksum + 2);
    reserved_ranges.push(layout.checksum_range.end..IMAGE_SIZE);
    reserved_ranges.extend(layout.revisions.iter().map(|&offset| offset..offset + 4));

    let mut writes: Vec<(usize, Vec<u8>)> = Vec::new();
    if let Some(payload) = payload {
        let base = layout
            .payload_base
            .context("payload address mapping is not defined for this firmware layout")?;
        for (&address, &value) in &payload.bytes {
            let offset = address
                .checked_sub(base)
                .context("payload address precedes the firmware image")?
                as usize;
            ensure!(
                offset < original.len(),
                "payload address {address:#x} is outside the firmware"
            );
            ensure!(
                original[offset] == 0xff,
                "payload destination {offset:#x} is not erased ff"
            );
            match writes.last_mut() {
                Some((start, bytes)) if *start + bytes.len() == offset => bytes.push(value),
                _ => writes.push((offset, vec![value])),
            }
        }
    }
    for (index, patch) in description.patches.iter().enumerate() {
        let number = index + 1;
        let value = match &patch.value {
            PatchValue::Bytes(bytes) => bytes.clone(),
            PatchValue::Symbol { name, size } => {
                let payload = payload.context("symbol patch requires a payload map")?;
                let &address = payload
                    .symbols
                    .get(name)
                    .with_context(|| format!("patch {number}: missing map symbol {name}"))?;
                ensure!(
                    *size == 4 || address <= 0xffffff,
                    "patch {number}: symbol address does not fit in {size} bytes"
                );
                ensure!(
                    payload.bytes.contains_key(&address),
                    "patch {number}: symbol {name} is outside the loaded payload"
                );
                address.to_be_bytes()[4 - size..].to_vec()
            }
        };
        writes.push((patch.offset, value));
    }
    if let Some(text) = &description.vendor_specific {
        let offset = layout
            .vendor_specific
            .context("vendor-specific inquiry field is not defined for this firmware layout")?;
        let mut value = vec![b' '; 20];
        value[..text.len()].copy_from_slice(text.as_bytes());
        writes.push((offset, value));
    }
    let mut ranges = Vec::with_capacity(writes.len());
    for (start, value) in &writes {
        let start = *start;
        ensure!(
            start < original.len() && value.len() <= original.len() - start,
            "write at {start:#x}: range is outside the firmware"
        );
        let end = start + value.len();
        for reserved in &reserved_ranges {
            ensure!(
                !(start < reserved.end && end > reserved.start),
                "write at {start:#x}: overlaps an automatic field or protected firmware region"
            );
        }
        ranges.push(start..end);
    }
    ranges.sort_unstable_by_key(|range| range.start);
    for pair in ranges.windows(2) {
        ensure!(
            pair[1].start >= pair[0].end,
            "patches overlap at {:#x}",
            pair[1].start
        );
    }

    let mut bytes = original.to_vec();
    for (offset, value) in &writes {
        bytes[*offset..offset + value.len()].copy_from_slice(value);
    }
    for &offset in layout.revisions {
        bytes[offset..offset + 4].copy_from_slice(description.revision.as_bytes());
    }
    let sum = checksum(&bytes, &layout);
    bytes[layout.checksum..layout.checksum + 2].copy_from_slice(&sum.to_be_bytes());
    validate_image(&bytes)?;
    Ok(PatchedImage {
        bytes,
        checksum: sum,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::BTreeMap;

    fn mont_image() -> Vec<u8> {
        let mut bytes = vec![0xff; IMAGE_SIZE];
        let identification = b"PLEXTOR CD-R   PX-W5224A";
        bytes[..identification.len()].copy_from_slice(identification);
        bytes[0x30..0x3c].copy_from_slice(b"PLEXTOR MONT");
        let sum = bytes[..0x79ffe]
            .iter()
            .fold(0x8000u32, |sum, b| sum + u32::from(*b)) as u16;
        bytes[0x79ffe..0x7a000].copy_from_slice(&sum.to_be_bytes());
        bytes
    }

    fn document(original: &[u8], patches: serde_json::Value) -> serde_json::Value {
        serde_json::json!({
            "sha1": Sha1::digest(original).iter().map(|b| format!("{b:02x}")).collect::<String>(),
            "revision": "G104",
            "payload": {"hex": "payload/c4.hex", "map": "payload/c4.map"},
            "patches": patches,
        })
    }

    fn description(document: &serde_json::Value) -> Result<Description> {
        parse_description(&serde_json::to_vec(document)?)
    }

    #[test]
    fn accepts_value_or_symbol_and_rejects_obsolete_or_ambiguous_fields() {
        let original = mont_image();
        for invalid in [
            serde_json::json!({"offset": "0x4207"}),
            serde_json::json!({"offset": "0x4207", "value": ""}),
            serde_json::json!({"offset": "0x4207", "value": "f"}),
            serde_json::json!({"offset": "0x4207", "value": 1}),
            serde_json::json!({"offset": "0x4207", "old": "ff", "new": "42"}),
            serde_json::json!({"offset": "0x4207", "old": "ff", "value": "42"}),
            serde_json::json!({"offset": "0x4207", "new": "42"}),
            serde_json::json!({"offset": "0x4207", "value": "42", "symbol": "handler", "size": 4}),
            serde_json::json!({"offset": "0x4207", "value": "42", "size": 4}),
            serde_json::json!({"offset": "0x4207", "symbol": "handler"}),
            serde_json::json!({"offset": "0x4207", "symbol": "handler", "size": 2}),
            serde_json::json!({"offset": "0x4207", "symbol": "", "size": 4}),
        ] {
            assert!(
                description(&document(&original, serde_json::json!([invalid]))).is_err(),
                "accepted {invalid}"
            );
        }
        let valid = document(
            &original,
            serde_json::json!([
                {"offset": "0x4207", "symbol": "handler", "size": 4},
                {"offset": "0x43bb", "value": "01"}
            ]),
        );
        let parsed = description(&valid).unwrap();
        assert_eq!(parsed.patch_count(), 2);
        assert_eq!(
            parsed
                .payload_paths(Path::new("firmware/mont.json"))
                .unwrap(),
            [
                PathBuf::from("firmware/payload/c4.hex"),
                PathBuf::from("firmware/payload/c4.map")
            ]
        );
        let mut without_map = valid.clone();
        without_map.as_object_mut().unwrap().remove("payload");
        assert!(description(&without_map).is_err());
        assert!(parse_description(b"[]").is_err());
    }

    #[test]
    fn applies_sparse_payload_symbols_and_padded_inquiry_then_recalculates_checksum() {
        let original = mont_image();
        let mut doc = document(
            &original,
            serde_json::json!([
                {"offset": "0x4207", "symbol": "handler", "size": 4},
                {"offset": "0x511d0", "symbol": "event", "size": 3}
            ]),
        );
        doc["vendor_specific"] = serde_json::json!("SpiralDrive v1.0.0");
        let payload = Payload {
            bytes: BTreeMap::from([(0xfdeae0, 0x42), (0xfdeae1, 0x43), (0xfef194, 0x44)]),
            symbols: BTreeMap::from([("handler".into(), 0xfdeae0), ("event".into(), 0xfef194)]),
        };
        let result = patch_image(&original, &description(&doc).unwrap(), Some(&payload)).unwrap();
        assert_eq!(&result.bytes[0x4207..0x420b], &[0, 0xfd, 0xea, 0xe0]);
        assert_eq!(&result.bytes[0x511d0..0x511d3], &[0xfe, 0xf1, 0x94]);
        assert_eq!(result.bytes[0x511cf], original[0x511cf]);
        assert_eq!(&result.bytes[0x5eae0..0x5eae2], &[0x42, 0x43]);
        assert_eq!(result.bytes[0x6f194], 0x44);
        assert_eq!(&result.bytes[0x60000..0x6f194], &original[0x60000..0x6f194]);
        assert_eq!(&result.bytes[0x18..0x1c], b"G104");
        assert_eq!(&result.bytes[0x1c..0x30], b"SpiralDrive v1.0.0  ");
        validate_image(&result.bytes).unwrap();
    }

    #[test]
    fn rejects_invalid_payload_placement_symbols_and_inquiry() {
        let original = mont_image();
        let doc = document(
            &original,
            serde_json::json!([
                {"offset": "0x4207", "symbol": "handler", "size": 3}
            ]),
        );
        let parsed = description(&doc).unwrap();
        let mut payload = Payload {
            bytes: BTreeMap::from([(0xfdeae0, 0x42)]),
            symbols: BTreeMap::from([("handler".into(), 0xfdeae0)]),
        };
        assert!(patch_image(&original, &parsed, None).is_err());
        for target in [0xf80000, 0xf7ffff, 0x1000000, 0xff0000, 0xff9ffe] {
            payload.bytes = BTreeMap::from([(target, 0x42)]);
            payload.symbols.insert("handler".into(), target);
            assert!(patch_image(&original, &parsed, Some(&payload)).is_err());
        }
        payload.bytes = BTreeMap::from([(0xfdeae0, 0x42)]);
        for address in [0xfdeae1, 0x1000000] {
            payload.symbols.insert("handler".into(), address);
            assert!(patch_image(&original, &parsed, Some(&payload)).is_err());
        }
        payload.symbols.clear();
        assert!(patch_image(&original, &parsed, Some(&payload)).is_err());
        payload.symbols.insert("handler".into(), 0xfdeae0);
        for patches in [
            serde_json::json!([{"offset": "0x5eae0", "value": "00"}]),
            serde_json::json!([{"offset": "0x50", "value": "00 01"}, {"offset": "0x51", "value": "02"}]),
            serde_json::json!([{"offset": "0x7ffff", "value": "00 01"}]),
            serde_json::json!([{"offset": format!("0x{:x}", usize::MAX), "value": "00 01"}]),
        ] {
            let parsed = description(&document(&original, patches)).unwrap();
            assert!(patch_image(&original, &parsed, Some(&payload)).is_err());
        }
        for text in ["x".repeat(21), "\n".into(), "é".into()] {
            let mut invalid = doc.clone();
            invalid["vendor_specific"] = serde_json::json!(text);
            assert!(description(&invalid).is_err());
        }
        let mut overlap = document(
            &original,
            serde_json::json!([{"offset": "0x1c", "value": "00"}]),
        );
        overlap["vendor_specific"] = serde_json::json!("SpiralDrive v1.0.0");
        assert!(patch_image(&original, &description(&overlap).unwrap(), Some(&payload)).is_err());
        let mut bad_sha = doc;
        bad_sha["sha1"] = serde_json::json!("0".repeat(40));
        assert!(patch_image(&original, &description(&bad_sha).unwrap(), Some(&payload)).is_err());
    }

    #[test]
    fn munchen_updates_both_revision_fields_and_protects_its_layout() {
        let mut original = vec![0xff; IMAGE_SIZE];
        let identification = b"PLEXTOR \0CD-R   PX-W8432T\0";
        original[0x5bd20..0x5bd20 + identification.len()].copy_from_slice(identification);
        original[0x5ff1c..0x5ff1c + 15].copy_from_slice(b"PLEXTOR MUNCHEN");
        let sum = original[0x10000..0x5fffe]
            .iter()
            .map(|&b| u32::from(b))
            .sum::<u32>() as u16;
        original[0x5fffe..0x60000].copy_from_slice(&sum.to_be_bytes());
        let mut description = Description {
            sha1: Sha1::digest(&original)
                .iter()
                .map(|b| format!("{b:02x}"))
                .collect(),
            revision: "G109".into(),
            patches: vec![Patch {
                offset: 0x5f560,
                value: PatchValue::Bytes(vec![0x42]),
            }],
            payload: None,
            vendor_specific: None,
        };
        let result = patch_image(&original, &description, None).unwrap();
        for offset in [0x5b99a, 0x5bd3a] {
            assert_eq!(&result.bytes[offset..offset + 4], b"G109");
        }
        let expected = result.bytes[0x10000..0x5fffe]
            .iter()
            .map(|&b| u32::from(b))
            .sum::<u32>() as u16;
        assert_eq!(&result.bytes[0x5fffe..0x60000], &expected.to_be_bytes());
        for offset in [0, 0x60000, 0x5b99a, 0x5bd3a, 0x5fffe] {
            description.patches[0].offset = offset;
            assert!(patch_image(&original, &description, None).is_err());
        }
    }

    #[test]
    fn model_layouts_preserve_reserved_bytes_and_place_checksum() {
        for (model, family, header, checksum_offset, bias, erased_probe) in [
            ("PX-W4824A", "YORK", 0, 0x79ffe, 0x8000, 0x70000),
            ("PX-W5224A", "MONT", 0, 0x79ffe, 0x8000, 0x70000),
            ("PX-W2410A", "WINCHES", 0x10000, 0x77ffe, 0, 0),
            ("PX-W1610A", "SEATTLE", 0x10000, 0x77ffe, 0, 0),
            ("PX-S88T", "OTTAWA", 0x10000, 0x77ffe, 0, 0),
        ] {
            let mut original = vec![0xff; IMAGE_SIZE];
            let identification = format!("PLEXTOR CD-R   {model}");
            original[header..header + identification.len()]
                .copy_from_slice(identification.as_bytes());
            let family = format!("PLEXTOR {family}");
            original[header + 0x30..header + 0x30 + family.len()]
                .copy_from_slice(family.as_bytes());
            let sum = original[..checksum_offset]
                .iter()
                .fold(bias, |sum: u32, &byte| sum + u32::from(byte)) as u16;
            original[checksum_offset..checksum_offset + 2].copy_from_slice(&sum.to_be_bytes());
            let mut description = Description {
                sha1: Sha1::digest(&original)
                    .iter()
                    .map(|byte| format!("{byte:02x}"))
                    .collect(),
                revision: "T104".into(),
                patches: vec![Patch {
                    offset: header + 0x50,
                    value: PatchValue::Bytes(vec![0x42]),
                }],
                payload: None,
                vendor_specific: None,
            };
            let result = patch_image(&original, &description, None).unwrap();
            assert_eq!(&result.bytes[header + 0x18..header + 0x1c], b"T104");
            assert_eq!(result.bytes[header + 0x50], 0x42);
            assert_eq!(result.bytes[erased_probe], 0xff);
            let expected = result.bytes[..checksum_offset]
                .iter()
                .fold(bias, |sum: u32, &byte| sum + u32::from(byte))
                as u16;
            assert_eq!(
                &result.bytes[checksum_offset..checksum_offset + 2],
                &expected.to_be_bytes()
            );
            for offset in [erased_probe, header + 0x18, checksum_offset] {
                description.patches[0].offset = offset;
                assert!(
                    patch_image(&original, &description, None)
                        .err()
                        .unwrap()
                        .to_string()
                        .contains("overlaps")
                );
            }
            original[header + 0x51] ^= 1;
            description.sha1 = Sha1::digest(&original)
                .iter()
                .map(|byte| format!("{byte:02x}"))
                .collect();
            assert!(
                patch_image(&original, &description, None)
                    .err()
                    .unwrap()
                    .to_string()
                    .contains("checksum mismatch")
            );
        }
    }
}
