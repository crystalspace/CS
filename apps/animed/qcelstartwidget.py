#!/usr/bin/python
from PyQt4.QtCore import SIGNAL, QTimer
from PyQt4.QtGui import QWidget, QSizePolicy, QApplication

from celstart import Celstart

import sip, sys, os, types, string, re, sys, traceback

from cspace import *
from blcelc import *
from pycsextra import iQtWindow

class QCelstartWidget(QWidget, Celstart):
   __pyqtSignals__ = ("Initialized(PyQt_PyObject)",)
   def __init__(self, parent):
       print "init QWidget"
       Celstart.__init__(self)
       QWidget.__init__(self, parent)
       self.setSizePolicy(QSizePolicy(QSizePolicy.Expanding,QSizePolicy.Expanding))
       self.setMinimumSize(400, 400)
       self.render_widget = False
       self.view = False

   def LoadWorld(self,real_path):
       # Override LoadWorld so we can provide our own pre and post
       # plugin registering code.
       self.InitializeEngine(real_path,self.PrePlugins,self.PostPlugins)
       self.ParseConfiguration(real_path)
       self.FindCamera()
       #self.resizeEvent = self.Resize
   def FindCamera(self):
       for ent in self.pl.Entities:
           cam = celGetDefaultCamera(ent)
	   self.player = ent
	   self.pccam = cam
           if cam:
               self.view = cam.GetView()
	       self.cam = self.pccam.GetCamera()
	       print "FOUND CAMERA",ent.Name
	       #self.resizeEvent = self.Resize
               return True

   def PostPlugins(self):
       self.qtwin = SCF_QUERY_INTERFACE(self.g2d, iQtWindow)
       self.qtwin.SetParentAddress(sip.unwrapinstance(self))
       self.vc1 = CS_QUERY_REGISTRY(self.object_reg, iVirtualClock)
       self.q = CS_QUERY_REGISTRY(self.object_reg, iEventQueue)

   def PrePlugins(self):
       plugmgr = CS_QUERY_REGISTRY(self.object_reg,iPluginManager)
       print "fontsrv",plugmgr
       fontsrv = CS_LOAD_PLUGIN (plugmgr,"crystalspace.font.server.default", iFontServer)
       self.object_reg.Register(fontsrv,"iFontServer")
       print iFontServer,fontsrv
       print "csqt4",plugmgr,iFontServer.scfGetVersion(),iEngine.scfGetVersion(),iGraphics2D.scfGetVersion()
       canvas = CS_LOAD_PLUGIN (plugmgr,"crystalspace.graphics2d.csqt4",iGraphics2D)
       self.object_reg.Register(canvas,"iGraphics2D")
       self.object_reg.Unregister(fontsrv,"iFontServer")

   def Run(self):
       #print "VIRTUAL",self.vc
       self.vc = CS_QUERY_REGISTRY(self.object_reg, iVirtualClock)
       if not self.render_widget:
           self.render_widget = sip.wrapinstance(self.qtwin.GetWindowAddress(), QWidget)
           self.render_widget.setAcceptDrops(True)

       self.timer = QTimer( self )
       self.connect( self.timer, SIGNAL("timeout()"), self.Advance)
       self.timer.start(1)

       self.emit(SIGNAL("Initialized(PyQt_PyObject)"), (self))

   def Advance (self):
       self.vc.Advance()
       self.vc1.Advance()
       self.q.Process()

   def resizeEvent(self, event):
       if self.view:
           #self.view.GetCamera().SetFOVAngle(90,self.g2d.GetWidth())
           #self.view.GetCamera().SetPerspectiveCenter(self.g2d.GetWidth()/2.0,self.g2d.GetHeight()/2.0)
           if self.render_widget:
               QApplication.sendEvent(self.render_widget, event);

   #def __del__(self):
   #   if csInitializer:
   #      csInitializer.DestroyApplication (self.object_reg)
   #     self.object_reg=None

