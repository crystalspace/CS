############################
# (c) Sebastian Duell 2001 #
############################

# This script exports Crystalspace meshobjects with animations.
# It needs a parameterfile for every exported object which contains
# the "ACTION" definitions for the animation. This file must 
# have the name of the object in Crystalspace with the extension .ani
# The statement F(f0,200) must not contain spaces.
# Unused frames are not saved.

import Blender210

exportAnimations = 1

class CrystalSpaceExport:
    def __init__(self, filename):
        self.file    = open(filename, "w")
        self.scene   = None
        self.display = None

    def export(self, scene):
        global exportAnimations

        print "exporting ..."
        self.scene = scene
        self.display = Blender210.getDisplaySettings()

        if exportAnimations:
            Blender210.setCurrentFrame(self.display.currentFrame)
            names = []
            for name in self.scene.objects:
                if not Blender210.isCamera(name) and not Blender210.isLamp(name):
                   names.append(name)
            for Name in names:
                self.file.close()
                self.file = open(Name, "w")
                anifile = open(Name + ".ani", "r")
                frames = []
                line = anifile.readline()
                while line != "":
                    i=0
                    while i<len(line):
                        if line[i] == 'F' and line[i+1] == '(':
                           fr = ""
                           i = i + 2
                           while line[i] != ',':
                               fr = fr + line[i]
                               i = i +1
                           frames.append(fr)
                        i = i + 1    
                    line = anifile.readline()
                self.file.write("MESHOBJ '%s' (\n  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n  PARAMS(\n  MATERIAL ( 'MISC_1_STPS')\n" % (Name))
                for frame in xrange(self.display.startFrame,
                                    self.display.endFrame + 1):
                    for f in frames:
                        if str(f) == ("f"+str(frame - self.display.startFrame)):
                           self.writeAnimFrame(frame,Name)
                           break
                anifile.seek(0,0)
                line = anifile.readline()
                while line!="":
                    self.file.write("    ")
                    self.file.write(line)
                    line = anifile.readline()
                self.writeTriangles(Name)
                self.file.write("  )\n)\n")
                anifile.close()
        else:
            self.writeFrame(self.display.currentFrame)
        self.writeEnd()

    def writeEnd(self):
        self.file.close()
        print "... finished"

    def writeAnimFrame(self, frame,name):
        Blender210.setCurrentFrame(frame)
        self.file.write("    FRAME 'f%s' (\n" % (frame - self.display.startFrame))
        self.writeObject(name)
        self.file.write("    )\n")

    def writeFrame(self, frame):
        Blender210.setCurrentFrame(frame)
        for name in self.scene.objects:
            if not Blender210.isCamera(name) and not Blender210.isLamp(name):
                self.file.write("MESHOBJ '%s' (\n  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n  PARAMS(\n  MATERIAL ( 'MISC_1_STPS')\n" % (name))
                self.file.write("    FRAME 'f%s' (\n" % (frame - self.display.startFrame))
                self.writeObject(name)
                self.file.write("    )\n    ACTION 'default' (F (f0,1000))\n")
                self.writeTriangles(name)
                self.file.write("  )\n)\n")

    def writeObject(self, name):
        if Blender210.isMesh(name):
            meshobj = Blender210.getObject(name)
            mesh    = Blender210.getMesh(meshobj.data)
            matrix  = meshobj.matrix

            for vertex in mesh.vertices:
                self.file.write("      V(%s,%s,%s:0,0)\n" %
                                (round((matrix[0][0]*vertex[0] + matrix[1][0]*vertex[1] + matrix[2][0]*vertex[2] + matrix[3][0]),6), round((matrix[0][1]*vertex[0] + matrix[1][1]*vertex[1] + matrix[2][1]*vertex[2] + matrix[3][1]),6),round((matrix[0][2]*vertex[0] + matrix[1][2]*vertex[1] + matrix[2][2]*vertex[2] + matrix[3][2]),6)))

    def writeTriangles(self, name):
        if Blender210.isMesh(name):
            meshobj = Blender210.getObject(name)
            mesh    = Blender210.getMesh(meshobj.data)
            faces   = mesh.faces
            for face in faces:
                 if face[4] != 0:
                     pass
                 elif face[2] == 0 and face[3] == 0:
                     print "can't export lines at the moment ..."
                 elif face[3] == 0:
                     self.file.write("    TRIANGLE (%s, %s, %s)\n" %
                                     (face[0], face[1], face[2]))
                 else:
                     self.file.write("    TRIANGLE (%s, %s, %s)\n"%
                                     (face[0], face[1], face[2]))
                     self.file.write("    TRIANGLE (%s, %s, %s)\n" %
                                     (face[0], face[2], face[3]))
  
csexport = CrystalSpaceExport("test.cs")
scene = Blender210.getCurrentScene()
csexport.export(scene)

