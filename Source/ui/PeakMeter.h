#pragma once
#include "../Theme.h"
#include <atomic>
#include <cmath>
class PeakMeter:public juce::Component,private juce::Timer{
public:
 PeakMeter(const juce::String&title,std::atomic<float>&left,std::atomic<float>&right,bool reversed=false):label(title),l(left),r(right),rightScale(reversed){
  // VBlank is preferred, but some FL Studio / host configurations don't
  // deliver it. This UI-only timer keeps both meters alive in that case.
  startTimerHz(30);
 }
 void setHoldMilliseconds(int millis) noexcept {holdMs=juce::jmax(1,millis);}
 void tick() noexcept {
  const double now=juce::Time::getMillisecondCounterHiRes();
  const float step=(float)(lastTickMs>0. ? juce::jlimit(0.0,100.0,now-lastTickMs)/17.0 : 1.0);
  lastTickMs=now;
  const float values[]={l.load(),r.load()};
  for(int i=0;i<2;++i){
   const float v=std::isfinite(values[i])?juce::jlimit(-100.f,12.f,values[i]):-100.f;
   shown[i]=v>shown[i]?v:juce::jmax(-100.f,shown[i]-.48f*step);
   if(v>=held[i]){held[i]=v;heldUntilMs[i]=now+(double)holdMs;}
   else if(now>=heldUntilMs[i])held[i]=juce::jmax(shown[i],held[i]-.8f*step);
  }
  repaint();
 }
 void paint(juce::Graphics&g)override {
  const auto value=[&](float d){return juce::jlimit(0.f,1.f,(d+60.f)/60.f);};
  const int barH=getHeight()-35-headerH;
  g.setColour(Theme::panelRaised);g.fillRoundedRectangle(21.f,(float)headerH,44.f,(float)barH+1.f,2.f);
  g.setColour(Theme::bone.withAlpha(.90f));g.setFont(Theme::mono(11));
  g.drawFittedText(label,21,0,44,headerH,juce::Justification::centred,1);
  for(int c=0;c<2;++c){const float d=shown[c],v=value(d);
    const int barX=22+c*27,barTop=headerH+1;
    // A partial last step previously drew two pixels above the LED area.
    for(int y=0;y+2<barH;y+=3){const float h=(float)y/(float)barH;
      // Reference-inspired LED palette: pale cyan low, lavender middle, pink high.
      const auto cyan=juce::Colour(0xff86efff),purple=juce::Colour(0xffc49ae9),pink=juce::Colour(0xffff87a8);
      const auto colour=(h<.53f?cyan.interpolatedWith(purple,h/.53f):purple.interpolatedWith(pink,(h-.53f)/.47f)).interpolatedWith(Theme::bone,.06f);
      g.setColour(h<=v?colour:colour.withAlpha(.38f)); // unlit segments stay faintly visible
      g.fillRect(barX,barTop+barH-y-3,15,2);
    }
    const float heldPos=(float)barTop+(float)barH*(1.f-value(held[c]));
    if(held[c]>-85.f){g.setColour(Theme::greenLight);g.fillRect((float)barX,heldPos,15.f,1.5f);}
  }
  for(int d:{0,-6,-12,-24,-48}){const float f=value((float)d);const int y=juce::jlimit(headerH+6,getHeight()-33,(int)((float)(headerH+1)+(float)barH*(1-f)));
   g.setFont(Theme::mono(10));g.setColour(Theme::faint.brighter(.64f));
   if(rightScale)g.drawText(juce::String(d),68,y-6,16,13,juce::Justification::centredLeft);
   else g.drawText(juce::String(d),0,y-6,20,13,juce::Justification::centredRight);
   g.drawLine(rightScale?64.f:18.f,(float)y,rightScale?68.f:22.f,(float)y,1.f);
  }
  g.setColour(Theme::panel);g.fillRoundedRectangle(9.f,(float)getHeight()-22,64.f,20.f,3.f);
  g.setColour(Theme::border);g.drawRoundedRectangle(9.f,(float)getHeight()-22,64.f,20.f,3.f,1.f);
  g.setColour(Theme::bone);g.setFont(Theme::mono(12));
  const float peak=juce::jmax(held[0],held[1]);
  g.drawFittedText(peak<=-89.f?juce::String("-INF"):juce::String(peak,1),
                   9,getHeight()-21,64,18,juce::Justification::centred,1);
 }
private:
 void timerCallback() override {
  if(juce::Time::getMillisecondCounterHiRes()-lastTickMs>75.0)tick();
 }
 static constexpr int headerH=13;
 juce::String label;std::atomic<float>&l,&r;bool rightScale;
 float shown[2]{-100.f,-100.f},held[2]{-100.f,-100.f};
 double heldUntilMs[2]{},lastTickMs=0.;
 int holdMs=1000;
};
