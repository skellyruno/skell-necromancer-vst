#include "../Source/ui/UiFramePacer.h"
#include <iostream>
int main() {
 using skell::necromancer::ui::FramePacer;
 for(int hz:{30,60,75,90,120,144}) {
  FramePacer pacer;
  int paints=0;
  for(int i=0;i<2*hz;++i)
    if(pacer.shouldRender(1.0/hz,false))++paints;
  std::cout<<hz<<" Hz display -> "<<paints<<" frames / 2s\n";
  if(paints<58||paints>62)return 1;
 }
 FramePacer fallback;
 for(int i=0;i<30;++i)if(!fallback.shouldRender(1.0/30.0,true))return 2;
 FramePacer stall;
 if(!stall.shouldRender(1.0,true))return 3;
 for(int i=0;i<60;++i)stall.shouldRender(1.0/120.0,false);
 std::cout<<"PASS: stable 30 FPS at every tested VBlank rate; fallback always advances\n";
 return 0;
}
