**English** | [Русский](https://github.com/libscanner/universal-reader/blob/main/README.ru.md)

# Universal code reader library (unir)

Reads codes of several symbologies in a frame with one call: **DataMatrix** (ECC 200, including GS1 / Chestny ZNAK marking codes — the Russian national product marking system), **GS1-128** (SSCC on logistics labels), **Code 128**, retail barcodes **EAN-13, EAN-8, UPC-A, UPC-E** and **QR** (model 2, versions 1–40, including GS1 QR).

`unir` is a commercial, closed-source C++20 library with a flat C ABI, a Python package and a command-line tool. One time budget per frame is shared between all symbologies, and one license covers them all. An optional scout network (shipped in the package) finds the codes first and speeds up reading of several codes in a frame.

This repository contains the public headers, examples, documentation links and license texts. **It contains no library source code.** Prebuilt binaries are attached to [GitHub Releases](https://github.com/libscanner/universal-reader/releases/latest).

**For AI coding assistants:** start with [`llms.txt`](llms.txt) — an index of the headers, examples and the full documentation in Markdown.

**[Read a code online from a photo](https://libscanner.com/en/scan/)** — free, in the browser, nothing to install.

## Platforms

| Platform | C++ static library + C ABI + CLI | Python wheel |
|---|---|---|
| Windows x64 | `unir-1.2.4-windows-x64.zip` | `libscanner_unir-1.2.4-py3-none-win_amd64.whl` |
| Linux x86-64 | `unir-1.2.4-linux-x64.tar.gz` | `libscanner_unir-1.2.4-py3-none-manylinux_2_36_x86_64.whl` |
| Linux ARM64 (Raspberry Pi 3/4/5, 64-bit) | `unir-1.2.4-linux-arm64.tar.gz` | `libscanner_unir-1.2.4-py3-none-manylinux_2_36_aarch64.whl` |
| Linux ARMv7 (Raspberry Pi 2/3/4/5, 32-bit) | `unir-1.2.4-linux-armhf.tar.gz` | `libscanner_unir-1.2.4-py3-none-manylinux_2_36_armv7l.whl` |

Each platform also has a CLI-only archive (`…-cli.zip` / `…-cli.tar.gz`) with just the `unir` tool.

- Windows packages are built with MinGW-w64 GCC (MSYS2 MINGW64). MSVC and other compilers use the C ABI (`unir_c.dll`).
- Linux packages are built with GCC 12 on Debian 12 (glibc 2.36): GCC 12 or newer is required for the C++ API; wheels need glibc 2.36+ (Debian 12 / Raspberry Pi OS Bookworm or newer).
- Python 3.9 or newer, no dependencies.

## Download

- **Binaries:** [GitHub Releases → latest](https://github.com/libscanner/universal-reader/releases/latest) — archives for each platform and Python wheels.
- **Python:** [PyPI → libscanner-unir](https://pypi.org/project/libscanner-unir/) — `pip install libscanner-unir`, imported as `unir`.
- **Licenses:** buy a plan at <https://libscanner.com/en/>. The activation key appears in your account after purchase.

### Trial

Every build works for **30 days from the first run on a machine** — no registration, no key, no network access during the trial. After that a license key from <https://libscanner.com> is required; until then `scan()` returns `Status::Unlicensed` without processing the frame.

**Containers (Docker, Kubernetes)** — since 1.2 a container has no trial without a key: the library runs under an **instance** license, the key goes into the `UNIR_LICENSE_KEY` environment variable or the `/run/secrets/unir.key` secret, and containers can be recreated freely. Get a container trial key (2 instances, 30 days) in your account at <https://libscanner.com/en/>. Details: [license in Docker and Kubernetes](https://libscanner.com/en/articles/library-license-docker-kubernetes/).

## Package contents

```text
include/unir/{Unir,Geometry,Image,License,Scanner,Settings,Symbology,Version}.h   C++ API
include/unir/unir_c.h              C ABI
lib/libunir.a                      C++ static library
lib/pkgconfig/unir.pc
lib/libunir_c.so | bin/unir_c.dll  C ABI library (+ lib/unir_c.dll.a on Windows)
lib/pkgconfig/unir_c.pc
bin/unir                           command-line tool
share/doc/unir/licenses/           third-party licenses
```

The headers in [`include/`](https://github.com/libscanner/universal-reader/tree/main/include/unir) of this repository declare exactly the same API as the headers shipped in the 1.2.4 packages; only the comments differ (translated to English here, Russian in the packages).

## Quick start

### C++

```cpp
#include <unir/Unir.h>
#include <iostream>

int main() {
    unir::Settings settings = unir::Settings::multi(0);   // every code in the frame
    settings.symbologies = unir::kAllSymbologies;         // or e.g. Symbology::DataMatrix | Symbology::Gs1_128

    const unir::Scanner scanner(settings);
    const unir::ScanResult r = scanner.scanFile("label.jpg");  // loading counts against the frame deadline

    for (const unir::Code& c : r.codes)
        std::cout << unir::nameOf(c.symbology) << ": " << c.text << "\n";
}
```

Link statically through pkg-config — the query **must** be `--static`:

```sh
PKG_CONFIG_PATH=/path/to/unir-1.2.4-linux-x64/lib/pkgconfig pkg-config --static --cflags --libs unir
```

A frame from your own capture pipeline (camera SDK, OpenCV buffer, bitmap) is passed as a view over the existing buffer, without copying:

```cpp
const unir::ImageView view{ m.data, m.cols, m.rows, (int)m.step, unir::PixelFormat::Bgr8 };
const unir::ScanResult r = scanner.scan(view);
```

Complete example: [`examples/cpp`](https://github.com/libscanner/universal-reader/tree/main/examples/cpp). C ABI example (MSVC, other languages): [`examples/c`](https://github.com/libscanner/universal-reader/tree/main/examples/c).

### Python

```sh
pip install libscanner-unir     # PyPI: Windows x64, Linux x86-64 / ARM64 / ARMv7 (glibc 2.36+)
```

```python
import unir

result = unir.scan_file("label.jpg", unir.Settings.multi(0))
for code in result.codes:
    print(code.symbology_name, code.text.decode("utf-8", errors="replace"))
```

`Code.text` is `bytes`: the GS1 separator stays as byte `0x1D`. The wheel also installs the `unir` command. Example: [`examples/python`](https://github.com/libscanner/universal-reader/tree/main/examples/python).

### Command line

```sh
unir label.jpg
unir label.jpg --codes 0 --sym dm,gs1
unir --folder ./frames
unir --license status --json
```

More commands: [`examples/cli`](https://github.com/libscanner/universal-reader/tree/main/examples/cli).

Messages are in the system language — English on any non-Russian system, Russian on a Russian one; `--lang en|ru` or the `LIBSCANNER_LANG` environment variable set it explicitly.

## Documentation

- C++ / C ABI / CLI: <https://libscanner.com/en/docs/universal-reader/> (Russian: <https://libscanner.com/docs/universal-reader/>)
- Python: <https://libscanner.com/en/docs/universal-reader/?lang=python> (Russian: <https://libscanner.com/docs/universal-reader/?lang=python>)

## License

Proprietary — see [LICENSE.txt](https://github.com/libscanner/universal-reader/blob/main/LICENSE.txt) (End User License Agreement; the Russian text is binding, [LICENSE.en.txt](https://github.com/libscanner/universal-reader/blob/main/LICENSE.en.txt) is an English translation for information). Starting to use the software means accepting the agreement.

Third-party components: [THIRD-PARTY-NOTICES.txt](https://github.com/libscanner/universal-reader/blob/main/THIRD-PARTY-NOTICES.txt) and the license texts in [`licenses/`](https://github.com/libscanner/universal-reader/tree/main/licenses).

Code in [`examples/`](https://github.com/libscanner/universal-reader/tree/main/examples) is licensed under MIT-0 ([examples/LICENSE](https://github.com/libscanner/universal-reader/blob/main/examples/LICENSE)) — copy it into your projects freely; the library itself is proprietary (LICENSE.txt).

## Contact

- Email: [libscanner@yandex.com](mailto:libscanner@yandex.com)
- Feedback form: <https://libscanner.com/en/feedback>
- Security issues: see [SECURITY.md](https://github.com/libscanner/universal-reader/blob/main/SECURITY.md)

“GS1” is a trademark of GS1 AISBL. “Chestny ZNAK” («Честный знак») is a trademark of Operator-CRPT LLC (ООО «Оператор-ЦРПТ»). This product is not affiliated with or certified by the owners of these marks; the names are used only to describe the supported formats.
