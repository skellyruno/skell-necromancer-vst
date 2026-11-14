#pragma once
// UI-only ~30 Hz display scheduler. Unlike a fixed every-other-VBlank toggle,
// this maintains a stable display rate on 30/60/75/90/120/144 Hz hosts.
#include <algorithm>
namespace skell::necromancer::ui {
class FramePacer {
public:
 bool shouldRender(double elapsedSeconds,bool forced=false) noexcept {
  constexpr double frameSeconds=1.0/30.0;
  if(forced){budget=0.0;return true;}
  budget+=std::clamp(elapsedSeconds,0.0,0.1);
  if(budget+1e-9<frameSeconds)return false;
  // Keep fractional remainder for 75/144 Hz hosts; cap accumulated lag after
  // an app stall rather than trying to catch up with many fast UI frames.
  budget=std::clamp(budget-frameSeconds,0.0,frameSeconds*.95);
  return true;
 }
private:
 double budget=1.0/30.0; // first paint is immediate
};
}
