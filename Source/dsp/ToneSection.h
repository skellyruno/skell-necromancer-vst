#pragma once
#include "Biquad.h"
namespace NecroDSP {
class ToneSection {
public:
 void prepare(double rate) noexcept {sr=rate; reset();}
 void reset() noexcept {for(auto& f:filters)f.reset();for(auto& f:midFilters)f.reset();rangeBlend.reset(sr,.025);rangeBlend.setCurrentAndTargetValue(0);}
 void set(float lowDb,float midDb,int range,float highDb,float airDb) noexcept {
  // Designs are updated per block, never per sample.
  filters[0].design(Biquad::Kind::lowShelf,sr,100,lowDb);
  filters[1].design(Biquad::Kind::highShelf,sr,8000,highDb);
  filters[2].design(Biquad::Kind::highShelf,sr,15000,airDb);
  const double hz[3]={400,1600,4000};for(int i=0;i<3;++i)midFilters[i].design(Biquad::Kind::peak,sr,hz[i],midDb,0.62);
  rangeBlend.setTargetValue(static_cast<float>(juce::jlimit(0,2,range)));
 }
 float nextRangeValue() noexcept {return rangeBlend.getNextValue();}
 float process(int ch,float x,float range) noexcept {
  auto y=filters[0].process(ch,x);
  double mids[3];for(int i=0;i<3;++i)mids[i]=midFilters[i].process(ch,y);
  const double r=range;const int i=juce::jlimit(0,1,(int)r);const double t=r-i;
  y=mids[i]+(mids[i+1]-mids[i])*t;
  return (float)filters[2].process(ch,filters[1].process(ch,y));
 }
private:
 double sr=44100;std::array<Biquad,3> filters;std::array<Biquad,3> midFilters;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> rangeBlend;
};
}
