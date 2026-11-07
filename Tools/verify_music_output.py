"""Check captured Mac mixer output against the actual imported music recordings."""
from pathlib import Path
import array, json, math, subprocess, wave, sys
import numpy as np

root = Path(__file__).resolve().parents[1]
folder = Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root / 'Verification/pranks/music'

def pcm(path, seconds=12):
    data = subprocess.check_output(['ffmpeg', '-v', 'error', '-i', str(path), '-t', str(seconds),
                                    '-ar', '8000', '-ac', '1', '-f', 'f32le', '-'])
    return np.frombuffer(data, dtype='<f4').astype(np.float64)

references = {
    'menu': pcm(next((root / 'ArtSource/Audio/Pixabay').glob('BGM_MainLoop*'))),
    'cancan': pcm(root / 'ArtSource/Audio/Gameplay/CanCan-BGM.mp3'),
    'chase': pcm(next((root / 'ArtSource/Audio/Pixabay').glob('BGM_Tension*'))),
}

def similarity(capture, reference):
    clip = capture[6400:25600]  # Skip crossfade and startup buffers.
    size = 1 << (len(reference) + len(clip) - 1).bit_length()
    cross = np.fft.irfft(np.fft.rfft(reference, size) * np.conj(np.fft.rfft(clip, size)), size)
    energy = np.r_[0, np.cumsum(reference * reference)]
    windows = energy[len(clip):] - energy[:-len(clip)]
    denom = np.sqrt(np.maximum(windows * np.dot(clip, clip), 1e-30))
    return float(np.max(np.abs(cross[:len(windows)]) / denom))

rows = []
for name, reference in [('menu','menu'), ('cancan','cancan'), ('muted','cancan'),
                        ('resumed','cancan'), ('chase','chase'),
                        ('held-chase','chase') if (folder/'output/held-chase.wav').exists() else ('returned','cancan')]:
    path = folder / 'output' / (name + '.wav')
    audio = pcm(path)
    rms = float(np.sqrt(np.mean(audio * audio)))
    correlation = similarity(audio, references[reference])
    passed = correlation < .2 if name == 'muted' else rms > .0001 and correlation > .6
    rows.append(dict(name=name,seconds=len(audio)/8000,rms_dbfs=20*math.log10(max(rms,1e-10)),
                     reference=reference,recording_similarity=correlation,passed=passed))
report = dict(source='independent Mac Shipping master-mix recording', passed=all(r['passed'] for r in rows), checks=rows)
(folder / 'output-levels.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
raise SystemExit(0 if report['passed'] else 1)
