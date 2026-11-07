# FL Studio latency / phase-alignment acceptance test

This is a **manual host test**, not a claim that the current VST3 has passed FL Studio validation. The standalone CTest suite validates FIR group delay and dry-path nulls without a DAW; host compensation and automation still require these checks in FL Studio.

## Expected plugin latency (samples)

The limiter reserves a fixed 2.5ms lookahead even when LIMITER is off. REANIMATE has two cascaded nonlinear stages, each using 32 original-rate samples at 2x or 48 at 4x.

| Sample rate | OFF | 2x | 4x |
| --- | ---: | ---: | ---: |
| 44,100 Hz | 111 | 175 | 207 |
| 48,000 Hz | 120 | 184 | 216 |
| 96,000 Hz | 240 | 304 | 336 |

Reported plugin delay should equal limiter lookahead + (2 × FIR stage delay). VST3 host PDC should remove that delay between mixer tracks. The total reported delay changes when oversampling changes; avoid switching it during critical live recording.

## Manual steps

1. Install the latest **Skell-Necromancer-Windows-VST3** artifact. Restart FL Studio and rescan VST plugins so the new version loads.
2. Set FL Studio **Mixer menu → Plugin delay compensation → Automatic** to ON. Disable any manual track-delay offsets before the test.
3. Make two copies of a short impulse or sharp rimshot, on separate mixer tracks and routed to the same output. Leave the first track completely dry. Insert Necromancer on the second.
4. For the timing/null test: set REANIMATE=0%, LOW/MID/HIGH/HF=0 dB, INPUT/OUTPUT=0 dB, LIMITER=OFF, GAIN MATCH=OFF, DELTA=OFF, WIDTH=100%, and MONO TO STEREO=OFF. Give the plugin at least 0.25s of preroll to finish the bypass/limiter fades.
5. Check the wrapper **Settings → Info → Latency** display. Confirm OFF, 2x, 4x report the expected samples above at your session rate; check that changing LIMITER ON/OFF does *not* alter PDC.
6. Render the two mixer tracks independently at 32-bit float, identical start/end and no normalization, then inspect the impulse onsets. With Automatic PDC enabled, the rendered onsets should align to the same sample.
7. For a more sensitive check, invert the polarity of one track, play both through the same bus and inspect the residual. The neutral chain should cancel after the initial fade; if not, note the sample offset and whether it depends on oversampling.
8. Repeat at 44.1, 48, and 96 kHz; then test 64, 128, 512 and 2048-sample audio buffers. Try changing oversampling while transport is stopped and restarting playback. Check preset reload and automation synchronization.
9. Save screenshots of wrapper latency and the dry/processed impulse alignment. Record host version, audio interface, buffer size, sample rate, mode, and any nonzero residual.

Image-Line documentation: https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/mixer_trackprops.htm and https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/plugins/wrapper.htm

Passing offline tests does not substitute for this FL Studio test.
