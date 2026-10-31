"""Run in the full Unreal Editor after prepare_cozy.py. Licensed assets stay local."""
import unreal,json,re
from pathlib import Path
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));base=root/'ArtSource/CozyLocal'
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary;report=[]
for row in json.loads((base/'manifest.json').read_text()):
 name=row['name'];dest='/Game/CozyLocal';opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_as_skeletal=False;opt.import_materials=False;opt.import_textures=False;opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=True
 t=unreal.AssetImportTask();t.filename=str(base/(name+'.fbx'));t.destination_path=dest;t.destination_name=name;t.options=opt;t.automated=True;t.replace_existing=True;t.save=True;at.import_asset_tasks([t]);mesh=unreal.load_asset(dest+'/'+name)
 if not isinstance(mesh,unreal.StaticMesh):raise RuntimeError(name)
 for index,slot in enumerate(mesh.static_materials):
  slotname=str(slot.material_slot_name)
  # FBX sanitizes material names. Match exact sanitized names, then token prefix.
  clean=lambda s:re.sub('[^a-z0-9]','',s.lower())
  matches=[m for m in row['materials'] if clean(m['name'])==clean(slotname)]
  if not matches:raise RuntimeError('Unmatched material '+name+' '+slotname)
  m=matches[0];mn='M_'+re.sub('[^a-zA-Z0-9_]','_',m['name']);mat=unreal.load_asset(dest+'/'+mn) or at.create_asset(mn,dest,unreal.Material,unreal.MaterialFactoryNew());lib.delete_all_material_expressions(mat)
  color=lib.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);color.constant=unreal.LinearColor(*m['color']);lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
  for prop,val in [(unreal.MaterialProperty.MP_ROUGHNESS,m['roughness']),(unreal.MaterialProperty.MP_METALLIC,m['metallic'])]:
   c=lib.create_material_expression(mat,unreal.MaterialExpressionConstant);c.r=val;lib.connect_material_property(c,'',prop)
  lib.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
  mesh.set_material(index,mat)
 unreal.EditorAssetLibrary.save_loaded_asset(mesh);report.append({'name':name,'materials':len(mesh.static_materials),'size':str(mesh.get_bounding_box())})
(root/'Verification/cozy-import.json').write_text(json.dumps(report,indent=2));unreal.log('COZY_IMPORT_OK');unreal.SystemLibrary.quit_editor()
