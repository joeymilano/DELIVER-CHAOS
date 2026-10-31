"""Read saved owner slots in a fresh editor process, including texture and usage checks."""
from pathlib import Path
import json,unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
mesh=unreal.load_asset('/Game/ResidentsLocal/Owner_Idle');lib=unreal.MaterialEditingLibrary;report=[]
if not isinstance(mesh,unreal.SkeletalMesh):raise RuntimeError('Owner mesh missing')
slots=mesh.get_editor_property('materials')
if len(slots)!=8:raise RuntimeError('Expected the supplied eight material slots')
for i,slot in enumerate(slots):
    name=str(slot.material_slot_name);mat=slot.material_interface
    if not isinstance(mat,unreal.Material) or mat.get_name()!='M_'+name:raise RuntimeError('Unassigned or wrong material: '+name)
    if not mat.get_editor_property('used_with_skeletal_mesh'):raise RuntimeError('Skeletal material usage missing: '+name)
    node=lib.get_material_property_input_node(mat,unreal.MaterialProperty.MP_BASE_COLOR)
    if not isinstance(node,unreal.MaterialExpressionTextureSample):raise RuntimeError('Colour texture not connected: '+name)
    tex=node.get_editor_property('texture')
    if not tex or tex.get_name()!='T_'+name or not tex.get_editor_property('srgb'):raise RuntimeError('Wrong colour texture or colour space: '+name)
    report.append({'slot':i,'name':name,'material':mat.get_path_name(),'texture':tex.get_path_name(),'srgb':True,'skeletal_usage':True})
folder=root/'Verification/owner-colour';folder.mkdir(exist_ok=True)
(folder/'after-materials.json').write_text(json.dumps(report,indent=2))
unreal.log('OWNER_MATERIALS_VERIFIED: 8 slots, 8 matching sRGB textures')
unreal.SystemLibrary.quit_editor()
