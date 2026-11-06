#!/bin/sh
set -eu

if [ "$#" -lt 1 ] || [ "$#" -gt 3 ]; then
    echo "usage: GHIDRA_HOME=/path/to/ghidra $0 PROJECT_DIR [PROJECT_NAME [--prepared-baseline]]" >&2
    exit 2
fi

: "${GHIDRA_HOME:?set GHIDRA_HOME to the Ghidra installation directory}"

project_dir=$1
project_name=${2:-gplextor}
baseline_argument=${3:-}
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
headless=$GHIDRA_HOME/support/analyzeHeadless

if [ -n "$baseline_argument" ] &&
        [ "$baseline_argument" != "--prepared-baseline" ]; then
    echo "unknown validation option: $baseline_argument" >&2
    exit 2
fi

if [ ! -x "$headless" ]; then
    echo "analyzeHeadless is not executable: $headless" >&2
    exit 1
fi
if [ ! -e "$project_dir/$project_name.gpr" ] ||
        [ ! -d "$project_dir/$project_name.rep" ]; then
    echo "complete Ghidra project does not exist: $project_dir/$project_name" >&2
    exit 1
fi

validate_program() {
    program_name=$1
    validation_log=$(mktemp "${TMPDIR:-/tmp}/gplextor-ghidra-validation.XXXXXX")
    if [ "$program_name" = MONT_104.bin ]; then
        if ! "$headless" "$project_dir" "$project_name/validated" \
                -process "$program_name" \
                -readOnly \
                -noanalysis \
                -scriptPath "$script_dir/ghidra_scripts" \
                -postScript ValidatePlextorCanonicalProject.java "$baseline_argument" \
                -postScript ValidatePlextor80251Language.java >"$validation_log" 2>&1; then
            cat "$validation_log"
            rm -f "$validation_log"
            return 1
        fi
    else
        if ! "$headless" "$project_dir" "$project_name/validated" \
                -process "$program_name" \
                -readOnly \
                -noanalysis \
                -scriptPath "$script_dir/ghidra_scripts" \
                -postScript ValidatePlextorCanonicalProject.java "$baseline_argument" >"$validation_log" 2>&1; then
            cat "$validation_log"
            rm -f "$validation_log"
            return 1
        fi
    fi
    cat "$validation_log"
    if grep -Fq 'ERROR REPORT SCRIPT ERROR' "$validation_log" ||
            grep -Fq 'CANONICAL_PROJECT_VALIDATION=FAIL' "$validation_log" ||
            ! grep -Fq 'CANONICAL_PROJECT_VALIDATION=PASS' "$validation_log"; then
        rm -f "$validation_log"
        echo "canonical Ghidra validation did not pass for $program_name" >&2
        return 1
    fi
    if [ "$program_name" = MONT_104.bin ] &&
            { ! grep -Fq 'DECODER_PROBES=PASS' "$validation_log" ||
              ! grep -Fq 'PROCESSOR_LANGUAGE_VALIDATION=PASS' "$validation_log"; }; then
        rm -f "$validation_log"
        echo "processor-language validation did not pass for $program_name" >&2
        return 1
    fi
    rm -f "$validation_log"
}

validate_program MONT_104.bin
validate_program read_memory_ff0000.debug
