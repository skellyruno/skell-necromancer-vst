#pragma once
#include <JuceHeader.h>
#include "BinaryData.h"
namespace Theme {
constexpr int width = 1157, height = 612;
constexpr float cx = 568.0f, cy = 325.0f;
constexpr int footerY = 580, headerY = 36;
const juce::Colour black(0xff07090a), panel(0xff14181a), panelRaised(0xff1c2123);
const juce::Colour green(0xff39ff3a), greenLight(0xff8dff8a), greenDim(0xff135c1a);
const juce::Colour bone(0xffe8e6da), violet(0xff9b5cff), cyan(0xff3affd9);
const juce::Colour faint(0xff687176), border(0xff424d4f);
constexpr juce::Rectangle<int> topBar(0,0,width,36), header(0,36,width,44), footer(0,580,width,32);
constexpr juce::Rectangle<int> dial(348,105,440,440), inputMeter(85,99,48,282), outputMeter(1012,99,48,282);
constexpr juce::Rectangle<int> low(230,170,65,65), mid(230,323,65,65), high(847,170,65,65), hf(847,323,65,65);
constexpr juce::Rectangle<int> input(73,443,66,66), output(1001,443,66,66);
constexpr juce::Rectangle<int> range(302,329,48,70);
constexpr juce::Rectangle<int> push(933,589,105,18);
inline juce::Font display(float size) {
#if SKELL_HAS_DISPLAY_FONT
 static auto typeface=juce::Typeface::createSystemTypefaceFor(BinaryData::display_ttf,BinaryData::display_ttfSize);
 if(typeface!=nullptr)return juce::Font(juce::FontOptions(typeface).withHeight(size));
#endif
 return juce::Font(juce::FontOptions("Orbitron",size,juce::Font::bold));
}
inline juce::Font mono(float size) {
#if SKELL_HAS_MONO_FONT
 static auto typeface=juce::Typeface::createSystemTypefaceFor(BinaryData::mono_ttf,BinaryData::mono_ttfSize);
 if(typeface!=nullptr)return juce::Font(juce::FontOptions(typeface).withHeight(size));
#endif
 return juce::Font(juce::FontOptions("JetBrains Mono",size,juce::Font::plain));
}
inline juce::Colour spectral(float t) { t=juce::jlimit(0.0f,1.0f,t); return (t<0.55f ? violet.interpolatedWith(green,t/0.55f) : green.interpolatedWith(cyan,(t-0.55f)/0.45f)); }
}
