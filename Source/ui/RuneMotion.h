#pragma once
// JUCE-independent orbit geometry for the Necromancer animated rune field.
// No allocations and no audio-thread usage; called from the editor's UI timer.
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace skell::necromancer::runes {
struct Position { float x, y; };
inline float phaseStep(std::size_t band, float dt, float live) noexcept {
 constexpr float speeds[3]{.48f,-.37f,.29f};
 return speeds[band % 3] * std::clamp(dt,0.f,.1f) * (.9f+.3f*std::clamp(live,0.f,1.f));
}
inline Position position(float originalAngle, float radius, float phase) noexcept {
 const float angle=originalAngle+phase;
 return {220.f+std::cos(angle)*radius,220.f+std::sin(angle)*radius};
}
inline float quietFade(float angle) noexcept {
 const float s=std::clamp((std::sin(angle)-.45f)/.5f,0.f,1.f);
 return 1.f-.85f*s*s*(3.f-2.f*s);
}
}
