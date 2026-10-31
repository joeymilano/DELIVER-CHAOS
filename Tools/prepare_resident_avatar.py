"""Prepare the supplied rigged avatar locally. Chloe is static and not used as a moving actor."""
from pathlib import Path
import bpy, math, json
from mathutils import Vector, Quaternion

root=Path(__file__).resolve().parents[1]
base=root/'ArtSource/ResidentsLocal';out=base/'Prepared';out.mkdir(exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(base/'Owner.glb'))
arm=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' for m in o.modifiers)]
for o in list(bpy.context.scene.objects):
    if o.type=='MESH' and o not in meshes:bpy.data.objects.remove(o,do_unlink=True)
mapping={'Hips_66':'Root','Spine_55':'spine.001','Spine1_54':'spine.002','Spine2_53':'spine.003','Head_3':'head'}
for side,prefix in [('L','Left'),('R','Right')]:
    for start,new in [('UpLeg','thigh'),('Leg','shin'),('Foot','foot'),('Arm','upper_arm'),('ForeArm','forearm')]:
        bone=next(b for b in arm.data.bones if b.name.startswith(prefix+start+'_'))
        mapping[bone.name]=new+'.'+side
for old,new in mapping.items():arm.data.bones[old].name=new
arm.name='OwnerAvatar'
# Retain the supplied UV textures and material slots. No colour-atlas replacement.
materials=[]
for mat in bpy.data.materials:
    if not mat.use_nodes:continue
    bsdf=next((n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED'),None)
    if not bsdf:continue
    tex=None
    if bsdf.inputs['Base Color'].is_linked:
        node=bsdf.inputs['Base Color'].links[0].from_node
        if node.type=='TEX_IMAGE':tex=node.image
    name=mat.name.replace('.','_')
    data={'name':name,'color':list(bsdf.inputs['Base Color'].default_value),'roughness':max(.45,bsdf.inputs['Roughness'].default_value)}
    if tex:
        filename='T_'+name+'.png';tex.filepath_raw=str(out/filename);tex.file_format='PNG';tex.save();data['texture']=filename
    mat.name=name;materials.append(data)
(out/'materials.json').write_text(json.dumps(materials,indent=2))

def rotate(name,axis,angle):
    b=arm.pose.bones[name];q=b.bone.matrix_local.to_quaternion()
    b.rotation_quaternion=(q.inverted()@Quaternion(Vector(axis),math.radians(angle))@q)@b.rotation_quaternion
bpy.context.scene.render.fps=30
for clip,count in [('Idle',72),('Walk',36),('Run',24),('Confused',60)]:
    arm.animation_data_clear();arm.animation_data_create();arm.animation_data.action=bpy.data.actions.new(clip)
    for f in range(1,count+1):
        bpy.context.scene.frame_set(f);t=(f-1)/(count-1)*math.tau
        for b in arm.pose.bones:b.rotation_mode='QUATERNION';b.rotation_quaternion=(1,0,0,0);b.location=(0,0,0)
        for side,phase,sign in [('L',0,1),('R',math.pi,-1)]:
            rotate('upper_arm.'+side,(0,1,0),sign*28)
            s=math.sin(t+phase);amp=29 if clip=='Run' else 19 if clip=='Walk' else 0
            if amp:
                rotate('thigh.'+side,(1,0,0),s*amp)
                rotate('shin.'+side,(1,0,0),max(0,s)*amp*1.4)
                rotate('foot.'+side,(1,0,0),-s*amp*.28)
                rotate('upper_arm.'+side,(1,0,0),-s*amp*.7)
            rotate('forearm.'+side,(1,0,0),-45 if clip=='Run' else -12)
        rotate('spine.002',(1,0,0),9 if clip=='Run' else math.sin(t)*.8)
        rotate('spine.003',(0,0,1),math.sin(t)*(3 if clip in ('Walk','Run') else 1))
        rotate('head',(0,0,1),math.sin(t)*(16 if clip=='Confused' else 1.3))
        arm.pose.bones['Root'].location.z=(.018 if clip=='Run' else .009 if clip=='Walk' else .002)*(1-math.cos(2*t))
        arm.pose.bones['Root'].keyframe_insert('location',frame=f)
        for b in arm.pose.bones:b.keyframe_insert('rotation_quaternion',frame=f)
    bpy.context.scene.frame_start=1;bpy.context.scene.frame_end=count;bpy.context.scene.frame_set(1)
    bpy.ops.object.select_all(action='DESELECT');arm.select_set(True)
    for mesh in meshes:mesh.select_set(True)
    bpy.context.view_layer.objects.active=arm
    bpy.ops.export_scene.fbx(filepath=str(out/f'Owner_{clip}.fbx'),use_selection=True,object_types={'ARMATURE','MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False,bake_anim=True,bake_anim_use_all_actions=False,bake_anim_use_nla_strips=False)
print('RESIDENT_AVATAR_PREPARED')
