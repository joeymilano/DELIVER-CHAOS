"""Check a complete Shipping mixer recording, not merely that a BGM component exists."""
from pathlib import Path
import csv,json,subprocess,sys
import numpy as np

root=Path(__file__).resolve().parents[1]
folder=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'Verification/chase-music'

def pcm(path):
    return np.frombuffer(subprocess.check_output(['ffmpeg','-v','error','-i',str(path),'-ar','8000','-ac','1','-f','f32le','-']),dtype='<f4').astype(np.float64)

def match(haystack,needle):
    n=1 << (len(haystack)+len(needle)-1).bit_length()
    cross=np.fft.irfft(np.fft.rfft(haystack,n)*np.conj(np.fft.rfft(needle,n)),n)
    energy=np.r_[0,np.cumsum(haystack*haystack)]
    denom=np.sqrt(np.maximum((energy[len(needle):]-energy[:-len(needle)])*np.dot(needle,needle),1e-30))
    scores=np.abs(cross[:len(denom)])/denom
    lag=int(np.argmax(scores));return float(scores[lag]),lag/8000

audio=pcm(folder/'full-chase.wav')
source=pcm(next((root/'ArtSource/Audio/Pixabay').glob('BGM_Tension*')))
main=pcm(root/'ArtSource/Audio/Gameplay/CanCan-BGM.mp3')
result=json.loads((folder/'result.json').read_text(encoding='utf-8-sig'))
rows=list(csv.DictReader((folder/'timeline.csv').open()))
score,at=match(audio[:12*8000],source[3*8000:6*8000]);delay=at-3
windows=[]
for seconds in [5,35,70,105,121]:
    # Imported/cooked audio and the source MP3 have different resampling paths.
    # Bound local alignment to 50 ms, so clock drift is not mistaken for a cut.
    a=audio[round((seconds+delay-.05)*8000):round((seconds+delay+1.85)*8000)]
    b=source[seconds*8000:round((seconds+1.8)*8000)]
    correlation,offset=match(a,b)
    windows.append(dict(source_seconds=seconds,similarity=correlation,local_alignment_seconds=offset-.05,passed=correlation>.6))
protected=[r for r in rows if 4<float(r['elapsed'])<result['source_duration']-1]
continuous=bool(protected) and all(r['variant']=='2' and r['playing']=='1' and r['paused']=='0' for r in protected)
returned=[float(r['elapsed']) for r in rows if r['variant']=='1']
can_can_match,_=match(main[:12*8000],audio[-3*8000:-8000])
passed=result['complete'] and result['full_passes']==1 and continuous and all(r['passed'] for r in windows) and bool(returned) and min(returned)>=result['source_duration']-.75 and can_can_match>.6
report=dict(passed=passed,source_seconds=len(source)/8000,recorded_seconds=len(audio)/8000,initial_match=score,start_delay_seconds=delay,protected_samples=len(protected),continuous_after_pursuit_and_round_end=continuous,windows=windows,returned_to_cancan_at=min(returned) if returned else None,cancan_similarity=can_can_match)
(folder/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2));raise SystemExit(0 if passed else 1)
