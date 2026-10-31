"""Original low-poly party props. One meter normalized assets for runtime scaling."""
import bpy, math
from pathlib import Path
from mathutils import Vector
root=Path(__file__).resolve().parents[1];out=root/'ArtSource/Party';out.mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
def mat(n,c):
 m=bpy.data.materials.new(n);m.diffuse_color=(*c,1);m.use_nodes=True;m.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(*c,1);m.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value=.65;return m
orange=mat('Party_Orange',(1,.20,.025));purple=mat('Party_Plum',(.19,.035,.29));ink=mat('Party_Ink',(.018,.023,.025));stem=mat('Party_Stem',(.12,.21,.08));kraft=mat('Party_Kraft',(.49,.30,.13));cream=mat('Party_Cream',(.94,.86,.68))
def cube(pos,size,m):
 bpy.ops.mesh.primitive_cube_add(size=1,location=pos);o=bpy.context.object;o.dimensions=size;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(m);return o
def mesh(n,vs,fs,m):
 data=bpy.data.meshes.new(n);data.from_pydata(vs,[],fs);data.materials.append(m);o=bpy.data.objects.new(n,data);bpy.context.collection.objects.link(o);return o
def finish(n):
 objs=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.ops.object.select_all(action='DESELECT')
 for o in objs:o.select_set(True)
 bpy.context.view_layer.objects.active=objs[0];bpy.ops.object.join();o=bpy.context.object;bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
 vs=o.data.vertices;lo=Vector([min(v.co[i] for v in vs) for i in range(3)]);hi=Vector([max(v.co[i] for v in vs) for i in range(3)])
 for v in vs:
  for i in range(3):v.co[i]=(v.co[i]-(lo[i]+hi[i])/2)/(hi[i]-lo[i])
 o.name=n;bpy.ops.export_scene.fbx(filepath=str(out/(n+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',add_leaf_bones=False);bpy.ops.object.delete(use_global=False)
# Ribbed pumpkin, angular eyes and a mischievous mouth on its front (-Y).
bpy.ops.mesh.primitive_uv_sphere_add(segments=48,ring_count=18,radius=.5)
o=bpy.context.object;o.data.materials.append(orange)
for v in o.data.vertices:
 a=math.atan2(v.co.y,v.co.x);rib=1+.055*math.cos(a*10);v.co.x*=rib;v.co.y*=rib;v.co.z*=.82
bpy.ops.mesh.primitive_cone_add(vertices=7,radius1=.07,radius2=.04,depth=.18,location=(0,0,.45));bpy.context.object.rotation_euler.y=.25;bpy.context.object.data.materials.append(stem)
for x in [-.19,.19]:mesh('eye',[(x-.09,-.461,.11),(x+.09,-.461,.11),(x,-.468,.24)],[(0,1,2)],ink)
mesh('mouth',[(-.25,-.475,-.09),(-.13,-.49,-.14),(0,-.50,-.10),(.13,-.49,-.14),(.25,-.475,-.09),(.14,-.48,-.24),(0,-.49,-.28),(-.14,-.48,-.24)],[tuple(range(8))],ink)
finish('SM_Pumpkin')
# Thick open bowl; convex collision intentionally keeps candy simulation simple.
vs=[];fs=[];n=48
for r,z in [(.18,-.22),(.50,.18),(.47,.22),(.43,.18),(.14,-.16)]:
 for i in range(n):a=i*math.tau/n;vs.append((r*math.cos(a),r*math.sin(a),z))
for row in range(4):
 for i in range(n):j=(i+1)%n;fs.append((row*n+i,row*n+j,(row+1)*n+j,(row+1)*n+i))
fs.append(tuple(reversed(range(n))));fs.append(tuple(4*n+i for i in range(n)))
mesh('bowl',vs,fs,purple);finish('SM_CandyBowl')
# Wrapped candy, rather than unidentifiable spheres.
bpy.ops.mesh.primitive_uv_sphere_add(segments=12,ring_count=8,radius=.3);bpy.context.object.scale=(1,.65,.65);bpy.context.object.data.materials.append(orange)
for x in [-.4,.4]:
 bpy.ops.mesh.primitive_cone_add(vertices=6,radius1=.2,radius2=.055,depth=.25,location=(x,0,0),rotation=(0,math.pi/2 if x<0 else -math.pi/2,0));bpy.context.object.data.materials.append(cream)
finish('SM_Candy')
# Open parcel, taped side, splayed flaps.
cube((0,0,-.45),(1,1,.08),kraft)
for x in [-.46,.46]:cube((x,0,0),(.08,1,.9),kraft)
for y in [-.46,.46]:cube((0,y,0),(1,.08,.9),kraft)
for x in [-.58,.58]:
 o=cube((x,0,.48),(.42,1,.04),kraft);o.rotation_euler.y=(-1 if x>0 else 1)*.4
cube((0,-.505,-.1),(.38,.012,.3),cream);finish('SM_Parcel')
print('PARTY_PROPS_EXPORTED')
