import sys, os
try:
    cspath=os.environ["CRYSTAL"]
except:
    cspath=""

sys.path.append (os.path.join(cspath, "scripts", "python"))
from cspace import *
object_reg = csInitializer.CreateEnvironment (sys.argv)
csInitializer.SetupConfigManager(object_reg)
plugin_requests = [
    CS_REQUEST_VFS, CS_REQUEST_SOFTWARE3D, CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER, CS_REQUEST_IMAGELOADER, CS_REQUEST_LEVELLOADER,
]
csInitializer.RequestPlugins(object_reg, plugin_requests)
ds = csTinyDocumentSystem()
object_reg.Register(ds, 'iDocumentSystem')
x = CS_QUERY_REGISTRY(object_reg, iDocumentSystem)
print x
x = None

if 1:
    iter = object_reg.Get()
    while 1:
        base = iter.GetCurrent()
        if not base:
            break
        print 'base =', base
        print 'tag =', iter.GetCurrentTag()
        if not iter.Next():
            break

csInitializer.DestroyApplication(object_reg)
object_reg = None
