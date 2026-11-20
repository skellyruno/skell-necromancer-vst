#!/usr/bin/env python3
"""Side-by-side reference vs actual JUCE UI capture; requires Pillow.
python tools/compare-ui.py --reference reference.png --actual ui-preview/ui_1157.png
"""
from __future__ import annotations
import argparse
from pathlib import Path
from PIL import Image, ImageDraw, ImageOps, ImageStat

def mean_luma(rgb: Image.Image) -> float:
    return float(ImageStat.Stat(rgb.convert("L")).mean[0])

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference", type=Path, required=True)
    parser.add_argument("--actual", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("ui-preview/reference-vs-necromancer.png"))
    args = parser.parse_args()
    if not args.reference.is_file() or not args.actual.is_file():
        parser.error("Reference and actual capture must both exist")
    ref = Image.open(args.reference).convert("RGB")
    actual = Image.open(args.actual).convert("RGB")
    rw, rh = actual.size
    if rw < 400 or rh < 200:
        parser.error("Capture dimensions are unexpectedly small")
    ref = ImageOps.fit(ref, (rw, rh), method=Image.Resampling.LANCZOS, centering=(.5,.5))
    spacing = 14
    margin = 30
    canvas = Image.new("RGB", (rw*2+spacing, rh+margin), (10,12,14))
    canvas.paste(ref, (0, margin))
    canvas.paste(actual, (rw+spacing,margin))
    draw = ImageDraw.Draw(canvas)
    draw.text((12,8), "REFERENCE", fill="#f1f1ef")
    draw.text((rw+spacing+12,8), "SKELL-NECROMANCER (ACTUAL JUCE RENDER)", fill="#c8fbbf")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(args.output)
    regions = {
       "header":(0,0,rw,int(rh*.13)),
       "metal faceplate":(0,int(rh*.14),rw,int(rh*.94)),
       "footer":(0,int(rh*.94),rw,rh),
    }
    print(f"A/B saved: {args.output}")
    for label,box in regions.items():
       print(f"{label:15} reference luminance={mean_luma(ref.crop(box)):.1f}, "
             f"Skell luminance={mean_luma(actual.crop(box)):.1f}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
