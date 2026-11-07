#pragma once
#include "../Theme.h"
class NecroLookAndFeel final:public juce::LookAndFeel_V4 {
public:
 NecroLookAndFeel(){
  setColour(juce::ComboBox::backgroundColourId,Theme::panelRaised);
  setColour(juce::ComboBox::outlineColourId,Theme::border);
  setColour(juce::ComboBox::textColourId,Theme::bone);
  setColour(juce::PopupMenu::backgroundColourId,Theme::panel);
  setColour(juce::PopupMenu::textColourId,Theme::bone);
  setColour(juce::PopupMenu::highlightedBackgroundColourId,Theme::greenDim);
  setColour(juce::TextButton::textColourOffId,Theme::bone);
 }
 void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down)override{
  auto r=b.getLocalBounds().toFloat().reduced(1);
  g.setColour(down?Theme::greenDim:(b.getToggleState()?Theme::greenDim:Theme::panelRaised));g.fillRoundedRectangle(r,5);
  g.setColour(b.getToggleState()?Theme::green:(hover?Theme::greenLight:Theme::border));g.drawRoundedRectangle(r,5,hover?1.5f:1.f);
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool hover,bool)override{
  g.setColour(hover?Theme::greenLight:Theme::bone);g.setFont(Theme::mono(12));
  g.drawFittedText(b.getButtonText(),b.getLocalBounds(),juce::Justification::centred,1);
 }
 void drawComboBox(juce::Graphics&g,int w,int h,bool,int,int,int,int,juce::ComboBox&)override{
  g.setColour(Theme::panel);g.fillRoundedRectangle(0,0,(float)w,(float)h,4);
  g.setColour(Theme::border);g.drawRoundedRectangle(.5f,.5f,(float)w-1,(float)h-1,4,1.f);
  g.setColour(Theme::green);juce::Path arrow;arrow.addTriangle(w-15.f,h*.42f,w-6.f,h*.42f,w-10.5f,h*.66f);g.fillPath(arrow);
 }
};
