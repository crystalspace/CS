#!/usr/bin/env python
"""
CsPython csutils module.
By Pablo Martin (caedes@grupoikusnet.com)

Some utilities to allow easy creation of pure python cs apps.

At the moment basically provides CsAppBase class, which automatically
creates cs environment, registers event handler and so on.
You can override the following methods to customize behaviour:
 CheckHelp() 			# check help
 SetupConfig()			# setup config
 RequestPlugins()		# plugins request
 InitPlugins()			# initialize plugins
 InitEventHandler()		# initialize event handler
 SetupApp()			# setup application specific code
 HandleEvent()			# event handler

Check tutorial4.py for an example on using this.
"""

import types, string, re, sys
import traceback

try:    # get in CS
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"
    traceback.print_exc()
    sys.exit(1) # die!!

# utils code
#############################
# Note: we are assuming a global 'object_reg'
# which will be defined later

class CsReporterApp(object):
    def __init__ (self,object_reg):
	self._oreg = object_reg
    def Report (self,severity, msg):
        "Reporting routine"
        csReport(self._oreg, severity, "crystalspace.application.python", msg)
    
    def Log(self,msg):
        self.Report(CS_REPORTER_SEVERITY_NOTIFY, msg)

    def FatalError(self,msg="FatalError"):
        "A Panic & die routine"
        self.Report(CS_REPORTER_SEVERITY_ERROR,msg)
        sys.exit(1)

# CsAppBase Class
#############################
class CsAppBase(csPyEventHandler,CsReporterApp):
    def __init__(self,object_reg=None):
	csPyEventHandler.__init__(self)
	if not object_reg:
            self.oreg = csInitializer.CreateEnvironment(sys.argv)
	else:
            self.oreg = object_reg
	CsReporterApp.__init__(self,self.oreg)
	# do init in several steps so app can override behaviour
	self.CheckHelp()
	self.SetupConfig()
	self.RequestPlugins()
	self.InitPlugins()
	self.InitEventHandler()
	self.SetupApp()
        self._initialized = True
    def CheckHelp(self):
        if csCommandLineHelper.CheckHelp(self.oreg):
            csCommandLineHelper.Help(self.oreg)
            sys.exit(0)

    def SetupConfig(self):
        if not csInitializer.SetupConfigManager(self.oreg):
            self.FatalError("Couldn't init app!")

    def RequestPlugins(self):
        plugin_requests = [
            CS_REQUEST_VFS, CS_REQUEST_OPENGL3D, CS_REQUEST_ENGINE,
            CS_REQUEST_FONTSERVER, CS_REQUEST_IMAGELOADER, 
	    CS_REQUEST_LEVELLOADER ]
        if not csInitializer.RequestPlugins(self.oreg, plugin_requests):
            self.FatalError("Plugin requests failed!")

    def Run(self):
        csDefaultRunLoop(self.oreg)

    def InitPlugins(self):
	self.vc = self.oreg.Get(iVirtualClock)
        self.engine = self.oreg.Get(iEngine)
        self.g3d = self.oreg.Get(iGraphics3D)
        self.loader = self.oreg.Get(iLoader)
        self.keybd = self.oreg.Get(iKeyboardDriver)
        
        if self.vc==None or self.engine==None or self.g3d==None or self.keybd==None or self.loader==None:
            self.FatalError("Error: in object registry query")
        
        if not csInitializer.OpenApplication(self.oreg):
            self.FatalError("Could not open the application!")
            
        self.view = csView(self.engine,self.g3d)
        self.g2d = self.g3d.GetDriver2D()
        self.view.SetRectangle(0, 0, self.g2d.GetWidth(), self.g2d.GetHeight ())
	self.KeyboardDown = csevKeyboardDown(self.oreg)
	self.Frame = csevFrame(self.oreg)

    def InitEventHandler(self):
        if not csInitializer.SetupEventHandler(self.oreg, self):
            self.FatalError("Could not initialize event handler!")

    def SetupApp(self):
	pass

    def HandleEvent(self,ev):
	if ((ev.Name  == self.KeyboardDown) and
	    (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC)):
	    q  = self.oreg.Get(iEventQueue)
	    if q:
	        q.GetEventOutlet().Broadcast(csevQuit(self.oreg))
	        q.RemoveListener(self)
	        return 1
	elif ev.Name == self.Frame:
	    self.SetupFrame()
	    self.FinishFrame()
	    return 1
	return 0

    def LoadMap(self,path,name):
        # Set VFS current directory to the level we want to load.
        vfs=self.oreg.Get(iVFS)
        vfs.ChDir(path);
        # Load the level file which is called 'world'.
        if not self.loader.LoadMapFile(name):
            self.FatalError("Couldn't load level!")
            
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
            self.FatalError("Can't find a valid starting position!")
        self.PlaceCamera(room,pos)

    def PlaceCamera(self,sector,origin):
        camera = self.view.GetCamera()
        camera.SetSector(sector)
        camera.GetTransform().SetOrigin(origin)

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
            self.FatalError()
        self.view.Draw()

    def FinishFrame(self):
        self.g3d.FinishDraw()
        self.g3d.Print(None)

# startup code
#############################
if __name__ == "__main__":
    app=CsAppBase()   # this is the one & only app
    app.Run() # enter the app loop

    object_reg = app.oreg
    app=None    # need to do this or you get 'unreleased instances' warning

    csInitializer.DestroyApplication (object_reg)   # bye bye
    object_reg=None # just to be complete (not really needed)

