"""Import the original transparent atlas and a matte masked sticker material."""
from pathlib import Path
import unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()));dest='/Game/Art/Halloween'
at=unreal.AssetToolsHelpers.get_asset_tools();lib=unreal.MaterialEditingLibrary
t=unreal.AssetImportTask();t.filename=str(root/'ArtSource/Decor/Halloween/StickerAtlas.png');t.destination_path=dest;t.destination_name='T_StickerAtlas';t.automated=True;t.replace_existing=True;t.save=True
at.import_asset_tasks([t]);tex=unreal.load_asset(dest+'/T_StickerAtlas')
if not isinstance(tex,unreal.Texture2D):raise RuntimeError('Atlas import failed')
tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_DEFAULT)
tex.set_editor_property('srgb',True);tex.set_editor_property('never_stream',True)
tex.set_editor_property('power_of_two_mode',unreal.TexturePowerOfTwoSetting.STRETCH_TO_POWER_OF_TWO)
tex.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
unreal.EditorAssetLibrary.save_loaded_asset(tex)
m=unreal.load_asset(dest+'/M_Sticker') or at.create_asset('M_Sticker',dest,unreal.Material,unreal.MaterialFactoryNew())
lib.delete_all_material_expressions(m);m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED);m.set_editor_property('two_sided',True);m.set_editor_property('opacity_mask_clip_value',.45)
def expr(cls):return lib.create_material_expression(m,cls)
def link(a,out,b,slot):assert lib.connect_material_expressions(a,out,b,slot)
def prop(a,out,p):assert lib.connect_material_property(a,out,p)
uv=expr(unreal.MaterialExpressionTextureCoordinate)
half=expr(unreal.MaterialExpressionConstant);half.r=.5
scale=expr(unreal.MaterialExpressionMultiply);link(uv,'',scale,'A');link(half,'',scale,'B')
x=expr(unreal.MaterialExpressionScalarParameter);x.set_editor_property('parameter_name','TileX');x.set_editor_property('default_value',0)
y=expr(unreal.MaterialExpressionScalarParameter);y.set_editor_property('parameter_name','TileY');y.set_editor_property('default_value',0)
offset=expr(unreal.MaterialExpressionAppendVector);link(x,'',offset,'A');link(y,'',offset,'B')
add=expr(unreal.MaterialExpressionAdd);link(scale,'',add,'A');link(offset,'',add,'B')
sample=expr(unreal.MaterialExpressionTextureSample);sample.texture=tex;link(add,'',sample,'UVs')
prop(sample,'RGB',unreal.MaterialProperty.MP_BASE_COLOR);prop(sample,'A',unreal.MaterialProperty.MP_OPACITY_MASK)
rough=expr(unreal.MaterialExpressionConstant);rough.r=.97;prop(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
lib.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log('HALLOWEEN_STICKERS_IMPORTED');unreal.SystemLibrary.quit_editor()
