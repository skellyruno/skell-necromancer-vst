#!/usr/bin/env python3
"""Deterministic, original, replaceable Skell-Necromancer PNG assets.
Run: python tools/generate_assets.py
Requires Pillow, numpy. No third-party glyphs or trademarks.
"""
from pathlib import Path
import math, random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
OUT=Path(__file__).resolve().parents[1]/'Assets'; OUT.mkdir(exist_ok=True)
rng=np.random.default_rng(13817)
W,H=1157,612
noise=rng.normal(0,1,(H,1)) * 2.1 + rng.normal(0,0.95,(H,W))
y=np.arange(H)[:,None];x=np.arange(W)[None,:]
vig=np.clip(1-.32*((x-W/2)/(W/2))**2-.28*((y-H/2)/(H/2))**2,.25,1)
base=(17+noise+3*np.sin(y/37)*np.cos(x/230))*vig
rgb=np.stack([base*.88,base,base*1.045],axis=-1)
arr=np.uint8(np.clip(rgb,0,255));im=Image.fromarray(arr,'RGB').convert('RGBA');d=ImageDraw.Draw(im,'RGBA')
for yy in (35,79,580): d.line([(0,yy),(W,yy)],fill=(105,118,122,85),width=1);d.line([(0,yy+1),(W,yy+1)],fill=(0,0,0,155),width=1)
for k in range(8):d.rounded_rectangle((8+k/2,84+k/2,W-8-k/2,574-k/2),radius=12,outline=(80,94,98,max(5,25-k*3)),width=1)
im.save(OUT/'bg_brushed.png')

def circle_asset(sz,outer,inner,small=False,ss=1):
    # ss>1 renders at ss-times resolution and box-downsamples for anti-aliased edges.
    zz=sz*ss; a=np.zeros((zz,zz,4),dtype=np.uint8)
    yy,xx=np.indices((zz,zz));dx=xx-(zz-1)/2;dy=yy-(zz-1)/2;r=np.sqrt(dx*dx+dy*dy)/(zz/2);ang=np.arctan2(dy,dx)
    shadow=np.clip(1-r,0,1)
    if outer:
        lum=(13+13*np.sin(10*r+1)+9*np.cos(2.7*ang))
        ring=((r>.93)&(r<.975))|((r>.865)&(r<.882))
        lum+=ring*65
        lum+=(r>.975)*0
        lum-=(r<.78)*6
    else:
        # Smooth machined-conic metallic sheen with faint circular brush lines.
        lum=137+43*np.cos(ang+.63)+23*np.cos(2*ang-1.5)+9*np.cos(4*ang+.28)
        lum+=4*np.cos(r*140)+2*np.cos(r*275)
        lum-=(r**2)*39
        lum+=rng.normal(0,2.8,(zz,zz))
        lum=np.clip(lum,38,231)
    lum=np.clip(lum,0,255).astype(np.uint8)
    a[:,:,:3]=np.stack([lum,lum,lum],axis=-1)
    a[:,:,3]=np.where(r<=.99,255,0)
    out=Image.fromarray(a,'RGBA'); dr=ImageDraw.Draw(out,'RGBA')
    if outer:
        for frac,c in ((.97,(190,205,205,95)),(.92,(3,5,5,240)),(.80,(44,55,58,180)),(.76,(0,0,0,215))):
            dr.ellipse((zz*(1-frac)/2,zz*(1-frac)/2,zz*(1+frac)/2,zz*(1+frac)/2),outline=c,width=3*ss)
        dr.arc((zz*.05,zz*.05,zz*.95,zz*.95),194,330,fill=(205,227,226,100),width=4*ss)
    else:
        for frac,c,wd in ((.97,(228,235,231,185),4),(.92,(5,8,9,200),4),(.875,(160,166,164,155),2)):
            dr.ellipse((zz*(1-frac)/2,zz*(1-frac)/2,zz*(1+frac)/2,zz*(1+frac)/2),outline=c,width=wd)
        if small:
            for i in range(72):
                t=2*math.pi*i/72;x0=zz/2+math.cos(t)*zz*.41;y0=zz/2+math.sin(t)*zz*.41
                x1=zz/2+math.cos(t)*zz*.455;y1=zz/2+math.sin(t)*zz*.455
                dr.line((x0,y0,x1,y1),fill=(5,5,6,160),width=2)
        else:
            # Original, faint skull and crossed bones relief, understated.
            cx,cy=zz*.5,zz*.47;s=zz*.18
            dr.ellipse((cx-s*.58,cy-s*.68,cx+s*.58,cy+s*.49),outline=(27,33,32,67),width=3)
            for ax in (-.24,.24):dr.ellipse((cx+ax*s-s*.14,cy-s*.03,cx+ax*s+s*.14,cy+s*.17),fill=(22,29,28,31))
            dr.polygon([(cx,cy+s*.13),(cx-s*.12,cy+s*.32),(cx+s*.12,cy+s*.32)],fill=(30,35,33,28))
            for sign in (-1,1):dr.line((cx-s*.55,cy+s*.65*sign,cx+s*.55,cy-s*.65*sign),fill=(15,22,21,40),width=4)
    if ss>1:out=out.convert('RGBa').resize((sz,sz),Image.BOX).convert('RGBA')
    return out
circle_asset(440,True,False,ss=4).save(OUT/'dial_well.png')
circle_asset(256,False,False).save(OUT/'knob_cap_big.png')
circle_asset(96,False,False,True).save(OUT/'knob_cap_small.png')
# Glyph atlas: exactly 40 original geometric glyphs in 8x5 32-pixel transparent cells.
scale=4; atlas=Image.new('RGBA',(8*32*scale,5*32*scale),(0,0,0,0));g=ImageDraw.Draw(atlas,'RGBA')
R=random.Random(42642)
for n in range(40):
    gx=(n%8)*32*scale;gy=(n//8)*32*scale;cx=gx+16*scale;cy=gy+16*scale
    r=R.randint(7,11)*scale;style=n%10;ink=(255,255,255,230);wd=2*scale
    t0=(n*17)%360
    if style==0: g.arc((cx-r,cy-r,cx+r,cy+r),45,315,fill=ink,width=wd)
    elif style==1: g.ellipse((cx-r,cy-r/2,cx+r,cy+r/2),outline=ink,width=wd);g.ellipse((cx-3*scale,cy-3*scale,cx+3*scale,cy+3*scale),fill=ink)
    elif style==2: g.line((cx,cy-r,cx+r,cy+r,cx-r,cy+r,cx,cy-r),fill=ink,width=wd)
    elif style==3: g.line((cx-r,cy+r,cx+r,cy-r),fill=ink,width=wd);g.arc((cx-r,cy-r,cx+r,cy+r),120,300,fill=ink,width=wd)
    elif style==4: g.line((cx-r,cy,cx+r,cy),fill=ink,width=wd);g.arc((cx-r//2,cy-r,cx+r//2,cy+r),0,360,fill=ink,width=wd)
    elif style==5: g.polygon([(cx,cy-r),(cx+r,cy),(cx,cy+r),(cx-r,cy)],outline=ink)
    elif style==6: g.arc((cx-r,cy-r,cx+r,cy+r),100,350,fill=ink,width=wd);g.arc((cx-r//2,cy-r//2,cx+r//2,cy+r//2),240,100,fill=ink,width=wd)
    elif style==7: g.line((cx,cy-r,cx,cy+r,cx-r,cy,cx+r,cy),fill=ink,width=wd);g.ellipse((cx-3*scale,cy-3*scale,cx+3*scale,cy+3*scale),outline=ink,width=wd)
    elif style==8: g.arc((cx-r,cy-r,cx+r,cy+r),20,280,fill=ink,width=wd);g.line((cx,cy-r//2,cx+r,cy+r),fill=ink,width=wd)
    elif style==9: g.polygon([(cx,cy-r),(cx+r,cy+r//2),(cx-r,cy+r//2)],outline=ink);g.line((cx,cy-r//3,cx,cy+r),fill=ink,width=wd)
    # Variation in outer spikes and markings
    if n%3==0:g.line((cx-r-2*scale,cy,cx-r+3*scale,cy),fill=ink,width=scale)
    if n%4==0:g.ellipse((cx+r//2,cy-r,cx+r//2+scale*2,cy-r+scale*2),fill=ink)
atlas.resize((256,160),Image.Resampling.LANCZOS).save(OUT/'rune_atlas.png')
print('Assets written:',*[f'{p.name} ({Image.open(p).size})' for p in sorted(OUT.glob('*.png'))],sep='\n - ')
