// Exact old/new TapeStage stereo parity across mode, rate and oversampling.
#include "../Source/dsp/TapeStage.h"
#include "reference/TapeStageReference.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
int main() {
 double worst=0;
 uint32_t seed=7079;
 for(double sr:{44100.,48000.,96000.})for(int mode=0;mode<3;++mode)
 for(int factor:{1,2,4}) {
  NecroDSP::TapeStage newer;NecroDSP::TapeStageReference old;
  newer.prepare(sr);old.prepare(sr);
  for(int n=0;n<32000;++n) {
   if(n%16==0) {
    const float amount=(n%8192<1024)?0.f:(n%8192<2048)?.27f:(n%8192<4096)?.65f:1.f;
    newer.set(amount,mode,factor);old.set(amount,mode,factor);
   }
   seed=1664525u*seed+1013904223u;
   const float noise=float((seed>>8)&65535u)/65535.f-.5f;
   float l=.28f*std::sin(juce::MathConstants<double>::twoPi*770.0*n/sr)+noise*.08f;
   float r=.35f*std::sin(juce::MathConstants<double>::twoPi*2221.0*n/sr)-noise*.07f;
   if(n%2009==0){l+=.6f;r-=.4f;}
   float lr=l,rr=r;
   l=newer.process(0,l);r=newer.process(1,r);
   lr=old.process(0,lr);rr=old.process(1,rr);
   if(!std::isfinite(l)||!std::isfinite(r))return 1;
   worst=std::max(worst,(double)std::max(std::abs(l-lr),std::abs(r-rr)));
  }
  if(worst>3.0e-7) {
   std::cerr<<"Tape output parity failed at "<<sr<<"/"<<mode<<"/"<<factor
            <<": "<<worst<<"\n";return 2;
  }
 }
 std::cout<<"PASS: exact old-vs-new TapeStage at 44.1/48/96k, all 3 modes and 1/2/4x, worst "<<worst<<"\n";
}
