#pragma once
#include "../Theme.h"
// Compact PUSH hardware control. GUI-only painter; existing Slider/APVTS
// automation, 0–24 dB gain and double-click reset remain unchanged.
class PushSlider:public juce::Slider{
public:
 PushSlider(){
  setSliderStyle(juce::Slider::LinearHorizontal);
  setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  setDoubleClickReturnValue(true,6);
  setPopupMenuEnabled(true);
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
  setTooltip("PUSH drives up to 24 dB of gain into the true-peak limiter.");
 }
 void paint(juce::Graphics&g)override{
  const auto b=getLocalBounds().toFloat();
  const float p=(float)valueToProportionOfLength(getValue());
  const float cy=b.getCentreY();
  const auto track=juce::Rectangle<float>(2.f,cy-2.5f,b.getWidth()-4.f,5.f);
  g.setColour(juce::Colour(0xff0b1114));g.fillRoundedRectangle(track,2.5f);
  g.setColour(juce::Colour(0xff586267).withAlpha(.68f));
  g.drawRoundedRectangle(track,2.5f,.75f);
  if(p>.005f){
   g.setColour(Theme::greenDim.withAlpha(.82f));
   g.fillRoundedRectangle(track.withWidth(track.getWidth()*p),2.5f);
  }
  // The reference hardware uses a pale steel thumb, not a neon-green bulb.
  const float cx=7.f+(b.getWidth()-14.f)*p;
  g.setColour(juce::Colours::black.withAlpha(.64f));
  g.fillEllipse(cx-6.7f,cy-6.7f,13.4f,13.4f);
  juce::ColourGradient metal(juce::Colour(0xfff0f1f0),cx-5.f,cy-5.f,
                             juce::Colour(0xff878f95),cx+5.f,cy+5.f,false);
  g.setGradientFill(metal);
  g.fillEllipse(cx-5.4f,cy-5.4f,10.8f,10.8f);
  g.setColour(juce::Colour(0xfff1f3f4).withAlpha(.55f));
  g.drawEllipse(cx-5.4f,cy-5.4f,10.8f,10.8f,.75f);
 }
};
