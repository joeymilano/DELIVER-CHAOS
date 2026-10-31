"""Repair existing local avatar materials without touching meshes, rigs or animation imports."""
from pathlib import Path
import unreal
lib=unreal.MaterialEditingLibrary
mesh=unreal.load_asset('/Game/ResidentsLocal/Owner_Idle')
if not isinstance(mesh,unreal.SkeletalMesh):raise RuntimeError('Owner mesh missing')
mesh.modify()
slots=mesh.get_editor_property('materials')
if len(slots)!=8:raise RuntimeError('Expected eight supplied body-part materials')
for i in range(len(slots)):
    slot=slots[i];name=str(slot.material_slot_name)
    mat=unreal.load_asset('/Game/ResidentsLocal/M_'+name)
    if not isinstance(mat,unreal.Material):raise RuntimeError('Material missing: '+name)
    node=lib.get_material_property_input_node(mat,unreal.MaterialProperty.MP_BASE_COLOR)
    if not isinstance(node,unreal.MaterialExpressionTextureSample):raise RuntimeError('Colour texture not connected: '+name)
    tex=node.get_editor_property('texture')
    if not tex or tex.get_name()!='T_'+name:raise RuntimeError('Wrong colour texture: '+name)
    tex.set_editor_property('srgb',True);unreal.EditorAssetLibrary.save_loaded_asset(tex)
    mat.set_editor_property('used_with_skeletal_mesh',True)
    errors=lib.recompile_material(mat)
    if errors:raise RuntimeError(str(errors))
    unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
    slot.set_editor_property('material_interface',mat);slots[i]=slot
mesh.set_editor_property('materials',slots)
if not unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False):raise RuntimeError('Owner material save failed')
for i,slot in enumerate(mesh.get_editor_property('materials')):
    if not slot.material_interface or slot.material_interface.get_name()!='M_'+str(slot.material_slot_name):raise RuntimeError('Material assignment did not persist: '+str(i))
unreal.log('OWNER_COLOUR_REPAIRED: 8 saved material slots')
unreal.SystemLibrary.quit_editor()
