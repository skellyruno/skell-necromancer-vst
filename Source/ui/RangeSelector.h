#pragma once
#include "../Theme.h"
// MID band range as a vertical I / II / III radio column. It stays a ComboBox so the
// parameter attachment and host automation work unchanged; only painting and clicks differ.
class RangeSelector:public juce::ComboBox {
public:
 RangeSelector(){addItemList({"I","II","III"},1);setTooltip("Select body (I), upper mids (II), or presence (III).");setMouseCursor(juce::MouseCursor::PointingHandCursor);}
 void paint(juce::Graphics&g)override {
  const char*names[]={"I","II","III"};
  for(int i=0;i<3;++i){
   const bool on=getSelectedItemIndex()==i;
   const float y=(float)getHeight()*((float)i+.5f)/3.f;
   // Cool blue selected lamp matches the small illuminated range indicator in
   // the reference; it does not alter the existing automated ComboBox choice.
   const auto lamp=juce::Colour(0xff83b8ff);
   if(on){g.setColour(lamp.withAlpha(.19f));g.fillEllipse(1.f,y-7.f,14.f,14.f);}
   g.setColour(on?lamp:Theme::black);g.fillEllipse(4.f,y-4.f,8.f,8.f);
   g.setColour(on?lamp.brighter(.2f):Theme::border);g.drawEllipse(4.f,y-4.f,8.f,8.f,1.f);
   g.setColour(on?Theme::bone:Theme::faint);g.setFont(Theme::mono(12).withExtraKerningFactor(.12f));
   g.drawText(names[i],20,(int)y-8,getWidth()-20,16,juce::Justification::centredLeft);
  }
 }
 void mouseDown(const juce::MouseEvent&e)override{setSelectedItemIndex(juce::jlimit(0,2,e.y*3/juce::jmax(1,getHeight())),juce::sendNotificationSync);}
 void mouseUp(const juce::MouseEvent&)override{}
 void mouseDrag(const juce::MouseEvent&)override{}
 void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&)override{}
};
