#######################################################################
#    CoPyLeFt (D) 2001 by Charles Quarra
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


def Write_MeshObj(file,OBJ,plugin,zbuffer):
   file.write("    MESHOBJ '%s' (\n" % OBJ.name )
   file.write("        PLUGIN ('%s')\n" % plugin )
   file.write("        %s ()\n" % zbuffer )
   file.write("        PARAMS (\n")
   for v in OBJ.verts:
      print v.co[0],v.co[1],v.co[2]
      file.write("            V (%3.4f,%3.4f,%3.4f" % (v.co[0], v.co[1], v.co[2]))
      if OBJ.has_uvco:
            file.write(":%3.4f,%3.4f" % (v.uvco[0], v.uvco[1]))
      file.write("    )\n")
   for face in OBJ.faces:
        num = len(face.v)
        if num == 3:
            file.write("            POLYGON (\n")
            file.write("                V (%d,%d,%d)\n" % (face.v[0].index, face.v[1].index, face.v[2].index))
	    file.write("            MATERIAL ('%s')\n" % string.replace(face.image.name ,".","_") )
            if OBJ.hasFaceUV:
                if len(face.uv) > 0:
                   file.write("                TEXTURE (\n")
                   file.write("                    UV (0, %3.4f, %3.4f, 1, %3.4f, %3.4f, 2, %3.4f, %3.4f)\n" % (face.uv[0][0], 1.0 - face.uv[0][1], face.uv[1][0], 1.0 - face.uv[1][1], face.uv[2][0], 1.0 - face.uv[2][1]))
                   file.write("                )\n")
        if num == 4:
            file.write("           POLYGON (\n")
            file.write("                V (%d,%d,%d,%d)\n" % (face.v[0].index, face.v[1].index, face.v[2].index, face.v[3].index))
            file.write("            MATERIAL ('%s')\n" % string.replace(face.image.name ,".","_") )
	    if OBJ.hasFaceUV:
                if len(face.uv) > 0:
                    file.write("                TEXTURE (\n")
                    file.write("                    UV (0, %3.4f, %3.4f, 1, %3.4f, %3.4f, 2, %3.4f, %3.4f , 3, %3.4f, %3.4f)\n" % (face.uv[0][0], 1.0 - face.uv[0][1], face.uv[1][0], 1.0 - face.uv[1][1] , face.uv[2][0], 1.0 - face.uv[2][1], face.uv[3][0], 1.0 - face.uv[3][1]))
                    file.write("                )\n")
        file.write("            )\n")
   file.write("            )\n")
   file.write("        )\n")
   file.write("    )\n")

def Write_Camera(file,OBJ):
   print " not need to implemented it yet, just one camera"

def Write_Lamp(file,light):
   OBJ=Blender.Object.Get(light.name)
   file.write("      LIGHT (\n")
   file.write("       CENTER (%3.4f,%3.4f,%3.4f)\n" % ( OBJ.loc.x , OBJ.loc.y , OBJ.loc.z ) )
   file.write("       RADIUS (%3.4f)\n" % ( 5*light.Energ*light.Energ ) )
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
  global CSPATH,WorkDir
  CSPATH = Create("/home/charles/fuckyo/CS/CS/")
  WorkDir = Create("./WorkDir/")
  Register(gui, event, bevent)

def Run():
 global CSPATH,WorkDir
 filename = "world"
 if not (os.path.exists(WorkDir.val)):
   os.system("mkdir " + WorkDir.val)

 file = open(WorkDir.val + filename,"w")

 file.write("WORLD (\n")

# TEXTURES and MATERIALS definitions

 file.write("    TEXTURES (\n")
 for ob in BlendModule.Image.Get():
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
     nickname=string.replace(ob.name,".","_")
     file.write("        MATERIAL '%s' (TEXTURE ('%s'))\n" % ( nickname , nickname ) )
 file.write("    )\n")
 Write_Defaults(file)

# to set the start i should know in what sector is the camera...how im going to do that?

 camera= Blender.Object.Get("Camera")
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
   if (Blender210.isCamera(M.name)):
       print M.name,"by now just camera 'Camera' and is the START"
   elif (Blender210.isLamp(M.name)):
     try:
       Write_Lamp(file,M)
       print " wrote Lamp ",M
     except:
       print M.name,"doesnt seem to be a valid lamp"
 for ob in obj:
   M = ob.data
   if (Blender210.isMesh(M.name)):
     try:
       Write_MeshObj(file,M,"thing","ZFILL")
       print " wrote MESHOBJ ",M
     except:
       print M.name,"doesnt seem to be a valid mesh"

 file.write(")\n")    #end SECTOR 'room' block
 file.write(")\n")    #end WORLD block
 file.close()
#zipping it and copying it to CS/data/flarge
 os.system('rm -f exported.zip')
 os.system('zip -rj exported.zip ' + WorkDir.val +'*')
#os.system('rm -f '+ CSPATH.val + 'data/flarge/exported.zip')
 os.system('cp -v exported.zip ' + CSPATH.val + 'data/flarge/' )
#os.system(CSPATH.val + '/walktest blend')


############### GUI interface

def gui():
    global CSPATH,WorkDir
    glClearColor( 0.5, 0.5 , 0.5 , 0.0)
    glClear(GL_COLOR_BUFFER_BIT)
    glRasterPos2i(50,500)
    Text("The Really Working Blender -> CrystalSpace exporter script ")
    glRasterPos2i(20,420)
    Text("After exporting, your level is copied to $CSPATH$/data/flarge ")
    glRasterPos2i(20,400)
    Text("for convenience, you should introduce an entry in vfs.cfg to be ")
    glRasterPos2i(20,380)
    Text("able to call your level like:")
    glRasterPos2i(20,360)
    Text("walktest (whatever VFS directory you're mounting your level)")
    Button("Exit",1,200,100,100,19)
    CSPATH=String("CS path (absolute path):",2,20,300,400,20,CSPATH.val,100)
    WorkDir=String("Working Dir (relative path):",3,20,250,400,20,WorkDir.val,100)
    Button("Export",4,60,100,100,19)


def event(evt,val):
   if (evt==ESCKEY and not val ): Exit()

def bevent(evt):
   if (evt==1): Exit()
   elif (evt==4):
     Run()
     Register(gui, event, bevent)

Init()




