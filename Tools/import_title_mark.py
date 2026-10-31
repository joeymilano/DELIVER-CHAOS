"""Import the approved brush title as an alpha-preserving, full-resolution UI asset."""
from pathlib import Path
import unreal

root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
source = root / 'ArtSource/UI/Title/TitleMark.png'
task = unreal.AssetImportTask()
task.filename = str(source)
task.destination_path = '/Game/UI/Title'
task.destination_name = 'T_TitleMark'
task.automated = True
task.replace_existing = True
task.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = unreal.load_asset('/Game/UI/Title/T_TitleMark')
if not isinstance(texture, unreal.Texture2D):
    raise RuntimeError('Title texture import failed')
texture.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property('never_stream', True)
texture.set_editor_property('srgb', True)
unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log('TITLE_MARK_IMPORTED')
unreal.SystemLibrary.quit_editor()
