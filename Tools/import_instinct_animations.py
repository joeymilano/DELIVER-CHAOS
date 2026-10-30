import os, json, unreal
root=unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
tools=unreal.AssetToolsHelpers.get_asset_tools();report=[]
for file in sorted(os.listdir(os.path.join(root,'ArtSource','Party'))):
    if not file.endswith('.fbx') or unreal.EditorAssetLibrary.does_asset_exist('/Game/Art/Party/'+file[:-4]):continue
    opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_mesh=True;opt.import_as_skeletal=False;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opt.import_materials=True;opt.import_textures=False;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=True
    task=unreal.AssetImportTask();task.filename=os.path.join(root,'ArtSource','Party',file);task.destination_path='/Game/Art/Party';task.destination_name=file[:-4];task.options=opt;task.automated=True;task.replace_existing=True;task.save=True
    tools.import_asset_tasks([task])
lib=unreal.MaterialEditingLibrary
for name,color in [('Party_Orange',(1,.20,.025)),('Party_Plum',(.19,.035,.29)),('Party_Ink',(.018,.023,.025)),('Party_Stem',(.12,.21,.08)),('Party_Kraft',(.49,.30,.13)),('Party_Cream',(.94,.86,.68))]:
    m=unreal.load_asset('/Game/Art/Party/'+name)
    if not m:raise RuntimeError(name)
    lib.delete_all_material_expressions(m)
    c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.constant=unreal.LinearColor(*color,1)
    lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    r=lib.create_material_expression(m,unreal.MaterialExpressionConstant);r.r=.7;lib.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
    lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
for cat in ['Ahuai','Jutuan','Wumei']:
    skel=unreal.load_asset('/Game/Characters/'+cat+'/SK_'+cat+'_Skeleton')
    if not isinstance(skel,unreal.Skeleton):raise RuntimeError('Skeleton missing '+cat)
    for name in ['Sit','Charge','Swipe','Push','Pounce']:
        opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False
        opt.import_mesh=False;opt.import_animations=True;opt.import_materials=False;opt.import_textures=False
        opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION;opt.skeleton=skel
        opt.anim_sequence_import_data.set_editor_property('snap_to_closest_frame_boundary',True)
        task=unreal.AssetImportTask();task.filename=os.path.join(root,'ArtSource','Models',cat,'Rig','Gesture_'+name+'.fbx')
        task.destination_path='/Game/Characters/'+cat;task.destination_name='A_'+name;task.options=opt
        task.automated=True;task.replace_existing=True;task.save=True;tools.import_asset_tasks([task])
        a=unreal.load_asset('/Game/Characters/'+cat+'/A_'+name)
        if not isinstance(a,unreal.AnimSequence) or a.sequence_length<=0:raise RuntimeError('Animation failed '+cat+name)
        report.append({'cat':cat,'gesture':name,'duration':a.sequence_length})
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
with open(os.path.join(root,'Verification','instinct-animations.json'),'w') as f:json.dump(report,f,indent=2)
unreal.log('CAT_INSTINCT_ANIMATIONS_OK')
unreal.SystemLibrary.quit_editor()
