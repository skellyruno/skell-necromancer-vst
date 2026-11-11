#pragma once
#include <JuceHeader.h>
#include <array>
#include <cstddef>
#include <cmath>

namespace NecroRef {
// Streaming 2x/4x FIR oversampler. Two 65-tap sparse half-band stages use
// symmetric linear-phase Kaiser-windowed sinc kernels (8.6 beta).
// Each complete 2x up/down stage has 32 base-rate samples of group delay;
// a second stage at twice the sample rate adds 16 more base-rate samples.
// Filtering/latency is independent of the waveshaper's parameters.
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
  const float result=c.dry[(head+drySize-delay)%drySize];
  c.dry[head]=x;
  c.dryHead=(head+1)%drySize;
  return result;
 }
private:
 static constexpr size_t drySize=128;
 struct Filter {
  std::array<float,65> hist{};
  int head=0;
  void push(float value) noexcept {hist[(size_t)head]=value;head=(head+1)%65;}
  float eval() const noexcept {
   const auto at=[this](int k) noexcept -> float {
    const int i=head+64-k;
    return hist[(size_t)(i>=65?i-65:i)];
   };
   double result=.5*at(32);
   for(int k=1;k<65;k+=2)result+=taps[(size_t)k]*at(k);
   return (float)result;
  }
 };
 struct Stage {
  Filter upFilter,downFilter;
  void up(float x,float& first,float& second) noexcept {
   upFilter.push(x); first=2.f*upFilter.eval();
   upFilter.push(0.f);second=2.f*upFilter.eval();
  }
  float down(float first,float second) noexcept {
   downFilter.push(first);const float result=downFilter.eval();
   downFilter.push(second);return result;
  }
 };
 struct Channel {
  Stage first,second;
  std::array<float,drySize>dry{};
  size_t dryHead=0;
 };
 std::array<Channel,2>channels{};
 int activeFactor=1;
 inline static constexpr std::array<double,65> taps{{0.000000000000000e+0, -3.427510634031215e-5, 0.000000000000000e+0, 1.210071895754952e-4, 0.000000000000000e+0, -3.041894932959578e-4, 0.000000000000000e+0, 6.431772274800812e-4, 0.000000000000000e+0, -1.216736827069430e-3, 0.000000000000000e+0, 2.125453996832458e-3, 0.000000000000000e+0, -3.495197046943546e-3, 0.000000000000000e+0, 5.483958663767519e-3, 0.000000000000000e+0, -8.296954933302179e-3, 0.000000000000000e+0, 1.222058907447923e-2, 0.000000000000000e+0, -1.770057452010787e-2, 0.000000000000000e+0, 2.553319021828257e-2, 0.000000000000000e+0, -3.739441334250908e-2, 0.000000000000000e+0, 5.764838564795769e-2, 0.000000000000000e+0, -1.023946893736637e-1, 0.000000000000000e+0, 3.170612686248571e-1, 5.000000000000000e-1, 3.170612686248571e-1, 0.000000000000000e+0, -1.023946893736637e-1, 0.000000000000000e+0, 5.764838564795769e-2, 0.000000000000000e+0, -3.739441334250908e-2, 0.000000000000000e+0, 2.553319021828257e-2, 0.000000000000000e+0, -1.770057452010787e-2, 0.000000000000000e+0, 1.222058907447923e-2, 0.000000000000000e+0, -8.296954933302179e-3, 0.000000000000000e+0, 5.483958663767519e-3, 0.000000000000000e+0, -3.495197046943546e-3, 0.000000000000000e+0, 2.125453996832458e-3, 0.000000000000000e+0, -1.216736827069430e-3, 0.000000000000000e+0, 6.431772274800812e-4, 0.000000000000000e+0, -3.041894932959578e-4, 0.000000000000000e+0, 1.210071895754952e-4, 0.000000000000000e+0, -3.427510634031215e-5, 0.000000000000000e+0}};
};
}
