#pragma once

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
};

} // namespace unir
