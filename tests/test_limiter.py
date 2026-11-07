"""Compiles a DSP-only probe against the tiny JUCE math shim, then measures
4x reconstructed sample peaks. Not a DAW/host compliance certification.
"""
from pathlib import Path
import subprocess,tempfile,sys
import numpy as np
from scipy.signal import resample_poly
root=Path(__file__).resolve().parents[1]
exe=Path(tempfile.gettempdir())/('necro_limiter_probe.exe' if sys.platform=='win32' else 'necro_limiter_probe')
subprocess.run(['clang++','-std=c++17','-O2','-I',str(root/'tests/stubs'),str(root/'tests/limiter_probe.cpp'),'-o',str(exe)],check=True)
for sr in (44100,48000,96000):
 for drive in (0,12,24):
  for typ in range(4):
   with tempfile.TemporaryDirectory() as tmp:
    dst=Path(tmp)/'out.bin'
    subprocess.run([str(exe),str(dst),str(sr),str(drive),'-1',str(typ)],check=True,stdout=subprocess.DEVNULL)
    x=np.fromfile(dst,dtype='<f4')
    tp=np.max(np.abs(resample_poly(x,4,1)))
    target=10**(-1/20)
    assert tp<=target*1.001,(sr,drive,typ,tp,target)
    print(f'{sr:6} Hz | push {drive:2} dB | stimulus {typ} | peak {20*np.log10(tp):6.2f} dBTP | ceiling -1.0 dBTP')
