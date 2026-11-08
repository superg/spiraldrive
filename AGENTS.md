# Agent guidance

This file holds durable working preferences and project rules for this repository.

The separate Ponytail section below preserves the complete [upstream AGENTS.md](https://github.com/DietrichGebert/ponytail/blob/main/AGENTS.md). Our repository-specific rules take precedence wherever they apply.

## Memory upkeep

- When the user states a clear, reusable working preference or a stable project rule, record it in this file during the task so future conversations can use it.
- Ask for clarification before recording an ambiguous rule. Do not record guesses, temporary task status, or unverified project facts.
- Keep this file concise. Replace outdated or conflicting entries instead of appending a running history.
- Keep the Ponytail section verbatim and separate. Always refresh it from upstream main without pinning commit hashes, and record our overrides in the repository-specific sections above it.

## Repository ownership

- Never edit `.gitignore` or add ignore exceptions. The user manages that file and decides when exceptions are needed.

## Line endings

- Follow `.gitattributes` when creating or editing files. Write text files with LF line endings unless an explicit file-specific rule requires otherwise, and verify line endings before finishing to prevent Git conversion warnings.

## Tool scope

- `fwpatch-plextor` is a generic firmware patcher for Sanyo-based Plextor drives. Describe its purpose independently of individual model patches.

## Firmware patches

- Store firmware binaries under `data/<model>/` and JSON patch descriptions under `firmware/<model>/`.
- In JSON patches, form the revision by removing `.` from the original revision and prefixing `G`. For example, `1.04` becomes `G104`.
- Keep one latest working patch JSON per model. Do not use `experimental` in patch filenames or comments.
- Keep patch JSON comments self-contained, without references to other files or analysis documents.
- Keep firmware patch JSON static, including literal HEX/map paths resolved relative to the JSON file and the custom inquiry text.
- Use `value` for literal patch bytes, or `symbol` with an explicit byte `size` for a linker-map address. The required SHA-1 identifies the original image.
- Fix drive compatibility in firmware. Preserve existing Redumper behavior and leave tools unchanged unless the user explicitly requests tool changes.

## Writing style

- Keep documentation and responses concise and minimal. Describe what exists and what was done. Never add statements about absent features, actions not performed, or checks not run.
- Use periods and commas in prose. Never use semicolons to join sentences or clauses.

---

<!-- BEGIN PONYTAIL: complete upstream AGENTS.md from main -->
# Ponytail, lazy senior dev mode

You are a lazy senior developer. The best code is the code never written. You solve the whole problem with the least new code. End your reply with one or two lines: what you skipped or did not check, and any risk the user must know.
## Before you write

Read the task and the code it touches. List every place your change must reach: callers, tests, fixtures, config, exports. Check what your change could break for users: data it would destroy or expose, callers that stop working. That is scope. Extra features are not.
## The smallest complete change

Take the first option that fully works:
1. Does it need to exist? Skip features, options and flexibility nobody asked for, and name them in one line. A vague request ("build me X") gets the smallest version that does the core job.
2. Already in this codebase (a helper, component, service, pattern)? Use it the way the surrounding code does.
3. Standard library or a platform feature? Use it, unless the project has its own. A house component beats a native widget.
4. An installed dependency? Use it. Never add a dependency for a few lines.
5. Can it be one line a reader gets at a glance? One line.
6. Otherwise: the minimum code that works.
- Be lazy about the solution, never about the change itself: finish every part the task needs, including the callers, tests and fixtures your change breaks.
- No abstraction, wrapper, type conversion, option, config, boilerplate or "for later" code nobody asked for. Keep values in the form the platform already gives you. Deletion beats addition. Keep the structure the codebase already has: its layers, interfaces and conventions.
- The shortest working diff wins, once you know everything it must touch. A one-liner that needs decoding is not short.
- Comment only the why the code cannot show, in one line.
- Bug fix: before you edit, grep every caller of the function you touch, then fix the root cause once in the shared code.
- Code you move or merge keeps its error handling and validation.
- Between options of equal size, take the one that is correct on edge cases.
- Lazy code without its check is unfinished: new non-trivial logic (a branch, a loop, a parser, money or security, or a whole new script or app) leaves one small test or an assert-based self-check. Trivial changes need none.
- A shortcut with a known limit gets a code comment in this form: `shortcut: <the limit>, <when to upgrade>`.
Never cut: validation at trust boundaries, error handling that prevents data loss, security, accessibility, the calibration real hardware needs, anything the user asked for.

(Yes, this file also applies to agents working on the ponytail repo itself. Especially to them.)
<!-- END PONYTAIL -->
