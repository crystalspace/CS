# This is a pure-Python program which demonstrates simple collision detection,
# just as does the C++ tutorial program "simpcd". Run this script by invoking
# "python pysimpcd.py" after setting PYTYHONPATH, CRYSTAL, LD_LIBRARY_PATH (or
# DLYD_LIBRARY_PATH for MacOS/X; or PATH for Windows) appropriately.

import sys, time, traceback, math
from cspace import *

def CreateRoom (matname):
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
            boom.Unpause()
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
    if ((ev.Name  == KeyboardDown) and
        (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC)):
        q  = object_reg.Get(iEventQueue)
        if q:
            q.GetEventOutlet().Broadcast(csevQuit(object_reg))
            return 1
    return 0

def EventHandler (ev):
    #print 'EventHandler called'
    if ev.Name == Frame:
        SetupFrame()
        FinishFrame()
        return 1
    else:
        try:
            return HandleEvent(ev)
        except:
            traceback.print_exc()
    return 0

def InitCollider (mesh):
    stringset = object_reg.Get('crystalspace.shared.stringset', iStringSet)
    tridata = mesh.GetMeshObject().GetObjectModel().GetTriangleData( \
        stringset.Request('base'))
    if tridata:
        wrap = csColliderWrapper(mesh.QueryObject(), cdsys, tridata)

        # Not needed in C++, but necessary here... strange... TODO
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
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_PLUGIN("crystalspace.sndsys.element.wav", iSndSysLoader),
    CS_REQUEST_PLUGIN("crystalspace.sndsys.renderer.software", iSndSysRenderer),
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
  
cdsys = object_reg.Get(iCollideSystem)
if not cdsys:
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.pysimpcd",
        "Can't find the collision detection system!")
    sys.exit(1)
 
vc = object_reg.Get(iVirtualClock)
if not vc:
    csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.pysimpcd",
        "Can't find the virtual clock!")
    sys.exit(1)

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

snd = object_reg.Get(iSndSysRenderer)
if not snd:
    Report(CS_REPORTER_SEVERITY_ERROR, "No iSndSysRenderer!")

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
CreateRoom('stone')

ll = room.GetLights()
light = engine.CreateLight("", csVector3(-3, 5, 0), 10, csColor(1, 0, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
ll.Add(light)
light = engine.CreateLight("", csVector3(3, 5, 0), 10, csColor(0, 0, 1), CS_LIGHT_DYNAMICTYPE_STATIC)
ll.Add(light)
light = engine.CreateLight("", csVector3(0, 5, -3), 10, csColor(0, 1, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
ll.Add(light)

engine.Prepare()

SimpleStaticLighter.ShineLights(room, engine, 4)

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

loader = object_reg.Get(iLoader)
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
spstate = parent_sprite.GetMeshObject().QueryInterface(iSprite3DState)
spstate.SetAction("default")
parent_sprite.GetMovable().Transform(csZRotMatrix3(math.pi/2.))
parent_sprite.GetMovable().UpdateMove()

# Now create the first child.
sprite1 = engine.CreateMeshWrapper(imeshfact, "Rotater1")
sprite1.GetMovable().SetPosition(csVector3(0, -.5, -.5))
sprite1.GetMovable().Transform(csZRotMatrix3(math.pi/2.))
sprite1.GetMovable().UpdateMove() 
spstate = sprite1.GetMeshObject().QueryInterface(iSprite3DState)
spstate.SetAction("default")
sprite1.QuerySceneNode().SetParent(parent_sprite.QuerySceneNode())

# Now create the second child.
sprite2 = engine.CreateMeshWrapper(imeshfact, "Rotater2")
sprite2.GetMovable().SetPosition(csVector3(0, .5, -.5))
sprite2.GetMovable().Transform(csZRotMatrix3(math.pi/2.))
sprite2.GetMovable().UpdateMove()
spstate = sprite2.GetMeshObject().QueryInterface(iSprite3DState)
spstate.SetAction("default")
sprite2.QuerySceneNode().SetParent(parent_sprite.QuerySceneNode())
    
# We only do collision detection for the rotating children
# so that's the only colliders we have to create.
sprite1_col = InitCollider(sprite1)
if not sprite1_col:
    sys.exit(1)
sprite2_col = InitCollider(sprite2)
if not sprite2_col:
    sys.exit(1)

if snd:
    boom = LevelLoader.LoadSoundStream('/lib/std/whoosh.wav', CS_SND3D_ABSOLUTE)
    if boom:
        src = snd.CreateSource(boom)
        src.SetVolume(0.8)
    else:
        csReport(object_reg, CS_REPORTER_SEVERITY_ERROR,
            "crystalspace.application.pysimpcd", "Error getting sound!"
        )
        boom = None

# Get some often used event IDs
KeyboardDown = csevKeyboardDown(object_reg)
Frame = csevFrame(object_reg)

csDefaultRunLoop(object_reg)

#csInitializer.DestroyApplication(object_reg)
#object_reg = None
