from pathlib import Path
import re
p=Path(__file__).resolve().parents[1]
names=('PluginProcessor.h PluginProcessor.cpp PluginEditor.h PluginEditor.cpp Theme.h').split()
ui='ReanimateDial RuneRing MetalKnob RangeSelector PeakMeter PushSlider TopBar PresetBar HintOverlay AutopsyReadout LookAndFeel'.split()
dsp='ReviveEngine TapeStage GlueStage HarmonicLife TransientRestore ToneSection TruePeakLimiter LufsMeter AutoAnalyzer Oversampler Biquad'.split()
for filename in [*[p/'Source'/v for v in names],*[p/'Source/ui'/(x+'.h') for x in ui],*[p/'Source/dsp'/(x+'.h') for x in dsp],p/'CMakeLists.txt']:
 assert filename.exists(),filename
 print('OK',filename.relative_to(p))
s=(p/'Source/PluginProcessor.cpp').read_text()
for param in ['reanimate','reanimateOn','bypass','input','output','low','mid','midRange','high','hfAdjust','limiterOn','push','ceiling','mode','mix','deltaListen','levelMatch','width','deEssGuard','oversampling','monoToStereo']:
 assert '"'+param+'"' in s,param
print('OK: all 20 automatable parameters present; AUTO is UI trigger')

cmake = (p/'CMakeLists.txt').read_text()
start = cmake.find("juce_add_plugin(SkellNecromancer")
assert start >= 0, 'SkellNecromancer JUCE plugin target is missing'
end = cmake.find(")", start)
assert end > start, 'SkellNecromancer JUCE plugin target is not closed'
plugin_target = cmake[start:end]
formats_lines = [line.strip() for line in plugin_target.splitlines() if line.strip().startswith("FORMATS ")]
assert formats_lines == ["FORMATS VST3"], formats_lines
assert "Standalone" not in plugin_target, 'Plugin Standalone format must not be built'
print('OK: SkellNecromancer product target is VST3-only')
