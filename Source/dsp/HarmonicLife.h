#pragma once
#include "Biquad.h"
#include "Oversampler.h"
namespace NecroDSP {
class HarmonicLife {
public:
 void prepare(double sr) noexcept {rate=sr;reset();for(auto& f:detectors)f.design(Biquad::Kind::highpass,sr,2700,0,.707);for(auto& f:split)f.design(Biquad::Kind::highpass,sr,2700,0,.707);for(auto& f:air)f.design(Biquad::Kind::highpass,sr,8000,0,.707);}
 void reset() noexcept {os.reset();for(auto& f:split)f.reset();for(auto& f:air)f.reset();for(auto& f:detectors)f.reset();hiRms=midRms=0;}
 void set(float amount,int mode,int osFactor,float guard) noexcept {a=amount;voice=mode;factor=osFactor;deEss=guard;}
 void advance(float l,float r) noexcept {auto x=std::max(std::abs(l),std::abs(r));const double k=1.0/(rate*.22);midRms+=(x*x-midRms)*k;const double hi=std::max(std::abs(detectors[0].process(0,l)),std::abs(detectors[1].process(1,r)));hiRms+=(hi*hi-hiRms)*k;}
 float process(int ch,float x) noexcept {
  auto upper=(float)split[ch].process(ch,x);
  const float dullness=juce::jlimit(0.f,1.f,1.f-(float)std::sqrt(hiRms/(midRms+1e-8)));
  const float voiceBoost=voice==0?1.25f:voice==1?.72f:1.f;
  auto e=os.process(ch,upper,factor,[&](float u) noexcept {return std::tanh(u*2.6f)-u;});
  const float sibilance=(float)std::sqrt(hiRms/(midRms+1e-8));
  const float deessFactor=voice==0?1.0f-deEss*juce::jlimit(0.f,.95f,(sibilance-.3f)*1.75f):1.f;
  return x + (float)air[ch].process(ch,e)*(a*.20f*voiceBoost*(.4f+.6f*dullness)*deessFactor);
 }
private:double rate=44100,hiRms=0,midRms=0;float a=0,deEss=0;int voice=0,factor=1;std::array<Biquad,2>split,air,detectors;Oversampler os;
};
}
