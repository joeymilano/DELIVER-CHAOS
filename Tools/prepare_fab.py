"""Prepare locally acquired Fab assets; no third-party source redistribution."""
from pathlib import Path
import bpy, math,json
from mathutils import Vector,Quaternion
root=Path(__file__).resolve().parents[1];base=root/'ArtSource/FabLocal';report=[]
import os
for group in ([] if os.environ.get('FAB_CHARACTERS_ONLY') else ['Furniture','Decor']):
 out=base/group/'Prepared';out.mkdir(exist_ok=True)
 for file in (base/group).rglob('*.fbx'):
  if 'Prepared' in file.parts:continue
  bpy.ops.wm.read_factory_settings(use_empty=True);bpy.ops.import_scene.fbx(filepath=str(file))
  meshes=[o for o in bpy.context.scene.objects if o.type=='MESH' and not o.name.startswith(('UCX','UBX'))]
  if not meshes:continue
  bpy.ops.object.select_all(action='DESELECT')
  for o in meshes:o.select_set(True)
  bpy.context.view_layer.objects.active=meshes[0];bpy.ops.object.join();o=bpy.context.object
  bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
  lo=Vector([min(v.co[i] for v in o.data.vertices) for i in range(3)]);hi=Vector([max(v.co[i] for v in o.data.vertices) for i in range(3)])
  for v in o.data.vertices:v.co-=Vector(((lo.x+hi.x)/2,(lo.y+hi.y)/2,lo.z))
  o.name=file.stem
  bpy.ops.export_scene.fbx(filepath=str(out/file.name),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,bake_anim=False)
  report.append({'name':file.stem,'group':group,'meters':list(hi-lo)})
for i,name in [(1,'Owner'),(2,'Guest')]:
 out=base/'Characters'/'Prepared';out.mkdir(exist_ok=True)
 bpy.ops.wm.read_factory_settings(use_empty=True);bpy.ops.import_scene.fbx(filepath=str(base/'Characters/FBX'/f'Character_{i}.fbx'))
 arm=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE');arm.name=name
 def rot(n,axis,angle):
  b=arm.pose.bones.get(n)
  if b:
   q=b.bone.matrix_local.to_quaternion();b.rotation_mode='QUATERNION';b.rotation_quaternion=q.inverted()@Quaternion(Vector(axis),math.radians(angle))@q
 bpy.context.scene.render.fps=30
 for clip in ['Idle','Walk','Run','Confused']:
  arm.animation_data_clear();a=bpy.data.actions.new(clip);arm.animation_data_create();arm.animation_data.action=a
  for f in range(1,31):
   t=(f-1)/29*math.tau;bpy.context.scene.frame_set(f)
   for b in arm.pose.bones:b.rotation_mode='QUATERNION';b.rotation_quaternion=(1,0,0,0);b.location=(0,0,0)
   rot('upper_arm.L',(0,1,0),78);rot('upper_arm.R',(0,1,0),-78)
   amp=32 if clip=='Run' else 21 if clip=='Walk' else 0
   for side,phase in [('L',0),('R',math.pi)]:
    s=math.sin(t+phase);rot('thigh.'+side,(1,0,0),s*amp);rot('shin.'+side,(1,0,0),max(0,-s)*amp*1.25)
    if amp:
     # Counter-swing the whole arm, then bend the elbow; preserve the lowered rest pose.
     b=arm.pose.bones['upper_arm.'+side];q=b.bone.matrix_local.to_quaternion()
     b.rotation_quaternion=(q.inverted()@Quaternion(Vector((1,0,0)),-s*math.radians(amp*.7))@q) @ b.rotation_quaternion
     rot('forearm.'+side,(1,0,0),-35 if clip=='Run' else -14-max(0,s)*12)
     rot('foot.'+side,(1,0,0),-s*amp*.25)
   rot('spine.002',(1,0,0),-8 if clip=='Run' else math.sin(t)*1.2)
   rot('spine.003',(0,0,1),math.sin(t)* (4 if amp else 1.5))
   arm.pose.bones['Root'].location.z=(.025 if clip=='Run' else .012 if clip=='Walk' else .004)*(1-math.cos(t*2))
   arm.pose.bones['Root'].keyframe_insert('location',frame=f)
   rot('spine.004',(0,0,1),math.sin(t)* (18 if clip=='Confused' else 2))
   for b in arm.pose.bones:b.keyframe_insert('rotation_quaternion',frame=f)
  bpy.context.scene.frame_start=1;bpy.context.scene.frame_end=30
  bpy.context.scene.frame_set(1);bpy.context.view_layer.update()
  bpy.ops.object.select_all(action='SELECT')
  bpy.ops.export_scene.fbx(filepath=str(out/f'{name}_{clip}.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False)
if report:(base/'dimensions.json').write_text(json.dumps(report,indent=2))
