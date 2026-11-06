#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Geometry.h"
#include "Image.h"
#include "Settings.h"
#include "Symbology.h"

namespace unir {

/// The same numbers as in dmr: the exit codes of the tools match.
enum class Status {
    Ok            = 0,  ///< at least one code read
    NotFound      = 1,  ///< no symbol found
    NotDecoded    = 2,  ///< a symbol was found but not read
    Unlicensed    = 3,  ///< no unir license and the trial is over; the frame was not processed (unir/License.h)
    ImageTooLarge = 4,  ///< the frame exceeds Settings::imageLimits, not processed
    InvalidImage  = 5,  ///< the frame description is inconsistent (row length, format)
    Failed        = 6,  ///< processing stopped on an internal error; the process is alive
};

struct Code {
    Symbology symbology = Symbology::None;

    /// Symbol data. A leading FNC1 (the GS1 flag) is not returned; an FNC1
    /// inside the data is the GS separator (0x1D).
    std::string text;

    /// Box and corners — in pixels of the original frame.
    Box  box;
    Quad corners;

    /// DataMatrix and QR: modules per side (0 — unknown).
    int moduleCount = 0;
    /// Linear symbologies and QR: module width, pixels (0 — unknown).
    float modulePx = 0.0f;
};

struct ScanResult {
    Status status = Status::NotFound;
    /// In reading order. One code: SSCC and EAN/UPC, DataMatrix, other
    /// Code 128; several: linear symbologies, then DataMatrix.
    std::vector<Code> codes;
    /// At least one engine was cut off by the frame deadline or did not get to start.
    bool timedOut = false;
    double elapsedMs = 0.0;

    bool ok() const { return status == Status::Ok; }
    /// Text of the first code; empty if there are no codes.
    const std::string& text() const;
};

class Scanner {
public:
    explicit Scanner(Settings s = Settings::stream());
    ~Scanner();
    Scanner(Scanner&&) noexcept;
    Scanner& operator=(Scanner&&) noexcept;
    Scanner(const Scanner&)            = delete;
    Scanner& operator=(const Scanner&) = delete;

    /// Process a frame. Not thread-safe on one Scanner in Stream mode (the
    /// DataMatrix size hint changes from frame to frame); use one Scanner per
    /// thread.
    ScanResult scan(ImageView frame) const;

    /// Load an image from disk and process it under ONE frame deadline
    /// (loading counts against it). load — the outcome of reading the file
    /// (may be nullptr).
    ScanResult scanFile(const std::string& path, LoadStatus* load = nullptr) const;

    const Settings& settings() const;

    /// The scout network is loaded (Settings::netModel is set and the file was
    /// read).
    bool netReady() const;
    /// Why the network did not load; empty — it loaded or is not set.
    const std::string& netError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> p_;
};

/// One-off processing of a frame.
ScanResult scan(ImageView frame, const Settings& s = Settings::single());

} // namespace unir
