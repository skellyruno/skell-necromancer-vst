#pragma once
#include "PresetBar.h"
class TopBar:public juce::Component {
public:explicit TopBar(SkellNecromancerAudioProcessor&p):presets(p){addAndMakeVisible(presets);addAndMakeVisible(help);addAndMakeVisible(settings);
 help.setButtonText("?");settings.setButtonText("SET");help.setTooltip("Cycle the five-step guided tour; hidden until opened.");settings.setTooltip("Window size, performance and audio configuration.");}
 void resized()override{
  // Reference-aligned preset strip, with room for the existing A/B workflow.
  presets.setBounds(72,5,610,27);
  help.setBounds(1078,5,28,26);settings.setBounds(1112,5,35,26);
 }
 void paint(juce::Graphics&g)override{g.setColour(Theme::faint);g.setFont(Theme::mono(9));g.drawText("PRESET",17,8,54,20,juce::Justification::centredLeft);}
 PresetBar presets;juce::TextButton help,settings;
};
