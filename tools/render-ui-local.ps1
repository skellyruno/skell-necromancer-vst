# Local Skell-Necromancer JUCE UI capture — run in PowerShell on Windows.
# No GitHub Actions, no packaging, and NO DSP changes required for UI review.
param(
  [string]$JucePath = "",
  [string]$BuildDir = "build-ui-preview",
  [string]$OutputDir = "ui-preview",
  [string]$ReferenceFile = "",
  [switch]$SkipBuild
)
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$build = Join-Path $root $BuildDir
$output = Join-Path $root $OutputDir
New-Item -ItemType Directory -Path $output -Force | Out-Null

if (-not $SkipBuild) {
  $cmakeArgs = @("-S", $root, "-B", $build, "-A", "x64",
    "-DSKELL_BUILD_UI_CAPTURE=ON", "-DSKELL_BUILD_PROCESSOR_TESTS=OFF")
  if ($JucePath) {
    $juceFull = (Resolve-Path $JucePath).Path
    $cmakeArgs += "-DJUCE_PATH=$juceFull"
  }
  Write-Host "Configuring local JUCE UI renderer..."
  & cmake @cmakeArgs
  if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }
  Write-Host "Building only the editor capture target (not full VST3)..."
  & cmake --build $build --config Release --target NecromancerUICapture --parallel 2
  if ($LASTEXITCODE -ne 0) { throw "Local editor capture build failed ($LASTEXITCODE)" }
}
$exe = Get-ChildItem -Path $build -Recurse -File -Filter "NecromancerUICapture.exe" |
  Where-Object { $_.FullName -match "[\\/]Release[\\/]" } |
  Select-Object -First 1 -ExpandProperty FullName
if (-not $exe) { throw "NecromancerUICapture.exe missing: build the UI target first" }

# Existing capture produces 70%, 100%, 150%, before and after frames.
# The capture executable validates that runes actually move during silence.
& $exe $output
if ($LASTEXITCODE -ne 0) { throw "Real-editor capture or motion check failed ($LASTEXITCODE)" }

$scenes = @("ui_1157.png","ui_810.png","ui_1736.png","rune_after.png")
foreach ($name in $scenes) {
  $file = Join-Path $output $name
  if (-not (Test-Path $file)) { throw "Missing required scene: $file" }
  $img = Get-Item $file
  if ($img.Length -lt 10000) { throw "Empty/invalid image: $file" }
  Write-Host ("PASS {0} ({1:N0} bytes)" -f $name, $img.Length)
}
Write-Host "Done. Four review images in: $output"
Write-Host "Other test artifact: rune_before.png (used for image-difference regression)"
if ($ReferenceFile) {
  $referencePath = (Resolve-Path $ReferenceFile).Path
  & python (Join-Path $PSScriptRoot "compare-ui.py") --reference $referencePath --actual (Join-Path $output "ui_1157.png") --output (Join-Path $output "reference-vs-necromancer.png")
  if ($LASTEXITCODE -ne 0) { throw "Reference comparison failed. Install Pillow: python -m pip install pillow" }
}
