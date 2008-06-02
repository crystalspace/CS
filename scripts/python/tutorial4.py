#!/usr/bin/env python
"""
CsPython Tutorial Example 4
By Pablo Martin (caedes@grupoikusnet.com)

A pure-Python script to show the use of Crystal Space.

This is the same as the tutorial3.py example, but using an
application base class provided in the sample csutils python
module.
"""

from csutils import CsAppBase
from cspace import *

# Python SimpMap
#############################
class PySimpMap(CsAppBase):
    def __init__(self):
	CsAppBase.__init__(self)
    def SetupApp(self):
        self.LoadMap("/lev/partsys","world")

# startup code
#############################
if __name__ == "__main__":
    app=PySimpMap() # this is the one & only app
    app.Run() # enter the app loop

    # everything below this is just necessary to get a perfectly clean
    # exit, should not be necessary for simple apps.
    object_reg = app.oreg # save object registry to be able to destroy cs
    app=None    # need to do this or you get 'unreleased instances' warning

    csInitializer.DestroyApplication (object_reg)   # bye bye
    object_reg=None # just to be complete (not really needed)

