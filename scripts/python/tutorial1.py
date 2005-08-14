#!/usr/bin/env python
"""
CsPython Tutorial Example 1

A pure-Python script to show the use of Crystal Space.

To use this, ensure that your PYTHONPATH, CRYSTAL, and LD_LIBRARY_PATH
(or DYLD_LIBRARY_PATH for MacOS/X; or PATH for Windows) variables are set
approrpriately, and then run the script with the command:

    python scripts/python/tutorial1.py

This performs the same features as the C++ tutorial1.
It creates a room and allows movement with the arrow keys, page-up, and
page-down.

===========================================================================
There are two ways to use the CsPython module.
Either as a plugin within CS (pysimp), 
or as a pure Python module (this example).

This is performs a function similar to the CS tutorial 1, rewritten in Python.
Please refer to the CS Tutorial 1 in the documentation
for detail on how the C++ version works.
"""

import sys, time, traceback
from cspace import *

DEBUG = 0

def CreateRoom (matname):
    if DEBUG: print 'Start creating polygons from Python script...'
    if DEBUG: print 'object_reg=',object_reg
    if DEBUG: print 'dir(object_reg)=',dir(object_reg)
    engine = CS_QUERY_REGISTRY(object_reg, iEngine)
    if DEBUG: print 'engine=',engine
    vc = CS_QUERY_REGISTRY(object_reg, iVirtualClock)
    if DEBUG: print 'vc=',vc
    loader = CS_QUERY_REGISTRY(object_reg, iLoader)
    if DEBUG: print 'loader=',loader
    matname = 'mystone'
    loader.LoadTexture (matname, "/lib/stdtex/bricks.jpg")
    room = engine.GetSectors().FindByName("room")
    walls = engine.CreateSectorWallsMesh(room, "walls")
    if DEBUG: print 'walls=',walls
    material=engine.GetMaterialList().FindByName(matname)
    thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)
    walls_state = thingstate.GetFactory()
    walls_state.AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5))
    walls_state.SetPolygonMaterial (CS_POLYRANGE_LAST, material);
    walls_state.SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);
    if DEBUG: print 'Finished!'

def SetupFrame ():
    if DEBUG: print 'SetupFrame called'
    elapsed_time = vc.GetElapsedTicks()
    current_time = vc.GetCurrentTicks()
    # Now rotate the camera according to keyboard state
    speed = (elapsed_time / 1000.) * (0.03 * 20);
    if kbd.GetKeyState(CSKEY_RIGHT):
        view.GetCamera().GetTransform().RotateThis(CS_VEC_ROT_RIGHT, speed)
    if kbd.GetKeyState(CSKEY_LEFT):
        view.GetCamera().GetTransform().RotateThis(CS_VEC_ROT_LEFT, speed)
    if kbd.GetKeyState(CSKEY_PGUP):
        view.GetCamera().GetTransform().RotateThis(CS_VEC_TILT_UP, speed)
    if kbd.GetKeyState(CSKEY_PGDN):
        view.GetCamera().GetTransform().RotateThis(CS_VEC_TILT_DOWN, speed)
    if kbd.GetKeyState(CSKEY_UP):
        view.GetCamera().Move(CS_VEC_FORWARD * 4 * speed)
    if kbd.GetKeyState(CSKEY_DOWN):
        view.GetCamera().Move(CS_VEC_BACKWARD * 4 * speed)
    # Tell 3D driver we're going to display 3D things.
    if not myG3D.BeginDraw(engine.GetBeginDrawFlags() | CSDRAW_3DGRAPHICS):
        sys.exit(1)
    if view:
        view.Draw()
    if DEBUG: print 'SetupFrame done'

def FinishFrame ():
    if DEBUG: print 'FinishFrame called'
    myG3D.FinishDraw()
    myG3D.Print(None)
    if DEBUG: print 'FinishFrame done'

def HandleEvent (ev):
    if DEBUG: print 'HandleEvent called'
    if ((ev.Type  == csevKeyboard ) and
        (csKeyEventHelper.GetEventType(ev) == csKeyEventTypeDown) and
        (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC)):
        
        q  = CS_QUERY_REGISTRY(object_reg, iEventQueue)
        if q:
            q.GetEventOutlet().Broadcast(cscmdQuit)
            return 1
    return 0

def EventHandler (ev):
    if DEBUG: print 'EventHandler called'
    if DEBUG: print '   ev=%s' % ev
    if ev.Type == csevBroadcast and csCommandEventHelper.GetCode(ev) == cscmdProcess:
        try:
            SetupFrame()
        except:
            traceback.print_exc()
        return 1
    elif ev.Type == csevBroadcast and csCommandEventHelper.GetCode(ev) == cscmdFinalProcess:
        try:
            FinishFrame()
        except:
            traceback.print_exc()
        return 1
    elif ev.Type == csevBroadcast and csCommandEventHelper.GetCode(ev) == cscmdCommandLineHelp:
        print 'No help today...'
        return 1
    else:
        try:
            return HandleEvent(ev)
        except:
            traceback.print_exc()
    return 0

object_reg = csInitializer.CreateEnvironment(sys.argv)

def Report (severity, msg):
    csReport(object_reg, severity, "crystalspace.application.python", msg)

if DEBUG: print 'Initializing application...'
if not csInitializer.SetupConfigManager(object_reg):
    Report(CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!")
    sys.exit(1)

plugin_requests = [
    CS_REQUEST_VFS, CS_REQUEST_SOFTWARE3D, CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER, CS_REQUEST_IMAGELOADER, CS_REQUEST_LEVELLOADER,
]
if DEBUG: print 'Requesting plugins...'
if not csInitializer.RequestPlugins(object_reg, plugin_requests):
    Report(CS_REPORTER_SEVERITY_ERROR, "Plugin requests failed!")
    sys.exit(1)

if DEBUG: print 'Setting up event handler...'
if not csInitializer.SetupEventHandler(object_reg, EventHandler):
    Report(CS_REPORTER_SEVERITY_ERROR, "Could not initialize event handler!")
    sys.exit(1)
  
if DEBUG: print 'Checking if help is needed...'
if csCommandLineHelper.CheckHelp(object_reg):
    csCommandLineHelper.Help(object_reg)
    sys.exit(0)
 
if DEBUG: print 'Getting virtual clock...'
vc = CS_QUERY_REGISTRY(object_reg, iVirtualClock)

if DEBUG: print 'Getting engine...'
engine = CS_QUERY_REGISTRY(object_reg, iEngine)
if not engine:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!")
    sys.exit(1)

if DEBUG: print 'Getting 3D graphics...'
myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D)
if not myG3D:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D loader plugin!")
    sys.exit(1)

LevelLoader = CS_QUERY_REGISTRY(object_reg, iLoader)
if not LevelLoader:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!")
    sys.exit(1)

kbd = CS_QUERY_REGISTRY(object_reg, iKeyboardDriver)
if not kbd:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!")
    sys.exit(1)

# Open the main system. This will open all the previously loaded plug-ins.
nw = myG3D.GetDriver2D().GetNativeWindow()
if nw:
    nw.SetTitle("Simple Crystal Space Python Application")
if not csInitializer.OpenApplication (object_reg):
    Report(CS_REPORTER_SEVERITY_ERROR, "Error opening system!")
    Cleanup()
    sys.exit(1)

# Some commercials...
Report(
    CS_REPORTER_SEVERITY_NOTIFY,
    "Simple Crystal Space Python Application version 0.1."
)
txtmgr = myG3D.GetTextureManager()

# First disable the lighting cache. Our app is simple enough not to need this.
engine.SetLightingCacheMode(0)

# Create our world.
Report(CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...")
  
LevelLoader.LoadTexture("stone", "/lib/std/stone4.gif")
room = engine.CreateSector("room")
 
plugin_mgr = CS_QUERY_REGISTRY(object_reg, iPluginManager)
 
if 0:
    Report(CS_REPORTER_SEVERITY_NOTIFY, "Loading script.python plugin...")
    # Initialize the python plugin.
    script = CS_LOAD_PLUGIN(
        plugin_mgr, "crystalspace.script.python", iScript
    )
    if script:
        Report(CS_REPORTER_SEVERITY_NOTIFY, "Loading pysimp module...")
        # Load a python module (scripts/python/pysimp.py).
        if not script.LoadModule("pysimp"):
            sys.exit(1)

        # Set up our room.
        # Execute one method defined in pysimp.py
        # This will create the polygons in the room.
        Report (CS_REPORTER_SEVERITY_NOTIFY, "calling pysimp.CreateRoom...")
        if script.RunText ("pysimp.CreateRoom('stone')"):
            sys.exit(1)
else:
    CreateRoom('stone')

light = engine.CreateLight("", csVector3(0, 5, 0), 10, csColor(1, 0, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
if DEBUG: print 'light=',light
room.GetLights().Add(light)

if DEBUG: print 'calling engine.Prepare()'

engine.Prepare()

Report(CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------")

# csView is a view encapsulating both a camera and a clipper.
# You don't have to use csView as you can do the same by
# manually creating a camera and a clipper but it makes things a little
# easier.
view = csView(engine, myG3D)
view.GetCamera().SetSector(room)
view.GetCamera().GetTransform().SetOrigin(csVector3(0, 2, 0))
g2d = myG3D.GetDriver2D()
view.SetRectangle(2, 2, g2d.GetWidth() - 4, g2d.GetHeight() - 4)

csDefaultRunLoop(object_reg)
