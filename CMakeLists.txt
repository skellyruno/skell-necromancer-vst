cmake_minimum_required(VERSION 3.22)
project(SkellNecromancer VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)
if(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
  add_subdirectory("${JUCE_PATH}" JUCE)
else()
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()
juce_add_plugin(SkellNecromancer
  COMPANY_NAME "Skell Audio"
  BUNDLE_ID "audio.skellyr.necromancer"
  PLUGIN_MANUFACTURER_CODE SkAu
  PLUGIN_CODE SnCr
  FORMATS VST3 Standalone
  PRODUCT_NAME "Skell-Necromancer"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(SkellNecromancer)
set(NECRO_ASSETS
  Assets/bg_brushed.png Assets/dial_well.png
  Assets/knob_cap_big.png Assets/knob_cap_small.png Assets/rune_atlas.png)
# Montserrat SemiBold (SIL OFL 1.1, licence in Assets/fonts) is bundled as the
# default display font. -DSKELL_DISPLAY_FONT / -DSKELL_MONO_FONT override it.
if(NOT DEFINED SKELL_DISPLAY_FONT)
  set(SKELL_DISPLAY_FONT "${CMAKE_CURRENT_SOURCE_DIR}/Assets/fonts/Montserrat-SemiBold.ttf")
endif()
set(NECRO_FONT_FLAGS SKELL_HAS_DISPLAY_FONT=0 SKELL_HAS_MONO_FONT=0)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/necro-fonts")
if(DEFINED SKELL_DISPLAY_FONT AND EXISTS "${SKELL_DISPLAY_FONT}")
  configure_file("${SKELL_DISPLAY_FONT}" "${CMAKE_CURRENT_BINARY_DIR}/necro-fonts/display.ttf" COPYONLY)
  list(APPEND NECRO_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/necro-fonts/display.ttf")
  list(REMOVE_ITEM NECRO_FONT_FLAGS SKELL_HAS_DISPLAY_FONT=0)
  list(APPEND NECRO_FONT_FLAGS SKELL_HAS_DISPLAY_FONT=1)
endif()
if(DEFINED SKELL_MONO_FONT AND EXISTS "${SKELL_MONO_FONT}")
  configure_file("${SKELL_MONO_FONT}" "${CMAKE_CURRENT_BINARY_DIR}/necro-fonts/mono.ttf" COPYONLY)
  list(APPEND NECRO_ASSETS "${CMAKE_CURRENT_BINARY_DIR}/necro-fonts/mono.ttf")
  list(REMOVE_ITEM NECRO_FONT_FLAGS SKELL_HAS_MONO_FONT=0)
  list(APPEND NECRO_FONT_FLAGS SKELL_HAS_MONO_FONT=1)
endif()
juce_add_binary_data(NecroAssets SOURCES ${NECRO_ASSETS})
target_sources(SkellNecromancer PRIVATE
  Source/Theme.h
  Source/PluginProcessor.h Source/PluginProcessor.cpp
  Source/PluginEditor.h Source/PluginEditor.cpp
  Source/dsp/Biquad.h Source/dsp/ToneSection.h
  Source/dsp/Oversampler.h Source/dsp/TapeStage.h Source/dsp/GlueStage.h
  Source/dsp/HarmonicLife.h Source/dsp/TransientRestore.h Source/dsp/Glow.h Source/dsp/ReviveEngine.h
  Source/dsp/TruePeakLimiter.h Source/dsp/LufsMeter.h Source/dsp/AutoAnalyzer.h
  Source/ui/ReanimateDial.h Source/ui/RuneRing.h Source/ui/RuneMotion.h Source/ui/MetalKnob.h
  Source/ui/RangeSelector.h Source/ui/PeakMeter.h Source/ui/PushSlider.h
  Source/ui/TopBar.h Source/ui/PresetBar.h Source/ui/HintOverlay.h
  Source/ui/AutopsyReadout.h Source/ui/LookAndFeel.h)
target_compile_features(SkellNecromancer PRIVATE cxx_std_17)
target_compile_definitions(SkellNecromancer PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 ${NECRO_FONT_FLAGS})
target_link_libraries(SkellNecromancer PRIVATE
  NecroAssets juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra juce::juce_dsp)

# Portable DSP regression target: uses only the tiny JUCE math shim in tests/stubs.
# The actual VST3 still builds against the full JUCE SDK above.
add_executable(NecromancerDSPTest tests/dsp_header_smoke.cpp)
target_include_directories(NecromancerDSPTest PRIVATE tests/stubs)
target_compile_features(NecromancerDSPTest PRIVATE cxx_std_17)
enable_testing()
add_test(NAME NecromancerDSP_StreamAndAuto COMMAND NecromancerDSPTest)

# Exact FIR group-delay, zero-amount revival null, and passband checks.
add_executable(NecromancerOversamplerTest tests/oversampler_latency.cpp)
target_include_directories(NecromancerOversamplerTest PRIVATE tests/stubs)
target_compile_features(NecromancerOversamplerTest PRIVATE cxx_std_17)
add_test(NAME NecromancerFIR_LatencyAndNull COMMAND NecromancerOversamplerTest)
add_executable(NecromancerOversamplerEquivalence tests/oversampler_equivalence.cpp)
target_include_directories(NecromancerOversamplerEquivalence PRIVATE tests/stubs)
target_compile_features(NecromancerOversamplerEquivalence PRIVATE cxx_std_17)
add_test(NAME NecromancerFIR_PolyphaseEquivalence COMMAND NecromancerOversamplerEquivalence)

# Deterministic probes compared with independent Python reference libraries.
add_executable(NecromancerLufsProbe tests/lufs_probe.cpp)
target_include_directories(NecromancerLufsProbe PRIVATE tests/stubs)
target_compile_features(NecromancerLufsProbe PRIVATE cxx_std_17)
add_executable(NecromancerLimiterProbe tests/limiter_probe.cpp)
target_include_directories(NecromancerLimiterProbe PRIVATE tests/stubs)
target_compile_features(NecromancerLimiterProbe PRIVATE cxx_std_17)

# Regression: rune glyphs must physically orbit even when live audio level is zero.
# This test uses exactly the orbit geometry compiled into the JUCE widget.
add_executable(NecromancerRuneMotionTest tests/rune_motion_test.cpp)
target_compile_features(NecromancerRuneMotionTest PRIVATE cxx_std_17)
add_test(NAME NecromancerUI_RuneMovement COMMAND NecromancerRuneMotionTest)

# Optional end-to-end processor stress test (NaN/Inf, latency, ceiling, aliasing, CPU). Off by default.
option(SKELL_BUILD_PROCESSOR_TESTS "Build the JUCE processor stress test" OFF)
if(SKELL_BUILD_PROCESSOR_TESTS)
  juce_add_console_app(NecromancerProcessorStress PRODUCT_NAME NecromancerProcessorStress)
  juce_generate_juce_header(NecromancerProcessorStress)
  target_sources(NecromancerProcessorStress PRIVATE tests/processor_stress.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_include_directories(NecromancerProcessorStress PRIVATE Source)
  target_compile_features(NecromancerProcessorStress PRIVATE cxx_std_17)
  target_compile_definitions(NecromancerProcessorStress PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_STANDALONE_APPLICATION=0 ${NECRO_FONT_FLAGS})
  target_link_libraries(NecromancerProcessorStress PRIVATE NecroAssets juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra juce::juce_dsp juce::juce_recommended_config_flags)
  add_test(NAME NecromancerProcessorStress COMMAND NecromancerProcessorStress)
endif()
