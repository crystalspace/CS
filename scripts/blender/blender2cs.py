import Blender
import math
import string

TAB="    "

def usage():
    print ("Load this program in the text editor within Blender and " +
           "start it with ALT-P.")

def start_library(filename):
    file = open(filename, "w")
    file.write("LIBRARY (\n")
    file.close()

def finish_library(filename):
    file = open(filename, "a")
    file.write(")\n")
    file.close()

def write_factory(filename, factmesh):
    file = open(filename, "a")
    file.write("    MESHFACT '%s' (\n" % factmesh.name)
    file.write("        PLUGIN ('crystalspace.mesh.loader.factory.thing')\n")
    file.write("        PARAMS (\n")
    file.write("            MATERIAL ('replace_me')\n")
    for v in factmesh.verts:
        file.write("            V (%3.4f,%3.4f,%3.4f" % (v.co[0], v.co[2], v.co[1]))
        if factmesh.has_uvco:
            file.write(":%3.4f,%3.4f" % (v.uvco[0], v.uvco[1]))
        file.write(")\n")
    for face in factmesh.faces:
        num = len(face.v)
        if num < 3:
            continue
        file.write("            POLYGON (\n")
        file.write("                V (%d,%d,%d)\n" % (face.v[0].index, face.v[2].index, face.v[1].index))
        if factmesh.hasFaceUV:
            if len(face.uv) > 0:
                file.write("                TEXTURE (\n")
                file.write("                    UV (0, %3.4f, %3.4f, 1, %3.4f, %3.4f, 2, %3.4f, %3.4f)\n" % (face.uv[0][0], 1.0 - face.uv[0][1], face.uv[2][0], 1.0 - face.uv[2][1], face.uv[1][0], 1.0 - face.uv[1][1]))
                file.write("                )\n")
        if num == 4:
            file.write("            )\n            POLYGON (\n")
            file.write("                V (%d,%d,%d)\n" % (face.v[0].index, face.v[3].index, face.v[2].index))
            if factmesh.hasFaceUV:
                if len(face.uv) > 0:
                    file.write("                TEXTURE (\n")
                    file.write("                    UV (0, %3.4f, %3.4f, 1, %3.4f, %3.4f, 2, %3.4f, %3.4f)\n" % (face.uv[0][0], 1.0 - face.uv[0][1], face.uv[3][0], 1.0 - face.uv[3][1], face.uv[2][0], 1.0 - face.uv[2][1]))
                    file.write("                )\n")
        file.write("            )\n")
    file.write("        )\n")
    file.write("    )\n")
    file.close()

if Blender.bylink:
    usage()
else:
    meshlist = {}
    filename = "exported.cs"
    file = open(filename, "w")

    file.write("WORLD (\n")
    camera = Blender.Object.Get("Camera")

    file.write("    TEXTURES (\n")
    file.write("        TEXTURE 'blue' (FILE (/lib/stdtex/blue.jpg))\n")
    file.write("        TEXTURE 'white' (FILE (/lib/stdtex/white.jpg))\n")
    file.write("    )\n")
    file.write("    MATERIALS (\n")
    file.write("        MATERIAL 'blue' (TEXTURE ('blue'))\n")
    file.write("        MATERIAL 'white' (TEXTURE ('white'))\n")
    file.write("    )\n")
    if camera:
        file.write("    START (\n")
        file.write("        SECTOR ('room')\n")
        file.write("        POSITION (%3.4f,%3.4f,%3.4f)\n" % (camera.loc.x, camera.loc.z, -camera.loc.y))
        file.write("        UP (0,0,1)\n")
        file.write("        FORWARD (0,1,0)\n")
        file.write("    )\n")
    objs = Blender.Object.Get()
    for obj in objs:
        if obj.data == None:
            continue
        name = obj.data.name
        if name == "Ball":
            file.write("    MESHFACT 'ballfact' (\n")
            file.write("        PLUGIN('crystalspace.mesh.loader.factory.ball')\n")
            file.write("        PARAMS()\n")
            file.write("    )\n")
            continue
        if meshlist.has_key(name):
            continue
        mesh = None
        if string.find(name,".") >= 0:
            fields = string.split(name, ".", 1);
            foundname = fields[0]
            mesh = Blender.NMesh.GetRaw(foundname)
            if mesh:
                name = foundname
        if mesh == None:
            mesh = Blender.NMesh.GetRaw(name)
        if mesh:
            meshlist[name] = 1
            factfile = name
            factfile = string.replace(factfile, "-", "_")    
            factfile = string.replace(factfile, ".", "_")
            factfile += ".cs"
            start_library(factfile)
            write_factory(factfile, mesh)
            for i in range(100):
                newname = "%s.%03d" % (name, i)
                newmesh = None
                newmesh = Blender.NMesh.GetRaw(newname)
                if newmesh:
                    meshlist[newname] = 1
                    write_factory(factfile, newmesh)
            finish_library(factfile)
            file.write("    LIBRARY '%s' ('%s')\n" % (factfile, factfile))

    file.write("    SECTOR 'room' (\n")
    file.write("        LIGHT  (CENTER (0,13,0) RADIUS (60) COLOR  (1,1,1) HALO    (0,0,0) ATTENUATION (none) )\n")

    mesh = Blender.NMesh.GetRaw("walls")
    if mesh:
        file.write("        MESHOBJ 'walls' (\n")
        file.write("            PLUGIN ('crystalspace.mesh.loader.thing')\n")
        file.write("            PARAMS (\n")
        file.write("                FACTORY ('walls')\n")
        file.write("            )\n")
        file.write("            ZFILL ()\n")
        file.write("        )\n")

    objs = Blender.Object.Get()
    for obj in objs:
        if obj.data == None:
            continue
        name = obj.name
        if name == "walls":
            continue
        if name == "Ball":
            file.write("        MESHOBJ 'Ball' (\n")
            file.write("            PLUGIN ('crystalspace.mesh.loader.ball')\n")
            file.write("            PARAMS(\n")
            file.write("                FACTORY('ballfact')\n")
            file.write("                MATERIAL('yellow')\n")
            file.write("                NUMRIM (16)\n")
            file.write("                RADIUS (0.5, 0.5, 0.5)\n")
            file.write("                SHIFT (0, 0, 0)\n")
            file.write("            )\n")
            file.write("            ZTEST ()\n")
            file.write("            MOVE (V (%3.4f,%3.4f,%3.4f))\n" % (obj.loc.x, obj.loc.z, obj.loc.y))
            file.write("        )\n")
            continue
        meshname = obj.data.name
        mesh = Blender.NMesh.GetRaw(meshname)
        if mesh == None:
            continue
        file.write("        MESHOBJ '%s' (\n" % name)
        file.write("            PLUGIN ('crystalspace.mesh.loader.thing')\n")
        file.write("            PARAMS (\n")
        file.write("                FACTORY ('%s')\n" % meshname)
        file.write("                MOVEABLE ()\n")
        file.write("            )\n")
        file.write("            MOVE (")
        if (obj.rot.x + obj.rot.y + obj.rot.z) != 0.0:
            file.write("MATRIX(ROT_X(%3.4f) ROT_Y(%3.4f) ROT_Z(%3.4f)) " % (obj.rot.x, obj.rot.z, obj.rot.y))
        file.write("V (%3.4f,%3.4f,%3.4f))\n" % (obj.loc.x, obj.loc.z, obj.loc.y))
        file.write("        )\n")

    file.write("    )\n")
    file.write(")\n")
    file.close()
