#pragma once
#include "../Theme.h"
class HintOverlay:public juce::Component{
public:
 HintOverlay(){setInterceptsMouseClicks(false,false);setVisible(false);}
 void showTour(int n){index=n;setVisible(true);repaint();}
 void paint(juce::Graphics&g)override{
  if(index<0)return;
  const juce::String tips[]={"1/5  REANIMATE: set restoration intensity.","2/5  TONE: correct broad spectral balance.","3/5  LIMITER: set ceiling, then PUSH.","4/5  AUTO: analyze the loudest section.","5/5  A/B: compare with level matching."};
  g.setColour(Theme::black.withAlpha(.86f));g.fillRoundedRectangle(370.f,557.f,405.f,22.f,5.f);
  g.setColour(Theme::green);g.drawRoundedRectangle(370.f,557.f,405.f,22.f,5.f,1.0f);
  g.setColour(Theme::bone);g.setFont(Theme::mono(11));g.drawFittedText(tips[juce::jlimit(0,4,index)],380,559,386,17,juce::Justification::centredLeft,1);
 }
 int index=-1;
};
