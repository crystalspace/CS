"""
CsPython PySimp Example 2
By Mark Gossage (mark@gossage.cjb.net)

A script to show the use of CsPython from within a plugin.

To use this, either modify the Pysimp app, changing the python calls
from "pysimp" to "pysimp2"
Or, just copy this file over Pysimp.py
Then execute:
    pysimp -python
This performs the same features at tutorial2.
This creates a room & a semi-transparent 3d sprite

===========================================================================
There are two ways to use the CsPython module.
Either as a plugin within CS (this example), 
or as a python module (see Tutorial2.py).

As a CS plugin, the python code is loaded by the C++ app (pysimp.cpp)
This example only has one function CreateRoom()
However it is simple to add more functions & add the C++ code to call them.

There is no event handling in this code, merely the creation of the 3d objects
However as a plugin, events could be incorperated in one of two ways:
By creating fns such as SetupFrame(), FinishFrame() and then calling them
from the C++ code
Or, by using csInitializer.SetupEventHandler(...) to create an 
event handler within python
(believe it or not, it is possible to have 2 event handlers, on in python
& one in C++)
See tutorial0.py as an example of how it could be done.

Most of this code looks very much like the original C++ code, 
any changes will be marked.

Should you have any errors when modifying this code, 
the python debugger pdb will be activated
To leave the debugger, type 'quit'
"""

import types, string, re

try:
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"
    import traceback
    traceback.print_exc()

def CreateRoom(matname):
    """This fn is the only fn called by the C++ code in pysimp
    We will do everything here
    """
    print 'Start creating polygons from Python script...'
    
    CreateTheRoom(matname)  # the normal stuff
    LoadSprite()            # new stuff
   
    print 'Finished!'

def CreateTheRoom(matname):
    """This is just the original code from pysimp.py"""

    # note: when using CsPython as a plugin
    # the variable object_reg (cspace.object_reg actually)
    # is defined as a pointer/reference to the C++ iObjectRegistry
    # with this we can do just about everything
    
    # as you can see, CS_QUERY_REGISTRY() works
    # as does SCF_QUERY_INTERFACE()
    engine = CS_QUERY_REGISTRY(object_reg, iEngine)

    room = engine.GetSectors().FindByName("room")
    walls = engine.CreateSectorWallsMesh(room,"walls")
    thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)

    material=engine.GetMaterialList().FindByName(matname)
    walls_state = thingstate.GetFactory()
    walls_state.AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5))
    walls_state.SetPolygonMaterial (CS_POLYRANGE_LAST, material);
    walls_state.SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

    # note: it is no longer neccesary to DecRef & IncRef objects
    # this it automatically handled by python

def FatalError(msg="FatalError"):
    "A 'Panic & die' routine"
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.application.python", msg)
    sys.exit(1)

def LoadSprite():            # new stuff
    """Create up a Sprite3d,
    To do this we will have to create a iLoader & use it to load stuff
    """
    # get a few plugins
    engine = CS_QUERY_REGISTRY(object_reg, iEngine)
    g3d = CS_QUERY_REGISTRY(object_reg, iGraphics3D)
    loader = CS_QUERY_REGISTRY(object_reg, iLoader)

    # The C++ NULL is mapped to the python None
    # to play it safe we should check for our return values
    if engine is None or g3d is None or loader is None:
        FatalError("Error getting plugins!")

    # Load a texture for our sprite.
    txtmgr=g3d.GetTextureManager()
    txt=loader.LoadTexture ("spark","/lib/std/spark.png",CS_TEXTURE_3D,txtmgr,1)
    if txt == None:
        FatalError("Error loading texture!")

    # Load a sprite template from disk.
    imeshfact=loader.LoadMeshObjectFactory("/lib/std/sprite1")
    if imeshfact == None:
        FatalError("Error loading mesh!")

    # Create the sprite and add it to the engine.
    room = engine.GetSectors().FindByName("room")
    sprite=engine.CreateMeshWrapper(imeshfact,"MySprite",room,csVector3 (-1, 2, 3))
    m=csMatrix3()
    m.Identity()    # make sure its identity
    m=m*5           # python doesnt support *= operator
    sprite.GetMovable().SetTransform(m)
    sprite.GetMovable().UpdateMove()
    spstate=SCF_QUERY_INTERFACE(sprite.GetMeshObject(),iSprite3DState)
    spstate.SetAction("default")
    spstate.SetMixMode(CS_FX_SETALPHA (.5))

    # The following two calls are not needed since CS_ZBUF_USE and
    # Object render priority are the default but they show how you
    # can do this.
    sprite.SetZBufMode(CS_ZBUF_USE)
    sprite.SetRenderPriority(engine.GetObjectRenderPriority())

    sprite.DeferUpdateLighting(CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10)
