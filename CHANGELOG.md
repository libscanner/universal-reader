# Changelog

## 1.0.1 — 2026-11-02

First release on GitHub and PyPI.

- Messages in English: the command-line tool, license messages and library errors speak English on any non-Russian system and Russian on a Russian one. `--lang ru|en` or the `LIBSCANNER_LANG` environment variable set the language explicitly; the language of `status().message` and library errors follows the same variable. `--json` output and `--batch`/`--serve` result lines do not depend on the language.
- DataMatrix engine updated to dmr 1.0.13 (non-ASCII file paths on Windows and POSIX).

## 1.0.0 — 2026-10-25

First release.

- C++20 static library `libunir.a` (headers `include/unir/`), flat C ABI library `unir_c` (`unir/unir_c.h`).
- Python package `unir` (wheels for Windows x64 and Linux x86-64, ARM64, ARMv7; Python 3.9+), with the `unir` command.
- Command-line tool `unir`: single image, `--batch`, `--folder`, `--serve`, `--license`.
- Symbologies: DataMatrix (ECC 200), Code 128, GS1-128 (SSCC), EAN-13, EAN-8, UPC-A, UPC-E; one time budget per frame for all of them.
- Platforms: Windows x64, Linux x86-64, Linux ARM64, Linux ARMv7 (Raspberry Pi).
- 30-day trial from the first run on a machine; license activation online or offline.
