#######################################################################
#    CoPyLeFt (D) 2002 by Charles Quarra (charsquarra@hotmail.com)
#
#    This script is free software; you can redistribute it and/or
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
#   Full Export Blender->Crystal Space map format

import Blender
import Blender210
import math
import GUI
import string
import os
import commands

from Blender.Draw import *
from Blender.BGL import *


print "Start"
print dir(Blender)




CSobjlist = []
print dir(CSobjlist)
ObjSelectName = ""
CSselected=Blender.Object.GetSelected()[0]

#for obj in Blender.Object.Get():
#  CSobjlist.append( CSObject(obj) )
#  print CSobjlist

class CSLight:
   BlObj=Blender.Object.GetSelected()[0]
   light=Blender.Object.GetSelected()[0].data
   Halo= [ 0 , 0 , 0 ]
   Attenuation = Create(1)
   def __init__(self,BlendObj):
      print "CSLight init: "
      self.BlObj=BlendObj
      self.light=BlendObj.data
   def get_CENTER(self):
      return [self.BlObj.loc.x , self.BlObj.loc.y , self.BlObj.loc.z ]
   def get_RADIUS(self):
      return 10*light.Energ
   def get_COLOR(self):
      return [self.light.R , self.light.G , self.light.B ]



class CSMeshObject:
   BlObj=Blender.Object.GetSelected()[0]
   plugin=Create(1)
   zbuffermode=Create(1)
   priority=Create(1)
   mesh=Blender.Object.GetSelected()[0].data
   def __init__(self,BlendObj):
       print "CSMeshObject init: "
       self.BlObj=BlendObj
       self.mesh=BlendObj.data


#for now it just loads everything
def Write_Defaults(file):
   file.write(" RENDERPRIORITIES (\n")
   file.write(" PRIORITY 'sky' (1,NONE)\n")
   file.write(" PRIORITY 'wall' (2,NONE)\n")
   file.write(" PRIORITY 'object' (3,NONE)\n")
   file.write(" PRIORITY 'alpha' (4,BACK2FRONT)\n")
   file.write(" )\n")
   file.write(" PLUGINS (\n")
   file.write(" PLUGIN 'thing' ('crystalspace.mesh.loader.thing')\n")
   file.write(" PLUGIN 'thingFact' ('crystalspace.mesh.loader.factory.thing')\n")
   file.write(" PLUGIN 'plane' ('crystalspace.mesh.loader.thing.plane')\n")
   file.write(" PLUGIN 'bezier' ('crystalspace.mesh.loader.thing.bezier')\n")
   file.write(" PLUGIN 'spr3dFact' ('crystalspace.mesh.loader.factory.sprite.3d')\n")
   file.write(" PLUGIN 'spr3d' ('crystalspace.mesh.loader.sprite.3d')\n")
   file.write(" PLUGIN 'spr2dFact' ('crystalspace.mesh.loader.factory.sprite.2d')\n")
   file.write(" PLUGIN 'spr2d' ('crystalspace.mesh.loader.sprite.2d')\n")
   file.write(" PLUGIN 'treeFact' ('crystalspace.mesh.loader.factory.genmesh.tree')\n")
   file.write(" PLUGIN 'meshFact' ('crystalspace.mesh.loader.factory.genmesh')\n")
   file.write(" PLUGIN 'mesh' ('crystalspace.mesh.loader.genmesh')\n")
   file.write(" PLUGIN 'metafact' ('crystalspace.mesh.loader.factory.metaball')\n")
   file.write(" PLUGIN 'metablob' ('crystalspace.mesh.loader.metaball')\n")
   file.write(" PLUGIN 'emitFact' ('crystalspace.mesh.loader.factory.emit')\n")
   file.write(" PLUGIN 'emit' ('crystalspace.mesh.loader.emit')\n")
   file.write(" )\n")


def Write_MeshObj(file,OBJ,Parent,plugin,zbuffer,priority):
   print " PRIORITY IS ",priority
   try:
     name=OBJ.name
     Parentcoords=[ Parent.LocX , Parent.LocY, Parent.LocZ ]
     coords=[OBJ.verts[0].co[0],OBJ.verts[0].co[1],OBJ.verts[0].co[2] ]
   except:
     print " bad vertexes "
     return
   try:
     facetest=OBJ.faces[0]
     name=facetest.image.name
   except:
     print "bad faces"
     return
   file.write("    MESHOBJ '%s' (\n" % OBJ.name )
   file.write("        PLUGIN ('%s')\n" % plugin )
   file.write("        %s ()\n" % zbuffer )
   file.write("   PRIORITY('%s')\n" % priority )
   file.write("        PARAMS (\n")
   print "Writing vertexes..."
   for v in OBJ.verts:
      matrix=Parent.matrix
#     v0=v.co[0]*matrix[0][0] + v.co[1]*matrix[0][1] + v.co[2]*matrix[0][2]
#     v1=v.co[0]*matrix[1][0] + v.co[1]*matrix[1][1] + v.co[2]*matrix[1][2]
#     v2=v.co[0]*matrix[2][0] + v.co[1]*matrix[2][1] + v.co[2]*matrix[2][2]
      v0=v.co[0]*matrix[0][0] + v.co[1]*matrix[1][0] + v.co[2]*matrix[2][0]
      v1=v.co[0]*matrix[0][1] + v.co[1]*matrix[1][1] + v.co[2]*matrix[2][1]
      v2=v.co[0]*matrix[0][2] + v.co[1]*matrix[1][2] + v.co[2]*matrix[2][2]
#     print v.co[0],v.co[1],v.co[2]
      file.write("            V (%3.9f,%3.9f,%3.9f" % (v0 + Parent.LocX, v1+ Parent.LocY, v2 + Parent.LocZ))
      if OBJ.has_uvco:
            file.write(":%3.9f,%3.9f" % (v.uvco[0], v.uvco[1]))
      file.write("    )\n")
   print "Succesful vertex write"
   print "Writing Faces..."
   for face in OBJ.faces:
        num = len(face.v)
        if num == 3:
            file.write("            POLYGON (\n")
            file.write("                V (%d,%d,%d)\n" % (face.v[0].index, face.v[1].index, face.v[2].index))
	    file.write("            MATERIAL ('%s')\n" % string.replace(face.image.name ,".","_") )
	    if OBJ.hasFaceUV:
                if len(face.uv) > 0:
                   file.write("                TEXTURE (\n")
                   file.write("                    UV (0, %3.9f, %3.9f, 1, %3.9f, %3.9f, 2, %3.9f, %3.9f)\n" % (face.uv[0][0], 1.0 - face.uv[0][1], face.uv[1][0], 1.0 - face.uv[1][1], face.uv[2][0], 1.0 - face.uv[2][1]))
                   file.write("                )\n")
        if num == 4:
            file.write("           POLYGON (\n")
            file.write("                V (%d,%d,%d,%d)\n" % (face.v[0].index, face.v[1].index, face.v[2].index, face.v[3].index))
	    file.write("            MATERIAL ('%s')\n" % string.replace(face.image.name ,".","_") )
	    if OBJ.hasFaceUV:
                if len(face.uv) > 0:
                    file.write("                TEXTURE (\n")
                    file.write("                    UV (0, %3.9f, %3.9f, 1, %3.9f, %3.9f, 2, %3.9f, %3.9f , 3, %3.9f, %3.9f)\n" % (face.uv[0][0], 1.0 - face.uv[0][1], face.uv[1][0], 1.0 - face.uv[1][1] , face.uv[2][0], 1.0 - face.uv[2][1], face.uv[3][0], 1.0 - face.uv[3][1]))
                    file.write("                )\n")
	file.write("            )\n")
   file.write("            )\n")
   file.write("        )\n")
#  file.write("    )\n")

def Write_Camera(file,OBJ):
   print " not need to implemented it yet, just one camera"

def Write_Lamp(file,light):
   OBJ=Blender.Object.Get(light.name)
   file.write("      LIGHT (\n")
   file.write("       CENTER (%3.4f,%3.4f,%3.4f)\n" % ( OBJ.loc.x , OBJ.loc.y , OBJ.loc.z ) )
   file.write("       RADIUS (%4.3f)\n" % ( 50*((light.Energ+1.)*(light.Energ+1.)) ) )
   file.write("      COLOR  (%3.4f,%3.4f,%3.4f)\n" % ( light.R ,light.G , light.B ) )
   file.write(" HALO  (0,0,0) ATTENUATION (none)  )\n")


# the current common Blender Module
BlendModule = Blender

# 210 API Blender Module
Blend210Module = Blender210

#print dir(obj)

obj = BlendModule.Object

#print dir(obj)

obj = obj.Get()

def Init():
# WARNING: THIS MUST BE SET BY HAND!! ...ok maybe i just make a GUI :)
  global CSPATH,WorkDir,ObjSelectName,CSobjlist,CSselected,TogClearBuf,TogClearScr
  CSPATH = Create("/home/charles/fuckyo/CS/CS/")
  WorkDir = Create("./WorkDir/")
  TogClearBuf=Create(1)
  TogClearScr=Create(1)
  CSobjlist = []
  CSselected=[]
  ObjSelectName = ""
  objects=Blender.Object.Get()
  for obj in objects:
     if obj.data.block_type=="NMesh":
        CSobjlist.append( CSMeshObject(obj) )
     elif obj.data.block_type=="Lamp":
        CSobjlist.append( CSLight(obj) )
#    elif obj.data.block_type=="Camera":
#       CSobjlist.append( obj )
  Register(gui, event, bevent)

def Run():
 global CSPATH,WorkDir,ObjSelectName,CSobjlist,TogClearBuf,TogClearScr
 filename = "world"
 if not (os.path.exists(WorkDir.val)):
   os.system("mkdir " + WorkDir.val)

 file = open(WorkDir.val + filename,"w")

 file.write("WORLD (\n")
 file.write(" SETTINGS(\n")
 if TogClearBuf.val:
    file.write(" CLEARZBUF()\n")
 if TogClearScr.val:
    file.write(" CLEARSCREEN()\n")
 file.write(" )\n")
# TEXTURES and MATERIALS definitions

 file.write("    TEXTURES (\n")
 for ob in BlendModule.Image.Get():
     actualfile=ob.name
     if actualfile!="":
       print "searching for ",ob.name
       search=commands.getoutput('locate -r ' + ob.name)
       strlist=search.splitlines()
       for str in strlist:
         os.system('cp -v -u ' + str + ' ' + WorkDir.val)
         if (os.name=="posix"):
           st=str.split('/')
         else:
           st=str.split('\\')
         for actualfile in st:
           print "looping list at ",actualfile

       nickname=string.replace(ob.name,".","_")
       file.write("        TEXTURE '%s' (FILE (/lev/blend/%s))\n" % ( nickname , actualfile ) )

 file.write("    )\n")
 file.write("    MATERIALS (\n")
 for ob in BlendModule.Image.Get():
     actualfile=ob.name
     if actualfile!="":
       nickname=string.replace(ob.name,".","_")
       file.write("        MATERIAL '%s' (TEXTURE ('%s'))\n" % ( nickname , nickname ) )
 file.write("    )\n")
 Write_Defaults(file)

# to set the start i should know in what sector is the camera...how im going to do that?

 camera=Blender.Object.Get("Camera")
 cam= Blender.Object.Get()
 for ob in cam:
   if ob.data.block_type=="Camera":
     camera=ob
 if camera:
         file.write("    START (\n")
         file.write("        SECTOR ('room')\n")
         file.write("        POSITION (%3.4f,%3.4f,%3.4f)\n" % (camera.loc.x, camera.loc.y, camera.loc.z))
         file.write("        UP (0,0,1)\n")
         file.write("        FORWARD (0,1,0)\n")
         file.write("    )\n")
 file.write("  SECTOR 'room' (\n")
 for ob in obj:
   M = ob.data
   try:
     if (M.block_type=="Camera"):
       print M.name,"by now just camera 'Camera' and is the START"
     elif (M.block_type=="Lamp"):
       try:
         for CSobj in CSobjlist:
          if CSobj:
	    if CSobj.BlObj.data.name==M.name:
              if M.block_type=="Lamp":
                 Write_Lamp(file,M)
                 print " wrote Lamp ",M
       except:
         print M.name,"doesnt seem to be a valid lamp"
   except:
     print "something happen at Blokc_type"

 for ob in obj:
   print "exporting mesh ",ob.name
   M = ob.data
   try:
    if (M.block_type=="NMesh"):
      try:
        for CSobj in CSobjlist:
          if CSobj:
	    if CSobj.BlObj.data.name==M.name:
              if M.block_type=="NMesh":
	        Zmode="ZFILL"
		if CSobj.zbuffermode.val==2:
		   Zmode="ZUSE"
		elif CSobj.zbuffermode.val==3:
		   Zmode="ZTEST"
		elif CSobj.zbuffermode.val==4:
		   Zmode="ZNONE"
		plug="thing"
		if CSobj.plugin.val==2:
		   plug="spr3d"
		elif CSobj.plugin.val==3:
		   plug="mesh"
		priority="object"
		if CSobj.priority.val==2:
		  priority="sky"
		elif CSobj.priority.val==3:
		  priority="wall"
                elif CSobj.priority.val==4:
		  priority="alpha"
		Write_MeshObj(file,M, ob , plug , Zmode ,priority)
                print " wrote MESHOBJ ",M
      except:
           print M.name,"doesnt seem to be a valid mesh"
   except:
      print "something happen at Blender210.isMesh()"
 file.write(")\n")    #end SECTOR 'room' block
 file.write(")\n")    #end WORLD block
 file.close()
#zipping it and copying it to CS/data/flarge
 os.system('rm -f exported.zip')
 os.system('zip -rj exported.zip ' + WorkDir.val +'*')
#os.system('rm -f '+ CSPATH.val + 'data/flarge/exported.zip')
 os.system('cp -v exported.zip ' + CSPATH.val + 'data/flarge/' )
#os.system('cp -v exported.zip $CRYSTAL/data/flarge/')
#os.system(CSPATH.val + '/walktest blend')

############### GUI interface

def gui():
    global CSPATH,WorkDir,ObjSelectName,CSobjlist,CSselected,TogClearBuf,TogClearScr
    glClearColor( 0.5, 0.5 , 0.5 , 0.0)
    glClear(GL_COLOR_BUFFER_BIT)
    if CSselected!=[]:
#     print " CSselected is now ",CSselected
      if CSselected.BlObj.data.block_type=="NMesh":
          CSselected.plugin=Menu("thing|spr3d|mesh",5,20,150,100,20,CSselected.plugin.val)
          CSselected.zbuffermode=Menu("ZFILL|ZUSE|ZTEST|ZNONE",6,20,280,100,20,CSselected.zbuffermode.val)
	  CSselected.priority=Menu("object|sky|wall|alpha",10,20,210,100,20,CSselected.priority.val)
      elif CSselected.BlObj.data.block_type=="Lamp":
          CSselected.Attenuation=Menu("none",7,20,150,100,20,CSselected.Attenuation.val)
#General Settings Buttons
    TogClearBuf=Toggle("ZBUFCLEAR",11,150,280,100,20,TogClearBuf.val)
    TogClearScr=Toggle("CLEARSCREEN",12,150,240,100,20,TogClearScr.val)
#Export Buttons
    Button("Exit",1,200,60,100,19)
    CSPATH=String("CS path (absolute path):",2,20,100,400,20,CSPATH.val,100)
    WorkDir=String("Working Dir (relative path):",3,20,80,400,20,WorkDir.val,100)
    Button("Export",9,60,60,100,19)


def event(evt,val):
   global ObjSelectName,CSobjlist,CSselected,TogClearBuf,TogClearScr
   ob=Blender.Object.GetSelected()
   if ob and ob[0].name!=ObjSelectName:
      ObjSelectName=ob[0].name
      Redraw()
   obj=Blender.Object.Get(ObjSelectName)
   isdefined="false"
   for csobj in CSobjlist:
#     print obj," but ",csobj.BlObj
      if csobj.BlObj.name==obj.name:
         CSselected=csobj
#        print "EQUAL!"
         isdefined="true"
   if isdefined=="false":
      if obj.data.block_type=="NMesh":
#        print "adding a NMesh"
         CSobjlist.append( CSMeshObject(obj) )
      elif obj.data.block_type=="Lamp":
#        print "adding a light"
         CSobjlist.append( CSLight(obj) )
#     elif obj.data.block_type=="Camera":
#       CSobjlist.append( obj )
   if (evt==ESCKEY and not val ): Exit()

def bevent(evt):
   if (evt==1): Exit()
   elif (evt==9):
     Run()
     Register(gui, event, bevent)

Init()




