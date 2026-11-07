"""Independent references: pyloudnorm for integrated K-weighted LUFS and
SciPy polyphase 8x reconstruction for true peaks. Synthetic regression only.
Not a standards certification or an FL Studio test.
"""
import argparse
import subprocess
import tempfile
from pathlib import Path
import numpy as np
import pyloudnorm as pyln
from scipy.signal import resample_poly

parser=argparse.ArgumentParser()
parser.add_argument('--lufs',type=Path,required=True)
parser.add_argument('--limiter',type=Path,required=True)
args=parser.parse_args()
cases=0;worst=0.0
for sr in (44100,48000,96000):
    t=np.arange(sr*6,dtype=np.float64)/sr
    for typ in range(4):
        if typ==0: l=.1*np.sin(2*np.pi*1000*t);r=l
        elif typ==1: l=np.where(t<2,.03,.1)*np.sin(2*np.pi*450*t);r=.8*l
        elif typ==2: l=.11*np.sin(2*np.pi*80*t)+.04*np.sin(2*np.pi*5100*t);r=.9*l
        else: l=.08*np.sin(2*np.pi*900*t);r=.08*np.sin(2*np.pi*1100*t)
        for ch in (1,2):
            data=l if ch==1 else np.stack([l,r],axis=1)
            ref=float(pyln.Meter(sr).integrated_loudness(data))
            raw=subprocess.check_output([str(args.lufs),str(sr),str(typ),str(ch)],text=True)
            measured=float(raw.split()[0])
            err=abs(measured-ref);worst=max(worst,err);cases+=1
            assert err<.12,(sr,typ,ch,measured,ref)
print(f'Integrated LUFS: {cases} reference comparisons passed; worst error {worst:.4f} LU')

cases=0;worst=-float('inf')
with tempfile.TemporaryDirectory() as folder:
    dst=Path(folder)/'out.bin'
    for sr in (44100,48000,96000):
        for ceiling in (-.1,-.3,-1.0):
            for push in (0,12,24):
                for stimulus in range(4):
                    subprocess.run([str(args.limiter),str(dst),str(sr),str(push),
                                    str(ceiling),str(stimulus)],check=True,
                                   stdout=subprocess.DEVNULL)
                    audio=np.fromfile(dst,dtype='<f4')
                    peak=float(np.max(np.abs(resample_poly(audio.astype(np.float64),8,1))))
                    db=20*np.log10(max(peak,1e-12))
                    worst=max(worst,db-ceiling);cases+=1
                    assert db<=ceiling+.05,(sr,ceiling,push,stimulus,db)
print(f'True peak: {cases} 8x reference comparisons passed; nearest ceiling margin {worst:+.3f} dB')
