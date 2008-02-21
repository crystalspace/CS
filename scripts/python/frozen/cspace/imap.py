# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.31
#
# Don't modify this file, modify the SWIG interface instead.

import _imap
import new
new_instancemethod = new.instancemethod
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


def _swig_setattr_nondynamic_method(set):
    def set_attr(self,name,value):
        if (name == "thisown"): return self.this.own(value)
        if hasattr(self,name) or (name == "this"):
            set(self,name,value)
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr


import core
_SetSCFPointer = _imap._SetSCFPointer
_GetSCFPointer = _imap._GetSCFPointer
if not "core" in dir():
    core = __import__("cspace").__dict__["core"]
core.AddSCFLink(_SetSCFPointer)
CSMutableArrayHelper = core.CSMutableArrayHelper

class iLoaderStatus(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def IsReady(*args): return _imap.iLoaderStatus_IsReady(*args)
    def IsError(*args): return _imap.iLoaderStatus_IsError(*args)
    __swig_destroy__ = _imap.delete_iLoaderStatus
    __del__ = lambda self : None;
iLoaderStatus_swigregister = _imap.iLoaderStatus_swigregister
iLoaderStatus_swigregister(iLoaderStatus)

class iMissingLoaderData(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def MissingMaterial(*args): return _imap.iMissingLoaderData_MissingMaterial(*args)
    def MissingTexture(*args): return _imap.iMissingLoaderData_MissingTexture(*args)
    def MissingShader(*args): return _imap.iMissingLoaderData_MissingShader(*args)
    def MissingFactory(*args): return _imap.iMissingLoaderData_MissingFactory(*args)
    def MissingMesh(*args): return _imap.iMissingLoaderData_MissingMesh(*args)
    def MissingSector(*args): return _imap.iMissingLoaderData_MissingSector(*args)
    def MissingLight(*args): return _imap.iMissingLoaderData_MissingLight(*args)
    __swig_destroy__ = _imap.delete_iMissingLoaderData
    __del__ = lambda self : None;
iMissingLoaderData_swigregister = _imap.iMissingLoaderData_swigregister
iMissingLoaderData_swigregister(iMissingLoaderData)

class csLoadResult(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    success = _swig_property(_imap.csLoadResult_success_get, _imap.csLoadResult_success_set)
    result = _swig_property(_imap.csLoadResult_result_get, _imap.csLoadResult_result_set)
    def __init__(self, *args): 
        this = _imap.new_csLoadResult(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _imap.delete_csLoadResult
    __del__ = lambda self : None;
csLoadResult_swigregister = _imap.csLoadResult_swigregister
csLoadResult_swigregister(csLoadResult)

class iLoader(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def LoadImage(*args): return _imap.iLoader_LoadImage(*args)
    def LoadTexture(*args): return _imap.iLoader_LoadTexture(*args)
    def LoadSoundSysData(*args): return _imap.iLoader_LoadSoundSysData(*args)
    def LoadSoundStream(*args): return _imap.iLoader_LoadSoundStream(*args)
    def LoadSoundWrapper(*args): return _imap.iLoader_LoadSoundWrapper(*args)
    def ThreadedLoadMapFile(*args): return _imap.iLoader_ThreadedLoadMapFile(*args)
    def LoadMapFile(*args): return _imap.iLoader_LoadMapFile(*args)
    def LoadMap(*args): return _imap.iLoader_LoadMap(*args)
    def LoadLibraryFile(*args): return _imap.iLoader_LoadLibraryFile(*args)
    def LoadLibrary(*args): return _imap.iLoader_LoadLibrary(*args)
    def LoadMeshObjectFactory(*args): return _imap.iLoader_LoadMeshObjectFactory(*args)
    def LoadMeshObject(*args): return _imap.iLoader_LoadMeshObject(*args)
    def Load(*args): return _imap.iLoader_Load(*args)
    def LoadShader(*args): return _imap.iLoader_LoadShader(*args)
    def SetAutoRegions(*args): return _imap.iLoader_SetAutoRegions(*args)
    def GetAutoRegions(*args): return _imap.iLoader_GetAutoRegions(*args)
    scfGetVersion = staticmethod(_imap.iLoader_scfGetVersion)
    __swig_destroy__ = _imap.delete_iLoader
    __del__ = lambda self : None;
iLoader_swigregister = _imap.iLoader_swigregister
iLoader_swigregister(iLoader)
iLoader_scfGetVersion = _imap.iLoader_scfGetVersion

class iLoaderPlugin(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def Parse(*args): return _imap.iLoaderPlugin_Parse(*args)
    scfGetVersion = staticmethod(_imap.iLoaderPlugin_scfGetVersion)
    __swig_destroy__ = _imap.delete_iLoaderPlugin
    __del__ = lambda self : None;
iLoaderPlugin_swigregister = _imap.iLoaderPlugin_swigregister
iLoaderPlugin_swigregister(iLoaderPlugin)
iLoaderPlugin_scfGetVersion = _imap.iLoaderPlugin_scfGetVersion

class iBinaryLoaderPlugin(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def Parse(*args): return _imap.iBinaryLoaderPlugin_Parse(*args)
    scfGetVersion = staticmethod(_imap.iBinaryLoaderPlugin_scfGetVersion)
    __swig_destroy__ = _imap.delete_iBinaryLoaderPlugin
    __del__ = lambda self : None;
iBinaryLoaderPlugin_swigregister = _imap.iBinaryLoaderPlugin_swigregister
iBinaryLoaderPlugin_swigregister(iBinaryLoaderPlugin)
iBinaryLoaderPlugin_scfGetVersion = _imap.iBinaryLoaderPlugin_scfGetVersion

class iSaver(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def SaveMapFile(*args): return _imap.iSaver_SaveMapFile(*args)
    def SaveAllRegions(*args): return _imap.iSaver_SaveAllRegions(*args)
    def SaveRegionFile(*args): return _imap.iSaver_SaveRegionFile(*args)
    def SaveRegion(*args): return _imap.iSaver_SaveRegion(*args)
    def SavePortal(*args): return _imap.iSaver_SavePortal(*args)
    __swig_destroy__ = _imap.delete_iSaver
    __del__ = lambda self : None;
iSaver_swigregister = _imap.iSaver_swigregister
iSaver_swigregister(iSaver)

def CS_REQUEST_LEVELLOADER ():
  return core.CS_REQUEST_PLUGIN("crystalspace.level.loader", iLoader)



