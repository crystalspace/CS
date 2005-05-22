# This is a pure-Python program which demonstrates simple collision detection,
# just as does the C++ tutorial program "simpcd". Run this script by invoking
# "python pysimpcd.py" after setting PYTYHONPATH, CRYSTAL, LD_LIBRARY_PATH (or
# DLYD_LIBRARY_PATH for MacOS/X; or PATH for Windows) appropriately.

import sys, time, traceback, math
from cspace import *

def CreateRoom (matname):
    engine = CS_QUERY_REGISTRY(object_reg, iEngine)
    room = engine.GetSectors().FindByName("room")
    walls = engine.CreateSectorWallsMesh(room, "walls")
    thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)
    material = engine.GetMaterialList().FindByName(matname)
    thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)
    walls_state = thingstate.GetFactory()
    walls_state.AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5))
    walls_state.SetPolygonMaterial (CS_POLYRANGE_LAST, material);
    walls_state.SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);


rot1_direction = 1.
rot2_direction = -1.

def SetupFrame ():
    global rot1_direction
    global rot2_direction
    elapsed_time = vc.GetElapsedTicks()
    speed = (elapsed_time / 1000.0) * (0.03 * 20)
    rotmat = csZRotMatrix3(speed / 5)
    parent_sprite.GetMovable().Transform(rotmat)
    parent_sprite.GetMovable().UpdateMove()
    rotmat1 = csZRotMatrix3(rot1_direction * speed / 2.5)
    old_trans1 = sprite1.GetMovable().GetTransform()
    sprite1.GetMovable().Transform(rotmat1)
    sprite1.GetMovable().UpdateMove()
    rotmat2 = csZRotMatrix3(rot2_direction * speed / 1.0)
    old_trans2 = sprite2.GetMovable().GetTransform()
    sprite2.GetMovable().Transform(rotmat2)
    sprite2.GetMovable().UpdateMove()
    cdsys.ResetCollisionPairs()
    cdsys.SetOneHitOnly(1)
    ft1 = sprite1.GetMovable().GetFullTransform()
    ft2 = sprite2.GetMovable().GetFullTransform()
    cd = cdsys.Collide(sprite1_col, ft1, sprite2_col, ft2)
    if cd:
        if snd and boom:
            sndsrc = boom.CreateSource(SOUND3D_ABSOLUTE)
            if sndsrc:
                sndsrc.Play()
        sprite1.GetMovable().SetTransform(old_trans1)
        sprite1.GetMovable().UpdateMove()
        sprite2.GetMovable().SetTransform(old_trans2)
        sprite2.GetMovable().UpdateMove()
        rot1_direction = -rot1_direction
        rot2_direction = -rot2_direction

    # Now rotate the camera according to keyboard state
    c = view.GetCamera()
    if kbd.GetKeyState(CSKEY_RIGHT):
        c.GetTransform().RotateThis(CS_VEC_ROT_RIGHT, speed)
    if kbd.GetKeyState(CSKEY_LEFT):
        c.GetTransform().RotateThis(CS_VEC_ROT_LEFT, speed)
    if kbd.GetKeyState(CSKEY_PGUP):
        c.GetTransform().RotateThis(CS_VEC_TILT_UP, speed)
    if kbd.GetKeyState(CSKEY_PGDN):
        c.GetTransform().RotateThis(CS_VEC_TILT_DOWN, speed)
    if kbd.GetKeyState(CSKEY_UP):
        c.Move(CS_VEC_FORWARD * 4 * speed)
    if kbd.GetKeyState(CSKEY_DOWN):
        c.Move(CS_VEC_BACKWARD * 4 * speed)
    # Tell 3D driver we're going to display 3D things.
    if not myG3D.BeginDraw(engine.GetBeginDrawFlags() | CSDRAW_3DGRAPHICS):
        sys.exit(1)
    if view:
        view.Draw()

def FinishFrame ():
    myG3D.FinishDraw()
    myG3D.Print(None)

def HandleEvent (ev):
    if ((ev.Type  == csevKeyboard ) and
        (csKeyEventHelper.GetEventType(ev) == csKeyEventTypeDown) and
        (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC)):
        
        q  = CS_QUERY_REGISTRY(object_reg, iEventQueue)
        if q:
            q.GetEventOutlet().Broadcast(cscmdQuit)
            return 1
    return 0

def EventHandler (ev):
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

def InitCollider (mesh):
    polmesh = SCF_QUERY_INTERFACE(mesh.GetMeshObject(), iPolygonMesh)
    if polmesh:
        wrap = csColliderWrapper(mesh.QueryObject(), cdsys, polmesh)
        if 1: # Not needed in C++, but necessary here... strange... TODO
            wrap.GetCollider().IncRef()
        return wrap.GetCollider()
    else:
        csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
            "crystalspace.application.pysimpcd",
            "Object doesn't support collision detection!")
        return None

def Report (severity, msg):
    csReport(object_reg, severity, "crystalspace.application.python", msg)

object_reg = csInitializer.CreateEnvironment(sys.argv)

if not csInitializer.SetupConfigManager(object_reg):
    Report(CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!")
    sys.exit(1)

plugin_requests = [
    CS_REQUEST_VFS,
    CS_REQUEST_SOFTWARE3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_PLUGIN("crystalspace.sound.loader.wav", iSoundLoader),
    CS_REQUEST_PLUGIN("crystalspace.sound.driver.oss", iSoundDriver),
    CS_REQUEST_PLUGIN("crystalspace.sound.render.software", iSoundRender),
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER, 
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN("crystalspace.collisiondetection.opcode", iCollideSystem),
]
if not csInitializer.RequestPlugins(object_reg, plugin_requests):
    Report(CS_REPORTER_SEVERITY_ERROR, "Plugin requests failed!")
    sys.exit(1)

if not csInitializer.SetupEventHandler(object_reg, EventHandler):
    Report(CS_REPORTER_SEVERITY_ERROR, "Could not initialize event handler!")
    sys.exit(1)
  
if csCommandLineHelper.CheckHelp(object_reg):
    csCommandLineHelper.Help(object_reg)
    sys.exit(0)
  
cdsys = CS_QUERY_REGISTRY(object_reg, iCollideSystem);
if not cdsys:
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.pysimpcd",
        "Can't find the collision detection system!");
    sys.exit(1)
 
vc = CS_QUERY_REGISTRY(object_reg, iVirtualClock)
if not vc:
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.pysimpcd",
        "Can't find the virtual clock!");
    sys.exit(1)

engine = CS_QUERY_REGISTRY(object_reg, iEngine)
if not engine:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!")
    sys.exit(1)

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

snd = CS_QUERY_REGISTRY(object_reg, iSoundRender)
if not snd:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iSoundRender!")

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
CreateRoom('stone')

ll = room.GetLights()
light = engine.CreateLight("", csVector3(-3, 5, 0), 10, csColor(1, 0, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
ll.Add(light)
light = engine.CreateLight("", csVector3(3, 5, 0), 10, csColor(0, 0, 1), CS_LIGHT_DYNAMICTYPE_STATIC)
ll.Add(light)
light = engine.CreateLight("", csVector3(0, 5, -3), 10, csColor(0, 1, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
ll.Add(light)

engine.Prepare()

Report(CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------")

# csView is a view encapsulating both a camera and a clipper.
# You don't have to use csView as you can do the same by
# manually creating a camera and a clipper but it makes things a little
# easier.
view = csView(engine, myG3D)
view.GetCamera().SetSector(room)
view.GetCamera().GetTransform().SetOrigin(csVector3(0, 5, -6))
g2d = myG3D.GetDriver2D()
view.SetRectangle(0, 0, g2d.GetWidth(), g2d.GetHeight())

loader = CS_QUERY_REGISTRY(object_reg, iLoader)
txt = loader.LoadTexture(
    'spark', '/lib/std/spark.png', CS_TEXTURE_3D, txtmgr, 1
)
if not txt:
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.pysimpcd", "Error loading texture!"
    )
    sys.exit(1)

imeshfact = loader.LoadMeshObjectFactory("/lib/std/sprite2")
if not imeshfact:
    csReport(
        object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.pysimpcd",
        "Error loading mesh object factory!"
    )
    sys.exit(1)

# First create the parent sprite.
parent_sprite = engine.CreateMeshWrapper(
    imeshfact, "Parent", room, csVector3(0, 5, 3.5)
)
spstate = SCF_QUERY_INTERFACE(parent_sprite.GetMeshObject(), iSprite3DState)
spstate.SetAction("default")
parent_sprite.GetMovable().Transform(csZRotMatrix3(math.pi/2.))
parent_sprite.GetMovable().UpdateMove()

# Now create the first child.
sprite1 = engine.CreateMeshWrapper(imeshfact, "Rotater1")
sprite1.GetMovable().SetPosition(csVector3(0, -.5, -.5))
sprite1.GetMovable().Transform(csZRotMatrix3(math.pi/2.))
sprite1.GetMovable().UpdateMove() 
spstate = SCF_QUERY_INTERFACE(sprite1.GetMeshObject(), iSprite3DState)
spstate.SetAction("default")
parent_sprite.GetChildren().Add(sprite1)

# Now create the second child.
sprite2 = engine.CreateMeshWrapper(imeshfact, "Rotater2")
sprite2.GetMovable().SetPosition(csVector3(0, .5, -.5))
sprite2.GetMovable().Transform(csZRotMatrix3(math.pi/2.))
sprite2.GetMovable().UpdateMove()
spstate = SCF_QUERY_INTERFACE(sprite2.GetMeshObject(), iSprite3DState)
spstate.SetAction("default")
parent_sprite.GetChildren().Add(sprite2)
    
# We only do collision detection for the rotating children
# so that's the only colliders we have to create.
sprite1_col = InitCollider(sprite1)
if not sprite1_col:
    sys.exit(1)
sprite2_col = InitCollider(sprite2)
if not sprite2_col:
    sys.exit(1)

if snd:
    w = LevelLoader.LoadSound('boom', '/lib/std/whoosh.wav')
    if w:
        boom = w.GetSound()
    else:
        csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
            "crystalspace.application.pysimpcd", "Error getting sound!"
        )
        boom = None

csDefaultRunLoop(object_reg)

#csInitializer.DestroyApplication(object_reg)
#object_reg = None
