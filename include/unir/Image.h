#pragma once

// A frame for unir. The frame view (ImageView) matches the views of the
// engines field for field: it is passed to them without copying pixels.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace unir {

enum class PixelFormat {
    Gray8,  ///< one channel: luminance; the cheapest for the scanner
    Bgr8,   ///< three channels, OpenCV order
    Rgb8,
    Bgra8,  ///< four channels, OpenCV and Windows order
    Rgba8,
};

constexpr int channelsOf(PixelFormat f) {
    switch (f) {
        case PixelFormat::Gray8:                          return 1;
        case PixelFormat::Bgr8:  case PixelFormat::Rgb8:  return 3;
        case PixelFormat::Bgra8: case PixelFormat::Rgba8: return 4;
    }
    return 0;
}

/// Someone else's pixel buffer, not owned. stride — bytes per row, 0 — rows
/// packed tightly.
struct ImageView {
    const uint8_t* data   = nullptr;
    int            width  = 0;
    int            height = 0;
    int            stride = 0;
    PixelFormat    format = PixelFormat::Gray8;

    bool empty()    const { return data == nullptr || width <= 0 || height <= 0; }
    int  channels() const { return channelsOf(format); }
    int  rowBytes() const { return stride > 0 ? stride : width * channels(); }
    const uint8_t* row(int y) const { return data + static_cast<std::ptrdiff_t>(y) * rowBytes(); }
};

/// A frame with its own buffer; rows packed tightly.
class Image {
public:
    Image() = default;
    Image(int width, int height, PixelFormat format) { reset(width, height, format); }

    void reset(int width, int height, PixelFormat format);
    void clear();

    int         width()  const { return w_; }
    int         height() const { return h_; }
    int         stride() const { return w_ * channelsOf(fmt_); }
    PixelFormat format() const { return fmt_; }
    bool        empty()  const { return buf_.empty(); }

    uint8_t*       data()       { return buf_.data(); }
    const uint8_t* data() const { return buf_.data(); }
    uint8_t*       row(int y)       { return buf_.data() + static_cast<std::ptrdiff_t>(y) * stride(); }
    const uint8_t* row(int y) const { return buf_.data() + static_cast<std::ptrdiff_t>(y) * stride(); }

    ImageView view() const { return ImageView{ buf_.data(), w_, h_, stride(), fmt_ }; }
    operator ImageView() const { return view(); }

private:
    std::vector<uint8_t> buf_;
    int         w_   = 0;
    int         h_   = 0;
    PixelFormat fmt_ = PixelFormat::Gray8;
};

/// Frame size limits — the same as in the engines.
struct ImageLimits {
    int       maxSide   = 16384;        ///< 0 — kHardMaxSide
    long long maxPixels = 100'000'000;  ///< 0 — kHardMaxPixels

    static constexpr int       kHardMaxSide   = 65535;
    static constexpr long long kHardMaxPixels = 1LL << 29;

    bool allows(long long w, long long h) const {
        if (w <= 0 || h <= 0) return false;
        const long long side = (maxSide > 0 && maxSide < kHardMaxSide) ? maxSide : kHardMaxSide;
        const long long area = (maxPixels > 0 && maxPixels < kHardMaxPixels) ? maxPixels : kHardMaxPixels;
        return w <= side && h <= side && w * h <= area;
    }
};

enum class LoadStatus {
    Ok,          ///< image read
    NotFound,    ///< no such file
    Unreadable,  ///< the file exists but is not a readable image (or did not finish in time)
    TooLarge,    ///< the size in the header exceeds ImageLimits; pixels were not read
};

enum class LoadAs {
    Color,  ///< PixelFormat::Bgr8
    Gray,   ///< PixelFormat::Gray8 — enough for the scanner
};

/// Image from disk: JPEG and PNG, with decompression-bomb protection and size
/// limits; other formats — LoadStatus::Unreadable. The path is UTF-8; on
/// Windows a string that is not valid UTF-8 is read in the system code page
/// (ANSI).
LoadStatus loadImage(const std::string& path, Image& out, LoadAs as = LoadAs::Gray,
                     const ImageLimits& limits = ImageLimits{});

} // namespace unir
