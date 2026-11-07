#pragma once
#include "Biquad.h"
namespace NecroDSP {
class TransientRestore {
public:
 void prepare(double rate) noexcept {sr=rate;reset();}
 void reset() noexcept {fast=slow=0;}
 void set(float strength,int mode) noexcept {a=strength*(mode==2?1.0f:mode==0?.65f:.48f);}
 void advance(float l,float r) noexcept {
   double x=std::max(std::abs(l),std::abs(r)),kf=std::exp(-1.0/(sr*.004)),ks=std::exp(-1.0/(sr*.060));fast=kf*fast+(1-kf)*x;slow=ks*slow+(1-ks)*x;
   const double d=juce::jlimit(0.0,1.0,(fast-slow)/std::max(.12,slow));gain=(float)dbToGain(a*3.0*d);
 }
 float process(float v) const noexcept {return v*gain;}
private:double sr=44100,fast=0,slow=0;float a=0,gain=1;
};
}
