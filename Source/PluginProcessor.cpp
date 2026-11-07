#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;
namespace {
using AP=AudioParameterFloat; using BP=AudioParameterBool;using CP=AudioParameterChoice;
constexpr const char* toneIds[4]={"low","mid","high","hfAdjust"};
constexpr const char* allParamIds[]={"reanimate","reanimateOn","bypass","input","output","low","mid","midRange","high","hfAdjust","limiterOn","push","ceiling","mode","mix","deltaListen","levelMatch","width","deEssGuard","oversampling","monoToStereo"};
}
SkellNecromancerAudioProcessor::SkellNecromancerAudioProcessor()
 :AudioProcessor(BusesProperties().withInput("Input",AudioChannelSet::stereo(),true).withOutput("Output",AudioChannelSet::stereo(),true)),
 apvts(*this,&undoManager,"PARAMETERS",makeParameters()) {
 writeUiDefaults();for(auto*id:allParamIds)apvts.addParameterListener(id,this);
}
SkellNecromancerAudioProcessor::~SkellNecromancerAudioProcessor(){
 for(auto*id:allParamIds)apvts.removeParameterListener(id,this);
 cancelPendingUpdate();
}
void SkellNecromancerAudioProcessor::writeUiDefaults(){
 uiState.setProperty("preset","INIT",nullptr);uiState.setProperty("window","M",nullptr);
 uiState.setProperty("hints",true,nullptr);uiState.setProperty("autopsy","AUTOPSY: standby",nullptr);
 uiState.setProperty("softClip",false,nullptr);uiState.setProperty("drag","vertical",nullptr);
 uiState.setProperty("meterHold",1000,nullptr);uiState.setProperty("lufsWindow","short",nullptr);
 uiState.setProperty("autoStatus","READY",nullptr);uiState.setProperty("abSelected",0,nullptr);
 uiState.setProperty("targetLUFS",-14.0f,nullptr);
}
AudioProcessorValueTreeState::ParameterLayout SkellNecromancerAudioProcessor::makeParameters(){
 std::vector<std::unique_ptr<RangedAudioParameter>> p;
 auto fl=[&](const char*id,const char*name,float min,float max,float def,float step=.01f){p.push_back(std::make_unique<AP>(ParameterID{id,1},name,NormalisableRange<float>(min,max,step),def));};
 auto b=[&](const char*id,const char*name,bool def){p.push_back(std::make_unique<BP>(ParameterID{id,1},name,def));};
 auto ch=[&](const char*id,const char*name,StringArray choices,int def){p.push_back(std::make_unique<CP>(ParameterID{id,1},name,choices,def));};
 fl("reanimate","Reanimate",0,100,65,.1);b("reanimateOn","Revive On",true);b("bypass","Bypass",false);
 fl("input","Input dB",-12,12,0);fl("output","Output dB",-12,12,0);
 fl("low","Low dB",-6,6,1.5f);fl("mid","Mid dB",-6,6,-1);ch("midRange","Mid Range",{"I","II","III"},0);
 fl("high","High dB",-6,6,2);fl("hfAdjust","HF Adjust dB",-6,6,1);
 b("limiterOn","Limiter On",true);fl("push","Push dB",0,24,6);ch("ceiling","Ceiling",{"-0.1 dBTP","-0.3 dBTP","-1.0 dBTP"},1);
 ch("mode","Mode",{"VOCAL","MASTER","BUS"},0);fl("mix","Parallel Mix %",0,100,100,.1);
 b("deltaListen","Delta Listen",false);b("levelMatch","Gain Match",true);
 fl("width","Stereo Width %",0,150,100,.5);fl("deEssGuard","De-ess Guard %",0,100,45,.5);
 ch("oversampling","Oversampling",{"OFF","2x","4x"},1);b("monoToStereo","Mono to Stereo",false);
 return {p.begin(),p.end()};
}
float SkellNecromancerAudioProcessor::param(const char*id)const noexcept {auto*raw=apvts.getRawParameterValue(id);return raw?raw->load():0.f;}
void SkellNecromancerAudioProcessor::setParam(const char*id,float value){
 if(auto* p=apvts.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}
}
bool SkellNecromancerAudioProcessor::isBusesLayoutSupported(const BusesLayout& b)const {
 const auto in=b.getMainInputChannelSet(),out=b.getMainOutputChannelSet();
 return (in==AudioChannelSet::mono()||in==AudioChannelSet::stereo())&&
        (out==in||(in==AudioChannelSet::mono()&&out==AudioChannelSet::stereo()));
}
void SkellNecromancerAudioProcessor::prepareToPlay(double sr,int block){
 sampleRateHz=sr;maxBlockSize=juce::jmax(32,block);
 revive.prepare(sr);tone.prepare(sr);limiter.prepare(sr,maxBlockSize);lufs.prepare(sr);analyzer.prepare(sr);
 const int cap=juce::jmax(128,(int)std::ceil(sr*.015));bypassDelay.prepare(cap+limiter.latencySamples());wetDelay.prepare(cap);
 for(auto*s:{&reviveSm,&mixSm,&inputSm,&outputSm,&widthSm,&pushSm,&guardSm})s->reset(sr,.02);fadeSm.reset(sr,.010);
 reviveSm.setCurrentAndTargetValue(param("reanimate")*.01f);mixSm.setCurrentAndTargetValue(param("mix")*.01f);
 inputSm.setCurrentAndTargetValue(param("input"));outputSm.setCurrentAndTargetValue(param("output"));widthSm.setCurrentAndTargetValue(param("width")*.01f);
 fadeSm.setCurrentAndTargetValue(param("reanimateOn")>.5f?1.f:0.f);pushSm.setCurrentAndTargetValue(param("push"));guardSm.setCurrentAndTargetValue(param("deEssGuard")*.01f);
 for(size_t i=0;i<toneSm.size();++i){toneSm[i].reset(sr,.03);toneSm[i].setCurrentAndTargetValue(param(toneIds[i]));}
 osQuality=juce::jlimit(0,2,(int)param("oversampling"));osRequested=osQuality;
 ensureLatency(osQuality);muteSamples=0;sampleCounter=0;sideLow[0]=sideLow[1]=0;monoHistory.fill(0);monoHead=0;softClipActive.store((bool)uiState.getProperty("softClip",false));
 wetPower=dryPower=1e-6f;inputL=inputR=outputL=outputR=-100;gainReduction=0;momentary=shortTerm=integrated=-100;
}
void SkellNecromancerAudioProcessor::releaseResources(){}
void SkellNecromancerAudioProcessor::ensureLatency(int quality){
 // Lookahead is fixed; oversampling additional delay is aligned for wet + dry.
 currentLatency=limiter.latencySamples()+(quality==0?0:quality==1?2:4);
 setLatencySamples(currentLatency.load());
}
void SkellNecromancerAudioProcessor::processBlock(AudioBuffer<float>& buffer,MidiBuffer& midi){
 ScopedNoDenormals noDenormals;midi.clear();const int n=buffer.getNumSamples();if(n==0)return;
 for(int i=getTotalNumInputChannels();i<buffer.getNumChannels();++i) buffer.clear(i,0,n);
 if(autoRequested.exchange(false)){
   bool playing=true;if(auto*playHead=getPlayHead())if(auto pos=playHead->getPosition())playing=pos->getIsPlaying();
   if(playing){
     if(analyzer.getState()==NecroDSP::AutoAnalyzer::needsReplay)analyzer.startVerification();
     else analyzer.start();
     previousAutoState=analyzer.getState();triggerAsyncUpdate();
   }
   else {analyzer.reset();autoNotice.store(1);triggerAsyncUpdate();}
 }
 int quality=juce::jlimit(0,2,(int)param("oversampling"));
 if(quality!=osQuality){osQuality=quality;osRequested=quality;muteSamples=(int)(sampleRateHz*.025);latencyChanged.store(true);triggerAsyncUpdate();revive.reset();}
 const int factor=quality==0?1:quality==1?2:4;
 const int mode=juce::jlimit(0,2,(int)param("mode"));
 reviveSm.setTargetValue(param("reanimate")*.01f);mixSm.setTargetValue(param("mix")*.01f);
 inputSm.setTargetValue(param("input"));outputSm.setTargetValue(param("output"));widthSm.setTargetValue(param("width")*.01f);
 fadeSm.setTargetValue(param("reanimateOn")>.5f?1.f:0.f);pushSm.setTargetValue(param("push"));guardSm.setTargetValue(param("deEssGuard")*.01f);
 for(size_t i=0;i<toneSm.size();++i)toneSm[i].setTargetValue(param(toneIds[i]));
 static constexpr float ceilings[]={-.1f,-.3f,-1.f};
 const bool reanimateOn=param("reanimateOn")>.5f,bypass=param("bypass")>.5f,delta=param("deltaListen")>.5f,matching=param("levelMatch")>.5f;
 const bool limiterEnabled=param("limiterOn")>.5f;
 auto*left=buffer.getWritePointer(0);auto*right=buffer.getNumChannels()>1?buffer.getWritePointer(1):nullptr;
 const float rmsSmooth=(float)std::exp(-1.0/(sampleRateHz*1.0));
 float inPeakL=0,inPeakR=0,outPeakL=0,outPeakR=0;
 const float monoBassCoeff=(float)(1.0-std::exp(-juce::MathConstants<double>::twoPi*120./sampleRateHz));
 for(int i=0;i<n;++i){
   const float rawL=left[i],rawR=(getTotalNumInputChannels()>1&&right)?right[i]:rawL;
   inPeakL=std::max(inPeakL,std::abs(rawL));inPeakR=std::max(inPeakR,std::abs(rawR));
   const float pre=(float)NecroDSP::dbToGain(inputSm.getNextValue());
   float l=rawL*pre,r=rawR*pre;
   float dryL=rawL,dryR=rawR;
   const float a=reviveSm.getNextValue(),parallel=mixSm.getNextValue(),fade=fadeSm.getNextValue();
   if(i%16==0){
       tone.set(toneSm[0].skip(16),toneSm[1].skip(16),(int)param("midRange"),toneSm[2].skip(16),toneSm[3].skip(16));
       revive.set(a,mode,factor,guardSm.skip(16));
       limiter.set(limiterEnabled,pushSm.skip(16),ceilings[juce::jlimit(0,2,(int)param("ceiling"))],softClipActive.load());
   }
   float revL=l,revR=r;revive.process(revL,revR);
   // The REVIVE bypass is crossfaded (10-20 ms), trim/EQ/limiter remain active.
   const float t=parallel*fade;l=l+(revL-l)*t;r=r+(revR-r)*t;
   const float rangeValue=tone.nextRangeValue();
   if(mode==1){float m=.5f*(l+r),s=.5f*(l-r);
       m=tone.process(0,m,rangeValue);s=tone.process(1,s,rangeValue);
       sideLow[0]+=monoBassCoeff*(s-sideLow[0]);
       s=(s-sideLow[0])*widthSm.getNextValue();l=m+s;r=m-s;
   }else{l=tone.process(0,l,rangeValue);r=tone.process(1,r,rangeValue);(void)widthSm.getNextValue();}
   if(getTotalNumInputChannels()==1&&right&&param("monoToStereo")>.5f){
       // Very subtle original upper-mid decorrelation; high-passed below 120 Hz.
       float delayed=monoHistory[(size_t)((monoHead+64-23)%64)];
       monoHistory[(size_t)monoHead]=l;monoHead=(monoHead+1)%64;
       const float d=(l-delayed)*.012f;l+=d;r-=d;
   }
   const float post=(float)NecroDSP::dbToGain(outputSm.getNextValue());l*=post;r*=post;
   limiter.process(l,r);
   wetDelay.process(l,r,quality==0?0:quality==1?2:4);
   bypassDelay.process(dryL,dryR,currentLatency.load());
   const float wetSquare=.5f*(l*l+r*r),drySquare=.5f*(dryL*dryL+dryR*dryR);
   wetPower=rmsSmooth*wetPower+(1.f-rmsSmooth)*wetSquare;
   dryPower=rmsSmooth*dryPower+(1.f-rmsSmooth)*drySquare;
   const float match=matching?juce::jlimit(.25f,4.f,std::sqrt(wetPower/std::max(dryPower,1e-9f))):1.f;
   const float matchedDryL=dryL*match,matchedDryR=dryR*match;
   if(delta){l-=matchedDryL;r-=matchedDryR;}
   else if(bypass){l=matchedDryL;r=matchedDryR;}
   if(muteSamples>0){const float m=1.f-(float)muteSamples/std::max(1.f,(float)(sampleRateHz*.025));l*=m;r*=m;--muteSamples;}
   lufs.process(l,r,getTotalNumOutputChannels());analyzer.push(rawL,rawR);
   outPeakL=std::max(outPeakL,std::abs(l));outPeakR=std::max(outPeakR,std::abs(r));
   left[i]=l;if(right)right[i]=r;
 }
 inputL.store((float)NecroDSP::gainToDb(inPeakL));inputR.store((float)NecroDSP::gainToDb(inPeakR));
 outputL.store((float)NecroDSP::gainToDb(outPeakL));outputR.store((float)NecroDSP::gainToDb(outPeakR));
 gainReduction.store(limiter.reduction()+revive.getGainReduction());
 momentary.store(lufs.m());shortTerm.store(lufs.s());integrated.store(lufs.i());
 liveLevel.store(juce::jlimit(0.f,1.f,(float)std::sqrt(wetPower)*2.2f));
 int st=analyzer.getState();if(st!=previousAutoState){previousAutoState=st;triggerAsyncUpdate();}
}
void SkellNecromancerAudioProcessor::requestAuto(){autoRequested.store(true);}
void SkellNecromancerAudioProcessor::cancelAuto(){analyzer.reset();autoStatusText="AUTO: cancelled";triggerAsyncUpdate();}
void SkellNecromancerAudioProcessor::updateAutoResult(){
 const auto a=analyzer.getResult();
 StringArray flags;if(a.dull)flags.add("dull top");if(a.thin)flags.add("thin low-mids");if(a.boxy)flags.add("boxy mids");
 if(a.muddy)flags.add("muddy bass");if(a.narrow)flags.add("narrow field");if(a.overDynamic)flags.add("over-dynamic");
 if(flags.isEmpty())flags.add("balanced spectrum");
 uiState.setProperty("autopsy","AUTOPSY: "+flags.joinIntoString(", "),nullptr);
 undoManager.beginNewTransaction("Necromancer AUTO");
 const int mode=(int)param("mode");
 setParam("input",juce::jlimit(-12.f,12.f,-18.f-a.rmsDb));
 float score=(a.dull?12.f:0.f)+(a.thin?5.f:0.f)+(a.overDynamic?5.f:0.f)+(a.boxy?3.f:0.f);
 setParam("reanimate",juce::jlimit(25.f,75.f,40.f+score));
 setParam("low",juce::jlimit(-4.f,4.f,(a.thin?2.4f:0.f)-(a.muddy?2.0f:0.f)));
 setParam("mid",juce::jlimit(-4.f,4.f,a.boxy?-2.7f:0.f));
 setParam("midRange",a.boxy?0.f:1.f);
 setParam("high",juce::jlimit(-4.f,4.f,a.dull?2.5f:0.f));
 setParam("hfAdjust",juce::jlimit(-3.f,3.f,a.dull?1.3f:0.f));
 setParam("deEssGuard",mode==0?65.f:35.f);
 // No host seek API: user must replay the region, then press AUTO again.
 autoPass1Applied=true;autoStatusText="AUTO: REPLAY LOUDEST 3s + PRESS AUTO";
}
void SkellNecromancerAudioProcessor::handleAsyncUpdate(){
 if(latencyChanged.exchange(false))ensureLatency(osRequested.load());
 if(autoNotice.exchange(0)==1)autoStatusText="AUTO: start playback first";
 const auto st=analyzer.getState();
 if(st==NecroDSP::AutoAnalyzer::listening){autoPass1Applied=false;autoStatusText="AUTO: LISTENING 12s";}
 else if(st==NecroDSP::AutoAnalyzer::verifying)autoStatusText="AUTO: VERIFYING 3s";
 else if(st==NecroDSP::AutoAnalyzer::needsReplay&&!autoPass1Applied)updateAutoResult();
 else if(st==NecroDSP::AutoAnalyzer::complete){
    // A conservative calibration; the target still requires real musical verification.
    undoManager.beginNewTransaction("Necromancer AUTO loudness");
    const float goal=(float)uiState.getProperty("targetLUFS",-14.0f),measured=analyzer.getReplayLufs();
    const float correction=juce::jlimit(-8.f,8.f,goal-measured);
    setParam("push",juce::jlimit(0.f,18.f,param("push")+correction*.6f));
    setParam("output",juce::jlimit(-12.f,12.f,param("output")+correction*.4f));
    autoStatusText="AUTO: STARTING POINT SET";analyzer.reset();
 }
 else if(st==NecroDSP::AutoAnalyzer::abort)autoStatusText="AUTO: signal too quiet";
 uiState.setProperty("autoStatus",autoStatusText,nullptr);
}
File SkellNecromancerAudioProcessor::presetDirectory()const{
 return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Skell Audio").getChildFile("Skell-Necromancer").getChildFile("Presets");
}
StringArray SkellNecromancerAudioProcessor::factoryPresetNames()const{return {"INIT","Dead Vocal Rescue","Raise the Master","Gentle Resurrection","Crypt Warmth","Bone Air","Grave Glue","Loud and Alive"};}
void SkellNecromancerAudioProcessor::applyFactoryPreset(const String&name){
 struct Preset{const char*name;float mode,rev,low,mid,range,high,hf,push,mix,width,deess;};
 static constexpr Preset presets[]={
 {"INIT",0,65,1.5f,-1,0,2,1,6,100,100,45},
 {"Dead Vocal Rescue",0,70,2.2f,-2,0,2.4f,1.4f,5,94,100,70},
 {"Raise the Master",1,58,1.4f,-.8f,1,1.1f,.5f,7,100,106,0},
 {"Gentle Resurrection",0,34,.7f,-.5f,1,.8f,.4f,3.5f,85,100,45},
 {"Crypt Warmth",2,72,3,-1.5f,1,-.4f,-.5f,4.5f,95,100,20},
 {"Bone Air",0,62,-.2f,-1.0f,0,2.4f,2.9f,5,90,100,80},
 {"Grave Glue",2,66,1.5f,-.4f,1,.8f,.2f,6,100,100,20},
 {"Loud and Alive",1,74,1.3f,-1.0f,1,1.5f,1.1f,11,100,110,0}};
 for(auto& p:presets)if(name==p.name){undoManager.beginNewTransaction("Preset: "+name);
  setParam("mode",p.mode);setParam("reanimate",p.rev);setParam("low",p.low);setParam("mid",p.mid);
  setParam("midRange",p.range);setParam("high",p.high);setParam("hfAdjust",p.hf);setParam("push",p.push);
  setParam("mix",p.mix);setParam("width",p.width);setParam("deEssGuard",p.deess);
  uiState.setProperty("preset",name,nullptr);presetDirty.store(false);break;}
}
void SkellNecromancerAudioProcessor::getStateInformation(MemoryBlock& data){
 XmlElement root("SKELL_NECROMANCER_STATE");
 if(auto a=apvts.copyState().createXml())root.addChildElement(a.release());
 if(auto ui=uiState.createXml())root.addChildElement(ui.release());
 copyXmlToBinary(root,data);
}
void SkellNecromancerAudioProcessor::setStateInformation(const void*data,int size){
 if(auto root=getXmlFromBinary(data,size)){
   if(root->hasTagName("SKELL_NECROMANCER_STATE")){
      if(auto* p=root->getChildByName("PARAMETERS"))apvts.replaceState(ValueTree::fromXml(*p));
      if(auto*ui=root->getChildByName("NECRO_UI"))uiState=ValueTree::fromXml(*ui);
      selectedSlot=(int)uiState.getProperty("abSelected",0);softClipActive.store((bool)uiState.getProperty("softClip",false));presetDirty.store(false);
   }else if(root->hasTagName("PARAMETERS"))apvts.replaceState(ValueTree::fromXml(*root));
 }
}
bool SkellNecromancerAudioProcessor::writeUserPreset(const String&name,String&error){
 File dir=presetDirectory();if(!dir.createDirectory()){error="Cannot create preset directory";return false;}
 File file=dir.getChildFile(File::createLegalFileName(name)+".xml");
 XmlElement root("NECRO_PRESET");root.setAttribute("name",name);
 if(auto x=apvts.copyState().createXml())root.addChildElement(x.release());
 if(!root.writeTo(file)){error="Cannot save "+file.getFullPathName();return false;}presetDirty.store(false);return true;
}
bool SkellNecromancerAudioProcessor::loadUserPreset(const File&file,String&error){
 auto xml=XmlDocument::parse(file);if(!xml){error="Invalid preset XML";return false;}
 const XmlElement*state=xml->hasTagName("PARAMETERS")?xml.get():xml->getChildByName("PARAMETERS");
 if(!state){error="Missing PARAMETERS";return false;}
 undoManager.beginNewTransaction("Load preset");apvts.replaceState(ValueTree::fromXml(*state));
 uiState.setProperty("preset",file.getFileNameWithoutExtension(),nullptr);presetDirty.store(false);return true;
}
void SkellNecromancerAudioProcessor::selectAB(int slot){
 slot=juce::jlimit(0,1,slot);
 if(slot==selectedSlot)return;
 const String oldKey=selectedSlot==0?"slotA":"slotB";
 if(auto current=apvts.copyState().createXml())uiState.setProperty(oldKey,current->toString(),nullptr);
 selectedSlot=slot;uiState.setProperty("abSelected",selectedSlot,nullptr);
 const String key=selectedSlot==0?"slotA":"slotB";
 auto xml=XmlDocument::parse(uiState.getProperty(key).toString());
 if(xml&&xml->hasTagName("PARAMETERS")){undoManager.beginNewTransaction("A/B recall");apvts.replaceState(ValueTree::fromXml(*xml));}
 else if(auto state=apvts.copyState().createXml())uiState.setProperty(key,state->toString(),nullptr);
}
void SkellNecromancerAudioProcessor::copyAB(int from,int to){
 const String a=from==0?"slotA":"slotB",b=to==0?"slotA":"slotB";
 if((int)uiState.getProperty("abSelected",0)==from){if(auto s=apvts.copyState().createXml())uiState.setProperty(b,s->toString(),nullptr);}
 else uiState.setProperty(b,uiState.getProperty(a),nullptr);
}
AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new SkellNecromancerAudioProcessor();}
