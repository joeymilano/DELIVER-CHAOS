import unreal,json
from pathlib import Path
rows=[]
for path in ['/Game/Audio/Gameplay/CanCanGameplay','/Game/Audio/Pixabay/Menu','/Game/Audio/Pixabay/Chase']:
 s=unreal.load_asset(path)
 if not isinstance(s,unreal.SoundWave):raise RuntimeError('Missing music: '+path)
 before={k:str(s.get_editor_property(k)) for k in ['looping','volume','virtualization_mode','loading_behavior','priority']}
 s.set_editor_property('looping',not path.endswith('/Chase'))
 s.set_editor_property('virtualization_mode',unreal.VirtualizationMode.PLAY_WHEN_SILENT)
 s.set_editor_property('loading_behavior',unreal.SoundWaveLoadingBehavior.FORCE_INLINE)
 s.set_editor_property('priority',100.0)
 s.modify()
 if not unreal.EditorAssetLibrary.save_loaded_asset(s,only_if_is_dirty=False):raise RuntimeError('Save failed: '+path)
 rows.append({'path':path,'duration':s.duration,'before':before,'after':{k:str(s.get_editor_property(k)) for k in before}})
p=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))/'Verification/pranks/music';p.mkdir(parents=True,exist_ok=True)
(p/'assets.json').write_text(json.dumps(rows,indent=2)+'\n')
unreal.log('BGM_ASSETS_OK '+json.dumps(rows));unreal.SystemLibrary.quit_editor()
