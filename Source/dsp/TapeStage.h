#pragma once
#include "Biquad.h"
#include "Oversampler.h"
namespace NecroDSP {
class TapeStage {
public:
 void prepare(double sampleRate) noexcept {sr=sampleRate;reset();}
 void reset() noexcept {os.reset();for(auto& q:dc)q.reset();for(auto& q:bump)q.reset();}
 void set(float amount,int mode,int factor) noexcept {
    a=amount;quality=factor;const double weight=mode==1?1.0:mode==0?.63:.8;
    for(auto& q:bump)q.design(Biquad::Kind::lowShelf,sr,78.0,amount*1.25*weight);
    for(auto& q:dc)q.design(Biquad::Kind::highpass,sr,17,0,0.707);
 }
 float process(int ch,float input) noexcept {
    auto x=(float)bump[ch].process(ch,input);
    const float drive=1.f+a*(1.55f);const float bias=.13f*a;
    x=os.process(ch,x,quality,[&](float s) noexcept {const float z=drive*s+bias;return std::tanh(z)-std::tanh(bias);});
    x/=std::max(.75f,drive*.78f);
    // Symmetric pre/post frequency shaping: the DC filter protects the host.
    const float processed=(float)dc[ch].process(ch,x);
    return input+(processed-input)*a;
 }
private: double sr=44100;float a=0;int quality=1;std::array<Biquad,2>dc,bump;Oversampler os;
};
}
