"""Refine existing paw clips locally; no model, rig, or other gesture is replaced."""
from pathlib import Path
import bpy
import math
from mathutils import Quaternion, Vector

root = Path(__file__).resolve().parents[1]
for cat in ("Ahuai", "Jutuan", "Wumei"):
    folder = root / "ArtSource/Models" / cat / "Rig"
    bpy.ops.wm.open_mainfile(filepath=str(folder / (cat.lower() + "-rig.blend")))
    arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
    arm.animation_data_clear()
    scene = bpy.context.scene
    scene.render.fps = 30

    def smooth(a, b, t):
        x = max(0, min(1, (t - a) / (b - a)))
        return x*x*(3-2*x)

    def rotate(name, axis, degrees):
        bone = arm.pose.bones.get(name)
        if bone:
            rest = bone.bone.matrix_local.to_quaternion()
            bone.rotation_quaternion = rest.inverted() @ Quaternion(Vector(axis), math.radians(degrees)) @ rest

    for name, frames in (("Push", 28), ("Swipe", 13)):
        action = bpy.data.actions.new("Contact_" + name)
        arm.animation_data_create()
        arm.animation_data.action = action
        for frame in range(1, frames+1):
            scene.frame_set(frame)
            t = (frame-1)/30
            for bone in arm.pose.bones:
                bone.rotation_mode = "QUATERNION"
                bone.rotation_quaternion = (1, 0, 0, 0)
                bone.location = (0, 0, 0)
                bone.scale = (1, 1, 1)
            if name == "Push":
                # Full extension at .30 s; a short physical push, then a deliberate withdrawal.
                reach = smooth(.03, .30, t) * (1-smooth(.50, .78, t))
                rotate("RightArm", (1, 0, 0), -58*reach)
                rotate("RightForeArm", (1, 0, 0), 20*reach)
                rotate("RightShoulder", (0, 0, 1), -12*reach)
                rotate("Spine2", (0, 0, 1), -3*reach)
                rotate("Head", (0, 0, 1), -15*reach)
                rotate("Neck", (1, 0, 0), -5*reach)
            else:
                reach = smooth(0, .18, t) * (1-smooth(.21, .40, t))
                rotate("RightArm", (1, 0, 0), -62*reach)
                rotate("RightForeArm", (1, 0, 0), 24*reach)
                rotate("RightShoulder", (0, 0, 1), -18*reach)
                rotate("Head", (0, 0, 1), -10*reach)
            for bone in arm.pose.bones:
                bone.keyframe_insert("rotation_quaternion", frame=frame)
                bone.keyframe_insert("location", frame=frame)
        scene.frame_start, scene.frame_end = 1, frames
        bpy.ops.object.select_all(action="DESELECT")
        arm.select_set(True)
        bpy.context.view_layer.objects.active = arm
        bpy.ops.export_scene.fbx(filepath=str(folder / ("Gesture_" + name + ".fbx")),
            use_selection=True, object_types={"ARMATURE"}, axis_forward="-Y", axis_up="Z",
            add_leaf_bones=False, bake_anim=True, bake_anim_use_all_actions=False,
            bake_anim_use_nla_strips=False, bake_anim_simplify_factor=0)
        arm.animation_data_clear()
        bpy.data.actions.remove(action)
print("CAT_CONTACT_CLIPS_EXPORTED")
