#######################################################################
#    Copyright (C) 2001 by Matthew Tang (mltang@carbon.cudenver.edu)
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
########################################################################

# export script blender -> crystal space

# this script is based on the csexp212.py
# which is (c) Norman Krämer 2001

# The import Blender210 is used to make the script work with Blender 2.14
# If you want the script to work with Blender 2.12, 2.10, other versions. Then replace all the Blender210 with Blender
# The next version of this script will support UV texture.

import Blender210

sectors = []
things = []
lights = []

class Light:
	name = None
	locX = None
	locY = None
	locZ = None
	red = None
	green = None
	blue = None
	radius = None
	energy = None
	parent = None

	def __init__(self):
		print "Light() init"

class Polygon:
	aface = None
	material = None
	def __init__(self):
		self.aface = []

class Thing:
	name = None
	verts = None
	polygons = None
	name = None
	parent = None
	defaultMaterial = None
	def __init__(self):
		self.verts = []
		self.polygons = []
		self.things = []

class Sector:
	verts = None
	polygons = None
	things = None
	lights = None
	name = None
	defaultMaterial = None

	def __init__(self):
		self.verts = []
		self.polygons = []
		self.things = []
		self.lights = []

def MV(m, v):
	k = [0,0,0]
	k[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0]
	k[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1]
	k[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2]
	return k

scene = Blender210.getCurrentScene()
for name in scene.objects:
	obj = Blender210.getObject(name)
	print obj
	print obj.data
	print "==================="
	if name[0:6] == "light_":
		if Blender210.isLamp(name):
			print "We have a light!"
			lamp = Light()
			lampobj = Blender210.getObject(name)
			lam    = Blender210.getLamp(lampobj.data)

			lamp.name = name
			lamp.parent = lampobj.data
			lamp.locX = lampobj.LocX
			lamp.locY = lampobj.LocY
			lamp.locZ = lampobj.LocZ
			lamp.red = lam.R
			lamp.green = lam.G
			lamp.blue = lam.B
			lamp.radius = lam.Dist
			lamp.energy = lam.Energ * 3
			lights.append(lamp)

	if name[0:7] == "sector_":
		if Blender210.isMesh(name):
			sec = Sector()
			sec.name = name[7:]

			meshobj = Blender210.getObject(name)
			mesh    = Blender210.getMesh(meshobj.data)
			m = meshobj.matrix
			# Add the mesh vertices to the tree
			for v in mesh.vertices:
				sec.verts.append(MV(m,v))
			# set the first material as default one
			if meshobj.materials and meshobj.materials[0]:
				sec.defaultMaterial = meshobj.materials[0]
			# add the mesh faces to the tree
			for face in mesh.faces:
				#if mesh.texture:
				if (meshobj.materials and meshobj.materials[face[5]]):
					namea = meshobj.materials[face[5]]
					print "------aaa--", namea, " -- "
				poly = Polygon()
				poly.aface.append(face)
				# If current face has a material assigned to it, then set the material name for current face
				if face[5]:
					poly.material = meshobj.materials[face[5]]
				sec.polygons.append(poly)
			# Add sector to sectors list
			sectors.append(sec)
	if name[0:6] == "thing_":
		if Blender210.isMesh(name):
			thi = Thing()
			thi.name = name[6:]
			thi.parent = obj.data
			meshobj = Blender210.getObject(name)
			mesh    = Blender210.getMesh(meshobj.data)
			m = meshobj.matrix
			# Add the mesh vertices to the tree
			for v in mesh.vertices:
				thi.verts.append(MV(m,v))
			# set the first material as default one
			if meshobj.materials and meshobj.materials[0]:
				thi.defaultMaterial = meshobj.materials[0]
			# add the mesh faces to the tree
			for face in mesh.faces:
				poly = Polygon()
				poly.aface.append(face)
				# If current face has a material assigned to it, then set the material name for current face
				if face[5]:
					poly.material = meshobj.materials[face[5]]
				thi.polygons.append(poly)
			# Add sector to sectors list
			things.append(thi)
		else:
			print "thing ", name, " is not a mesh!"

# Now write the data tree to a file in CS world format.

file = open("world", "w")

fileStart = open("template", "r")
file.write(fileStart.read())
fileStart.close ()

for i in things:
	file.write("  MESHFACT '%s'(\n" % i.name)
	file.write("    PLUGIN ('thingFact')\n")
	file.write("    PARAMS (\n")

	for o in i.verts:
		file.write ("      VERTEX (%s, %s, %s)\n" % (o[0], o[1], -1*o[2]))
	file.write("      MATERIAL ('%s')\n" % i.defaultMaterial)
	nPoly = 0
	for p in i.polygons:
		file.write ("      POLYGON '%s_%s' (\n" % (i.name, nPoly))
		file.write ("        VERTICES (")
		face = p.aface
		if face[0][3]:
			file.write ("%s,%s,%s," % (face[0][3], face[0][2], face[0][1]))
			file.write ("%s)\n" % face[0][0])
		else:
			file.write ("%s,%s,%s)\n" % (face[0][2], face[0][1], face[0][0]))
		if p.material != None:
			file.write ("        MATERIAL ('%s')\n" % p.material)
			# @@@ Can Things have portals?
		file.write ("      )\n")
		nPoly += 1
	file.write ("    )\n")
	#file.write ("    zfill()\n")
	file.write ("  )\n")

for i in sectors:
	file.write("  SECTOR '%s'(\n    MESHOBJ 'walls' (\n" % i.name)
	file.write("      PLUGIN ('crystalspace.mesh.loader.factory.thing')\n")
	file.write("      PARAMS (\n")
	for o in i.verts:
		file.write ("        VERTEX (%s, %s, %s)\n" % (o[0], o[1], -1*o[2]))
	file.write("        MATERIAL ('%s')\n" % i.defaultMaterial)
	nPoly = 0;
	for p in i.polygons:
		file.write ("        POLYGON '%s_%s' (\n" % (name, nPoly))
		file.write ("          VERTICES (")
		face = p.aface
		if face[0][3]:
			file.write ("%s,%s,%s," % (face[0][0], face[0][1], face[0][2]))
			file.write ("%s)\n" % face[0][3])
		else:
			file.write ("%s,%s,%s)\n" % (face[0][0], face[0][1], face[0][2]))
		if p.material != None:
			if p.material[0:7] == "portal_":
				file.write ("          PORTAL ('%s')\n" % p.material[7:])
			else:
				file.write ("          MATERIAL ('%s')\n" % p.material)
		file.write ("        )\n")
		nPoly += 1
	file.write ("      )\n")
	file.write ("      zfill()\n")
	file.write ("    )\n")

	nThing = 0
	for t in things:
		if t.parent == i.name:
			print "Thing ", t.name, " linked to ", i.name
			file.write ("    MESHOBJ '%s_%s' (\n" % (t.name, nThing))
			file.write ("    PLUGIN ('thing')\n")
			file.write ("      PARAMS (FACTORY ('%s'))\n" % t.name)
			file.write ("    )\n")
			nThing += 1
	for l in lights:
		if l.parent == i.name:
			file.write ("    LIGHT '%s' (\n" % l.name)
			file.write ("      CENTER (%s, %s, %s)\n" % (l.locX, l.locY, l.locZ))
			file.write ("      COLOR (%s, %s, %s)\n" % (l.red * l.energy, l.green * l.energy, l.blue * l.energy))
			file.write ("      RADIUS (%s)\n" % l.radius)
			file.write ("      ATTENUATION (realistic)\n")
			file.write ("    )\n")

	file.write ("  )\n")



file.write (")\n")
file.close ()
print "Done!"



