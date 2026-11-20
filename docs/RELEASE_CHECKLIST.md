# Skell-Necromancer release checklist

## Protected Windows VST3 baseline

- Product version: 0.1.0; format: Windows x64 VST3 only; no plugin Standalone target.
- Protected source head: 8df3a63cc0c011d435530aeeec13eff0d31bceec (PR #4).
- Exact-head Windows run: [#44153560771](https://github.com/skellyruno/skell-necromancer-vst/actions/runs/44153560771) — configure/build, VST3-only output assertion, CTest 8/8, source integrity, reference UI geometry, LUFS/true-peak references, JUCE UI capture, package verifier 17/17, and pluginval 1.0.4 strictness 5 passed.
- Protected artifact #14361011381 outer ZIP SHA-256: 3a9878a83d5fab62cac6ee45e4958c2e9bfcc257e39cfd35e02d62219ec317af.
- Inner VST3 ZIP SHA-256: ee5d1dd6f16a75818c5a299ae5418e180e5a818d6e26511f6f4d59b4a43c9890.
- VST3 module SHA-256: 6d5096ba6d580af35ff6cd88de9de9b606808ad879d7aec83ffef8ffd879e938 (5,180,928 bytes; AMD64 PE32+).
- Preserved processor/controller CIDs: ABCDEF019182FAEB536B4175536E4372 and ABCDEF011234ABCD536B4175536E4372.
- Baseline manifest records commercial_release=false, no product license, and empty vendor URL/support email. These values are intentionally not guessed.

## Current installer work

- Candidate branch: release/necromancer-private-preview-installer-rollback-20261118, stacked on PR #4's exact VST3-only head; [draft PR #5](https://github.com/skellyruno/skell-necromancer-vst/pull/5), head 92f2ea0340922d7795e234318381d570ec9e8152.
- Exact-head Windows run [#44430327542](https://github.com/skellyruno/skell-necromancer-vst/actions/runs/44430327542) passed build, VST3-only output assertion, CTest, source integrity, UI geometry, 24 LUFS references, 108 8x true-peak references, JUCE UI capture/rune motion, package verifier 17/17, pluginval 1.0.4 strictness 5, installer self-tests, and real-package WhatIf/no-write verification.
- Independent artifact inspection: VST3 artifact #14486882501 outer SHA-256 d06b159851e778c3a14cb7daa99649b1f482b2a1ed116293bff9539a033d0157; product ZIP SHA-256 916d94f9fc96f377d8c2c467c329269ad98487028c2ed91a62893be6bbaedf2a; module SHA-256 eb33daa39dfe67b8307001ef04bb4be4fb2720bafcdababefc7adb04f34cca97; strict moduleinfo SHA-256 42639509c1a67debfe73064a9d213f31492bd8c671f47d6ae39dea95fd9a77cc. Both preserved CIDs and AMD64 PE32+ verified.
- Installer artifact #14486772683 outer SHA-256 3fd0b04a0fab2c7e70d5594aca5787245783fb5df8552c242ab91edb526148a6; ZIP CRC and all 8/8 SHA-256 entries passed. The PowerShell installer uses SupportsShouldProcess/WhatIf; the 20-assertion self-test and real-package no-write check passed.
- Pluginval artifact #14486292403 SHA-256 8520b436a37a5fa34d615a889b179389b2ff8ffd30c7d13ef56076167480a300; strictness 5 passed. Its optional Steinberg VST3 validator subtest was skipped because no validator path was configured.
- UI artifact #14486137522 SHA-256 8e614bb28c9bf67a361ff75ad23426dd8c9772f8eed97a4f8c3aaa2e1899bcff. Real JUCE captures at 810, 1157 and 1736 px plus mid-range were inspected: no visible clipping/overlap; rune-before/after captures show motion. This is screenshot review, not FL Studio host acceptance.
- **Distribution warning:** the repository is public. The VST3 and installer artifacts from run #44430327542 are therefore publicly accessible, not private. They remain available until their existing expiry dates; they were not deleted. The updated workflow commit 086a939 adds a visibility gate so future product artifacts are uploaded only when repository visibility is private. Its exact-head CI run is still required.
- Keep PR #5 draft. The candidate is CI/package validated but not FL Studio/audio/commercial accepted.

## Release-readiness matrix

| Gate | Status | Evidence / next test |
|---|---|---|
| Windows VST3 build/download | Exact-head CI passed; private distribution blocked | Run #44430327542 passed; future product artifact uploads are withheld while the repo is public |
| DSP / synthetic metering | CI passed | CTest, 24 LUFS references and 108 8x true-peak references passed; real listening remains separate |
| pluginval | CI passed, strictness 5 | Exact staged VST3 passed pluginval 1.0.4; optional Steinberg validator was skipped (no path configured) |
| Real sound / musical A/B | UNVERIFIED | Rights-cleared real clips and owner listening approval |
| FL Studio host compatibility | UNVERIFIED | Scan/load, bypass, automation, save/reopen, old-project and PDC checks |
| Presets / state / automation recall | UNVERIFIED | Factory/user preset round-trip and host automation playback |
| CPU / latency on target hardware | UNVERIFIED | Measure on the owner's Windows machine at 44.1/48/96/192 kHz and representative block sizes |
| Licensing / attribution | Partial | Montserrat OFL included; full asset/dependency review and owner-approved root product license required |
| Installer / backup / rollback | CI passed | 20-assertion self-test and real-package WhatIf/no-write passed; owner-side install/update/rollback still needed |
| Private distribution | BLOCKED — repository public | Workflow now withholds VST3/installer product artifacts unless repository visibility is private; historical artifacts remain public until expiry |
| Updater | Not implemented | Keep manual owner-test installer; no auto-update until approved |
| Storefront / email signup | Separate website work | Coordinate with website automation; no deployment in this task |
| Release notes / support | Partial | Add owner-approved HTTPS vendor URL/support email and final release notes |
| Pricing / trial | Owner decision required | No prices, trial terms, payments or licensing claims invented |
| Signing / commercial release | Blocked | Approved metadata/license, signing and explicit release approval |

This document tracks evidence, not permission to publish. The package remains unsigned and owner-test-only. It is not privately distributed while the repository is public.
