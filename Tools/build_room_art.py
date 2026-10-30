import bpy,os,math,json
from mathutils import Vector
root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
out=os.path.join(root,'ArtSource','Environment');os.makedirs(out,exist_ok=True)
def mat(name,c):
 m=bpy.data.materials.new(name);m.diffuse_color=(*c,1);m.use_nodes=True;m.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(*c,1);m.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value=.7;return m
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
base=mat('Prop_Base',(.85,.78,.65));ink=mat('Prop_Ink',(.025,.032,.035));paper=mat('Prop_Paper',(.95,.92,.85));orange=mat('Prop_Orange',(1,.24,.04))
def box(loc,dim,m=base,bevel=.04):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.dimensions=dim;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(m)
 if bevel:
  mod=o.modifiers.new('Round','BEVEL');mod.width=bevel;mod.segments=3;bpy.ops.object.modifier_apply(modifier=mod.name)
 return o
def cyl(loc,r,depth,m=base,vertices=32):
 bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=depth,location=loc);o=bpy.context.object;o.data.materials.append(m);return o
def tor(loc,major,minor,m=base,rot=(0,0,0)):
 bpy.ops.mesh.primitive_torus_add(major_radius=major,minor_radius=minor,major_segments=36,minor_segments=8,location=loc,rotation=rot);bpy.context.object.data.materials.append(m)
def finish(name,normalize=True):
 objs=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.ops.object.select_all(action='DESELECT')
 for o in objs:o.select_set(True)
 bpy.context.view_layer.objects.active=objs[0];bpy.ops.object.join();o=bpy.context.object;bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
 vs=[v.co for v in o.data.vertices];lo=Vector([min(v[i] for v in vs) for i in range(3)]);hi=Vector([max(v[i] for v in vs) for i in range(3)])
 if normalize:
  for v in vs:
   for i in range(3):v[i]=(v[i]-(lo[i]+hi[i])*.5)/max(hi[i]-lo[i],.00001)
 o.name=name
 bpy.ops.export_scene.fbx(filepath=os.path.join(out,name+'.fbx'),use_selection=True,object_types={'MESH'},axis_forward='-Y',axis_up='Z',mesh_smooth_type='FACE',add_leaf_bones=False)
 bpy.ops.object.delete(use_global=False)
box((0,0,0),(1,1,1),bevel=.12);finish('SM_RoundedBlock')
# Cup with open mouth, a thick rim and a handle.
cyl((0,0,-.43),.37,.10)
verts=[];faces=[];n=36
for z,r in [(-.45,.37),(.45,.40),(.45,.34),(-.37,.31)]:
 for i in range(n):a=i*math.tau/n;verts.append((r*math.cos(a),r*math.sin(a),z))
for row in range(3):
 for i in range(n):j=(i+1)%n;faces.append((row*n+i,row*n+j,(row+1)*n+j,(row+1)*n+i))
mesh=bpy.data.meshes.new('cup');mesh.from_pydata(verts,[],faces);mesh.materials.append(base);o=bpy.data.objects.new('cup',mesh);bpy.context.collection.objects.link(o)
tor((.48,0,.02),.23,.065,rot=(math.pi/2,0,0));finish('SM_Cup')
box((0,0,0),(.92,.90,.75),paper,.01);box((0,0,.43),(1,1,.12),base,.02);box((0,0,-.43),(1,1,.12),base,.02);box((-.47,0,0),(.09,1,.8),base,.02);finish('SM_Book')
box((0,0,-.38),(1,1,.18),base,.06)
for x in [-.45,.45]:box((x,0,.05),(.10,1,.65),base,.03)
for y in [-.45,.45]:box((0,y,.05),(1,.10,.65),base,.03)
finish('SM_Tray')
cyl((0,0,-.46),.38,.08,ink);cyl((0,0,-.08),.055,.75,ink)
bpy.ops.mesh.primitive_cone_add(vertices=32,radius1=.5,radius2=.24,depth=.42,location=(0,0,.28));bpy.context.object.data.materials.append(base);finish('SM_Lamp')
box((0,0,.05),(.13,1,.76),ink,.035);box((-.075,0,.05),(.015,.90,.65),ink,.01);box((0,0,-.4),(.34,.54,.08),base,.02);box((0,0,-.32),(.08,.12,.15),ink,.01);finish('SM_TV')
box((0,0,.08),(.95,.65,.40),base,.09);box((-.10,0,.35),(.45,.58,.27),base,.07)
for x in [-.30,.30]:
 for y in [-.33,.33]:
  o=cyl((x,y,-.22),.2,.10,ink,20);o.rotation_euler[0]=math.pi/2
finish('SM_Car')
# Helical spring accessory, with closed end caps.
verts=[];faces=[];steps=160;sides=8
for j in range(steps):
 a=j/(steps-1)*math.tau*3;center=Vector((.24*math.cos(a),.24*math.sin(a),j/(steps-1)-.5))
 for k in range(sides):b=k*math.tau/sides;verts.append(center+Vector((.045*math.cos(b)*math.cos(a),.045*math.cos(b)*math.sin(a),.045*math.sin(b))))
for j in range(steps-1):
 for k in range(sides):n=(k+1)%sides;faces.append((j*sides+k,j*sides+n,(j+1)*sides+n,(j+1)*sides+k))
mesh=bpy.data.meshes.new('coil');mesh.from_pydata(verts,[],faces);mesh.materials.append(orange);o=bpy.data.objects.new('coil',mesh);bpy.context.collection.objects.link(o);finish('SM_Spring')
box((0,0,0),(.55,.85,1),ink,.08)
for z in [-.23,.24]:
 o=cyl((-.29,0,z),.19,.04,orange,24);o.rotation_euler[1]=math.pi/2
finish('SM_Speaker')
print('ROOM_ART_EXPORTED')
