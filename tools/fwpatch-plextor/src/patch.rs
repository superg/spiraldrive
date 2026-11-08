use anyhow::{Context, Result, ensure};
use serde::{
    Deserialize, Deserializer,
    de::{MapAccess, Visitor, value::MapAccessDeserializer},
};
use sha1::{Digest, Sha1};
use std::{fmt, marker::PhantomData, ops::Range};

const IMAGE_SIZE: usize = 0x80000;

struct ImageLayout {
    revisions: &'static [usize],
    checksum: usize,
    checksum_range: Range<usize>,
    checksum_bias: u32,
    erased: &'static [Range<usize>],
}

// Derived struct deserializers also accept sequences. Require JSON objects at
// both levels, then let Serde reject duplicate, missing, and unknown fields.
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
    // Default permits omission, while String rejects explicit null/comments
    // of other types. Renaming keeps this a recognized, duplicate-checked key.
    #[serde(default, rename = "comment")]
    _comment: String,
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct PatchFields {
    offset: String,
    old: String,
    new: String,
    #[serde(default, rename = "comment")]
    _comment: String,
}

struct Patch {
    offset: usize,
    old: Vec<u8>,
    new: Vec<u8>,
}

pub struct Description {
    sha1: String,
    revision: String,
    patches: Vec<Patch>,
}

impl Description {
    pub fn revision(&self) -> &str {
        &self.revision
    }

    pub fn patch_count(&self) -> usize {
        self.patches.len()
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
        let old = parse_bytes(&entry.old).with_context(|| format!("patch {number} old"))?;
        let new = parse_bytes(&entry.new).with_context(|| format!("patch {number} new"))?;
        ensure!(
            old.len() == new.len(),
            "patch {number}: old and new lengths differ"
        );
        patches.push(Patch { offset, old, new });
    }
    Ok(Description {
        sha1: raw.sha1.to_ascii_lowercase(),
        revision: raw.revision,
        patches,
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
            revisions: &[0x18],
            checksum: 0x79ffe,
            checksum_range: 0..0x79ffe,
            checksum_bias: 0x8000,
            erased: &[0x70000..0x78000, 0x7a000..IMAGE_SIZE],
        }
    } else if winc || seat || otta {
        ImageLayout {
            revisions: &[0x10018],
            checksum: 0x77ffe,
            checksum_range: 0..0x77ffe,
            checksum_bias: 0,
            erased: &[0..0x10000, 0x78000..IMAGE_SIZE],
        }
    } else if munc {
        ImageLayout {
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

pub fn patch_image(original: &[u8], description: &Description) -> Result<PatchedImage> {
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
    let mut reserved_ranges = layout.erased.to_vec();
    reserved_ranges.push(0..layout.checksum_range.start);
    reserved_ranges.push(layout.checksum..layout.checksum + 2);
    reserved_ranges.push(layout.checksum_range.end..IMAGE_SIZE);
    reserved_ranges.extend(layout.revisions.iter().map(|&offset| offset..offset + 4));

    let mut ranges = Vec::with_capacity(description.patches.len());
    for (index, patch) in description.patches.iter().enumerate() {
        let number = index + 1;
        let start = patch.offset;
        ensure!(
            start < original.len() && patch.old.len() <= original.len() - start,
            "patch {number} at {start:#x}: range is outside the firmware"
        );
        let end = start + patch.old.len();
        for reserved in &reserved_ranges {
            ensure!(
                !(start < reserved.end && end > reserved.start),
                "patch {number} at {start:#x}: overlaps an automatic field or protected firmware region"
            );
        }
        if let Some(mismatch) = original[start..end]
            .iter()
            .zip(&patch.old)
            .position(|(a, b)| a != b)
        {
            anyhow::bail!(
                "patch {number} at {start:#x}: old byte mismatch at {:#x}",
                start + mismatch
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
    for patch in &description.patches {
        bytes[patch.offset..patch.offset + patch.new.len()].copy_from_slice(&patch.new);
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
                old: vec![0xff],
                new: vec![0x42],
            }],
        };
        let result = patch_image(&original, &description).unwrap();
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
            assert!(patch_image(&original, &description).is_err());
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
                    old: vec![0xff],
                    new: vec![0x42],
                }],
            };
            let result = patch_image(&original, &description).unwrap();
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
                description.patches[0].old = vec![original[offset]];
                assert!(
                    patch_image(&original, &description)
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
                patch_image(&original, &description)
                    .err()
                    .unwrap()
                    .to_string()
                    .contains("checksum mismatch")
            );
        }
    }
}
