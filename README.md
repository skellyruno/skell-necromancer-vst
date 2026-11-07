# SKELL-NECROMANCER

**RAISE THE DEAD MIX** · original SKELL-branded JUCE audio effect · source prototype v0.1.0

This is a complete, self-contained **source package** for a VST3 + Standalone target, starting from the structure of the `skellyruno/skell-vst-template` JUCE/CMake project. The WAV Monopoly screenshots were used as **layout and material references only**. No reference artwork, rune sprite, trademark, or DSP code has been copied.

> **Status:** Latest Windows JUCE VST3/Standalone compilation, DSP regression test, and VST3 packaging **passed** ([GitHub Actions #42403563396](https://github.com/skellyruno/skell-necromancer-vst/actions/runs/42403563396)). FL Studio host-load testing, reference listening, metering conformity, and CPU benchmarks remain outstanding. A green build is not mastering-grade certification.

## Functional architecture

`Input trim → Revive (Tape → Glue → Harmonic Life → Transient Restore → Glow, depending on mode) → Revive parallel mix + click-free enable fade → 4-band Tone → output trim → oversampled-detection lookahead limiter / Push → output`.

There is also a delayed dry reference for `BYPASS` and level-matched `DELTA LISTEN` and an optional M/S width path in `MASTER`. MONO → STEREO has a conservative, optional, low-level upper-mid decorrelation. Parameter values use APVTS; DSP telemetry uses atomics. The processor has no heap allocations in `processBlock`.

### Implemented controls

- Main REANIMATE (default 65%), REVIVE ON, BYPASS, AUTO, VOCAL / MASTER / BUS.
- Compact window now opens at **S 70%** (approximately **810×428**) by default, including a one-time migration of older saved windows. Optional M 100%, L 125%, XL 150% remain. The first-run tour is opt-in from Help rather than blocking the title.
- Slider/button labels use ASCII-safe text for reliable rendering with optional font fallbacks. REVIVE and full bypass transitions are smoothed; the limiter enable/disable also crossfades while preserving its lookahead delay.
- Input, Output, LOW, MID (I/II/III ranges), HIGH, HF ADJUST; parallel MIX and optional DELTA LISTEN and gain-matched BYPASS.
- LIMITER and PUSH (0–24 dB), -0.1/-0.3/-1.0 dBTP ceiling choices, optional soft clip; oversampling Off/2×/4×, M/S width and mono-to-stereo.
- Stereo input/output bars with peak-hold ticks, gain-reduction, 400 ms momentary / 3 s short-term / approximate gated-integrated LUFS, and target setting (-14, -9, or custom in half-LU increments).
- Factory presets INIT, Dead Vocal Rescue, Raise the Master, Gentle Resurrection, Crypt Warmth, Bone Air, Grave Glue, Loud and Alive. User preset XML import/export, preset name field and dirty indicator, undo/redo, A/B slots (copy and recall) and UI state persistence.
- Original seeded 400-glyph rune field with cached color tiers, metallic cap, 270° gradient arc, tour/help, menus, resizable fixed aspect layout in S/M/L/XL sizes, VBlank updates (and low-rate text refresh).

### AUTO: two intentional passes

1. Start the DAW transport and press **AUTO**. The detector reads 12 seconds, keeps the loudest 3-second analysis, and suggests conservative EQ/restoration parameters. If the host reports transport stopped or audio is too quiet, AUTO aborts.
2. The footer will ask for the loudest region to be replayed. **Seek to that section yourself and press AUTO again.** The plug-in monitors 3 seconds and sets an approximate PUSH/OUTPUT starting point. The DAW does not expose a portable plug-in API for seeking arbitrary host transport positions; automatic seeking is deliberately not claimed.

AUTO suggestions are bounded and host gesture notifications are sent from the message thread, not the audio thread. **AUTO is a starting-point assistant, not an infallible reference-matching or ±0.5 LU mastering service.**

## Build

Requires CMake 3.22+, a C++17 toolchain, JUCE 8.0.12 (or a compatible JUCE 7+ source checkout), and Windows Visual Studio or macOS Xcode. Internet is needed on the first CMake configure **unless** you pass `JUCE_PATH`.

### Windows (MSVC 2022 or newer)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DJUCE_PATH="C:/JUCE"
cmake --build build --config Release --parallel 2
```

If you use Visual Studio 2026, select the generator installed on your system and keep the rest unchanged. Expected paths:

```text
build/SkellNecromancer_artefacts/Release/VST3/Skell-Necromancer.vst3
build/SkellNecromancer_artefacts/Release/Standalone/Skell-Necromancer.exe
```

### macOS (Xcode/Clang)

```bash
cmake -S . -B build -G Xcode -DJUCE_PATH="$HOME/JUCE"
cmake --build build --config Release --parallel 2
```

Expected paths:

```text
build/SkellNecromancer_artefacts/Release/VST3/Skell-Necromancer.vst3
build/SkellNecromancer_artefacts/Release/Standalone/Skell-Necromancer.app
```

Without `JUCE_PATH`, the project follows the SKELL starter template's `FetchContent` path to JUCE 8.0.12. The included `.github/workflows/build.yml` compiles Windows VST3 and Standalone, packages the VST3, and uploads a GitHub Actions artifact. macOS is supported by local CMake/Xcode instructions but is not currently a GitHub Actions target. The workflow runs on source changes and cancels superseded runs.

## Original art assets

| Asset | Exact dimensions | PNG alpha | Role |
|---|---:|---|---|
| `Assets/bg_brushed.png` | 1157 × 612 | Opaque | Gunmetal background, vignette, panel lines |
| `Assets/dial_well.png` | 440 × 440 | Transparent outside circle | Recessed dial well and metal rings |
| `Assets/knob_cap_big.png` | 256 × 256 | Transparent outside circle | Runtime-scaled ~210px metal cap, faint original skull/bones engraving |
| `Assets/knob_cap_small.png` | 96 × 96 | Transparent outside circle | Runtime-scaled ~70px dark-knurled cap |
| `Assets/rune_atlas.png` | 256 × 160 | Transparent | 8×5 sheet of 40 original 32px geometric glyph sprites |

The deterministic generator is `tools/generate_assets.py` (Pillow and NumPy). These images are ready to swap for a final art pass without changing the DSP.

**Fonts:** The source archive deliberately does not distribute font files. By default the UI requests installed `Orbitron` and `JetBrains Mono` families with standard system fallbacks. To embed properly licensed local fonts into your own build, add CMake arguments `-DSKELL_DISPLAY_FONT="C:/path/Orbitron.ttf" -DSKELL_MONO_FONT="C:/path/JetBrainsMono.ttf"`. They will be copied into the local CMake build directory, embedded using `juce_add_binary_data`, and loaded by `Theme`. Keep font redistribution and licensing under your control.

## Checks executed here

```bash
python tests/test_assets.py
python tests/test_project_structure.py
python tests/test_eq_inverse.py
clang++ -std=c++17 -O2 -I tests/stubs tests/dsp_header_smoke.cpp -o dsp_header_smoke
./dsp_header_smoke
python tests/test_limiter.py
```

- **Passed:** PNG dimensions/alpha and full expected source tree; all APVTS IDs present.
- **Passed (independent numerical model, not the VST3):** boost-then-cut null-test for shelves/peaks at 44.1, 48, 96 kHz: residual lower than -90 dB, normally about -240 dB or lower for the test vector.
- **Passed:** DSP header compilation and 13 seconds of synthetic audio against a minimal JUCE math/parameter shim, no NaNs/Inf, and AUTO entered its second-pass waiting state.
- **Passed (limited artificial stress):** 4× reconstructed true-peak checks on 36 synthetic cases across 44.1/48/96 kHz with PUSH 0/12/24 dB at a -1.0 dBTP ceiling. The limiter uses a conservative safety margin. **This is not an ITU, EBU, or commercial limiter certification.**
- **Passed for the latest changes:** Windows CMake configuration, VST3/Standalone compilation, CTest streaming/AUTO/limiter smoke, source integrity check, and VST3 packaging/artifact upload.
- **Not executed for the latest changes:** VST3 host load, macOS/Xcode build, actual DAW UI rendering, reference LUFS validation, CPU profiling, reference vocal/master listening, high-quality polyphase oversampling checks, and bypass null tests during oversampling changes.

## Known engineering gaps / release blockers

1. The `Oversampler` is a streaming interpolation and two-pole-decimation **prototype**. It is not yet a production multi-stage half-band FIR/IIR oversampler, and its phase/delay cannot be treated as precisely calibrated across all quality settings. The quality changes use a short fade and latency update, but should be subjected to sample-accurate transition/host-PDC regression tests.
2. The true-peak limiter uses 4× interpolated **detection** with a 2.5ms lookahead plus a sliding maximum. It needs BS.1770/EBU/DAW verification across edge cases. A conservative safety margin means its actual ceiling may be slightly quieter than selected.
3. The loudness meter uses K-weight-like double biquads and 100ms buckets; LUFS/gating and AUTO target calibration are **approximate**, not independently validated against a conformance suite. The ±0.5 LU target is *not guaranteed*.
4. AUTO suggests correction from simplified spectral ratios and requires manual replay of the loudest region. It is not a trained classifier, machine-learning reference matcher, or DAW timeline editor. A/B and AUTO may group multiple host gestures into more than one undo transaction depending on the host.
5. Numeric right-click value entry and advanced tooltip guidance require further UI validation; JUCE slider popup controls are enabled, but a dedicated numeric text-entry bubble is not yet implemented. The original artwork is a replaceable generated draft.
6. No real listening validation, 10–20 clip benchmark results, CPU/latency profiles, or crash/host automation regression reports are included. **Do not ship this as a commercial mastering-grade plugin without these checks.**

## Suggested regression matrix

Create 10–20 rights-cleared clips representing dull/thin/harsh voices, muddy/narrow/flat masters, over-compressed buses, silence, DC offset, mono/stereo cases and fast transients. Repeat at 44.1/48/96/192 kHz and 64/128/512/2048 sample blocks. Test automation sweeps, ON/OFF, bypass null and host PDC, 4× true peaks with extreme input, oversampling transitions, A/B recall, XML preset round-trip, AUTO transport/quiet abort, UI resizing/HiDPI, and DAW close/reopen. Publish CPU and latency reports before declaring release-ready.

**No plugin binary is included**: the source must be built with JUCE first.
