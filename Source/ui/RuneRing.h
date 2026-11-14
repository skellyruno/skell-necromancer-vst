#pragma once
#include "../Theme.h"
#include "BinaryData.h"
#include <array>
#include <cmath>
// Dense field of original rune glyphs around the REANIMATE dial. Glyphs stay anchored; moving
// illumination bands, not glyph rotation or random cluster teleportation, animate the field.
// Brief independent flares add the intermittent sparkle seen in the supplied reference video.
// Static glyphs are baked into a few cached layers that breathe slowly. `active` fades the field in/out.
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
   p.x=220.f+std::cos(p.angle)*p.radius;p.y=220.f+std::sin(p.angle)*p.radius;
   p.index=random.nextInt(40);p.size=7.f+random.nextFloat()*7.f;p.weight=.55f+.45f*random.nextFloat();
   // The lower arc stays quiet so the percentage readout remains legible.
   const float s=juce::jlimit(0.f,1.f,(std::sin(p.angle)-.45f)/.5f);
   p.fade=1.f-.85f*s*s*(3.f-2.f*s);
  }
  // Distinct, persistent orbital sweeps. They never respawn at a random angle: abrupt
  // jumps read as flicker, whereas the reference has highlights travelling across runes.
  constexpr std::array<float,3> initialAngles{-2.3f,.75f,2.85f};
  constexpr std::array<float,3> sweepSpeeds{.86f,.62f,1.06f};
  for(size_t i=0;i<clusters.size();++i){
   auto& c=clusters[i];c.angle=initialAngles[i];c.spin=sweepSpeeds[i];
   c.radius=163.f+6.f*(float)i;c.size=47.f+6.f*(float)i;
   c.life=3.8f+.8f*(float)i;c.age=.75f*(float)i;
  }
  for(size_t i=0;i<flashes.size();++i){flashes[i].age=(float)i*.03f;flashes[i].life=.2f+(float)i*.02f;flashes[i].glyph=random.nextInt(glyphCount);}
  for(auto& img:layers)img=juce::Image(juce::Image::ARGB,440,440,true);
  for(int j=0;j<layerCount;++j){
   juce::Graphics g(layers[(size_t)j]);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
   for(int i=j;i<glyphCount;i+=layerCount){const auto&p=placements[(size_t)i];
    g.setOpacity(p.weight*p.fade);
    g.setColour(tint(p).withAlpha(p.weight*p.fade));draw(g,p);}
  }
  // Per-glyph flare sprites: slightly enlarged, tinted toward white.
  for(size_t i=0;i<placements.size();++i){
   const auto&p=placements[i];const float size=p.size*1.45f;const int n=(int)std::ceil(size)+2;
   sprites[i]=juce::Image(juce::Image::ARGB,n,n,true);
   juce::Graphics sg(sprites[i]);sg.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
   sg.setColour(tint(p).interpolatedWith(juce::Colours::white,.8f));
   const auto glyph=atlas.getClippedImage({(p.index%8)*32,(p.index/8)*32,32,32});
   sg.drawImageTransformed(glyph,juce::AffineTransform::scale(size/32.f).translated(((float)n-size)/2,((float)n-size)/2),true);
  }
 }
 // Advance the sparkle state to time `t` (seconds). Call once per animation frame.
 void advance(double t,float live){
  const double dt=juce::jlimit(0.0,.1,t-lastT);lastT=t;
  for(auto&c:clusters){
   c.age+=(float)dt;
   if(c.age>=c.life)c.age=std::fmod(c.age,c.life);
   c.angle+=c.spin*(float)dt*(.8f+.35f*live);
   // Wrapping the phase changes no rendered position; it only prevents precision drift.
   if(c.angle>juce::MathConstants<float>::twoPi)c.angle-=juce::MathConstants<float>::twoPi;
  }
  for(auto&f:flashes){
   f.age+=(float)dt;
   if(f.age>=f.life){f.glyph=animRandom.nextInt(glyphCount);f.life=.18f+animRandom.nextFloat()*.4f;f.age=0.f;}
  }
 }
 // amount: REANIMATE 0..1, live: signal activity 0..1, active: fade 0..1, t: seconds
 void paint(juce::Graphics&g,float amount,float live,float active,double t) const {
  if(active<.004f)return;
  juce::Graphics::ScopedSaveState state(g);
  for(int j=0;j<layerCount;++j){
   const float breathe=.5f+.5f*(float)std::sin(t*(.5+.21*j)+j*1.7);
   g.setOpacity(active*(.5f+.3f*amount)*(.78f+.22f*breathe));
   g.drawImageAt(layers[(size_t)j],0,0);
  }
  g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
  const float gain=active*(.75f+.25f*live);
  for(const auto&c:clusters){
   const float env=.62f+.38f*std::sin(juce::MathConstants<float>::twoPi*c.age/c.life);
   const float cx=220.f+std::cos(c.angle)*c.radius,cy=220.f+std::sin(c.angle)*c.radius;
   for(const auto&p:placements){
    const float dx=p.x-cx,dy=p.y-cy,d2=dx*dx+dy*dy;if(d2>c.size*c.size)continue;
    const float fall=1.f-std::sqrt(d2)/c.size;
    drawBright(g,p,gain*env*fall*p.fade*1.7f);
   }
  }
  for(const auto&f:flashes){
   const float env=std::sin(juce::MathConstants<float>::pi*f.age/f.life);
   const auto&p=placements[(size_t)f.glyph];drawBright(g,p,gain*env*p.fade*1.4f);
  }
 }
private:
 struct Placement {float angle=0,radius=0,x=0,y=0,size=0,weight=1,fade=1;int index=0;};
 struct Cluster {float angle=0,radius=160,age=1,life=1,spin=0,size=18;};
 struct Flash {int glyph=0;float age=1,life=1;};
 static juce::Colour tint(const Placement&p){
  return Theme::spectral(.5f+.5f*std::cos(p.angle)).interpolatedWith(Theme::bone,.35f);
 }
 void draw(juce::Graphics&g,const Placement&p) const {
  const float x=p.x-p.size/2,y=p.y-p.size/2;
  const auto glyph=atlas.getClippedImage({(p.index%8)*32,(p.index/8)*32,32,32});
  g.drawImageTransformed(glyph,juce::AffineTransform::scale(p.size/32.f).translated(x,y),true);
 }
 // Bright flares are plain blits of pre-baked per-glyph sprites (transform-filling glyphs every frame is far too slow).
 void drawBright(juce::Graphics&g,const Placement&p,float alpha) const {
  if(alpha<.01f)return;
  const auto&sprite=sprites[(size_t)(&p-placements.data())];
  g.setOpacity(juce::jlimit(0.f,1.f,alpha));
  g.drawImageAt(sprite,(int)std::lround(p.x)-sprite.getWidth()/2,(int)std::lround(p.y)-sprite.getHeight()/2);
 }
 std::array<Placement,glyphCount>placements{};std::array<Cluster,3>clusters{};std::array<Flash,16>flashes{};juce::Random animRandom{20260101};double lastT=0;juce::Image atlas;std::array<juce::Image,glyphCount>sprites;std::array<juce::Image,layerCount>layers;
};
