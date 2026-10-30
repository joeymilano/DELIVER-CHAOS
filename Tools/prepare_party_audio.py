"""Offline placeholder dialogue and short original effects, no background music."""
from pathlib import Path
import math, random, struct, subprocess, wave
root = Path(__file__).resolve().parents[1]
out = root/'ArtSource'/'Audio'
out.mkdir(parents=True, exist_ok=True)
lines = {'OwnerCaught':'咪咪！你在干嘛！', 'OwnerGood':'还是我家咪咪最乖。', 'OwnerHome':'主人到家了！我的派对怎么了？', 'Meow':'喵！'}
for name, text in lines.items():
    subprocess.run(['espeak-ng','-v','cmn+f3','-s','165','-p','58','-w',str(out/(name+'.wav')),text],check=True)
random.seed(3)
for name, seconds, freq in [('Clink',.13,1800), ('Thud',.24,70), ('CameraOff',.28,500)]:
    rate=22050
    with wave.open(str(out/(name+'.wav')),'wb') as wav:
        wav.setparams((1,2,rate,0,'NONE','not compressed'))
        values=[]
        for i in range(int(rate*seconds)):
            t=i/rate
            tone=math.sin(2*math.pi*freq*t*(1-.45*t/seconds))
            noise=random.uniform(-1,1)*(.4 if name=='Thud' else .05)
            values.append(int(12000*math.exp(-t*20)*(tone+noise)))
        wav.writeframes(struct.pack('<'+'h'*len(values),*values))
(out/'README.md').write_text('''# Party audio\n\nOwnerCaught / OwnerGood / OwnerHome / Meow are temporary Mandarin synthetic voices rendered offline with eSpeak NG 1.52.0. Dialogue is original project text. The speech engine is not bundled. These are deliberately replaceable placeholders; they are not professional voice acting or a natural cat recording.\n\nClink / Thud / CameraOff are original short synthesized game effects produced by Tools/prepare_party_audio.py. No background music is generated or included.\n\nRun the script with espeak-ng installed, then Tools/import_party_assets.py in Unreal Editor to rebuild.\n''')
