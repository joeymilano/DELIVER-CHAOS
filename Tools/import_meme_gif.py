from pathlib import Path
import unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
source=root/'ArtSource/MemesLocal/OiiaAtlas.png'
if source.exists():
 t=unreal.AssetImportTask();t.filename=str(source);t.destination_path='/Game/UI/MemesLocal';t.destination_name='OiiaAtlas';t.automated=True;t.replace_existing=True;t.save=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
 tex=unreal.load_asset('/Game/UI/MemesLocal/OiiaAtlas');tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON);tex.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS);tex.set_editor_property('srgb',True)
 unreal.EditorAssetLibrary.save_loaded_asset(tex)
 unreal.log('MEME_GIF_IMPORTED')
unreal.SystemLibrary.quit_editor()
