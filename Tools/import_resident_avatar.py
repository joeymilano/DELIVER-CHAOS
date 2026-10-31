"""Run in the full editor after prepare_resident_avatar.py. Assets stay local."""
from pathlib import Path
import unreal,json
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
base=root/'ArtSource/ResidentsLocal/Prepared';dest='/Game/ResidentsLocal';at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
def imp(file,name,options=None):
    t=unreal.AssetImportTask();t.filename=str(file);t.destination_path=dest;t.destination_name=name;t.automated=True;t.replace_existing=True;t.save=True
    if options:t.options=options
    at.import_asset_tasks([t]);return unreal.load_asset(dest+'/'+name)
materials={}
for row in json.loads((base/'materials.json').read_text()):
    name='M_'+row['name'];m=unreal.load_asset(dest+'/'+name) or at.create_asset(name,dest,unreal.Material,unreal.MaterialFactoryNew());lib.delete_all_material_expressions(m)
    if row.get('texture'):
        tex=imp(base/row['texture'],'T_'+row['name']);c=lib.create_material_expression(m,unreal.MaterialExpressionTextureSample);c.texture=tex;lib.connect_material_property(c,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    else:
        c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.constant=unreal.LinearColor(*row['color']);lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    r=lib.create_material_expression(m,unreal.MaterialExpressionConstant);r.r=row['roughness'];lib.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS);lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);materials[row['name']]=m
skeleton=None
for clip in ['Idle','Walk','Run','Confused']:
    file=base/f'Owner_{clip}.fbx';opt=unreal.FbxImportUI();opt.automated_import_should_detect_type=False;opt.import_as_skeletal=True;opt.import_mesh=clip=='Idle';opt.import_animations=True;opt.import_materials=False;opt.import_textures=False
    opt.mesh_type_to_import=unreal.FBXImportType.FBXIT_SKELETAL_MESH if clip=='Idle' else unreal.FBXImportType.FBXIT_ANIMATION
    if skeleton:opt.skeleton=skeleton
    asset=imp(file,f'Owner_{clip}' if clip=='Idle' else f'Owner_{clip}_Anim',opt)
    if not asset:raise RuntimeError(clip)
    if clip=='Idle':
        skeleton=asset.skeleton;arr=asset.materials
        for slot in arr:
            key=str(slot.material_slot_name)
            if key not in materials:raise RuntimeError('Unknown material slot '+key)
            slot.material_interface=materials[key]
        asset.materials=arr;unreal.EditorAssetLibrary.save_loaded_asset(asset)
        # Mesh import can skip the animation on reimport; always import it explicitly.
        anim=unreal.FbxImportUI();anim.automated_import_should_detect_type=False;anim.import_as_skeletal=True;anim.import_mesh=False;anim.import_animations=True;anim.skeleton=skeleton;anim.mesh_type_to_import=unreal.FBXImportType.FBXIT_ANIMATION
        imp(file,'Owner_Idle_Anim',anim)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
unreal.log('RESIDENT_AVATAR_IMPORTED')
unreal.SystemLibrary.quit_editor()
