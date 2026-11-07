from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]/'Assets'
expected={'bg_brushed.png':(1157,612),'dial_well.png':(440,440),'knob_cap_big.png':(256,256),'knob_cap_small.png':(96,96),'rune_atlas.png':(256,160)}
for fname,size in expected.items():
    im=Image.open(root/fname)
    assert im.size==size,(fname,im.size)
    if fname!='bg_brushed.png':
        assert im.mode=='RGBA',(fname,im.mode)
        assert im.getchannel('A').getextrema()[0]==0,(fname,'needs transparency')
    print(f'OK {fname}: {im.size} / {im.mode}')
