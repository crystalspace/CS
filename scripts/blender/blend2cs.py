# File: blend2cs.py
# Created: Rene Dudfield 22 June
#
# Purpose: to save the currently selected blender object into a crystal space
#  format.
#
# Todo: will load the data directly into cs structures instead of using a
#       file format.  Then they can be pickled.

import sys

sys.path.append("scripts")

import cswriter

import Blender

# Select the first object to save.
OBJECT_NAME = Blender.Object.GetSelected()[0].name

# Name of the file to save the thing in.
THING_FILE_SAVE = "athing"

# Name of the file to save the sprite in.
SPRITE_FILE_SAVE = "asprite"


mesh = Blender.NMesh.GetRaw(OBJECT_NAME)


#########################################################################
# Do the sprite thing.

spriteVertexList = []


for x in mesh.verts:
  print x.co
  print x.co
  print x.co
  print x.co
  
  t = []
  t.append(x.co[0])
  t.append(x.co[1])
  t.append(x.co[2])
  t.append(x.uvco[0])
  t.append(x.uvco[1])
  
  spriteVertexList.append(cswriter.csSpriteVertex(x.co[0],x.co[1],x.co[2],
                                                  x.uvco[0],x.uvco[1]))




# Bad ju ju :(
# No animation frames are done yet.

  sf1 = cswriter.csSpriteFrame("stand1", spriteVertexList)
  
  triangleList = []
  
  for x in mesh.faces:
    tri = []
    
    tri.append(x.v[0].index)
    tri.append(x.v[1].index)
    tri.append(x.v[2].index)
    
    triangleList.append(cswriter.csTriangle(tri))


  act1 = cswriter.csAction("stand",[["stand1",30], ["stand1", 100]])
  
  asprite = cswriter.csSprite3d([sf1], triangleList, [act1], "sydney.gif", "sydney")


#######################################################################
# Do the thing thing.

cswrite = cswriter.csWriter()

verts = []

for x in mesh.verts:
  verts.append(x.co)


faces = []

for x in mesh.faces:
  tmp = []
  for v in x.v:
    tmp.append(v.index)
  #  using * 1 to make a copy of the list.
  faces.append(tmp * 1)


ding = "TEXNR ('andrew_wood.gif') TEXLEN (1)\n"
thing = cswrite.csThingWriter(verts, faces, "something", 2 ,ding)


f = open(THING_FILE_SAVE,"w")
f.write(thing)
f.close()

f = open(SPRITE_FILE_SAVE,"w")
f.write(str(asprite))
f.close()
