#!/usr/bin/python
import sys, os, types, string, re, sys, traceback
sys.path.append(os.environ["CRYSTAL"])
#sys.path.append(os.environ["CSEXTRA"])

import pycrystal
from cspace import *
from blcelc import *
import blcelc
import cspace
QueryPlugin = lambda s: pycrystal.ObjectRegistry.Get(s)

def initPyCel(oreg,pl):
    " * setting up pycel pointers"
    blcelc.object_reg_ptr = oreg
    blcelc.physicallayer_ptr = pl
    import pycel

def FatalError(error):
   print "Error:",error
   sys.exit(1)

class Celstart(csPyEventHandler):
   def __init__(self):
      csPyEventHandler.__init__(self)
      self.path = "/tmp/celstart"

   def HandleEvent(self,ev):
      try:
         if ev.Name == self.evFinalProcess:
             self.FinishFrame()
             return True
         return self.EatEvent(ev)
      except:
         traceback.print_exc()
         sys.exit(1)
         return 0

   def EatEvent(self, ev):
      return False

   def InitializeEngine(self,map=None,pre_plugins_cb=None,post_plugins_cb=None):
      sys.path.append(map)
      self.object_reg = pycrystal.ObjectRegistry
      vfs = pycrystal.VFS
      print "mounting",map,"at",self.path
      vfs.Mount(self.path,map)
      if self.object_reg is None:
         FatalError("Couldn't create enviroment!")

      if csCommandLineHelper.CheckHelp(self.object_reg):
         csCommandLineHelper.Help(self.object_reg)
         sys.exit(0)

      if not vfs.Exists("/tmp/celstart/celstart.cfg"):
         FatalError("file is not celstart world")

      if not csInitializer.SetupConfigManager(self.object_reg,
                                    "/tmp/celstart/celstart.cfg"):
            FatalError("Couldn't init app!")

      corecvar.iSCF_SCF.ScanPluginsPath(os.environ["CEL"], False, "cel")
      print "SetupCelPluginDirs"
      #celInitializer.SetupCelPluginDirs(self.object_reg)
      print "RequestPlugins"
      if pre_plugins_cb:
	 pre_plugins_cb()
      if not csInitializer.RequestPlugins(self.object_reg, [CS_REQUEST_VFS]):
         FatalError("Plugin requests failed!")
      self.QueryEventIds()
      self.QueryPlugins()
      if post_plugins_cb:
	 post_plugins_cb()
      if not csInitializer.OpenApplication(self.object_reg):
         FatalError("Could not open the application!")
      self.SetupEventHandler()
   def QueryEventIds(self):
      self.evProcess = csevProcess(self.object_reg)
      self.evPreProcess = csevPreProcess(self.object_reg)
      self.evPostProcess = csevPostProcess(self.object_reg)
      self.evFinalProcess = csevFinalProcess(self.object_reg)
      self.evInput = csevInput(self.object_reg)
      self.evKeyboardEvent = csevKeyboardEvent(self.object_reg)
      self.evMouseEvent = csevMouseEvent(self.object_reg)
      self.evMouseDown = csevMouseDown(self.object_reg,0)
      self.evQuit = csevQuit(self.object_reg)

   def QueryPlugins(self):
      self.engine = QueryPlugin(iEngine)
      self.g3d = QueryPlugin(iGraphics3D)
      self.vfs = QueryPlugin(iVFS)
      self.loader = QueryPlugin(iLoader)
      self.g2d = self.g3d.GetDriver2D()
      self.pl = QueryPlugin(iCelPlLayer)
      initPyCel(self.object_reg,self.pl)
      if self.engine==None or self.g3d==None or self.loader==None:
         FatalError("Cs core plugins missing!")
      if not self.pl:
         FatalError("No physical layer!")

   def LoadWorld(self, real_path):
      self.InitializeEngine(real_path)
      self.ParseConfiguration(real_path)

   def ParseConfiguration(self, real_path):
      Config = QueryPlugin(iConfigManager)
      # Some misc data
      name = Config.GetStr("Celstart.Name")
      minimum_version = Config.GetInt("CelStart.MinimumVersion")
      maximum_version = Config.GetInt("CelStart.MaximumVersion")
      desc = Config.GetInt("CelStart.Description",0)
      icon = Config.GetStr("CelStart.Icon")
      # Config files
      enum = Config.Enumerate("CelStart.ConfigDir.")
      self.vfs.PushDir("/tmp/celstart/")
      while enum and enum.Next():
         cfg_dir = enum.GetStr()
         cfg_dir+="/"
         if self.vfs.Exists(cfg_dir):
            cfg_files = self.vfs.FindFiles(cfg_dir)
            for i in xrange(cfg_files.GetSize()):
               Config.Load(cfg_files.Get(i),self.vfs,True)
      self.vfs.PopDir()
      # Behaviour layers
      print "loading behaviour layers"
      enum = Config.Enumerate("CelStart.BehaviourLayer.")
      plugmgr = QueryPlugin(iPluginManager)
      while enum and enum.Next():
         bhname = enum.GetKey(True)
         id = enum.GetStr()
         bl = CS_LOAD_PLUGIN (plugmgr,id,iCelBlLayer)
         if not bl:
            print "cant load bl",id
	 self.bl = bl
         self.object_reg.Register(bl,bhname)
         self.pl.RegisterBehaviourLayer(bl)
      # Entities
      print "loading entities"
      enum = Config.Enumerate("CelStart.Entity.")
      while enum and enum.Next():
         entityname = enum.GetKey(True)
         ent = self.pl.CreateEntity()
         ent.SetName(entityname)
         behname_key = "CelStart.EntityBehaviour."+entityname
         behname = Config.GetStr(behname_key)
         if behname:
            bl_key = "CelStart.EntityBehaviourLayer."+entityname
            blname = Config.GetStr(bl_key)
            if blname:
               bl = self.pl.FindBehaviourLayer(blname)
            else:
               bl = QueryPlugin(iCelBlLayer)
            if not bl:
               FatalError("Couldnt find behaviour layer "+blname)
            beh = bl.CreateBehaviour(ent,behname)
            if not beh:
               FatalError("Couldnt create behaviour "+behname)
      # Load map files
      enum = Config.Enumerate("CelStart.MapFile.")
      while enum and enum.Next():
         file = enum.GetStr()
         self.vfs.ChDirAuto(self.path) #,0,0,file)
         self.loader.LoadMapFile(file,False)
      # Misc
      do_clearscreen = Config.GetBool("CelStart.ClearScreen", False)
      # Set native window title
      title = Config.GetStr ("CelStart.WindowTitle","CelStart Application")
      native_window = self.g2d.GetNativeWindow()
      native_window.SetTitle(title)
      return True

   def SetupEventHandler(self):
      eventids = [self.evInput, self.evKeyboardEvent,
                  self.evMouseEvent, self.evQuit, self.evProcess,
                  self.evFinalProcess, CS_EVENTLIST_END]
      if not csInitializer.SetupEventHandler(self.object_reg, self, eventids):
         FatalError("Could not initialize event handler!")

   def Run(self):
      csDefaultRunLoop(self.object_reg)
        
   def FinishFrame(self):
      self.g3d.FinishDraw()
      self.g3d.Print(None)

   def __del__(self):
      if csInitializer:
         csInitializer.DestroyApplication (self.object_reg)
         self.object_reg=None

if __name__ == '__main__':
   celstart = Celstart()
   celstart.LoadWorld(sys.argv[1])
   celstart.Run()

