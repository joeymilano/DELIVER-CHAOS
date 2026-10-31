"""Render existing gesture definitions on the candidate Dafu rig for review.
No generated model or existing locomotion animation is overwritten.
"""
from pathlib import Path
import bpy, math
from mathutils import Quaternion, Vector
root=Path(__file__).resolve().parents[1]
for cat in ['Dafu']:
    folder=root/'ArtSource/V2Fun/Dafu'
    bpy.ops.wm.open_mainfile(filepath=str(folder/('Dafu-rig.blend')))
    arm=next(o for o in bpy.data.objects if o.type=='ARMATURE')
    arm.animation_data_clear()
    for action in list(bpy.data.actions): bpy.data.actions.remove(action)
    scene=bpy.context.scene;scene.render.fps=30
    def rotate(name,axis,degrees):
        b=arm.pose.bones.get(name)
        if not b:return
        rest=b.bone.matrix_local.to_quaternion()
        b.rotation_quaternion=rest.inverted() @ Quaternion(Vector(axis),math.radians(degrees)) @ rest
    for name,frames in [('Sit',45),('Charge',20),('Swipe',12),('Push',30),('Pounce',14)]:
        action=bpy.data.actions.new('Gesture_'+name)
        arm.animation_data_create();arm.animation_data.action=action
        for frame in range(1,frames+1):
            scene.frame_set(frame);t=(frame-1)/(frames-1)
            for b in arm.pose.bones:
                b.rotation_mode='QUATERNION';b.rotation_quaternion=(1,0,0,0);b.location=(0,0,0);b.scale=(1,1,1)
            if name=='Sit':
                arm.pose.bones['Hips'].location=arm.data.bones['Hips'].matrix_local.to_quaternion().inverted() @ Vector((0,0,-.42))
                rotate('Hips',(1,0,0),-45)
                for side in ['Left','Right']:
                    rotate(side+'UpLeg',(1,0,0),-50)
                    rotate(side+'Leg',(1,0,0),125)
                    rotate(side+'HindFoot',(1,0,0),-30)
                    rotate(side+'Arm',(1,0,0),45)
                rotate('Neck',(1,0,0),38)
                rotate('Head',(0,0,1),-17+math.sin(t*math.tau)*2)
            elif name=='Charge':
                arm.pose.bones['Hips'].location=arm.data.bones['Hips'].matrix_local.to_quaternion().inverted() @ Vector((0,0,-.07))
                rotate('Hips',(0,0,1),math.sin(t*math.tau)*8)
                rotate('Spine',(0,0,1),-math.sin(t*math.tau)*7)
                for side in ['Left','Right']:
                    rotate(side+'Arm',(1,0,0),25);rotate(side+'ForeArm',(1,0,0),-35)
            elif name in ['Swipe','Push']:
                v=math.sin(t*math.pi) if name=='Swipe' else .5+.5*math.sin(t*math.tau)
                rotate('RightArm',(1,0,0),-58*v)
                rotate('RightForeArm',(1,0,0),25*v)
                rotate('RightShoulder',(0,0,1),-18*v)
                rotate('Head',(0,0,1),-38 if name=='Push' else -10*v)
            else:
                v=math.sin(t*math.pi)
                for side in ['Left','Right']:
                    rotate(side+'Arm',(1,0,0),-55*v);rotate(side+'UpLeg',(1,0,0),45*v)
            for b in arm.pose.bones:
                b.keyframe_insert('rotation_quaternion',frame=frame)
                b.keyframe_insert('location',frame=frame)
        scene.frame_start=1;scene.frame_end=frames
        bpy.ops.object.select_all(action='DESELECT');arm.select_set(True);bpy.context.view_layer.objects.active=arm
        if True:
            scene.frame_set(frames//2);scene.cycles.samples=8;scene.render.resolution_x=640;scene.render.resolution_y=640
            scene.render.filepath=str(folder/('gesture-'+name.lower()+'.png'))
            bpy.ops.render.render(write_still=True)
        arm.animation_data_clear();bpy.data.actions.remove(action)
print('DAFU_GESTURES_REVIEWED')
