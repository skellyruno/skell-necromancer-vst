// Independent reference parity: Glow must preserve its previous signal
// exactly while removing the sample-rate-dependent exp() from process().
#include "../Source/dsp/Glow.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

struct ReferenceGlow {
 double sr=44100,phase=0;
 float wet=0,low=0,stateA=0,stateB=0;
 void prepare(double rate) {sr=rate;phase=0;low=stateA=stateB=0;}
 void set(float amount,int mode) {
  wet=mode==1?juce::jlimit(0.f,.025f,.025f*amount):0.f;
 }
 void process(float& l,float& r) {
  if(wet<=0)return;
  const float mid=.5f*(l+r);
  const float coeff=(float)(1.0-std::exp(-juce::MathConstants<double>::twoPi*1800.0/sr));
  low+=coeff*(mid-low);
  const float hi=mid-low;
  phase+=juce::MathConstants<double>::twoPi*.13/sr;
  if(phase>=juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;
  const float a=.25f+.08f*(float)std::sin(phase);
  const float b=.47f-.08f*(float)std::cos(phase);
  const float ya=stateA-a*hi;stateA=hi+a*ya;
  const float yb=stateB-b*hi;stateB=hi+b*yb;
  const float delta=wet*(ya-yb);
  l+=delta;r-=delta;
 }
};
int main() {
 double worst=0,monoError=0;
 uint32_t seed=120932;
 for(double rate:{44100.,48000.,96000.,192000.}) {
  NecroDSP::Glow current;ReferenceGlow old;
  current.prepare(rate);old.prepare(rate);
  for(int n=0;n<48000;++n) {
   if(n%2048==0) {
    const int mode=(n/2048)%4==0?0:1;
    const float wet=((n/2048)%3==0)?0.f:((n/2048)%3==1?.44f:1.f);
    current.set(wet,mode);old.set(wet,mode);
   }
   seed=1664525u*seed+1013904223u;
   const float noise=float((seed>>8)&0xffff)/65535.f-.5f;
   const float inputL=.35f*std::sin(juce::MathConstants<double>::twoPi*780.*n/rate)+.08f*noise;
   const float inputR=.27f*std::sin(juce::MathConstants<double>::twoPi*2400.*n/rate)-.12f*noise;
   float l=inputL,r=inputR,lr=l,rr=r;
   current.process(l,r);old.process(lr,rr);
   if(!std::isfinite(l)||!std::isfinite(r))return 1;
   worst=std::max(worst,(double)std::max(std::abs(l-lr),std::abs(r-rr)));
   monoError=std::max(monoError,(double)std::abs(((l+r)-(inputL+inputR))*.5f));
  }
  if(worst>2.e-7||monoError>2.e-7) {
   std::cerr<<"Glow parity/mono failure at "<<rate<<" Hz: "<<worst<<" / "<<monoError<<"\n";
   return 2;
  }
 }
 // Prepare with pathological host sample rate must not introduce nonfinite state.
 NecroDSP::Glow bad;bad.prepare(0);bad.set(.8f,1);
 float l=.25f,r=-.25f;
 for(int i=0;i<1000;++i){bad.process(l,r);if(!std::isfinite(l)||!std::isfinite(r))return 3;}
 std::cout<<"PASS: 4 sample rates, modes and automation, <=2e-7 output difference, "
          <<"mono sum preserved; worst "<<worst<<"\n";
}
