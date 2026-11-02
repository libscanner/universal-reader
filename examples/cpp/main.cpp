// SPDX-License-Identifier: MIT-0
// Minimal unir example: read DataMatrix, Code 128, GS1-128 and EAN/UPC codes
// from an image file.
//
//   test_unir <image> [--all]
//
// --all looks for every code in the frame instead of the first one.

#include <unir/Unir.h>

#include <cstring>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <image> [--all]\n";
        return 2;
    }
    const bool all = argc > 2 && std::strcmp(argv[2], "--all") == 0;

    std::cout << "unir " << unir::version() << " (" << unir::engineVersions() << ")\n";

    // Trial period or license: without either, scan() returns Status::Unlicensed.
    const unir::LicenseStatus lic = unir::license::status();
    if (!lic.canScan()) {
        std::cerr << lic.message << "\n";
        return 3;
    }

    // Single images: Settings::single(); every code — Settings::multi(0).
    // For a camera stream keep one Scanner with the default Settings::stream().
    unir::Settings settings = all ? unir::Settings::multi(0) : unir::Settings::single();
    settings.symbologies = unir::kAllSymbologies;   // or e.g. Symbology::DataMatrix | Symbology::Gs1_128
    const unir::Scanner scanner(settings);

    // Loading counts against the same frame deadline as the scan.
    unir::LoadStatus load = unir::LoadStatus::Ok;
    const unir::ScanResult r = scanner.scanFile(argv[1], &load);

    if (load == unir::LoadStatus::NotFound)  { std::cerr << "no such file\n"; return 1; }
    if (load != unir::LoadStatus::Ok)        { std::cerr << "not a readable image\n"; return 1; }
    if (!r.ok()) {
        std::cout << (r.status == unir::Status::NotFound ? "no code in the frame\n"
                                                         : "code found but could not be read\n");
        return 1;
    }

    for (const unir::Code& c : r.codes) {
        // c.text holds raw bytes: the GS1 separator stays as byte 0x1D.
        std::cout << unir::nameOf(c.symbology) << " at (" << c.box.x << ", " << c.box.y
                  << "): " << c.text << "\n";
    }
    std::cout << r.elapsedMs << " ms\n";
    return 0;
}
