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
- FL Studio host PDC and commercial audio-quality certification remain unverified. See docs/FL_STUDIO_PDC_TEST.md.
