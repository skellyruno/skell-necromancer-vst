#pragma once
#include "Biquad.h"
namespace NecroDSP {
class GlueStage {
public:
 void prepare(double sr) noexcept {rate=sr;attackCoeff=std::exp(-1.0/(sr*.013));shortReleaseCoeff=std::exp(-1.0/(sr*.12));longReleaseCoeff=std::exp(-1.0/(sr*.50));hp.design(Biquad::Kind::highpass,sr,90,0,.707);reset();}
 void reset() noexcept {envFast=envSlow=0;gain=1;hp.reset();}
 void set(float amount,int mode) noexcept {a=amount;maxGr=mode==0?5.0:mode==1?3.0:4.0;ratio=1.0+0.9*a;}
 // Feed-forward detector using stereo-linked peak of high-passed signals.
 void advance(float l,float r) noexcept {
  double detect=std::max(std::abs(hp.process(0,l)),std::abs(hp.process(1,r)));
  const auto attack=attackCoeff,shortR=shortReleaseCoeff,longR=longReleaseCoeff;
  const auto k=detect>envFast?attack:shortR;envFast=k*envFast+(1-k)*detect;
  envSlow=longR*envSlow+(1-longR)*detect;
  const auto detector=std::max(envFast,envSlow*.65),db=gainToDb(detector);
  const double threshold=-18.0-(a*4),knee=6.0;
  double over=db-threshold;double compressed=over<=-knee/2?0:(over>=knee/2?over*(1.-1./ratio):((over+knee/2)*(over+knee/2)/(2*knee))*(1.-1./ratio));
  compressed=juce::jlimit(0.0,(double)maxGr,compressed);
  const auto target=dbToGain(-compressed), release=target<gain?attack:shortR;
  gain=release*gain+(1-release)*target;
 }
 float process(float x) const noexcept {return (float)(x*gain);}
 float getGainReduction() const noexcept {return (float)-gainToDb(gain);}
private:double rate=44100,envFast=0,envSlow=0,gain=1;
 double attackCoeff=.998,shortReleaseCoeff=.9998,longReleaseCoeff=.99995;float a=0,ratio=1,maxGr=5;Biquad hp;
};
}
