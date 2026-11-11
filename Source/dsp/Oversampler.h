#pragma once
#include <JuceHeader.h>
#include <array>
#include <cstddef>
#include <cmath>

namespace NecroDSP {
// Streaming 2x/4x FIR oversampler. Two 65-tap sparse half-band stages use
// symmetric linear-phase Kaiser-windowed sinc kernels (8.6 beta).
// Each complete 2x up/down stage has 32 base-rate samples of group delay;
// a second stage at twice the sample rate adds 16 more base-rate samples.
// Filtering/latency is independent of the waveshaper's parameters.
//
// Implementation is the exact polyphase form of that filter: the half-band
// kernel is zero on even taps except the 0.5 centre, so the up-sampler's even
// phase is a pure 16-sample delay and the odd phase is a 32-tap FIR that is
// folded by kernel symmetry into 16 multiplies. Same for the down-sampler.
// No modulo indexing, no heap use, fixed-size state.
class Oversampler {
public:
 static constexpr int latencyForFactor(int factor) noexcept {
  return factor == 2 ? 32 : factor == 4 ? 48 : 0;
 }
 void reset() noexcept { for(auto& c:channels)c=Channel{}; activeFactor=1; }
 template<class Fn>
 float process(int ch,float x,int factor,Fn&& nonlinearity) noexcept {
  factor=(factor==2 || factor==4)?factor:1;
  if(factor!=activeFactor){reset();activeFactor=factor;}
  auto& c=channels[(size_t)ch];
  if(factor==1)return nonlinearity(x);
  float a0=0,a1=0;
  c.first.up(x,a0,a1);
  if(factor==2)return c.first.down(nonlinearity(a0),nonlinearity(a1));
  float b0=0,b1=0;
  c.second.up(a0,b0,b1);
  const float d0=c.second.down(nonlinearity(b0),nonlinearity(b1));
  c.second.up(a1,b0,b1);
  const float d1=c.second.down(nonlinearity(b0),nonlinearity(b1));
  return c.first.down(d0,d1);
 }
 // Matches the exact FIR group delay in the parallel unprocessed signal.
 float alignDry(int ch,float x,int factor) noexcept {
  if(factor==1)return x;
  const size_t delay=(size_t)latencyForFactor(factor);
  auto& c=channels[(size_t)ch];
  const size_t head=c.dryHead;
  const float result=c.dry[(head+drySize-delay)&(drySize-1)];
  c.dry[head]=x;
  c.dryHead=(head+1)&(drySize-1);
  return result;
 }
private:
 static constexpr size_t drySize=128; // power of two (mask indexing)
 // 32-sample history, newest first, mirrored so reads never wrap.
 struct History {
  std::array<float,64> h{};
  int pos=0;
  void push(float v) noexcept {pos=(pos+31)&31;h[(size_t)pos]=v;h[(size_t)pos+32]=v;}
  float operator[](int j) const noexcept {return h[(size_t)(pos+j)];}
  // Odd-tap half-band phase, folded by symmetry: taps[2j+1]==taps[63-2j].
  double fir() const noexcept {
   const float* p=h.data()+pos;
   double s=0;
   for(int j=0;j<16;++j)s+=oddTaps[(size_t)j]*((double)p[j]+(double)p[31-j]);
   return s;
  }
 };
 struct Stage {
  History upHist,downHist;
  std::array<float,16> firstDelay{};
  int fd=0;
  void up(float x,float& first,float& second) noexcept {
   upHist.push(x);
   first=upHist[16];                 // 2*0.5*x[n-16], exact
   second=(float)(2.0*upHist.fir()); // odd phase
  }
  float down(float first,float second) noexcept {
   const double s=downHist.fir();    // uses second[n-1-j]
   const float delayed=firstDelay[(size_t)fd];
   firstDelay[(size_t)fd]=first;fd=(fd+1)&15;
   downHist.push(second);
   return (float)(.5*(double)delayed+s);
  }
 };
 struct Channel {
  Stage first,second;
  std::array<float,drySize>dry{};
  size_t dryHead=0;
 };
 std::array<Channel,2>channels{};
 int activeFactor=1;
 inline static constexpr std::array<double,16> oddTaps{{-3.427510634031215e-05, 1.210071895754952e-04, -3.041894932959578e-04, 6.431772274800812e-04, -1.216736827069430e-03, 2.125453996832458e-03, -3.495197046943546e-03, 5.483958663767519e-03, -8.296954933302179e-03, 1.222058907447923e-02, -1.770057452010787e-02, 2.553319021828257e-02, -3.739441334250908e-02, 5.764838564795769e-02, -1.023946893736637e-01, 3.170612686248571e-01}};
};
}
