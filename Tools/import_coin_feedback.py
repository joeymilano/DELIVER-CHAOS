"""Import a short coin cue derived from the owner's PR #2 cash-register recording."""
from pathlib import Path
import unreal,subprocess
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
source=root/'ArtSource/Audio/Pixabay/SFX_CashRegister_cash-register-open-567194.mp3';wav=Path('/tmp/DeliveryCoinBurst.wav')
subprocess.run(['/opt/homebrew/bin/ffmpeg','-y','-i',str(source),'-t','0.65','-af','loudnorm=I=-20:TP=-2:LRA=7,afade=t=in:d=0.006,afade=t=out:st=0.54:d=0.11','-ar','44100','-ac','1','-c:a','pcm_s16le',str(wav)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
t=unreal.AssetImportTask();t.filename=str(wav);t.destination_path='/Game/Audio/Pixabay';t.destination_name='CoinBurst';t.automated=True;t.replace_existing=True;t.save=True;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);s=unreal.load_asset('/Game/Audio/Pixabay/CoinBurst')
if not isinstance(s,unreal.SoundWave):raise RuntimeError('Coin cue import failed')
s.set_editor_property('looping',False);unreal.EditorAssetLibrary.save_loaded_asset(s);unreal.log('COIN_CUE_IMPORTED');unreal.SystemLibrary.quit_editor()
