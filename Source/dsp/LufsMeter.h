#pragma once
#include "Biquad.h"
#include <array>
namespace NecroDSP {
// 100ms-block K-weighted streaming meter with 400ms/100ms overlapping
// integrated gating windows (-70 LUFS absolute and -10 LU relative). A
// fixed 0.1-LU histogram gives bounded memory/CPU without audio-thread heap use.
class LufsMeter {
public:
 void prepare(double sampleRate) noexcept {sr=sampleRate;blockSamples=juce::jmax(1,(int)std::round(sr*.1));
   for(auto& f:shelf)f.design(Biquad::Kind::highShelf,sr,1500.0,4.0,.7071067811865475);
   for(auto& f:hpf)f.design(Biquad::Kind::highpass,sr,38.0,0,.5);
   reset();}
 void reset() noexcept {for(auto& f:shelf)f.reset();for(auto& f:hpf)f.reset();window.fill(0);histCount.fill(0);histEnergy.fill(0);head=count=sampleCount=integratedTick=0;sampleEnergy=0;momentary=shortTerm=integrated=-100;}
 void process(float l,float r,int channels=2) noexcept {
    double a=hpf[0].process(0,shelf[0].process(0,l));double b=hpf[1].process(1,shelf[1].process(1,r));
    sampleEnergy+=a*a+(channels==1?0.0:b*b);
    if(++sampleCount<blockSamples)return;
    double energy=sampleEnergy/blockSamples;sampleEnergy=0;sampleCount=0;
    window[(size_t)head]=energy;head=(head+1)%30;count=juce::jmin(count+1,30);
    double m=0,s=0;for(int i=0;i<count;++i){const int p=(head+30-1-i)%30;if(i<4)m+=window[(size_t)p];s+=window[(size_t)p];}
    momentary=count>=4?toLufs(m/4.0):-100.;
    shortTerm=toLufs(s/count);
    // EBU R128: 400ms overlapping gating blocks with 100ms hop.
    if(count>=4){
      const double gatedEnergy=m/4.0,gatedLufs=toLufs(gatedEnergy);
      if(gatedLufs>-70.0) {
        const int bucket=juce::jlimit(0,1100,(int)std::floor((gatedLufs+70.0)*10.0));
        histCount[(size_t)bucket]++;
        histEnergy[(size_t)bucket]+=gatedEnergy;
      }
    }
    if(++integratedTick>=10){integratedTick=0;refreshIntegrated();}
 }
 float m()const noexcept{return (float)momentary;}float s()const noexcept{return (float)shortTerm;}float i()const noexcept{return (float)integrated;}
private:
 static double toLufs(double e) noexcept{return e<=1.e-14?-100.0:-.691+10.0*std::log10(e);}
 void refreshIntegrated() noexcept {
  double total=0;uint64_t n=0;for(int k=0;k<1101;++k){total+=histEnergy[(size_t)k];n+=histCount[(size_t)k];}
  if(!n)return;
  const double relative=toLufs(total/n)-10.;total=0;n=0;
  for(int k=0;k<1101;++k) if(k*0.1-70.>=relative) {total+=histEnergy[(size_t)k];n+=histCount[(size_t)k];}
  integrated=n?toLufs(total/n):-100.;
 }
 double sr=44100,sampleEnergy=0,momentary=-100,shortTerm=-100,integrated=-100;
 int blockSamples=4410,sampleCount=0,head=0,count=0,integratedTick=0;
 std::array<double,30>window{};std::array<uint64_t,1101>histCount{};std::array<double,1101>histEnergy{};
 std::array<Biquad,2>shelf,hpf;
};
}
