#pragma once
#include "../Theme.h"
#include "RuneMotion.h"
#include "BinaryData.h"
#include <array>
#include <cmath>
// Moving rune field around the REANIMATE dial. 75% of original glyphs physically orbit
// in three bands (different direction/speed); the remaining glyphs are baked into static
// layers for depth and lower CPU. Separate light sweeps and flashes ride on their current
// positions. No glyphs teleport, no image transforms are used in the per-frame paint.
// `active` fades the field in/out, including the moving glyphs.
class RuneRing {
public:
 static constexpr int glyphCount=640,layerCount=5;
 RuneRing(){
  atlas=juce::ImageCache::getFromMemory(BinaryData::rune_atlas_png,BinaryData::rune_atlas_pngSize);
  constexpr float pi=juce::MathConstants<float>::pi;
  juce::Random random(142857);
  size_t glyphNumber=0;
  for(auto& p:placements){
   p.angle=random.nextFloat()*2.f*pi;
   p.radius=std::sqrt(juce::jmap(random.nextFloat(),150.f*150.f,187.f*187.f)); // even area density
   p.x=220.f+std::cos(p.angle)*p.radius;p.y=220.f+std::sin(p.angle)*p.radius;
   p.index=random.nextInt(40);p.size=8.f+random.nextFloat()*6.f;p.weight=.64f+.36f*random.nextFloat();
   // The lower arc stays quiet so the percentage readout remains legible.
   p.fade=skell::necromancer::runes::quietFade(p.angle);
   p.band=p.radius<162.f?0:p.radius<175.f?1:2;
   p.mobile=(glyphNumber++%4)!=0; // 480 actual moving glyphs, 160 static
  }
  for(size_t i=0;i<placements.size();++i){
   const auto&p=placements[i];animatedPositions[i]={p.x,p.y};animatedFade[i]=p.fade;
  }
  // Distinct, persistent orbital sweeps. They never respawn at a random angle: abrupt
  // jumps read as flicker, whereas the reference has highlights travelling across runes.
  constexpr std::array<float,3> initialAngles{-2.3f,.75f,2.85f};
  constexpr std::array<float,3> sweepSpeeds{.86f,.62f,1.06f};
  for(size_t i=0;i<clusters.size();++i){
   auto& c=clusters[i];c.angle=initialAngles[i];c.spin=sweepSpeeds[i];
   c.radius=163.f+6.f*(float)i;c.size=32.f+5.f*(float)i;
   c.life=3.8f+.8f*(float)i;c.age=.75f*(float)i;
  }
  for(size_t i=0;i<flashes.size();++i){flashes[i].age=(float)i*.03f;flashes[i].life=.2f+(float)i*.02f;flashes[i].glyph=random.nextInt(glyphCount);}
  for(auto& img:layers)img=juce::Image(juce::Image::ARGB,440,440,true);
  for(int j=0;j<layerCount;++j){
   juce::Graphics g(layers[(size_t)j]);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
   for(int i=j;i<glyphCount;i+=layerCount){const auto&p=placements[(size_t)i];
    if(p.mobile)continue; // mobile glyphs must not leave static ghost copies
    g.setOpacity(p.weight*p.fade);
    g.setColour(tint(p).withAlpha(p.weight*p.fade));draw(g,p);}
  }
  // Per-glyph flare sprites: slightly enlarged, tinted toward white.
  for(size_t i=0;i<placements.size();++i){
   const auto&p=placements[i];const float size=p.size*1.45f;const int n=(int)std::ceil(size)+2;
   sprites[i]=juce::Image(juce::Image::ARGB,n,n,true);
   juce::Graphics sg(sprites[i]);sg.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
   sg.setColour(tint(p).interpolatedWith(juce::Colours::white,.48f));
   const auto glyph=atlas.getClippedImage({(p.index%8)*32,(p.index/8)*32,32,32});
   sg.drawImageTransformed(glyph,juce::AffineTransform::scale(size/32.f).translated(((float)n-size)/2,((float)n-size)/2),true);
   // Separate normal-sized sprites for moving runes: the existing 1.45x flare
   // sprites are too large to use for the body of the animated glyph field.
   if(p.mobile){
    const int normalN=(int)std::ceil(p.size)+2;
    motionSprites[i]=juce::Image(juce::Image::ARGB,normalN,normalN,true);
    juce::Graphics mg(motionSprites[i]);
    mg.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    // drawImageTransformed(..., true) fills the atlas alpha with the current
    // brush. Without setColour here, 480 orbiting symbols were BLACK!
    // Bake their colour once, not during the real-time paint loop.
    mg.setColour(tint(p).interpolatedWith(juce::Colours::white,.16f));
    mg.drawImageTransformed(glyph,juce::AffineTransform::scale(p.size/32.f).translated(((float)normalN-p.size)/2,((float)normalN-p.size)/2),true);
   }
  }
 }
 // Bypassed dials do not need to orbit 480 glyphs on every VBlank.
 // Keep the timestamp fresh so reactivation has no time jump.
 void pause(double t) noexcept {lastT=t;}
 // Advance the sparkle state to time `t` (seconds). Call once per animation frame.
 void advance(double t,float live){
  const double dt=juce::jlimit(0.0,.1,t-lastT);lastT=t;
  // Physical rune motion is independent of input level: it must still be visible
  // on silence. Audio activity gently accelerates the three counter-rotating bands.
  for(size_t b=0;b<phase.size();++b){
   phase[b]+=skell::necromancer::runes::phaseStep(b,(float)dt,live);
   if(phase[b]>juce::MathConstants<float>::twoPi)phase[b]-=juce::MathConstants<float>::twoPi;
   if(phase[b]<-juce::MathConstants<float>::twoPi)phase[b]+=juce::MathConstants<float>::twoPi;
  }
  for(size_t i=0;i<placements.size();++i){
   const auto&p=placements[i];
   if(!p.mobile)continue;
   const float a=p.angle+phase[(size_t)p.band];
   const auto pos=skell::necromancer::runes::position(p.angle,p.radius,phase[(size_t)p.band]);
   animatedPositions[i]={pos.x,pos.y};
   // Fade the bottom of the orbit so moving runes cannot cover the percent label.
   animatedFade[i]=skell::necromancer::runes::quietFade(a);
  }
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
   g.setOpacity(active*(.52f+.24f*amount)*(.8f+.2f*breathe));
   g.drawImageAt(layers[(size_t)j],0,0);
  }
  g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
  // Draw cached per-rune sprites at their *changing coordinates* every frame.
  // The fixed layer images no longer contain these glyphs, so actual movement
  // is visible instead of just a glow passing over stationary characters.
  for(size_t i=0;i<placements.size();++i){
   const auto&p=placements[i];if(!p.mobile)continue;
   const auto&pt=animatedPositions[i];const auto&sprite=motionSprites[i];
   // Colored orbital sprites stay intentionally dim at rest: moving flashes
   // travel across them instead of making the whole ring glow at once.
   g.setOpacity(active*(.30f+.08f*amount)*p.weight*animatedFade[i]);
   g.drawImageAt(sprite,(int)std::lround(pt.x)-sprite.getWidth()/2,(int)std::lround(pt.y)-sprite.getHeight()/2);
  }
  const float gain=active*(.57f+.2f*live);
  for(const auto&c:clusters){
   const float env=.62f+.38f*std::sin(juce::MathConstants<float>::twoPi*c.age/c.life);
   const float cx=220.f+std::cos(c.angle)*c.radius,cy=220.f+std::sin(c.angle)*c.radius;
   for(size_t i=0;i<placements.size();++i){
    const auto&p=placements[i];const auto&pt=animatedPositions[i];
    const float dx=pt.x-cx,dy=pt.y-cy,d2=dx*dx+dy*dy;if(d2>c.size*c.size)continue;
    const float fall=1.f-std::sqrt(d2)/c.size;
    drawBright(g,p,pt,gain*env*fall*animatedFade[i]*.86f);
   }
  }
  for(const auto&f:flashes){
   const float env=std::sin(juce::MathConstants<float>::pi*f.age/f.life);
   const auto i=(size_t)f.glyph;const auto&p=placements[i];drawBright(g,p,animatedPositions[i],gain*env*animatedFade[i]*.95f);
  }
 }
private:
 struct Placement {float angle=0,radius=0,x=0,y=0,size=0,weight=1,fade=1;int index=0,band=0;bool mobile=false;};
 struct Cluster {float angle=0,radius=160,age=1,life=1,spin=0,size=18;};
 struct Flash {int glyph=0;float age=1,life=1;};
 static juce::Colour tint(const Placement&p){
  // The reference mixes lavender, cyan, pink and occasional warm highlights,
  // rather than mostly the same green. Keep original rune silhouettes.
  static const std::array<juce::Colour,7> palette{
   juce::Colour(0xffa681f5), juce::Colour(0xff86e4ed),
   juce::Colour(0xffeb8bae), juce::Colour(0xff8ec7ff),
   juce::Colour(0xffffbb86), juce::Colour(0xffb6edba),
   juce::Colour(0xffc5a4df)
  };
  return palette[(size_t)((p.index*5 + p.band*3)%7)].interpolatedWith(Theme::bone,.12f);
 }
 void draw(juce::Graphics&g,const Placement&p) const {
  const float x=p.x-p.size/2,y=p.y-p.size/2;
  const auto glyph=atlas.getClippedImage({(p.index%8)*32,(p.index/8)*32,32,32});
  g.drawImageTransformed(glyph,juce::AffineTransform::scale(p.size/32.f).translated(x,y),true);
 }
 // Bright flares are plain blits of pre-baked per-glyph sprites (transform-filling glyphs every frame is far too slow).
 void drawBright(juce::Graphics&g,const Placement&p,const juce::Point<float>&pt,float alpha) const {
  if(alpha<.01f)return;
  const auto&sprite=sprites[(size_t)(&p-placements.data())];
  g.setOpacity(juce::jlimit(0.f,1.f,alpha));
  g.drawImageAt(sprite,(int)std::lround(pt.x)-sprite.getWidth()/2,(int)std::lround(pt.y)-sprite.getHeight()/2);
 }
 std::array<Placement,glyphCount>placements{};
 std::array<juce::Point<float>,glyphCount> animatedPositions{};
 std::array<float,glyphCount> animatedFade{};
 std::array<float,3> phase{};
 std::array<Cluster,3>clusters{};std::array<Flash,16>flashes{};
 juce::Random animRandom{20260101};double lastT=0;juce::Image atlas;
 std::array<juce::Image,glyphCount>sprites,motionSprites;
 std::array<juce::Image,layerCount>layers;
};
