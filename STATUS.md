# Skell-Necromancer — engineering status (2026-11-07)

## Build status
- Earlier Windows JUCE 8 VST3 build: passed on GitHub Actions.
- Latest compact UI/DSP + regression suite: **PASSED** [Windows build #42403563396](https://github.com/skellyruno/skell-necromancer-vst/actions/runs/42403563396), including CMake, plugin compilation, DSP CTest, source integrity, packaging, and uploaded Windows VST3.
- Build success is distinct from validating sound quality, true-peak compliance, or DAW stability.

## Recent changes
- Default compact 70% UI, with persisted L/XL preferences respected.
- Preserved custom metal knobs, 3D central cap and rune ring; clearer control typography, panels, and bypass status.
- Reduced duplicate dial paints and avoided per-sample dB conversion for input/output trims.
- VOCAL high/air boosts now respond to stereo-linked sibilance detection and the de-ess guard amount.
- AUTO verification now measures the processed signal using a dedicated K-weighted 3-second meter, rather than the input RMS.
- Added Windows CTest smoke target for the DSP path, two-pass AUTO and limiter switching, with Release-safe failure checks.
- CI triggers on Source, tests, assets and CMake changes; cancels superseded workflow runs.

## Known limitations / release blockers
- Prototype interpolation/decimation oversampler is not a measured mastering-quality half-band implementation.
- True-peak detection and LUFS output require independent standard-reference testing and sample-accurate host verification.
- Oversampling mode changes and reported latency need explicit host PDC/automation testing.
- Real-time CPU budget, 44.1/48/96/192 kHz stress tests, live knob automation, mono/stereo tests, and listening benchmarks remain incomplete.
- FL Studio / Reaper VST3 load, resize, preset restore and host-close tests are still outstanding.

Do not market this build as production-mastering certified without these checks.

## 2026-11-07 FIR / loudness engineering pass
- Replaced linear-interpolation prototype with 65-tap sparse linear-phase FIR half-band up/down stages (2x and 4x).
- Accounted for two serial stage delays in both the REVIVE dry mix and host-reported latency: lookahead + 64 samples at 2x, or lookahead + 96 samples at 4x.
- Tightened the limiter safety margin after 8x independent reconstruction found ceiling misses with the former 4x-only test.
- Switched LUFS gating to 400ms blocks with 100ms hops, adjusted K-weighting to the independent reference, and added 0.1 LU histogram bins.
- Added standalone CTest phase/latency/alias checks and a cross-check against pyloudnorm and SciPy.
- GitHub Actions #42409204081 (commit 4a60df5) PASSED: Windows VST3/Standalone build, FIR PDC/phase tests, source integrity, 24 pyloudnorm integrated-LUFS comparisons, 108 independent SciPy 8x true-peak comparisons, and VST3 artifact upload. FL Studio host PDC and commercial audio-quality certification remain unverified. See docs/FL_STUDIO_PDC_TEST.md.

## 2026-11-10 Oversampler CPU checkpoint
- Oversampler rewritten as exact polyphase half-band (even phase = pure delay, odd phase folded by symmetry to 16 MACs, mirrored histories, mask indexing). Latency (32/48 samples/stage) and output unchanged.
- New CTest `NecromancerFIR_PolyphaseEquivalence` compares against frozen direct-form copy (`tests/reference/OversamplerReference.h`): max |diff| = 0 over 200k noisy/step samples, 2x/4x, plus factor switching.
- Local microbenchmark (g++ -O2, tanh waveshaper, 2M samples): 2x 308→141 ms, 4x 788→333 ms (~2.2-2.4x faster oversampler). Not a plugin-wide or MSVC measurement.
- Verified locally: latency/null/alias, DSP smoke, limiter probe, Python structure/asset/EQ/limiter tests. Windows CI for this change not yet run.

## 2026-11-10 UI polish pass (verified by headless JUCE render at 810/1157/1736 px)
- Meters: IN/OUT labels were drawn under the bars (IN hidden, OUT shown as "JT"); now a header inside PeakMeter. Scale font 8->9 pt.
- Layout mirrored about the dial axis (x=568); meter/side-panel frames no longer overlap (8 px gutters); side frames trimmed to their content; title and tagline centred over the dial; INPUT/OUTPUT knobs moved 8 px clear of the meter frames.
- Dial: high-quality resampling, rune glyphs drawn with sub-pixel placement and tint (previously integer-placed, aliased); REANIMATE label moved from the arc to the dial's bottom gap.
- `dial_well.png` regenerated with 4x supersampling (previously hard-edged, stair-stepped rings). Geometry identical; other assets deliberately left as the original files (regenerating them is not byte-identical).
- Preset bar: UNDO/REDO/SAVE buttons widened so text no longer touches borders.
- Not verified: rendering in a real host (FL Studio), Windows font rendering (headless render used fallback fonts), Windows CI for this commit.

## 2026-11-11 UI + DSP robustness pass
UI (verified by headless render at 810/1157/1736 px and scripted knob tests):
- Footer elements share one centre line; logo moved off the window edge; tagline off the header border.
- MetalKnob: hover now repaints (ring glow works on enter/exit); clicking the value readout opens an inline numeric editor (Return commits, Esc/focus-loss cancels, out-of-range clamps, non-numeric ignored; deferred teardown).
DSP / processor (tests/processor_stress.cpp, optional target `-DSKELL_BUILD_PROCESSOR_TESTS=ON`; run headless on Linux against the real processor):
- BUG FIXED: a single NaN/Inf input sample permanently poisoned filter/limiter/meter state (output non-finite, then silent). processBlock now zeroes non-finite input and bounds it to +/-1e6.
- Verified: 135/135 configs (3 sample rates x 3 oversampling x 3 modes x block sizes 1/37/64/512/4096, prepared at 512) are finite and under the selected ceiling; measured impulse latency equals reported latency for Off/2x/4x (120/184/216 @48k) with REVIVE off and with BYPASS; +60 dB overload and oversampling toggles every 100 ms stay finite and under ceiling; silence decays below -120 dBFS with no denormal slowdown; mono layout OK.
- Aliasing (9.75 kHz sine, REANIMATE 100): non-harmonic energy -20 dB (Off) vs -50 dB (2x and 4x). 2x and 4x are identical here because aliases at 2x fold above 24 kHz and are removed by the decimator; the -50 dB floor comes from base-rate stages, not the oversampler. 4x has no measured benefit on this test.
- CPU (Linux g++ -O3, one core, 48 kHz/512, includes test-signal generation): Off ~1.9 %, 2x ~3.0 %, 4x ~4.5 %.
- Fixed all -Wall -Wextra warnings in dsp headers (LufsMeter, AutoAnalyzer).
Not verified: the new CMake option itself on Windows (the same sources build and pass in a Linux harness), Windows CI, FL Studio PDC, real-material voicing.
