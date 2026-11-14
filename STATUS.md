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

## 2026-11-12 Reference-driven UI pass + switch de-clicking
UI (verified by headless render at 810/1157/1736 px, dial filmstrip, scripted knob tests; layout/motion follow the reference video, artwork/glyphs/branding stay original):
- Dial: rune field rebuilt as 640 original glyphs in 5 cached layers that shimmer independently, plus a travelling highlight cone (speed/brightness follow live level). Field fades in/out (~150 ms) with REVIVE on/off and BYPASS; a bypassed, settled dial repaints nothing. Animation repaints at 30 Hz (~3.6 ms per frame in the software renderer). Glyphs are confined inside the bezel and the lower arc is kept quiet for the % readout.
- Arc/ticks dim to grey when inactive (empty well like a bypassed unit); knob cap stays bright.
- Fixed a latent alpha bug: `drawImage` takes its alpha from the current colour, which left the metal knob caps drawn at 50% (and would have faded the dial cap in bypass). Opacity is now reset before each cap.
- Meters: dim ghost segments show the full LED strip; heavy outline removed.
- One main rounded instrument panel replaces the four boxed groups. Vector icons (undo, redo, save, settings, help) and power-icon pill buttons (AUTO/ON/BYPASS/LIMITER); letter-spaced title; MID range is now a vertical I/II/III dot selector (still a ComboBox, same parameter attachment); mode dropdown moved inside the panel; lighter knob halos.
DSP:
- Oversampling change no longer clicks: output fades to silence (5 ms) with the old configuration, then resets/switches latency, holds one latency period while the fresh delay lines fill, then fades in (25 ms). Previously a hard cut (0.28 step on a 0.3 sine).
- DELTA listen now crossfades over 15 ms (previously a hard switch, 0.29 step).
- New stress check T10: max sample-to-sample step on a steady 1 kHz tone while toggling BYPASS, REVIVE, LIMITER, MODE, OVERSAMPLING, DELTA, GAIN MATCH is <= 1.5x the tone's own max step in every case.
- Stress test remains: 135/135 configs, latency exact at Off/2x/4x, NaN/Inf survival, ceiling, silence/denormals, mono, alias, CPU (this run on a busier machine: ~2.7 / 4.1 / 5.5 % of one core at Off/2x/4x, includes test-signal generation).
Not verified: Windows CI, FL Studio host behaviour (PDC with the new fade-out/in on oversampling changes, animation smoothness at the host's frame rate), Windows font rendering, the optional CMake stress target on Windows.

## 2026-11-12 Bottom-area cleanup
- DELTA, MIX slider and GAIN MATCH now form one symmetric row under the dial (centred on x=568, inside the panel border; previously GAIN MATCH sat on the panel edge next to OUTPUT).
- Removed the redundant "REVIVE / TONE / LOUDNESS" caption.
- Status message moved from the panel's bottom-left (collided with the INPUT label on long messages) into a two-line footer block: status above, dimmed LUFS / GR readouts below.
- Verified: headless render at 810/1157/1736 px, 7/7 knob tests, stress test passes, project/asset tests pass. Not verified in a real host or on Windows.

## 2026-11-13 typography + polish
- Bundled Montserrat SemiBold (SIL OFL, licence in Assets/fonts) as default display font; override via SKELL_DISPLAY_FONT.
- Rune glyph roaming clusters/flashes (sprite blits, ~3.4 ms/frame).
- Meter unlit segments brighter (.22), dial arc 4px.
- Verified: harness render at 3 sizes, knob-edit tests pass. Not verified: Windows CI, FL Studio.

## 2026-11-13 Reference-video motion + visual refinement (draft PR #1)
- Inspected the user-provided 10.9 s video as a full 2 fps overview, plus a 10 fps crop from the active interval. The artwork remains aligned while bright rune highlights travel; bypass clears the field.
- RuneRing: changed short-lived randomly respawning illuminated clusters into three continuous orbiting sweeps; kept 640 original glyph placements, 5 cached layers, 16 short flashes, original sprites, bypass fade and repaint throttling.
- PeakMeter: 15 px LED columns, brighter ghost segments, pink/lavender/cyan scale.
- REANIMATE dial: thinner 2.7 px gradient arc; metal cap and dial geometry unchanged.
- Main panel title: slightly smaller with wider tracking.
- **Local validation:** verified the 10 fps extraction and continuity of the mathematical sweep model; inspected the exact changes as a Git patch; no real JUCE UI render is claimed.
- **Still required:** Windows compilation/CTest and reference DSP tests from PR CI; screenshot comparisons at 810, 1157 and 1736 px; run-time paint budget comparison vs the ~3.4 ms prior harness result; FL Studio bypass/PDC/resize and audio checks. Brushed-metal texture still needs reference review.
- Work isolated to `feat/reference-rune-sweep-20261113`; `main` untouched.

## 2026-11-13 Correction: physically moving runes (follow-up)
- User correctly reported the earlier sweep-only update did not move the glyphs themselves.
- Updated `RuneRing.h`: **480/640 original glyphs now travel on three circular orbits**, one band counter-rotating, even at zero live-audio level. The other 160 stay in dim cached layers for depth.
- Each moving glyph is excluded from the static cached layer to prevent visible duplicates. Separate native-size cached sprites preserve rune geometry, while the existing enlarged sprites are retained only for glow flares.
- Flare positions follow the current glyph positions; the bottom arc is dynamically dimmed as glyphs travel so the REANIMATE percentage remains readable.
- No per-frame image transform or heap allocation was added; movement uses precomputed glyph sprites and per-frame position updates.
- `ReanimateDial` now has a host-safe 30 Hz message-thread timer when VBlank stalls; fallback repaints on every fallback tick. The editor's 10 Hz timer also syncs active/live state when VBlank is missing.
- Extracted JUCE-free `RuneMotion.h` geometry and added `NecromancerUI_RuneMovement` CTest. **Local g++ -std=c++17 -Wall -Wextra -Werror test PASSED**, including 30 frames of travel in silence, per-frame continuity, radius and direction, and 480 mobile glyph count. Measured one-second path displacement: 66.44 / 56.02 / 47.37 px at representative radii.
- **Still unverified:** actual JUCE rendered frame sequence, animation CPU at three window sizes, latest Windows CI build, and FL Studio host callback/resize behavior. Source change is isolated to the draft PR branch, not main.

## 2026-11-13 UI-only chrome + motion refinement (no DSP changes)
- Local Windows screenshot path added: `tools/render-ui-local.ps1` builds **only** NecromancerUICapture (incremental) and produces four review images without GitHub Actions after the first JUCE setup. Use `-JucePath` for offline local source; `-SkipBuild` to re-capture existing binary.
- REANIMATE face: adds a **cached** machine-polished metal centre image over the existing big-cap rim; removes the busy centre graphic without deleting the original cap asset. Generation is once during editor creation, not per frame.
- 480 genuinely moving glyphs preserved; flare clusters narrowed and de-intensified; the base field remains visible in silence. All ring changes are UI-only.
- Small knobs: less neon-green tick glare, brighter chrome readouts, subtler hover, preserved original small cap.
- Faceplate: modestly brighter brushed steel; no striped resize-corner overlay; no change to resize limits.
- Meters: brighter unlit pink/lavender/cyan LEDs; no fake peak-hold line at negative-infinity; silence readout uses -INF.
- Top settings icon replaced with toothed gear to distinguish it from a sun.
- Pending review: real 810/1157/1736 screenshots, motion-frame differences, UI paint budget, Windows compilation and FL Studio manual interaction test. DSP processing code explicitly unchanged.

## 2026-11-13 Real screenshot correction
- Actual Windows JUCE screenshot revealed wavy moire on the first polygon-wedge cap face at the 100% size.
- Replaced cap construction with continuous per-pixel angular luminance using juce::Image::BitmapData, still generated **only once** on editor initialization. No per-frame gradient/wedge painting, no audio-thread work.
- Restored slightly more base brightness to the 480 moving glyphs (without re-introducing the large white flare hotspots).
- Moved REANIMATE into the top-of-dial heading position seen in the reference, with a translucent dark readability strip; retained the percent readout at the bottom.
- Windows rebuild / capture review pending. No DSP source changes.

## 2026-11-13 UI review: reference-level meter screenshots and first-frame polish
- Fixed empty MIX, PUSH, LUFS, GR and status labels on initial open by calling the editor's existing update method at construction. No DSP/parameter-value changes.
- Increased minimum mobile glyph size and weight while retaining 480 physically orbiting runes, the tuned dimming and all existing cached sprites.
- Slightly improved small knob text readability at 70%; hoisted a per-paint repeated tick computation outside the tick loop.
- `tests/ui_capture.cpp` now activates **only the visual screenshot's** meter levels at IN -0.4/-0.7 dB and OUT -0.1/-0.3 dB. Audio is not processed and these synthetic levels never appear in the shipping plugin.
- Screenshot target additionally reports six actual editor snapshot times as informational UI rasterization numbers, NOT an audio-thread/FL Studio CPU benchmark.
- Changes isolated to UI and capture harness. DSP source unaffected.
- Windows CI, visual inspection, 70% legibility, FL Studio interaction, and frame-time review remain release gates.

## 2026-11-13 Meter UI fallback and time-correct ballistics
- PeakMeter now has an independent **UI-only** 30 Hz fallback timer when hosts stop sending VBlank events. The existing host VBlank callback is still the preferred high-refresh update route.
- Fixed hold timing: peak hold is now measured in real milliseconds rather than assumed 17 ms frames. Previous fallback decays could keep hold levels longer than the configured 0.5 / 1 / 2 s.
- Meter decay scales with elapsed UI time rather than frame count; nonfinite level display is clamped to silence. This affects on-screen animation ONLY; metering calculations and audio processing are unchanged.
- The editor 10 Hz status callback also applies meter-hold settings even without VBlank.
- Latest capture visual QA and FL Studio no-VBlank host testing still need to confirm this fallback at runtime.
