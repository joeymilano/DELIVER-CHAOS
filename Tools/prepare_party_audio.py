"""Short original impact effects only, no background music."""
from pathlib import Path
import math, random, struct, subprocess, wave
root = Path(__file__).resolve().parents[1]
out = root/'ArtSource'/'Audio'
out.mkdir(parents=True, exist_ok=True)
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
