#include "../Source/dsp/LufsMeter.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
int main(int argc,char** argv){
 const int sr=argc>1?std::atoi(argv[1]):48000;
 const int type=argc>2?std::atoi(argv[2]):0;
 const int channels=argc>3?std::atoi(argv[3]):2;
 NecroDSP::LufsMeter meter;meter.prepare(sr);
 constexpr double pi=juce::MathConstants<double>::pi;
 for(int i=0;i<sr*6;++i){
  const double t=double(i)/sr;
  float l=0,r=0;
  if(type==0){l=.1f*std::sin(2*pi*1000*t);r=l;}
  else if(type==1){l=(t<2?.03f:.1f)*std::sin(2*pi*450*t);r=.8f*l;}
  else if(type==2){l=.11f*std::sin(2*pi*80*t)+.04f*std::sin(2*pi*5100*t);r=.9f*l;}
  else {l=.08f*std::sin(2*pi*900*t);r=.08f*std::sin(2*pi*1100*t);}
  meter.process(l,r,channels);
 }
 std::cout<<meter.i()<<" "<<meter.s()<<" "<<meter.m()<<"\n";
}
