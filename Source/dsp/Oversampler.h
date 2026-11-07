#pragma once
#include <JuceHeader.h>
namespace NecroDSP {
// Streaming interpolation/decimation utility for waveshapers. Quality is an
// approximation, not a polyphase mastering-grade anti-aliasing filter.
class Oversampler {
public:
 void reset() noexcept {for(auto& c:channels)c=Channel{};}
 template<class Fn> float process(int channel,float input,int factor,Fn&& fn) noexcept {
   auto& c=channels[(size_t)channel];factor=juce::jlimit(1,4,factor);
   if(factor==1) {c.previous=input;return fn(input);}
   double sum=0;for(int i=1;i<=factor;++i) {
       float interpolated=c.previous+(input-c.previous)*((float)i/factor);
       float shaped=fn(interpolated);
       // 2-pole smoothing at the higher rate suppresses some foldback.
       const float a=1.0f/(factor*.67f);c.lp1+=a*(shaped-c.lp1);c.lp2+=a*(c.lp1-c.lp2);sum+=c.lp2;
   }
   c.previous=input;return (float)(sum/factor);
 }
private:struct Channel {float previous=0,lp1=0,lp2=0;};std::array<Channel,2>channels;
};
}
