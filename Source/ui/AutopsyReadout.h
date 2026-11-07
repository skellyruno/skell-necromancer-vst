#pragma once
#include "../PluginProcessor.h"
class AutopsyReadout:public juce::Component {
public:explicit AutopsyReadout(SkellNecromancerAudioProcessor&p):processor(p){}
 void paint(juce::Graphics&g)override{
  auto b=getLocalBounds().toFloat().reduced(1);
  g.setColour(Theme::black);g.fillRoundedRectangle(b,4);g.setColour(Theme::border);g.drawRoundedRectangle(b,4,1);
  const auto msg=processor.autopsyString();const bool has=msg!="AUTOPSY: standby";
  for(int i=0;i<3;++i){g.setColour(has?(i==0?Theme::green:Theme::greenDim):Theme::faint.darker(.6f));g.fillEllipse(12.f+i*16,5.f,6,6);}
  g.setFont(Theme::mono(9));g.setColour(has?Theme::greenLight:Theme::faint);
  g.drawFittedText(msg,8,13,getWidth()-16,15,juce::Justification::centredLeft,1);
 }
private:SkellNecromancerAudioProcessor&processor;
};
