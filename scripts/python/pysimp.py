#!/usr/bin/env python2

import types, string, re

try:
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"
    import traceback
    traceback.print_exc()

print 'imported cspace'


# This is never used in here? - jamest
def CreateRoom(matname):
    print 'Start creating polygons from Python script...'
    engine = CS_QUERY_REGISTRY(object_reg, iEngine)
    room = engine.GetSectors().FindByName("room")
    walls = engine.CreateSectorWallsMesh(room,"walls")
    thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)
    material=engine.GetMaterialList().FindByName(matname)
    walls_state = thingstate.GetFactory()
    walls_state.AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5))
    walls_state.SetPolygonMaterial (CS_POLYRANGE_LAST, material);
    walls_state.SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);
                                        
    print 'Finished!'

