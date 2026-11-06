#pragma once

#include <string>

#include "Image.h"
#include "Symbology.h"

namespace unir {

struct Settings {
    /// A frame stream from one camera: the DataMatrix engine keeps a symbol
    /// size hint between frames. This is how the dmr and unir tools work.
    static Settings stream();
    /// Frames are unrelated; DataMatrix tries all standard sizes if the size
    /// could not be measured.
    static Settings single();
    /// Up to maxCodes codes per frame (0 — all).
    static Settings multi(int maxCodes = 0);

    /// Which symbologies to look for.
    Symbologies symbologies = kAllSymbologies;

    /// 1 — stop at the first code read (any symbology), N — up to N, 0 — all.
    int maxCodes = 1;

    /// Time limit per frame, ms — ONE for all symbologies: the engines share
    /// it in turn. 0 — no limit (timeoutMs applies).
    double budgetMs = 0.0;

    /// Safety cut-off, ms: the same interruption as budgetMs, but it does not
    /// change how the engines work. 0 — the 60 s ceiling.
    double timeoutMs = 10000.0;

    ImageLimits imageLimits{};

    /// DataMatrix mode: stream (size hint between frames) or single frames.
    enum class Mode { Stream, Single, Multi };
    Mode mode = Mode::Stream;

    /// Scout network (optional): a .unet model file. Empty — no network, the
    /// engines go in their own order over the whole frame. With the network it
    /// first finds the codes (class and region, 15–25 ms), the engines read the
    /// crops, and the full search follows netFallback.
    std::string netModel;
    /// Network confidence below which a detection is ignored.
    float netConf = 0.35f;
    /// Full search after the crops. Always — always (the crops only speed up
    /// the first read); Unread — only for classes the network found but did not
    /// read, and for a frame where it found nothing: symbologies the network
    /// does not see are not searched at all. QR and DataMatrix over the whole
    /// frame — only if the crops read no two-dimensional code. In single-code
    /// mode — always, if the crops gave nothing.
    enum class NetFallback { Always, Unread };
    NetFallback netFallback = NetFallback::Unread;
    /// Threads per frame for the network crops in multi-code mode (the crops
    /// are independent): 1 — one by one (default: an embedder with its own
    /// frame pool keeps it), N — up to N, 0 — one per core. The answer is the
    /// same with any number.
    int threads = 1;
};

} // namespace unir
