# Changelog

## 1.2.1 — 2026-11-06

- **The time budget is kept on large photos.** On 12-megapixel photos with a textured background the 1D barcode detector merged thousands of small regions pair by pair before its first budget check, so with `budgetMs` = 60 a frame could take up to 0.7 s (once 2 s). The merge now looks up neighbours by position and bar angle; the budget is overrun by at most ~30 ms. Results are byte-for-byte the same as in 1.2.0.

## 1.2.0 — 2026-11-06

Licensing for containers; reading is the same as in 1.1.0.

- **Containers: instance license.** In Docker and Kubernetes the library now runs under an *instance* license: each running process takes a seat on the license server at start, reports in the background every 30 minutes and frees the seat on a normal exit; containers can be recreated freely. Pass the key in the `UNIR_LICENSE_KEY` environment variable or as the `/run/secrets/unir.key` secret (checked in that order, then the key file next to the library). What counts is how many instances run at the same time; peaks up to twice the purchased number are allowed within a monthly allowance. Without a connection to the license server an instance keeps working for 7 days since its last report.
- **No trial without a key in a container.** Container state disappears on every restart, so a local trial would restart forever; instead get a container trial key (2 instances, 30 days) in your account. Outside containers the 30-day trial works as before. A machine license key is not activated in a container — the message says an instance license is needed.
- **Worker processes after fork** (gunicorn with `--preload`, `multiprocessing`, uWSGI without `lazy-apps`) are separate instances: each worker takes its own seat on its first scan, and a master process that does not scan itself frees its seat within 30 minutes.
- **Emergency extension.** If our license server is unavailable for a long time, the offline period can be extended by a signed notice published on independent mirrors (up to 30 days); the library fetches it by itself. `LicenseStatus::emergencyUntil` (`emergency_until` in Python and in `--license status --json`) shows the extended deadline.
- **Offline machines for the whole term.** A machine without internet can be activated with a file for the whole paid subscription term: tick "Machine without internet — work the whole term offline" when uploading the activation request in your account.
- Details: [license in Docker and Kubernetes](https://libscanner.com/en/articles/library-license-docker-kubernetes/).

## 1.1.0 — 2026-11-05

- **QR codes**: QR model 2, versions 1–40, including GS1 QR (`Symbology::Qr`, `UNIR_SYM_QR`, `qr` in `--sym`). Text is UTF-8 (ECI, kanji from Shift-JIS); an FNC1 inside GS1 QR is the Gs separator (0x1D), as with DataMatrix.
- **Scout network** (optional, shipped in the package as `share/unir/net/seg16a.unet`; in the wheel — `unir.default_net_model()`): finds the codes first (class and region, 15–25 ms), the reader then decodes the crops. `Settings::netModel`, `netConf`, `netFallback`, `threads`; `Scanner::netReady()` / `netError()`; `--net` in the command-line tool.
- **C ABI 1.1**: `net_model`, `net_conf`, `net_fallback`, `threads` appended to `unir_settings_t` (callers built with the 1.0 header work as before), `unir_scanner_net_ready()`, `unir_scanner_net_error()`.

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
