# Skell-Necromancer 0.1.0 — Windows VST3 owner-test preview

This is an **unsigned owner-test preview**, not a commercial release. Artifact visibility follows the repository's visibility. The current repository is public, so existing GitHub Actions artifacts are publicly accessible; future product-artifact uploads are withheld by CI until the repository is private. Do not describe historical artifact URLs as private or share them as a secure distribution channel. Use a disposable DAW project until the separate FL Studio and sound-acceptance gates are complete.

## Install

1. Close FL Studio and any other host that may have Skell-Necromancer loaded.
2. Extract the complete owner-test package to a normal folder. Keep the build-info file, checksum manifest, installer, notices, and Skell-Necromancer VST3 bundle together.
3. Open PowerShell 7 in that extracted folder.
4. Validate the package without changing files by running:

   **.\Install-Skell-NecromancerPreview.ps1 -PackageRoot . -WhatIf**

5. If validation passes, install with explicit confirmation:

   **.\Install-Skell-NecromancerPreview.ps1 -PackageRoot .**

The default target is the per-user VST3 folder under LOCALAPPDATA, Programs/Common/VST3. No administrator elevation, registry edits, machine-wide install, or automatic updater is used. If your host does not scan that folder, add it to the host's VST3 search paths or pass a different Destination value.

## Update and rollback

When a verified Skell-Necromancer installation already exists, the installer verifies its identity and tree hash, backs it up under LOCALAPPDATA, Skelly/Backups/Skell-Necromancer, verifies the backup, stages the new bundle, and swaps it into place. Unknown or modified bundles are not overwritten.

Preview rollback first:

**.\Install-Skell-NecromancerPreview.ps1 -Rollback -WhatIf**

Then confirm:

**.\Install-Skell-NecromancerPreview.ps1 -Rollback**

Rollback restores only the immediately previous verified Skell-Necromancer bundle. The newer bundle is retained so rollback can be reversed. A first-time install has no previous version to restore. If you used custom Destination or BackupRoot values, pass the same values to rollback.

## Safety and acceptance

- WhatIf validates the package and proposed operation without writing files.
- The installer checks the complete SHA-256 inventory, provenance, strict VST3 JSON, both preserved processor/controller CIDs, AMD64 PE32+, and pluginval strictness 5.
- Reparse points, path traversal, overlapping package/install/backup paths, tampered bundles, unknown installations, and modified rollback targets are rejected.
- The installer does not alter other plugins, presets, host state, automation IDs, FL Studio projects, registry entries, or licensing files.
- The package is unsigned and preview-only. Do not distribute it publicly or treat CI success as FL Studio acceptance. Product artifact uploads are withheld while this repository is public; existing historical Actions artifacts remain public until expiry.
- FL Studio scan/load, old-project compatibility, preset/state/automation recall, real musical A/B, target-hardware CPU/latency, final third-party attribution, owner-approved vendor URL/support email/license, code signing, and commercial release approval remain separate gates.
