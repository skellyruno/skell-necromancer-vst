"""Numerical test for the RBJ coefficient formulas implemented in Source/dsp/Biquad.h.
Does not compile or execute the C++ plugin: this is an independent transfer-curve test.
"""
import math
import numpy as np
from scipy.signal import lfilter

def design(kind,sr,hz,db,q=.70710678):
    w=2*math.pi*min(sr*.45,max(5.,hz))/sr;c=math.cos(w);s=math.sin(w);A=10**(db/40)
    if kind=='peak':
        al=s/(2*q);b=[1+al*A,-2*c,1-al*A];a=[1+al/A,-2*c,1-al/A]
    else:
        root=2*math.sqrt(A)*s/math.sqrt(2)
        if kind=='lowShelf':
            b=[A*((A+1)-(A-1)*c+root),2*A*((A-1)-(A+1)*c),A*((A+1)-(A-1)*c-root)]
            a=[(A+1)+(A-1)*c+root,-2*((A-1)+(A+1)*c),(A+1)+(A-1)*c-root]
        else:
            b=[A*((A+1)+(A-1)*c+root),-2*A*((A-1)+(A+1)*c),A*((A+1)+(A-1)*c-root)]
            a=[(A+1)-(A-1)*c+root,2*((A-1)-(A+1)*c),(A+1)-(A-1)*c-root]
    return np.array(b)/a[0], np.array(a)/a[0]

def run():
    rng=np.random.default_rng(42)
    for sr in (44100,48000,96000):
        noise=rng.normal(0,.07,int(sr*.33))
        for kind,hz,db,q in [('lowShelf',100,3,.707),('peak',400,3,.62),('peak',1600,-3,.62),('peak',4000,3,.62),('highShelf',8000,3,.707),('highShelf',15000,-3,.707)]:
            bp,ap=design(kind,sr,hz,db,q)
            bm,am=design(kind,sr,hz,-db,q)
            y=lfilter(bm,am,lfilter(bp,ap,noise))
            res=20*math.log10(max(1e-15,np.sqrt(np.mean((y-noise)**2))/np.sqrt(np.mean(noise**2))))
            assert res<-90,(sr,kind,hz,res)
            print(f'{sr:6} Hz | {kind:10} {hz:5} Hz | null residual {res:7.1f} dB')
if __name__=='__main__':run()
