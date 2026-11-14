#!/usr/bin/env python3
"""Static JUCE layout assertions derived from the supplied 1512x798 reference.
Actual screenshots are separately checked by NecromancerUICapture.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
editor = (root / "Source/PluginEditor.cpp").read_text(encoding="utf-8")
theme = (root / "Source/Theme.h").read_text(encoding="utf-8")
meters = (root / "Source/ui/PeakMeter.h").read_text(encoding="utf-8")
top = (root / "Source/ui/TopBar.h").read_text(encoding="utf-8")
preset = (root / "Source/ui/PresetBar.h").read_text(encoding="utf-8")

def find(content, pattern, label):
    matches = re.findall(pattern, content)
    assert len(matches) == 1, f"{label}: expected one definition, found {len(matches)}"
    return tuple(map(int, matches[0]))

def rect(name, source=editor):
    return find(source, rf"\b{re.escape(name)}\.setBounds\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", name)

def button(name):
    return find(editor, rf"setTextButton\(\s*{name}\s*,\s*\"[^\"]+\"\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)", name)

def overlaps(a, b):
    return max(a[0],b[0])<min(a[0]+a[2],b[0]+b[2]) and max(a[1],b[1])<min(a[1]+a[3],b[1]+b[3])

dial=find(theme, r"\bdial\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)", "dial")
controls={n:rect(n) for n in ("low","mid","high","hf","input","output","range","inMeter","outMeter","mode","mix","mixValue","push","pushValue","autopsy","lufs","gr","status")}
buttons={n:button(n) for n in ("autoButton","onButton","bypassButton","limiterButton","deltaButton","matchButton")}
for name,(x,y,w,h) in {"dial":dial,**controls,**buttons}.items():
    assert x>=0 and y>=0 and w>0 and h>0 and x+w<=1157 and y+h<=612, f"{name} outside panel"
assert dial==(358,99,440,440)
assert abs((dial[0]+dial[2]/2)-578.5)<1
for name in ("low","mid","high","hf","input","output","range","inMeter","outMeter","mode"):
    assert not overlaps(dial,controls[name]), f"{name} overlaps REANIMATE"
for a,b in (("low","mid"),("high","hf"),("inMeter","input"),("outMeter","output"),("mode","low")):
    assert not overlaps(controls[a],controls[b]), f"{a}/{b} overlap"
for a,b in (("autoButton","onButton"),("onButton","bypassButton")):
    assert not overlaps(buttons[a],buttons[b]), f"{a}/{b} overlap"
for name in ("autoButton","onButton","bypassButton"):
    assert buttons[name][1]+buttons[name][3]<=80, f"{name} enters main panel"
assert controls["mode"][0]+controls["mode"][2]<dial[0]
for name in ("deltaButton","matchButton"):
    assert buttons[name][1]>=dial[1]+dial[3],f"{name} overlaps dial"
assert controls["mix"][1]>=dial[1]+dial[3]
assert re.search(r"barX\s*=\s*22\s*\+\s*c\s*\*\s*27",meters)
assert re.search(r"headerH\s*=\s*13",meters)
assert re.search(r"barH\s*=\s*getHeight\(\)\s*-\s*35\s*-\s*headerH",meters)
for name,xref in (("inMeter",78),("outMeter",994)):
    x,y,w,h=controls[name]
    assert x==xref and y+13+1==98 and y+13+1+(h-35-13)==375 and 22+27+15<=w
assert rect("presets",top)==(72,5,610,27)
for name in ("name","drop","undo","redo","save","abA","abB","copy"):
    x,y,w,h=rect(name,preset)
    assert x>=0 and y>=0 and x+w<=610 and y+h<=27
for factor in (.70,1.0,1.25,1.50):
    W,H=round(1157*factor),round(612*factor)
    assert W>=809 and H>=428 and (dial[0]+dial[2])*factor<=W+1 and (dial[1]+dial[3])*factor<=H+1
print("PASS: reference alignment, no component intersections, meter scale, 70-150% bounds")

# Preserve the user-approved original engraved centre cap instead of hiding it.
dial_source = (root / "Source/ui/ReanimateDial.h").read_text(encoding="utf-8")
runes_source = (root / "Source/ui/RuneRing.h").read_text(encoding="utf-8")
assert "g.drawImage(cap," in dial_source and "polishedFace" not in dial_source
assert "mg.setColour(tint(p)" in runes_source, "moving rune glyphs must not be black"

assert "dial_well_pngSize).createCopy()" in dial_source, "bezel edits must not mutate shared cached original asset"

# Rune CPU work is scheduled at paint cadence, not thrown away on alternating VBlanks.
assert "const bool renderFrame=" in dial_source
assert "if(renderFrame){ring.advance(now,live);repaint();}" in dial_source
# Reference-like blue range lamp and brushed-panel balance remain UI-only.
assert "juce::Colour(0xff83b8ff)" in (root / "Source/ui/RangeSelector.h").read_text(encoding="utf-8")

assert "framePacer.shouldRender(dt,fallback)" in dial_source, "frame cadence must not halve on 30Hz hosts"
assert "UiFramePacer.h" in dial_source

# A private image copy is mandatory: the original Skell art cannot be
# recoloured in ImageCache or masked per animation frame.
assert "dial_well_pngSize).createCopy()" in dial_source
assert "rim.strokePath(quietZone,juce::PathStrokeType(38.f))" in dial_source
assert "g.drawImage(cap," in dial_source and "polishedFace" not in dial_source
assert dial_source.index("rim.strokePath(quietZone") < dial_source.index("void paint(")

# The original engraved skull cap is visible, resized to match the reference.
assert "g.drawImage(cap,{cx-97.f,cy-97.f,194.f,194.f})" in dial_source
assert "headingFade(a)" in runes_source and "headingFade(p.angle)" in runes_source
laf=(root / "Source/ui/LookAndFeel.h").read_text(encoding="utf-8")
assert 'const bool abSlot=b.getButtonText()=="A"||b.getButtonText()=="B"' in laf
