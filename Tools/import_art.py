"""Import approved character portraits; rerunnable without touching unrelated assets."""
import os
import unreal

root = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
for cat, name in [('ahuai', 'Ahuai'), ('jutuan', 'Jutuan'), ('wumei', 'Wumei')]:
    source = os.path.join(root, 'ArtSource', 'References', 'Approved', cat + '-lowpoly-v2.png')
    if not os.path.isfile(source):
        raise RuntimeError('Missing approved reference: ' + source)
    task = unreal.AssetImportTask()
    task.filename = source
    task.destination_path = '/Game/UI/Cats'
    task.destination_name = 'T_' + name
    task.automated = True
    task.replace_existing = True
    task.save = True
    asset_tools.import_asset_tasks([task])
    texture = unreal.load_asset('/Game/UI/Cats/T_' + name)
    if not isinstance(texture, unreal.Texture2D):
        raise RuntimeError('Portrait import failed: ' + name)
    texture.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log('BAD_CAT_APPROVED_PORTRAITS_OK')
