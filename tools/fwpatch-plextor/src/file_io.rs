use anyhow::{Context, Result, ensure};
use std::{
    env,
    fs::{self, File},
    io::{ErrorKind, Read, Write},
    path::{Path, PathBuf},
};

pub fn read_file(path: &Path) -> Result<Vec<u8>> {
    let mut file =
        File::open(path).with_context(|| format!("cannot open input '{}'", path.display()))?;
    let metadata = file
        .metadata()
        .with_context(|| format!("cannot inspect input '{}'", path.display()))?;
    ensure!(
        metadata.is_file(),
        "input is not a regular file: '{}'",
        path.display()
    );
    let mut bytes = Vec::new();
    file.read_to_end(&mut bytes)
        .with_context(|| format!("cannot read input '{}'", path.display()))?;
    ensure!(
        bytes.len() as u64 == metadata.len(),
        "input size changed while reading '{}'",
        path.display()
    );
    Ok(bytes)
}

pub fn output_path(firmware: &Path) -> Result<PathBuf> {
    let mut name = firmware
        .file_stem()
        .context("firmware path has no filename")?
        .to_os_string();
    name.push("_patched");
    if let Some(extension) = firmware.extension() {
        name.push(".");
        name.push(extension);
    }
    Ok(env::current_dir()
        .context("cannot determine current working directory")?
        .join(name))
}

fn protect_inputs(output: &Path, inputs: &[PathBuf]) -> Result<()> {
    match fs::metadata(output) {
        Ok(_) => {
            for input in inputs {
                let same = same_file::is_same_file(output, input).with_context(|| {
                    format!(
                        "cannot compare output '{}' with input '{}'",
                        output.display(),
                        input.display()
                    )
                })?;
                ensure!(!same, "output would overwrite input '{}'", input.display());
            }
        }
        // Also permits replacing a dangling output symlink as a directory
        // entry. We never follow that symlink to write its target.
        Err(error) if error.kind() == ErrorKind::NotFound => {}
        Err(error) => {
            return Err(error)
                .with_context(|| format!("cannot inspect output '{}'", output.display()));
        }
    }
    Ok(())
}

pub fn write_output(output: &Path, bytes: &[u8], inputs: &[PathBuf]) -> Result<()> {
    protect_inputs(output, inputs)?;
    let directory = output.parent().context("output has no parent directory")?;
    let mut temporary = tempfile::Builder::new()
        .prefix(".fwpatch-plextor-")
        .suffix(".tmp")
        .tempfile_in(directory)
        .with_context(|| {
            format!(
                "cannot create temporary output in '{}'",
                directory.display()
            )
        })?;
    temporary
        .write_all(bytes)
        .context("cannot write temporary output")?;
    temporary
        .as_file()
        .sync_all()
        .context("cannot synchronize temporary output")?;
    protect_inputs(output, inputs)?;
    // persist atomically replaces an existing entry on the same filesystem.
    // Keep the returned file owner on failure so its Drop removes the temp.
    if let Err(error) = temporary.persist(output) {
        let tempfile::PersistError {
            error,
            file: _temporary,
        } = error;
        return Err(error).with_context(|| format!("cannot replace output '{}'", output.display()));
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn protects_every_input_and_hard_link_before_replacing_output() {
        let directory = tempfile::tempdir().unwrap();
        let inputs: Vec<_> = ["stock.bin", "patch.json", "payload.hex", "payload.map"]
            .iter()
            .map(|name| directory.path().join(name))
            .collect();
        for input in &inputs {
            fs::write(input, b"original").unwrap();
        }
        let output = directory.path().join("patched.bin");
        for input in &inputs {
            assert!(write_output(input, b"patched", &inputs).is_err());
            fs::hard_link(input, &output).unwrap();
            assert!(write_output(&output, b"patched", &inputs).is_err());
            assert_eq!(fs::read(input).unwrap(), b"original");
            fs::remove_file(&output).unwrap();
        }
        write_output(&output, b"patched", &inputs).unwrap();
        write_output(&output, b"replacement", &inputs).unwrap();
        assert_eq!(fs::read(output).unwrap(), b"replacement");
        assert_eq!(
            fs::read_dir(directory.path()).unwrap().count(),
            inputs.len() + 1
        );
    }
}
