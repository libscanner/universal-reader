# Changelog

## 1.2.6 — 2026-11-12

- **No false reads of EAN/UPC and Code 128 on our photo sets.** We searched for false reads on 18,586 images: our 1D photo sets, the whole SmartEngines set (including 6,336 Code 39, Codabar and Code 93 images, where any read of ours is false), 4,231 images without a barcode and conveyor frames. In color, 1.2.5 returned 14 false codes there; in grayscale, 8. Now none, and none on the same images shifted by half a pixel, scaled by 0.9, with noise or JPEG 85. The causes were a piece of a longer symbol (or of a neighbouring one) read as a short code, because the quiet zone beyond the end of the scan line or the crop was treated as empty; and a long Code 128 decoded on the bars of another symbology. Quiet zones are now checked on the frame itself beyond the line and the crop, and a Code 128 must match the expected number of bars and spaces.
- **More codes on color photos.** When nothing is read on a color frame and a symbol is seen, the frame is read once more from its blue channel: colored bars or labels that lose contrast in the grayscale conversion keep it there. Nearly gray frames skip this pass. On the same images, 111 more codes read in color (5,472 → 5,583), with no false reads.
- SSCC reading is byte-for-byte the same (5,840 synthetic frames and the tilt grid).
- Frames where nothing is read take longer in color (about +28 % on average, within the same time budget); about +8 % on average over all frames.

## 1.2.5 — 2026-11-11

- **Faster on phone photos.** The DataMatrix engine is updated to dmr 1.1.2: a large candidate is downscaled already in the full pass (to 640 pixels instead of 2048), and the extra detectors for strongly tilted symbols run only on frames whose shorter side is under 1000 pixels — on a phone photo they took up to 2.8 s of a 4 s budget and read nothing.
- **QR codes taken for a barcode.** With the scout network on, a QR code that the network confidently took for a linear barcode was tried only by the linear engine and was lost. Now, if the linear engine reads nothing on such a find and the QR finder patterns are seen in it, the QR engine is tried too.
- On 609 unread photos from our online demo and chat bot (scout network on, 2 cores and a 4 s budget, as in our demo; two runs): 186 and 184 → 190 and 191 photos read with one code, median time 3770 → 3570 ms; 189 → 189 and 191 with all codes (the few photos that change either way are read at the very end of the budget). On our set of 130 photos with QR codes, all codes: 126 → 127. Camera, conveyor and EAN sets: the same frames read.

## 1.2.4 — 2026-11-10

- **No wrong SSCC numbers on heavy motion blur.** With motion blur of about 7 pixels across the bars (about three modules at 2.2 pixels per module) the reader could return a wrong SSCC: both the Code 128 checksum and the SSCC check digit matched and two scan lines of the same symbol agreed, but in one position two characters scored almost the same and the wrong one won. An SSCC line is now accepted only if it is confident overall or the best character beats the second best in every position by a margin. On 7840 synthetic SSCC frames: wrong reads 2 → 0, read 6211 → 6185 (all losses at 7 px motion blur); on real conveyor frames the results are the same. EAN/UPC and other Code 128 are not affected.

## 1.2.3 — 2026-11-10

- **Large DataMatrix on phone photos.** The DataMatrix engine is updated to dmr 1.1.1: a symbol that fills a big part of a photo (500–1800 pixels across, 15–80 pixels per module) was found but not read, because the sampling grid caught on the print texture inside the modules. When nothing is read, each large candidate is now retried cut out and downscaled to about 240 pixels across, and the whole frame is retried downscaled by 4, 6 and 8 for symbols the detectors see only in pieces (dot-peen marking, laser on metal). With short budgets (60–100 ms) the retries are skipped.
- On 609 unread photos from our online demo and chat bot (scout network on, 4 s budget): 174 → 198 photos read with one code, 172 → 198 with all codes; no frame lost, no string changed.
- Frames where nothing is read spend more time, within the same budget.

## 1.2.2 — 2026-11-09

- **Tilted labels are read.** A label tilted away from the camera around a slanted axis turns the barcode into a parallelogram: its bars are no longer perpendicular to the symbol axis, so a scan line across the bars left them before reaching the stop pattern. The reader now estimates this skew and reads such symbols along their axis. On our synthetic SSCC set with a label tilted by 55° and 65°: 50 of 50 frames each (21 and 15 before).
- **More linear codes on real photos.** EAN/UPC and Code 128 in a skewed region are read from a straightened crop. On public photo sets: DEAL 2843 → 2876 of 3200, SmartEngines 1273 → 1391 of 3000, Open Food Facts 134 → 137 of 145. No false reads on 4231 photos without a barcode.
- Frames where nothing is read take longer: about 92 → 142 ms on average with all symbologies enabled, within the same time budget.

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
