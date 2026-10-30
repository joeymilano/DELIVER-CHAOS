from pathlib import Path
import unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
files=[(root/'ArtSource/Audio'/f'{n}.wav','/Game/Audio') for n in ['Meow','MeowSoft']]
files += [(p,'/Game/Audio/ElevenLabsLocal') for p in (root/'ArtSource/Audio/ElevenLabsLocal').glob('*.wav')]
for source,destination in files:
    task=unreal.AssetImportTask();task.filename=str(source);task.destination_path=destination;task.destination_name=source.stem
    task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    sound=unreal.load_asset(destination+'/'+source.stem)
    if not isinstance(sound,unreal.SoundWave):raise RuntimeError(str(source))
    sound.set_editor_property('looping',False);unreal.EditorAssetLibrary.save_loaded_asset(sound)
unreal.log('VOICE_REFRESH_OK')
