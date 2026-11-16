#pragma once
// Skell-Necromancer's editor-only 8 x 8 rune catalogue.
// 40 original glyphs + 24 freshly drawn Egyptian-inspired line marks.
// Index stability preserves the original symbols and deterministic placements.
namespace skell::necromancer::runes::glyphs {
inline constexpr int cellSize = 32;
inline constexpr int columns = 8;
inline constexpr int originalCount = 40;
inline constexpr int egyptianCount = 24;
inline constexpr int totalCount = originalCount + egyptianCount;
inline constexpr float egyptianFraction = float(egyptianCount) / float(totalCount);
constexpr bool isEgyptian(int index) noexcept {
 return index >= originalCount && index < totalCount;
}
constexpr int motif(int index) noexcept {
 return isEgyptian(index) ? index - originalCount : -1;
}
constexpr bool validIndex(int index) noexcept {
 return index >= 0 && index < totalCount;
}
static_assert(totalCount == columns * columns, "Expected eight rows of eight glyphs");
static_assert(egyptianCount * 8 == totalCount * 3, "Egyptian share must be 37.5%");
}
