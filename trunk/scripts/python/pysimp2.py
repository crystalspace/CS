"""
CsPython PySimp Example 2
By Mark Gossage (mark@gossage.cjb.net)

A script to show the use of CsPython from within a plugin.

This is an adjunct script to the "pysimp" demonstration program.
To use this, just run the pysimp program which the name of this script
(sans the .py extension) as its sole argument. For example:

    pysimp pysimp2

This script demonstrates the same features as the C++ tutorial2.
It creates a room and a 3D sprite.

===========================================================================
There are two ways to use the CsPython module.
Either as a plugin within CS (this example), 
or as a pure Python module (see Tutorial2.py).

As a CS plugin, the python code is loaded by the C++ app (pysimp.cpp)
This example only has one function CreateRoom()
However it is simple to add more functions & add the C++ code to call them.

There is no event handling in this code, merely the creation of the 3D objects.
Instead, the event handling is done by the C++ code.  However, as a plugin,
events could be incorperated in one of two ways: By creating functions such as
SetupFrame(), FinishFrame() and then calling them from the C++ code; or, by
using csInitializer.SetupEventHandler(...) to create an event handler within
Python (believe it or not, it is possible to have 2 event handlers, one in
Python & one in C++). See tutorial0.py as an example of how it can be done.

Most of this code looks very much like the original C++ code; any changes will
be marked.

Should you have any errors when modifying this code, you will see the exception
message on Python's sys.stderr (unless you ran pysimp with the
--python-enable-reporter option). You can instruct the CsPython plugin to run
the Python debugger upon an exception by launching pysimp with the
--python-enable-debugger option). To leave the debugger, type 'quit'
"""

import types, string, re

try:
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"
    import traceback
    traceback.print_exc()

def CreateRoom(matname):
    """This function is the only one called by the C++ code in pysimp.
    We will do everything here.
    """
    print 'Start creating polygons from Python script...'
    
    CreateTheRoom(matname)  # the normal stuff
    LoadSprite()            # new stuff
   
    print 'Finished!'

def CreateTheRoom(matname):
    """This is derived from pysimp.py code"""

    # Note: When using CsPython as a plugin,
    # the variable object_reg (cspace.object_reg actually)
    # is defined as a pointer/reference to the C++ iObjectRegistry
    # with this we can do just about everything.
    
    engine = object_reg.Get(iEngine)

    room = engine.GetSectors().FindByName("room")
    mapper = DensityTextureMapper(0.3)
    box = TesselatedBox(csVector3(-5, 0, -5), csVector3(5, 20, 5))
    box.SetLevel(3)
    box.SetMapper(mapper)
    box.SetFlags(Primitives.CS_PRIMBOX_INSIDE)
    walls = GeneralMeshBuilder.CreateFactoryAndMesh (engine, room, \
        "walls", "walls_factory", box)
    material = engine.GetMaterialList().FindByName(matname)
    walls.GetMeshObject().SetMaterialWrapper(material)

    # Add some lights, otherwise it gets really dark, here
    ll = room.GetLights()
    light = engine.CreateLight("", csVector3(-3, 5, 0), 10, csColor(1, 0, 0), \
        CS_LIGHT_DYNAMICTYPE_STATIC)
    ll.Add(light)
    light = engine.CreateLight("", csVector3(3, 5,  0), 10, csColor(0, 0, 1), \
        CS_LIGHT_DYNAMICTYPE_STATIC)
    ll.Add (light)
    engine.Prepare()
    SimpleStaticLighter.ShineLights(room, engine, 22)

def FatalError(msg="FatalError"):
    "A 'Panic & die' routine"
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.application.python", msg)
    sys.exit(1)

def LoadSprite():            # new stuff
    """Create up a Sprite3d,
    To do this we will have to create a iLoader & use it to load stuff
    """
    # get a few plugins
    engine = object_reg.Get(iEngine)
    g3d = object_reg.Get(iGraphics3D)
    loader = object_reg.Get(iLoader)

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
    m=m/5           # python doesnt support *= operator
    sprite.GetMovable().SetTransform(m)
    sprite.GetMovable().UpdateMove()
    spstate=sprite.GetMeshObject().QueryInterface(iSprite3DState)
    spstate.SetAction("default")
    spstate.SetMixMode(CS_FX_SETALPHA_INT (100))

    # The following two calls are not needed since CS_ZBUF_USE and
    # Object render priority are the default but they show how you
    # can do this.
    sprite.SetZBufMode(CS_ZBUF_USE)
    sprite.SetRenderPriority(engine.GetObjectRenderPriority())
