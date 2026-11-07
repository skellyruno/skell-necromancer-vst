#include "../Source/dsp/ReviveEngine.h"
#include "../Source/dsp/ToneSection.h"
#include "../Source/dsp/TruePeakLimiter.h"
#include "../Source/dsp/LufsMeter.h"
#include "../Source/dsp/AutoAnalyzer.h"
#include <iostream>
#include <cstdlib>
int main(){
 const auto require=[](bool valid,const char* message){if(!valid){std::cerr<<"DSP regression: "<<message<<"\n";std::exit(2);}};
 NecroDSP::ReviveEngine revive;revive.prepare(48000);revive.set(.65f,0,4,.55f);
 NecroDSP::ToneSection tone;tone.prepare(48000);tone.set(1.5f,-1,0,2.f,1.f);
 NecroDSP::TruePeakLimiter limiter;limiter.prepare(48000,512);limiter.set(true,6,-.3f,true);
 NecroDSP::LufsMeter loud;loud.prepare(48000);
 NecroDSP::AutoAnalyzer autoAnalyzer;autoAnalyzer.prepare(48000);autoAnalyzer.start();
 float l=0,r=0;
 for(int n=0;n<48000*13;++n){
  l=r=.03f*std::sin(2*juce::MathConstants<double>::pi*440*n/48000);
  autoAnalyzer.push(l,r);revive.process(l,r);
  const float range=tone.nextRangeValue();l=tone.process(0,l,range);r=tone.process(1,r,range);
  limiter.process(l,r);loud.process(l,r);
  require(std::isfinite(l)&&std::isfinite(r),"nonfinite audio output");
 }
 std::cout<<"DSP smoke OK | GR="<<limiter.reduction()<<" | LUFS="<<loud.s()<<" | auto state="<<autoAnalyzer.getState()<<"\n";
 require(autoAnalyzer.getState()==NecroDSP::AutoAnalyzer::needsReplay,"AUTO first pass did not finish");
 // Second AUTO pass must measure a processed (not raw) stereo program via K weighting.
 autoAnalyzer.startVerification();
 for(int n=0;n<48000*3;++n) {
   const float x=.12f*std::sin(2*juce::MathConstants<double>::pi*820*n/48000);
   autoAnalyzer.push(x,x*.88f);
 }
 require(autoAnalyzer.getState()==NecroDSP::AutoAnalyzer::complete,"AUTO replay verification incomplete");
 const float verifyLufs=autoAnalyzer.getReplayLufs();
 require(std::isfinite(verifyLufs) && verifyLufs>-55.f && verifyLufs<0.f,"AUTO verification LUFS invalid");
 std::cout<<"AUTO verification short-term LUFS="<<verifyLufs<<"\n";
 // Limiter ON/OFF fades should not produce infinities or reset its lookahead ring.
 limiter.set(false,6.f,-.3f,false);
 for(int n=0;n<48000/5;++n){
   l=.2f*std::sin(2*juce::MathConstants<double>::pi*1200*n/48000);
   r=l;limiter.process(l,r);require(std::isfinite(l)&&std::isfinite(r),"nonfinite limiter fade output");
 }
 return 0;
}
