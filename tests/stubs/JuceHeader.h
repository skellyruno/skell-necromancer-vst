// Minimal compile-only JUCE math/SmoothedValue shim for DSP unit smoke test.
// This is NOT the JUCE SDK; do not use it to build the VST3 target.
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <vector>
#include <cstddef>
namespace juce {
template<typename T>constexpr T jlimit(T low,T high,T v)noexcept{return std::min(high,std::max(low,v));}
template<typename T>constexpr T jmax(T a,T b)noexcept{return a>b?a:b;}
template<typename T>constexpr T jmin(T a,T b)noexcept{return a<b?a:b;}
template<typename T>struct MathConstants{static constexpr T pi=(T)3.14159265358979323846;
 static constexpr T twoPi=(T)6.283185307179586476925286766559;};
namespace ValueSmoothingTypes {struct Linear{};}
template<class T,class Type>class SmoothedValue{
 T current{},target{},delta{};int remaining=0,ramp=1;
public:
 void reset(double sr,double seconds){ramp=std::max(1,(int)std::round(sr*seconds));remaining=0;}
 void setCurrentAndTargetValue(T value){current=target=value;delta=0;remaining=0;}
 void setTargetValue(T value){if(value==target)return;target=value;remaining=ramp;delta=(target-current)/remaining;}
 T getNextValue(){if(remaining>0){current+=delta;--remaining;if(!remaining)current=target;}return current;}
 T skip(int n){for(int i=0;i<n;++i)getNextValue();return current;}
};
}
