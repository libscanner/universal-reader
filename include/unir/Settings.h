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

    /// Scout network. With it, it first finds the codes (class and region,
    /// 15–25 ms), the engines read the crops, and the full search follows
    /// netFallback; without it, the engines go in their own order over the
    /// whole frame. The model is built into the library (seg16a); netModel —
    /// your own model from a file. Auto (default, unir 1.2.9) — by the task,
    /// see netEnabled(): no symbologies chosen (all are searched) — the
    /// network for any number of codes; chosen — only for several codes
    /// (maxCodes ≠ 1) and if DataMatrix or QR is among them. Barcodes and a
    /// single code of a known kind are faster without the network at the same
    /// reading; two-dimensional codes gain a lot from it. On — always, Off —
    /// no network.
    enum class NetMode { Auto, On, Off };
    NetMode netMode = NetMode::Auto;
    /// Your own network model (.unet) instead of the built-in one; empty — the
    /// built-in one. A path given with Auto always turns the network on (as
    /// before unir 1.2.9).
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

/// Whether the scout network is on with these settings (Settings::netMode).
inline bool netEnabled(const Settings& s) {
    if (s.netMode != Settings::NetMode::Auto) return s.netMode == Settings::NetMode::On;
    if (!s.netModel.empty()) return true;
    if ((s.symbologies & kAllSymbologies) == kAllSymbologies) return true;   // kind of code not given
    return s.maxCodes != 1 && (has(s.symbologies, Symbology::DataMatrix) || has(s.symbologies, Symbology::Qr));
}

} // namespace unir
