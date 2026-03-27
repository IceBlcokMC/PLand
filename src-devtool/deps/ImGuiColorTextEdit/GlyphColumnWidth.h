#pragma once
#include "imgui.h"

inline int GetGlyphColumnWidth(ImWchar codepoint) {
    // Special markers reserved specifically for tabs
    if (codepoint == '\t') return -1;

    // The range of wide characters (full-width),
    // including Chinese, Japanese, and Korean characters, full-width punctuation, etc.
    if (codepoint >= 0x1100 &&
       (codepoint <= 0x115f ||
        codepoint == 0x2329 || codepoint == 0x232a ||
       (codepoint >= 0x2e80 && codepoint <= 0xa4cf && codepoint != 0x303f) ||
       (codepoint >= 0xac00 && codepoint <= 0xd7a3) ||
       (codepoint >= 0xf900 && codepoint <= 0xfaff) ||
       (codepoint >= 0xfe10 && codepoint <= 0xfe19) ||
       (codepoint >= 0xfe30 && codepoint <= 0xfe6f) ||
       (codepoint >= 0xff00 && codepoint <= 0xff60) ||
       (codepoint >= 0xffe0 && codepoint <= 0xffe6))) {
        return 2; // Chinese monospaced characters occupy 2 columns
       }
    return 1; // English characters occupy 1 column
}