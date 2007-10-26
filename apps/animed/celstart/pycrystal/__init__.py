import cspace
try:
	import blcelc
except:
	pass
import sys

print "Init PyCrystal"

ObjectRegistry = cspace.csInitializer.CreateEnvironment(sys.argv)
VFS = cspace.csInitializer.SetupVFS(ObjectRegistry)


