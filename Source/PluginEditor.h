#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/MetalKnob.h"
#include "ui/ReanimateDial.h"
#include "ui/RangeSelector.h"
#include "ui/PeakMeter.h"
#include "ui/PushSlider.h"
#include "ui/TopBar.h"
#include "ui/HintOverlay.h"
#include "ui/AutopsyReadout.h"
class SkellNecromancerAudioProcessorEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:explicit SkellNecromancerAudioProcessorEditor(SkellNecromancerAudioProcessor&);~SkellNecromancerAudioProcessorEditor()override;
 void paint(juce::Graphics&)override;void resized()override;
private:
 class Panel:public juce::Component{
 public:Panel();void paint(juce::Graphics&)override;private:juce::Image bg;
 } panel;
 void timerCallback()override;void openSettings();void changeSize(int);void showTour();
 void attachSlider(juce::Slider&,const juce::String&);void attachButton(juce::Button&,const juce::String&);void attachChoice(juce::ComboBox&,const juce::String&);
 void setTextButton(juce::TextButton&,juce::String,int,int,int,int,const juce::String&,bool toggle=false);
 SkellNecromancerAudioProcessor&processor;NecroLookAndFeel look;
 TopBar top;ReanimateDial dial;
 MetalKnob low{"LOW"},mid{"MID"},high{"HIGH"},hf{"HF ADJUST"},input{"INPUT"},output{"OUTPUT"};
 RangeSelector range;
 PeakMeter inMeter,outMeter;
 PushSlider push;
 juce::ComboBox mode,mixMode;
 juce::Slider mix;
 juce::TextButton autoButton,onButton,bypassButton,limiterButton,deltaButton,matchButton;
 AutopsyReadout autopsy;
 HintOverlay hints;
 juce::Label lufs,gr,pushValue,mixValue,status;
 std::unique_ptr<juce::TooltipWindow>tooltips;
 std::unique_ptr<juce::VBlankAttachment>vblank;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>sliderAttachments;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>buttonAttachments;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>choiceAttachments;
 int tourStep=-1;bool hintsOn=true;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkellNecromancerAudioProcessorEditor)
};
