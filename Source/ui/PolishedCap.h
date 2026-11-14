#pragma once
#include <JuceHeader.h>
#include <cmath>
// A smooth, cached anisotropic silver face for the large dial. Writing the
// pixels directly avoids radial spokes and 320 adjacent antialiased wedge
// edges, which created visible moire at 70–100% editor sizes.
// Generated once when the editor is constructed, NEVER on the audio thread.
namespace NecroPolishedCap {
inline juce::Image makeFace() {
 constexpr int diameter=208;
 constexpr float centre=104.f, radius=89.f;
 juce::Image image(juce::Image::ARGB,diameter,diameter,true);
 {
  juce::Image::BitmapData pixels(image,juce::Image::BitmapData::readWrite);
  for(int y=0;y<diameter;++y)for(int x=0;x<diameter;++x){
   const float dx=(float)x+.5f-centre,dy=(float)y+.5f-centre;
   const float rr=std::sqrt(dx*dx+dy*dy);
   if(rr>=radius+1.f)continue;
   const float a=std::atan2(dy,dx);
   // Two broad metallic reflection lobes with a low-amplitude secondary
   // highlight. Continuous analytical colour eliminates radial aliasing.
   float light=.47f+.20f*std::cos(2.f*a-1.1f)
                    +.045f*std::cos(4.f*a+.3f);
   const float radial=juce::jlimit(0.f,1.f,rr/radius);
   light+=.035f*(1.f-radial)-.018f*radial;
   light=juce::jlimit(.22f,.80f,light);
   // Alpha-coverage antialiasing at the physical circular boundary.
   const float alpha=juce::jlimit(0.f,1.f,radius+.5f-rr);
   pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
       light,light*.995f,light*.985f,alpha));
  }
 }
 juce::Graphics g(image);
 g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
 // Hairline machined rim; original cap bitmap supplies the wider outer bezel.
 g.setColour(juce::Colours::black.withAlpha(.62f));
 g.drawEllipse(centre-radius-1.5f,centre-radius-1.5f,
               (radius+1.5f)*2.f,(radius+1.5f)*2.f,1.3f);
 g.setColour(juce::Colours::white.withAlpha(.22f));
 g.drawEllipse(centre-radius+.5f,centre-radius+.5f,
               (radius-.5f)*2.f,(radius-.5f)*2.f,.7f);
 return image;
}
} // namespace NecroPolishedCap
