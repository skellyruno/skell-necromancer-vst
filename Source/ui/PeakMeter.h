#pragma once
#include "../Theme.h"
#include <atomic>
class PeakMeter:public juce::Component{
public:
 PeakMeter(const juce::String&title,std::atomic<float>&left,std::atomic<float>&right,bool reversed=false):label(title),l(left),r(right),rightScale(reversed){}
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
  g.setColour(Theme::panelRaised);g.fillRoundedRectangle(21.f,(float)headerH,34.f,(float)(getHeight()-24-headerH),3.f);
  g.setColour(Theme::bone);g.setFont(Theme::mono(12));
  g.drawFittedText(label,21,0,34,headerH,juce::Justification::centred,1);
  for(int c=0;c<2;++c){const float d=shown[c],v=value(d);
    const int barX=22+c*17,barTop=headerH+1,barH=getHeight()-25-headerH;
    for(int y=0;y<barH;y+=3){const float h=(float)y/(float)barH;
      // Reference-inspired LED palette: pale cyan low, lavender middle, pink high.
      const auto cyan=juce::Colour(0xff86efff),purple=juce::Colour(0xffc49ae9),pink=juce::Colour(0xffff87a8);
      const auto colour=(h<.53f?cyan.interpolatedWith(purple,h/.53f):purple.interpolatedWith(pink,(h-.53f)/.47f)).interpolatedWith(Theme::bone,.06f);
      g.setColour(h<=v?colour:colour.withAlpha(.28f)); // unlit segments stay faintly visible
      g.fillRect(barX,barTop+barH-y-3,15,2);
    }
    const float heldPos=(float)barTop+(float)barH*(1.f-value(held[c]));
    g.setColour(Theme::greenLight);g.fillRect((float)barX,heldPos,15.f,1.5f);
  }
  for(int d:{0,-6,-12,-24,-48}){const float f=value((float)d);const int y=juce::jlimit(headerH+6,getHeight()-33,(int)((float)(headerH+1)+(float)(getHeight()-25-headerH)*(1-f)));
   g.setFont(Theme::mono(9));g.setColour(Theme::faint.brighter(.4f));
   if(rightScale)g.drawText(juce::String(d),55,y-6,21,13,juce::Justification::centredLeft);
   else g.drawText(juce::String(d),0,y-6,20,13,juce::Justification::centredRight);
   g.drawLine(rightScale?53.f:18.f,(float)y,rightScale?57.f:22.f,(float)y,1.f);
  }
  g.setColour(Theme::panel);g.fillRoundedRectangle(9.f,(float)getHeight()-22,64.f,20.f,3.f);
  g.setColour(Theme::border);g.drawRoundedRectangle(9.f,(float)getHeight()-22,64.f,20.f,3.f,1.f);
  g.setColour(Theme::bone);g.setFont(Theme::mono(11));
  g.drawFittedText(juce::String(juce::jmax(held[0],held[1]),1),9,getHeight()-21,64,18,juce::Justification::centred,1);
 }
private:static constexpr int headerH=16;
 juce::String label;std::atomic<float>&l,&r;bool rightScale;
 float shown[2]{-100.f,-100.f},held[2]{-100.f,-100.f};int counter[2]{},holdFrames=60;
};
