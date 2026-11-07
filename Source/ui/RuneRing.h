#pragma once
#include "../Theme.h"
#include "BinaryData.h"
#include <array>
#include <cmath>
class RuneRing {
public:
 RuneRing(){
  atlas=juce::ImageCache::getFromMemory(BinaryData::rune_atlas_png,BinaryData::rune_atlas_pngSize);
  juce::Random random(142857);for(auto& p:placements){p.angle=random.nextFloat()*juce::MathConstants<float>::twoPi;
      p.radius=151.f+random.nextFloat()*55.f;p.index=random.nextInt(40);p.size=4.0f+random.nextFloat()*8.5f;}
  for(int j=0;j<3;++j){cached[j]=juce::Image(juce::Image::ARGB,440,440,true);juce::Graphics g(cached[j]);
     for(int i=0;i<400;++i){const auto&p=placements[(size_t)i];
       if(i%3!=j)continue;
       const float x=220+std::cos(p.angle)*p.radius-p.size/2,y=220+std::sin(p.angle)*p.radius-p.size/2;
       juce::Rectangle<int>src((p.index%8)*32,(p.index/8)*32,32,32);
       g.setOpacity(juce::jmap((float)(i%9),0.f,8.f,.18f,.65f));
       g.setColour(Theme::spectral((float)j/2.f));
       g.drawImage(atlas,(int)x,(int)y,(int)p.size,(int)p.size,src.getX(),src.getY(),32,32,true);
     }
   }
 }
 void paint(juce::Graphics&g,float amount,float live,int tick) const {
   juce::Graphics::ScopedSaveState state(g);
   g.setOpacity(.14f+.16f*amount);g.drawImageAt(cached[0],0,0);
   g.setOpacity(.11f+.36f*amount);g.drawImageAt(cached[1],0,0);
   g.setOpacity(.10f+.58f*amount+.1f*live);g.drawImageAt(cached[2],0,0);
   for(int i=0;i<9;++i){auto&p=placements[(size_t)(i*37)];const float a=p.angle+std::sin(tick*.004f+i)*.018f;
    const float x=220+std::cos(a)*p.radius,y=220+std::sin(a)*p.radius;
    g.setColour(Theme::spectral((i%6)/6.f).withAlpha(.13f+live*.25f));g.fillEllipse(x-1.7f,y-1.7f,3.4f,3.4f);
   }
 }
private:
 struct Placement {float angle=0,radius=0,size=0;int index=0;};
 std::array<Placement,400>placements{};juce::Image atlas;std::array<juce::Image,3>cached;
};
