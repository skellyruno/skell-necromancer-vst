#pragma once
#include "LufsMeter.h"
#include <JuceHeader.h>
#include <array>
#include <cmath>
namespace NecroDSP {
struct Autopsy {
 float rmsDb=-100,crest=0,low=0,lowMid=0,mid=0,high=0,air=0,sideRatio=0;
 bool dull=false,thin=false,boxy=false,muddy=false,narrow=false,overDynamic=false;
};
class AutoAnalyzer {
public:
 enum State {idle=0,listening=1,needsReplay=2,verifying=3,complete=4,abort=5};
 void prepare(double sampleRate) noexcept {sr=sampleRate;framesPerBucket=juce::jmax(1,(int)std::round(sr*.10));
  constexpr double cutoff[4]={110,450,2400,8000};
  for(int i=0;i<4;++i)onePoleAlpha[(size_t)i]=1.0-std::exp(-juce::MathConstants<double>::twoPi*cutoff[i]/sr);
  verifyLoudness.prepare(sampleRate);
  reset();}
 void reset() noexcept {state.store(idle);sampleInBucket=0;bucket=0;filled=totalBuckets=0;windows.fill({});sum.fill(0);current.fill(0);best=Autopsy{};bestEnergy=0;totalFrames=0;onePole.fill(0);}
 void start() noexcept {reset();state.store(listening);}
 void startVerification() noexcept {sampleInBucket=0;bucket=0;verifyEnergy=0;verifyN=0;totalFrames=0;verifyLoudness.reset();state.store(verifying);}
 int getState()const noexcept {return state.load(std::memory_order_acquire);}
 Autopsy getResult()const noexcept {return best;}
 float getReplayLufs()const noexcept {return verifyLoudness.s();}
 void push(float l,float r) noexcept {
  const int st=state.load(std::memory_order_relaxed);if(st!=listening&&st!=verifying)return;
  const double center=.5*(l+r);const double side=.5*(l-r);
  if(st==verifying) {verifyLoudness.process(l,r,2);++verifyN;if(++totalFrames>=sr*3.0)state.store(complete,std::memory_order_release);return;}
  const double x=center;double e[8]{};
  for(int i=0;i<4;++i){onePole[(size_t)i]+=onePoleAlpha[(size_t)i]*(x-onePole[(size_t)i]);}
  e[0]=onePole[0]*onePole[0];e[1]=(onePole[1]-onePole[0])*(onePole[1]-onePole[0]);
  e[2]=(onePole[2]-onePole[1])*(onePole[2]-onePole[1]);e[3]=(onePole[3]-onePole[2])*(onePole[3]-onePole[2]);e[4]=(x-onePole[3])*(x-onePole[3]);
  e[5]=center*center+side*side;e[6]=side*side;e[7]=std::max(std::abs(l),std::abs(r));
  for(int i=0;i<7;++i)current[(size_t)i]+=e[i];
  current[7]=std::max(current[7],e[7]);
  if(++sampleInBucket<framesPerBucket)return;
  sampleInBucket=0;std::array<double,8> previous=windows[(size_t)bucket];
  for(int i=0;i<7;++i){const double v=current[(size_t)i]/framesPerBucket;sum[(size_t)i]+=v-previous[(size_t)i];windows[(size_t)bucket][(size_t)i]=v;}
  windows[(size_t)bucket][7]=current[7];current.fill(0);bucket=(bucket+1)%30;filled=juce::jmin(filled+1,30);
  if(filled>=30){ // 3s sliding window and highest-energy selection.
    double power=sum[5]/30.0;
    if(power>bestEnergy){bestEnergy=power;best.rmsDb=(float)(10*std::log10(std::max(power,1e-12)));
      best.low=(float)(sum[0]/30);best.lowMid=(float)(sum[1]/30);best.mid=(float)(sum[2]/30);
      best.high=(float)(sum[3]/30);best.air=(float)(sum[4]/30);
      best.sideRatio=(float)std::sqrt(std::max(0.,sum[6])/std::max(1e-12,sum[5]));
      double peak=0;for(const auto& win:windows)peak=std::max(peak,win[7]);
      best.crest=(float)(20*std::log10(std::max(peak,1.e-9))-best.rmsDb);
    }
  }
  if(++totalBuckets>=120){
   if(best.rmsDb<-58||bestEnergy<1e-7){state.store(abort,std::memory_order_release);return;}
   const auto hf=best.high+best.air,mid=best.lowMid+best.mid;
   best.dull=hf<mid*.085f;best.thin=best.low+best.lowMid<best.mid*.3f;
   best.boxy=best.lowMid>best.mid*1.8f;best.muddy=best.low>best.mid*1.5f;
   best.narrow=best.sideRatio<.06f;best.overDynamic=best.crest>16;
   state.store(needsReplay,std::memory_order_release);
  }
 }
private:
 double sr=44100,bestEnergy=0,verifyEnergy=0;int framesPerBucket=4410,sampleInBucket=0,bucket=0,filled=0,totalBuckets=0,totalFrames=0,verifyN=0;
 std::atomic<int>state{idle};Autopsy best;LufsMeter verifyLoudness;
 std::array<double,4>onePole{},onePoleAlpha{};std::array<double,8>sum{},current{};
 std::array<std::array<double,8>,30>windows{};
};
}
