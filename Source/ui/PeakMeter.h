#pragma once
#include "../Theme.h"
#include <atomic>
class PeakMeter:public juce::Component{
public:
 PeakMeter(std::atomic<float>&left,std::atomic<float>&right,bool reversed=false):l(left),r(right),rightScale(reversed){}
 void setHoldMilliseconds(int millis) noexcept {holdFrames=juce::jmax(1,millis/17);}
 void tick() noexcept {
    const float values[]={l.load(),r.load()};
    for(int i=0;i<2;++i){const float v=values[i];
      shown[i]=v>shown[i]?v:juce::jmax(-100.f,shown[i]-.48f);
      if(v>=held[i]){held[i]=v;counter[i]=holdFrames;}
      else if(counter[i]>0)--counter[i];
      else held[i]=juce::jmax(shown[i],held[i]-.8f);
    }
    repaint();
 }
 void paint(juce::Graphics&g)override {
  const auto value=[&](float d){return juce::jlimit(0.f,1.f,(d+60.f)/60.f);};
  g.setColour(Theme::panelRaised);g.fillRoundedRectangle(21.f,0.f,33.f,(float)getHeight()-24,3.f);
  for(int c=0;c<2;++c){const float d=shown[c],v=value(d);
    const int barX=22+c*17,barTop=1,barH=getHeight()-25;
    for(int y=0;y<barH;y+=3){float h=(float)y/barH;if(h>v)continue;
      g.setColour(Theme::spectral(1.f-h).interpolatedWith(Theme::bone,.12f));
      g.fillRect(barX,barTop+barH-y-3,13,2);
    }
    g.setColour(Theme::bone.withAlpha(.75f));g.drawRect(barX,barTop,13,barH,1);
    const float heldPos=(float)barH*(1.f-value(held[c]));
    g.setColour(Theme::greenLight);g.fillRect((float)barX,heldPos,13.f,1.5f);
  }
  for(int d:{0,-6,-12,-24,-48}){const float f=value((float)d);int y=(int)((getHeight()-25)*(1-f));
   g.setFont(Theme::mono(8));g.setColour(Theme::faint.brighter(.4f));
   if(rightScale)g.drawText(juce::String(d),55,y-6,21,13,juce::Justification::centredLeft);
   else g.drawText(juce::String(d),0,y-6,20,13,juce::Justification::centredRight);
   g.drawLine(rightScale?53.f:18.f,(float)y,rightScale?57.f:22.f,(float)y,1.f);
  }
  g.setColour(Theme::panel);g.fillRoundedRectangle(9.f,(float)getHeight()-22,64.f,20.f,3.f);
  g.setColour(Theme::border);g.drawRoundedRectangle(9.f,(float)getHeight()-22,64.f,20.f,3.f,1.f);
  g.setColour(Theme::bone);g.setFont(Theme::mono(11));
  g.drawFittedText(juce::String(juce::jmax(held[0],held[1]),1),9,getHeight()-21,64,18,juce::Justification::centred,1);
 }
private:std::atomic<float>&l,&r;bool rightScale;
 float shown[2]{-100.f,-100.f},held[2]{-100.f,-100.f};int counter[2]{},holdFrames=60;
};
