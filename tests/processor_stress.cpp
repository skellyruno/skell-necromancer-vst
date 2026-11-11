// End-to-end stress test of the real SkellNecromancerAudioProcessor (headless). Build with -DSKELL_BUILD_PROCESSOR_TESTS=ON.
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <chrono>
#include <cmath>
#include <iostream>
using namespace juce;
static int fails=0;
static void check(bool ok,const String&w){std::cout<<(ok?"PASS ":"FAIL ")<<w<<std::endl;fails+=!ok;}
static void setP(SkellNecromancerAudioProcessor&p,const char*id,float v){auto*q=p.apvts.getParameter(id);q->setValueNotifyingHost(q->convertTo0to1(v));}
struct RunStats{bool finite=true;float peak=0;int worstIdx=-1;};
// Fill with a loud, broadband test signal and run in blocks of `blk`.
static RunStats run(SkellNecromancerAudioProcessor&p,int ch,double sr,int blk,int total,float gain,int&pos,uint32_t&seed,int warmup=0){
 RunStats r;AudioBuffer<float> b(ch,blk);MidiBuffer m;
 for(int done=0;done<total;done+=blk){
  for(int c=0;c<ch;++c)for(int n=0;n<blk;++n){
   seed=seed*1664525u+1013904223u;const float nz=((seed>>9)*(1.f/4194304.f))-1.f;
   const float t=(float)(pos+n)/(float)sr;
   float v=.55f*std::sin(MathConstants<float>::twoPi*(180.f+60.f*c)*t)+.25f*std::sin(MathConstants<float>::twoPi*5200.f*t)+.2f*nz;
   if(((pos+n)/(int)(sr*.35))%2==0)v*=1.8f;               // loud, transient-ish bursts
   b.setSample(c,n,v*gain);}
  pos+=blk;p.processBlock(b,m);
  if(done>=warmup)for(int c=0;c<ch;++c)for(int n=0;n<blk;++n){float v=b.getSample(c,n);
    if(!std::isfinite(v)){r.finite=false;}else if(std::abs(v)>r.peak){r.peak=std::abs(v);}}
 }
 return r;}
int main(int,char**){
 ScopedJuceInitialiser_GUI init;
 const float ceilLin[]={std::pow(10.f,-.1f/20),std::pow(10.f,-.3f/20),std::pow(10.f,-1.f/20)};
 // ---- T1: finite + ceiling across modes / oversampling / sample rates / block sizes (prepared at 512) ----
 {int bad=0,count=0;
  for(double sr:{44100.,48000.,96000.})for(int os=0;os<3;++os)for(int mode=0;mode<3;++mode)for(int blk:{1,37,64,512,4096}){
   SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,sr,512);p.prepareToPlay(sr,512);
   setP(p,"oversampling",(float)os);setP(p,"mode",(float)mode);setP(p,"reanimate",100);setP(p,"push",12);setP(p,"ceiling",(float)(mode%3));setP(p,"high",6);setP(p,"hfAdjust",6);setP(p,"low",6);
   int pos=0;uint32_t seed=7;
   auto r=run(p,2,sr,blk,(int)(sr*1.0)/blk*blk,1.f,pos,seed,(int)(sr*.2)/blk*blk);
   ++count;const bool ok=r.finite&&r.peak<=ceilLin[mode%3]+1e-4f;
   if(!ok){++bad;std::cout<<"  bad: sr="<<sr<<" os="<<os<<" mode="<<mode<<" blk="<<blk<<" finite="<<r.finite<<" peak="<<r.peak<<" ceil="<<ceilLin[mode%3]<<std::endl;}
  }
  check(bad==0,"T1 finite output and sample-peak <= ceiling ("+String(count-bad)+"/"+String(count)+" configs)");}
 // ---- T2: NaN / Inf input must not poison state ----
 {SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,48000,512);p.prepareToPlay(48000,512);setP(p,"oversampling",2);
  int pos=0;uint32_t seed=3;run(p,2,48000,512,48000/2,1.f,pos,seed);
  AudioBuffer<float> b(2,512);MidiBuffer m;
  const float bad[]={std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()};
  bool finiteDuring=true;
  for(float v:bad){b.clear();for(int i=100;i<110;++i){b.setSample(0,i,v);b.setSample(1,i,v);}p.processBlock(b,m);
    for(int c=0;c<2;++c)for(int i=0;i<512;++i)finiteDuring&=std::isfinite(b.getSample(c,i));}
  check(finiteDuring,"T2a output stays finite during NaN/Inf input");
  auto r=run(p,2,48000,512,48000,1.f,pos,seed,0);
  check(r.finite&&r.peak>.05f,"T2b plugin still passes audio after NaN/Inf input (peak "+String(r.peak,3)+")");}
 // ---- T3: silence after loud material decays to true silence; no denormal slowdown ----
 {SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,48000,512);p.prepareToPlay(48000,512);setP(p,"oversampling",2);setP(p,"reanimate",100);
  int pos=0;uint32_t seed=5;run(p,2,48000,512,48000,1.f,pos,seed);
  AudioBuffer<float> b(2,512);MidiBuffer m;
  double tSilence=0;float tail=0;
  for(int blk=0;blk<48000*4/512;++blk){b.clear();auto t0=std::chrono::steady_clock::now();p.processBlock(b,m);
    if(blk>=48000*3/512)tSilence+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
    if(blk>=48000*3/512)for(int c=0;c<2;++c)for(int i=0;i<512;++i)tail=std::max(tail,std::abs(b.getSample(c,i)));}
  check(tail<1e-6f,"T3a output after 3 s of silence is < -120 dBFS (max "+String(tail,9)+")");
  // reference cost of loud material for the same amount of audio
  double tLoud=0;int pos2=0;for(int blk=0;blk<48000/512;++blk){auto t0=std::chrono::steady_clock::now();run(p,2,48000,512,512,1.f,pos2,seed);tLoud+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();}
  std::cout<<"  silence(1s) "<<tSilence/ (48000/512.0*0+ (48000/512.0)) <<" vs loud(1s) ~"<<tLoud<<" ms (info)\n";
  check(tSilence/(48000.0/512.0)*(48000.0/512.0)<tLoud*3+5,"T3b silent processing is not slower than loud processing (denormals)");}
 // ---- T4: measured latency == reported latency (REVIVE off + bypass paths) ----
 for(int os=0;os<3;++os)for(int byp=0;byp<2;++byp){
   SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,48000,512);p.prepareToPlay(48000,512);
   setP(p,"oversampling",(float)os);setP(p,"reanimateOn",0);setP(p,"bypass",(float)byp);setP(p,"low",0);setP(p,"mid",0);setP(p,"high",0);setP(p,"hfAdjust",0);setP(p,"push",0);setP(p,"limiterOn",0);setP(p,"input",0);setP(p,"output",0);
   AudioBuffer<float> b(2,512);MidiBuffer m;
   for(int k=0;k<12;++k){b.clear();p.processBlock(b,m);MessageManager::getInstance()->runDispatchLoopUntil(5);}  // settle smoothers; deliver async latency update
   const int impulseAt=100;int peakIdx=-1;float peak=0;int sampleIdx=0;
   for(int blk=0;blk<8;++blk){b.clear();if(blk==0)b.setSample(0,impulseAt,.5f),b.setSample(1,impulseAt,.5f);
     if(blk==0){p.processBlock(b,m);}else p.processBlock(b,m);
     for(int i=0;i<512;++i){float v=std::abs(b.getSample(0,i));if(v>peak){peak=v;peakIdx=sampleIdx+i;}}sampleIdx+=512;}
   p.processBlock(b,m); // flush
   const int measured=peakIdx-impulseAt;
   check(measured==p.getLatencySamples(),"T4 latency os="+String(os)+" bypass="+String(byp)+": measured "+String(measured)+" vs reported "+String(p.getLatencySamples())+" (peak "+String(peak,3)+")");
 }
 // ---- T5: extreme input (huge level, DC) stays finite and under ceiling ----
 {SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,48000,512);p.prepareToPlay(48000,512);setP(p,"push",24);setP(p,"reanimate",100);setP(p,"oversampling",2);
  int pos=0;uint32_t seed=9;auto r=run(p,2,48000,512,48000,1000.f,pos,seed,9600);
  check(r.finite&&r.peak<=ceilLin[0]+1e-4f,"T5 +60 dB overload input: finite, peak "+String(r.peak,4));}
 // ---- T6: toggling oversampling during playback ----
 {SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,48000,512);p.prepareToPlay(48000,512);setP(p,"reanimate",80);
  int pos=0;uint32_t seed=11;bool fin=true;float pk=0;
  for(int k=0;k<40;++k){setP(p,"oversampling",(float)(k%3));auto r=run(p,2,48000,256,4800,.7f,pos,seed);fin&=r.finite;pk=std::max(pk,r.peak);}
  check(fin&&pk<=1.0f,"T6 oversampling toggles every 100 ms: finite, peak "+String(pk,3));}
 // ---- T7: mono layout ----
 {SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(1,1,44100,512);p.prepareToPlay(44100,512);setP(p,"oversampling",1);
  int pos=0;uint32_t seed=13;auto r=run(p,1,44100,512,44100,1.f,pos,seed,4410);check(r.finite&&r.peak>.05f,"T7 mono in/out finite, peak "+String(r.peak,3));}

 // ---- T9: end-to-end alias rejection (non-harmonic energy vs oversampling) ----
 {const int N=65536,k=13312;const double sr=48000.;const float f=(float)(k*sr/N);double aliasDb[3]={};
  for(int os=0;os<3;++os){
   SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,sr,512);p.prepareToPlay(sr,512);
   setP(p,"oversampling",(float)os);setP(p,"reanimate",100);setP(p,"limiterOn",0);setP(p,"low",0);setP(p,"mid",0);setP(p,"high",0);setP(p,"hfAdjust",0);setP(p,"input",0);setP(p,"output",0);setP(p,"levelMatch",0);
   AudioBuffer<float> b(2,512);MidiBuffer m;std::vector<float> y;y.reserve(N);int pos=0;
   for(int blk=0;blk<(N+48000)/512+2;++blk){for(int n=0;n<512;++n){float v=.7f*std::sin(MathConstants<float>::twoPi*f*(float)(pos+n)/(float)sr);b.setSample(0,n,v);b.setSample(1,n,v);}
     pos+=512;p.processBlock(b,m);MessageManager::getInstance()->runDispatchLoopUntil(1);
     if((int)(blk*512)>=48000&&(int)y.size()<N)for(int n=0;n<512&&(int)y.size()<N;++n)y.push_back(b.getSample(0,n));}
   dsp::FFT fft(16);std::vector<float> data(2*N,0.f);
   for(int i=0;i<N;++i)data[(size_t)i]=y[(size_t)i]*(.5f-.5f*std::cos(MathConstants<float>::twoPi*(float)i/(float)N));
   fft.performFrequencyOnlyForwardTransform(data.data());
   auto isHarm=[&](int bin){for(int h=1;h*k<=N/2;++h)if(std::abs(bin-h*k)<=3)return true;return false;};
   double fund=0,alias=0;for(int bin=3;bin<=N/2;++bin){double e=(double)data[(size_t)bin]*data[(size_t)bin];if(std::abs(bin-k)<=3)fund+=e;else if(!isHarm(bin))alias+=e;}
   aliasDb[os]=10*std::log10(alias/fund+1e-30);
   {std::vector<std::pair<double,int>> top;for(int bin=3;bin<=N/2;++bin)if(std::abs(bin-k)>3&&!isHarm(bin))top.push_back({(double)data[(size_t)bin]*data[(size_t)bin],bin});
    std::sort(top.begin(),top.end(),[](auto&x,auto&y){return x.first>y.first;});
    std::cout<<"    top residual bins:";for(int q=0;q<6;++q)std::cout<<" "<<(int)(top[(size_t)q].second*sr/N)<<"Hz("<<(int)(10*std::log10(top[(size_t)q].first/fund))<<"dB)";std::cout<<"\n";}
   std::cout<<"  alias (non-harmonic) level os="<<os<<": "<<aliasDb[os]<<" dB rel fundamental\n";}
  check(aliasDb[2]<aliasDb[0]-6.0&&aliasDb[1]<aliasDb[0]-3.0,"T9 oversampling reduces aliasing vs off (4x by >6 dB, 2x by >3 dB)");}
 // ---- T8: CPU (single thread, 48 kHz, 512-sample blocks, 10 s of audio) ----
 for(int os=0;os<3;++os){
   SkellNecromancerAudioProcessor p;p.setPlayConfigDetails(2,2,48000,512);p.prepareToPlay(48000,512);setP(p,"oversampling",(float)os);setP(p,"reanimate",70);
   int pos=0;uint32_t seed=1;run(p,2,48000,512,48000,.5f,pos,seed);
   auto t0=std::chrono::steady_clock::now();run(p,2,48000,512,48000*10,.5f,pos,seed);
   const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
   std::cout<<"  CPU os="<<os<<": "<<ms/10000.0*100.0<<" % of one core (includes test-signal generation)\n";}
 std::cout<<(fails?"STRESS FAILED\n":"STRESS PASSED\n");return fails?1:0;}
