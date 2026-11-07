#include "../Source/dsp/TruePeakLimiter.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <random>
int main(int argc,char**argv){
 const int sr=argc>2?std::atoi(argv[2]):48000,n=sr;
 const float drive=argc>3?std::atof(argv[3]):12.f,limitDb=argc>4?std::atof(argv[4]):-1.f;const int typ=argc>5?std::atoi(argv[5]):0;
 NecroDSP::TruePeakLimiter lim;lim.prepare(sr,512);lim.set(true,drive,limitDb,false);
 std::mt19937 rng(41);std::normal_distribution<float>norm(0,1);
 std::vector<float>output;output.reserve(n);
 float max=0;
 for(int k=0;k<n;++k){
  float x;
  if(typ==0)x=.9f*std::sin(2*juce::MathConstants<double>::pi*997*k/sr)+.15f*norm(rng);
  else if(typ==1)x=.98f*std::sin(2*juce::MathConstants<double>::pi*(sr*.42)*k/sr);
  else if(typ==2)x=norm(rng)*.44f;
  else x=.4f*std::sin(2*juce::MathConstants<double>::pi*37*k/sr)+.6f*std::sin(2*juce::MathConstants<double>::pi*823*k/sr);
  if(k%4000==0)x=1.4f;
  float l=x,r=x;lim.process(l,r);max=std::max(max,std::abs(l));output.push_back(l);
 }
 std::ofstream file(argc>1?argv[1]:"/tmp/necro_output.bin",std::ios::binary);
 file.write(reinterpret_cast<char*>(output.data()),static_cast<std::streamsize>(output.size()*sizeof(float)));
 std::cout<<"limiter max sample "<<max<<" (target "<<std::pow(10.,limitDb/20.)<<")\n";
}
