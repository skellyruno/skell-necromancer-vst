#pragma once
#include <JuceHeader.h>
#include <cmath>
// Cached vector-machined centre face. Drawn once per editor instance, not per
// animation frame. Keeps the previous metal cap's outer bevel and Skell artwork
// assets available for rollback; no generated bitmap assets or DSP dependencies.
namespace NecroPolishedCap {
inline juce::Image makeFace() {
 constexpr int diameter=208;
 constexpr float centre=104.f, radius=89.f;
 constexpr float pi=juce::MathConstants<float>::pi;
 constexpr float turn=juce::MathConstants<float>::twoPi;
 juce::Image image(juce::Image::ARGB,diameter,diameter,true);
 juce::Graphics g(image);
 g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
 // Shadowed perimeter and hairline machined edge.
 g.setColour(juce::Colour(0xff090b0d).withAlpha(.96f));
 g.fillEllipse(centre-radius-4.f,centre-radius-4.f,(radius+4.f)*2.f,(radius+4.f)*2.f);
 g.setColour(juce::Colour(0xffa3a7a8));
 g.fillEllipse(centre-radius-1.f,centre-radius-1.f,(radius+1.f)*2.f,(radius+1.f)*2.f);
 juce::Path face;
 face.addEllipse(centre-radius,centre-radius,radius*2.f,radius*2.f);
 {
  juce::Graphics::ScopedSaveState state(g);
  g.reduceClipRegion(face);
  // Conical anisotropic reflection; soft sector blending follows the brushed
  // aluminium highlights in the reference without baking in the vendor logo.
  constexpr int sectors=320;
  for(int i=0;i<sectors;++i){
   const float a0=-pi+turn*(float)i/(float)sectors;
   const float a1=-pi+turn*(float)(i+1)/(float)sectors;
   const float a=(a0+a1)*.5f;
   float light=.46f+.205f*std::cos(a*2.f-1.1f)
                    +.09f*std::cos(a*4.f+.3f)
                    +.035f*std::cos(a*8.f-2.3f);
   light=juce::jlimit(.18f,.79f,light);
   const float c=light;
   g.setColour(juce::Colour::fromFloatRGBA(c,c*.995f,c*.98f,1.f));
   juce::Path wedge;
   wedge.startNewSubPath(centre,centre);
   wedge.lineTo(centre+std::cos(a0)*radius*1.03f,
                centre+std::sin(a0)*radius*1.03f);
   wedge.lineTo(centre+std::cos(a1)*radius*1.03f,
                centre+std::sin(a1)*radius*1.03f);
   wedge.closeSubPath();
   g.fillPath(wedge);
  }
  juce::ColourGradient glaze(juce::Colours::white.withAlpha(.13f),
    centre-38.f,centre-41.f,juce::Colours::black.withAlpha(.17f),
    centre+62.f,centre+73.f,false);
  g.setGradientFill(glaze);
  g.fillEllipse(centre-radius,centre-radius,radius*2.f,radius*2.f);
  // Sparse subpixel brushing is deliberately low contrast.
  for(int i=0;i<84;++i){
   const float a=turn*(float)i/84.f;
   g.setColour(juce::Colours::white.withAlpha(.018f));
   g.drawLine(centre,centre,
       centre+std::cos(a)*radius,centre+std::sin(a)*radius,.4f);
  }
 }
 g.setColour(juce::Colours::black.withAlpha(.46f));
 g.drawEllipse(centre-radius,centre-radius,radius*2.f,radius*2.f,1.7f);
 g.setColour(juce::Colours::white.withAlpha(.28f));
 g.drawEllipse(centre-radius+1.5f,centre-radius+1.5f,
               (radius-1.5f)*2.f,(radius-1.5f)*2.f,.7f);
 return image;
}
} // namespace NecroPolishedCap
