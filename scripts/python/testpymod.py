import sys, time, traceback
from cspace import *

def CreateRoom (matname):
	#print 'Start creating polygons from Python script...'
	#print 'object_reg=',object_reg
	#print 'dir(object_reg)=',dir(object_reg)
	engine = CS_QUERY_REGISTRY(object_reg, iEngine)
	#print 'engine=',engine
	vc = CS_QUERY_REGISTRY(object_reg, iVirtualClock)
	#print 'vc=',vc
	loader = CS_QUERY_REGISTRY(object_reg, iLoader)
	#print 'loader=',loader
	matname = 'mystone'
	loader.LoadTexture (matname, "/lib/stdtex/bricks.jpg")
	room = engine.GetSectors().FindByName("room")
	walls = engine.CreateSectorWallsMesh(room, "walls")
	#print 'walls=',walls
	thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)
	material=engine.GetMaterialList().FindByName(matname)

	poly=thingstate.CreatePolygon('floor')
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('ceiling')
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(-5,20,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w1')
	poly.CreateVertex(csVector3(-5,20,5))
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.CreateVertex(csVector3(-5,0,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w2')
	poly.CreateVertex(csVector3(5,20,5))
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.CreateVertex(csVector3(5,0,5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w3')
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(-5,20,5))
	poly.CreateVertex(csVector3(-5,0,5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)

	poly=thingstate.CreatePolygon('w4')
	poly.CreateVertex(csVector3(5,20,-5))
	poly.CreateVertex(csVector3(-5,20,-5))
	poly.CreateVertex(csVector3(-5,0,-5))
	poly.CreateVertex(csVector3(5,0,-5))
	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
	poly.SetMaterial(material)
	thingstate.DecRef()
	#print 'Finished!'

def SetupFrame ():
	#print 'SetupFrame called'
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
	if not myG3D.BeginDraw(CSDRAW_3DGRAPHICS):
		sys.exit(1)
	if view:
		view.Draw()
	#print 'SetupFrame done'

def FinishFrame ():
	#print 'FinishFrame called'
	myG3D.FinishDraw()
	myG3D.Print(None)
	#print 'FinishFrame done'

def HandleEvent (ev):
	#print 'HandleEvent called'
	if ev.Type == csevKeyDown and ev.Key.Code == CSKEY_ESC:
		q  = CS_QUERY_REGISTRY(object_reg, iEventQueue)
		if q:
			q.GetEventOutlet().Broadcast(cscmdQuit)
			return 1
	return 0

def EventHandler (ev):
	#print 'EventHandler called'
	#print '   ev=%s' % ev
	if ev.Type == csevBroadcast and ev.Command.Code == cscmdProcess:
		try:
			SetupFrame()
		except:
			traceback.print_exc()
		return 1
  	elif ev.Type == csevBroadcast and ev.Command.Code == cscmdFinalProcess:
		try:
			FinishFrame()
		except:
			traceback.print_exc()
		return 1
	elif ev.Type == csevBroadcast and ev.Command.Code == cscmdCommandLineHelp:
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
	CS_REQUEST_VFS, CS_REQUEST_SOFTWARE3D, CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER, CS_REQUEST_IMAGELOADER, CS_REQUEST_LEVELLOADER,
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
  
vc = CS_QUERY_REGISTRY(object_reg, iVirtualClock)

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
txtmgr.SetVerbose(1)

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
		plugin_mgr.__deref__(), "crystalspace.script.python", iScript
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

light = engine.CreateLight("", csVector3(0, 5, 0), 10, csColor(1, 0, 0), 0)
#print 'light=',light
room.GetLights().Add(light.QueryLight())

#print 'calling engine.Prepare()'

engine.Prepare()

Report(CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------")

# csView is a view encapsulating both a camera and a clipper.
# You don't have to use csView as you can do the same by
# manually creating a camera and a clipper but it makes things a little
# easier.
view = csView(engine.__deref__(), myG3D.__deref__())
view.GetCamera().SetSector(room)
view.GetCamera().GetTransform().SetOrigin(csVector3(0, 2, 0))
g2d = myG3D.GetDriver2D()
view.SetRectangle(2, 2, g2d.GetWidth() - 4, g2d.GetHeight() - 4)

csDefaultRunLoop(object_reg)



