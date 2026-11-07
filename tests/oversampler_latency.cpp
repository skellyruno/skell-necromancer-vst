#include "../Source/dsp/Oversampler.h"
#include "../Source/dsp/ReviveEngine.h"
#include "../Source/dsp/TruePeakLimiter.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
static void require(bool valid,const char* reason){
 if(!valid){std::cerr<<"Oversampler/PDC failure: "<<reason<<"\n";std::exit(3);}
}
int main(){
 constexpr double sr=48000.0;
 for(int factor:{1,2,4}){
  NecroDSP::Oversampler os;
  const int delay=NecroDSP::Oversampler::latencyForFactor(factor);
  int peakAt=-1;float peak=0;
  double error=0,energy=0;
  for(int n=0;n<4096;++n){
   const float x=n==100?1.f:0.f;
   const float wet=os.process(0,x,factor,[](float y){return y;});
   const float dry=os.alignDry(0,x,factor);
   if(std::abs(wet)>peak){peak=std::abs(wet);peakAt=n;}
   error+=(wet-dry)*(wet-dry);energy+=dry*dry;
  }
  require(peakAt==100+delay,"FIR impulse group delay is not the declared value");
  require(std::sqrt(error/energy)<.14,"FIR impulse passband mismatch");
  NecroDSP::ReviveEngine revive;revive.prepare(sr);revive.set(0.f,0,factor,0.f);
  NecroDSP::SampleDelay baseline;baseline.prepare(256);
  error=energy=0;
  for(int n=0;n<4096;++n){
   const float x=.25f*std::sin(juce::MathConstants<double>::twoPi*987.0*n/sr)
                 +.06f*std::sin(juce::MathConstants<double>::twoPi*1809.0*n/sr);
   float l=x,r=.8f*x,dl=l,dr=r;
   revive.process(l,r);
   baseline.process(dl,dr,2*delay);
   energy+=dl*dl+dr*dr;
   error+=(l-dl)*(l-dl)+(r-dr)*(r-dr);
  }
  require(energy>1e-2,"dry test signal vanished");
  require(std::sqrt(error/energy)<1e-6,"zero-amount REVIVE does not null with dry delay");
  std::cout<<factor<<"x: FIR="<<delay<<" samples; total REVIVE="<<2*delay<<" samples; dry null passed\n";
 }
 for(int factor:{2,4}){
  NecroDSP::Oversampler os;
  double wetE=0,dryE=0;
  for(int n=0;n<48000;++n){
   const float x=.3f*std::sin(juce::MathConstants<double>::twoPi*997*n/sr);
   const float y=os.process(0,x,factor,[](float v){return v;});
   const float dry=os.alignDry(0,x,factor);
   if(n>2000){wetE+=y*y;dryE+=dry*dry;}
  }
  require(std::abs(10*std::log10(wetE/dryE))<.05,"1-kHz FIR gain error too large");
 }
 // 10kHz overdriven sine: 3rd harmonic folds to 18kHz without oversampling.
 const auto alias=[&](int factor){
  NecroDSP::Oversampler os;
  double re=0,im=0;const int total=48000;
  for(int n=0;n<total;++n){
   const float x=.5f*std::sin(juce::MathConstants<double>::twoPi*10000.0*n/sr);
   const float y=os.process(0,x,factor,[](float v){return std::tanh(5.f*v);});
   if(n>2000){
    re+=y*std::sin(juce::MathConstants<double>::twoPi*18000.0*n/sr);
    im+=y*std::cos(juce::MathConstants<double>::twoPi*18000.0*n/sr);
   }
  }
  return std::hypot(re,im);
 };
 const auto rawAlias=alias(1);
 require(alias(2)<rawAlias*.01,"2x alias suppression less than 40 dB");
 require(alias(4)<rawAlias*.01,"4x alias suppression less than 40 dB");
 std::cout<<"2x/4x alias suppression exceeds 40 dB for the synthetic stress case\n";
}
