"""Import offline sound cues and the pet camera's translucent cone material."""
from pathlib import Path
import unreal
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
tools=unreal.AssetToolsHelpers.get_asset_tools()
for source in sorted((root/'ArtSource/Audio').glob('*.wav')):
    task=unreal.AssetImportTask()
    task.filename=str(source);task.destination_path='/Game/Audio';task.destination_name=source.stem
    task.automated=True;task.replace_existing=True;task.save=True
    tools.import_asset_tasks([task])
    sound=unreal.load_asset('/Game/Audio/'+source.stem)
    if not isinstance(sound,unreal.SoundWave):raise RuntimeError(str(source))
path='/Game/Art/M_CameraVision'
m=unreal.load_asset(path)
if not m:m=tools.create_asset('M_CameraVision','/Game/Art',unreal.Material,unreal.MaterialFactoryNew())
unreal.MaterialEditingLibrary.delete_all_material_expressions(m)
m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
m.set_editor_property('two_sided',True)
m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
color=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant3Vector)
color.set_editor_property('constant',unreal.LinearColor(1,.025,.012,1))
a=unreal.MaterialEditingLibrary.create_material_expression(m,unreal.MaterialExpressionConstant)
a.set_editor_property('r',.15)
unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
unreal.MaterialEditingLibrary.connect_material_property(a,'',unreal.MaterialProperty.MP_OPACITY)
unreal.MaterialEditingLibrary.recompile_material(m)
unreal.EditorAssetLibrary.save_loaded_asset(m)
unreal.log('PARTY_AUDIO_AND_MATERIALS_OK')
