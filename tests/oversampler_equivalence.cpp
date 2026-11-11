// Verifies the polyphase Oversampler matches the frozen direct-form reference
// (tests/reference/OversamplerReference.h) and reports relative CPU cost.
#include "../Source/dsp/Oversampler.h"
#include "reference/OversamplerReference.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
static void require(bool ok,const char* why){if(!ok){std::cerr<<"Oversampler equivalence failure: "<<why<<"\n";std::exit(4);}}
int main(){
 std::mt19937 rng(1234);std::uniform_real_distribution<float> noise(-1.f,1.f);
 const auto nl=[](float v){return std::tanh(3.f*v);};
 for(int factor:{2,4}){
  NecroDSP::Oversampler fast;NecroRef::Oversampler ref;
  double worst=0;
  for(int n=0;n<200000;++n){
   // noise bursts + sines + hard steps, both channels, mid-run factor-neutral
   const float x=(n%5000<1000?.9f*noise(rng):.5f*std::sin(.037f*n))+(n%7919==0?1.f:0.f);
   for(int ch=0;ch<2;++ch){
    const float a=fast.process(ch,ch?-x:x,factor,nl),b=ref.process(ch,ch?-x:x,factor,nl);
    worst=std::max(worst,(double)std::abs(a-b));
    require(fast.alignDry(ch,x,factor)==ref.alignDry(ch,x,factor),"dry alignment differs");
   }
  }
  std::cout<<factor<<"x max |new-ref| = "<<worst<<"\n";
  require(worst<2e-6,"output deviates from reference");
 }
 // Factor switching must behave identically (reset semantics).
 {NecroDSP::Oversampler a;NecroRef::Oversampler b;double worst=0;
  for(int n=0;n<60000;++n){const int f=(n/10000)%3==0?1:((n/10000)%3==1?2:4);const float x=.7f*std::sin(.05f*n);
   worst=std::max(worst,(double)std::abs(a.process(0,x,f,nl)-b.process(0,x,f,nl)));}
  require(worst<2e-6,"factor switching deviates");std::cout<<"switching max diff = "<<worst<<"\n";}
 // Informational benchmark (not asserted: CI timing is noisy).
 const auto bench=[&](auto& os,int factor){
  volatile float sink=0;const auto t0=std::chrono::steady_clock::now();
  for(int n=0;n<2000000;++n){sink=sink+os.process(0,.4f*std::sin(.01f*(float)(n&1023)),factor,nl);}
  return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();};
 for(int factor:{2,4}){NecroDSP::Oversampler f;NecroRef::Oversampler r;
  const double tr=bench(r,factor),tf=bench(f,factor);
  std::cout<<factor<<"x bench (2M samples): reference "<<tr<<" ms, polyphase "<<tf<<" ms, speedup "<<tr/tf<<"x\n";}
 std::cout<<"Oversampler polyphase equivalence passed\n";
}
