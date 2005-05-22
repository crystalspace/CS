#!/usr/bin/env python
"""
CsPython Tutorial Example 2
By Mark Gossage (mark@gossage.cjb.net)

A pure-Python script to show the use of Crystal Space.

To use this, ensure that your PYTHONPATH, CRYSTAL, and LD_LIBRARY_PATH
(or DYLD_LIBRARY_PATH for MacOS/X; or PATH for Windows) variables are set
approrpriately, and then run the script with the command:

    python scripts/python/tutorial1.py

This performs the same features as the C++ tutorial2.
It creates a room and a 3D sprite.

===========================================================================
There are two ways to use the CsPython module.
Either as a plugin within CS (pysimp), 
or as a pure Python module (this example).

This is just the CS C++ tutorial 2 rewritten in Python.
Overall it looks remarkable like the original C++ code, 
just with Python syntax; but the functions are all the same names and formats
(use None instead of NULL, and "" instead of (char*)NULL).

Please refer to the CS Tutorial 2 in the documentation
for detail on how the C++ works.
"""

import types, string, re, sys
import traceback

try:    # get in CS
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"
    sys.exit(1) # die!!

# utils code
#############################
# Note: we are assuming a global 'object_reg'
# which will be defined later

def Report (severity, msg):
    "Reporting routine"
    csReport(object_reg, severity, "crystalspace.application.python", msg)
    
def Log(msg):
    Report(CS_REPORTER_SEVERITY_NOTIFY, msg)

def FatalError(msg="FatalError"):
    "A Panic & die routine"
    Report(CS_REPORTER_SEVERITY_ERROR,msg)
    sys.exit(1)

# EventHandler
#############################
class MyCsApp:
    def Init(self):
        Log('MyCsApp.Init()...')
        self.vc = CS_QUERY_REGISTRY(object_reg, iVirtualClock)
        self.engine = CS_QUERY_REGISTRY(object_reg, iEngine)
        self.g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D)
        self.loader = CS_QUERY_REGISTRY(object_reg, iLoader)
        self.keybd = CS_QUERY_REGISTRY(object_reg, iKeyboardDriver)
        
        if self.vc==None or self.engine==None or self.g3d==None or self.keybd==None or self.loader==None:
            FatalError("Error: in object registry query")
        
        if not csInitializer.OpenApplication(object_reg):
            FatalError("Could not open the application!")
            
        txtmgr=self.g3d.GetTextureManager()
        
        room=self.SetupRoom()   # creates & returns the room
        self.CreateLights(room)
        self.LoadSprites(room)
        self.CreateCamera(room,csVector3(0, 5, -3))       

        self.engine.Prepare()
        Log('MyCsApp.Init() finished')

    def SetupRoom(self):
        # First disable the lighting cache. Our app is simple enough
        # not to need this.
        self.engine.SetLightingCacheMode(0)
        
        # load a texture
        if self.loader.LoadTexture("stone", "/lib/std/stone4.gif")==None:
            FatalError("Error: unable to load texture")

        # now get it as a material from the engine
        material=self.engine.GetMaterialList().FindByName("stone")

        # create the 'room'  
        room = self.engine.CreateSector("room")
        walls = self.engine.CreateSectorWallsMesh(room,"walls")
        thingstate = SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState)
        walls_state = thingstate.GetFactory()
        walls_state.AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5))
        walls_state.SetPolygonMaterial (CS_POLYRANGE_LAST, material);
        walls_state.SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);        
        return room

    def CreateLights(self,room):
        # lights
        ll = room.GetLights ()
        light = self.engine.CreateLight ("", csVector3 (-3, 5, 0), 10,csColor (1, 0, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
        ll.Add (light)
        light = self.engine.CreateLight ("", csVector3 (3, 5,  0), 10,csColor (0, 0, 1), CS_LIGHT_DYNAMICTYPE_STATIC)
        ll.Add (light)
        light = self.engine.CreateLight ("", csVector3 (0, 5, -3), 10,csColor (0, 1, 0), CS_LIGHT_DYNAMICTYPE_STATIC)
        ll.Add (light)
    
    def LoadSprites(self,room):
        # Load a texture for our sprite.
        txtmgr=self.g3d.GetTextureManager()
        txt=self.loader.LoadTexture ("spark","/lib/std/spark.png",CS_TEXTURE_3D,txtmgr,1)
        if txt == None:
            FatalError("Error loading texture!")

        # Load a sprite template from disk.
        imeshfact=self.loader.LoadMeshObjectFactory("/lib/std/sprite1")
        if imeshfact == None:
            FatalError("Error loading mesh!")

        # Create the sprite and add it to the engine.
        sprite=self.engine.CreateMeshWrapper(imeshfact,"MySprite",room,csVector3 (-3, 5, 3))
        m=csMatrix3()
        m.Identity()    # make sure its identity
        #m.__imul__(5.)  # this is the same as m=m*5      
        m=m*5
        sprite.GetMovable().SetTransform(m)
        sprite.GetMovable().UpdateMove()
        spstate=SCF_QUERY_INTERFACE(sprite.GetMeshObject(),iSprite3DState)
        spstate.SetAction("default")
        #spstate.SetMixMode(CS_FX_SETALPHA (.5))

        # The following two calls are not needed since CS_ZBUF_USE and
        # Object render priority are the default but they show how you
        # can do this.
        sprite.SetZBufMode(CS_ZBUF_USE)
        sprite.SetRenderPriority(self.engine.GetObjectRenderPriority())


    def CreateCamera(self,room,origin):
        self.view = csView(self.engine, self.g3d)
        self.view.GetCamera().SetSector(room)
        self.view.GetCamera().GetTransform().SetOrigin(origin)
        g2d = self.g3d.GetDriver2D()
        self.view.SetRectangle(2, 2, g2d.GetWidth() - 4, g2d.GetHeight() - 4)


    def SetupFrame (self):
        #print 'SetupFrame called',
        elapsed_time = self.vc.GetElapsedTicks()
        # Now rotate the camera according to keyboard state
        speed = (elapsed_time / 1000.) * (0.03 * 20);
        if self.keybd.GetKeyState(CSKEY_RIGHT):
            self.view.GetCamera().GetTransform().RotateThis(CS_VEC_ROT_RIGHT, speed)
        if self.keybd.GetKeyState(CSKEY_LEFT):
            self.view.GetCamera().GetTransform().RotateThis(CS_VEC_ROT_LEFT, speed)
        if self.keybd.GetKeyState(CSKEY_PGUP):
            self.view.GetCamera().GetTransform().RotateThis(CS_VEC_TILT_UP, speed)
        if self.keybd.GetKeyState(CSKEY_PGDN):
            self.view.GetCamera().GetTransform().RotateThis(CS_VEC_TILT_DOWN, speed)
        if self.keybd.GetKeyState(CSKEY_UP):
            self.view.GetCamera().Move(CS_VEC_FORWARD * 4 * speed)
        if self.keybd.GetKeyState(CSKEY_DOWN):
            self.view.GetCamera().Move(CS_VEC_BACKWARD * 4 * speed)
        # Tell 3D driver we're going to display 3D things.
        if not self.g3d.BeginDraw(self.engine.GetBeginDrawFlags() | CSDRAW_3DGRAPHICS):
            FatalError()
        self.view.Draw()
        #print 'SetupFrame done'

    def FinishFrame(self):
        #print 'FinishFrame called'
        self.g3d.FinishDraw()
        self.g3d.Print(None)
        #print 'FinishFrame done'

# EventHandler
#############################
# IMPORTANT:
# due to the nature of the event handler (its called directly from the Cs mainloop)
# any exceptions thrown will not display error messages/ halt the interpreter
# (including those caused by syntax errors)
# therefore we must add in our own code to catch this
def EventHandler(ev):
    try:
        #print 'EventHandler called'
        if ((ev.Type  == csevKeyboard ) and
            (csKeyEventHelper.GetEventType(ev) == csKeyEventTypeDown) and
            (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC)):
            q  = CS_QUERY_REGISTRY(object_reg, iEventQueue)
            if q:
                q.GetEventOutlet().Broadcast(cscmdQuit)
                return 1
        elif ev.Type == csevBroadcast and csCommandEventHelper.GetCode(ev) == cscmdProcess:
            app.SetupFrame()
            return 1
        elif ev.Type == csevBroadcast and csCommandEventHelper.GetCode(ev) == cscmdFinalProcess:
            app.FinishFrame()
            return 1
    except:
        traceback.print_exc()   # prints the usual error messages
        sys.exit(1)             # stop dead
    return 0

# startup code
#############################
# we could write a 'main' fn for this
# but I decided to put in in the body of the app

object_reg = csInitializer.CreateEnvironment(sys.argv)

if object_reg is None:
    FatalError("Couldn't create enviroment!")

if csCommandLineHelper.CheckHelp(object_reg):
    csCommandLineHelper.Help(object_reg)
    sys.exit(0)

if not csInitializer.SetupConfigManager(object_reg):
    FatalError("Couldn't init app!")

plugin_requests = [
    CS_REQUEST_VFS, CS_REQUEST_SOFTWARE3D, CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER, CS_REQUEST_IMAGELOADER, CS_REQUEST_LEVELLOADER,
]
if not csInitializer.RequestPlugins(object_reg, plugin_requests):
    FatalError("Plugin requests failed!")

# setup the event handler:
# note: we need not even make EventHandler() a global fn
# python would accept it as a member fn of MyCsApp
if not csInitializer.SetupEventHandler(object_reg, EventHandler):
    FatalError("Could not initialize event handler!")
        
app=MyCsApp()   # this is the one & only app
  
app.Init()  # turn on the app
# this also now calls OpenApplication

csDefaultRunLoop(object_reg)

app=None    # need to do this or you get 'unreleased instances' warning
# See! CsPython manages the smart pointers correctly

csInitializer.DestroyApplication (object_reg)   # bye bye
object_reg=None # just to be complete (not really needed)
