"""Export selected licensed Nasha models locally, preserving solid colour materials."""
from pathlib import Path
import bpy,json,os
root=Path(__file__).resolve().parents[1];out=root/'ArtSource/CozyLocal';out.mkdir(exist_ok=True)
choices={'Sweet_Halloween':['Pumpkin_Fireplace','Candy_Hutch','Candy_Buffet','Ghost_Window_Garland','Pumpkin_Fairy_Lights','Broom_and_Hat_Stand','Little_Happy_Ghost','Ghost_Cookie_Tray','Candy_Jar_Set','Pumpkin_Tea_Cake','Friendly_Cat_Plush'], 'Fortune_Teller':['Crystal_Ball','Plum_Teapot','Potion_Trio','Moon_Cookies','Moon_Phase_Board']}
report=[]
for pack,names in choices.items():
 base=Path(os.environ.get('COZY_DOWNLOADS',str(Path.home()/'Downloads')))/('Nashas_Cozy_3D_'+pack)
 for name in names:
  bpy.ops.wm.read_factory_settings(use_empty=True);bpy.ops.import_scene.gltf(filepath=str(base/'Models/GLB'/(name+'.glb')))
  meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.ops.object.select_all(action='DESELECT')
  for o in meshes:o.select_set(True)
  bpy.context.view_layer.objects.active=meshes[0];bpy.ops.object.join();o=bpy.context.object
  bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
  mats=[]
  for m in o.data.materials:
   bs=next((n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED'),None) if m.use_nodes else None
   mats.append({'name':m.name,'color':list(bs.inputs['Base Color'].default_value if bs else m.diffuse_color),'roughness':bs.inputs['Roughness'].default_value if bs else .7,'metallic':bs.inputs['Metallic'].default_value if bs else 0})
  bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,bake_anim=False)
  report.append({'name':name,'pack':pack,'materials':mats})
(out/'manifest.json').write_text(json.dumps(report,indent=2))
