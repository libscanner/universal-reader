# SPDX-License-Identifier: MIT-0
"""Read DataMatrix, Code 128, GS1-128 and EAN/UPC codes with the unir Python package.

Install the package from PyPI first:

    pip install libscanner-unir

Usage:

    python example.py label.jpg [more images...]
"""

import sys

import unir
import unir.license as license


def main(paths):
    print("unir", unir.version(), f"({unir.engine_versions()})")

    # Trial period (30 days from the first run) or a license is required.
    status = license.status()
    if not status["can_scan"]:
        print(status["message"])
        return 3

    rc = 0
    # Unrelated images, every code in the frame. Narrow the search with
    # settings.symbologies, e.g. unir.parse_symbologies("dm,gs1").
    settings = unir.Settings.multi(0)
    with unir.Scanner(settings) as scanner:
        for path in paths:
            # Loading and scanning under one frame deadline.
            result = scanner.scan_file(path)
            if result.load_status not in (None, unir.LoadStatus.OK):
                print(f"{path}: {result.load_status.name}")  # NOT_FOUND, UNREADABLE, TOO_LARGE
                rc = 1
                continue

            if result.ok:
                for code in result.codes:
                    # code.text is bytes: the GS1 separator stays as byte 0x1D.
                    text = code.text.decode("utf-8", errors="replace").replace("\x1d", "<GS>")
                    print(f"{path}: {code.symbology_name}: {text}  ({result.elapsed_ms:.1f} ms)")
            elif result.status == unir.Status.NOT_FOUND:
                print(f"{path}: no code in the frame")
                rc = 1
            else:
                print(f"{path}: code found but could not be read")
                rc = 1
    return rc


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1:]))
