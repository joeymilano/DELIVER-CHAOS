"""Blender: normalize, reduce, export PBR sources and render four views of a V2Fun GLB.
Usage: Blender --background --python Tools/prepare_v2fun_assets.py -- AssetName input.glb
"""
import bpy,json,math,sys
from pathlib import Path
from mathutils import Vector
name,source=sys.argv[sys.argv.index('--')+1:];root=Path(__file__).resolve().parents[1]
out=root/'ArtSource/V2Fun'/name;out.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=source)
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
assert meshes
for o in meshes:
 bpy.context.view_layer.objects.active=o;o.select_set(True)
 bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
verts=[o.matrix_world@Vector(c) for o in meshes for c in o.bound_box]
lo=Vector([min(v[i] for v in verts) for i in range(3)]);hi=Vector([max(v[i] for v in verts) for i in range(3)])
center=(hi+lo)/2;factor=1/(hi-lo).z
source_tri=sum(sum(len(p.vertices)-2 for p in o.data.polygons) for o in meshes)
budget=32000 if name=='Dafu' else 7000
for o in meshes:
 # Bake standardized world coordinates in meters. Feet/base rest on Z=0.
 matrix=o.matrix_world.copy()
 for v in o.data.vertices:
  p=matrix@v.co;p-=Vector((center.x,center.y,lo.z));v.co=p*factor
 o.matrix_world.identity()
 bpy.context.view_layer.objects.active=o
 # V2Fun GLBs contain split triangle vertices. Weld coincident positions BEFORE
 # decimation; otherwise isolated triangles disappear and leave visible cracks.
 bpy.ops.object.select_all(action='DESELECT');o.select_set(True)
 bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT')
 bpy.ops.mesh.remove_doubles(threshold=0.00001)
 bpy.ops.mesh.normals_make_consistent(inside=False);bpy.ops.object.mode_set(mode='OBJECT')
 tri=sum(len(p.vertices)-2 for p in o.data.polygons)
 if source_tri>budget:
  mod=o.modifiers.new('RuntimeBudget','DECIMATE');mod.ratio=budget/source_tri;bpy.ops.object.modifier_apply(modifier=mod.name)
 o.name='SM_'+name
 if name!='Dafu':
  for poly in o.data.polygons:poly.use_smooth=True
 for mat in o.data.materials:
  if mat:mat.name='M_'+name+'_'+str(list(o.data.materials).index(mat))
images=[]
for i,img in enumerate(list(bpy.data.images)):
 if img.type!='IMAGE' or img.size[0]==0:continue
 if not img.packed_file and not img.filepath:continue
 original=list(img.size)
 if max(img.size)>2048:
  ratio=2048/max(img.size);img.scale(round(img.size[0]*ratio),round(img.size[1]*ratio))
 img.name='T_'+name+'_'+str(i);img.filepath_raw=str(out/(img.name+'.png'));img.file_format='PNG';img.save()
 images.append({'file':img.name+'.png','original_size':original,'game_size':list(img.size)})
bpy.ops.object.select_all(action='DESELECT')
for o in meshes:o.select_set(True)
bpy.context.view_layer.objects.active=meshes[0]
bpy.ops.export_scene.fbx(filepath=str(out/('SM_'+name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',path_mode='COPY',embed_textures=True,bake_anim=False)
bpy.ops.export_scene.gltf(filepath=str(out/(name+'-game.glb')),use_selection=True,export_format='GLB')
materials=[]
for mat in {m for o in meshes for m in o.data.materials if m}:
 nodes=mat.node_tree.nodes if mat.use_nodes else []
 nodes_info=[{'name':n.name,'type':n.type,'image':n.image.name if n.type=='TEX_IMAGE' and n.image else None} for n in nodes]
 links_info=[{'from':l.from_node.name,'output':l.from_socket.name,'to':l.to_node.name,'input':l.to_socket.name} for l in mat.node_tree.links] if mat.use_nodes else []
 materials.append({'name':mat.name,'nodes':nodes_info,'links':links_info})
(out/'material-report.json').write_text(json.dumps(materials,indent=2)+'\n')
# Neutral contact-sheet views are rendered from actual reduced game mesh.
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=16
scene.render.resolution_x=640;scene.render.resolution_y=640;scene.render.resolution_percentage=100
scene.world.use_nodes=True;scene.world.node_tree.nodes['Background'].inputs[0].default_value=(.32,.32,.32,1)
scene.world.node_tree.nodes['Background'].inputs[1].default_value=.7
scene.view_settings.view_transform='AgX'
for loc,power in [((2,-3,4),350),((-3,-1,2),150),((0,3,3),250)]:
 bpy.ops.object.light_add(type='AREA',location=loc);l=bpy.context.object;l.data.energy=power;l.data.size=3;l.rotation_euler=(Vector((0,0,.5))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add();cam=bpy.context.object;cam.data.type='ORTHO';cam.data.ortho_scale=1.65;scene.camera=cam
report={'asset':name,'source_triangles':source_tri,'game_triangles':sum(sum(len(p.vertices)-2 for p in o.data.polygons) for o in meshes),'materials':list({m.name for o in meshes for m in o.data.materials if m}),'textures':images,'source_bounds':[list(lo),list(hi)],'armatures':sum(o.type=='ARMATURE' for o in scene.objects)}
(out/'geometry-report.json').write_text(json.dumps(report,indent=2)+'\n')
for label,loc in [('front',(2,-3,1.5)),('back',(-2,3,1.5)),('left',(-3,-2,1.4)),('right',(3,2,1.4))]:
 cam.location=loc;cam.rotation_euler=(Vector((0,0,.5))-cam.location).to_track_quat('-Z','Y').to_euler();scene.render.filepath=str(out/('view-'+label+'.png'));bpy.ops.render.render(write_still=True)
bpy.ops.wm.save_as_mainfile(filepath=str(out/(name+'-inspection.blend')))
print('V2FUN_GAME_MESH_OK',json.dumps(report))
