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

- Candidate branch: release/necromancer-private-preview-installer-rollback-20261118, stacked on PR #4's exact head.
- Adds a PowerShell 7 per-user install/update/backup/rollback path, full package checksum verification, safe staging, explicit confirmation, WhatIf, and a 20-assertion self-test adapted to the preserved Necromancer product identity.
- Exact-head Windows validation is pending. Do not mark the installer or new package accepted until its new CI steps pass and the uploaded artifact is independently inspected.

## Release-readiness matrix

| Gate | Status | Evidence / next test |
|---|---|---|
| Windows VST3 build/download | Baseline passed; installer package pending | Re-run exact-head Windows build and inspect new artifact |
| DSP / synthetic metering | Baseline CI passed | Re-run CTest and independent reference checks |
| pluginval | Baseline strictness 5 passed | Validate the exact staged VST3 again |
| Real sound / musical A/B | UNVERIFIED | Rights-cleared real clips and owner listening approval |
| FL Studio host compatibility | UNVERIFIED | Scan/load, bypass, automation, save/reopen, old-project and PDC checks |
| Presets / state / automation recall | UNVERIFIED | Factory/user preset round-trip and host automation playback |
| CPU / latency on target hardware | UNVERIFIED | Measure on the owner's Windows machine at 44.1/48/96/192 kHz and representative block sizes |
| Licensing / attribution | Partial | Montserrat OFL included; full asset/dependency review and product license need owner approval |
| Installer / backup / rollback | Pending exact-head CI | Self-test, package WhatIf, update, backup integrity and reversible rollback |
| Updater | Not implemented | Keep manual preview installer; no auto-update until approved |
| Storefront / email signup | Separate website work | Coordinate with website automation; no deployment in this task |
| Release notes / support | Partial | Add owner-approved support URL/email and final release notes |
| Pricing / trial | Owner decision required | No prices, trial terms, payments or licensing claims invented |
| Signing / commercial release | Blocked | Approved metadata/license, signing and explicit release approval |

This document tracks evidence, not permission to publish. The package remains private-preview-only and unsigned.
