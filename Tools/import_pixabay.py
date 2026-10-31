import unreal,json
from pathlib import Path
out=Path('/tmp/delivery-pixabay');report=[]
for row in json.loads((out/'manifest.json').read_text()):
 t=unreal.AssetImportTask();t.filename=str(out/(row['name']+'.wav'));t.destination_path='/Game/Audio/Pixabay';t.destination_name=row['name'];t.automated=True;t.replace_existing=True;t.save=True;unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);s=unreal.load_asset(t.destination_path+'/'+row['name'])
 if not isinstance(s,unreal.SoundWave):raise RuntimeError(row['name'])
 s.set_editor_property('looping',row['loop']);unreal.EditorAssetLibrary.save_loaded_asset(s);report.append(dict(row,duration=s.duration))
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));(root/'Verification/pixabay-import.json').write_text(json.dumps(report,indent=2));unreal.log('PIXABAY_IMPORT_OK');unreal.SystemLibrary.quit_editor()
