from pathlib import Path
import unreal,json,os
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));base=root/'ArtSource/FabLocal'
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary;report=[]
def imp(file,dest,name,opt=None):
 t=unreal.AssetImportTask();t.filename=str(file);t.destination_path=dest;t.destination_name=name;t.automated=True;t.replace_existing=True;t.save=True
 if opt:t.options=opt
 at.import_asset_tasks([t]);return unreal.load_asset(dest+'/'+name)
for group in (['Characters'] if os.environ.get('FAB_CHARACTERS_ONLY') else ['Furniture','Decor','Characters']):
 dest='/Game/FabLocal/'+group
 texfile=next((base/group).rglob('*.png'));tex=imp(texfile,dest,'T_Color')
 mat=unreal.load_asset(dest+'/M_Color') or at.create_asset('M_Color',dest,unreal.Material,unreal.MaterialFactoryNew());lib.delete_all_material_expressions(mat)
 t=lib.create_material_expression(mat,unreal.MaterialExpressionTextureSample);t.texture=tex;lib.connect_material_property(t,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
 r=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);r.r=.72;lib.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS);lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
 skeletons={}
 for file in sorted((base/group/'Prepared').glob('*.fbx'),key=lambda p:('Idle' not in p.stem,p.stem)):
  opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_materials=False;opt.import_textures=False
  sk=group=='Characters';opt.import_as_skeletal=sk;opt.import_mesh=True;opt.import_animations=sk
  opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH if sk else unreal.FBXImportType.FBXIT_STATIC_MESH
  if sk:
   who=file.stem.split('_')[0]
   if who in skeletons:opt.skeleton=skeletons[who]
  else:opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=True
  mesh=imp(file,dest,file.stem,opt)
  if not mesh:raise RuntimeError(file)
  if sk:
   skeletons[who]=mesh.skeleton
   # Reimporting a skeletal mesh alone leaves its existing animation unchanged.
   animopt=unreal.FbxImportUI();animopt.automated_import_should_detect_type=False;animopt.import_mesh=False;animopt.import_as_skeletal=True;animopt.import_animations=True;animopt.skeleton=mesh.skeleton;animopt.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION
   imp(file,dest,file.stem+'_Anim',animopt)
   arr=mesh.materials
   for a in arr:a.material_interface=mat
   mesh.materials=arr
  else:
   for i in range(mesh.get_num_sections(0)):mesh.set_material(i,mat)
  unreal.EditorAssetLibrary.save_loaded_asset(mesh);report.append(mesh.get_path_name())
for file in (root/'ArtSource/Audio/World').glob('*.wav'):
 sound=imp(file,'/Game/Audio/World',file.stem);sound.set_editor_property('looping',file.stem=='SneakySnitch');unreal.EditorAssetLibrary.save_loaded_asset(sound)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
(root/'Verification/fab-import.json').write_text(json.dumps(report,indent=2))
unreal.log('FAB_IMPORT_OK')
unreal.SystemLibrary.quit_editor()
