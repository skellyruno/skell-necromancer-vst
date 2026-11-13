#pragma once
#include "../Theme.h"
#include "BinaryData.h"
#include <array>
#include <cmath>
// Dense field of original rune glyphs around the REANIMATE dial. Glyphs are baked into a few
// cached layers; per frame the layers shimmer independently and a soft highlight cone travels
// around the ring (faster/brighter with live level). `active` fades the whole field in or out.
class RuneRing {
public:
 static constexpr int glyphCount=640,layerCount=5;
 RuneRing(){
  atlas=juce::ImageCache::getFromMemory(BinaryData::rune_atlas_png,BinaryData::rune_atlas_pngSize);
  constexpr float pi=juce::MathConstants<float>::pi;
  juce::Random random(142857);
  for(auto& p:placements){
   p.angle=random.nextFloat()*2.f*pi;
   p.radius=std::sqrt(juce::jmap(random.nextFloat(),150.f*150.f,187.f*187.f)); // even area density
   p.index=random.nextInt(40);p.size=7.f+random.nextFloat()*7.f;p.weight=.55f+.45f*random.nextFloat();
   // The lower arc stays quiet so the percentage readout remains legible.
   const float s=juce::jlimit(0.f,1.f,(std::sin(p.angle)-.45f)/.5f);
   p.fade=1.f-.85f*s*s*(3.f-2.f*s);
  }
  for(auto& img:layers)img=juce::Image(juce::Image::ARGB,440,440,true);
  bright=juce::Image(juce::Image::ARGB,440,440,true);
  for(int j=0;j<layerCount;++j){
   juce::Graphics g(layers[(size_t)j]);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
   for(int i=j;i<glyphCount;i+=layerCount){const auto&p=placements[(size_t)i];
    g.setOpacity(p.weight*p.fade);
    g.setColour(tint(p).withAlpha(p.weight*p.fade));draw(g,p);}
  }
  juce::Graphics g(bright);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
  for(const auto&p:placements){g.setColour(Theme::bone.withAlpha(p.fade));draw(g,p);}
 }
 // amount: REANIMATE 0..1, live: signal activity 0..1, active: fade 0..1, t: seconds
 void paint(juce::Graphics&g,float amount,float live,float active,double t) const {
  if(active<.004f)return;
  juce::Graphics::ScopedSaveState state(g);
  for(int j=0;j<layerCount;++j){
   const float pulse=.5f+.5f*(float)std::sin(t*(.9+.37*j)+j*1.7);
   g.setOpacity(active*(.55f+.35f*amount+.1f*live)*(.55f+.45f*pulse));
   g.drawImageAt(layers[(size_t)j],0,0);
  }
  const float a0=(float)(t*(.42+.5*live)+.9*std::sin(t*.21));
  spot(g,a0,.55f+.2f*live,active*(.7f+.3f*live));
  spot(g,a0+2.6f,.4f,active*.45f);
 }
private:
 struct Placement {float angle=0,radius=0,size=0,weight=1,fade=1;int index=0;};
 static juce::Colour tint(const Placement&p){
  return Theme::spectral(.5f+.5f*std::cos(p.angle)).interpolatedWith(Theme::bone,.35f);
 }
 void draw(juce::Graphics&g,const Placement&p) const {
  const float x=220+std::cos(p.angle)*p.radius-p.size/2,y=220+std::sin(p.angle)*p.radius-p.size/2;
  const auto glyph=atlas.getClippedImage({(p.index%8)*32,(p.index/8)*32,32,32});
  g.drawImageTransformed(glyph,juce::AffineTransform::scale(p.size/32.f).translated(x,y),true);
 }
 // Soft cone: four nested wedges of the bright layer build a smooth angular falloff.
 void spot(juce::Graphics&g,float angle,float halfWidth,float gain) const {
  constexpr float quarterTurn=juce::MathConstants<float>::halfPi;
  for(int k=0;k<4;++k){
   const float w=halfWidth*(1.f-.24f*(float)k);
   juce::Path wedge;wedge.addPieSegment(juce::Rectangle<float>(0.f,0.f,440.f,440.f),angle-w+quarterTurn,angle+w+quarterTurn,0.f);
   juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(wedge);
   g.setOpacity(gain*(.16f+.06f*(float)k));g.drawImageAt(bright,0,0);
  }
 }
 std::array<Placement,glyphCount>placements{};juce::Image atlas,bright;std::array<juce::Image,layerCount>layers;
};
