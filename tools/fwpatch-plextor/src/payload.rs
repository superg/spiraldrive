use anyhow::{Context, Result, ensure};
use std::collections::BTreeMap;

pub struct Payload {
    pub bytes: BTreeMap<u32, u8>,
    pub symbols: BTreeMap<String, u32>,
}

pub fn parse(hex: &[u8], map: &[u8]) -> Result<Payload> {
    let bytes = parse_hex(std::str::from_utf8(hex).context("payload HEX is not UTF-8")?)?;
    let mut symbols = BTreeMap::new();
    for line in std::str::from_utf8(map)
        .context("payload map is not UTF-8")?
        .lines()
    {
        let fields: Vec<_> = line.split_ascii_whitespace().collect();
        let (address, name) = match fields.as_slice() {
            [address, "ECODE", "---", name] | [address, "PUBLIC", "ECODE", "---", name] => {
                (address, name)
            }
            _ => continue,
        };
        let address = address.strip_suffix('H').context("invalid map address")?;
        let address = u32::from_str_radix(address, 16).context("invalid map address")?;
        let name = name.trim_end_matches('?');
        if let Some(previous) = symbols.insert(name.to_owned(), address) {
            ensure!(previous == address, "conflicting map symbol: {name}");
        }
    }
    Ok(Payload { bytes, symbols })
}

fn parse_hex(hex: &str) -> Result<BTreeMap<u32, u8>> {
    let mut bytes = BTreeMap::new();
    let mut upper = 0u32;
    let mut ended = false;
    for (index, line) in hex.lines().enumerate() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        let number = index + 1;
        ensure!(!ended, "HEX line {number}: record after EOF");
        let digits = line
            .strip_prefix(':')
            .context("HEX record must start with ':'")?;
        ensure!(
            digits.len() >= 10
                && digits.len().is_multiple_of(2)
                && digits.bytes().all(|b| b.is_ascii_hexdigit()),
            "HEX line {number}: invalid record"
        );
        let record: Vec<u8> = (0..digits.len())
            .step_by(2)
            .map(|i| u8::from_str_radix(&digits[i..i + 2], 16))
            .collect::<std::result::Result<_, _>>()?;
        let count = usize::from(record[0]);
        ensure!(
            record.len() == count + 5,
            "HEX line {number}: length mismatch"
        );
        ensure!(
            record.iter().fold(0u8, |sum, b| sum.wrapping_add(*b)) == 0,
            "HEX line {number}: checksum mismatch"
        );
        let offset = u32::from(u16::from_be_bytes([record[1], record[2]]));
        match record[3] {
            0 => {
                ensure!(
                    offset + count as u32 <= 0x10000,
                    "HEX line {number}: record crosses a 64-KiB boundary"
                );
                for (i, &value) in record[4..4 + count].iter().enumerate() {
                    let address = upper + offset + i as u32;
                    ensure!(
                        address <= 0xffffff,
                        "HEX line {number}: address exceeds 24 bits"
                    );
                    ensure!(
                        bytes.insert(address, value).is_none(),
                        "HEX line {number}: duplicate address {address:#x}"
                    );
                }
            }
            1 => {
                ensure!(count == 0 && offset == 0, "HEX line {number}: invalid EOF");
                ended = true;
            }
            4 => {
                ensure!(
                    count == 2 && offset == 0,
                    "HEX line {number}: invalid extended linear address"
                );
                upper = u32::from(u16::from_be_bytes([record[4], record[5]])) << 16;
                ensure!(
                    upper <= 0xff0000,
                    "HEX line {number}: address exceeds 24 bits"
                );
            }
            kind => anyhow::bail!("HEX line {number}: unsupported record type {kind}"),
        }
    }
    ensure!(ended, "HEX has no EOF record");
    ensure!(!bytes.is_empty(), "HEX contains no payload");
    Ok(bytes)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn record(kind: u8, offset: u16, data: &[u8]) -> String {
        let mut bytes = vec![data.len() as u8, (offset >> 8) as u8, offset as u8, kind];
        bytes.extend_from_slice(data);
        bytes.push(0u8.wrapping_sub(bytes.iter().fold(0u8, |sum, b| sum.wrapping_add(*b))));
        format!(
            ":{}\n",
            bytes.iter().map(|b| format!("{b:02X}")).collect::<String>()
        )
    }

    fn valid_hex() -> String {
        record(4, 0, &[0, 0xfd])
            + &record(0, 0xeae0, &[0x42, 0x43])
            + &record(4, 0, &[0, 0xfe])
            + &record(0, 0xf194, &[0x44])
            + &record(1, 0, &[])
    }

    #[test]
    fn reads_sparse_hex_and_keil_symbols() {
        let map = b"00FDEAE0H ECODE --- handler??\n00FDEAE0H PUBLIC ECODE --- handler??\n00FEF194H PUBLIC ECODE --- event??\n";
        let payload = parse(valid_hex().as_bytes(), map).unwrap();
        assert_eq!(payload.bytes.len(), 3);
        assert_eq!(payload.bytes[&0xfef194], 0x44);
        assert_eq!(payload.symbols["handler"], 0xfdeae0);
        assert!(
            parse(
                valid_hex().as_bytes(),
                b"00FDEAE0H ECODE --- handler??\n00FEF194H ECODE --- handler??"
            )
            .is_err()
        );
    }

    #[test]
    fn rejects_malformed_hex() {
        let extended = record(4, 0, &[0, 0xfd]);
        let data = record(0, 0xeae0, &[0x42]);
        let eof = record(1, 0, &[]);
        for invalid in [
            String::new(),
            extended.clone() + &data,
            extended.clone() + &data + &data + &eof,
            valid_hex() + &data,
            extended.clone() + &record(0, 0xffff, &[1, 2]) + &eof,
            record(4, 0, &[1, 0]) + &data + &eof,
            record(4, 1, &[0, 0xfd]) + &data + &eof,
            extended.clone() + &data + &record(1, 1, &[]),
            extended + &record(2, 0, &[0, 0]) + &data + &eof,
            valid_hex().replacen("FD", "FC", 1),
            valid_hex().replacen(":02", ":03", 1),
            String::from(":01000000é0\n"),
        ] {
            assert!(parse_hex(&invalid).is_err(), "accepted {invalid:?}");
        }
    }
}
