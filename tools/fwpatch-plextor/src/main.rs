mod file_io;
mod patch;

use anyhow::{Context, Result, ensure};
use std::{env, io::Write, path::PathBuf, process::ExitCode};

fn run() -> Result<()> {
    let args: Vec<_> = env::args_os().skip(1).collect();
    ensure!(
        args.len() == 2,
        "Usage: fwpatch-plextor <original-firmware> <description.json>"
    );
    let firmware_path = PathBuf::from(&args[0]);
    let description_path = PathBuf::from(&args[1]);
    let description = patch::parse_description(&file_io::read_file(&description_path)?)
        .with_context(|| format!("invalid description '{}'", description_path.display()))?;
    let original = file_io::read_file(&firmware_path)?;
    let result = patch::patch_image(&original, &description)?;
    let destination = file_io::output_path(&firmware_path)?;
    file_io::write_output(
        &destination,
        &result.bytes,
        &firmware_path,
        &description_path,
    )?;

    let mut output = std::io::stdout().lock();
    writeln!(output, "Output: {}", destination.display())?;
    writeln!(output, "Revision: {}", description.revision())?;
    writeln!(output, "Patches: {}", description.patch_count())?;
    writeln!(output, "Checksum: 0x{:04x}", result.checksum)?;
    Ok(())
}

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            // Return normally so temporary files are cleaned up on failure.
            let _ = writeln!(std::io::stderr().lock(), "fwpatch-plextor: {error:#}");
            ExitCode::FAILURE
        }
    }
}
