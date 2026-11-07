#pragma once
#include "Biquad.h"
#include <array>
#include <vector>
namespace NecroDSP {
class TruePeakLimiter {
public:
 void prepare(double rate,int maxBlock) {
   sr=rate;lookahead=juce::jmax(1,(int)std::ceil(sr*.0025));
   // Keep wet and dry signals in the same lookahead ring so limiter bypass is latency matched.
   ring.assign((size_t)(lookahead+juce::jmax(maxBlock,32)+32)*4,0.f);ringSize=(int)ring.size()/4;queue.assign((size_t)lookahead+64,{});
   history.fill({}); buildKernel();reset();
 }
 void reset() noexcept {std::fill(ring.begin(),ring.end(),0.f);history.fill({});head=0;env=1.0;gr=0;qhead=qtail=0;frame=0;enabledBlend=on?1.f:0.f;}
 int latencySamples() const noexcept {return lookahead;}
 void set(bool enabled,float pushDb,float ceilingDb,bool softclip) noexcept {on=enabled;push=dbToGain(pushDb);ceiling=dbToGain(ceilingDb);clip=softclip;}
 void process(float& l,float& r) noexcept {
  const float dryL=l,dryR=r;
  float x[2]={(float)(l*push),(float)(r*push)};
  if(clip)for(auto& v:x)v=std::tanh(v*.87f)/std::tanh(.87f);
  for(int c=0;c<2;++c) {
      for(int i=15;i>0;--i)history[(size_t)c][(size_t)i]=history[(size_t)c][(size_t)i-1];
      history[(size_t)c][0]=x[c];
  }
  double peak=1e-10;
  for(int c=0;c<2;++c){peak=std::max(peak,(double)std::abs(x[c]));for(int phase=0;phase<4;++phase){double y=0;for(int k=0;k<16;++k)y+=kernel[phase][k]*history[c][k];peak=std::max(peak,std::abs(y));}}
  // Allocation-free monotonic maximum queue: amortized O(1) per sample.
  // Protect the delayed output with a future-peak window (lookahead + FIR lag).
  const auto cap=queue.size();
  while(qtail>qhead&&queue[(qtail-1)%cap].peak<=peak)--qtail;
  queue[qtail%cap]={frame,peak};++qtail;
  while(qtail>qhead&&queue[qhead%cap].index+static_cast<uint64_t>(lookahead+9)<frame)++qhead;
  const double futurePeak=queue[qhead%cap].peak;
  ++frame;
  const double target=std::min(1.0,ceiling*0.915/futurePeak); // conservative 4x reconstruction margin
  if(target<env)env=target; else {const double a=std::exp(-1.0/(sr*.13));env=a*env+(1-a)*target;}
  gr=(float)-gainToDb(env);
  const int rd=(head+ringSize-lookahead)%ringSize;
  const size_t ri=(size_t)rd*4,wi=(size_t)head*4;
  const float delayedWetL=ring[ri],delayedWetR=ring[ri+1];
  const float delayedDryL=ring[ri+2],delayedDryR=ring[ri+3];
  ring[wi]=x[0];ring[wi+1]=x[1];
  ring[wi+2]=dryL;ring[wi+3]=dryR;
  head=(head+1)%ringSize;
  // Smooth enabling/disabling over 12 ms; preserve exactly the same lookahead.
  const float delta=(on?1.f:0.f)-enabledBlend;
  const float step=(float)(1.0/(sr*.012));
  enabledBlend+=juce::jlimit(-step,step,delta);
  l=delayedDryL+(delayedWetL*(float)env-delayedDryL)*enabledBlend;
  r=delayedDryR+(delayedWetR*(float)env-delayedDryR)*enabledBlend;
 }
 float reduction() const noexcept {return on?gr:0.f;}
private:
 void buildKernel() noexcept {
  // Windowed sinc interpolation for 4x inter-sample peak estimation.
  // The fractional position is centered ~7 input samples in the past.
  constexpr double pi=juce::MathConstants<double>::pi;
  for(int phase=0;phase<4;++phase){double sum=0;for(int i=0;i<16;++i){const double t=i-7.0-phase/4.0;const double sinc=std::abs(t)<1e-12?1.0:std::sin(pi*t)/(pi*t);const double win=.42-.5*std::cos(2*pi*i/15)+.08*std::cos(4*pi*i/15);kernel[phase][i]=sinc*win;sum+=kernel[phase][i];}for(auto& k:kernel[phase])k/=sum;}
 }
 double sr=44100,push=1,ceiling=dbToGain(-.3),env=1;int head=0,ringSize=1,lookahead=128;bool on=true,clip=false;float gr=0,enabledBlend=1.f;
 struct Peak{uint64_t index=0;double peak=0;};
 std::vector<float>ring;std::vector<Peak>queue;size_t qhead=0,qtail=0;uint64_t frame=0;std::array<std::array<float,16>,2>history{};std::array<std::array<double,16>,4>kernel{};
};
// Mono/stereo transparent delay, allocated only in prepareToPlay.
class SampleDelay {
public:
 void prepare(int maximumSamples) {buffer.assign((size_t)(juce::jmax(2,maximumSamples)+4)*2,0);length=(int)buffer.size()/2;head=0;}
 void reset() noexcept {std::fill(buffer.begin(),buffer.end(),0);head=0;}
 void process(float& l,float& r,int delay) noexcept {
     delay=juce::jlimit(0,length-1,delay);int rd=(head+length-delay)%length;
     float a=delay?buffer[(size_t)rd*2]:l,b=delay?buffer[(size_t)rd*2+1]:r;
     buffer[(size_t)head*2]=l;buffer[(size_t)head*2+1]=r;head=(head+1)%length;l=a;r=b;
 }
private:std::vector<float>buffer;int length=4,head=0;
};
}
