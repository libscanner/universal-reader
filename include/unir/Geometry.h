#pragma once

// Code geometry in the frame — in pixels of the original frame.

namespace unir {

struct Point {
    float x = 0.0f;
    float y = 0.0f;
};

/// Rectangle aligned with the frame axes.
struct Box {
    int x = 0, y = 0, width = 0, height = 0;
    bool empty() const { return width <= 0 || height <= 0; }
};

/// The four corners of the symbol, in order around it. The meaning of the
/// indices depends on the symbology:
/// DataMatrix — corner[3] is the one the detector took for the corner of the
/// solid L-shaped finder; good for drawing, not for conclusions about content
/// rotation.
/// Code 128 — corner[0] is on the start side, corner[1] on the stop side along
/// the same edge of the bars, corner[2] and corner[3] on the opposite edge.
struct Quad {
    Point corner[4];
};

} // namespace unir
