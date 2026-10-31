"""Import decoded user-supplied CanCan WAV (CAN_CAN_WAV environment or /tmp/CanCanGameplay.wav)."""
import os,unreal
source=os.environ.get('CAN_CAN_WAV','/tmp/CanCanGameplay.wav')
t=unreal.AssetImportTask();t.filename=source;t.destination_path='/Game/Audio/Gameplay';t.destination_name='CanCanGameplay';t.automated=True;t.replace_existing=True;t.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
s=unreal.load_asset('/Game/Audio/Gameplay/CanCanGameplay')
if not isinstance(s,unreal.SoundWave):raise RuntimeError('CanCan audio import failed')
s.set_editor_property('looping',True);unreal.EditorAssetLibrary.save_loaded_asset(s)
unreal.log('CANCAN_GAMEPLAY_IMPORTED')
unreal.SystemLibrary.quit_editor()
