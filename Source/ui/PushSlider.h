#pragma once
#include "../Theme.h"
class PushSlider:public juce::Slider{
public:PushSlider(){setSliderStyle(juce::Slider::LinearHorizontal);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 setDoubleClickReturnValue(true,6);setPopupMenuEnabled(true);setMouseCursor(juce::MouseCursor::PointingHandCursor);setTooltip("PUSH drives up to 24 dB of gain into the true-peak limiter.");}
 void paint(juce::Graphics&g)override{
  const auto b=getLocalBounds().toFloat();
  const float p=(float)valueToProportionOfLength(getValue());
  const float y=b.getCentreY();
  g.setColour(Theme::panel);g.fillRoundedRectangle(2,y-3,b.getWidth()-4,6,3);
  g.setColour(Theme::greenDim);g.fillRoundedRectangle(2,y-3,(b.getWidth()-4)*p,6,3);
  g.setColour(Theme::green);g.fillEllipse(2+(b.getWidth()-15)*p,y-7,14,14);
 }
};
