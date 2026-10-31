import json
from pathlib import Path
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
tools = unreal.AssetToolsHelpers.get_asset_tools()
unreal.SystemLibrary.execute_console_command(None, 'Interchange.FeatureFlags.Import.FBX 0')
report = []
for cat in ("Ahuai", "Jutuan", "Wumei", "Dafu"):
    source_cat = 'Ahuai' if cat == 'Dafu' else cat
    skeleton = unreal.load_asset('/Game/Characters/' + source_cat + '/SK_' + source_cat + '_Skeleton')
    for name in ("Push", "Swipe"):
        opt = unreal.FbxImportUI()
        opt.automated_import_should_detect_type = False
        opt.import_mesh = False
        opt.import_animations = True
        opt.import_materials = opt.import_textures = False
        opt.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
        opt.skeleton = skeleton
        opt.anim_sequence_import_data.set_editor_property('snap_to_closest_frame_boundary', True)
        task = unreal.AssetImportTask()
        task.filename = str(root / 'ArtSource/Models' / source_cat / 'Rig' / ('Gesture_' + name + '.fbx'))
        task.destination_path = '/Game/Characters/' + cat
        task.destination_name = 'A_' + name
        task.options = opt
        task.automated = task.replace_existing = task.save = True
        tools.import_asset_tasks([task])
        clip = unreal.load_asset(task.destination_path + '/A_' + name)
        if not isinstance(clip, unreal.AnimSequence) or clip.sequence_length <= 0:
            raise RuntimeError('Invalid clip: ' + cat + name)
        report.append(dict(cat=cat, clip=name, duration=clip.sequence_length))
out = root / 'Verification/cat-contact'
out.mkdir(parents=True, exist_ok=True)
(out / 'clips.json').write_text(json.dumps(report, indent=2))
unreal.log('CAT_CONTACT_CLIPS_IMPORTED')
