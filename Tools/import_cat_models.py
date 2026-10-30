import os, json
import unreal
root=unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
tools=unreal.AssetToolsHelpers.get_asset_tools()
reports=[]
for cat in ['Ahuai','Jutuan','Wumei']:
    if os.environ.get('BAD_CAT_NAME') and os.environ['BAD_CAT_NAME']!=cat:continue
    folder=os.path.join(root,'ArtSource','Models',cat)
    for skeletal in [False,True]:
        name=('SK_' if skeletal else 'SM_')+cat
        source=os.path.join(folder,'Rig',name+'.fbx') if skeletal else os.path.join(folder,name+'.fbx')
        if not os.path.isfile(source):continue
        options=unreal.FbxImportUI()
        options.set_editor_property('automated_import_should_detect_type',False)
        options.set_editor_property('import_mesh',True)
        options.set_editor_property('import_as_skeletal',skeletal)
        options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_SKELETAL_MESH if skeletal else unreal.FBXImportType.FBXIT_STATIC_MESH)
        options.set_editor_property('import_animations',skeletal)
        if skeletal:
            options.anim_sequence_import_data.set_editor_property('snap_to_closest_frame_boundary',True)
        options.set_editor_property('import_materials',True)
        options.set_editor_property('import_textures',True)
        if not skeletal:options.static_mesh_import_data.set_editor_property('combine_meshes',True)
        task=unreal.AssetImportTask();task.filename=source;task.destination_path='/Game/Characters/'+cat
        task.destination_name=name;task.options=options;task.automated=True;task.replace_existing=True;task.save=True
        tools.import_asset_tasks([task])
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
        reports.append({'cat':cat,'skeletal':skeletal,'assets':list(task.imported_object_paths)})
        for path in unreal.EditorAssetLibrary.list_assets('/Game/Characters/'+cat,recursive=True):
            a=unreal.load_asset(path)
            if isinstance(a,unreal.AnimSequence):
                original=a.get_name()
                for match in ['Idle','Run','Walk']:
                    if 'Canine'+match in original:
                        target='/Game/Characters/'+cat+'/A_'+match
                        if not unreal.EditorAssetLibrary.does_asset_exist(target):
                            unreal.EditorAssetLibrary.rename_asset(path,target)
        unreal.EditorAssetLibrary.save_directory('/Game/Characters/'+cat,only_if_is_dirty=False,recursive=True)
with open(os.path.join(root,'Verification','model-import.json'),'w') as f:json.dump(reports,f,indent=2)
unreal.log('BAD_CAT_MODELS_IMPORTED')
