#pragma once
#include <JuceHeader.h>
#include "Theme.h"
#include "dsp/ReviveEngine.h"
#include "dsp/ToneSection.h"
#include "dsp/TruePeakLimiter.h"
#include "dsp/LufsMeter.h"
#include "dsp/AutoAnalyzer.h"
#include <array>
class SkellNecromancerAudioProcessor final : public juce::AudioProcessor,private juce::AsyncUpdater,private juce::AudioProcessorValueTreeState::Listener {
public:
 SkellNecromancerAudioProcessor();~SkellNecromancerAudioProcessor()override;
 const juce::String getName()const override{return "Skell-Necromancer";}
 void prepareToPlay(double,int)override;void releaseResources()override;void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 juce::AudioProcessorEditor* createEditor()override;bool hasEditor()const override{return true;}
 bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return .20;}
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{};
 const juce::String getProgramName(int)override{return "Current";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
 juce::UndoManager undoManager;
 juce::AudioProcessorValueTreeState apvts;
 juce::ValueTree uiState{"NECRO_UI"};
 void requestAuto();void cancelAuto();int autoState()const noexcept{return analyzer.getState();}
 juce::String autopsyString()const{return uiState.getProperty("autopsy","AUTOPSY: standby").toString();}
 void applyFactoryPreset(const juce::String&);juce::StringArray factoryPresetNames()const;
 bool writeUserPreset(const juce::String&,juce::String&);
 bool loadUserPreset(const juce::File&,juce::String&);
 juce::File presetDirectory()const;
 void selectAB(int);void copyAB(int from,int to);int selectedSlot=0;
 void setSoftClip(bool x) {softClipActive.store(x);uiState.setProperty("softClip",x,nullptr);}
 std::atomic<float>inputL{-100},inputR{-100},outputL{-100},outputR{-100},gainReduction{0};
 std::atomic<float>momentary{-100},shortTerm{-100},integrated{-100},liveLevel{0};
 std::atomic<float>targetLufs{-14};
 std::atomic<bool>presetDirty{false};
 juce::String autoStatus()const {return autoStatusText;}
private:
 void handleAsyncUpdate()override;
 void parameterChanged(const juce::String&,float)override {presetDirty.store(true);}void setParam(const char*,float);float param(const char*)const noexcept;
 void ensureLatency(int quality);void updateAutoResult();void writeUiDefaults();
 NecroDSP::ReviveEngine revive;NecroDSP::ToneSection tone;
 NecroDSP::TruePeakLimiter limiter;NecroDSP::SampleDelay bypassDelay,reviveDryDelay;
 NecroDSP::LufsMeter lufs;NecroDSP::AutoAnalyzer analyzer;
 std::atomic<bool>autoRequested{false},autoCancelRequested{false},latencyChanged{false},softClipActive{false};
 std::atomic<int>autoNotice{0};
 int osQuality=0,muteSamples=0,previousAutoState=0;
 std::atomic<int>osRequested{0},currentLatency{0};
 bool autoPass1Applied=false;
 double sampleRateHz=44100;int maxBlockSize=512;
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> reviveSm,mixSm,inputSm,outputSm,widthSm,fadeSm,bypassSm,pushSm,guardSm;
 std::array<juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear>,4> toneSm;
 float sideLow[2]{},wetPower=1.e-6f,dryPower=1.e-6f;int sampleCounter=0;
 std::array<float,64>monoHistory{};int monoHead=0;
 juce::String autoStatusText{"READY"};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellNecromancerAudioProcessor)
};
