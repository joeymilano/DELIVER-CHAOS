"""Blender export of the downloaded quadruped skeleton and animation clips."""
import bpy, os, json
from mathutils import Vector
cat=os.environ.get('BAD_CAT_NAME','Ahuai')
assert cat in ['Ahuai','Jutuan','Wumei']
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
folder=os.path.join(root,'ArtSource','Models',cat,'Rig')
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=os.path.join(folder,'base_basic_pbr.glb'))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
arm=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
# Avoid Unreal's special Armature node removal changing animation root scale.
arm.name='CatRig'
for obj in meshes:
    bpy.context.view_layer.objects.active=obj
    tri=sum(len(p.vertices)-2 for p in obj.data.polygons)
    mod=obj.modifiers.new('GameBudget','DECIMATE');mod.ratio=min(1,30000/tri)
    bpy.ops.object.modifier_apply(modifier=mod.name)
    obj.name='SK_'+cat
for image in bpy.data.images:
    if image.packed_file:
        image.filepath_raw=os.path.join(folder,image.name+'.png');image.file_format='PNG';image.save()
actions=list(bpy.data.actions)
# Canine clips fold this cat's long upright tail. Keep its authored rest shape.
for action in actions:
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for curve in list(bag.fcurves):
                    if 'pose.bones["Tail' in curve.data_path:
                        bag.fcurves.remove(curve)
for bone in arm.pose.bones:
    if bone.name.startswith('Tail'):
        bone.rotation_quaternion=(1,0,0,0)
        bone.location=(0,0,0)
        bone.scale=(1,1,1)
with open(os.path.join(folder,'rig-report.json'),'w') as f:json.dump({'bones':[b.name for b in arm.data.bones],'actions':[(a.name,list(a.frame_range)) for a in actions],'vertices':sum(len(o.data.vertices) for o in meshes)},f,indent=2)
bpy.ops.object.select_all(action='DESELECT')
arm.select_set(True)
for o in meshes:o.select_set(True)
bpy.context.view_layer.objects.active=arm
bpy.ops.export_scene.fbx(filepath=os.path.join(folder,'SK_'+cat+'.fbx'),use_selection=True,object_types={'ARMATURE','MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,path_mode='COPY',embed_textures=True,bake_anim=True,bake_anim_use_all_actions=True,bake_anim_use_nla_strips=False)
scene=bpy.context.scene
scene.render.engine='CYCLES';scene.cycles.samples=16
scene.render.resolution_x=1000;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
scene.world.color=(.3,.3,.3)
bpy.ops.object.camera_add(location=(3,-4,2.3));cam=bpy.context.object;cam.rotation_euler=(Vector((0,0,.8))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=2.7;scene.camera=cam
for loc,power in [((2,-3,4),450),((-3,-1,2),250),((0,3,3),400)]:
    bpy.ops.object.light_add(type='AREA',location=loc);l=bpy.context.object;l.data.energy=power;l.data.size=4;l.rotation_euler=(Vector((0,0,.8))-l.location).to_track_quat('-Z','Y').to_euler()
for a in actions:
    arm.animation_data.action=a
    if hasattr(a,'slots') and a.slots:arm.animation_data.action_slot=a.slots[0]
    scene.frame_set(int((a.frame_range[0]+a.frame_range[1])*.5))
    scene.render.filepath=os.path.join(folder,a.name+'-check.png')
    bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(folder,cat.lower()+'-rig.blend'))
