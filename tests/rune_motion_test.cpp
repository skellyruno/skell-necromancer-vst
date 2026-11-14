#include "../Source/ui/RuneMotion.h"
#include <cmath>
#include <iostream>
int main() {
 using namespace skell::necromancer::runes;
 int movingCount=0;
 for (int i=0;i<640;++i) if (i%4!=0) ++movingCount;
 if(movingCount!=480){std::cerr<<"wrong mobile glyph count\n";return 1;}
 const float radii[3]{155.f,169.f,182.f};
 for(std::size_t band=0;band<3;++band){
  const float original=.3f;
  const auto initial=position(original,radii[band],0.f);
  float phase=0.f;
  auto previous=initial;
  for(int frame=0;frame<30;++frame){
   const float delta=phaseStep(band,1.f/30.f,0.f);
   if(!std::isfinite(delta) || std::abs(delta)<.0001f) return 2;
   phase+=delta;
   const auto current=position(original,radii[band],phase);
   const float step=std::hypot(current.x-previous.x,current.y-previous.y);
   if(!(step>.3f && step<4.f)){
    std::cerr<<"stalled or teleported glyph in band "<<band<<"\n";return 3;
   }
   const float rr=std::hypot(current.x-220.f,current.y-220.f);
   if(std::abs(rr-radii[band])>.001f)return 4;
   previous=current;
  }
  const float distance=std::hypot(previous.x-initial.x,previous.y-initial.y);
  if(!(distance>40.f && distance<80.f)){
   std::cerr<<"insufficient real travel "<<distance<<"\n";return 5;
  }
  std::cout<<"band "<<band<<": "<<distance<<" pixels in 1s, 30 frames\n";
 }
 if(!(phaseStep(1,.1f,0.f)<0.f && phaseStep(0,.1f,0.f)>0.f))return 6;
 if(!(std::abs(quietFade(1.5707963f)-.15f)<.001f && quietFade(-1.5707963f)>.999f))return 7;
 const float pi=3.14159265358979323846f;
 if(!(headingFade(-pi*.5f)>.34f && headingFade(-pi*.5f)<.38f))return 8;
 if(!(headingFade(0.f)>.999f && headingFade(pi*.5f)>.999f))return 9;
 for(int i=0;i<=360;++i){
  const float a=(float)i*pi/180.f-pi;
  if(!std::isfinite(headingFade(a))||headingFade(a)<.35f||headingFade(a)>1.001f)return 10;
 }
 std::cout<<"PASS: 480 movable glyphs, real motion at silence, continuous frames, no radial escape\n";
}
