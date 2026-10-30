"""Optional local audio from the user's supplied OIIA reference. No remote calls."""
from pathlib import Path
import unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
source=root/'ArtSource/Audio/LocalReference/OiiaReference.wav'
if source.exists():
    task=unreal.AssetImportTask()
    task.filename=str(source);task.destination_path='/Game/Audio/LocalReference';task.destination_name='OiiaReference'
    task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    sound=unreal.load_asset('/Game/Audio/LocalReference/OiiaReference')
    if not isinstance(sound,unreal.SoundWave):raise RuntimeError('Reference audio failed to import')
    sound.set_editor_property('looping',False)
    unreal.EditorAssetLibrary.save_loaded_asset(sound)
    unreal.log('OIIA_LOCAL_AUDIO_OK')
else:
    unreal.log('Optional OIIA audio absent; visual celebration remains available')
