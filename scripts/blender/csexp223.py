#######################################################################
#    Copyright (C) 2001 by Norman Krämer
#    Copyright (C) 2002 by Yvon Tanguy
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
# since the GUI module is currently rather poor (theres only a FileSelector at this moment)
# you will have to edit the list of meshes to export

# this script is based on the csexp212.py for old Blender API
# this script is based on the povexport.py that comes with blender 2.12, 
# which is (c) Jan Walter 2000


import Blender210
import math
import string
#import dospath

# edit the list of meshes you want to export or leave it undefined to export all
exportMeshes = None #["Cube.004"]
# prepend this to the filename of textures
texPrefix = "textures/"
fileExport = "\\tmp\\tmp-map.tmp" # on Windows
# fileExport = "/tmp/tmp-map.tmp" # on Linux

# MESHFACT prefix
mfPrefix = "mf_"
# SECTOR prefix
sePrefix = "se_"
# START prefix
saPrefix = "sa_"
# LIGHT prefix
liPrefix = "li_"

class CSExport:
    """exports a blender 2.12 scene to CrystalSpace things"""
    def __init__(self, filename):
        self.filename = filename
        self.file    = None
        self.scene   = None
        self.display = None
        self.lampNames = []
        self.meshNames = []
        self.materialNames = []
        self.textureNames = []
	self.meshObjects = []
	self.nbObjects = 0
	self.thingObjects = []
	self.nbThings = 0
	self.lightObjects = []
    # __init__

    
    def writeCamera(self):
	# since CS doesnt know the notion of a CAMERA in its fileformat (yet), we will save it in an KEY
        camobj = self.scene.getCurrentCamera()
        camera = Blender210.getCamera(camobj.data)
        angle = 360.0 * math.atan(16.0 / camera.Lens) / math.pi
	self.file.write ("; CAMERA: <vec3-right>, <vec3-up>, angle, <vec3-rotate>\n")
        self.file.write("KEY ('CAMERA', '-%s, 0, 0"
			% (self.display.xResolution / float(self.display.yResolution)))
        self.file.write(", 0, 1, 0, %s, 0, 180, 0')\n" % angle)
    # writeCamera

    def prepName (self, name, replacelist, makeUpper):
	for c in replacelist:
	    name = string.replace(name, c, "_")
	if makeUpper:
	    name = string.upper (name)
	return name
    # prepName
    
    def writeMaterials(self):
        self.file.write ("  MATERIALS (\n")
        for name in self.scene.objects:
	    if Blender210.isMesh(name):
		object = Blender210.getObject(name)
		if object.materials:
		    for matName in object.materials:
			# materials can be shared !!!
			if matName not in self.materialNames and matName:
			    self.materialNames.append(matName)
			    matname = self.prepName (matName, ".", 1)
			    self.file.write("    MATERIAL 'MA_%s' (TEXTURE ('%s'))\n" % (matname,matname))
		mesh = Blender210.getMesh (object.data)
		if mesh.texture:
		    matName = mesh.texture
		    matname = self.prepName (matName, ".\\/", 1)
		    self.file.write("    MATERIAL 'MA_%s' (TEXTURE ('%s'))\n" % (matname, matname))
	self.file.write("  )\n")
    # writeMaterials

    def writeTextures(self):
		self.file.write ("  TEXTURES (\n")
		for name in self.scene.objects:
			if Blender210.isMesh(name):
				object = Blender210.getObject(name)
				mesh   = Blender210.getMesh(object.data)
				if mesh.texture:
					if mesh.texture not in self.textureNames:
						texname = self.prepName (mesh.texture, ".\\/", 1)
						self.textureNames.append(mesh.texture)
						filename = mesh.texture
						if texPrefix:
							filename = texPrefix + dospath.split (filename)[1] + ".jpeg"
							#filename = filename.lower()
						self.file.write("    TEXTURE '%s' (FILE (%s))\n" % (texname, filename))
			
		self.file.write(";currently there is no way to find the filename of a texture\n")
		self.file.write(";thats bound to a material. So you have to edit the filenames")
		self.file.write(" of the textures below\n")
		for name in self.scene.objects:
			if Blender210.isMesh(name):
				object = Blender210.getObject(name)
				if object.materials:
					for matName in object.materials:
						if matName and matName not in self.textureNames:
							self.textureNames.append(matName)
							matname = self.prepName (matName, ".", 1)
							if texPrefix:
								filename = texPrefix + matname
							else:
								filename = matname
							self.file.write("    TEXTURE '%s' (FILE (%s))\n" % (matname, filename))
		self.file.write("  )\n")
	# writeTextures

    def writeLight(self, name):
        if Blender210.isLamp(name):
            print "write light " + name
            lampobj = Blender210.getObject(name)
            lamp    = Blender210.getLamp(lampobj.data)
            if lampobj.data not in self.lampNames:
                self.lampNames.append(lampobj.data)
                lightname = string.replace(lampobj.data, ".", "_")
                lightname = string.upper(lightname)
                self.file.write("    LIGHT '%s' (\n" % lightname)
                self.file.write("        CENTER (%s, %s, %s)\n" % (lampobj.LocX, lampobj.LocY, lampobj.LocZ))
                self.file.write("        COLOR (%s, %s, %s)\n" % (lamp.R, lamp.G, lamp.B))
                self.file.write("        RADIUS (%s)\n" % lamp.Dist)
                self.file.write(";        ATTENUATION (realistic)\n")
                self.file.write("    )\n")
    # writeLight

    def UVCoo(self, mesh, vert, idx):
	return "%s, %s, %s" % (vert, mesh.texcoords[idx][vert][0], mesh.texcoords[idx][vert][1])
    # UVCoo
    
    def UVString(self, mesh, faceidx):
	return "UV (%s, %s, %s)" % (self.UVCoo(mesh, 0, faceidx),
				    self.UVCoo(mesh, 1, faceidx),
				    self.UVCoo(mesh, 2, faceidx))
    #def UVString

    def MV(self, m, v):
	k = [0,0,0]
	k[0] = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0]
	k[1] = m[0][1] * v[0] + m[1][1] * v[1] + m[2][1] * v[2] + m[3][1]
	k[2] = m[0][2] * v[0] + m[1][2] * v[1] + m[2][2] * v[2] + m[3][2]
	return k
    # MV
    
    def printM(self, m):
	print "%s %s %s %s\n" % (m[0][0], m[1][0], m[2][0], m[3][0])
	print "%s %s %s %s\n" % (m[0][1], m[1][1], m[2][1], m[3][1])
	print "%s %s %s %s\n" % (m[0][2], m[1][2], m[2][2], m[3][2])
    # printM

    def writeThing(self, name):
        if Blender210.isMesh(name):
	    meshobj = Blender210.getObject(name)
	    mesh    = Blender210.getMesh(meshobj.data)
	 
	    if meshobj.data not in self.meshNames:
		self.meshNames.append(meshobj.data)
		meshname = string.replace(meshobj.data, ".", "_")
		meshname = string.upper(meshname)
		m = meshobj.matrix
		self.file.write("    PLUGIN ('crystalspace.mesh.loader.factory.thing')\n")
		self.file.write("    PARAMS (\n")
		# write all vertices
		for v in mesh.vertices:
                    v = self.MV(m,v)
		    self.file.write ("      VERTEX (%s, %s, %s)\n" % (v[0], v[1], -1*v[2]))
		# set the first material as default one
		if meshobj.materials and meshobj.materials[0]:
		    matName = meshobj.materials[0]
		    matname = string.replace(matName, ".", "_")
		    mname = "MA_" + string.upper(matname)
		    self.file.write("      MATERIAL ('%s')\n" % mname)
		    
		# write all polygons
		nPoly = 0
		meshObjName = name.split("_", 2)
		if len(meshObjName) == 3:
			meshObjName = meshObjName[2]
		else:
			meshObjName = name[3:]
		for face in mesh.faces:
		    self.file.write ("      POLYGON '%s_%s' (\n" % (meshObjName, nPoly))
		    self.file.write ("        VERTICES (")
		    if face[3]:
			self.file.write ("%s," % face[3])
		    self.file.write ("%s,%s,%s)\n" % (face[2], face[1], face[0]))
		    nUV = 0
		    if mesh.texture:
			self.file.write ("        TEXTURE (\n")
			self.file.write ("          %s\n" % self.UVString (mesh, nPoly))
			self.file.write ("        )\n")
			matname = self.prepName(mesh.texture, ".\\/",1)
			self.file.write("        MATERIAL ('MA_%s')\n" % matname)
		    
		    if face[5]:
			matname = self.prepName(meshobj.materials[face[5]], ".",1)
			self.file.write ("        MATERIAL ('MA_%s')\n" % matname)
		    if mesh.colors:
			self.file.write ("        COLORS (%s,%s,%s" % (getColor(mesh.colors[face[0]]),
								     getColor(mesh.colors[face[1]]),
								     getColor(mesh.colors[face[2]])))
			if face[3]:
			    self.file.write (",%s", getColor(mesh.colors[face[3]]))
			self.file.write ("\n)")
		    self.file.write ("      )\n")
		    nPoly += 1
		self.file.write ("    )\n")
	else:
            print "Sorry can export meshes only ..."
    # writeThing

    def writeEnd(self):
        print "... finished"
    # writeEnd

    def writeObject(self, name):
        if Blender210.isMesh(name):
            object = Blender210.getObject(name)
	    if name[0:3] == mfPrefix:
	        meshFactName = name.split("_", 2)
		self.file.write("  MESHFACT '%s'(\n" % meshFactName[2])
		self.writeThing (name)
		self.file.write("  )\n")
    # writeObject

    def writeSector(self, name):
	if Blender210.isMesh(name):
	    self.file.write("  SECTOR '%s'(\n" % name)
	    self.file.write("    MESHOBJ '%s' (\n" % name[3:])
	    self.writeThing(name)
	    self.file.write("    ZFILL()\n")
            self.file.write("    )\n")
	    for thingName in self.thingObjects:
	    	if thingName.find("_" + name[3:] + "_") != -1:
	    		meshObjName = thingName.split("_", 2)
			self.file.write("    MESHOBJ '%s' (\n" % meshObjName[2])
			self.file.write("      PLUGIN('crystalspace.mesh.loader.factory.thing')\n")
			self.file.write("      PARAMS (FACTORY('%s'))\n" % meshObjName[2])
                	self.file.write("    )\n")
            for lightName in self.lightObjects:
            	if lightName[3:] == name[3:]:
            		self.writeLight(lightName)
            self.file.write("  )\n")
    # writeSector

    def writeSectors(self):
	for name in self.meshObjects:
		print "write sector " + name
		self.writeSector(name)
    # writeSectors
    
    def writeStart(self, name):
		print "write start position"
		if Blender210.isMesh(name):
			meshobj = Blender210.getObject(name)
			mesh = Blender210.getMesh(meshobj.data)
			object = Blender210.getObject(name)
			self.file.write("  START (\n")
			self.file.write("    SECTOR('%s')\n" % ("se_" + name[3:]))
			v = mesh.vertices[0]
			self.file.write("    POSITION(%s, %s, %s)\n" % (v[0], v[1], v[2]))
			self.file.write("  )\n")
    # writeStart

    def writeWorld(self):
		self.file.write ("WORLD (\n")
		# TEXTURES
		self.writeTextures ()
		# MATERIALS
		self.writeMaterials ()
		# MESH FACTORIES
		for name in self.scene.objects:
			if not Blender210.isCamera(name) and (not exportMeshes or name in exportMeshes):
				print "found object " + name
				if name[0:3] == mfPrefix:
					print "write object " + name
					self.writeObject(name)
					self.thingObjects.append(name)
				if name[0:3] == sePrefix:
					self.meshObjects.append(name)
				if name[0:3] == saPrefix:
					self.writeStart(name)
				if name[0:3] == liPrefix:
					self.lightObjects.append(name)
				
		self.writeSectors()
		self.file.write(")\n")
		# writeWorld

    def export(self, scene):
	global exportAnimations

	print "exporting  to " + self.filename + "..."
	self.scene = scene
	self.display = Blender210.getDisplaySettings()
	self.file = open(self.filename, "w")
	self.writeWorld()
	self.file.close ()
	print "exporting: done\n"
	# export
# class CSExport

scene = Blender210.getCurrentScene()
csexport = CSExport(fileExport)
csexport.export(scene)


