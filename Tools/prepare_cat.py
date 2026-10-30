"""Blender: inspect and make an efficient static import from the source GLB."""
import bpy, os, json
from mathutils import Vector
cat=os.environ.get('BAD_CAT_NAME','Ahuai')
assert cat in ['Ahuai','Jutuan','Wumei']
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
folder=os.path.join(root,'ArtSource','Models',cat)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=os.path.join(folder,'base_basic_pbr.glb'))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
stats=[]
for obj in meshes:
    bpy.context.view_layer.objects.active=obj
    tri=sum(len(p.vertices)-2 for p in obj.data.polygons)
    mod=obj.modifiers.new('GameBudget','DECIMATE');mod.ratio=min(1,30000/tri)
    bpy.ops.object.modifier_apply(modifier=mod.name)
    stats.append({'name':obj.name,'source_triangles':tri,'game_triangles':sum(len(p.vertices)-2 for p in obj.data.polygons),'dimensions':list(obj.dimensions)})
    obj.name='SM_'+cat
for image in bpy.data.images:
    if image.packed_file:
        image.filepath_raw=os.path.join(folder,image.name+'.png')
        image.file_format='PNG';image.save()
bpy.ops.object.select_all(action='DESELECT')
for obj in meshes:obj.select_set(True)
bpy.ops.export_scene.fbx(filepath=os.path.join(folder,'SM_'+cat+'.fbx'),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,path_mode='COPY',embed_textures=True)
with open(os.path.join(folder,'geometry-report.json'),'w') as f:json.dump(stats,f,indent=2)
scene=bpy.context.scene
scene.render.engine='CYCLES';scene.cycles.samples=24
scene.render.resolution_x=1100;scene.render.resolution_y=1100;scene.render.resolution_percentage=100
scene.world.color=(.32,.32,.32)
bpy.ops.object.camera_add(location=(3,-4,2.3))
cam=bpy.context.object;cam.rotation_euler=(Vector((0,0,.8))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=2.6;scene.camera=cam
for loc,power,size in [((2,-3,4),450,4),((-3,-1,2),250,3),((0,3,3),400,3)]:
    bpy.ops.object.light_add(type='AREA',location=loc);light=bpy.context.object;light.data.energy=power;light.data.shape='DISK';light.data.size=size;light.rotation_euler=(Vector((0,0,.8))-light.location).to_track_quat('-Z','Y').to_euler()
scene.render.filepath=os.path.join(folder,'inspection.png')
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(folder,cat.lower()+'-inspection.blend'))
bpy.ops.render.render(write_still=True)
