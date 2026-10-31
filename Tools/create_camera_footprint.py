"""Create a depth-tested vertex-colour material for the ground surveillance graphic."""
import unreal
path='/Game/Art/M_CameraFootprint'
m=unreal.load_asset(path)
if not m:
    m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_CameraFootprint','/Game/Art',unreal.Material,unreal.MaterialFactoryNew())
lib=unreal.MaterialEditingLibrary
lib.delete_all_material_expressions(m)
m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
m.set_editor_property('two_sided',True)
m.set_editor_property('disable_depth_test',False)
color=lib.create_material_expression(m,unreal.MaterialExpressionVertexColor)
strength=lib.create_material_expression(m,unreal.MaterialExpressionConstant);strength.r=3
bright=lib.create_material_expression(m,unreal.MaterialExpressionMultiply)
assert lib.connect_material_expressions(color,'',bright,'A')
assert lib.connect_material_expressions(strength,'',bright,'B')
assert lib.connect_material_property(bright,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
assert lib.connect_material_property(color,'A',unreal.MaterialProperty.MP_OPACITY)
lib.recompile_material(m)
unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log('CAMERA_FOOTPRINT_MATERIAL_READY')
unreal.SystemLibrary.quit_editor()
