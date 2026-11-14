#pragma once
#include "RuneRing.h"
#include "PolishedCap.h"
class ReanimateDial:public juce::Slider,private juce::Timer {
public:
 ReanimateDial() {setRange(0.0,100.0,0.1);setValue(65.0,juce::dontSendNotification);setSliderStyle(juce::Slider::RotaryVerticalDrag);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
 setDoubleClickReturnValue(true,65);setPopupMenuEnabled(true);setPopupDisplayEnabled(true,false,nullptr);setMouseCursor(juce::MouseCursor::PointingHandCursor);
 well=juce::ImageCache::getFromMemory(BinaryData::dial_well_png,BinaryData::dial_well_pngSize);
 cap=juce::ImageCache::getFromMemory(BinaryData::knob_cap_big_png,BinaryData::knob_cap_big_pngSize);
 polishedFace=NecroPolishedCap::makeFace();
 setTooltip("REANIMATE blends tape weight, gentle compression, harmonic life and restored transients. Double-click returns to 65%.");
 // Some VST hosts do not deliver VBlank callbacks consistently. The JUCE
 // message-thread timer advances the runes at 30 Hz only when VBlank stalls.
 startTimerHz(30);}
 void setLive(float v){live=juce::jlimit(0.f,1.f,v);}
 // REVIVE on and not bypassed: the rune field fades in; otherwise the well empties (as in a bypassed unit).
 void setActive(bool on){target=on?1.f:0.f;}
 void tickFromVBlank(){lastVBlankMs=juce::Time::getMillisecondCounterHiRes();advance();}
 void advance(bool fallback=false){
  const double now=juce::Time::getMillisecondCounterHiRes()*.001,dt=juce::jlimit(0.0,.1,now-lastTime);lastTime=now;
  const bool fading=std::abs(target-act)>.002f;
  act=fading?act+(target-act)*(1.f-(float)std::exp(-dt*9.0)):target;
  seconds=now;ring.advance(now,live);
  // VBlank usually calls at ~60 Hz: paint every other frame. The timer fallback
  // runs at 30 Hz already, so paint *every* timer tick (not 15 Hz).
  if(((++frame&1)==0||fallback)&&(act>.002f||fading))repaint();
 }
 void paint(juce::Graphics&g)override {
  g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
  g.drawImage(well,getLocalBounds().toFloat());
  ring.paint(g,(float)getValue()*.01f,live,act,seconds);
  auto cx=220.f,cy=220.f;const float p=(float)(getValue()/100.0);
  const float start=juce::MathConstants<float>::pi*.75f,span=juce::MathConstants<float>::pi*1.5f;
  for(int t=0;t<90;++t){float a=start+span*t/90.f;float x1=cx+std::cos(a)*139.f,y1=cy+std::sin(a)*139.f;
   const bool major=t%10==0;const float rad=major?148:144;
   g.setColour(t<p*90?Theme::bone.withAlpha(.35f+.52f*act):Theme::faint.withAlpha(.32f));
   g.drawLine(x1,y1,cx+std::cos(a)*rad,cy+std::sin(a)*rad,major?1.2f:.7f);
  }
  juce::Path path;for(int s=0;s<=180;++s){float a=start+span*(s/180.f)*p;
    auto x=cx+std::cos(a)*133.f,y=cy+std::sin(a)*133.f;if(s==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}
  // Thin spectral trace: violet/pink at the bottom-left, warm light across the
  // upper arc, then green-to-cyan at the right. The metal cap remains unchanged.
  juce::ColourGradient gradient(juce::Colour(0xffac7aff),96,338,Theme::cyan,355,115,false);
  gradient.addColour(.23,juce::Colour(0xfff38ec7));
  gradient.addColour(.45,juce::Colour(0xffffca79));
  gradient.addColour(.74,Theme::greenLight);
  const juce::PathStrokeType stroke(2.7f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded);
  if(act<.999f){g.setColour(Theme::faint.withAlpha(.45f*(1.f-act)));g.strokePath(path,stroke);}
  {juce::Graphics::ScopedSaveState arcState(g);g.setGradientFill(gradient);g.setOpacity(act);g.strokePath(path,stroke);}
  const float angle=start+span*p;
  {juce::Graphics::ScopedSaveState save(g);g.setOpacity(1.f); // drawImage takes its alpha from the current colour
   g.addTransform(juce::AffineTransform::rotation(angle-start-1.18f,cx,cy));
   g.drawImage(cap,{cx-104.f,cy-104.f,208.f,208.f});
   // Original outer hardware rim remains; the central face is now clean metal.
   g.drawImageAt(polishedFace,(int)cx-104,(int)cy-104);}
  g.setColour(Theme::bone);g.drawLine(cx+std::cos(angle)*109.f,cy+std::sin(angle)*109.f,cx+std::cos(angle)*127.f,cy+std::sin(angle)*127.f,5.f);
  // Reference-accurate heading above the metal face, protected by subtle dark
  // glazing so moving symbols remain legible behind it.
  g.setColour(juce::Colours::black.withAlpha(.36f));
  g.fillRoundedRectangle(132.f,48.f,176.f,25.f,4.f);
  g.setColour(Theme::bone.withAlpha(.94f));
  g.setFont(Theme::display(12).withExtraKerningFactor(.28f));
  g.drawFittedText("REANIMATE",juce::Rectangle<int>(132,50,176,21),juce::Justification::centred,1);
  g.setFont(Theme::display(36));g.drawFittedText(juce::String((int)std::lround(getValue()))+"%",juce::Rectangle<int>(130,377,180,47),juce::Justification::centred,1);
  if(isMouseOverOrDragging()){g.setColour(Theme::greenLight.withAlpha(.28f));g.drawEllipse(90,90,260,260,1.f);}
 }
private:
 void timerCallback() override {
  if(juce::Time::getMillisecondCounterHiRes()-lastVBlankMs>75.0)advance(true);
 }
 juce::Image well,cap,polishedFace;RuneRing ring;float live=0,act=1.f,target=1.f;
 int frame=0;double seconds=0,lastTime=0,lastVBlankMs=0;
};
