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
plugin_target = re.search(r'juce_add_plugin\\(SkellNecromancer\\b(.*?)\\)', cmake, re.S)
assert plugin_target, 'SkellNecromancer JUCE plugin target is missing'
formats = re.search(r'^\\s*FORMATS\\s+(.+?)\\s*, plugin_target.group(1), re.M)
assert formats and formats.group(1).split() == ['VST3'], formats.group(1) if formats else 'FORMATS line missing'
assert 'Standalone' not in plugin_target.group(1), 'Plugin Standalone format must not be built'
print('OK: SkellNecromancer product target is VST3-only')
