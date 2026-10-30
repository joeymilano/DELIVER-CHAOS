import unreal,os,json
root=unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
report=[]
for name in sorted(os.listdir(os.path.join(root,'ArtSource','Environment'))):
 if not name.endswith('.fbx'):continue
 opts=unreal.FbxImportUI();opts.automated_import_should_detect_type=False;opts.import_mesh=True;opts.import_as_skeletal=False;opts.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opts.import_materials=True;opts.import_textures=False;opts.static_mesh_import_data.combine_meshes=True;opts.static_mesh_import_data.auto_generate_collision=True
 t=unreal.AssetImportTask();t.filename=os.path.join(root,'ArtSource','Environment',name);t.destination_path='/Game/Art/Props';t.destination_name=name[:-4];t.options=opts;t.automated=True;t.replace_existing=True;t.save=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
 unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
 report+=list(t.imported_object_paths)
with open(os.path.join(root,'Verification','room-import.json'),'w') as f:json.dump(report,f,indent=2)
unreal.log('BAD_CAT_ROOM_ART_IMPORTED')
