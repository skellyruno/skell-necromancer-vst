#pragma once
#include "../Theme.h"
#include "BinaryData.h"
class MetalKnob:public juce::Slider{
public:
 explicit MetalKnob(juce::String text):caption(std::move(text)) {
  setSliderStyle(juce::Slider::RotaryVerticalDrag);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
  setDoubleClickReturnValue(true,0.0);setPopupMenuEnabled(true);setPopupDisplayEnabled(true,false,nullptr);
  small=juce::ImageCache::getFromMemory(BinaryData::knob_cap_small_png,BinaryData::knob_cap_small_pngSize);
 }
 void setDefault(double d){setDoubleClickReturnValue(true,d);}
 void paint(juce::Graphics&g)override{
  const auto b=getLocalBounds().toFloat();
  const float cx=b.getCentreX();
  const float radius=std::min(b.getWidth(),b.getHeight()-31.f)*.39f;
  const float cy=radius+7, r=radius;
  g.setColour(Theme::greenDim.withAlpha(.32f));g.fillEllipse(cx-r-7,cy-r-7,2*(r+7),2*(r+7));
  for(int k=0;k<=25;++k){float angle=juce::MathConstants<float>::pi*(.75f+1.5f*k/25.f);
    auto rr=r+5;auto xx=cx+std::cos(angle)*rr,yy=cy+std::sin(angle)*rr;
    g.setColour(k<(int)(25*(float)valueToProportionOfLength(getValue()))?Theme::green:Theme::faint.withAlpha(.5f));
    g.drawLine(xx,yy,cx+std::cos(angle)*(rr+3.f),cy+std::sin(angle)*(rr+3.f),1.f);
  }
  if(!small.isNull()){juce::Graphics::ScopedSaveState save(g);const float a=(float)(valueToProportionOfLength(getValue())*4.712389-2.35619);
      g.addTransform(juce::AffineTransform::rotation(a,cx,cy));g.drawImage(small,juce::Rectangle<float>(cx-r,cy-r,r*2,r*2));}
  else{g.setColour(Theme::border);g.fillEllipse(cx-r,cy-r,2*r,2*r);}
  if(isMouseOverOrDragging()){g.setColour(Theme::green.withAlpha(.8f));g.drawEllipse(cx-r-2,cy-r-2,2*r+4,2*r+4,1.f);}
  g.setFont(Theme::mono(14));g.setColour(Theme::bone);
  g.drawFittedText(juce::String(getValue(),1)+(caption=="MIX"||caption=="WIDTH"?" %":" dB"),
     juce::Rectangle<int>(0,(int)(cy+r+9),getWidth(),18),juce::Justification::centred,1);
  g.setFont(Theme::mono(10));g.setColour(Theme::faint.brighter(.7f));
  g.drawFittedText(caption,juce::Rectangle<int>(0,(int)(cy+r+26),getWidth(),18),juce::Justification::centred,1);
 }
private:juce::String caption;juce::Image small;
};
