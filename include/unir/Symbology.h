#pragma once

// Symbologies read by unir. Each one is a separate bit: Settings::symbologies
// is a mask of them, Code::symbology is exactly one bit.

#include <cstdint>
#include <string>

namespace unir {

enum class Symbology : uint32_t {
    None       = 0,
    DataMatrix = 1u << 0,  ///< ECC 200, ISO/IEC 16022
    /// Code 128 without the GS1 flag. Acceptance is stricter than for SSCC:
    /// at least two scanlines must agree.
    Code128    = 1u << 1,
    /// GS1-128: Code 128 starting with FNC1. Without Code128 in the mask only
    /// SSCC (AI 00) is read; with Code128, other GS1-128 data as well.
    Gs1_128    = 1u << 2,
    /// EAN/UPC: accepted when two scanlines agree and the quiet zones are
    /// present. UPC-A is 12 digits (EAN-13 with a leading 0), UPC-E is 8
    /// digits (number system, 6 digits, check digit), not expanded to UPC-A.
    Ean13      = 1u << 3,
    Ean8       = 1u << 4,
    UpcA       = 1u << 5,
    UpcE       = 1u << 6,
    /// QR model 2, versions 1–40 (the qrr engine). Text is UTF-8 (ECI, kanji
    /// from Shift-JIS); for GS1 QR the leading FNC1 is not passed, an FNC1
    /// inside is the Gs separator (0x1D), as with DataMatrix.
    Qr         = 1u << 7,
};

/// Mask of symbologies.
using Symbologies = uint32_t;

constexpr Symbologies operator|(Symbology a, Symbology b) {
    return static_cast<Symbologies>(a) | static_cast<Symbologies>(b);
}
constexpr Symbologies operator|(Symbologies a, Symbology b) { return a | static_cast<Symbologies>(b); }
constexpr bool has(Symbologies mask, Symbology s) { return (mask & static_cast<Symbologies>(s)) != 0; }

/// EAN-13, EAN-8, UPC-A and UPC-E: the engine reads them together, but only
/// those present in the mask are returned.
constexpr Symbologies kEanUpc = Symbology::Ean13 | Symbology::Ean8 | Symbology::UpcA | Symbology::UpcE;

/// All symbologies of this build.
constexpr Symbologies kAllSymbologies =
    Symbology::DataMatrix | Symbology::Code128 | Symbology::Gs1_128 | kEanUpc | static_cast<Symbologies>(Symbology::Qr);

/// Short name: "datamatrix", "code128", "gs1-128", "ean-13", "ean-8",
/// "upc-a", "upc-e", "qr"; None — "none".
const char* nameOf(Symbology s);

/// AIM symbology identifier (ISO/IEC 15424) without the data modifier:
/// "]d", "]C", "]E", "]Q".
const char* aimPrefixOf(Symbology s);

/// Mask from a comma-separated list of names: "dm,128,gs1-128", "all".
/// Accepts datamatrix|dm, code128|128, gs1-128|gs1|sscc, ean-13|ean13,
/// ean-8|ean8, upc-a|upca, upc-e|upce, ean — all four EAN/UPC, and qr. An unknown
/// name returns false and leaves mask unchanged.
bool parseSymbologies(const std::string& list, Symbologies& mask);

} // namespace unir
