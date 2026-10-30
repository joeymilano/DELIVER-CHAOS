"""Bind the original Dafu mesh to the existing quadruped rig; retain shared animation compatibility."""
import bpy,json
from pathlib import Path
from mathutils import Vector
root=Path(__file__).resolve().parents[1];out=root/'ArtSource/V2Fun/Dafu'
bpy.ops.wm.open_mainfile(filepath=str(root/'ArtSource/Models/Ahuai/Rig/ahuai-rig.blend'))
arm=next(o for o in bpy.data.objects if o.type=='ARMATURE');arm.data.pose_position='REST'
source=max((o for o in bpy.data.objects if o.type=='MESH' and len(o.vertex_groups)>0),key=lambda o:len(o.vertex_groups))
old=set(bpy.data.objects);bpy.ops.import_scene.gltf(filepath=str(out/'Dafu-game.glb'))
target=next(o for o in bpy.data.objects if o not in old and o.type=='MESH')
# Both providers' verified forward direction is -Y. Fit the new mesh to shared rest bounds.
def bounds(o):
 v=[o.matrix_world@Vector(c) for c in o.bound_box];return Vector([min(x[i] for x in v) for i in range(3)]),Vector([max(x[i] for x in v) for i in range(3)])
bpy.context.view_layer.update();sl,sh=bounds(source);tl,th=bounds(target)
world=target.matrix_world.copy()
for v in target.data.vertices:
 p=world@v.co
 for i in range(3):p[i]=sl[i]+(p[i]-tl[i])/(th[i]-tl[i])*(sh[i]-sl[i])
 v.co=p
# Bring the lifted rear paw in the generated reference toward the shared standing stance.
for v in target.data.vertices:
 if v.co.y>-.25 and v.co.z<.65:
  w=max(0.,min(1.,(.65-v.co.z)/.5))*max(0.,min(1.,(v.co.y+.25)/.18))
  v.co.y+=.20*w;v.co.z-=.08*w
target.matrix_world.identity();target.name='SK_Dafu'
bpy.ops.object.select_all(action='DESELECT');target.select_set(True);bpy.context.view_layer.objects.active=target
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.remove_doubles(threshold=0.0001);bpy.ops.object.mode_set(mode='OBJECT')
arm.select_set(True);bpy.context.view_layer.objects.active=arm
bpy.ops.object.parent_set(type='ARMATURE_AUTO')
bpy.context.view_layer.objects.active=target;arm.select_set(False)
# Cap influences, normalize, and fail rather than silently ship unbound vertices.
bpy.ops.object.select_all(action='DESELECT');target.select_set(True);bpy.ops.object.vertex_group_limit_total(limit=4);bpy.ops.object.vertex_group_normalize_all(lock_active=False)
unweighted=sum(not any(g.weight>0.0001 for g in v.groups) for v in target.data.vertices)
assert unweighted==0,unweighted
target.parent=arm
for o in list(old):
 if o.type=='MESH':bpy.data.objects.remove(o,do_unlink=True)
arm.animation_data.action=None
for p in arm.pose.bones:p.matrix_basis.identity()
arm.data.pose_position='POSE';bpy.context.view_layer.update()
bpy.ops.object.select_all(action='DESELECT');arm.select_set(True);target.select_set(True);bpy.context.view_layer.objects.active=arm
bpy.ops.export_scene.fbx(filepath=str(out/'SK_Dafu.fbx'),use_selection=True,object_types={'ARMATURE','MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,path_mode='COPY',embed_textures=True,bake_anim=False)
scene=bpy.context.scene;scene.render.resolution_x=800;scene.render.resolution_y=800;scene.cycles.samples=16
scene.camera.location=(3,-4,2.2);scene.camera.rotation_euler=(Vector((0,0,.8))-scene.camera.location).to_track_quat('-Z','Y').to_euler();scene.camera.data.ortho_scale=2.7
scene.render.filepath=str(out/'rig-rest.png');bpy.ops.render.render(write_still=True)
for action in [a for a in bpy.data.actions if 'Run' in a.name or 'Walk' in a.name]:
 arm.animation_data.action=action
 if action.slots:arm.animation_data.action_slot=action.slots[0]
 scene.frame_set(int(sum(action.frame_range)/2));scene.render.filepath=str(out/('rig-'+action.name+'.png'));bpy.ops.render.render(write_still=True)
(out/'rig-report.json').write_text(json.dumps({'source_rig':'Ahuai quadruped','method':'Blender bone heat after welding; four influences normalized','unweighted_vertices':unweighted,'vertices':len(target.data.vertices),'bones':[b.name for b in arm.data.bones],'shared_animation_skeleton':'/Game/Characters/Ahuai/SK_Ahuai_Skeleton'},indent=2)+'\n')
bpy.ops.wm.save_as_mainfile(filepath=str(out/'Dafu-rig.blend'))
print('DAFU_RIG_EXPORTED')
