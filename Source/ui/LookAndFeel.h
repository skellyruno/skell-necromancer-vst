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
  const auto text=b.getButtonText();
  const bool bypass=b.getToggleState()&&text.equalsIgnoreCase("BYPASS");
  const auto colour=bypass?juce::Colour(0xffffd8ae):(hover?Theme::greenLight:Theme::bone);
  g.setColour(colour);
  if(drawIcon(g,text,b.getLocalBounds().toFloat().withSizeKeepingCentre(15.f,15.f)))return;
  const bool power=text=="AUTO"||text=="ON"||text=="BYPASS"||text=="LIMITER";
  const auto font=Theme::mono(12).withExtraKerningFactor(power?.1f:0.f);
  g.setFont(font);
  auto area=b.getLocalBounds().reduced(3,1);
  if(power){
   juce::GlyphArrangement glyphs;glyphs.addLineOfText(font,text,0.f,0.f);
   const float textWidth=glyphs.getBoundingBox(0,-1,true).getWidth(),total=13.f+7.f+textWidth;
   auto row=area.toFloat().withSizeKeepingCentre(juce::jmin(total,(float)area.getWidth()),(float)area.getHeight());
   drawIcon(g,"POWER",row.removeFromLeft(13.f).withSizeKeepingCentre(13.f,13.f));
   row.removeFromLeft(7.f);
   g.drawFittedText(text,row.toNearestInt(),juce::Justification::centredLeft,1);
   return;
  }
  g.drawFittedText(text,area,juce::Justification::centred,1);
 }
 // Vector icons drawn in the current colour; returns false when `name` has no icon.
 static bool drawIcon(juce::Graphics&g,const juce::String&name,juce::Rectangle<float> r){
  const auto at=[&](float x,float y){return juce::Point<float>(r.getX()+x*r.getWidth(),r.getY()+y*r.getHeight());};
  const float s=r.getWidth(),stroke=juce::jmax(1.2f,s*.11f);
  const juce::PathStrokeType pen(stroke,juce::PathStrokeType::curved,juce::PathStrokeType::rounded);
  juce::Path p;
  constexpr float pi=juce::MathConstants<float>::pi;
  if(name=="V"){
   p.startNewSubPath(at(.23f,.39f));p.lineTo(at(.5f,.65f));p.lineTo(at(.77f,.39f));
  } else if(name=="POWER"){
   p.addCentredArc(at(.5f,.56f).x,at(.5f,.56f).y,s*.38f,s*.38f,0.f,.62f,2.f*pi-.62f,true);
   p.startNewSubPath(at(.5f,.04f));p.lineTo(at(.5f,.5f));
  } else if(name=="UNDO"||name=="REDO"){
   const bool redo=name=="REDO";const float m=redo?-1.f:1.f;
   const auto c=at(.5f,.58f);
   p.addCentredArc(c.x,c.y,s*.34f,s*.34f,0.f,m*(.5f),m*(pi*1.1f),true);
   const auto tip=at(redo?.74f:.26f,.36f);
   p.startNewSubPath(tip.x+m*-s*.0f,tip.y-s*.2f);p.lineTo(tip);p.lineTo(tip.x+(redo?-1.f:1.f)*s*.2f,tip.y+s*.02f);
  } else if(name=="SAVE"){
   p.addRoundedRectangle(at(.1f,.1f).x,at(.1f,.1f).y,s*.8f,s*.8f,s*.1f);
   p.addRectangle(at(.28f,.1f).x,at(.28f,.1f).y,s*.4f,s*.28f);
   p.addRectangle(at(.26f,.58f).x,at(.26f,.58f).y,s*.48f,s*.32f);
  } else if(name=="SET"){
   const auto c=at(.5f,.5f);
   p.addEllipse(c.x-s*.2f,c.y-s*.2f,s*.4f,s*.4f);
   for(int k=0;k<8;++k){const float a=k*pi/4.f;
    p.startNewSubPath(c.x+std::cos(a)*s*.3f,c.y+std::sin(a)*s*.3f);p.lineTo(c.x+std::cos(a)*s*.46f,c.y+std::sin(a)*s*.46f);}
  } else if(name=="?"){
   g.setFont(Theme::mono(14).boldened());g.drawFittedText("?",r.toNearestInt().expanded(4),juce::Justification::centred,1);return true;
  } else return false;
  g.strokePath(p,pen);
  return true;
 }
 void drawComboBox(juce::Graphics&g,int w,int h,bool,int,int,int,int,juce::ComboBox&)override{
  g.setColour(Theme::panel);g.fillRoundedRectangle(0,0,(float)w,(float)h,4);
  g.setColour(Theme::border);g.drawRoundedRectangle(.5f,.5f,(float)w-1,(float)h-1,4,1.f);
  g.setColour(Theme::green);juce::Path arrow;arrow.addTriangle(w-15.f,h*.42f,w-6.f,h*.42f,w-10.5f,h*.66f);g.fillPath(arrow);
 }
};
