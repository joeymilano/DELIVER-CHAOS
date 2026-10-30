"""Unreal Editor import with explicit glTF PBR channel reconstruction; offline runtime assets."""
from pathlib import Path
import json,unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
tools=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
names=['Dafu','PetCamera','HalloweenCakeV2'];report=[]
def import_file(source,dest,name,opts=None):
 t=unreal.AssetImportTask();t.filename=str(source);t.destination_path=dest;t.destination_name=name;t.automated=True;t.replace_existing=True;t.save=True
 if opts:t.options=opts
 tools.import_asset_tasks([t]);return unreal.load_asset(dest+'/'+name)
for name in names:
 folder=root/'ArtSource/V2Fun'/name
 if not (folder/('SM_'+name+'.fbx')).exists():continue
 dest='/Game/Characters/Dafu' if name=='Dafu' else '/Game/Art/V2Fun/'+name
 textures={}
 for f in folder.glob('T_*.png'):
  t=import_file(f,dest,f.stem);t.set_editor_property('max_texture_size',2048)
  if f.stem.endswith('_1'):
   t.set_editor_property('srgb',False);t.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_MASKS)
  unreal.EditorAssetLibrary.save_loaded_asset(t);textures[f.stem]=t
 mats=[]
 for definition in json.loads((folder/'material-report.json').read_text()):
  m=unreal.load_asset(dest+'/'+definition['name']) or tools.create_asset(definition['name'],dest,unreal.Material,unreal.MaterialFactoryNew());lib.delete_all_material_expressions(m)
  base=lib.create_material_expression(m,unreal.MaterialExpressionTextureSample);base.set_editor_property('texture',textures['T_'+name+'_0']);base.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
  lib.connect_material_property(base,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
  orm=lib.create_material_expression(m,unreal.MaterialExpressionTextureSample);orm.set_editor_property('texture',textures['T_'+name+'_1']);orm.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
  lib.connect_material_property(orm,'G',unreal.MaterialProperty.MP_ROUGHNESS);lib.connect_material_property(orm,'B',unreal.MaterialProperty.MP_METALLIC)
  lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);mats.append(m)
 for skeletal in ([False,True] if name=='Dafu' else [False]):
  meshname=('SK_' if skeletal else 'SM_')+name;file=folder/(meshname+'.fbx')
  if not file.exists():continue
  opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_mesh=True;opt.import_as_skeletal=skeletal;opt.import_animations=False;opt.import_materials=False;opt.import_textures=False;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH if skeletal else unreal.FBXImportType.FBXIT_STATIC_MESH
  if skeletal:opt.skeleton=unreal.load_asset('/Game/Characters/Ahuai/SK_Ahuai_Skeleton')
  else:opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=True
  mesh=import_file(file,dest,meshname,opt)
  if not mesh:raise RuntimeError('Mesh import failed: '+meshname)
  if skeletal:
   arr=mesh.get_editor_property('materials')
   for i,m in enumerate(mats):
    if i<len(arr):arr[i].material_interface=m
   mesh.set_editor_property('materials',arr)
  else:
   for i,m in enumerate(mats):mesh.set_material(i,m)
   unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem).set_convex_decomposition_collisions(mesh,4 if name=='PetCamera' else 1,16,100000)
  unreal.EditorAssetLibrary.save_loaded_asset(mesh);report.append({'asset':mesh.get_path_name(),'class':mesh.get_class().get_name()})
 if name=='Dafu':
  portrait=import_file(root/'ArtSource/References/Generated/dafu-v1.png','/Game/UI/Cats','T_Dafu');portrait.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI);unreal.EditorAssetLibrary.save_loaded_asset(portrait)
  for anim in ['Idle','Walk','Run','Sit','Charge','Swipe','Push','Pounce']:
   if not unreal.EditorAssetLibrary.does_asset_exist(dest+'/A_'+anim):unreal.EditorAssetLibrary.duplicate_asset('/Game/Characters/Ahuai/A_'+anim,dest+'/A_'+anim)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
(root/'Verification/v2fun-import.json').write_text(json.dumps(report,indent=2)+'\n')
unreal.log('V2FUN_IMPORT_OK')
