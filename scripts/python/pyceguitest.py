#!/usr/bin/env python
"""
Cs Cegui Python Example

A pure-Python script to show the use of Crystal Space and Cegui.

To use this, ensure that your PYTHONPATH, CRYSTAL, and LD_LIBRARY_PATH
(or DYLD_LIBRARY_PATH for MacOS/X; or PATH for Windows) variables are set
approrpriately, and then run the script with the command:

    python scripts/python/pyceguitest.py

This performs the similar features as the c++ ceguitest tutorial.

Note to be able to run this you'll need to 
"""

import sys, time, traceback
from cspace import *
try:
    # this is just a test to see if we have cegui support. note
    # from cspace import * already imports all symbols from this module.
    import cspace.pycscegui
except:
    print "cspace python bindings dont have cegui support"
    sys.exit(1)
try:
    import cegui
except:
    print "cegui python module not present!"
    sys.exit(1)
Cegui = None
def onQuit(args):
    print args.__dict__
    print "quit"
def onBar(args):
    print "changed textbox",args.Window.Name
def onSlider(args):
    btn1 = args.Window
    print btn1.Name,"value",btn1.CurrentValue

def InitCegui():
    global Cegui
    vfs = object_reg.Get(iVFS)
    Cegui = object_reg.Get(iCEGUI)
    if not Cegui:
        Report(CS_REPORTER_SEVERITY_ERROR, "Cegui plugin not present in registry!")
	return False
    # init cs cegui system
    Cegui.Initialize()
    winMgr = Cegui.WindowManager
    sysPtr = Cegui.System
    vfs.ChDir ("/ceguitest/0.5/")
    # load scheme
    Cegui.SchemeManager.loadScheme("ice.scheme")
    # set cursor
    sysPtr.DefaultMouseCursor = ("ice","MouseArrow")
    # load font
    cegui_font = Cegui.FontManager.createFont("FreeType","Vera", "/fonts/ttf/Vera.ttf")
    cegui_font.setProperty("PointSize", "10")
    cegui_font.load()
    # load layout
    sysPtr.GUISheet = winMgr.loadWindowLayout("ice.layout")
    # subscribe some events
    btn = winMgr.getWindow("Demo7/Window1/Quit")
    btn.subscribeEvent(cegui.PushButton.EventClicked,onQuit)
    btn = winMgr.getWindow("Demo7/Window1/Editbox")
    btn.subscribeEvent(cegui.Editbox.EventTextAccepted,onBar)
    btn.subscribeEvent(cegui.Editbox.EventTextChanged,onBar)
    btn = winMgr.getWindow("Demo7/Window1/Checkbox")
    btn.subscribeEvent(cegui.Checkbox.EventActivated,onBar)
    btn.subscribeEvent(cegui.Checkbox.EventEnabled,onBar)
    btn.subscribeEvent(cegui.Checkbox.EventMouseClick,onBar)
    btn = winMgr.getWindow("Demo7/Window3/MLEditbox")
    btn.subscribeEvent(cegui.MultiLineEditbox.EventTextChanged,onBar)
    btn = winMgr.getWindow("Demo7/Window1/Slider1")
    btn.subscribeEvent(cegui.Slider.EventValueChanged,onSlider)
    return True

def CreateRoom (matname):
    engine = object_reg.Get(iEngine)
    vc = object_reg.Get(iVirtualClock)
    loader = object_reg.Get(iLoader)
    matname = 'mystone'
    loader.LoadTexture (matname, "/lib/stdtex/bricks.jpg")
    room = engine.GetSectors().FindByName("room")
    material=engine.GetMaterialList().FindByName(matname)

    mapper = DensityTextureMapper(0.3)
    box = TesselatedBox(csVector3(-5, 0, -5), csVector3(5, 20, 5))
    box.SetLevel(3)
    box.SetMapper(mapper)
    box.SetFlags(Primitives.CS_PRIMBOX_INSIDE)
    walls = GeneralMeshBuilder.CreateFactoryAndMesh (engine, room, "walls", \
        "walls_factory", box)
    walls.GetMeshObject().SetMaterialWrapper(material)

def SetupFrame ():
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
    # render cegui interface
    Cegui.Render()

def FinishFrame ():
    myG3D.FinishDraw()
    myG3D.Print(None)

def HandleEvent (ev):
    if ((ev.Name  == KeyboardDown) and
        (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC)):
        q  = object_reg.Get(iEventQueue)
        if q:
            q.GetEventOutlet().Broadcast(csevQuit(object_reg))
            return 1
    return 0

def EventHandler (ev):
    if ev.Name == Frame:
        try:
            SetupFrame()
            FinishFrame()
        except:
            traceback.print_exc()
        return 1
    elif ev.Name == CommandLineHelp:
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

if not csInitializer.SetupConfigManager(object_reg):
    Report(CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!")
    sys.exit(1)

plugin_requests = [
    CS_REQUEST_VFS, CS_REQUEST_OPENGL3D, CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER, CS_REQUEST_IMAGELOADER, CS_REQUEST_LEVELLOADER,
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI)
]
if not csInitializer.RequestPlugins(object_reg, plugin_requests):
    Report(CS_REPORTER_SEVERITY_ERROR, "Plugin requests failed!")
    sys.exit(1)

if not csInitializer.SetupEventHandler(object_reg, EventHandler):
    Report(CS_REPORTER_SEVERITY_ERROR, "Could not initialize event handler!")
    sys.exit(1)
  
# Get some often used event IDs
KeyboardDown = csevKeyboardDown(object_reg)
Frame = csevFrame(object_reg)
CommandLineHelp = csevCommandLineHelp(object_reg)

if csCommandLineHelper.CheckHelp(object_reg):
    csCommandLineHelper.Help(object_reg)
    sys.exit(0)
 
vc = object_reg.Get(iVirtualClock)

engine = object_reg.Get(iEngine)
if not engine:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!")
    sys.exit(1)

myG3D = object_reg.Get(iGraphics3D)
if not myG3D:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D loader plugin!")
    sys.exit(1)

LevelLoader = object_reg.Get(iLoader)
if not LevelLoader:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!")
    sys.exit(1)

kbd = object_reg.Get(iKeyboardDriver)
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

# Create our world.
Report(CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...")
  
LevelLoader.LoadTexture("stone", "/lib/std/stone4.gif")
room = engine.CreateSector("room")
 
plugin_mgr = object_reg.Get(iPluginManager)
 
if not InitCegui():
    sys.exit(1)

CreateRoom('stone')

light = engine.CreateLight("", csVector3(0, 5, 0), 10, csColor(1, 0, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
room.GetLights().Add(light)

engine.Prepare()

SimpleStaticLighter.ShineLights(room, engine, 4)

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
