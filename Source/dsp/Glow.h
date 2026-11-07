#pragma once
#include "Biquad.h"
namespace NecroDSP {
// Very low-level upper-mid decorrelation; anti-phase injection preserves exact
// mono summation (L+R)/2. Maximum nominal gain < 2.5% of the HP component.
class Glow {
public:
 void prepare(double sampleRate)noexcept {sr=sampleRate;reset();}
 void reset()noexcept {low=0;stateA=stateB=0;phase=0;}
 void set(float amount,int mode)noexcept {wet=mode==1?juce::jlimit(0.f,.025f,.025f*amount):0.f;}
 void process(float&l,float&r)noexcept {
  if(wet<=0)return;
  const float mid=.5f*(l+r);
  const float onePole=(float)(1.0-std::exp(-juce::MathConstants<double>::twoPi*1800.0/sr));
  low+=onePole*(mid-low);const float hi=mid-low;
  phase+=juce::MathConstants<double>::twoPi*.13/sr;
  if(phase>=juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;
  // Two simple allpass phases create a slowly shifting, mono-safe side field.
  const float a=.25f+.08f*(float)std::sin(phase);
  const float b=.47f-.08f*(float)std::cos(phase);
  const float ya=stateA-a*hi;stateA=hi+a*ya;
  const float yb=stateB-b*hi;stateB=hi+b*yb;
  const float injected=wet*(ya-yb);
  l+=injected;r-=injected;
 }
private:double sr=44100,phase=0;float wet=0,low=0,stateA=0,stateB=0;
};
}
