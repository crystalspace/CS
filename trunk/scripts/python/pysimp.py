# This is an adjunct of the "pysimp" demonstration program. "pysimp" loads this
# file via iScript::LoadModule("pysimp"). This file, in turn, defines the
# relatively simple pure-Python function CreateRoom(). Later, after the
# "pysimp" program has performed additional initialization and has set up a 3D
# sector, it then invokes the CreateRoom() on the Python side, allowing the
# Python code to create the actual room within the already-prepared sector.
# This demonstrates how C++ code can defer certain operations to scripting
# code. The room created by this script is like that created by the C++
# tutorial program `simple1'.

import types, string, re

try:
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"
    import traceback
    traceback.print_exc()

print 'imported cspace'

def CreateRoom(matname):
    print 'Start creating polygons from Python script...'
    engine = object_reg.Get(iEngine)
    room = engine.GetSectors().FindByName("room")
    
    mapper = DensityTextureMapper (0.3)
    box = TesselatedBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5))
    box.SetLevel (3)
    box.SetMapper (mapper)
    box.SetFlags (Primitives.CS_PRIMBOX_INSIDE)

    walls = GeneralMeshBuilder.CreateFactoryAndMesh (engine, room, 
      "walls", "walls_factory", box)
    walls_state = walls.GetMeshObject ().QueryInterface(iGeneralMeshState)
    walls_state.SetShadowReceiving(True)
    walls_factory = walls.GetMeshObject().GetFactory()
    material = engine.GetMaterialList().FindByName(matname)
    walls.GetMeshObject().SetMaterialWrapper(material)
    print 'Finished!'
