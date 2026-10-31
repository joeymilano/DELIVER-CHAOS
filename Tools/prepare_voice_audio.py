"""Normalize downloaded voice/cat source files without replacing missing originals."""
from pathlib import Path
import array, math, shutil, subprocess, wave
root=Path(__file__).resolve().parents[1]
ffmpeg=shutil.which('ffmpeg') or '/opt/homebrew/bin/ffmpeg'
audio=root/'ArtSource/Audio'
def convert(source,target,filters):
    subprocess.run([ffmpeg,'-y','-v','error','-i',str(source),'-af',filters,'-ar','44100','-ac','1','-c:a','pcm_s16le',str(target)],check=True)
for source,name in [('fthgurdy-528193.mp3','MeowSoft'),('tuberatanka-110011.mp3','Meow')]:
    target=audio/(name+'.wav')
    convert(audio/'FieldRecordings'/source,target,'highpass=f=100,lowpass=f=12000')
    with wave.open(str(target),'rb') as f:params=f.getparams();samples=array.array('h',f.readframes(f.getnframes()))
    peak=max(abs(x) for x in samples)
    if not peak:raise ValueError('Silent cat recording')
    gain=32767*10**(-6/20)/peak
    normalized=array.array('h',(round(x*gain) for x in samples))
    with wave.open(str(target),'wb') as f:f.setparams(params);f.writeframes(normalized.tobytes())
for source in (audio/'ElevenLabsLocal').glob('Owner*.mp3'):
    convert(source,source.with_suffix('.wav'),'highpass=f=85,lowpass=f=12000,loudnorm=I=-20:TP=-3:LRA=8')
