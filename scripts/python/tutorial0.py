#!/usr/bin/env python
"""
CsPython Tutorial Example 0
By Mark Gossage (mark@gossage.cjb.net)

Minimal pure-Python script to setup Crystal Space from Python.

To use this, ensure that your PYTHONPATH, CRYSTAL, and LD_LIBRARY_PATH
(or DYLD_LIBRARY_PATH for MacOS/X; or PATH for Windows) variables are set
approrpriately, and then run the script with the command:

    python scripts/python/tutorial1.py

Trivial example, sets up CS, opens the app & waits for the escape key

===========================================================================
There are two ways to use the CsPython module.
Either as a plugin within CS (pysimp.py), 
or as a pure Python module (this example).

If you refer to the first part of the CS Tutorial 1, this is quite similar
Just about all the C++ functions are mirrored in Python.

The program structure is deliberately done without functions, 
just to keep it simple.
"""

import types, string, re, sys
import traceback

try:    # get in CS
    from cspace import *
except:
    print "WARNING: Failed to import module cspace"

# utils code
#############################
# Note: we are assuming a global 'object_reg'
# which will be defined later

def FatalError(msg="FatalError"):
    "A Panic & die routine"
    csReport(object_reg,CS_REPORTER_SEVERITY_ERROR, "crystalspace.application.python", msg)
    sys.exit(1)

        
# IMPORTANT:
# due to the nature of the event handler 
# (its called directly from the C++ code)
# any exceptions thrown (including those caused by syntax errors)
# will not display error messages/cause the execution to end
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
    except: # the exception catcher
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

if not csInitializer.SetupEventHandler(object_reg, EventHandler):
    FatalError("Could not initialize event handler!")
  
if not csInitializer.OpenApplication(object_reg):
    FatalError("Could not open the application!")

print "CsPython is Go..."
csDefaultRunLoop(object_reg)
print "CsPython is ending"

csInitializer.DestroyApplication (object_reg)
