#pragma once
#include "PresetBar.h"
class TopBar:public juce::Component {
public:explicit TopBar(SkellNecromancerAudioProcessor&p):presets(p){addAndMakeVisible(presets);addAndMakeVisible(help);addAndMakeVisible(settings);
 help.setButtonText("?");settings.setButtonText("SET");help.setTooltip("Cycle the five-step guided tour; hidden until opened.");settings.setTooltip("Window size, performance and audio configuration.");}
 void resized()override{presets.setBounds(77,5,521,27);help.setBounds(1066,6,30,25);settings.setBounds(1102,6,44,25);}
 void paint(juce::Graphics&g)override{g.setColour(Theme::faint);g.setFont(Theme::mono(9));g.drawText("PRESET",17,8,54,20,juce::Justification::centredLeft);}
 PresetBar presets;juce::TextButton help,settings;
};
