#pragma once
#include "RuneRing.h"
class ReanimateDial:public juce::Slider {
public:
 ReanimateDial() {setRange(0.0,100.0,0.1);setValue(65.0,juce::dontSendNotification);setSliderStyle(juce::Slider::RotaryVerticalDrag);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 setDoubleClickReturnValue(true,65);setPopupMenuEnabled(true);setPopupDisplayEnabled(true,false,nullptr);setMouseCursor(juce::MouseCursor::PointingHandCursor);
 well=juce::ImageCache::getFromMemory(BinaryData::dial_well_png,BinaryData::dial_well_pngSize);
 cap=juce::ImageCache::getFromMemory(BinaryData::knob_cap_big_png,BinaryData::knob_cap_big_pngSize);
 setTooltip("REANIMATE blends tape weight, gentle compression, harmonic life and restored transients. Double-click returns to 65%.");}
 void setLive(float v){live=juce::jlimit(0.f,1.f,v);}
 void advance(){++frame;repaint();}
 void paint(juce::Graphics&g)override {
  g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
  g.drawImage(well,getLocalBounds().toFloat());
  ring.paint(g,(float)getValue()*.01f,live,frame);
  auto cx=220.f,cy=220.f;const float p=(float)(getValue()/100.0);
  const float start=juce::MathConstants<float>::pi*.75f,span=juce::MathConstants<float>::pi*1.5f;
  for(int t=0;t<90;++t){float a=start+span*t/90.f;float x1=cx+std::cos(a)*139.f,y1=cy+std::sin(a)*139.f;
   const bool major=t%10==0;const float rad=major?148:144;
   g.setColour(t<p*90?Theme::bone.withAlpha(.87f):Theme::faint.withAlpha(.32f));
   g.drawLine(x1,y1,cx+std::cos(a)*rad,cy+std::sin(a)*rad,major?1.2f:.7f);
  }
  juce::Path path;for(int s=0;s<=180;++s){float a=start+span*(s/180.f)*p;
    auto x=cx+std::cos(a)*133.f,y=cy+std::sin(a)*133.f;if(s==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
  juce::ColourGradient gradient(Theme::violet,96,338,Theme::cyan,355,115,false);gradient.addColour(.57,Theme::green);
  g.setGradientFill(gradient);g.strokePath(path,juce::PathStrokeType(5.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
  const float angle=start+span*p;
  {juce::Graphics::ScopedSaveState save(g);
   g.addTransform(juce::AffineTransform::rotation(angle-start-1.18f,cx,cy));
   g.drawImage(cap,{cx-104.f,cy-104.f,208.f,208.f});}
  g.setColour(Theme::bone);g.drawLine(cx+std::cos(angle)*109.f,cy+std::sin(angle)*109.f,cx+std::cos(angle)*127.f,cy+std::sin(angle)*127.f,5.f);
  g.setColour(Theme::bone);g.setFont(Theme::display(13));g.drawFittedText("R E A N I M A T E",juce::Rectangle<int>(130,330,180,18),juce::Justification::centred,1);
  g.setFont(Theme::display(36));g.drawFittedText(juce::String((int)std::lround(getValue()))+"%",juce::Rectangle<int>(130,377,180,47),juce::Justification::centred,1);
  if(isMouseOverOrDragging()){g.setColour(Theme::green.withAlpha(.5f));g.drawEllipse(90,90,260,260,1.4f);}
 }
private:juce::Image well,cap;RuneRing ring;float live=0;int frame=0;
};
