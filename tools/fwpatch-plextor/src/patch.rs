use anyhow::{Context, Result, ensure};
use serde::{
    Deserialize, Deserializer,
    de::{MapAccess, Visitor, value::MapAccessDeserializer},
};
use sha1::{Digest, Sha1};
use std::{fmt, marker::PhantomData, ops::Range};

const IMAGE_SIZE: usize = 0x80000;
const REVISION_OFFSET: usize = 0x18;
const CHECKSUM_OFFSET: usize = 0x79ffe;
const ERASED_RANGES: [Range<usize>; 2] = [0x70000..0x78000, 0x7a000..IMAGE_SIZE];
const RESERVED_RANGES: [Range<usize>; 3] = [
    REVISION_OFFSET..REVISION_OFFSET + 4,
    0x70000..0x78000,
    CHECKSUM_OFFSET..IMAGE_SIZE,
];

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

fn checksum(image: &[u8]) -> u16 {
    let sum = image[..CHECKSUM_OFFSET]
        .iter()
        .fold(0x8000u32, |sum, &byte| sum + u32::from(byte));
    (sum & 0xffff) as u16
}

fn validate_image(image: &[u8]) -> Result<()> {
    ensure!(
        image.len() == IMAGE_SIZE,
        "firmware must be exactly 0x80000 bytes (512 KiB)"
    );
    let york = image.starts_with(b"PLEXTOR CD-R   PX-W4824A")
        && image[0x30..].starts_with(b"PLEXTOR YORK");
    let mont = image.starts_with(b"PLEXTOR CD-R   PX-W5224A")
        && image[0x30..].starts_with(b"PLEXTOR MONT");
    ensure!(
        york || mont,
        "unsupported firmware identification: expected PX-W4824A/YORK or PX-W5224A/MONT"
    );
    for range in ERASED_RANGES {
        ensure!(
            image[range.clone()].iter().all(|&b| b == 0xff),
            "firmware erased region {:#x}..{:#x} is not all ff",
            range.start,
            range.end - 1
        );
    }
    let stored = u16::from_be_bytes([image[CHECKSUM_OFFSET], image[CHECKSUM_OFFSET + 1]]);
    let calculated = checksum(image);
    ensure!(
        stored == calculated,
        "firmware checksum mismatch: stored {stored:#06x}, calculated {calculated:#06x}"
    );
    Ok(())
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
    validate_image(original)?;

    let mut ranges = Vec::with_capacity(description.patches.len());
    for (index, patch) in description.patches.iter().enumerate() {
        let number = index + 1;
        let start = patch.offset;
        ensure!(
            start < original.len() && patch.old.len() <= original.len() - start,
            "patch {number} at {start:#x}: range is outside the firmware"
        );
        let end = start + patch.old.len();
        for reserved in RESERVED_RANGES {
            ensure!(
                !(start < reserved.end && end > reserved.start),
                "patch {number} at {start:#x}: overlaps an automatic field or erased recovery/padding region"
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
    bytes[REVISION_OFFSET..REVISION_OFFSET + 4].copy_from_slice(description.revision.as_bytes());
    let sum = checksum(&bytes);
    bytes[CHECKSUM_OFFSET..CHECKSUM_OFFSET + 2].copy_from_slice(&sum.to_be_bytes());
    validate_image(&bytes)?;
    Ok(PatchedImage {
        bytes,
        checksum: sum,
    })
}
