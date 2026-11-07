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
  const auto r=b.getLocalBounds().toFloat().reduced(1.0f);
  const bool active=b.getToggleState();
  const bool isBypass=b.getButtonText().equalsIgnoreCase("BYPASS");
  const auto accent=isBypass?juce::Colour(0xffefb271):Theme::green;
  g.setColour(active?(isBypass?juce::Colour(0xff35271b):Theme::greenDim.brighter(.12f))
                    :Theme::panelRaised);
  g.fillRoundedRectangle(r,5.0f);
  if(hover||active) {
   g.setColour(accent.withAlpha(active?.10f:.035f));
   g.fillRoundedRectangle(r.reduced(2.0f),4.0f);
  }
  g.setColour(active?accent:(hover?Theme::greenLight:Theme::border.brighter(.22f)));
  g.drawRoundedRectangle(r,5.0f,active||hover?1.5f:1.0f);
  if(down){g.setColour(accent.withAlpha(.27f));g.drawRoundedRectangle(r.reduced(2.0f),4.0f,1.0f);}
 }
 void drawButtonText(juce::Graphics&g,juce::TextButton&b,bool hover,bool)override{
  const bool bypass=b.getToggleState()&&b.getButtonText().equalsIgnoreCase("BYPASS");
  g.setColour(bypass?juce::Colour(0xffffd8ae):(hover?Theme::greenLight:Theme::bone));
  g.setFont(Theme::mono(12));
  g.drawFittedText(b.getButtonText(),b.getLocalBounds().reduced(3,1),juce::Justification::centred,1);
 }
 void drawComboBox(juce::Graphics&g,int w,int h,bool,int,int,int,int,juce::ComboBox&)override{
  g.setColour(Theme::panel);g.fillRoundedRectangle(0,0,(float)w,(float)h,4);
  g.setColour(Theme::border);g.drawRoundedRectangle(.5f,.5f,(float)w-1,(float)h-1,4,1.f);
  g.setColour(Theme::green);juce::Path arrow;arrow.addTriangle(w-15.f,h*.42f,w-6.f,h*.42f,w-10.5f,h*.66f);g.fillPath(arrow);
 }
};
