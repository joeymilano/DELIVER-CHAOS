"""Normalize imported prop colors and preserve painted cat readability in shadow."""
import unreal
lib=unreal.MaterialEditingLibrary
for name,color in [('Prop_Base',(.85,.78,.65)),('Prop_Ink',(.025,.032,.035)),('Prop_Paper',(.95,.92,.85)),('Prop_Orange',(1,.145,.025))]:
    m=unreal.load_asset('/Game/Art/Props/'+name)
    assert m,name
    lib.delete_all_material_expressions(m)
    c=lib.create_material_expression(m,unreal.MaterialExpressionConstant3Vector,0,0)
    c.set_editor_property('constant',unreal.LinearColor(*color,1))
    lib.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    r=lib.create_material_expression(m,unreal.MaterialExpressionConstant,0,150);r.set_editor_property('r',.75)
    lib.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
    lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
for cat in ['Ahuai','Jutuan','Wumei']:
    m=unreal.load_asset('/Game/Characters/'+cat+'/model')
    base=lib.get_material_property_input_node(m,unreal.MaterialProperty.MP_BASE_COLOR)
    output=lib.get_material_property_input_node_output_name(m,unreal.MaterialProperty.MP_BASE_COLOR)
    assert base,cat
    # A small painted-color contribution protects the black cat's eyes in shadow.
    mult=next((e for e in lib.get_material_expressions(m) if e.get_editor_property('desc')=='Cat painted fill'),None)
    if not mult:
        mult=lib.create_material_expression(m,unreal.MaterialExpressionMultiply,300,300)
        mult.set_editor_property('desc','Cat painted fill')
    mult.set_editor_property('const_b',.12)
    lib.connect_material_expressions(base,output,mult,'A')
    lib.connect_material_property(mult,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log('BAD_CAT_MATERIAL_POLISH_OK')
