#!/usr/bin/env python
"""
CsPython Tutorial Example 3
By Mark Gossage (mark@gossage.cjb.net)

A pure-Python script to show the use of Crystal Space.

To use this, ensure that your PYTHONPATH, CRYSTAL, and LD_LIBRARY_PATH
(or DYLD_LIBRARY_PATH for MacOS/X; or PATH for Windows) variables are set
approrpriately, and then run the script with the command:

    python scripts/python/tutorial1.py

This performs the same features at tutorial C++ tutorial 'simpmap',
loading a map with all the nice mesh effects.

===========================================================================
There are two ways to use the CsPython module.
Either as a plugin within CS (pysimp), 
or as a pure Python module (this example).

This is just the CS tutorial 'simpmap' rewritten in Python.
Overall it looks quite like the original C++ code, 
just with Python syntax; but the functions are all the same names and formats
(use None instead of NULL, and "" instead of (char*)NULL).

Please refer to the CS Tutorial 3 in the documentation
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
            
        self.view=csView(self.engine,self.g3d)
        g2d = self.g3d.GetDriver2D()
        self.view.SetRectangle(0, 0, g2d.GetWidth(), g2d.GetHeight ())

        self.LoadMap("world")

        Log('MyCsApp.Init() finished')


    def LoadMap(self,name):
        # Set VFS current directory to the level we want to load.
        vfs=CS_QUERY_REGISTRY(object_reg,iVFS)
        vfs.ChDir("/lev/partsys");
        # Load the level file which is called 'world'.
        if not self.loader.LoadMapFile(name):
            FatalError("Couldn't load level!")
            
        self.engine.Prepare()

        # Find the starting position in this level.
        pos=csVector3(0,0,0)
        room=None
        
        if self.engine.GetCameraPositions().GetCount()>0:
            #// There is a valid starting position defined in the level file.
            campos = self.engine.GetCameraPositions().Get(0)
            room = self.engine.GetSectors().FindByName(campos.GetSector())
            pos = campos.GetPosition()
        else:
            #// We didn't find a valid starting position. So we default
            #// to going to room called 'room' at position (0,0,0).
            room = self.engine.GetSectors().FindByName("room")

        if room==None:
            FatalError("Can't find a valid starting position!")

        self.view.GetCamera().SetSector(room)
        self.view.GetCamera().GetTransform().SetOrigin(pos)

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
