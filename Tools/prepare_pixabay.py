"""Decode selected PR #2 tracks; ordinary meow/eating remain candidates."""
from pathlib import Path
import subprocess,json
root=Path(__file__).resolve().parents[1];base=root/'ArtSource/Audio/Pixabay';out=Path('/tmp/delivery-pixabay');out.mkdir(exist_ok=True)
items=[('Menu','BGM_MainLoop*',None),('Chase','BGM_Tension*',None),('Win','Jingle*',5.2),('Swipe','SFX_Swipe*',1.03),('Landing','SFX_Pounce*',.73),('Glass','SFX_Shatter*',1.94),('Fusion','SFX_Fusion*',3.07),('Key','SFX_Key*',1.1),('Cash','SFX_Cash*',2.09),('MeowLoud','SFX_MeowEvil*',1.8)]
report=[]
for name,pattern,duration in items:
 file=next(base.glob(pattern));cmd=['/opt/homebrew/bin/ffmpeg','-y','-i',str(file)]
 if duration:cmd+=['-t',str(duration)]
 fade='afade=t=in:d=0.01'+(',afade=t=out:st='+str(max(0,duration-.06))+':d=0.06' if duration else '')
 cmd+=['-af','loudnorm=I='+('-20' if name in ['Menu','Chase'] else '-18')+':TP=-2:LRA=8,'+fade,'-ar','44100','-ac','2' if name in ['Menu','Chase','Win'] else '1','-c:a','pcm_s16le',str(out/(name+'.wav'))]
 subprocess.run(cmd,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True);report.append({'name':name,'source':file.name,'loop':name in ['Menu','Chase'],'trim_seconds':duration})
(out/'manifest.json').write_text(json.dumps(report,indent=2))
