# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _cspace

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


class csWrapPtr(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csWrapPtr, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csWrapPtr, name)
    def __repr__(self):
        return "<C csWrapPtr instance at %s>" % (self.this,)
    __swig_setmethods__["Ref"] = _cspace.csWrapPtr_Ref_set
    __swig_getmethods__["Ref"] = _cspace.csWrapPtr_Ref_get
    if _newclass:Ref = property(_cspace.csWrapPtr_Ref_get, _cspace.csWrapPtr_Ref_set)
    __swig_setmethods__["VoidPtr"] = _cspace.csWrapPtr_VoidPtr_set
    __swig_getmethods__["VoidPtr"] = _cspace.csWrapPtr_VoidPtr_get
    if _newclass:VoidPtr = property(_cspace.csWrapPtr_VoidPtr_get, _cspace.csWrapPtr_VoidPtr_set)
    __swig_getmethods__["Type"] = _cspace.csWrapPtr_Type_get
    if _newclass:Type = property(_cspace.csWrapPtr_Type_get)
    def __init__(self, *args):
        _swig_setattr(self, csWrapPtr, 'this', _cspace.new_csWrapPtr(*args))
        _swig_setattr(self, csWrapPtr, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csWrapPtr):
        try:
            if self.thisown: destroy(self)
        except: pass

class csWrapPtrPtr(csWrapPtr):
    def __init__(self, this):
        _swig_setattr(self, csWrapPtr, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csWrapPtr, 'thisown', 0)
        _swig_setattr(self, csWrapPtr,self.__class__,csWrapPtr)
_cspace.csWrapPtr_swigregister(csWrapPtrPtr)

CS_VOIDED_PTR = _cspace.CS_VOIDED_PTR
class iBase(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, iBase, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, iBase, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iBase instance at %s>" % (self.this,)
    def IncRef(*args): return _cspace.iBase_IncRef(*args)
    def DecRef(*args): return _cspace.iBase_DecRef(*args)
    def GetRefCount(*args): return _cspace.iBase_GetRefCount(*args)
    def QueryInterface(*args): return _cspace.iBase_QueryInterface(*args)
    __swig_getmethods__["QueryInterfaceSafe"] = lambda x: _cspace.iBase_QueryInterfaceSafe
    if _newclass:QueryInterfaceSafe = staticmethod(_cspace.iBase_QueryInterfaceSafe)
    def AddRefOwner(*args): return _cspace.iBase_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.iBase_RemoveRefOwner(*args)
    def __del__(self, destroy=_cspace.delete_iBase):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iBase_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iBase_scfGetVersion)
    def _DynamicCast(*args): return _cspace.iBase__DynamicCast(*args)

class iBasePtr(iBase):
    def __init__(self, this):
        _swig_setattr(self, iBase, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iBase, 'thisown', 0)
        _swig_setattr(self, iBase,self.__class__,iBase)
_cspace.iBase_swigregister(iBasePtr)
cvar = _cspace.cvar
csArrayItemNotFound = cvar.csArrayItemNotFound

iBase_QueryInterfaceSafe = _cspace.iBase_QueryInterfaceSafe

iBase_scfGetVersion = _cspace.iBase_scfGetVersion

SCF_STATIC_CLASS_CONTEXT = _cspace.SCF_STATIC_CLASS_CONTEXT
class iFactory(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFactory, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFactory, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFactory instance at %s>" % (self.this,)
    def CreateInstance(*args): return _cspace.iFactory_CreateInstance(*args)
    def TryUnload(*args): return _cspace.iFactory_TryUnload(*args)
    def QueryDescription(*args): return _cspace.iFactory_QueryDescription(*args)
    def QueryDependencies(*args): return _cspace.iFactory_QueryDependencies(*args)
    def QueryClassID(*args): return _cspace.iFactory_QueryClassID(*args)
    def QueryModuleName(*args): return _cspace.iFactory_QueryModuleName(*args)
    def __del__(self, destroy=_cspace.delete_iFactory):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iFactory_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iFactory_scfGetVersion)

class iFactoryPtr(iFactory):
    def __init__(self, this):
        _swig_setattr(self, iFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFactory, 'thisown', 0)
        _swig_setattr(self, iFactory,self.__class__,iFactory)
_cspace.iFactory_swigregister(iFactoryPtr)

iFactory_scfGetVersion = _cspace.iFactory_scfGetVersion


scfCompatibleVersion = _cspace.scfCompatibleVersion
class iSCF(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSCF, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSCF, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSCF instance at %s>" % (self.this,)
    def RegisterClasses(*args): return _cspace.iSCF_RegisterClasses(*args)
    def ClassRegistered(*args): return _cspace.iSCF_ClassRegistered(*args)
    def CreateInstance(*args): return _cspace.iSCF_CreateInstance(*args)
    def GetClassDescription(*args): return _cspace.iSCF_GetClassDescription(*args)
    def GetClassDependencies(*args): return _cspace.iSCF_GetClassDependencies(*args)
    def GetPluginMetadata(*args): return _cspace.iSCF_GetPluginMetadata(*args)
    def UnloadUnusedModules(*args): return _cspace.iSCF_UnloadUnusedModules(*args)
    def RegisterClass(*args): return _cspace.iSCF_RegisterClass(*args)
    def RegisterFactoryFunc(*args): return _cspace.iSCF_RegisterFactoryFunc(*args)
    def UnregisterClass(*args): return _cspace.iSCF_UnregisterClass(*args)
    def GetInterfaceName(*args): return _cspace.iSCF_GetInterfaceName(*args)
    def GetInterfaceID(*args): return _cspace.iSCF_GetInterfaceID(*args)
    def Finish(*args): return _cspace.iSCF_Finish(*args)
    def QueryClassList(*args): return _cspace.iSCF_QueryClassList(*args)
    def ScanPluginsPath(*args): return _cspace.iSCF_ScanPluginsPath(*args)
    def RegisterPlugin(*args): return _cspace.iSCF_RegisterPlugin(*args)
    def __del__(self, destroy=_cspace.delete_iSCF):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSCF_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSCF_scfGetVersion)

class iSCFPtr(iSCF):
    def __init__(self, this):
        _swig_setattr(self, iSCF, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSCF, 'thisown', 0)
        _swig_setattr(self, iSCF,self.__class__,iSCF)
_cspace.iSCF_swigregister(iSCFPtr)

iSCF_scfGetVersion = _cspace.iSCF_scfGetVersion

CS_DBGHELP_UNITTEST = _cspace.CS_DBGHELP_UNITTEST
CS_DBGHELP_BENCHMARK = _cspace.CS_DBGHELP_BENCHMARK
CS_DBGHELP_TXTDUMP = _cspace.CS_DBGHELP_TXTDUMP
CS_DBGHELP_GFXDUMP = _cspace.CS_DBGHELP_GFXDUMP
CS_DBGHELP_STATETEST = _cspace.CS_DBGHELP_STATETEST
class iDebugHelper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDebugHelper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDebugHelper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDebugHelper instance at %s>" % (self.this,)
    def GetSupportedTests(*args): return _cspace.iDebugHelper_GetSupportedTests(*args)
    def UnitTest(*args): return _cspace.iDebugHelper_UnitTest(*args)
    def StateTest(*args): return _cspace.iDebugHelper_StateTest(*args)
    def Benchmark(*args): return _cspace.iDebugHelper_Benchmark(*args)
    def Dump(*args): return _cspace.iDebugHelper_Dump(*args)
    def DebugCommand(*args): return _cspace.iDebugHelper_DebugCommand(*args)
    def __del__(self, destroy=_cspace.delete_iDebugHelper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iDebugHelper_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iDebugHelper_scfGetVersion)

class iDebugHelperPtr(iDebugHelper):
    def __init__(self, this):
        _swig_setattr(self, iDebugHelper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDebugHelper, 'thisown', 0)
        _swig_setattr(self, iDebugHelper,self.__class__,iDebugHelper)
_cspace.iDebugHelper_swigregister(iDebugHelperPtr)

iDebugHelper_scfGetVersion = _cspace.iDebugHelper_scfGetVersion

class csColor(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csColor, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csColor, name)
    def __repr__(self):
        return "<C csColor instance at %s>" % (self.this,)
    __swig_setmethods__["red"] = _cspace.csColor_red_set
    __swig_getmethods__["red"] = _cspace.csColor_red_get
    if _newclass:red = property(_cspace.csColor_red_get, _cspace.csColor_red_set)
    __swig_setmethods__["green"] = _cspace.csColor_green_set
    __swig_getmethods__["green"] = _cspace.csColor_green_get
    if _newclass:green = property(_cspace.csColor_green_get, _cspace.csColor_green_set)
    __swig_setmethods__["blue"] = _cspace.csColor_blue_set
    __swig_getmethods__["blue"] = _cspace.csColor_blue_get
    if _newclass:blue = property(_cspace.csColor_blue_get, _cspace.csColor_blue_set)
    def __init__(self, *args):
        _swig_setattr(self, csColor, 'this', _cspace.new_csColor(*args))
        _swig_setattr(self, csColor, 'thisown', 1)
    def Set(*args): return _cspace.csColor_Set(*args)
    def Clamp(*args): return _cspace.csColor_Clamp(*args)
    def ClampDown(*args): return _cspace.csColor_ClampDown(*args)
    def assign(*args): return _cspace.csColor_assign(*args)
    def __imul__(*args): return _cspace.csColor___imul__(*args)
    def __iadd__(*args): return _cspace.csColor___iadd__(*args)
    def __isub__(*args): return _cspace.csColor___isub__(*args)
    def __eq__(*args): return _cspace.csColor___eq__(*args)
    def __ne__(*args): return _cspace.csColor___ne__(*args)
    def Add(*args): return _cspace.csColor_Add(*args)
    def Subtract(*args): return _cspace.csColor_Subtract(*args)
    def __add__(*args): return _cspace.csColor___add__(*args)
    def __sub__(*args): return _cspace.csColor___sub__(*args)
    def __mul__(*args): return _cspace.csColor___mul__(*args)
    def __rmul__(*args): return _cspace.csColor___rmul__(*args)
    def __del__(self, destroy=_cspace.delete_csColor):
        try:
            if self.thisown: destroy(self)
        except: pass

class csColorPtr(csColor):
    def __init__(self, this):
        _swig_setattr(self, csColor, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csColor, 'thisown', 0)
        _swig_setattr(self, csColor,self.__class__,csColor)
_cspace.csColor_swigregister(csColorPtr)

class csColor4(csColor):
    __swig_setmethods__ = {}
    for _s in [csColor]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csColor4, name, value)
    __swig_getmethods__ = {}
    for _s in [csColor]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csColor4, name)
    def __repr__(self):
        return "<C csColor4 instance at %s>" % (self.this,)
    __swig_setmethods__["alpha"] = _cspace.csColor4_alpha_set
    __swig_getmethods__["alpha"] = _cspace.csColor4_alpha_get
    if _newclass:alpha = property(_cspace.csColor4_alpha_get, _cspace.csColor4_alpha_set)
    def __init__(self, *args):
        _swig_setattr(self, csColor4, 'this', _cspace.new_csColor4(*args))
        _swig_setattr(self, csColor4, 'thisown', 1)
    def Set(*args): return _cspace.csColor4_Set(*args)
    def assign(*args): return _cspace.csColor4_assign(*args)
    def __imul__(*args): return _cspace.csColor4___imul__(*args)
    def __iadd__(*args): return _cspace.csColor4___iadd__(*args)
    def __isub__(*args): return _cspace.csColor4___isub__(*args)
    def __eq__(*args): return _cspace.csColor4___eq__(*args)
    def __ne__(*args): return _cspace.csColor4___ne__(*args)
    def __del__(self, destroy=_cspace.delete_csColor4):
        try:
            if self.thisown: destroy(self)
        except: pass

class csColor4Ptr(csColor4):
    def __init__(self, this):
        _swig_setattr(self, csColor4, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csColor4, 'thisown', 0)
        _swig_setattr(self, csColor4,self.__class__,csColor4)
_cspace.csColor4_swigregister(csColor4Ptr)

class csCommandLineHelper(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csCommandLineHelper, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csCommandLineHelper, name)
    def __repr__(self):
        return "<C csCommandLineHelper instance at %s>" % (self.this,)
    __swig_getmethods__["Help"] = lambda x: _cspace.csCommandLineHelper_Help
    if _newclass:Help = staticmethod(_cspace.csCommandLineHelper_Help)
    __swig_getmethods__["CheckHelp"] = lambda x: _cspace.csCommandLineHelper_CheckHelp
    if _newclass:CheckHelp = staticmethod(_cspace.csCommandLineHelper_CheckHelp)
    def __init__(self, *args):
        _swig_setattr(self, csCommandLineHelper, 'this', _cspace.new_csCommandLineHelper(*args))
        _swig_setattr(self, csCommandLineHelper, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csCommandLineHelper):
        try:
            if self.thisown: destroy(self)
        except: pass

class csCommandLineHelperPtr(csCommandLineHelper):
    def __init__(self, this):
        _swig_setattr(self, csCommandLineHelper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csCommandLineHelper, 'thisown', 0)
        _swig_setattr(self, csCommandLineHelper,self.__class__,csCommandLineHelper)
_cspace.csCommandLineHelper_swigregister(csCommandLineHelperPtr)

csCommandLineHelper_Help = _cspace.csCommandLineHelper_Help

csCommandLineHelper_CheckHelp = _cspace.csCommandLineHelper_CheckHelp

class csStringSetIterator(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csStringSetIterator, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csStringSetIterator, name)
    def __repr__(self):
        return "<C csStringSetIterator instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csStringSetIterator, 'this', _cspace.new_csStringSetIterator(*args))
        _swig_setattr(self, csStringSetIterator, 'thisown', 1)
    def HasNext(*args): return _cspace.csStringSetIterator_HasNext(*args)
    def Next(*args): return _cspace.csStringSetIterator_Next(*args)
    def __del__(self, destroy=_cspace.delete_csStringSetIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class csStringSetIteratorPtr(csStringSetIterator):
    def __init__(self, this):
        _swig_setattr(self, csStringSetIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csStringSetIterator, 'thisown', 0)
        _swig_setattr(self, csStringSetIterator,self.__class__,csStringSetIterator)
_cspace.csStringSetIterator_swigregister(csStringSetIteratorPtr)

class csStringSet(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csStringSet, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csStringSet, name)
    def __repr__(self):
        return "<C csStringSet instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csStringSet, 'this', _cspace.new_csStringSet(*args))
        _swig_setattr(self, csStringSet, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csStringSet):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Request(*args): return _cspace.csStringSet_Request(*args)
    def Contains(*args): return _cspace.csStringSet_Contains(*args)
    def Clear(*args): return _cspace.csStringSet_Clear(*args)

class csStringSetPtr(csStringSet):
    def __init__(self, this):
        _swig_setattr(self, csStringSet, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csStringSet, 'thisown', 0)
        _swig_setattr(self, csStringSet,self.__class__,csStringSet)
_cspace.csStringSet_swigregister(csStringSetPtr)

class iString(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iString, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iString, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iString instance at %s>" % (self.this,)
    def GetData(*args): return _cspace.iString_GetData(*args)
    def __ne__(*args): return _cspace.iString___ne__(*args)
    def __del__(self, destroy=_cspace.delete_iString):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iString_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iString_scfGetVersion)
    def __getitem__(*args): return _cspace.iString___getitem__(*args)
    def __setitem__(*args): return _cspace.iString___setitem__(*args)

class iStringPtr(iString):
    def __init__(self, this):
        _swig_setattr(self, iString, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iString, 'thisown', 0)
        _swig_setattr(self, iString,self.__class__,iString)
_cspace.iString_swigregister(iStringPtr)

iString_scfGetVersion = _cspace.iString_scfGetVersion

class csString(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csString, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csString, name)
    def __repr__(self):
        return "<C csString instance at %s>" % (self.this,)
    def GetData(*args): return _cspace.csString_GetData(*args)
    def GetDataSafe(*args): return _cspace.csString_GetDataSafe(*args)
    def Length(*args): return _cspace.csString_Length(*args)
    def IsEmpty(*args): return _cspace.csString_IsEmpty(*args)
    def FindStr(*args): return _cspace.csString_FindStr(*args)
    def FindReplace(*args): return _cspace.csString_FindReplace(*args)
    def StartsWith(*args): return _cspace.csString_StartsWith(*args)
    def __init__(self, *args):
        _swig_setattr(self, csString, 'this', _cspace.new_csString(*args))
        _swig_setattr(self, csString, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csString):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __lt__(*args): return _cspace.csString___lt__(*args)
    def __gt__(*args): return _cspace.csString___gt__(*args)
    def __getitem__(*args): return _cspace.csString___getitem__(*args)
    def __setitem__(*args): return _cspace.csString___setitem__(*args)
    def __delitem__(*args): return _cspace.csString___delitem__(*args)

class csStringPtr(csString):
    def __init__(self, this):
        _swig_setattr(self, csString, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csString, 'thisown', 0)
        _swig_setattr(self, csString,self.__class__,csString)
_cspace.csString_swigregister(csStringPtr)

class csVector2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csVector2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csVector2, name)
    def __repr__(self):
        return "<C csVector2 instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cspace.csVector2_x_set
    __swig_getmethods__["x"] = _cspace.csVector2_x_get
    if _newclass:x = property(_cspace.csVector2_x_get, _cspace.csVector2_x_set)
    __swig_setmethods__["y"] = _cspace.csVector2_y_set
    __swig_getmethods__["y"] = _cspace.csVector2_y_get
    if _newclass:y = property(_cspace.csVector2_y_get, _cspace.csVector2_y_set)
    def __init__(self, *args):
        _swig_setattr(self, csVector2, 'this', _cspace.new_csVector2(*args))
        _swig_setattr(self, csVector2, 'thisown', 1)
    def Description(*args): return _cspace.csVector2_Description(*args)
    def Set(*args): return _cspace.csVector2_Set(*args)
    def Get(*args): return _cspace.csVector2_Get(*args)
    def Norm(*args): return _cspace.csVector2_Norm(*args)
    def SquaredNorm(*args): return _cspace.csVector2_SquaredNorm(*args)
    def Rotate(*args): return _cspace.csVector2_Rotate(*args)
    def IsLeft(*args): return _cspace.csVector2_IsLeft(*args)
    def __iadd__(*args): return _cspace.csVector2___iadd__(*args)
    def __isub__(*args): return _cspace.csVector2___isub__(*args)
    def __imul__(*args): return _cspace.csVector2___imul__(*args)
    def __idiv__(*args): return _cspace.csVector2___idiv__(*args)
    def __pos__(*args): return _cspace.csVector2___pos__(*args)
    def __neg__(*args): return _cspace.csVector2___neg__(*args)
    def __add__(*args): return _cspace.csVector2___add__(*args)
    def __sub__(*args): return _cspace.csVector2___sub__(*args)
    def __mul__(*args): return _cspace.csVector2___mul__(*args)
    def __div__(*args): return _cspace.csVector2___div__(*args)
    def __eq__(*args): return _cspace.csVector2___eq__(*args)
    def __ne__(*args): return _cspace.csVector2___ne__(*args)
    def __lt__(*args): return _cspace.csVector2___lt__(*args)
    def __gt__(*args): return _cspace.csVector2___gt__(*args)
    def __rmul__(*args): return _cspace.csVector2___rmul__(*args)
    def __abs__(*args): return _cspace.csVector2___abs__(*args)
    def __getitem__(*args): return _cspace.csVector2___getitem__(*args)
    def __setitem__(*args): return _cspace.csVector2___setitem__(*args)
    def __del__(self, destroy=_cspace.delete_csVector2):
        try:
            if self.thisown: destroy(self)
        except: pass

class csVector2Ptr(csVector2):
    def __init__(self, this):
        _swig_setattr(self, csVector2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csVector2, 'thisown', 0)
        _swig_setattr(self, csVector2,self.__class__,csVector2)
_cspace.csVector2_swigregister(csVector2Ptr)

CS_AXIS_NONE = _cspace.CS_AXIS_NONE
CS_AXIS_X = _cspace.CS_AXIS_X
CS_AXIS_Y = _cspace.CS_AXIS_Y
CS_AXIS_Z = _cspace.CS_AXIS_Z
class csVector3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csVector3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csVector3, name)
    def __repr__(self):
        return "<C csVector3 instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cspace.csVector3_x_set
    __swig_getmethods__["x"] = _cspace.csVector3_x_get
    if _newclass:x = property(_cspace.csVector3_x_get, _cspace.csVector3_x_set)
    __swig_setmethods__["y"] = _cspace.csVector3_y_set
    __swig_getmethods__["y"] = _cspace.csVector3_y_get
    if _newclass:y = property(_cspace.csVector3_y_get, _cspace.csVector3_y_set)
    __swig_setmethods__["z"] = _cspace.csVector3_z_set
    __swig_getmethods__["z"] = _cspace.csVector3_z_get
    if _newclass:z = property(_cspace.csVector3_z_get, _cspace.csVector3_z_set)
    def __init__(self, *args):
        _swig_setattr(self, csVector3, 'this', _cspace.new_csVector3(*args))
        _swig_setattr(self, csVector3, 'thisown', 1)
    def Description(*args): return _cspace.csVector3_Description(*args)
    def Cross(*args): return _cspace.csVector3_Cross(*args)
    def __iadd__(*args): return _cspace.csVector3___iadd__(*args)
    def __isub__(*args): return _cspace.csVector3___isub__(*args)
    def __pos__(*args): return _cspace.csVector3___pos__(*args)
    def __neg__(*args): return _cspace.csVector3___neg__(*args)
    def Set(*args): return _cspace.csVector3_Set(*args)
    def Get(*args): return _cspace.csVector3_Get(*args)
    def Norm(*args): return _cspace.csVector3_Norm(*args)
    def SquaredNorm(*args): return _cspace.csVector3_SquaredNorm(*args)
    def Unit(*args): return _cspace.csVector3_Unit(*args)
    def Normalize(*args): return _cspace.csVector3_Normalize(*args)
    def IsZero(*args): return _cspace.csVector3_IsZero(*args)
    def __add__(*args): return _cspace.csVector3___add__(*args)
    def __sub__(*args): return _cspace.csVector3___sub__(*args)
    def __mul__(*args): return _cspace.csVector3___mul__(*args)
    def __eq__(*args): return _cspace.csVector3___eq__(*args)
    def __ne__(*args): return _cspace.csVector3___ne__(*args)
    def __lt__(*args): return _cspace.csVector3___lt__(*args)
    def __gt__(*args): return _cspace.csVector3___gt__(*args)
    def __imul__(*args): return _cspace.csVector3___imul__(*args)
    def __idiv__(*args): return _cspace.csVector3___idiv__(*args)
    def __div__(*args): return _cspace.csVector3___div__(*args)
    def project(*args): return _cspace.csVector3_project(*args)
    def __rmul__(*args): return _cspace.csVector3___rmul__(*args)
    def __abs__(*args): return _cspace.csVector3___abs__(*args)
    def __getitem__(*args): return _cspace.csVector3___getitem__(*args)
    def __setitem__(*args): return _cspace.csVector3___setitem__(*args)
    def __nonzero__(*args): return _cspace.csVector3___nonzero__(*args)
    def __del__(self, destroy=_cspace.delete_csVector3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csVector3Ptr(csVector3):
    def __init__(self, this):
        _swig_setattr(self, csVector3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csVector3, 'thisown', 0)
        _swig_setattr(self, csVector3,self.__class__,csVector3)
_cspace.csVector3_swigregister(csVector3Ptr)

class csMatrix2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csMatrix2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csMatrix2, name)
    def __repr__(self):
        return "<C csMatrix2 instance at %s>" % (self.this,)
    __swig_setmethods__["m11"] = _cspace.csMatrix2_m11_set
    __swig_getmethods__["m11"] = _cspace.csMatrix2_m11_get
    if _newclass:m11 = property(_cspace.csMatrix2_m11_get, _cspace.csMatrix2_m11_set)
    __swig_setmethods__["m12"] = _cspace.csMatrix2_m12_set
    __swig_getmethods__["m12"] = _cspace.csMatrix2_m12_get
    if _newclass:m12 = property(_cspace.csMatrix2_m12_get, _cspace.csMatrix2_m12_set)
    __swig_setmethods__["m21"] = _cspace.csMatrix2_m21_set
    __swig_getmethods__["m21"] = _cspace.csMatrix2_m21_get
    if _newclass:m21 = property(_cspace.csMatrix2_m21_get, _cspace.csMatrix2_m21_set)
    __swig_setmethods__["m22"] = _cspace.csMatrix2_m22_set
    __swig_getmethods__["m22"] = _cspace.csMatrix2_m22_get
    if _newclass:m22 = property(_cspace.csMatrix2_m22_get, _cspace.csMatrix2_m22_set)
    def __init__(self, *args):
        _swig_setattr(self, csMatrix2, 'this', _cspace.new_csMatrix2(*args))
        _swig_setattr(self, csMatrix2, 'thisown', 1)
    def Row1(*args): return _cspace.csMatrix2_Row1(*args)
    def Row2(*args): return _cspace.csMatrix2_Row2(*args)
    def Col1(*args): return _cspace.csMatrix2_Col1(*args)
    def Col2(*args): return _cspace.csMatrix2_Col2(*args)
    def Set(*args): return _cspace.csMatrix2_Set(*args)
    def __iadd__(*args): return _cspace.csMatrix2___iadd__(*args)
    def __isub__(*args): return _cspace.csMatrix2___isub__(*args)
    def __imul__(*args): return _cspace.csMatrix2___imul__(*args)
    def __idiv__(*args): return _cspace.csMatrix2___idiv__(*args)
    def __pos__(*args): return _cspace.csMatrix2___pos__(*args)
    def __neg__(*args): return _cspace.csMatrix2___neg__(*args)
    def Transpose(*args): return _cspace.csMatrix2_Transpose(*args)
    def GetTranspose(*args): return _cspace.csMatrix2_GetTranspose(*args)
    def GetInverse(*args): return _cspace.csMatrix2_GetInverse(*args)
    def Invert(*args): return _cspace.csMatrix2_Invert(*args)
    def Determinant(*args): return _cspace.csMatrix2_Determinant(*args)
    def Identity(*args): return _cspace.csMatrix2_Identity(*args)
    def __del__(self, destroy=_cspace.delete_csMatrix2):
        try:
            if self.thisown: destroy(self)
        except: pass

class csMatrix2Ptr(csMatrix2):
    def __init__(self, this):
        _swig_setattr(self, csMatrix2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csMatrix2, 'thisown', 0)
        _swig_setattr(self, csMatrix2,self.__class__,csMatrix2)
_cspace.csMatrix2_swigregister(csMatrix2Ptr)

class csMatrix3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csMatrix3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csMatrix3, name)
    def __repr__(self):
        return "<C csMatrix3 instance at %s>" % (self.this,)
    __swig_setmethods__["m11"] = _cspace.csMatrix3_m11_set
    __swig_getmethods__["m11"] = _cspace.csMatrix3_m11_get
    if _newclass:m11 = property(_cspace.csMatrix3_m11_get, _cspace.csMatrix3_m11_set)
    __swig_setmethods__["m12"] = _cspace.csMatrix3_m12_set
    __swig_getmethods__["m12"] = _cspace.csMatrix3_m12_get
    if _newclass:m12 = property(_cspace.csMatrix3_m12_get, _cspace.csMatrix3_m12_set)
    __swig_setmethods__["m13"] = _cspace.csMatrix3_m13_set
    __swig_getmethods__["m13"] = _cspace.csMatrix3_m13_get
    if _newclass:m13 = property(_cspace.csMatrix3_m13_get, _cspace.csMatrix3_m13_set)
    __swig_setmethods__["m21"] = _cspace.csMatrix3_m21_set
    __swig_getmethods__["m21"] = _cspace.csMatrix3_m21_get
    if _newclass:m21 = property(_cspace.csMatrix3_m21_get, _cspace.csMatrix3_m21_set)
    __swig_setmethods__["m22"] = _cspace.csMatrix3_m22_set
    __swig_getmethods__["m22"] = _cspace.csMatrix3_m22_get
    if _newclass:m22 = property(_cspace.csMatrix3_m22_get, _cspace.csMatrix3_m22_set)
    __swig_setmethods__["m23"] = _cspace.csMatrix3_m23_set
    __swig_getmethods__["m23"] = _cspace.csMatrix3_m23_get
    if _newclass:m23 = property(_cspace.csMatrix3_m23_get, _cspace.csMatrix3_m23_set)
    __swig_setmethods__["m31"] = _cspace.csMatrix3_m31_set
    __swig_getmethods__["m31"] = _cspace.csMatrix3_m31_get
    if _newclass:m31 = property(_cspace.csMatrix3_m31_get, _cspace.csMatrix3_m31_set)
    __swig_setmethods__["m32"] = _cspace.csMatrix3_m32_set
    __swig_getmethods__["m32"] = _cspace.csMatrix3_m32_get
    if _newclass:m32 = property(_cspace.csMatrix3_m32_get, _cspace.csMatrix3_m32_set)
    __swig_setmethods__["m33"] = _cspace.csMatrix3_m33_set
    __swig_getmethods__["m33"] = _cspace.csMatrix3_m33_get
    if _newclass:m33 = property(_cspace.csMatrix3_m33_get, _cspace.csMatrix3_m33_set)
    def __init__(self, *args):
        _swig_setattr(self, csMatrix3, 'this', _cspace.new_csMatrix3(*args))
        _swig_setattr(self, csMatrix3, 'thisown', 1)
    def Row1(*args): return _cspace.csMatrix3_Row1(*args)
    def Row2(*args): return _cspace.csMatrix3_Row2(*args)
    def Row3(*args): return _cspace.csMatrix3_Row3(*args)
    def Col1(*args): return _cspace.csMatrix3_Col1(*args)
    def Col2(*args): return _cspace.csMatrix3_Col2(*args)
    def Col3(*args): return _cspace.csMatrix3_Col3(*args)
    def Set(*args): return _cspace.csMatrix3_Set(*args)
    def assign(*args): return _cspace.csMatrix3_assign(*args)
    def __iadd__(*args): return _cspace.csMatrix3___iadd__(*args)
    def __isub__(*args): return _cspace.csMatrix3___isub__(*args)
    def __idiv__(*args): return _cspace.csMatrix3___idiv__(*args)
    def __pos__(*args): return _cspace.csMatrix3___pos__(*args)
    def __neg__(*args): return _cspace.csMatrix3___neg__(*args)
    def Transpose(*args): return _cspace.csMatrix3_Transpose(*args)
    def GetTranspose(*args): return _cspace.csMatrix3_GetTranspose(*args)
    def GetInverse(*args): return _cspace.csMatrix3_GetInverse(*args)
    def Invert(*args): return _cspace.csMatrix3_Invert(*args)
    def Determinant(*args): return _cspace.csMatrix3_Determinant(*args)
    def Identity(*args): return _cspace.csMatrix3_Identity(*args)
    def IsIdentity(*args): return _cspace.csMatrix3_IsIdentity(*args)
    def __add__(*args): return _cspace.csMatrix3___add__(*args)
    def __sub__(*args): return _cspace.csMatrix3___sub__(*args)
    def __div__(*args): return _cspace.csMatrix3___div__(*args)
    def __eq__(*args): return _cspace.csMatrix3___eq__(*args)
    def __ne__(*args): return _cspace.csMatrix3___ne__(*args)
    def __lt__(*args): return _cspace.csMatrix3___lt__(*args)
    def __mul__(*args): return _cspace.csMatrix3___mul__(*args)
    def __imul__(*args): return _cspace.csMatrix3___imul__(*args)
    def __rmul__(*args): return _cspace.csMatrix3___rmul__(*args)
    def __del__(self, destroy=_cspace.delete_csMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csMatrix3Ptr(csMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csMatrix3, 'thisown', 0)
        _swig_setattr(self, csMatrix3,self.__class__,csMatrix3)
_cspace.csMatrix3_swigregister(csMatrix3Ptr)

class csXRotMatrix3(csMatrix3):
    __swig_setmethods__ = {}
    for _s in [csMatrix3]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csXRotMatrix3, name, value)
    __swig_getmethods__ = {}
    for _s in [csMatrix3]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csXRotMatrix3, name)
    def __repr__(self):
        return "<C csXRotMatrix3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csXRotMatrix3, 'this', _cspace.new_csXRotMatrix3(*args))
        _swig_setattr(self, csXRotMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csXRotMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csXRotMatrix3Ptr(csXRotMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csXRotMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csXRotMatrix3, 'thisown', 0)
        _swig_setattr(self, csXRotMatrix3,self.__class__,csXRotMatrix3)
_cspace.csXRotMatrix3_swigregister(csXRotMatrix3Ptr)

class csYRotMatrix3(csMatrix3):
    __swig_setmethods__ = {}
    for _s in [csMatrix3]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csYRotMatrix3, name, value)
    __swig_getmethods__ = {}
    for _s in [csMatrix3]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csYRotMatrix3, name)
    def __repr__(self):
        return "<C csYRotMatrix3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csYRotMatrix3, 'this', _cspace.new_csYRotMatrix3(*args))
        _swig_setattr(self, csYRotMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csYRotMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csYRotMatrix3Ptr(csYRotMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csYRotMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csYRotMatrix3, 'thisown', 0)
        _swig_setattr(self, csYRotMatrix3,self.__class__,csYRotMatrix3)
_cspace.csYRotMatrix3_swigregister(csYRotMatrix3Ptr)

class csZRotMatrix3(csMatrix3):
    __swig_setmethods__ = {}
    for _s in [csMatrix3]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csZRotMatrix3, name, value)
    __swig_getmethods__ = {}
    for _s in [csMatrix3]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csZRotMatrix3, name)
    def __repr__(self):
        return "<C csZRotMatrix3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csZRotMatrix3, 'this', _cspace.new_csZRotMatrix3(*args))
        _swig_setattr(self, csZRotMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csZRotMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csZRotMatrix3Ptr(csZRotMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csZRotMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csZRotMatrix3, 'thisown', 0)
        _swig_setattr(self, csZRotMatrix3,self.__class__,csZRotMatrix3)
_cspace.csZRotMatrix3_swigregister(csZRotMatrix3Ptr)

class csXScaleMatrix3(csMatrix3):
    __swig_setmethods__ = {}
    for _s in [csMatrix3]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csXScaleMatrix3, name, value)
    __swig_getmethods__ = {}
    for _s in [csMatrix3]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csXScaleMatrix3, name)
    def __repr__(self):
        return "<C csXScaleMatrix3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csXScaleMatrix3, 'this', _cspace.new_csXScaleMatrix3(*args))
        _swig_setattr(self, csXScaleMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csXScaleMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csXScaleMatrix3Ptr(csXScaleMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csXScaleMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csXScaleMatrix3, 'thisown', 0)
        _swig_setattr(self, csXScaleMatrix3,self.__class__,csXScaleMatrix3)
_cspace.csXScaleMatrix3_swigregister(csXScaleMatrix3Ptr)

class csYScaleMatrix3(csMatrix3):
    __swig_setmethods__ = {}
    for _s in [csMatrix3]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csYScaleMatrix3, name, value)
    __swig_getmethods__ = {}
    for _s in [csMatrix3]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csYScaleMatrix3, name)
    def __repr__(self):
        return "<C csYScaleMatrix3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csYScaleMatrix3, 'this', _cspace.new_csYScaleMatrix3(*args))
        _swig_setattr(self, csYScaleMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csYScaleMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csYScaleMatrix3Ptr(csYScaleMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csYScaleMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csYScaleMatrix3, 'thisown', 0)
        _swig_setattr(self, csYScaleMatrix3,self.__class__,csYScaleMatrix3)
_cspace.csYScaleMatrix3_swigregister(csYScaleMatrix3Ptr)

class csZScaleMatrix3(csMatrix3):
    __swig_setmethods__ = {}
    for _s in [csMatrix3]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csZScaleMatrix3, name, value)
    __swig_getmethods__ = {}
    for _s in [csMatrix3]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csZScaleMatrix3, name)
    def __repr__(self):
        return "<C csZScaleMatrix3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csZScaleMatrix3, 'this', _cspace.new_csZScaleMatrix3(*args))
        _swig_setattr(self, csZScaleMatrix3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csZScaleMatrix3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csZScaleMatrix3Ptr(csZScaleMatrix3):
    def __init__(self, this):
        _swig_setattr(self, csZScaleMatrix3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csZScaleMatrix3, 'thisown', 0)
        _swig_setattr(self, csZScaleMatrix3,self.__class__,csZScaleMatrix3)
_cspace.csZScaleMatrix3_swigregister(csZScaleMatrix3Ptr)

class csTransform(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csTransform, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csTransform, name)
    def __repr__(self):
        return "<C csTransform instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csTransform, 'this', _cspace.new_csTransform(*args))
        _swig_setattr(self, csTransform, 'thisown', 1)
    def Identity(*args): return _cspace.csTransform_Identity(*args)
    def IsIdentity(*args): return _cspace.csTransform_IsIdentity(*args)
    def GetO2T(*args): return _cspace.csTransform_GetO2T(*args)
    def GetO2TTranslation(*args): return _cspace.csTransform_GetO2TTranslation(*args)
    def GetOrigin(*args): return _cspace.csTransform_GetOrigin(*args)
    def SetO2T(*args): return _cspace.csTransform_SetO2T(*args)
    def SetO2TTranslation(*args): return _cspace.csTransform_SetO2TTranslation(*args)
    def SetOrigin(*args): return _cspace.csTransform_SetOrigin(*args)
    def Translate(*args): return _cspace.csTransform_Translate(*args)
    def Other2ThisRelative(*args): return _cspace.csTransform_Other2ThisRelative(*args)
    def Other2This(*args): return _cspace.csTransform_Other2This(*args)
    __swig_getmethods__["GetReflect"] = lambda x: _cspace.csTransform_GetReflect
    if _newclass:GetReflect = staticmethod(_cspace.csTransform_GetReflect)
    def __mul__(*args): return _cspace.csTransform___mul__(*args)
    def __rmul__(*args): return _cspace.csTransform___rmul__(*args)
    def __del__(self, destroy=_cspace.delete_csTransform):
        try:
            if self.thisown: destroy(self)
        except: pass

class csTransformPtr(csTransform):
    def __init__(self, this):
        _swig_setattr(self, csTransform, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csTransform, 'thisown', 0)
        _swig_setattr(self, csTransform,self.__class__,csTransform)
_cspace.csTransform_swigregister(csTransformPtr)

csTransform_GetReflect = _cspace.csTransform_GetReflect

class csReversibleTransform(csTransform):
    __swig_setmethods__ = {}
    for _s in [csTransform]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csReversibleTransform, name, value)
    __swig_getmethods__ = {}
    for _s in [csTransform]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csReversibleTransform, name)
    def __repr__(self):
        return "<C csReversibleTransform instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csReversibleTransform, 'this', _cspace.new_csReversibleTransform(*args))
        _swig_setattr(self, csReversibleTransform, 'thisown', 1)
    def GetT2O(*args): return _cspace.csReversibleTransform_GetT2O(*args)
    def GetT2OTranslation(*args): return _cspace.csReversibleTransform_GetT2OTranslation(*args)
    def GetInverse(*args): return _cspace.csReversibleTransform_GetInverse(*args)
    def SetO2T(*args): return _cspace.csReversibleTransform_SetO2T(*args)
    def SetT2O(*args): return _cspace.csReversibleTransform_SetT2O(*args)
    def This2OtherRelative(*args): return _cspace.csReversibleTransform_This2OtherRelative(*args)
    def This2Other(*args): return _cspace.csReversibleTransform_This2Other(*args)
    def RotateOther(*args): return _cspace.csReversibleTransform_RotateOther(*args)
    def RotateThis(*args): return _cspace.csReversibleTransform_RotateThis(*args)
    def LookAt(*args): return _cspace.csReversibleTransform_LookAt(*args)
    def __imul__(*args): return _cspace.csReversibleTransform___imul__(*args)
    def __mul__(*args): return _cspace.csReversibleTransform___mul__(*args)
    def __idiv__(*args): return _cspace.csReversibleTransform___idiv__(*args)
    def __div__(*args): return _cspace.csReversibleTransform___div__(*args)
    def __del__(self, destroy=_cspace.delete_csReversibleTransform):
        try:
            if self.thisown: destroy(self)
        except: pass

class csReversibleTransformPtr(csReversibleTransform):
    def __init__(self, this):
        _swig_setattr(self, csReversibleTransform, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csReversibleTransform, 'thisown', 0)
        _swig_setattr(self, csReversibleTransform,self.__class__,csReversibleTransform)
_cspace.csReversibleTransform_swigregister(csReversibleTransformPtr)

class csOrthoTransform(csReversibleTransform):
    __swig_setmethods__ = {}
    for _s in [csReversibleTransform]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csOrthoTransform, name, value)
    __swig_getmethods__ = {}
    for _s in [csReversibleTransform]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csOrthoTransform, name)
    def __repr__(self):
        return "<C csOrthoTransform instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csOrthoTransform, 'this', _cspace.new_csOrthoTransform(*args))
        _swig_setattr(self, csOrthoTransform, 'thisown', 1)
    def SetO2T(*args): return _cspace.csOrthoTransform_SetO2T(*args)
    def SetT2O(*args): return _cspace.csOrthoTransform_SetT2O(*args)
    def __del__(self, destroy=_cspace.delete_csOrthoTransform):
        try:
            if self.thisown: destroy(self)
        except: pass

class csOrthoTransformPtr(csOrthoTransform):
    def __init__(self, this):
        _swig_setattr(self, csOrthoTransform, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csOrthoTransform, 'thisown', 0)
        _swig_setattr(self, csOrthoTransform,self.__class__,csOrthoTransform)
_cspace.csOrthoTransform_swigregister(csOrthoTransformPtr)

class csSphere(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSphere, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSphere, name)
    def __repr__(self):
        return "<C csSphere instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csSphere, 'this', _cspace.new_csSphere(*args))
        _swig_setattr(self, csSphere, 'thisown', 1)
    def GetCenter(*args): return _cspace.csSphere_GetCenter(*args)
    def SetCenter(*args): return _cspace.csSphere_SetCenter(*args)
    def GetRadius(*args): return _cspace.csSphere_GetRadius(*args)
    def SetRadius(*args): return _cspace.csSphere_SetRadius(*args)
    def Union(*args): return _cspace.csSphere_Union(*args)
    def __iadd__(*args): return _cspace.csSphere___iadd__(*args)
    def __imul__(*args): return _cspace.csSphere___imul__(*args)
    def __div__(*args): return _cspace.csSphere___div__(*args)
    def __del__(self, destroy=_cspace.delete_csSphere):
        try:
            if self.thisown: destroy(self)
        except: pass

class csSpherePtr(csSphere):
    def __init__(self, this):
        _swig_setattr(self, csSphere, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSphere, 'thisown', 0)
        _swig_setattr(self, csSphere,self.__class__,csSphere)
_cspace.csSphere_swigregister(csSpherePtr)

CS_POLY_IN = _cspace.CS_POLY_IN
CS_POLY_ON = _cspace.CS_POLY_ON
CS_POLY_OUT = _cspace.CS_POLY_OUT
class csPlane2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPlane2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPlane2, name)
    def __repr__(self):
        return "<C csPlane2 instance at %s>" % (self.this,)
    __swig_setmethods__["norm"] = _cspace.csPlane2_norm_set
    __swig_getmethods__["norm"] = _cspace.csPlane2_norm_get
    if _newclass:norm = property(_cspace.csPlane2_norm_get, _cspace.csPlane2_norm_set)
    __swig_setmethods__["CC"] = _cspace.csPlane2_CC_set
    __swig_getmethods__["CC"] = _cspace.csPlane2_CC_get
    if _newclass:CC = property(_cspace.csPlane2_CC_get, _cspace.csPlane2_CC_set)
    def __init__(self, *args):
        _swig_setattr(self, csPlane2, 'this', _cspace.new_csPlane2(*args))
        _swig_setattr(self, csPlane2, 'thisown', 1)
    def Normal(*args): return _cspace.csPlane2_Normal(*args)
    def GetNormal(*args): return _cspace.csPlane2_GetNormal(*args)
    def A(*args): return _cspace.csPlane2_A(*args)
    def B(*args): return _cspace.csPlane2_B(*args)
    def C(*args): return _cspace.csPlane2_C(*args)
    def Set(*args): return _cspace.csPlane2_Set(*args)
    __swig_getmethods__["Classify"] = lambda x: _cspace.csPlane2_Classify
    if _newclass:Classify = staticmethod(_cspace.csPlane2_Classify)
    def Distance(*args): return _cspace.csPlane2_Distance(*args)
    def SquaredDistance(*args): return _cspace.csPlane2_SquaredDistance(*args)
    def Invert(*args): return _cspace.csPlane2_Invert(*args)
    def Normalize(*args): return _cspace.csPlane2_Normalize(*args)
    def __del__(self, destroy=_cspace.delete_csPlane2):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPlane2Ptr(csPlane2):
    def __init__(self, this):
        _swig_setattr(self, csPlane2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPlane2, 'thisown', 0)
        _swig_setattr(self, csPlane2,self.__class__,csPlane2)
_cspace.csPlane2_swigregister(csPlane2Ptr)

csPlane2_Classify = _cspace.csPlane2_Classify

class csPlane3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPlane3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPlane3, name)
    def __repr__(self):
        return "<C csPlane3 instance at %s>" % (self.this,)
    __swig_setmethods__["norm"] = _cspace.csPlane3_norm_set
    __swig_getmethods__["norm"] = _cspace.csPlane3_norm_get
    if _newclass:norm = property(_cspace.csPlane3_norm_get, _cspace.csPlane3_norm_set)
    __swig_setmethods__["DD"] = _cspace.csPlane3_DD_set
    __swig_getmethods__["DD"] = _cspace.csPlane3_DD_get
    if _newclass:DD = property(_cspace.csPlane3_DD_get, _cspace.csPlane3_DD_set)
    def __init__(self, *args):
        _swig_setattr(self, csPlane3, 'this', _cspace.new_csPlane3(*args))
        _swig_setattr(self, csPlane3, 'thisown', 1)
    def Normal(*args): return _cspace.csPlane3_Normal(*args)
    def A(*args): return _cspace.csPlane3_A(*args)
    def B(*args): return _cspace.csPlane3_B(*args)
    def C(*args): return _cspace.csPlane3_C(*args)
    def D(*args): return _cspace.csPlane3_D(*args)
    def GetNormal(*args): return _cspace.csPlane3_GetNormal(*args)
    def Set(*args): return _cspace.csPlane3_Set(*args)
    __swig_getmethods__["Classify"] = lambda x: _cspace.csPlane3_Classify
    if _newclass:Classify = staticmethod(_cspace.csPlane3_Classify)
    def Distance(*args): return _cspace.csPlane3_Distance(*args)
    def Invert(*args): return _cspace.csPlane3_Invert(*args)
    def Normalize(*args): return _cspace.csPlane3_Normalize(*args)
    def FindPoint(*args): return _cspace.csPlane3_FindPoint(*args)
    def ClipPolygon(*args): return _cspace.csPlane3_ClipPolygon(*args)
    def __imul__(*args): return _cspace.csPlane3___imul__(*args)
    def __idiv__(*args): return _cspace.csPlane3___idiv__(*args)
    def __div__(*args): return _cspace.csPlane3___div__(*args)
    def __del__(self, destroy=_cspace.delete_csPlane3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPlane3Ptr(csPlane3):
    def __init__(self, this):
        _swig_setattr(self, csPlane3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPlane3, 'thisown', 0)
        _swig_setattr(self, csPlane3,self.__class__,csPlane3)
_cspace.csPlane3_swigregister(csPlane3Ptr)

csPlane3_Classify = _cspace.csPlane3_Classify

class csMath2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csMath2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csMath2, name)
    def __repr__(self):
        return "<C csMath2 instance at %s>" % (self.this,)
    __swig_getmethods__["WhichSide2D"] = lambda x: _cspace.csMath2_WhichSide2D
    if _newclass:WhichSide2D = staticmethod(_cspace.csMath2_WhichSide2D)
    __swig_getmethods__["WhichSide2D"] = lambda x: _cspace.csMath2_WhichSide2D
    if _newclass:WhichSide2D = staticmethod(_cspace.csMath2_WhichSide2D)
    __swig_getmethods__["InPoly2D"] = lambda x: _cspace.csMath2_InPoly2D
    if _newclass:InPoly2D = staticmethod(_cspace.csMath2_InPoly2D)
    __swig_getmethods__["Area2"] = lambda x: _cspace.csMath2_Area2
    if _newclass:Area2 = staticmethod(_cspace.csMath2_Area2)
    __swig_getmethods__["Right"] = lambda x: _cspace.csMath2_Right
    if _newclass:Right = staticmethod(_cspace.csMath2_Right)
    __swig_getmethods__["Left"] = lambda x: _cspace.csMath2_Left
    if _newclass:Left = staticmethod(_cspace.csMath2_Left)
    __swig_getmethods__["Visible"] = lambda x: _cspace.csMath2_Visible
    if _newclass:Visible = staticmethod(_cspace.csMath2_Visible)
    __swig_getmethods__["PlanesEqual"] = lambda x: _cspace.csMath2_PlanesEqual
    if _newclass:PlanesEqual = staticmethod(_cspace.csMath2_PlanesEqual)
    __swig_getmethods__["PlanesClose"] = lambda x: _cspace.csMath2_PlanesClose
    if _newclass:PlanesClose = staticmethod(_cspace.csMath2_PlanesClose)
    def __init__(self, *args):
        _swig_setattr(self, csMath2, 'this', _cspace.new_csMath2(*args))
        _swig_setattr(self, csMath2, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csMath2):
        try:
            if self.thisown: destroy(self)
        except: pass

class csMath2Ptr(csMath2):
    def __init__(self, this):
        _swig_setattr(self, csMath2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csMath2, 'thisown', 0)
        _swig_setattr(self, csMath2,self.__class__,csMath2)
_cspace.csMath2_swigregister(csMath2Ptr)

csMath2_WhichSide2D = _cspace.csMath2_WhichSide2D

csMath2_InPoly2D = _cspace.csMath2_InPoly2D

csMath2_Area2 = _cspace.csMath2_Area2

csMath2_Right = _cspace.csMath2_Right

csMath2_Left = _cspace.csMath2_Left

csMath2_Visible = _cspace.csMath2_Visible

csMath2_PlanesEqual = _cspace.csMath2_PlanesEqual

csMath2_PlanesClose = _cspace.csMath2_PlanesClose

class csIntersect2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csIntersect2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csIntersect2, name)
    def __repr__(self):
        return "<C csIntersect2 instance at %s>" % (self.this,)
    __swig_getmethods__["PlanePolygon"] = lambda x: _cspace.csIntersect2_PlanePolygon
    if _newclass:PlanePolygon = staticmethod(_cspace.csIntersect2_PlanePolygon)
    __swig_getmethods__["SegmentSegment"] = lambda x: _cspace.csIntersect2_SegmentSegment
    if _newclass:SegmentSegment = staticmethod(_cspace.csIntersect2_SegmentSegment)
    __swig_getmethods__["SegmentLine"] = lambda x: _cspace.csIntersect2_SegmentLine
    if _newclass:SegmentLine = staticmethod(_cspace.csIntersect2_SegmentLine)
    __swig_getmethods__["LineLine"] = lambda x: _cspace.csIntersect2_LineLine
    if _newclass:LineLine = staticmethod(_cspace.csIntersect2_LineLine)
    __swig_getmethods__["SegmentPlane"] = lambda x: _cspace.csIntersect2_SegmentPlane
    if _newclass:SegmentPlane = staticmethod(_cspace.csIntersect2_SegmentPlane)
    __swig_getmethods__["SegmentPlane"] = lambda x: _cspace.csIntersect2_SegmentPlane
    if _newclass:SegmentPlane = staticmethod(_cspace.csIntersect2_SegmentPlane)
    __swig_getmethods__["SegmentPlaneNoTest"] = lambda x: _cspace.csIntersect2_SegmentPlaneNoTest
    if _newclass:SegmentPlaneNoTest = staticmethod(_cspace.csIntersect2_SegmentPlaneNoTest)
    __swig_getmethods__["SegmentPlaneNoTest"] = lambda x: _cspace.csIntersect2_SegmentPlaneNoTest
    if _newclass:SegmentPlaneNoTest = staticmethod(_cspace.csIntersect2_SegmentPlaneNoTest)
    __swig_getmethods__["PlanePlane"] = lambda x: _cspace.csIntersect2_PlanePlane
    if _newclass:PlanePlane = staticmethod(_cspace.csIntersect2_PlanePlane)
    def __init__(self, *args):
        _swig_setattr(self, csIntersect2, 'this', _cspace.new_csIntersect2(*args))
        _swig_setattr(self, csIntersect2, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csIntersect2):
        try:
            if self.thisown: destroy(self)
        except: pass

class csIntersect2Ptr(csIntersect2):
    def __init__(self, this):
        _swig_setattr(self, csIntersect2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csIntersect2, 'thisown', 0)
        _swig_setattr(self, csIntersect2,self.__class__,csIntersect2)
_cspace.csIntersect2_swigregister(csIntersect2Ptr)

csIntersect2_PlanePolygon = _cspace.csIntersect2_PlanePolygon

csIntersect2_SegmentSegment = _cspace.csIntersect2_SegmentSegment

csIntersect2_SegmentLine = _cspace.csIntersect2_SegmentLine

csIntersect2_LineLine = _cspace.csIntersect2_LineLine

csIntersect2_SegmentPlane = _cspace.csIntersect2_SegmentPlane

csIntersect2_SegmentPlaneNoTest = _cspace.csIntersect2_SegmentPlaneNoTest

csIntersect2_PlanePlane = _cspace.csIntersect2_PlanePlane

class csPoly2D(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPoly2D, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPoly2D, name)
    def __repr__(self):
        return "<C csPoly2D instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csPoly2D, 'this', _cspace.new_csPoly2D(*args))
        _swig_setattr(self, csPoly2D, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPoly2D):
        try:
            if self.thisown: destroy(self)
        except: pass
    def assign(*args): return _cspace.csPoly2D_assign(*args)
    def MakeEmpty(*args): return _cspace.csPoly2D_MakeEmpty(*args)
    def GetVertexCount(*args): return _cspace.csPoly2D_GetVertexCount(*args)
    def GetVertices(*args): return _cspace.csPoly2D_GetVertices(*args)
    def GetVertex(*args): return _cspace.csPoly2D_GetVertex(*args)
    def GetFirst(*args): return _cspace.csPoly2D_GetFirst(*args)
    def GetLast(*args): return _cspace.csPoly2D_GetLast(*args)
    __swig_getmethods__["In"] = lambda x: _cspace.csPoly2D_In
    if _newclass:In = staticmethod(_cspace.csPoly2D_In)
    def MakeRoom(*args): return _cspace.csPoly2D_MakeRoom(*args)
    def SetVertexCount(*args): return _cspace.csPoly2D_SetVertexCount(*args)
    def AddVertex(*args): return _cspace.csPoly2D_AddVertex(*args)
    def SetVertices(*args): return _cspace.csPoly2D_SetVertices(*args)
    def ClipAgainst(*args): return _cspace.csPoly2D_ClipAgainst(*args)
    def Intersect(*args): return _cspace.csPoly2D_Intersect(*args)
    def ClipPlane(*args): return _cspace.csPoly2D_ClipPlane(*args)
    def ExtendConvex(*args): return _cspace.csPoly2D_ExtendConvex(*args)
    def GetSignedArea(*args): return _cspace.csPoly2D_GetSignedArea(*args)
    def Random(*args): return _cspace.csPoly2D_Random(*args)
    def __getitem__(*args): return _cspace.csPoly2D___getitem__(*args)
    def __setitem__ (self, i, v):
      own_v = self.__getitem__(i)
      for i in range(2):
        own_v[i] = v[i]


class csPoly2DPtr(csPoly2D):
    def __init__(self, this):
        _swig_setattr(self, csPoly2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPoly2D, 'thisown', 0)
        _swig_setattr(self, csPoly2D,self.__class__,csPoly2D)
_cspace.csPoly2D_swigregister(csPoly2DPtr)

csPoly2D_In = _cspace.csPoly2D_In

class csPoly2DFactory(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPoly2DFactory, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPoly2DFactory, name)
    def __repr__(self):
        return "<C csPoly2DFactory instance at %s>" % (self.this,)
    def Create(*args): return _cspace.csPoly2DFactory_Create(*args)
    def __init__(self, *args):
        _swig_setattr(self, csPoly2DFactory, 'this', _cspace.new_csPoly2DFactory(*args))
        _swig_setattr(self, csPoly2DFactory, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPoly2DFactory):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPoly2DFactoryPtr(csPoly2DFactory):
    def __init__(self, this):
        _swig_setattr(self, csPoly2DFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPoly2DFactory, 'thisown', 0)
        _swig_setattr(self, csPoly2DFactory,self.__class__,csPoly2DFactory)
_cspace.csPoly2DFactory_swigregister(csPoly2DFactoryPtr)


fSqr = _cspace.fSqr
class csMath3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csMath3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csMath3, name)
    def __repr__(self):
        return "<C csMath3 instance at %s>" % (self.this,)
    __swig_getmethods__["WhichSide3D"] = lambda x: _cspace.csMath3_WhichSide3D
    if _newclass:WhichSide3D = staticmethod(_cspace.csMath3_WhichSide3D)
    __swig_getmethods__["Visible"] = lambda x: _cspace.csMath3_Visible
    if _newclass:Visible = staticmethod(_cspace.csMath3_Visible)
    __swig_getmethods__["Visible"] = lambda x: _cspace.csMath3_Visible
    if _newclass:Visible = staticmethod(_cspace.csMath3_Visible)
    __swig_getmethods__["FindIntersection"] = lambda x: _cspace.csMath3_FindIntersection
    if _newclass:FindIntersection = staticmethod(_cspace.csMath3_FindIntersection)
    __swig_getmethods__["Between"] = lambda x: _cspace.csMath3_Between
    if _newclass:Between = staticmethod(_cspace.csMath3_Between)
    __swig_getmethods__["SetMinMax"] = lambda x: _cspace.csMath3_SetMinMax
    if _newclass:SetMinMax = staticmethod(_cspace.csMath3_SetMinMax)
    __swig_getmethods__["DoubleArea3"] = lambda x: _cspace.csMath3_DoubleArea3
    if _newclass:DoubleArea3 = staticmethod(_cspace.csMath3_DoubleArea3)
    __swig_getmethods__["Direction3"] = lambda x: _cspace.csMath3_Direction3
    if _newclass:Direction3 = staticmethod(_cspace.csMath3_Direction3)
    __swig_getmethods__["CalcNormal"] = lambda x: _cspace.csMath3_CalcNormal
    if _newclass:CalcNormal = staticmethod(_cspace.csMath3_CalcNormal)
    __swig_getmethods__["CalcNormal"] = lambda x: _cspace.csMath3_CalcNormal
    if _newclass:CalcNormal = staticmethod(_cspace.csMath3_CalcNormal)
    __swig_getmethods__["CalcPlane"] = lambda x: _cspace.csMath3_CalcPlane
    if _newclass:CalcPlane = staticmethod(_cspace.csMath3_CalcPlane)
    __swig_getmethods__["PlanesEqual"] = lambda x: _cspace.csMath3_PlanesEqual
    if _newclass:PlanesEqual = staticmethod(_cspace.csMath3_PlanesEqual)
    __swig_getmethods__["PlanesClose"] = lambda x: _cspace.csMath3_PlanesClose
    if _newclass:PlanesClose = staticmethod(_cspace.csMath3_PlanesClose)
    __swig_getmethods__["OuterPlanes"] = lambda x: _cspace.csMath3_OuterPlanes
    if _newclass:OuterPlanes = staticmethod(_cspace.csMath3_OuterPlanes)
    __swig_getmethods__["FindObserverSides"] = lambda x: _cspace.csMath3_FindObserverSides
    if _newclass:FindObserverSides = staticmethod(_cspace.csMath3_FindObserverSides)
    __swig_getmethods__["SpherePosition"] = lambda x: _cspace.csMath3_SpherePosition
    if _newclass:SpherePosition = staticmethod(_cspace.csMath3_SpherePosition)
    def __init__(self, *args):
        _swig_setattr(self, csMath3, 'this', _cspace.new_csMath3(*args))
        _swig_setattr(self, csMath3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csMath3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csMath3Ptr(csMath3):
    def __init__(self, this):
        _swig_setattr(self, csMath3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csMath3, 'thisown', 0)
        _swig_setattr(self, csMath3,self.__class__,csMath3)
_cspace.csMath3_swigregister(csMath3Ptr)

csMath3_WhichSide3D = _cspace.csMath3_WhichSide3D

csMath3_Visible = _cspace.csMath3_Visible

csMath3_FindIntersection = _cspace.csMath3_FindIntersection

csMath3_Between = _cspace.csMath3_Between

csMath3_SetMinMax = _cspace.csMath3_SetMinMax

csMath3_DoubleArea3 = _cspace.csMath3_DoubleArea3

csMath3_Direction3 = _cspace.csMath3_Direction3

csMath3_CalcNormal = _cspace.csMath3_CalcNormal

csMath3_CalcPlane = _cspace.csMath3_CalcPlane

csMath3_PlanesEqual = _cspace.csMath3_PlanesEqual

csMath3_PlanesClose = _cspace.csMath3_PlanesClose

csMath3_OuterPlanes = _cspace.csMath3_OuterPlanes

csMath3_FindObserverSides = _cspace.csMath3_FindObserverSides

csMath3_SpherePosition = _cspace.csMath3_SpherePosition

class csSquaredDist(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSquaredDist, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSquaredDist, name)
    def __repr__(self):
        return "<C csSquaredDist instance at %s>" % (self.this,)
    __swig_getmethods__["PointPoint"] = lambda x: _cspace.csSquaredDist_PointPoint
    if _newclass:PointPoint = staticmethod(_cspace.csSquaredDist_PointPoint)
    __swig_getmethods__["PointLine"] = lambda x: _cspace.csSquaredDist_PointLine
    if _newclass:PointLine = staticmethod(_cspace.csSquaredDist_PointLine)
    __swig_getmethods__["PointPlane"] = lambda x: _cspace.csSquaredDist_PointPlane
    if _newclass:PointPlane = staticmethod(_cspace.csSquaredDist_PointPlane)
    __swig_getmethods__["PointPoly"] = lambda x: _cspace.csSquaredDist_PointPoly
    if _newclass:PointPoly = staticmethod(_cspace.csSquaredDist_PointPoly)
    def __init__(self, *args):
        _swig_setattr(self, csSquaredDist, 'this', _cspace.new_csSquaredDist(*args))
        _swig_setattr(self, csSquaredDist, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csSquaredDist):
        try:
            if self.thisown: destroy(self)
        except: pass

class csSquaredDistPtr(csSquaredDist):
    def __init__(self, this):
        _swig_setattr(self, csSquaredDist, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSquaredDist, 'thisown', 0)
        _swig_setattr(self, csSquaredDist,self.__class__,csSquaredDist)
_cspace.csSquaredDist_swigregister(csSquaredDistPtr)

csSquaredDist_PointPoint = _cspace.csSquaredDist_PointPoint

csSquaredDist_PointLine = _cspace.csSquaredDist_PointLine

csSquaredDist_PointPlane = _cspace.csSquaredDist_PointPlane

csSquaredDist_PointPoly = _cspace.csSquaredDist_PointPoly

class csIntersect3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csIntersect3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csIntersect3, name)
    def __repr__(self):
        return "<C csIntersect3 instance at %s>" % (self.this,)
    __swig_getmethods__["PlanePolygon"] = lambda x: _cspace.csIntersect3_PlanePolygon
    if _newclass:PlanePolygon = staticmethod(_cspace.csIntersect3_PlanePolygon)
    __swig_getmethods__["SegmentFrustum"] = lambda x: _cspace.csIntersect3_SegmentFrustum
    if _newclass:SegmentFrustum = staticmethod(_cspace.csIntersect3_SegmentFrustum)
    __swig_getmethods__["SegmentTriangle"] = lambda x: _cspace.csIntersect3_SegmentTriangle
    if _newclass:SegmentTriangle = staticmethod(_cspace.csIntersect3_SegmentTriangle)
    __swig_getmethods__["SegmentPolygon"] = lambda x: _cspace.csIntersect3_SegmentPolygon
    if _newclass:SegmentPolygon = staticmethod(_cspace.csIntersect3_SegmentPolygon)
    __swig_getmethods__["SegmentPlanes"] = lambda x: _cspace.csIntersect3_SegmentPlanes
    if _newclass:SegmentPlanes = staticmethod(_cspace.csIntersect3_SegmentPlanes)
    __swig_getmethods__["SegmentPlane"] = lambda x: _cspace.csIntersect3_SegmentPlane
    if _newclass:SegmentPlane = staticmethod(_cspace.csIntersect3_SegmentPlane)
    __swig_getmethods__["SegmentPlane"] = lambda x: _cspace.csIntersect3_SegmentPlane
    if _newclass:SegmentPlane = staticmethod(_cspace.csIntersect3_SegmentPlane)
    __swig_getmethods__["ThreePlanes"] = lambda x: _cspace.csIntersect3_ThreePlanes
    if _newclass:ThreePlanes = staticmethod(_cspace.csIntersect3_ThreePlanes)
    __swig_getmethods__["PlaneXPlane"] = lambda x: _cspace.csIntersect3_PlaneXPlane
    if _newclass:PlaneXPlane = staticmethod(_cspace.csIntersect3_PlaneXPlane)
    __swig_getmethods__["PlaneYPlane"] = lambda x: _cspace.csIntersect3_PlaneYPlane
    if _newclass:PlaneYPlane = staticmethod(_cspace.csIntersect3_PlaneYPlane)
    __swig_getmethods__["PlaneZPlane"] = lambda x: _cspace.csIntersect3_PlaneZPlane
    if _newclass:PlaneZPlane = staticmethod(_cspace.csIntersect3_PlaneZPlane)
    __swig_getmethods__["PlaneAxisPlane"] = lambda x: _cspace.csIntersect3_PlaneAxisPlane
    if _newclass:PlaneAxisPlane = staticmethod(_cspace.csIntersect3_PlaneAxisPlane)
    __swig_getmethods__["SegmentZ0Plane"] = lambda x: _cspace.csIntersect3_SegmentZ0Plane
    if _newclass:SegmentZ0Plane = staticmethod(_cspace.csIntersect3_SegmentZ0Plane)
    __swig_getmethods__["SegmentZ0Plane"] = lambda x: _cspace.csIntersect3_SegmentZ0Plane
    if _newclass:SegmentZ0Plane = staticmethod(_cspace.csIntersect3_SegmentZ0Plane)
    __swig_getmethods__["SegmentXPlane"] = lambda x: _cspace.csIntersect3_SegmentXPlane
    if _newclass:SegmentXPlane = staticmethod(_cspace.csIntersect3_SegmentXPlane)
    __swig_getmethods__["SegmentXPlane"] = lambda x: _cspace.csIntersect3_SegmentXPlane
    if _newclass:SegmentXPlane = staticmethod(_cspace.csIntersect3_SegmentXPlane)
    __swig_getmethods__["SegmentYPlane"] = lambda x: _cspace.csIntersect3_SegmentYPlane
    if _newclass:SegmentYPlane = staticmethod(_cspace.csIntersect3_SegmentYPlane)
    __swig_getmethods__["SegmentYPlane"] = lambda x: _cspace.csIntersect3_SegmentYPlane
    if _newclass:SegmentYPlane = staticmethod(_cspace.csIntersect3_SegmentYPlane)
    __swig_getmethods__["SegmentZPlane"] = lambda x: _cspace.csIntersect3_SegmentZPlane
    if _newclass:SegmentZPlane = staticmethod(_cspace.csIntersect3_SegmentZPlane)
    __swig_getmethods__["SegmentZPlane"] = lambda x: _cspace.csIntersect3_SegmentZPlane
    if _newclass:SegmentZPlane = staticmethod(_cspace.csIntersect3_SegmentZPlane)
    __swig_getmethods__["SegmentAxisPlane"] = lambda x: _cspace.csIntersect3_SegmentAxisPlane
    if _newclass:SegmentAxisPlane = staticmethod(_cspace.csIntersect3_SegmentAxisPlane)
    __swig_getmethods__["SegmentXFrustum"] = lambda x: _cspace.csIntersect3_SegmentXFrustum
    if _newclass:SegmentXFrustum = staticmethod(_cspace.csIntersect3_SegmentXFrustum)
    __swig_getmethods__["SegmentXFrustum"] = lambda x: _cspace.csIntersect3_SegmentXFrustum
    if _newclass:SegmentXFrustum = staticmethod(_cspace.csIntersect3_SegmentXFrustum)
    __swig_getmethods__["SegmentYFrustum"] = lambda x: _cspace.csIntersect3_SegmentYFrustum
    if _newclass:SegmentYFrustum = staticmethod(_cspace.csIntersect3_SegmentYFrustum)
    __swig_getmethods__["SegmentYFrustum"] = lambda x: _cspace.csIntersect3_SegmentYFrustum
    if _newclass:SegmentYFrustum = staticmethod(_cspace.csIntersect3_SegmentYFrustum)
    __swig_getmethods__["BoxSegment"] = lambda x: _cspace.csIntersect3_BoxSegment
    if _newclass:BoxSegment = staticmethod(_cspace.csIntersect3_BoxSegment)
    __swig_getmethods__["BoxFrustum"] = lambda x: _cspace.csIntersect3_BoxFrustum
    if _newclass:BoxFrustum = staticmethod(_cspace.csIntersect3_BoxFrustum)
    __swig_getmethods__["BoxSphere"] = lambda x: _cspace.csIntersect3_BoxSphere
    if _newclass:BoxSphere = staticmethod(_cspace.csIntersect3_BoxSphere)
    __swig_getmethods__["BoxPlane"] = lambda x: _cspace.csIntersect3_BoxPlane
    if _newclass:BoxPlane = staticmethod(_cspace.csIntersect3_BoxPlane)
    __swig_getmethods__["BoxPlane"] = lambda x: _cspace.csIntersect3_BoxPlane
    if _newclass:BoxPlane = staticmethod(_cspace.csIntersect3_BoxPlane)
    __swig_getmethods__["BoxTriangle"] = lambda x: _cspace.csIntersect3_BoxTriangle
    if _newclass:BoxTriangle = staticmethod(_cspace.csIntersect3_BoxTriangle)
    __swig_getmethods__["BoxBox"] = lambda x: _cspace.csIntersect3_BoxBox
    if _newclass:BoxBox = staticmethod(_cspace.csIntersect3_BoxBox)
    __swig_getmethods__["FrustumFrustum"] = lambda x: _cspace.csIntersect3_FrustumFrustum
    if _newclass:FrustumFrustum = staticmethod(_cspace.csIntersect3_FrustumFrustum)
    __swig_getmethods__["FrustumFrustum"] = lambda x: _cspace.csIntersect3_FrustumFrustum
    if _newclass:FrustumFrustum = staticmethod(_cspace.csIntersect3_FrustumFrustum)
    def __init__(self, *args):
        _swig_setattr(self, csIntersect3, 'this', _cspace.new_csIntersect3(*args))
        _swig_setattr(self, csIntersect3, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csIntersect3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csIntersect3Ptr(csIntersect3):
    def __init__(self, this):
        _swig_setattr(self, csIntersect3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csIntersect3, 'thisown', 0)
        _swig_setattr(self, csIntersect3,self.__class__,csIntersect3)
_cspace.csIntersect3_swigregister(csIntersect3Ptr)

csIntersect3_PlanePolygon = _cspace.csIntersect3_PlanePolygon

csIntersect3_SegmentFrustum = _cspace.csIntersect3_SegmentFrustum

csIntersect3_SegmentTriangle = _cspace.csIntersect3_SegmentTriangle

csIntersect3_SegmentPolygon = _cspace.csIntersect3_SegmentPolygon

csIntersect3_SegmentPlanes = _cspace.csIntersect3_SegmentPlanes

csIntersect3_SegmentPlane = _cspace.csIntersect3_SegmentPlane

csIntersect3_ThreePlanes = _cspace.csIntersect3_ThreePlanes

csIntersect3_PlaneXPlane = _cspace.csIntersect3_PlaneXPlane

csIntersect3_PlaneYPlane = _cspace.csIntersect3_PlaneYPlane

csIntersect3_PlaneZPlane = _cspace.csIntersect3_PlaneZPlane

csIntersect3_PlaneAxisPlane = _cspace.csIntersect3_PlaneAxisPlane

csIntersect3_SegmentZ0Plane = _cspace.csIntersect3_SegmentZ0Plane

csIntersect3_SegmentXPlane = _cspace.csIntersect3_SegmentXPlane

csIntersect3_SegmentYPlane = _cspace.csIntersect3_SegmentYPlane

csIntersect3_SegmentZPlane = _cspace.csIntersect3_SegmentZPlane

csIntersect3_SegmentAxisPlane = _cspace.csIntersect3_SegmentAxisPlane

csIntersect3_SegmentXFrustum = _cspace.csIntersect3_SegmentXFrustum

csIntersect3_SegmentYFrustum = _cspace.csIntersect3_SegmentYFrustum

csIntersect3_BoxSegment = _cspace.csIntersect3_BoxSegment

csIntersect3_BoxFrustum = _cspace.csIntersect3_BoxFrustum

csIntersect3_BoxSphere = _cspace.csIntersect3_BoxSphere

csIntersect3_BoxPlane = _cspace.csIntersect3_BoxPlane

csIntersect3_BoxTriangle = _cspace.csIntersect3_BoxTriangle

csIntersect3_BoxBox = _cspace.csIntersect3_BoxBox

csIntersect3_FrustumFrustum = _cspace.csIntersect3_FrustumFrustum

class csGeomDebugHelper(iDebugHelper):
    __swig_setmethods__ = {}
    for _s in [iDebugHelper]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csGeomDebugHelper, name, value)
    __swig_getmethods__ = {}
    for _s in [iDebugHelper]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csGeomDebugHelper, name)
    def __repr__(self):
        return "<C csGeomDebugHelper instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csGeomDebugHelper, 'this', _cspace.new_csGeomDebugHelper(*args))
        _swig_setattr(self, csGeomDebugHelper, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csGeomDebugHelper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_setmethods__["scfRefCount"] = _cspace.csGeomDebugHelper_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace.csGeomDebugHelper_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace.csGeomDebugHelper_scfRefCount_get, _cspace.csGeomDebugHelper_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace.csGeomDebugHelper_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace.csGeomDebugHelper_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace.csGeomDebugHelper_scfWeakRefOwners_get, _cspace.csGeomDebugHelper_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace.csGeomDebugHelper_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace.csGeomDebugHelper_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace.csGeomDebugHelper_scfParent_get
    if _newclass:scfParent = property(_cspace.csGeomDebugHelper_scfParent_get, _cspace.csGeomDebugHelper_scfParent_set)
    def IncRef(*args): return _cspace.csGeomDebugHelper_IncRef(*args)
    def DecRef(*args): return _cspace.csGeomDebugHelper_DecRef(*args)
    def GetRefCount(*args): return _cspace.csGeomDebugHelper_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csGeomDebugHelper_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csGeomDebugHelper_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csGeomDebugHelper_QueryInterface(*args)
    def GetSupportedTests(*args): return _cspace.csGeomDebugHelper_GetSupportedTests(*args)
    def UnitTest(*args): return _cspace.csGeomDebugHelper_UnitTest(*args)
    def StateTest(*args): return _cspace.csGeomDebugHelper_StateTest(*args)
    def Benchmark(*args): return _cspace.csGeomDebugHelper_Benchmark(*args)
    def Dump(*args): return _cspace.csGeomDebugHelper_Dump(*args)
    def DebugCommand(*args): return _cspace.csGeomDebugHelper_DebugCommand(*args)

class csGeomDebugHelperPtr(csGeomDebugHelper):
    def __init__(self, this):
        _swig_setattr(self, csGeomDebugHelper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csGeomDebugHelper, 'thisown', 0)
        _swig_setattr(self, csGeomDebugHelper,self.__class__,csGeomDebugHelper)
_cspace.csGeomDebugHelper_swigregister(csGeomDebugHelperPtr)

CS_POL_SAME_PLANE = _cspace.CS_POL_SAME_PLANE
CS_POL_FRONT = _cspace.CS_POL_FRONT
CS_POL_BACK = _cspace.CS_POL_BACK
CS_POL_SPLIT_NEEDED = _cspace.CS_POL_SPLIT_NEEDED
class csPoly3D(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPoly3D, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPoly3D, name)
    def __repr__(self):
        return "<C csPoly3D instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csPoly3D, 'this', _cspace.new_csPoly3D(*args))
        _swig_setattr(self, csPoly3D, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPoly3D):
        try:
            if self.thisown: destroy(self)
        except: pass
    def MakeEmpty(*args): return _cspace.csPoly3D_MakeEmpty(*args)
    def GetVertexCount(*args): return _cspace.csPoly3D_GetVertexCount(*args)
    def GetVertices(*args): return _cspace.csPoly3D_GetVertices(*args)
    def GetVertex(*args): return _cspace.csPoly3D_GetVertex(*args)
    def GetFirst(*args): return _cspace.csPoly3D_GetFirst(*args)
    def GetLast(*args): return _cspace.csPoly3D_GetLast(*args)
    __swig_getmethods__["In"] = lambda x: _cspace.csPoly3D_In
    if _newclass:In = staticmethod(_cspace.csPoly3D_In)
    def MakeRoom(*args): return _cspace.csPoly3D_MakeRoom(*args)
    def SetVertexCount(*args): return _cspace.csPoly3D_SetVertexCount(*args)
    def AddVertex(*args): return _cspace.csPoly3D_AddVertex(*args)
    def SetVertices(*args): return _cspace.csPoly3D_SetVertices(*args)
    def ProjectXPlane(*args): return _cspace.csPoly3D_ProjectXPlane(*args)
    def ProjectYPlane(*args): return _cspace.csPoly3D_ProjectYPlane(*args)
    def ProjectZPlane(*args): return _cspace.csPoly3D_ProjectZPlane(*args)
    def ProjectAxisPlane(*args): return _cspace.csPoly3D_ProjectAxisPlane(*args)
    __swig_getmethods__["Classify"] = lambda x: _cspace.csPoly3D_Classify
    if _newclass:Classify = staticmethod(_cspace.csPoly3D_Classify)
    def Classify(*args): return _cspace.csPoly3D_Classify(*args)
    def ClassifyX(*args): return _cspace.csPoly3D_ClassifyX(*args)
    def ClassifyY(*args): return _cspace.csPoly3D_ClassifyY(*args)
    def ClassifyZ(*args): return _cspace.csPoly3D_ClassifyZ(*args)
    def ClassifyAxis(*args): return _cspace.csPoly3D_ClassifyAxis(*args)
    def IsAxisAligned(*args): return _cspace.csPoly3D_IsAxisAligned(*args)
    def CutToPlane(*args): return _cspace.csPoly3D_CutToPlane(*args)
    def SplitWithPlane(*args): return _cspace.csPoly3D_SplitWithPlane(*args)
    def SplitWithPlaneX(*args): return _cspace.csPoly3D_SplitWithPlaneX(*args)
    def SplitWithPlaneY(*args): return _cspace.csPoly3D_SplitWithPlaneY(*args)
    def SplitWithPlaneZ(*args): return _cspace.csPoly3D_SplitWithPlaneZ(*args)
    __swig_getmethods__["ComputeNormal"] = lambda x: _cspace.csPoly3D_ComputeNormal
    if _newclass:ComputeNormal = staticmethod(_cspace.csPoly3D_ComputeNormal)
    __swig_getmethods__["ComputeNormal"] = lambda x: _cspace.csPoly3D_ComputeNormal
    if _newclass:ComputeNormal = staticmethod(_cspace.csPoly3D_ComputeNormal)
    __swig_getmethods__["ComputeNormal"] = lambda x: _cspace.csPoly3D_ComputeNormal
    if _newclass:ComputeNormal = staticmethod(_cspace.csPoly3D_ComputeNormal)
    def ComputeNormal(*args): return _cspace.csPoly3D_ComputeNormal(*args)
    __swig_getmethods__["ComputePlane"] = lambda x: _cspace.csPoly3D_ComputePlane
    if _newclass:ComputePlane = staticmethod(_cspace.csPoly3D_ComputePlane)
    __swig_getmethods__["ComputePlane"] = lambda x: _cspace.csPoly3D_ComputePlane
    if _newclass:ComputePlane = staticmethod(_cspace.csPoly3D_ComputePlane)
    __swig_getmethods__["ComputePlane"] = lambda x: _cspace.csPoly3D_ComputePlane
    if _newclass:ComputePlane = staticmethod(_cspace.csPoly3D_ComputePlane)
    def ComputePlane(*args): return _cspace.csPoly3D_ComputePlane(*args)
    def GetArea(*args): return _cspace.csPoly3D_GetArea(*args)
    def GetCenter(*args): return _cspace.csPoly3D_GetCenter(*args)
    def __getitem__(*args): return _cspace.csPoly3D___getitem__(*args)
    def __setitem__ (self, i, v):
      own_v = self.__getitem__(i)
      for i in range(3):
        own_v[i] = v[i]


class csPoly3DPtr(csPoly3D):
    def __init__(self, this):
        _swig_setattr(self, csPoly3D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPoly3D, 'thisown', 0)
        _swig_setattr(self, csPoly3D,self.__class__,csPoly3D)
_cspace.csPoly3D_swigregister(csPoly3DPtr)

csPoly3D_In = _cspace.csPoly3D_In

class csCompressVertex(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csCompressVertex, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csCompressVertex, name)
    def __repr__(self):
        return "<C csCompressVertex instance at %s>" % (self.this,)
    __swig_setmethods__["orig_idx"] = _cspace.csCompressVertex_orig_idx_set
    __swig_getmethods__["orig_idx"] = _cspace.csCompressVertex_orig_idx_get
    if _newclass:orig_idx = property(_cspace.csCompressVertex_orig_idx_get, _cspace.csCompressVertex_orig_idx_set)
    __swig_setmethods__["x"] = _cspace.csCompressVertex_x_set
    __swig_getmethods__["x"] = _cspace.csCompressVertex_x_get
    if _newclass:x = property(_cspace.csCompressVertex_x_get, _cspace.csCompressVertex_x_set)
    __swig_setmethods__["y"] = _cspace.csCompressVertex_y_set
    __swig_getmethods__["y"] = _cspace.csCompressVertex_y_get
    if _newclass:y = property(_cspace.csCompressVertex_y_get, _cspace.csCompressVertex_y_set)
    __swig_setmethods__["z"] = _cspace.csCompressVertex_z_set
    __swig_getmethods__["z"] = _cspace.csCompressVertex_z_get
    if _newclass:z = property(_cspace.csCompressVertex_z_get, _cspace.csCompressVertex_z_set)
    __swig_setmethods__["new_idx"] = _cspace.csCompressVertex_new_idx_set
    __swig_getmethods__["new_idx"] = _cspace.csCompressVertex_new_idx_get
    if _newclass:new_idx = property(_cspace.csCompressVertex_new_idx_get, _cspace.csCompressVertex_new_idx_set)
    __swig_setmethods__["used"] = _cspace.csCompressVertex_used_set
    __swig_getmethods__["used"] = _cspace.csCompressVertex_used_get
    if _newclass:used = property(_cspace.csCompressVertex_used_get, _cspace.csCompressVertex_used_set)
    def __init__(self, *args):
        _swig_setattr(self, csCompressVertex, 'this', _cspace.new_csCompressVertex(*args))
        _swig_setattr(self, csCompressVertex, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csCompressVertex):
        try:
            if self.thisown: destroy(self)
        except: pass

class csCompressVertexPtr(csCompressVertex):
    def __init__(self, this):
        _swig_setattr(self, csCompressVertex, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csCompressVertex, 'thisown', 0)
        _swig_setattr(self, csCompressVertex,self.__class__,csCompressVertex)
_cspace.csCompressVertex_swigregister(csCompressVertexPtr)

class csVector3Array(csPoly3D):
    __swig_setmethods__ = {}
    for _s in [csPoly3D]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csVector3Array, name, value)
    __swig_getmethods__ = {}
    for _s in [csPoly3D]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csVector3Array, name)
    def __repr__(self):
        return "<C csVector3Array instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csVector3Array, 'this', _cspace.new_csVector3Array(*args))
        _swig_setattr(self, csVector3Array, 'thisown', 1)
    def AddVertexSmart(*args): return _cspace.csVector3Array_AddVertexSmart(*args)
    __swig_getmethods__["CompressVertices"] = lambda x: _cspace.csVector3Array_CompressVertices
    if _newclass:CompressVertices = staticmethod(_cspace.csVector3Array_CompressVertices)
    __swig_getmethods__["CompressVertices"] = lambda x: _cspace.csVector3Array_CompressVertices
    if _newclass:CompressVertices = staticmethod(_cspace.csVector3Array_CompressVertices)
    def __del__(self, destroy=_cspace.delete_csVector3Array):
        try:
            if self.thisown: destroy(self)
        except: pass

class csVector3ArrayPtr(csVector3Array):
    def __init__(self, this):
        _swig_setattr(self, csVector3Array, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csVector3Array, 'thisown', 0)
        _swig_setattr(self, csVector3Array,self.__class__,csVector3Array)
_cspace.csVector3Array_swigregister(csVector3ArrayPtr)

csVector3Array_CompressVertices = _cspace.csVector3Array_CompressVertices

class csTriangle(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csTriangle, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csTriangle, name)
    def __repr__(self):
        return "<C csTriangle instance at %s>" % (self.this,)
    __swig_setmethods__["a"] = _cspace.csTriangle_a_set
    __swig_getmethods__["a"] = _cspace.csTriangle_a_get
    if _newclass:a = property(_cspace.csTriangle_a_get, _cspace.csTriangle_a_set)
    __swig_setmethods__["b"] = _cspace.csTriangle_b_set
    __swig_getmethods__["b"] = _cspace.csTriangle_b_get
    if _newclass:b = property(_cspace.csTriangle_b_get, _cspace.csTriangle_b_set)
    __swig_setmethods__["c"] = _cspace.csTriangle_c_set
    __swig_getmethods__["c"] = _cspace.csTriangle_c_get
    if _newclass:c = property(_cspace.csTriangle_c_get, _cspace.csTriangle_c_set)
    def __init__(self, *args):
        _swig_setattr(self, csTriangle, 'this', _cspace.new_csTriangle(*args))
        _swig_setattr(self, csTriangle, 'thisown', 1)
    def assign(*args): return _cspace.csTriangle_assign(*args)
    def Set(*args): return _cspace.csTriangle_Set(*args)
    def __del__(self, destroy=_cspace.delete_csTriangle):
        try:
            if self.thisown: destroy(self)
        except: pass

class csTrianglePtr(csTriangle):
    def __init__(self, this):
        _swig_setattr(self, csTriangle, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csTriangle, 'thisown', 0)
        _swig_setattr(self, csTriangle,self.__class__,csTriangle)
_cspace.csTriangle_swigregister(csTrianglePtr)

class csRect(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csRect, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csRect, name)
    def __repr__(self):
        return "<C csRect instance at %s>" % (self.this,)
    __swig_setmethods__["xmin"] = _cspace.csRect_xmin_set
    __swig_getmethods__["xmin"] = _cspace.csRect_xmin_get
    if _newclass:xmin = property(_cspace.csRect_xmin_get, _cspace.csRect_xmin_set)
    __swig_setmethods__["ymin"] = _cspace.csRect_ymin_set
    __swig_getmethods__["ymin"] = _cspace.csRect_ymin_get
    if _newclass:ymin = property(_cspace.csRect_ymin_get, _cspace.csRect_ymin_set)
    __swig_setmethods__["xmax"] = _cspace.csRect_xmax_set
    __swig_getmethods__["xmax"] = _cspace.csRect_xmax_get
    if _newclass:xmax = property(_cspace.csRect_xmax_get, _cspace.csRect_xmax_set)
    __swig_setmethods__["ymax"] = _cspace.csRect_ymax_set
    __swig_getmethods__["ymax"] = _cspace.csRect_ymax_get
    if _newclass:ymax = property(_cspace.csRect_ymax_get, _cspace.csRect_ymax_set)
    def __init__(self, *args):
        _swig_setattr(self, csRect, 'this', _cspace.new_csRect(*args))
        _swig_setattr(self, csRect, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csRect):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Intersect(*args): return _cspace.csRect_Intersect(*args)
    def Intersects(*args): return _cspace.csRect_Intersects(*args)
    def Union(*args): return _cspace.csRect_Union(*args)
    def Exclude(*args): return _cspace.csRect_Exclude(*args)
    def Subtract(*args): return _cspace.csRect_Subtract(*args)
    def IsEmpty(*args): return _cspace.csRect_IsEmpty(*args)
    def MakeEmpty(*args): return _cspace.csRect_MakeEmpty(*args)
    def Set(*args): return _cspace.csRect_Set(*args)
    def SetPos(*args): return _cspace.csRect_SetPos(*args)
    def SetSize(*args): return _cspace.csRect_SetSize(*args)
    def Move(*args): return _cspace.csRect_Move(*args)
    def Width(*args): return _cspace.csRect_Width(*args)
    def Height(*args): return _cspace.csRect_Height(*args)
    def Contains(*args): return _cspace.csRect_Contains(*args)
    def ContainsRel(*args): return _cspace.csRect_ContainsRel(*args)
    def Equal(*args): return _cspace.csRect_Equal(*args)
    def Normalize(*args): return _cspace.csRect_Normalize(*args)
    def Area(*args): return _cspace.csRect_Area(*args)
    def AddAdjanced(*args): return _cspace.csRect_AddAdjanced(*args)
    def __eq__(*args): return _cspace.csRect___eq__(*args)
    def __ne__(*args): return _cspace.csRect___ne__(*args)
    def Extend(*args): return _cspace.csRect_Extend(*args)
    def Join(*args): return _cspace.csRect_Join(*args)
    def Outset(*args): return _cspace.csRect_Outset(*args)
    def Inset(*args): return _cspace.csRect_Inset(*args)
    def ClipLineGeneral(*args): return _cspace.csRect_ClipLineGeneral(*args)
    def ClipLine(*args): return _cspace.csRect_ClipLine(*args)
    def ClipLineSafe(*args): return _cspace.csRect_ClipLineSafe(*args)

class csRectPtr(csRect):
    def __init__(self, this):
        _swig_setattr(self, csRect, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csRect, 'thisown', 0)
        _swig_setattr(self, csRect,self.__class__,csRect)
_cspace.csRect_swigregister(csRectPtr)

class csRectRegion(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csRectRegion, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csRectRegion, name)
    def __repr__(self):
        return "<C csRectRegion instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csRectRegion, 'this', _cspace.new_csRectRegion(*args))
        _swig_setattr(self, csRectRegion, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csRectRegion):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Include(*args): return _cspace.csRectRegion_Include(*args)
    def Exclude(*args): return _cspace.csRectRegion_Exclude(*args)
    def ClipTo(*args): return _cspace.csRectRegion_ClipTo(*args)
    def Count(*args): return _cspace.csRectRegion_Count(*args)
    def RectAt(*args): return _cspace.csRectRegion_RectAt(*args)
    def makeEmpty(*args): return _cspace.csRectRegion_makeEmpty(*args)

class csRectRegionPtr(csRectRegion):
    def __init__(self, this):
        _swig_setattr(self, csRectRegion, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csRectRegion, 'thisown', 0)
        _swig_setattr(self, csRectRegion,self.__class__,csRectRegion)
_cspace.csRectRegion_swigregister(csRectRegionPtr)
FRAGMENT_BUFFER_SIZE = cvar.FRAGMENT_BUFFER_SIZE

class csQuaternion(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csQuaternion, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csQuaternion, name)
    def __repr__(self):
        return "<C csQuaternion instance at %s>" % (self.this,)
    def Init(*args): return _cspace.csQuaternion_Init(*args)
    def __init__(self, *args):
        _swig_setattr(self, csQuaternion, 'this', _cspace.new_csQuaternion(*args))
        _swig_setattr(self, csQuaternion, 'thisown', 1)
    def __imul__(*args): return _cspace.csQuaternion___imul__(*args)
    def Conjugate(*args): return _cspace.csQuaternion_Conjugate(*args)
    def Negate(*args): return _cspace.csQuaternion_Negate(*args)
    def Invert(*args): return _cspace.csQuaternion_Invert(*args)
    def GetAxisAngle(*args): return _cspace.csQuaternion_GetAxisAngle(*args)
    def SetWithAxisAngle(*args): return _cspace.csQuaternion_SetWithAxisAngle(*args)
    def PrepRotation(*args): return _cspace.csQuaternion_PrepRotation(*args)
    def Rotate(*args): return _cspace.csQuaternion_Rotate(*args)
    def Normalize(*args): return _cspace.csQuaternion_Normalize(*args)
    def SetWithEuler(*args): return _cspace.csQuaternion_SetWithEuler(*args)
    def GetEulerAngles(*args): return _cspace.csQuaternion_GetEulerAngles(*args)
    def ToAxisAngle(*args): return _cspace.csQuaternion_ToAxisAngle(*args)
    def Slerp(*args): return _cspace.csQuaternion_Slerp(*args)
    __swig_setmethods__["r"] = _cspace.csQuaternion_r_set
    __swig_getmethods__["r"] = _cspace.csQuaternion_r_get
    if _newclass:r = property(_cspace.csQuaternion_r_get, _cspace.csQuaternion_r_set)
    __swig_setmethods__["x"] = _cspace.csQuaternion_x_set
    __swig_getmethods__["x"] = _cspace.csQuaternion_x_get
    if _newclass:x = property(_cspace.csQuaternion_x_get, _cspace.csQuaternion_x_set)
    __swig_setmethods__["y"] = _cspace.csQuaternion_y_set
    __swig_getmethods__["y"] = _cspace.csQuaternion_y_get
    if _newclass:y = property(_cspace.csQuaternion_y_get, _cspace.csQuaternion_y_set)
    __swig_setmethods__["z"] = _cspace.csQuaternion_z_set
    __swig_getmethods__["z"] = _cspace.csQuaternion_z_get
    if _newclass:z = property(_cspace.csQuaternion_z_get, _cspace.csQuaternion_z_set)
    def __add__(*args): return _cspace.csQuaternion___add__(*args)
    def __sub__(*args): return _cspace.csQuaternion___sub__(*args)
    def __mul__(*args): return _cspace.csQuaternion___mul__(*args)
    def __del__(self, destroy=_cspace.delete_csQuaternion):
        try:
            if self.thisown: destroy(self)
        except: pass

class csQuaternionPtr(csQuaternion):
    def __init__(self, this):
        _swig_setattr(self, csQuaternion, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csQuaternion, 'thisown', 0)
        _swig_setattr(self, csQuaternion,self.__class__,csQuaternion)
_cspace.csQuaternion_swigregister(csQuaternionPtr)

class csSpline(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSpline, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSpline, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C csSpline instance at %s>" % (self.this,)
    def __del__(self, destroy=_cspace.delete_csSpline):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetDimensionCount(*args): return _cspace.csSpline_GetDimensionCount(*args)
    def GetPointCount(*args): return _cspace.csSpline_GetPointCount(*args)
    def InsertPoint(*args): return _cspace.csSpline_InsertPoint(*args)
    def RemovePoint(*args): return _cspace.csSpline_RemovePoint(*args)
    def SetTimeValues(*args): return _cspace.csSpline_SetTimeValues(*args)
    def SetTimeValue(*args): return _cspace.csSpline_SetTimeValue(*args)
    def GetTimeValues(*args): return _cspace.csSpline_GetTimeValues(*args)
    def GetTimeValue(*args): return _cspace.csSpline_GetTimeValue(*args)
    def SetDimensionValues(*args): return _cspace.csSpline_SetDimensionValues(*args)
    def SetDimensionValue(*args): return _cspace.csSpline_SetDimensionValue(*args)
    def GetDimensionValues(*args): return _cspace.csSpline_GetDimensionValues(*args)
    def GetDimensionValue(*args): return _cspace.csSpline_GetDimensionValue(*args)
    def SetIndexValues(*args): return _cspace.csSpline_SetIndexValues(*args)
    def GetIndexValues(*args): return _cspace.csSpline_GetIndexValues(*args)
    def Calculate(*args): return _cspace.csSpline_Calculate(*args)
    def GetCurrentIndex(*args): return _cspace.csSpline_GetCurrentIndex(*args)
    def GetInterpolatedDimension(*args): return _cspace.csSpline_GetInterpolatedDimension(*args)

class csSplinePtr(csSpline):
    def __init__(self, this):
        _swig_setattr(self, csSpline, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSpline, 'thisown', 0)
        _swig_setattr(self, csSpline,self.__class__,csSpline)
_cspace.csSpline_swigregister(csSplinePtr)

class csCubicSpline(csSpline):
    __swig_setmethods__ = {}
    for _s in [csSpline]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csCubicSpline, name, value)
    __swig_getmethods__ = {}
    for _s in [csSpline]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csCubicSpline, name)
    def __repr__(self):
        return "<C csCubicSpline instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csCubicSpline, 'this', _cspace.new_csCubicSpline(*args))
        _swig_setattr(self, csCubicSpline, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csCubicSpline):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Calculate(*args): return _cspace.csCubicSpline_Calculate(*args)
    def GetInterpolatedDimension(*args): return _cspace.csCubicSpline_GetInterpolatedDimension(*args)

class csCubicSplinePtr(csCubicSpline):
    def __init__(self, this):
        _swig_setattr(self, csCubicSpline, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csCubicSpline, 'thisown', 0)
        _swig_setattr(self, csCubicSpline,self.__class__,csCubicSpline)
_cspace.csCubicSpline_swigregister(csCubicSplinePtr)

class csBSpline(csSpline):
    __swig_setmethods__ = {}
    for _s in [csSpline]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csBSpline, name, value)
    __swig_getmethods__ = {}
    for _s in [csSpline]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csBSpline, name)
    def __repr__(self):
        return "<C csBSpline instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csBSpline, 'this', _cspace.new_csBSpline(*args))
        _swig_setattr(self, csBSpline, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csBSpline):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Calculate(*args): return _cspace.csBSpline_Calculate(*args)
    def GetInterpolatedDimension(*args): return _cspace.csBSpline_GetInterpolatedDimension(*args)

class csBSplinePtr(csBSpline):
    def __init__(self, this):
        _swig_setattr(self, csBSpline, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csBSpline, 'thisown', 0)
        _swig_setattr(self, csBSpline,self.__class__,csBSpline)
_cspace.csBSpline_swigregister(csBSplinePtr)

class csCatmullRomSpline(csBSpline):
    __swig_setmethods__ = {}
    for _s in [csBSpline]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csCatmullRomSpline, name, value)
    __swig_getmethods__ = {}
    for _s in [csBSpline]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csCatmullRomSpline, name)
    def __repr__(self):
        return "<C csCatmullRomSpline instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csCatmullRomSpline, 'this', _cspace.new_csCatmullRomSpline(*args))
        _swig_setattr(self, csCatmullRomSpline, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csCatmullRomSpline):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Clone(*args): return _cspace.csCatmullRomSpline_Clone(*args)

class csCatmullRomSplinePtr(csCatmullRomSpline):
    def __init__(self, this):
        _swig_setattr(self, csCatmullRomSpline, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csCatmullRomSpline, 'thisown', 0)
        _swig_setattr(self, csCatmullRomSpline,self.__class__,csCatmullRomSpline)
_cspace.csCatmullRomSpline_swigregister(csCatmullRomSplinePtr)

class csPoint(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPoint, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPoint, name)
    def __repr__(self):
        return "<C csPoint instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cspace.csPoint_x_set
    __swig_getmethods__["x"] = _cspace.csPoint_x_get
    if _newclass:x = property(_cspace.csPoint_x_get, _cspace.csPoint_x_set)
    __swig_setmethods__["y"] = _cspace.csPoint_y_set
    __swig_getmethods__["y"] = _cspace.csPoint_y_get
    if _newclass:y = property(_cspace.csPoint_y_get, _cspace.csPoint_y_set)
    def __init__(self, *args):
        _swig_setattr(self, csPoint, 'this', _cspace.new_csPoint(*args))
        _swig_setattr(self, csPoint, 'thisown', 1)
    def Set(*args): return _cspace.csPoint_Set(*args)
    def __del__(self, destroy=_cspace.delete_csPoint):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPointPtr(csPoint):
    def __init__(self, this):
        _swig_setattr(self, csPoint, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPoint, 'thisown', 0)
        _swig_setattr(self, csPoint,self.__class__,csPoint)
_cspace.csPoint_swigregister(csPointPtr)

CS_BOUNDINGBOX_MAXVALUE = _cspace.CS_BOUNDINGBOX_MAXVALUE
CS_BOX_CORNER_xy = _cspace.CS_BOX_CORNER_xy
CS_BOX_CORNER_xY = _cspace.CS_BOX_CORNER_xY
CS_BOX_CORNER_Xy = _cspace.CS_BOX_CORNER_Xy
CS_BOX_CORNER_XY = _cspace.CS_BOX_CORNER_XY
CS_BOX_CENTER2 = _cspace.CS_BOX_CENTER2
CS_BOX_EDGE_xy_Xy = _cspace.CS_BOX_EDGE_xy_Xy
CS_BOX_EDGE_Xy_xy = _cspace.CS_BOX_EDGE_Xy_xy
CS_BOX_EDGE_Xy_XY = _cspace.CS_BOX_EDGE_Xy_XY
CS_BOX_EDGE_XY_Xy = _cspace.CS_BOX_EDGE_XY_Xy
CS_BOX_EDGE_XY_xY = _cspace.CS_BOX_EDGE_XY_xY
CS_BOX_EDGE_xY_XY = _cspace.CS_BOX_EDGE_xY_XY
CS_BOX_EDGE_xY_xy = _cspace.CS_BOX_EDGE_xY_xy
CS_BOX_EDGE_xy_xY = _cspace.CS_BOX_EDGE_xy_xY
class csBox2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csBox2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csBox2, name)
    def __repr__(self):
        return "<C csBox2 instance at %s>" % (self.this,)
    def MinX(*args): return _cspace.csBox2_MinX(*args)
    def MinY(*args): return _cspace.csBox2_MinY(*args)
    def MaxX(*args): return _cspace.csBox2_MaxX(*args)
    def MaxY(*args): return _cspace.csBox2_MaxY(*args)
    def Min(*args): return _cspace.csBox2_Min(*args)
    def Max(*args): return _cspace.csBox2_Max(*args)
    def GetCorner(*args): return _cspace.csBox2_GetCorner(*args)
    def GetCenter(*args): return _cspace.csBox2_GetCenter(*args)
    def SetCenter(*args): return _cspace.csBox2_SetCenter(*args)
    def SetSize(*args): return _cspace.csBox2_SetSize(*args)
    def GetEdgeInfo(*args): return _cspace.csBox2_GetEdgeInfo(*args)
    def GetEdge(*args): return _cspace.csBox2_GetEdge(*args)
    __swig_getmethods__["Intersect"] = lambda x: _cspace.csBox2_Intersect
    if _newclass:Intersect = staticmethod(_cspace.csBox2_Intersect)
    __swig_getmethods__["Intersect"] = lambda x: _cspace.csBox2_Intersect
    if _newclass:Intersect = staticmethod(_cspace.csBox2_Intersect)
    def Intersect(*args): return _cspace.csBox2_Intersect(*args)
    def In(*args): return _cspace.csBox2_In(*args)
    def Overlap(*args): return _cspace.csBox2_Overlap(*args)
    def Contains(*args): return _cspace.csBox2_Contains(*args)
    def Empty(*args): return _cspace.csBox2_Empty(*args)
    def SquaredOriginDist(*args): return _cspace.csBox2_SquaredOriginDist(*args)
    def SquaredOriginMaxDist(*args): return _cspace.csBox2_SquaredOriginMaxDist(*args)
    def StartBoundingBox(*args): return _cspace.csBox2_StartBoundingBox(*args)
    def AddBoundingVertex(*args): return _cspace.csBox2_AddBoundingVertex(*args)
    def AddBoundingVertexSmart(*args): return _cspace.csBox2_AddBoundingVertexSmart(*args)
    def AddBoundingVertexTest(*args): return _cspace.csBox2_AddBoundingVertexTest(*args)
    def AddBoundingVertexSmartTest(*args): return _cspace.csBox2_AddBoundingVertexSmartTest(*args)
    def __init__(self, *args):
        _swig_setattr(self, csBox2, 'this', _cspace.new_csBox2(*args))
        _swig_setattr(self, csBox2, 'thisown', 1)
    def Set(*args): return _cspace.csBox2_Set(*args)
    def SetMin(*args): return _cspace.csBox2_SetMin(*args)
    def SetMax(*args): return _cspace.csBox2_SetMax(*args)
    def Description(*args): return _cspace.csBox2_Description(*args)
    def __iadd__(*args): return _cspace.csBox2___iadd__(*args)
    def __imul__(*args): return _cspace.csBox2___imul__(*args)
    def TestIntersect(*args): return _cspace.csBox2_TestIntersect(*args)
    def __mul__(*args): return _cspace.csBox2___mul__(*args)
    def __ne__(*args): return _cspace.csBox2___ne__(*args)
    def __gt__(*args): return _cspace.csBox2___gt__(*args)
    def __add__(*args): return _cspace.csBox2___add__(*args)
    def __lt__(*args): return _cspace.csBox2___lt__(*args)
    def __del__(self, destroy=_cspace.delete_csBox2):
        try:
            if self.thisown: destroy(self)
        except: pass

class csBox2Ptr(csBox2):
    def __init__(self, this):
        _swig_setattr(self, csBox2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csBox2, 'thisown', 0)
        _swig_setattr(self, csBox2,self.__class__,csBox2)
_cspace.csBox2_swigregister(csBox2Ptr)

CS_BOX_CORNER_xyz = _cspace.CS_BOX_CORNER_xyz
CS_BOX_CORNER_xyZ = _cspace.CS_BOX_CORNER_xyZ
CS_BOX_CORNER_xYz = _cspace.CS_BOX_CORNER_xYz
CS_BOX_CORNER_xYZ = _cspace.CS_BOX_CORNER_xYZ
CS_BOX_CORNER_Xyz = _cspace.CS_BOX_CORNER_Xyz
CS_BOX_CORNER_XyZ = _cspace.CS_BOX_CORNER_XyZ
CS_BOX_CORNER_XYz = _cspace.CS_BOX_CORNER_XYz
CS_BOX_CORNER_XYZ = _cspace.CS_BOX_CORNER_XYZ
CS_BOX_CENTER3 = _cspace.CS_BOX_CENTER3
CS_BOX_SIDE_x = _cspace.CS_BOX_SIDE_x
CS_BOX_SIDE_X = _cspace.CS_BOX_SIDE_X
CS_BOX_SIDE_y = _cspace.CS_BOX_SIDE_y
CS_BOX_SIDE_Y = _cspace.CS_BOX_SIDE_Y
CS_BOX_SIDE_z = _cspace.CS_BOX_SIDE_z
CS_BOX_SIDE_Z = _cspace.CS_BOX_SIDE_Z
CS_BOX_INSIDE = _cspace.CS_BOX_INSIDE
CS_BOX_EDGE_Xyz_xyz = _cspace.CS_BOX_EDGE_Xyz_xyz
CS_BOX_EDGE_xyz_Xyz = _cspace.CS_BOX_EDGE_xyz_Xyz
CS_BOX_EDGE_xyz_xYz = _cspace.CS_BOX_EDGE_xyz_xYz
CS_BOX_EDGE_xYz_xyz = _cspace.CS_BOX_EDGE_xYz_xyz
CS_BOX_EDGE_xYz_XYz = _cspace.CS_BOX_EDGE_xYz_XYz
CS_BOX_EDGE_XYz_xYz = _cspace.CS_BOX_EDGE_XYz_xYz
CS_BOX_EDGE_XYz_Xyz = _cspace.CS_BOX_EDGE_XYz_Xyz
CS_BOX_EDGE_Xyz_XYz = _cspace.CS_BOX_EDGE_Xyz_XYz
CS_BOX_EDGE_Xyz_XyZ = _cspace.CS_BOX_EDGE_Xyz_XyZ
CS_BOX_EDGE_XyZ_Xyz = _cspace.CS_BOX_EDGE_XyZ_Xyz
CS_BOX_EDGE_XyZ_XYZ = _cspace.CS_BOX_EDGE_XyZ_XYZ
CS_BOX_EDGE_XYZ_XyZ = _cspace.CS_BOX_EDGE_XYZ_XyZ
CS_BOX_EDGE_XYZ_XYz = _cspace.CS_BOX_EDGE_XYZ_XYz
CS_BOX_EDGE_XYz_XYZ = _cspace.CS_BOX_EDGE_XYz_XYZ
CS_BOX_EDGE_XYZ_xYZ = _cspace.CS_BOX_EDGE_XYZ_xYZ
CS_BOX_EDGE_xYZ_XYZ = _cspace.CS_BOX_EDGE_xYZ_XYZ
CS_BOX_EDGE_xYZ_xYz = _cspace.CS_BOX_EDGE_xYZ_xYz
CS_BOX_EDGE_xYz_xYZ = _cspace.CS_BOX_EDGE_xYz_xYZ
CS_BOX_EDGE_xYZ_xyZ = _cspace.CS_BOX_EDGE_xYZ_xyZ
CS_BOX_EDGE_xyZ_xYZ = _cspace.CS_BOX_EDGE_xyZ_xYZ
CS_BOX_EDGE_xyZ_xyz = _cspace.CS_BOX_EDGE_xyZ_xyz
CS_BOX_EDGE_xyz_xyZ = _cspace.CS_BOX_EDGE_xyz_xyZ
CS_BOX_EDGE_xyZ_XyZ = _cspace.CS_BOX_EDGE_xyZ_XyZ
CS_BOX_EDGE_XyZ_xyZ = _cspace.CS_BOX_EDGE_XyZ_xyZ
class csBox3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csBox3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csBox3, name)
    def __repr__(self):
        return "<C csBox3 instance at %s>" % (self.this,)
    def MinX(*args): return _cspace.csBox3_MinX(*args)
    def MinY(*args): return _cspace.csBox3_MinY(*args)
    def MinZ(*args): return _cspace.csBox3_MinZ(*args)
    def MaxX(*args): return _cspace.csBox3_MaxX(*args)
    def MaxY(*args): return _cspace.csBox3_MaxY(*args)
    def MaxZ(*args): return _cspace.csBox3_MaxZ(*args)
    def Min(*args): return _cspace.csBox3_Min(*args)
    def Max(*args): return _cspace.csBox3_Max(*args)
    def GetCorner(*args): return _cspace.csBox3_GetCorner(*args)
    def GetEdgeInfo(*args): return _cspace.csBox3_GetEdgeInfo(*args)
    def GetFaceEdges(*args): return _cspace.csBox3_GetFaceEdges(*args)
    def GetCenter(*args): return _cspace.csBox3_GetCenter(*args)
    def SetCenter(*args): return _cspace.csBox3_SetCenter(*args)
    def SetSize(*args): return _cspace.csBox3_SetSize(*args)
    def GetSide(*args): return _cspace.csBox3_GetSide(*args)
    def GetAxisPlane(*args): return _cspace.csBox3_GetAxisPlane(*args)
    def GetVisibleSides(*args): return _cspace.csBox3_GetVisibleSides(*args)
    __swig_getmethods__["OtherSide"] = lambda x: _cspace.csBox3_OtherSide
    if _newclass:OtherSide = staticmethod(_cspace.csBox3_OtherSide)
    def GetEdge(*args): return _cspace.csBox3_GetEdge(*args)
    def In(*args): return _cspace.csBox3_In(*args)
    def Overlap(*args): return _cspace.csBox3_Overlap(*args)
    def Contains(*args): return _cspace.csBox3_Contains(*args)
    def Empty(*args): return _cspace.csBox3_Empty(*args)
    def StartBoundingBox(*args): return _cspace.csBox3_StartBoundingBox(*args)
    def AddBoundingVertex(*args): return _cspace.csBox3_AddBoundingVertex(*args)
    def AddBoundingVertexSmart(*args): return _cspace.csBox3_AddBoundingVertexSmart(*args)
    def AddBoundingVertexTest(*args): return _cspace.csBox3_AddBoundingVertexTest(*args)
    def AddBoundingVertexSmartTest(*args): return _cspace.csBox3_AddBoundingVertexSmartTest(*args)
    def __init__(self, *args):
        _swig_setattr(self, csBox3, 'this', _cspace.new_csBox3(*args))
        _swig_setattr(self, csBox3, 'thisown', 1)
    def Set(*args): return _cspace.csBox3_Set(*args)
    def SetMin(*args): return _cspace.csBox3_SetMin(*args)
    def SetMax(*args): return _cspace.csBox3_SetMax(*args)
    def Description(*args): return _cspace.csBox3_Description(*args)
    def Split(*args): return _cspace.csBox3_Split(*args)
    def TestSplit(*args): return _cspace.csBox3_TestSplit(*args)
    def AdjacentX(*args): return _cspace.csBox3_AdjacentX(*args)
    def AdjacentY(*args): return _cspace.csBox3_AdjacentY(*args)
    def AdjacentZ(*args): return _cspace.csBox3_AdjacentZ(*args)
    def Adjacent(*args): return _cspace.csBox3_Adjacent(*args)
    def CalculatePointSegment(*args): return _cspace.csBox3_CalculatePointSegment(*args)
    def GetConvexOutline(*args): return _cspace.csBox3_GetConvexOutline(*args)
    def Between(*args): return _cspace.csBox3_Between(*args)
    def ManhattanDistance(*args): return _cspace.csBox3_ManhattanDistance(*args)
    def SquaredOriginDist(*args): return _cspace.csBox3_SquaredOriginDist(*args)
    def SquaredOriginMaxDist(*args): return _cspace.csBox3_SquaredOriginMaxDist(*args)
    def ProjectBox(*args): return _cspace.csBox3_ProjectBox(*args)
    def ProjectOutline(*args): return _cspace.csBox3_ProjectOutline(*args)
    def ProjectBoxAndOutline(*args): return _cspace.csBox3_ProjectBoxAndOutline(*args)
    def __iadd__(*args): return _cspace.csBox3___iadd__(*args)
    def __imul__(*args): return _cspace.csBox3___imul__(*args)
    def TestIntersect(*args): return _cspace.csBox3_TestIntersect(*args)
    def __mul__(*args): return _cspace.csBox3___mul__(*args)
    def __ne__(*args): return _cspace.csBox3___ne__(*args)
    def __gt__(*args): return _cspace.csBox3___gt__(*args)
    def __add__(*args): return _cspace.csBox3___add__(*args)
    def __lt__(*args): return _cspace.csBox3___lt__(*args)
    def __del__(self, destroy=_cspace.delete_csBox3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csBox3Ptr(csBox3):
    def __init__(self, this):
        _swig_setattr(self, csBox3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csBox3, 'thisown', 0)
        _swig_setattr(self, csBox3,self.__class__,csBox3)
_cspace.csBox3_swigregister(csBox3Ptr)

csBox3_OtherSide = _cspace.csBox3_OtherSide

class csSegment2(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSegment2, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSegment2, name)
    def __repr__(self):
        return "<C csSegment2 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csSegment2, 'this', _cspace.new_csSegment2(*args))
        _swig_setattr(self, csSegment2, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csSegment2):
        try:
            if self.thisown: destroy(self)
        except: pass
    def Set(*args): return _cspace.csSegment2_Set(*args)
    def SetStart(*args): return _cspace.csSegment2_SetStart(*args)
    def SetEnd(*args): return _cspace.csSegment2_SetEnd(*args)
    def Start(*args): return _cspace.csSegment2_Start(*args)
    def End(*args): return _cspace.csSegment2_End(*args)

class csSegment2Ptr(csSegment2):
    def __init__(self, this):
        _swig_setattr(self, csSegment2, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSegment2, 'thisown', 0)
        _swig_setattr(self, csSegment2,self.__class__,csSegment2)
_cspace.csSegment2_swigregister(csSegment2Ptr)

class csSegment3(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSegment3, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSegment3, name)
    def __repr__(self):
        return "<C csSegment3 instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csSegment3, 'this', _cspace.new_csSegment3(*args))
        _swig_setattr(self, csSegment3, 'thisown', 1)
    def Set(*args): return _cspace.csSegment3_Set(*args)
    def SetStart(*args): return _cspace.csSegment3_SetStart(*args)
    def SetEnd(*args): return _cspace.csSegment3_SetEnd(*args)
    def Start(*args): return _cspace.csSegment3_Start(*args)
    def End(*args): return _cspace.csSegment3_End(*args)
    def __del__(self, destroy=_cspace.delete_csSegment3):
        try:
            if self.thisown: destroy(self)
        except: pass

class csSegment3Ptr(csSegment3):
    def __init__(self, this):
        _swig_setattr(self, csSegment3, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSegment3, 'thisown', 0)
        _swig_setattr(self, csSegment3,self.__class__,csSegment3)
_cspace.csSegment3_swigregister(csSegment3Ptr)

class csRGBcolor(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csRGBcolor, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csRGBcolor, name)
    def __repr__(self):
        return "<C csRGBcolor instance at %s>" % (self.this,)
    __swig_setmethods__["red"] = _cspace.csRGBcolor_red_set
    __swig_getmethods__["red"] = _cspace.csRGBcolor_red_get
    if _newclass:red = property(_cspace.csRGBcolor_red_get, _cspace.csRGBcolor_red_set)
    __swig_setmethods__["green"] = _cspace.csRGBcolor_green_set
    __swig_getmethods__["green"] = _cspace.csRGBcolor_green_get
    if _newclass:green = property(_cspace.csRGBcolor_green_get, _cspace.csRGBcolor_green_set)
    __swig_setmethods__["blue"] = _cspace.csRGBcolor_blue_set
    __swig_getmethods__["blue"] = _cspace.csRGBcolor_blue_get
    if _newclass:blue = property(_cspace.csRGBcolor_blue_get, _cspace.csRGBcolor_blue_set)
    def __init__(self, *args):
        _swig_setattr(self, csRGBcolor, 'this', _cspace.new_csRGBcolor(*args))
        _swig_setattr(self, csRGBcolor, 'thisown', 1)
    def Set(*args): return _cspace.csRGBcolor_Set(*args)
    def __eq__(*args): return _cspace.csRGBcolor___eq__(*args)
    def __ne__(*args): return _cspace.csRGBcolor___ne__(*args)
    def __add__(*args): return _cspace.csRGBcolor___add__(*args)
    def __del__(self, destroy=_cspace.delete_csRGBcolor):
        try:
            if self.thisown: destroy(self)
        except: pass

class csRGBcolorPtr(csRGBcolor):
    def __init__(self, this):
        _swig_setattr(self, csRGBcolor, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csRGBcolor, 'thisown', 0)
        _swig_setattr(self, csRGBcolor,self.__class__,csRGBcolor)
_cspace.csRGBcolor_swigregister(csRGBcolorPtr)

class csRGBpixel(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csRGBpixel, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csRGBpixel, name)
    def __repr__(self):
        return "<C csRGBpixel instance at %s>" % (self.this,)
    __swig_setmethods__["red"] = _cspace.csRGBpixel_red_set
    __swig_getmethods__["red"] = _cspace.csRGBpixel_red_get
    if _newclass:red = property(_cspace.csRGBpixel_red_get, _cspace.csRGBpixel_red_set)
    __swig_setmethods__["green"] = _cspace.csRGBpixel_green_set
    __swig_getmethods__["green"] = _cspace.csRGBpixel_green_get
    if _newclass:green = property(_cspace.csRGBpixel_green_get, _cspace.csRGBpixel_green_set)
    __swig_setmethods__["blue"] = _cspace.csRGBpixel_blue_set
    __swig_getmethods__["blue"] = _cspace.csRGBpixel_blue_get
    if _newclass:blue = property(_cspace.csRGBpixel_blue_get, _cspace.csRGBpixel_blue_set)
    __swig_setmethods__["alpha"] = _cspace.csRGBpixel_alpha_set
    __swig_getmethods__["alpha"] = _cspace.csRGBpixel_alpha_get
    if _newclass:alpha = property(_cspace.csRGBpixel_alpha_get, _cspace.csRGBpixel_alpha_set)
    def __init__(self, *args):
        _swig_setattr(self, csRGBpixel, 'this', _cspace.new_csRGBpixel(*args))
        _swig_setattr(self, csRGBpixel, 'thisown', 1)
    def __eq__(*args): return _cspace.csRGBpixel___eq__(*args)
    def __ne__(*args): return _cspace.csRGBpixel___ne__(*args)
    def asRGBcolor(*args): return _cspace.csRGBpixel_asRGBcolor(*args)
    def eq(*args): return _cspace.csRGBpixel_eq(*args)
    def Intensity(*args): return _cspace.csRGBpixel_Intensity(*args)
    def Luminance(*args): return _cspace.csRGBpixel_Luminance(*args)
    def Set(*args): return _cspace.csRGBpixel_Set(*args)
    def __iadd__(*args): return _cspace.csRGBpixel___iadd__(*args)
    def UnsafeAdd(*args): return _cspace.csRGBpixel_UnsafeAdd(*args)
    def SafeAdd(*args): return _cspace.csRGBpixel_SafeAdd(*args)
    def __del__(self, destroy=_cspace.delete_csRGBpixel):
        try:
            if self.thisown: destroy(self)
        except: pass

class csRGBpixelPtr(csRGBpixel):
    def __init__(self, this):
        _swig_setattr(self, csRGBpixel, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csRGBpixel, 'thisown', 0)
        _swig_setattr(self, csRGBpixel,self.__class__,csRGBpixel)
_cspace.csRGBpixel_swigregister(csRGBpixelPtr)

R_COEF = _cspace.R_COEF
G_COEF = _cspace.G_COEF
B_COEF = _cspace.B_COEF
R_COEF_SQ = _cspace.R_COEF_SQ
G_COEF_SQ = _cspace.G_COEF_SQ
B_COEF_SQ = _cspace.B_COEF_SQ

csDefaultRunLoop = _cspace.csDefaultRunLoop

csPlatformStartup = _cspace.csPlatformStartup

csPlatformShutdown = _cspace.csPlatformShutdown

csPrintf = _cspace.csPrintf

csFPutErr = _cspace.csFPutErr

csPrintfErr = _cspace.csPrintfErr

csGetTicks = _cspace.csGetTicks

csSleep = _cspace.csSleep

csGetUsername = _cspace.csGetUsername

csGetPlatformConfigPath = _cspace.csGetPlatformConfigPath
class csPluginRequest(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPluginRequest, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPluginRequest, name)
    def __repr__(self):
        return "<C csPluginRequest instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csPluginRequest, 'this', _cspace.new_csPluginRequest(*args))
        _swig_setattr(self, csPluginRequest, 'thisown', 1)
    def assign(*args): return _cspace.csPluginRequest_assign(*args)
    def __eq__(*args): return _cspace.csPluginRequest___eq__(*args)
    def __ne__(*args): return _cspace.csPluginRequest___ne__(*args)
    def GetClassName(*args): return _cspace.csPluginRequest_GetClassName(*args)
    def GetInterfaceName(*args): return _cspace.csPluginRequest_GetInterfaceName(*args)
    def GetInterfaceID(*args): return _cspace.csPluginRequest_GetInterfaceID(*args)
    def GetInterfaceVersion(*args): return _cspace.csPluginRequest_GetInterfaceVersion(*args)
    def __del__(self, destroy=_cspace.delete_csPluginRequest):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPluginRequestPtr(csPluginRequest):
    def __init__(self, this):
        _swig_setattr(self, csPluginRequest, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPluginRequest, 'thisown', 0)
        _swig_setattr(self, csPluginRequest,self.__class__,csPluginRequest)
_cspace.csPluginRequest_swigregister(csPluginRequestPtr)

class csInitializer(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csInitializer, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csInitializer, name)
    def __repr__(self):
        return "<C csInitializer instance at %s>" % (self.this,)
    __swig_getmethods__["CreateEnvironment"] = lambda x: _cspace.csInitializer_CreateEnvironment
    if _newclass:CreateEnvironment = staticmethod(_cspace.csInitializer_CreateEnvironment)
    __swig_getmethods__["InitializeSCF"] = lambda x: _cspace.csInitializer_InitializeSCF
    if _newclass:InitializeSCF = staticmethod(_cspace.csInitializer_InitializeSCF)
    __swig_getmethods__["CreateObjectRegistry"] = lambda x: _cspace.csInitializer_CreateObjectRegistry
    if _newclass:CreateObjectRegistry = staticmethod(_cspace.csInitializer_CreateObjectRegistry)
    __swig_getmethods__["CreatePluginManager"] = lambda x: _cspace.csInitializer_CreatePluginManager
    if _newclass:CreatePluginManager = staticmethod(_cspace.csInitializer_CreatePluginManager)
    __swig_getmethods__["CreateEventQueue"] = lambda x: _cspace.csInitializer_CreateEventQueue
    if _newclass:CreateEventQueue = staticmethod(_cspace.csInitializer_CreateEventQueue)
    __swig_getmethods__["CreateVirtualClock"] = lambda x: _cspace.csInitializer_CreateVirtualClock
    if _newclass:CreateVirtualClock = staticmethod(_cspace.csInitializer_CreateVirtualClock)
    __swig_getmethods__["CreateCommandLineParser"] = lambda x: _cspace.csInitializer_CreateCommandLineParser
    if _newclass:CreateCommandLineParser = staticmethod(_cspace.csInitializer_CreateCommandLineParser)
    __swig_getmethods__["CreateVerbosityManager"] = lambda x: _cspace.csInitializer_CreateVerbosityManager
    if _newclass:CreateVerbosityManager = staticmethod(_cspace.csInitializer_CreateVerbosityManager)
    __swig_getmethods__["CreateConfigManager"] = lambda x: _cspace.csInitializer_CreateConfigManager
    if _newclass:CreateConfigManager = staticmethod(_cspace.csInitializer_CreateConfigManager)
    __swig_getmethods__["CreateInputDrivers"] = lambda x: _cspace.csInitializer_CreateInputDrivers
    if _newclass:CreateInputDrivers = staticmethod(_cspace.csInitializer_CreateInputDrivers)
    __swig_getmethods__["CreateStringSet"] = lambda x: _cspace.csInitializer_CreateStringSet
    if _newclass:CreateStringSet = staticmethod(_cspace.csInitializer_CreateStringSet)
    __swig_getmethods__["SetupConfigManager"] = lambda x: _cspace.csInitializer_SetupConfigManager
    if _newclass:SetupConfigManager = staticmethod(_cspace.csInitializer_SetupConfigManager)
    __swig_getmethods__["SetupVFS"] = lambda x: _cspace.csInitializer_SetupVFS
    if _newclass:SetupVFS = staticmethod(_cspace.csInitializer_SetupVFS)
    __swig_getmethods__["_RequestPlugins"] = lambda x: _cspace.csInitializer__RequestPlugins
    if _newclass:_RequestPlugins = staticmethod(_cspace.csInitializer__RequestPlugins)
    __swig_getmethods__["OpenApplication"] = lambda x: _cspace.csInitializer_OpenApplication
    if _newclass:OpenApplication = staticmethod(_cspace.csInitializer_OpenApplication)
    __swig_getmethods__["CloseApplication"] = lambda x: _cspace.csInitializer_CloseApplication
    if _newclass:CloseApplication = staticmethod(_cspace.csInitializer_CloseApplication)
    __swig_getmethods__["_SetupEventHandler"] = lambda x: _cspace.csInitializer__SetupEventHandler
    if _newclass:_SetupEventHandler = staticmethod(_cspace.csInitializer__SetupEventHandler)
    __swig_getmethods__["DestroyApplication"] = lambda x: _cspace.csInitializer_DestroyApplication
    if _newclass:DestroyApplication = staticmethod(_cspace.csInitializer_DestroyApplication)
    def __init__(self, *args):
        _swig_setattr(self, csInitializer, 'this', _cspace.new_csInitializer(*args))
        _swig_setattr(self, csInitializer, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csInitializer):
        try:
            if self.thisown: destroy(self)
        except: pass

class csInitializerPtr(csInitializer):
    def __init__(self, this):
        _swig_setattr(self, csInitializer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csInitializer, 'thisown', 0)
        _swig_setattr(self, csInitializer,self.__class__,csInitializer)
_cspace.csInitializer_swigregister(csInitializerPtr)

csInitializer_CreateEnvironment = _cspace.csInitializer_CreateEnvironment

csInitializer_InitializeSCF = _cspace.csInitializer_InitializeSCF

csInitializer_CreateObjectRegistry = _cspace.csInitializer_CreateObjectRegistry

csInitializer_CreatePluginManager = _cspace.csInitializer_CreatePluginManager

csInitializer_CreateEventQueue = _cspace.csInitializer_CreateEventQueue

csInitializer_CreateVirtualClock = _cspace.csInitializer_CreateVirtualClock

csInitializer_CreateCommandLineParser = _cspace.csInitializer_CreateCommandLineParser

csInitializer_CreateVerbosityManager = _cspace.csInitializer_CreateVerbosityManager

csInitializer_CreateConfigManager = _cspace.csInitializer_CreateConfigManager

csInitializer_CreateInputDrivers = _cspace.csInitializer_CreateInputDrivers

csInitializer_CreateStringSet = _cspace.csInitializer_CreateStringSet

csInitializer_SetupConfigManager = _cspace.csInitializer_SetupConfigManager

csInitializer_SetupVFS = _cspace.csInitializer_SetupVFS

csInitializer__RequestPlugins = _cspace.csInitializer__RequestPlugins

csInitializer_OpenApplication = _cspace.csInitializer_OpenApplication

csInitializer_CloseApplication = _cspace.csInitializer_CloseApplication

csInitializer__SetupEventHandler = _cspace.csInitializer__SetupEventHandler

csInitializer_DestroyApplication = _cspace.csInitializer_DestroyApplication

class csPluginRequestArray(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPluginRequestArray, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPluginRequestArray, name)
    def __repr__(self):
        return "<C csArray<(csPluginRequest)> instance at %s>" % (self.this,)
    def __del__(self, destroy=_cspace.delete_csPluginRequestArray):
        try:
            if self.thisown: destroy(self)
        except: pass
    def __init__(self, *args):
        _swig_setattr(self, csPluginRequestArray, 'this', _cspace.new_csPluginRequestArray(*args))
        _swig_setattr(self, csPluginRequestArray, 'thisown', 1)
    def Length(*args): return _cspace.csPluginRequestArray_Length(*args)
    def Get(*args): return _cspace.csPluginRequestArray_Get(*args)
    def Push(*args): return _cspace.csPluginRequestArray_Push(*args)
    def Pop(*args): return _cspace.csPluginRequestArray_Pop(*args)
    def Top(*args): return _cspace.csPluginRequestArray_Top(*args)
    def Insert(*args): return _cspace.csPluginRequestArray_Insert(*args)
    def Truncate(*args): return _cspace.csPluginRequestArray_Truncate(*args)
    def Empty(*args): return _cspace.csPluginRequestArray_Empty(*args)
    def DeleteIndex(*args): return _cspace.csPluginRequestArray_DeleteIndex(*args)
    def DeleteIndexFast(*args): return _cspace.csPluginRequestArray_DeleteIndexFast(*args)
    def DeleteRange(*args): return _cspace.csPluginRequestArray_DeleteRange(*args)
    def DeleteFast(*args): return _cspace.csPluginRequestArray_DeleteFast(*args)

class csPluginRequestArrayPtr(csPluginRequestArray):
    def __init__(self, this):
        _swig_setattr(self, csPluginRequestArray, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPluginRequestArray, 'thisown', 0)
        _swig_setattr(self, csPluginRequestArray,self.__class__,csPluginRequestArray)
_cspace.csPluginRequestArray_swigregister(csPluginRequestArrayPtr)

class iAwsKey(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsKey instance at %s>" % (self.this,)
    def Type(*args): return _cspace.iAwsKey_Type(*args)
    def Name(*args): return _cspace.iAwsKey_Name(*args)
    def __del__(self, destroy=_cspace.delete_iAwsKey):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iAwsKey_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iAwsKey_scfGetVersion)

class iAwsKeyPtr(iAwsKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsKey, 'thisown', 0)
        _swig_setattr(self, iAwsKey,self.__class__,iAwsKey)
_cspace.iAwsKey_swigregister(iAwsKeyPtr)
aws_debug = cvar.aws_debug
AWSF_AlwaysEraseWindows = cvar.AWSF_AlwaysEraseWindows
AWSF_AlwaysRedrawWindows = cvar.AWSF_AlwaysRedrawWindows
AWSF_RaiseOnMouseOver = cvar.AWSF_RaiseOnMouseOver
AWSF_KeyboardControl = cvar.AWSF_KeyboardControl

iAwsKey_scfGetVersion = _cspace.iAwsKey_scfGetVersion

class iAwsIntKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsIntKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsIntKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsIntKey instance at %s>" % (self.this,)
    def Value(*args): return _cspace.iAwsIntKey_Value(*args)
    def __del__(self, destroy=_cspace.delete_iAwsIntKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsIntKeyPtr(iAwsIntKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsIntKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsIntKey, 'thisown', 0)
        _swig_setattr(self, iAwsIntKey,self.__class__,iAwsIntKey)
_cspace.iAwsIntKey_swigregister(iAwsIntKeyPtr)

class iAwsFloatKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsFloatKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsFloatKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsFloatKey instance at %s>" % (self.this,)
    def Value(*args): return _cspace.iAwsFloatKey_Value(*args)
    def __del__(self, destroy=_cspace.delete_iAwsFloatKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsFloatKeyPtr(iAwsFloatKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsFloatKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsFloatKey, 'thisown', 0)
        _swig_setattr(self, iAwsFloatKey,self.__class__,iAwsFloatKey)
_cspace.iAwsFloatKey_swigregister(iAwsFloatKeyPtr)

class iAwsStringKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsStringKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsStringKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsStringKey instance at %s>" % (self.this,)
    def Value(*args): return _cspace.iAwsStringKey_Value(*args)
    def __del__(self, destroy=_cspace.delete_iAwsStringKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsStringKeyPtr(iAwsStringKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsStringKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsStringKey, 'thisown', 0)
        _swig_setattr(self, iAwsStringKey,self.__class__,iAwsStringKey)
_cspace.iAwsStringKey_swigregister(iAwsStringKeyPtr)

class iAwsRectKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsRectKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsRectKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsRectKey instance at %s>" % (self.this,)
    def Value(*args): return _cspace.iAwsRectKey_Value(*args)
    def __del__(self, destroy=_cspace.delete_iAwsRectKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsRectKeyPtr(iAwsRectKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsRectKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsRectKey, 'thisown', 0)
        _swig_setattr(self, iAwsRectKey,self.__class__,iAwsRectKey)
_cspace.iAwsRectKey_swigregister(iAwsRectKeyPtr)

class iAwsRGBKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsRGBKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsRGBKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsRGBKey instance at %s>" % (self.this,)
    def Value(*args): return _cspace.iAwsRGBKey_Value(*args)
    def __del__(self, destroy=_cspace.delete_iAwsRGBKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsRGBKeyPtr(iAwsRGBKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsRGBKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsRGBKey, 'thisown', 0)
        _swig_setattr(self, iAwsRGBKey,self.__class__,iAwsRGBKey)
_cspace.iAwsRGBKey_swigregister(iAwsRGBKeyPtr)

class iAwsPointKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsPointKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsPointKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsPointKey instance at %s>" % (self.this,)
    def Value(*args): return _cspace.iAwsPointKey_Value(*args)
    def __del__(self, destroy=_cspace.delete_iAwsPointKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsPointKeyPtr(iAwsPointKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsPointKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsPointKey, 'thisown', 0)
        _swig_setattr(self, iAwsPointKey,self.__class__,iAwsPointKey)
_cspace.iAwsPointKey_swigregister(iAwsPointKeyPtr)

class iAwsConnectionKey(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsConnectionKey, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsConnectionKey, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsConnectionKey instance at %s>" % (self.this,)
    def Sink(*args): return _cspace.iAwsConnectionKey_Sink(*args)
    def Trigger(*args): return _cspace.iAwsConnectionKey_Trigger(*args)
    def Signal(*args): return _cspace.iAwsConnectionKey_Signal(*args)
    def __del__(self, destroy=_cspace.delete_iAwsConnectionKey):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsConnectionKeyPtr(iAwsConnectionKey):
    def __init__(self, this):
        _swig_setattr(self, iAwsConnectionKey, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsConnectionKey, 'thisown', 0)
        _swig_setattr(self, iAwsConnectionKey,self.__class__,iAwsConnectionKey)
_cspace.iAwsConnectionKey_swigregister(iAwsConnectionKeyPtr)

class iAwsKeyContainer(iAwsKey):
    __swig_setmethods__ = {}
    for _s in [iAwsKey]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsKeyContainer, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKey]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsKeyContainer, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsKeyContainer instance at %s>" % (self.this,)
    def Find(*args): return _cspace.iAwsKeyContainer_Find(*args)
    def Children(*args): return _cspace.iAwsKeyContainer_Children(*args)
    def Add(*args): return _cspace.iAwsKeyContainer_Add(*args)
    def GetAt(*args): return _cspace.iAwsKeyContainer_GetAt(*args)
    def Length(*args): return _cspace.iAwsKeyContainer_Length(*args)
    def Remove(*args): return _cspace.iAwsKeyContainer_Remove(*args)
    def RemoveAll(*args): return _cspace.iAwsKeyContainer_RemoveAll(*args)
    def Consume(*args): return _cspace.iAwsKeyContainer_Consume(*args)
    def __del__(self, destroy=_cspace.delete_iAwsKeyContainer):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsKeyContainerPtr(iAwsKeyContainer):
    def __init__(self, this):
        _swig_setattr(self, iAwsKeyContainer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsKeyContainer, 'thisown', 0)
        _swig_setattr(self, iAwsKeyContainer,self.__class__,iAwsKeyContainer)
_cspace.iAwsKeyContainer_swigregister(iAwsKeyContainerPtr)

class iAwsComponentNode(iAwsKeyContainer):
    __swig_setmethods__ = {}
    for _s in [iAwsKeyContainer]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsComponentNode, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsKeyContainer]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsComponentNode, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsComponentNode instance at %s>" % (self.this,)
    def ComponentTypeName(*args): return _cspace.iAwsComponentNode_ComponentTypeName(*args)
    def __del__(self, destroy=_cspace.delete_iAwsComponentNode):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsComponentNodePtr(iAwsComponentNode):
    def __init__(self, this):
        _swig_setattr(self, iAwsComponentNode, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsComponentNode, 'thisown', 0)
        _swig_setattr(self, iAwsComponentNode,self.__class__,iAwsComponentNode)
_cspace.iAwsComponentNode_swigregister(iAwsComponentNodePtr)

class iAws(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAws, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAws, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAws instance at %s>" % (self.this,)
    def GetPrefMgr(*args): return _cspace.iAws_GetPrefMgr(*args)
    def GetSinkMgr(*args): return _cspace.iAws_GetSinkMgr(*args)
    def SetPrefMgr(*args): return _cspace.iAws_SetPrefMgr(*args)
    def RegisterComponentFactory(*args): return _cspace.iAws_RegisterComponentFactory(*args)
    def FindComponentFactory(*args): return _cspace.iAws_FindComponentFactory(*args)
    def GetTopComponent(*args): return _cspace.iAws_GetTopComponent(*args)
    def SetTopComponent(*args): return _cspace.iAws_SetTopComponent(*args)
    def GetFocusedComponent(*args): return _cspace.iAws_GetFocusedComponent(*args)
    def SetFocusedComponent(*args): return _cspace.iAws_SetFocusedComponent(*args)
    def GetKeyboardFocusedComponent(*args): return _cspace.iAws_GetKeyboardFocusedComponent(*args)
    def ComponentAt(*args): return _cspace.iAws_ComponentAt(*args)
    def MouseInComponent(*args): return _cspace.iAws_MouseInComponent(*args)
    def Print(*args): return _cspace.iAws_Print(*args)
    def Redraw(*args): return _cspace.iAws_Redraw(*args)
    def Mark(*args): return _cspace.iAws_Mark(*args)
    def Unmark(*args): return _cspace.iAws_Unmark(*args)
    def Erase(*args): return _cspace.iAws_Erase(*args)
    def MaskEraser(*args): return _cspace.iAws_MaskEraser(*args)
    def InvalidateUpdateStore(*args): return _cspace.iAws_InvalidateUpdateStore(*args)
    def CaptureMouse(*args): return _cspace.iAws_CaptureMouse(*args)
    def ReleaseMouse(*args): return _cspace.iAws_ReleaseMouse(*args)
    def SetModal(*args): return _cspace.iAws_SetModal(*args)
    def UnSetModal(*args): return _cspace.iAws_UnSetModal(*args)
    def HandleEvent(*args): return _cspace.iAws_HandleEvent(*args)
    def GetCanvas(*args): return _cspace.iAws_GetCanvas(*args)
    def G2D(*args): return _cspace.iAws_G2D(*args)
    def G3D(*args): return _cspace.iAws_G3D(*args)
    def CreateWindowFrom(*args): return _cspace.iAws_CreateWindowFrom(*args)
    def CreateEmbeddableComponent(*args): return _cspace.iAws_CreateEmbeddableComponent(*args)
    def CreateParmList(*args): return _cspace.iAws_CreateParmList(*args)
    def CreateTransition(*args): return _cspace.iAws_CreateTransition(*args)
    def CreateTransitionEx(*args): return _cspace.iAws_CreateTransitionEx(*args)
    def SetFlag(*args): return _cspace.iAws_SetFlag(*args)
    def ClearFlag(*args): return _cspace.iAws_ClearFlag(*args)
    def GetFlags(*args): return _cspace.iAws_GetFlags(*args)
    def GetObjectRegistry(*args): return _cspace.iAws_GetObjectRegistry(*args)
    def AllWindowsHidden(*args): return _cspace.iAws_AllWindowsHidden(*args)
    def ComponentIsInTransition(*args): return _cspace.iAws_ComponentIsInTransition(*args)
    def ComponentDestroyed(*args): return _cspace.iAws_ComponentDestroyed(*args)
    def __del__(self, destroy=_cspace.delete_iAws):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iAws_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iAws_scfGetVersion)
    def SetupCanvas(*args): return _cspace.iAws_SetupCanvas(*args)

class iAwsPtr(iAws):
    def __init__(self, this):
        _swig_setattr(self, iAws, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAws, 'thisown', 0)
        _swig_setattr(self, iAws,self.__class__,iAws)
_cspace.iAws_swigregister(iAwsPtr)

iAws_scfGetVersion = _cspace.iAws_scfGetVersion

class iAwsPrefManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsPrefManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsPrefManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsPrefManager instance at %s>" % (self.this,)
    def Setup(*args): return _cspace.iAwsPrefManager_Setup(*args)
    def Load(*args): return _cspace.iAwsPrefManager_Load(*args)
    def NameToId(*args): return _cspace.iAwsPrefManager_NameToId(*args)
    def SelectDefaultSkin(*args): return _cspace.iAwsPrefManager_SelectDefaultSkin(*args)
    def LookupIntKey(*args): return _cspace.iAwsPrefManager_LookupIntKey(*args)
    def LookupStringKey(*args): return _cspace.iAwsPrefManager_LookupStringKey(*args)
    def LookupRectKey(*args): return _cspace.iAwsPrefManager_LookupRectKey(*args)
    def LookupRGBKey(*args): return _cspace.iAwsPrefManager_LookupRGBKey(*args)
    def LookupPointKey(*args): return _cspace.iAwsPrefManager_LookupPointKey(*args)
    def GetInt(*args): return _cspace.iAwsPrefManager_GetInt(*args)
    def GetFloat(*args): return _cspace.iAwsPrefManager_GetFloat(*args)
    def GetRect(*args): return _cspace.iAwsPrefManager_GetRect(*args)
    def GetString(*args): return _cspace.iAwsPrefManager_GetString(*args)
    def GetRGB(*args): return _cspace.iAwsPrefManager_GetRGB(*args)
    def FindWindowDef(*args): return _cspace.iAwsPrefManager_FindWindowDef(*args)
    def FindSkinDef(*args): return _cspace.iAwsPrefManager_FindSkinDef(*args)
    def RemoveWindowDef(*args): return _cspace.iAwsPrefManager_RemoveWindowDef(*args)
    def RemoveAllWindowDefs(*args): return _cspace.iAwsPrefManager_RemoveAllWindowDefs(*args)
    def RemoveSkinDef(*args): return _cspace.iAwsPrefManager_RemoveSkinDef(*args)
    def RemoveAllSkinDefs(*args): return _cspace.iAwsPrefManager_RemoveAllSkinDefs(*args)
    def SetColor(*args): return _cspace.iAwsPrefManager_SetColor(*args)
    def GetColor(*args): return _cspace.iAwsPrefManager_GetColor(*args)
    def FindColor(*args): return _cspace.iAwsPrefManager_FindColor(*args)
    def GetDefaultFont(*args): return _cspace.iAwsPrefManager_GetDefaultFont(*args)
    def GetFont(*args): return _cspace.iAwsPrefManager_GetFont(*args)
    def GetTexture(*args): return _cspace.iAwsPrefManager_GetTexture(*args)
    def SetTextureManager(*args): return _cspace.iAwsPrefManager_SetTextureManager(*args)
    def SetFontServer(*args): return _cspace.iAwsPrefManager_SetFontServer(*args)
    def SetDefaultFont(*args): return _cspace.iAwsPrefManager_SetDefaultFont(*args)
    def SetWindowMgr(*args): return _cspace.iAwsPrefManager_SetWindowMgr(*args)
    def SetupPalette(*args): return _cspace.iAwsPrefManager_SetupPalette(*args)
    def RegisterConstant(*args): return _cspace.iAwsPrefManager_RegisterConstant(*args)
    def ConstantExists(*args): return _cspace.iAwsPrefManager_ConstantExists(*args)
    def GetConstantValue(*args): return _cspace.iAwsPrefManager_GetConstantValue(*args)
    def CreateKeyFactory(*args): return _cspace.iAwsPrefManager_CreateKeyFactory(*args)
    def CreateConnectionNodeFactory(*args): return _cspace.iAwsPrefManager_CreateConnectionNodeFactory(*args)
    def __del__(self, destroy=_cspace.delete_iAwsPrefManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsPrefManagerPtr(iAwsPrefManager):
    def __init__(self, this):
        _swig_setattr(self, iAwsPrefManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsPrefManager, 'thisown', 0)
        _swig_setattr(self, iAwsPrefManager,self.__class__,iAwsPrefManager)
_cspace.iAwsPrefManager_swigregister(iAwsPrefManagerPtr)

class iAwsSinkManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsSinkManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsSinkManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsSinkManager instance at %s>" % (self.this,)
    def RegisterSink(*args): return _cspace.iAwsSinkManager_RegisterSink(*args)
    def RemoveSink(*args): return _cspace.iAwsSinkManager_RemoveSink(*args)
    def FindSink(*args): return _cspace.iAwsSinkManager_FindSink(*args)
    def CreateSink(*args): return _cspace.iAwsSinkManager_CreateSink(*args)
    def CreateSlot(*args): return _cspace.iAwsSinkManager_CreateSlot(*args)
    def __del__(self, destroy=_cspace.delete_iAwsSinkManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsSinkManagerPtr(iAwsSinkManager):
    def __init__(self, this):
        _swig_setattr(self, iAwsSinkManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsSinkManager, 'thisown', 0)
        _swig_setattr(self, iAwsSinkManager,self.__class__,iAwsSinkManager)
_cspace.iAwsSinkManager_swigregister(iAwsSinkManagerPtr)

class iAwsSink(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsSink, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsSink, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsSink instance at %s>" % (self.this,)
    def GetTriggerID(*args): return _cspace.iAwsSink_GetTriggerID(*args)
    def HandleTrigger(*args): return _cspace.iAwsSink_HandleTrigger(*args)
    def RegisterTrigger(*args): return _cspace.iAwsSink_RegisterTrigger(*args)
    def GetError(*args): return _cspace.iAwsSink_GetError(*args)
    def __del__(self, destroy=_cspace.delete_iAwsSink):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsSinkPtr(iAwsSink):
    def __init__(self, this):
        _swig_setattr(self, iAwsSink, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsSink, 'thisown', 0)
        _swig_setattr(self, iAwsSink,self.__class__,iAwsSink)
_cspace.iAwsSink_swigregister(iAwsSinkPtr)

class iAwsSource(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsSource, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsSource, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsSource instance at %s>" % (self.this,)
    def GetComponent(*args): return _cspace.iAwsSource_GetComponent(*args)
    def RegisterSlot(*args): return _cspace.iAwsSource_RegisterSlot(*args)
    def UnregisterSlot(*args): return _cspace.iAwsSource_UnregisterSlot(*args)
    def Broadcast(*args): return _cspace.iAwsSource_Broadcast(*args)
    def __del__(self, destroy=_cspace.delete_iAwsSource):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsSourcePtr(iAwsSource):
    def __init__(self, this):
        _swig_setattr(self, iAwsSource, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsSource, 'thisown', 0)
        _swig_setattr(self, iAwsSource,self.__class__,iAwsSource)
_cspace.iAwsSource_swigregister(iAwsSourcePtr)

class iAwsSlot(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsSlot, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsSlot, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsSlot instance at %s>" % (self.this,)
    def Connect(*args): return _cspace.iAwsSlot_Connect(*args)
    def Disconnect(*args): return _cspace.iAwsSlot_Disconnect(*args)
    def Emit(*args): return _cspace.iAwsSlot_Emit(*args)
    def __del__(self, destroy=_cspace.delete_iAwsSlot):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsSlotPtr(iAwsSlot):
    def __init__(self, this):
        _swig_setattr(self, iAwsSlot, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsSlot, 'thisown', 0)
        _swig_setattr(self, iAwsSlot,self.__class__,iAwsSlot)
_cspace.iAwsSlot_swigregister(iAwsSlotPtr)

class iAwsLayoutManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsLayoutManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsLayoutManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsLayoutManager instance at %s>" % (self.this,)
    def SetOwner(*args): return _cspace.iAwsLayoutManager_SetOwner(*args)
    def AddComponent(*args): return _cspace.iAwsLayoutManager_AddComponent(*args)
    def RemoveComponent(*args): return _cspace.iAwsLayoutManager_RemoveComponent(*args)
    def LayoutComponents(*args): return _cspace.iAwsLayoutManager_LayoutComponents(*args)
    def __del__(self, destroy=_cspace.delete_iAwsLayoutManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsLayoutManagerPtr(iAwsLayoutManager):
    def __init__(self, this):
        _swig_setattr(self, iAwsLayoutManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsLayoutManager, 'thisown', 0)
        _swig_setattr(self, iAwsLayoutManager,self.__class__,iAwsLayoutManager)
_cspace.iAwsLayoutManager_swigregister(iAwsLayoutManagerPtr)

class iAwsComponent(iAwsSource):
    __swig_setmethods__ = {}
    for _s in [iAwsSource]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsComponent, name, value)
    __swig_getmethods__ = {}
    for _s in [iAwsSource]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsComponent, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsComponent instance at %s>" % (self.this,)
    def Create(*args): return _cspace.iAwsComponent_Create(*args)
    def Setup(*args): return _cspace.iAwsComponent_Setup(*args)
    def HandleEvent(*args): return _cspace.iAwsComponent_HandleEvent(*args)
    def GetProperty(*args): return _cspace.iAwsComponent_GetProperty(*args)
    def SetProperty(*args): return _cspace.iAwsComponent_SetProperty(*args)
    def Execute(*args): return _cspace.iAwsComponent_Execute(*args)
    def Invalidate(*args): return _cspace.iAwsComponent_Invalidate(*args)
    def Frame(*args): return _cspace.iAwsComponent_Frame(*args)
    def ClientFrame(*args): return _cspace.iAwsComponent_ClientFrame(*args)
    def Type(*args): return _cspace.iAwsComponent_Type(*args)
    def SetFlag(*args): return _cspace.iAwsComponent_SetFlag(*args)
    def ClearFlag(*args): return _cspace.iAwsComponent_ClearFlag(*args)
    def Flags(*args): return _cspace.iAwsComponent_Flags(*args)
    def WindowManager(*args): return _cspace.iAwsComponent_WindowManager(*args)
    def Parent(*args): return _cspace.iAwsComponent_Parent(*args)
    def Window(*args): return _cspace.iAwsComponent_Window(*args)
    def Layout(*args): return _cspace.iAwsComponent_Layout(*args)
    def SetParent(*args): return _cspace.iAwsComponent_SetParent(*args)
    def SetLayout(*args): return _cspace.iAwsComponent_SetLayout(*args)
    def AddToLayout(*args): return _cspace.iAwsComponent_AddToLayout(*args)
    def getPreferredSize(*args): return _cspace.iAwsComponent_getPreferredSize(*args)
    def setPreferredSize(*args): return _cspace.iAwsComponent_setPreferredSize(*args)
    def clearPreferredSize(*args): return _cspace.iAwsComponent_clearPreferredSize(*args)
    def getMinimumSize(*args): return _cspace.iAwsComponent_getMinimumSize(*args)
    def getInsets(*args): return _cspace.iAwsComponent_getInsets(*args)
    def Overlaps(*args): return _cspace.iAwsComponent_Overlaps(*args)
    def isHidden(*args): return _cspace.iAwsComponent_isHidden(*args)
    def SetFocusable(*args): return _cspace.iAwsComponent_SetFocusable(*args)
    def Focusable(*args): return _cspace.iAwsComponent_Focusable(*args)
    def isFocused(*args): return _cspace.iAwsComponent_isFocused(*args)
    def IsMaximized(*args): return _cspace.iAwsComponent_IsMaximized(*args)
    def Hide(*args): return _cspace.iAwsComponent_Hide(*args)
    def Show(*args): return _cspace.iAwsComponent_Show(*args)
    def SetFocus(*args): return _cspace.iAwsComponent_SetFocus(*args)
    def UnsetFocus(*args): return _cspace.iAwsComponent_UnsetFocus(*args)
    def Move(*args): return _cspace.iAwsComponent_Move(*args)
    def MoveTo(*args): return _cspace.iAwsComponent_MoveTo(*args)
    def Resize(*args): return _cspace.iAwsComponent_Resize(*args)
    def ResizeTo(*args): return _cspace.iAwsComponent_ResizeTo(*args)
    def Maximize(*args): return _cspace.iAwsComponent_Maximize(*args)
    def UnMaximize(*args): return _cspace.iAwsComponent_UnMaximize(*args)
    def LayoutChildren(*args): return _cspace.iAwsComponent_LayoutChildren(*args)
    def isDeaf(*args): return _cspace.iAwsComponent_isDeaf(*args)
    def SetDeaf(*args): return _cspace.iAwsComponent_SetDeaf(*args)
    def GetID(*args): return _cspace.iAwsComponent_GetID(*args)
    def SetID(*args): return _cspace.iAwsComponent_SetID(*args)
    def FindChild(*args): return _cspace.iAwsComponent_FindChild(*args)
    def DoFindChild(*args): return _cspace.iAwsComponent_DoFindChild(*args)
    def ChildAt(*args): return _cspace.iAwsComponent_ChildAt(*args)
    def AddChild(*args): return _cspace.iAwsComponent_AddChild(*args)
    def RemoveChild(*args): return _cspace.iAwsComponent_RemoveChild(*args)
    def GetChildCount(*args): return _cspace.iAwsComponent_GetChildCount(*args)
    def GetTopChild(*args): return _cspace.iAwsComponent_GetTopChild(*args)
    def ComponentAbove(*args): return _cspace.iAwsComponent_ComponentAbove(*args)
    def ComponentBelow(*args): return _cspace.iAwsComponent_ComponentBelow(*args)
    def SetComponentAbove(*args): return _cspace.iAwsComponent_SetComponentAbove(*args)
    def SetComponentBelow(*args): return _cspace.iAwsComponent_SetComponentBelow(*args)
    def AddToTabOrder(*args): return _cspace.iAwsComponent_AddToTabOrder(*args)
    def TabNext(*args): return _cspace.iAwsComponent_TabNext(*args)
    def TabPrev(*args): return _cspace.iAwsComponent_TabPrev(*args)
    def GetTabLength(*args): return _cspace.iAwsComponent_GetTabLength(*args)
    def GetTabComponent(*args): return _cspace.iAwsComponent_GetTabComponent(*args)
    def GetFirstFocusableChild(*args): return _cspace.iAwsComponent_GetFirstFocusableChild(*args)
    def Raise(*args): return _cspace.iAwsComponent_Raise(*args)
    def Lower(*args): return _cspace.iAwsComponent_Lower(*args)
    def HasChildren(*args): return _cspace.iAwsComponent_HasChildren(*args)
    def SetRedrawTag(*args): return _cspace.iAwsComponent_SetRedrawTag(*args)
    def RedrawTag(*args): return _cspace.iAwsComponent_RedrawTag(*args)
    def OnDraw(*args): return _cspace.iAwsComponent_OnDraw(*args)
    def OnMouseDown(*args): return _cspace.iAwsComponent_OnMouseDown(*args)
    def OnMouseUp(*args): return _cspace.iAwsComponent_OnMouseUp(*args)
    def OnMouseMove(*args): return _cspace.iAwsComponent_OnMouseMove(*args)
    def OnMouseClick(*args): return _cspace.iAwsComponent_OnMouseClick(*args)
    def OnMouseDoubleClick(*args): return _cspace.iAwsComponent_OnMouseDoubleClick(*args)
    def OnMouseExit(*args): return _cspace.iAwsComponent_OnMouseExit(*args)
    def OnMouseEnter(*args): return _cspace.iAwsComponent_OnMouseEnter(*args)
    def OnKeyboard(*args): return _cspace.iAwsComponent_OnKeyboard(*args)
    def OnLostFocus(*args): return _cspace.iAwsComponent_OnLostFocus(*args)
    def OnGainFocus(*args): return _cspace.iAwsComponent_OnGainFocus(*args)
    def OnFrame(*args): return _cspace.iAwsComponent_OnFrame(*args)
    def OnAdded(*args): return _cspace.iAwsComponent_OnAdded(*args)
    def OnResized(*args): return _cspace.iAwsComponent_OnResized(*args)
    def OnChildMoved(*args): return _cspace.iAwsComponent_OnChildMoved(*args)
    def OnRaise(*args): return _cspace.iAwsComponent_OnRaise(*args)
    def OnLower(*args): return _cspace.iAwsComponent_OnLower(*args)
    def OnChildHide(*args): return _cspace.iAwsComponent_OnChildHide(*args)
    def OnChildShow(*args): return _cspace.iAwsComponent_OnChildShow(*args)
    def OnSetFocus(*args): return _cspace.iAwsComponent_OnSetFocus(*args)
    def OnUnsetFocus(*args): return _cspace.iAwsComponent_OnUnsetFocus(*args)
    def Unlink(*args): return _cspace.iAwsComponent_Unlink(*args)
    def LinkAbove(*args): return _cspace.iAwsComponent_LinkAbove(*args)
    def LinkBelow(*args): return _cspace.iAwsComponent_LinkBelow(*args)
    def SetTopChild(*args): return _cspace.iAwsComponent_SetTopChild(*args)
    def __del__(self, destroy=_cspace.delete_iAwsComponent):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsComponentPtr(iAwsComponent):
    def __init__(self, this):
        _swig_setattr(self, iAwsComponent, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsComponent, 'thisown', 0)
        _swig_setattr(self, iAwsComponent,self.__class__,iAwsComponent)
_cspace.iAwsComponent_swigregister(iAwsComponentPtr)

class iAwsComponentFactory(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsComponentFactory, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsComponentFactory, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsComponentFactory instance at %s>" % (self.this,)
    def Create(*args): return _cspace.iAwsComponentFactory_Create(*args)
    def Register(*args): return _cspace.iAwsComponentFactory_Register(*args)
    def RegisterConstant(*args): return _cspace.iAwsComponentFactory_RegisterConstant(*args)
    def __del__(self, destroy=_cspace.delete_iAwsComponentFactory):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsComponentFactoryPtr(iAwsComponentFactory):
    def __init__(self, this):
        _swig_setattr(self, iAwsComponentFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsComponentFactory, 'thisown', 0)
        _swig_setattr(self, iAwsComponentFactory,self.__class__,iAwsComponentFactory)
_cspace.iAwsComponentFactory_swigregister(iAwsComponentFactoryPtr)

class iAwsKeyFactory(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsKeyFactory, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsKeyFactory, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsKeyFactory instance at %s>" % (self.this,)
    def Initialize(*args): return _cspace.iAwsKeyFactory_Initialize(*args)
    def AddToWindowList(*args): return _cspace.iAwsKeyFactory_AddToWindowList(*args)
    def AddFactory(*args): return _cspace.iAwsKeyFactory_AddFactory(*args)
    def AddIntKey(*args): return _cspace.iAwsKeyFactory_AddIntKey(*args)
    def AddStringKey(*args): return _cspace.iAwsKeyFactory_AddStringKey(*args)
    def AddRectKey(*args): return _cspace.iAwsKeyFactory_AddRectKey(*args)
    def AddRGBKey(*args): return _cspace.iAwsKeyFactory_AddRGBKey(*args)
    def AddPointKey(*args): return _cspace.iAwsKeyFactory_AddPointKey(*args)
    def AddConnectionKey(*args): return _cspace.iAwsKeyFactory_AddConnectionKey(*args)
    def AddConnectionNode(*args): return _cspace.iAwsKeyFactory_AddConnectionNode(*args)
    def GetThisNode(*args): return _cspace.iAwsKeyFactory_GetThisNode(*args)
    def __del__(self, destroy=_cspace.delete_iAwsKeyFactory):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsKeyFactoryPtr(iAwsKeyFactory):
    def __init__(self, this):
        _swig_setattr(self, iAwsKeyFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsKeyFactory, 'thisown', 0)
        _swig_setattr(self, iAwsKeyFactory,self.__class__,iAwsKeyFactory)
_cspace.iAwsKeyFactory_swigregister(iAwsKeyFactoryPtr)

class iAwsConnectionNodeFactory(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAwsConnectionNodeFactory, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAwsConnectionNodeFactory, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAwsConnectionNodeFactory instance at %s>" % (self.this,)
    def Initialize(*args): return _cspace.iAwsConnectionNodeFactory_Initialize(*args)
    def AddConnectionKey(*args): return _cspace.iAwsConnectionNodeFactory_AddConnectionKey(*args)
    def GetThisNode(*args): return _cspace.iAwsConnectionNodeFactory_GetThisNode(*args)
    def __del__(self, destroy=_cspace.delete_iAwsConnectionNodeFactory):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAwsConnectionNodeFactoryPtr(iAwsConnectionNodeFactory):
    def __init__(self, this):
        _swig_setattr(self, iAwsConnectionNodeFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAwsConnectionNodeFactory, 'thisown', 0)
        _swig_setattr(self, iAwsConnectionNodeFactory,self.__class__,iAwsConnectionNodeFactory)
_cspace.iAwsConnectionNodeFactory_swigregister(iAwsConnectionNodeFactoryPtr)

MAX_OUTPUT_VERTICES = _cspace.MAX_OUTPUT_VERTICES
CS_CLIP_OUTSIDE = _cspace.CS_CLIP_OUTSIDE
CS_CLIP_INSIDE = _cspace.CS_CLIP_INSIDE
CS_CLIP_CLIPPED = _cspace.CS_CLIP_CLIPPED
class csVertexStatus(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csVertexStatus, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csVertexStatus, name)
    def __repr__(self):
        return "<C csVertexStatus instance at %s>" % (self.this,)
    __swig_setmethods__["Type"] = _cspace.csVertexStatus_Type_set
    __swig_getmethods__["Type"] = _cspace.csVertexStatus_Type_get
    if _newclass:Type = property(_cspace.csVertexStatus_Type_get, _cspace.csVertexStatus_Type_set)
    __swig_setmethods__["Vertex"] = _cspace.csVertexStatus_Vertex_set
    __swig_getmethods__["Vertex"] = _cspace.csVertexStatus_Vertex_get
    if _newclass:Vertex = property(_cspace.csVertexStatus_Vertex_get, _cspace.csVertexStatus_Vertex_set)
    __swig_setmethods__["Pos"] = _cspace.csVertexStatus_Pos_set
    __swig_getmethods__["Pos"] = _cspace.csVertexStatus_Pos_get
    if _newclass:Pos = property(_cspace.csVertexStatus_Pos_get, _cspace.csVertexStatus_Pos_set)
    def __init__(self, *args):
        _swig_setattr(self, csVertexStatus, 'this', _cspace.new_csVertexStatus(*args))
        _swig_setattr(self, csVertexStatus, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csVertexStatus):
        try:
            if self.thisown: destroy(self)
        except: pass

class csVertexStatusPtr(csVertexStatus):
    def __init__(self, this):
        _swig_setattr(self, csVertexStatus, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csVertexStatus, 'thisown', 0)
        _swig_setattr(self, csVertexStatus,self.__class__,csVertexStatus)
_cspace.csVertexStatus_swigregister(csVertexStatusPtr)

CS_VERTEX_ORIGINAL = _cspace.CS_VERTEX_ORIGINAL
CS_VERTEX_ONEDGE = _cspace.CS_VERTEX_ONEDGE
CS_VERTEX_INSIDE = _cspace.CS_VERTEX_INSIDE
class iClipper2D(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iClipper2D, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iClipper2D, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iClipper2D instance at %s>" % (self.this,)
    def Clip(*args): return _cspace.iClipper2D_Clip(*args)
    def ClipInPlace(*args): return _cspace.iClipper2D_ClipInPlace(*args)
    def ClassifyBox(*args): return _cspace.iClipper2D_ClassifyBox(*args)
    def IsInside(*args): return _cspace.iClipper2D_IsInside(*args)
    def GetVertexCount(*args): return _cspace.iClipper2D_GetVertexCount(*args)
    def GetClipPoly(*args): return _cspace.iClipper2D_GetClipPoly(*args)
    clipperPoly = _cspace.iClipper2D_clipperPoly
    clipperBox = _cspace.iClipper2D_clipperBox
    def GetClipperType(*args): return _cspace.iClipper2D_GetClipperType(*args)
    def __del__(self, destroy=_cspace.delete_iClipper2D):
        try:
            if self.thisown: destroy(self)
        except: pass

class iClipper2DPtr(iClipper2D):
    def __init__(self, this):
        _swig_setattr(self, iClipper2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iClipper2D, 'thisown', 0)
        _swig_setattr(self, iClipper2D,self.__class__,iClipper2D)
_cspace.iClipper2D_swigregister(iClipper2DPtr)

class iObjectModelListener(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iObjectModelListener, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iObjectModelListener, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iObjectModelListener instance at %s>" % (self.this,)
    def ObjectModelChanged(*args): return _cspace.iObjectModelListener_ObjectModelChanged(*args)
    def __del__(self, destroy=_cspace.delete_iObjectModelListener):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iObjectModelListener_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iObjectModelListener_scfGetVersion)

class iObjectModelListenerPtr(iObjectModelListener):
    def __init__(self, this):
        _swig_setattr(self, iObjectModelListener, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iObjectModelListener, 'thisown', 0)
        _swig_setattr(self, iObjectModelListener,self.__class__,iObjectModelListener)
_cspace.iObjectModelListener_swigregister(iObjectModelListenerPtr)

iObjectModelListener_scfGetVersion = _cspace.iObjectModelListener_scfGetVersion

class iObjectModel(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iObjectModel, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iObjectModel, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iObjectModel instance at %s>" % (self.this,)
    def GetShapeNumber(*args): return _cspace.iObjectModel_GetShapeNumber(*args)
    def GetPolygonMeshBase(*args): return _cspace.iObjectModel_GetPolygonMeshBase(*args)
    def GetPolygonMeshColldet(*args): return _cspace.iObjectModel_GetPolygonMeshColldet(*args)
    def SetPolygonMeshColldet(*args): return _cspace.iObjectModel_SetPolygonMeshColldet(*args)
    def GetPolygonMeshViscull(*args): return _cspace.iObjectModel_GetPolygonMeshViscull(*args)
    def SetPolygonMeshViscull(*args): return _cspace.iObjectModel_SetPolygonMeshViscull(*args)
    def GetPolygonMeshShadows(*args): return _cspace.iObjectModel_GetPolygonMeshShadows(*args)
    def SetPolygonMeshShadows(*args): return _cspace.iObjectModel_SetPolygonMeshShadows(*args)
    def CreateLowerDetailPolygonMesh(*args): return _cspace.iObjectModel_CreateLowerDetailPolygonMesh(*args)
    def GetObjectBoundingBox(*args): return _cspace.iObjectModel_GetObjectBoundingBox(*args)
    def SetObjectBoundingBox(*args): return _cspace.iObjectModel_SetObjectBoundingBox(*args)
    def GetRadius(*args): return _cspace.iObjectModel_GetRadius(*args)
    def AddListener(*args): return _cspace.iObjectModel_AddListener(*args)
    def RemoveListener(*args): return _cspace.iObjectModel_RemoveListener(*args)
    def __del__(self, destroy=_cspace.delete_iObjectModel):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iObjectModel_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iObjectModel_scfGetVersion)

class iObjectModelPtr(iObjectModel):
    def __init__(self, this):
        _swig_setattr(self, iObjectModel, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iObjectModel, 'thisown', 0)
        _swig_setattr(self, iObjectModel,self.__class__,iObjectModel)
_cspace.iObjectModel_swigregister(iObjectModelPtr)

iObjectModel_scfGetVersion = _cspace.iObjectModel_scfGetVersion

CS_POLYMESH_CLOSED = _cspace.CS_POLYMESH_CLOSED
CS_POLYMESH_NOTCLOSED = _cspace.CS_POLYMESH_NOTCLOSED
CS_POLYMESH_CONVEX = _cspace.CS_POLYMESH_CONVEX
CS_POLYMESH_NOTCONVEX = _cspace.CS_POLYMESH_NOTCONVEX
CS_POLYMESH_DEFORMABLE = _cspace.CS_POLYMESH_DEFORMABLE
CS_POLYMESH_TRIANGLEMESH = _cspace.CS_POLYMESH_TRIANGLEMESH
class csMeshedPolygon(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csMeshedPolygon, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csMeshedPolygon, name)
    def __repr__(self):
        return "<C csMeshedPolygon instance at %s>" % (self.this,)
    __swig_setmethods__["num_vertices"] = _cspace.csMeshedPolygon_num_vertices_set
    __swig_getmethods__["num_vertices"] = _cspace.csMeshedPolygon_num_vertices_get
    if _newclass:num_vertices = property(_cspace.csMeshedPolygon_num_vertices_get, _cspace.csMeshedPolygon_num_vertices_set)
    __swig_setmethods__["vertices"] = _cspace.csMeshedPolygon_vertices_set
    __swig_getmethods__["vertices"] = _cspace.csMeshedPolygon_vertices_get
    if _newclass:vertices = property(_cspace.csMeshedPolygon_vertices_get, _cspace.csMeshedPolygon_vertices_set)
    def __init__(self, *args):
        _swig_setattr(self, csMeshedPolygon, 'this', _cspace.new_csMeshedPolygon(*args))
        _swig_setattr(self, csMeshedPolygon, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csMeshedPolygon):
        try:
            if self.thisown: destroy(self)
        except: pass

class csMeshedPolygonPtr(csMeshedPolygon):
    def __init__(self, this):
        _swig_setattr(self, csMeshedPolygon, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csMeshedPolygon, 'thisown', 0)
        _swig_setattr(self, csMeshedPolygon,self.__class__,csMeshedPolygon)
_cspace.csMeshedPolygon_swigregister(csMeshedPolygonPtr)

class iPolygonMesh(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPolygonMesh, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPolygonMesh, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPolygonMesh instance at %s>" % (self.this,)
    def GetVertexCount(*args): return _cspace.iPolygonMesh_GetVertexCount(*args)
    def GetVertices(*args): return _cspace.iPolygonMesh_GetVertices(*args)
    def GetPolygonCount(*args): return _cspace.iPolygonMesh_GetPolygonCount(*args)
    def GetPolygons(*args): return _cspace.iPolygonMesh_GetPolygons(*args)
    def GetTriangleCount(*args): return _cspace.iPolygonMesh_GetTriangleCount(*args)
    def GetTriangles(*args): return _cspace.iPolygonMesh_GetTriangles(*args)
    def Lock(*args): return _cspace.iPolygonMesh_Lock(*args)
    def Unlock(*args): return _cspace.iPolygonMesh_Unlock(*args)
    def GetFlags(*args): return _cspace.iPolygonMesh_GetFlags(*args)
    def GetChangeNumber(*args): return _cspace.iPolygonMesh_GetChangeNumber(*args)
    def __del__(self, destroy=_cspace.delete_iPolygonMesh):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iPolygonMesh_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iPolygonMesh_scfGetVersion)

class iPolygonMeshPtr(iPolygonMesh):
    def __init__(self, this):
        _swig_setattr(self, iPolygonMesh, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPolygonMesh, 'thisown', 0)
        _swig_setattr(self, iPolygonMesh,self.__class__,iPolygonMesh)
_cspace.iPolygonMesh_swigregister(iPolygonMeshPtr)

iPolygonMesh_scfGetVersion = _cspace.iPolygonMesh_scfGetVersion

class csPolygonMesh(iPolygonMesh):
    __swig_setmethods__ = {}
    for _s in [iPolygonMesh]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPolygonMesh, name, value)
    __swig_getmethods__ = {}
    for _s in [iPolygonMesh]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csPolygonMesh, name)
    def __repr__(self):
        return "<C csPolygonMesh instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csPolygonMesh, 'this', _cspace.new_csPolygonMesh(*args))
        _swig_setattr(self, csPolygonMesh, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPolygonMesh):
        try:
            if self.thisown: destroy(self)
        except: pass
    def SetVertices(*args): return _cspace.csPolygonMesh_SetVertices(*args)
    def SetPolygons(*args): return _cspace.csPolygonMesh_SetPolygons(*args)
    def SetPolygonIndices(*args): return _cspace.csPolygonMesh_SetPolygonIndices(*args)
    def SetPolygonIndexCount(*args): return _cspace.csPolygonMesh_SetPolygonIndexCount(*args)
    def GetPolygonIndices(*args): return _cspace.csPolygonMesh_GetPolygonIndices(*args)
    def SetVertexCount(*args): return _cspace.csPolygonMesh_SetVertexCount(*args)
    def SetPolygonCount(*args): return _cspace.csPolygonMesh_SetPolygonCount(*args)
    def ShapeChanged(*args): return _cspace.csPolygonMesh_ShapeChanged(*args)
    __swig_setmethods__["scfRefCount"] = _cspace.csPolygonMesh_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace.csPolygonMesh_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace.csPolygonMesh_scfRefCount_get, _cspace.csPolygonMesh_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace.csPolygonMesh_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace.csPolygonMesh_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace.csPolygonMesh_scfWeakRefOwners_get, _cspace.csPolygonMesh_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace.csPolygonMesh_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace.csPolygonMesh_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace.csPolygonMesh_scfParent_get
    if _newclass:scfParent = property(_cspace.csPolygonMesh_scfParent_get, _cspace.csPolygonMesh_scfParent_set)
    def IncRef(*args): return _cspace.csPolygonMesh_IncRef(*args)
    def DecRef(*args): return _cspace.csPolygonMesh_DecRef(*args)
    def GetRefCount(*args): return _cspace.csPolygonMesh_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csPolygonMesh_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csPolygonMesh_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csPolygonMesh_QueryInterface(*args)
    def GetVertexCount(*args): return _cspace.csPolygonMesh_GetVertexCount(*args)
    def GetVertices(*args): return _cspace.csPolygonMesh_GetVertices(*args)
    def GetPolygonCount(*args): return _cspace.csPolygonMesh_GetPolygonCount(*args)
    def GetPolygons(*args): return _cspace.csPolygonMesh_GetPolygons(*args)
    def GetTriangleCount(*args): return _cspace.csPolygonMesh_GetTriangleCount(*args)
    def GetTriangles(*args): return _cspace.csPolygonMesh_GetTriangles(*args)
    def Lock(*args): return _cspace.csPolygonMesh_Lock(*args)
    def Unlock(*args): return _cspace.csPolygonMesh_Unlock(*args)
    def GetFlags(*args): return _cspace.csPolygonMesh_GetFlags(*args)
    def GetChangeNumber(*args): return _cspace.csPolygonMesh_GetChangeNumber(*args)

class csPolygonMeshPtr(csPolygonMesh):
    def __init__(self, this):
        _swig_setattr(self, csPolygonMesh, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPolygonMesh, 'thisown', 0)
        _swig_setattr(self, csPolygonMesh,self.__class__,csPolygonMesh)
_cspace.csPolygonMesh_swigregister(csPolygonMeshPtr)

class csPolygonMeshBox(iPolygonMesh):
    __swig_setmethods__ = {}
    for _s in [iPolygonMesh]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPolygonMeshBox, name, value)
    __swig_getmethods__ = {}
    for _s in [iPolygonMesh]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csPolygonMeshBox, name)
    def __repr__(self):
        return "<C csPolygonMeshBox instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csPolygonMeshBox, 'this', _cspace.new_csPolygonMeshBox(*args))
        _swig_setattr(self, csPolygonMeshBox, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPolygonMeshBox):
        try:
            if self.thisown: destroy(self)
        except: pass
    def SetBox(*args): return _cspace.csPolygonMeshBox_SetBox(*args)
    __swig_setmethods__["scfRefCount"] = _cspace.csPolygonMeshBox_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace.csPolygonMeshBox_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace.csPolygonMeshBox_scfRefCount_get, _cspace.csPolygonMeshBox_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace.csPolygonMeshBox_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace.csPolygonMeshBox_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace.csPolygonMeshBox_scfWeakRefOwners_get, _cspace.csPolygonMeshBox_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace.csPolygonMeshBox_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace.csPolygonMeshBox_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace.csPolygonMeshBox_scfParent_get
    if _newclass:scfParent = property(_cspace.csPolygonMeshBox_scfParent_get, _cspace.csPolygonMeshBox_scfParent_set)
    def IncRef(*args): return _cspace.csPolygonMeshBox_IncRef(*args)
    def DecRef(*args): return _cspace.csPolygonMeshBox_DecRef(*args)
    def GetRefCount(*args): return _cspace.csPolygonMeshBox_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csPolygonMeshBox_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csPolygonMeshBox_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csPolygonMeshBox_QueryInterface(*args)
    def GetVertexCount(*args): return _cspace.csPolygonMeshBox_GetVertexCount(*args)
    def GetVertices(*args): return _cspace.csPolygonMeshBox_GetVertices(*args)
    def GetPolygonCount(*args): return _cspace.csPolygonMeshBox_GetPolygonCount(*args)
    def GetPolygons(*args): return _cspace.csPolygonMeshBox_GetPolygons(*args)
    def GetTriangleCount(*args): return _cspace.csPolygonMeshBox_GetTriangleCount(*args)
    def GetTriangles(*args): return _cspace.csPolygonMeshBox_GetTriangles(*args)
    def Lock(*args): return _cspace.csPolygonMeshBox_Lock(*args)
    def Unlock(*args): return _cspace.csPolygonMeshBox_Unlock(*args)
    def GetFlags(*args): return _cspace.csPolygonMeshBox_GetFlags(*args)
    def GetChangeNumber(*args): return _cspace.csPolygonMeshBox_GetChangeNumber(*args)

class csPolygonMeshBoxPtr(csPolygonMeshBox):
    def __init__(self, this):
        _swig_setattr(self, csPolygonMeshBox, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPolygonMeshBox, 'thisown', 0)
        _swig_setattr(self, csPolygonMeshBox,self.__class__,csPolygonMeshBox)
_cspace.csPolygonMeshBox_swigregister(csPolygonMeshBoxPtr)

class iFrustumViewUserdata(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFrustumViewUserdata, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFrustumViewUserdata, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFrustumViewUserdata instance at %s>" % (self.this,)
    def __del__(self, destroy=_cspace.delete_iFrustumViewUserdata):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iFrustumViewUserdata_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iFrustumViewUserdata_scfGetVersion)

class iFrustumViewUserdataPtr(iFrustumViewUserdata):
    def __init__(self, this):
        _swig_setattr(self, iFrustumViewUserdata, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFrustumViewUserdata, 'thisown', 0)
        _swig_setattr(self, iFrustumViewUserdata,self.__class__,iFrustumViewUserdata)
_cspace.iFrustumViewUserdata_swigregister(iFrustumViewUserdataPtr)

iFrustumViewUserdata_scfGetVersion = _cspace.iFrustumViewUserdata_scfGetVersion

class csFrustumContext(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csFrustumContext, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csFrustumContext, name)
    def __repr__(self):
        return "<C csFrustumContext instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csFrustumContext, 'this', _cspace.new_csFrustumContext(*args))
        _swig_setattr(self, csFrustumContext, 'thisown', 1)
    def assign(*args): return _cspace.csFrustumContext_assign(*args)
    def GetShadows(*args): return _cspace.csFrustumContext_GetShadows(*args)
    def SetShadows(*args): return _cspace.csFrustumContext_SetShadows(*args)
    def SetNewShadows(*args): return _cspace.csFrustumContext_SetNewShadows(*args)
    def IsShared(*args): return _cspace.csFrustumContext_IsShared(*args)
    def SetLightFrustum(*args): return _cspace.csFrustumContext_SetLightFrustum(*args)
    def SetNewLightFrustum(*args): return _cspace.csFrustumContext_SetNewLightFrustum(*args)
    def GetLightFrustum(*args): return _cspace.csFrustumContext_GetLightFrustum(*args)
    def SetMirrored(*args): return _cspace.csFrustumContext_SetMirrored(*args)
    def IsMirrored(*args): return _cspace.csFrustumContext_IsMirrored(*args)
    def __del__(self, destroy=_cspace.delete_csFrustumContext):
        try:
            if self.thisown: destroy(self)
        except: pass

class csFrustumContextPtr(csFrustumContext):
    def __init__(self, this):
        _swig_setattr(self, csFrustumContext, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csFrustumContext, 'thisown', 0)
        _swig_setattr(self, csFrustumContext,self.__class__,csFrustumContext)
_cspace.csFrustumContext_swigregister(csFrustumContextPtr)

class iFrustumView(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFrustumView, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFrustumView, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFrustumView instance at %s>" % (self.this,)
    def GetFrustumContext(*args): return _cspace.iFrustumView_GetFrustumContext(*args)
    def CreateFrustumContext(*args): return _cspace.iFrustumView_CreateFrustumContext(*args)
    def CopyFrustumContext(*args): return _cspace.iFrustumView_CopyFrustumContext(*args)
    def SetFrustumContext(*args): return _cspace.iFrustumView_SetFrustumContext(*args)
    def RestoreFrustumContext(*args): return _cspace.iFrustumView_RestoreFrustumContext(*args)
    def SetObjectFunction(*args): return _cspace.iFrustumView_SetObjectFunction(*args)
    def CallObjectFunction(*args): return _cspace.iFrustumView_CallObjectFunction(*args)
    def GetRadius(*args): return _cspace.iFrustumView_GetRadius(*args)
    def GetSquaredRadius(*args): return _cspace.iFrustumView_GetSquaredRadius(*args)
    def ThingShadowsEnabled(*args): return _cspace.iFrustumView_ThingShadowsEnabled(*args)
    def CheckShadowMask(*args): return _cspace.iFrustumView_CheckShadowMask(*args)
    def CheckProcessMask(*args): return _cspace.iFrustumView_CheckProcessMask(*args)
    def StartNewShadowBlock(*args): return _cspace.iFrustumView_StartNewShadowBlock(*args)
    def SetUserdata(*args): return _cspace.iFrustumView_SetUserdata(*args)
    def GetUserdata(*args): return _cspace.iFrustumView_GetUserdata(*args)
    def CreateShadowBlock(*args): return _cspace.iFrustumView_CreateShadowBlock(*args)
    def __del__(self, destroy=_cspace.delete_iFrustumView):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iFrustumView_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iFrustumView_scfGetVersion)

class iFrustumViewPtr(iFrustumView):
    def __init__(self, this):
        _swig_setattr(self, iFrustumView, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFrustumView, 'thisown', 0)
        _swig_setattr(self, iFrustumView,self.__class__,iFrustumView)
_cspace.iFrustumView_swigregister(iFrustumViewPtr)

iFrustumView_scfGetVersion = _cspace.iFrustumView_scfGetVersion

CS_LIGHT_THINGSHADOWS = _cspace.CS_LIGHT_THINGSHADOWS
CS_LIGHT_ACTIVEHALO = _cspace.CS_LIGHT_ACTIVEHALO
CS_LIGHT_DYNAMICTYPE_STATIC = _cspace.CS_LIGHT_DYNAMICTYPE_STATIC
CS_LIGHT_DYNAMICTYPE_PSEUDO = _cspace.CS_LIGHT_DYNAMICTYPE_PSEUDO
CS_LIGHT_DYNAMICTYPE_DYNAMIC = _cspace.CS_LIGHT_DYNAMICTYPE_DYNAMIC
CS_DEFAULT_LIGHT_LEVEL = _cspace.CS_DEFAULT_LIGHT_LEVEL
CS_NORMAL_LIGHT_LEVEL = _cspace.CS_NORMAL_LIGHT_LEVEL
CS_ATTN_NONE = _cspace.CS_ATTN_NONE
CS_ATTN_LINEAR = _cspace.CS_ATTN_LINEAR
CS_ATTN_INVERSE = _cspace.CS_ATTN_INVERSE
CS_ATTN_REALISTIC = _cspace.CS_ATTN_REALISTIC
CS_ATTN_CLQ = _cspace.CS_ATTN_CLQ
CS_LIGHT_POINTLIGHT = _cspace.CS_LIGHT_POINTLIGHT
CS_LIGHT_DIRECTIONAL = _cspace.CS_LIGHT_DIRECTIONAL
CS_LIGHT_SPOTLIGHT = _cspace.CS_LIGHT_SPOTLIGHT
class iLightCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLightCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLightCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLightCallback instance at %s>" % (self.this,)
    def OnColorChange(*args): return _cspace.iLightCallback_OnColorChange(*args)
    def OnPositionChange(*args): return _cspace.iLightCallback_OnPositionChange(*args)
    def OnSectorChange(*args): return _cspace.iLightCallback_OnSectorChange(*args)
    def OnRadiusChange(*args): return _cspace.iLightCallback_OnRadiusChange(*args)
    def OnDestroy(*args): return _cspace.iLightCallback_OnDestroy(*args)
    def OnAttenuationChange(*args): return _cspace.iLightCallback_OnAttenuationChange(*args)
    def __del__(self, destroy=_cspace.delete_iLightCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iLightCallbackPtr(iLightCallback):
    def __init__(self, this):
        _swig_setattr(self, iLightCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLightCallback, 'thisown', 0)
        _swig_setattr(self, iLightCallback,self.__class__,iLightCallback)
_cspace.iLightCallback_swigregister(iLightCallbackPtr)

class iLight(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLight, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLight, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLight instance at %s>" % (self.this,)
    def GetLightID(*args): return _cspace.iLight_GetLightID(*args)
    def QueryObject(*args): return _cspace.iLight_QueryObject(*args)
    def GetDynamicType(*args): return _cspace.iLight_GetDynamicType(*args)
    def GetCenter(*args): return _cspace.iLight_GetCenter(*args)
    def SetCenter(*args): return _cspace.iLight_SetCenter(*args)
    def GetSector(*args): return _cspace.iLight_GetSector(*args)
    def GetMovable(*args): return _cspace.iLight_GetMovable(*args)
    def GetColor(*args): return _cspace.iLight_GetColor(*args)
    def SetColor(*args): return _cspace.iLight_SetColor(*args)
    def GetSpecularColor(*args): return _cspace.iLight_GetSpecularColor(*args)
    def SetSpecularColor(*args): return _cspace.iLight_SetSpecularColor(*args)
    def GetType(*args): return _cspace.iLight_GetType(*args)
    def SetType(*args): return _cspace.iLight_SetType(*args)
    def GetDirection(*args): return _cspace.iLight_GetDirection(*args)
    def SetDirection(*args): return _cspace.iLight_SetDirection(*args)
    def GetAttenuationMode(*args): return _cspace.iLight_GetAttenuationMode(*args)
    def SetAttenuationMode(*args): return _cspace.iLight_SetAttenuationMode(*args)
    def SetAttenuationConstants(*args): return _cspace.iLight_SetAttenuationConstants(*args)
    def GetAttenuationConstants(*args): return _cspace.iLight_GetAttenuationConstants(*args)
    def GetCutoffDistance(*args): return _cspace.iLight_GetCutoffDistance(*args)
    def SetCutoffDistance(*args): return _cspace.iLight_SetCutoffDistance(*args)
    def GetDirectionalCutoffRadius(*args): return _cspace.iLight_GetDirectionalCutoffRadius(*args)
    def SetDirectionalCutoffRadius(*args): return _cspace.iLight_SetDirectionalCutoffRadius(*args)
    def SetSpotLightFalloff(*args): return _cspace.iLight_SetSpotLightFalloff(*args)
    def GetSpotLightFalloff(*args): return _cspace.iLight_GetSpotLightFalloff(*args)
    def CreateCrossHalo(*args): return _cspace.iLight_CreateCrossHalo(*args)
    def CreateNovaHalo(*args): return _cspace.iLight_CreateNovaHalo(*args)
    def CreateFlareHalo(*args): return _cspace.iLight_CreateFlareHalo(*args)
    def GetHalo(*args): return _cspace.iLight_GetHalo(*args)
    def GetBrightnessAtDistance(*args): return _cspace.iLight_GetBrightnessAtDistance(*args)
    def GetFlags(*args): return _cspace.iLight_GetFlags(*args)
    def SetLightCallback(*args): return _cspace.iLight_SetLightCallback(*args)
    def RemoveLightCallback(*args): return _cspace.iLight_RemoveLightCallback(*args)
    def GetLightCallbackCount(*args): return _cspace.iLight_GetLightCallbackCount(*args)
    def GetLightCallback(*args): return _cspace.iLight_GetLightCallback(*args)
    def GetLightNumber(*args): return _cspace.iLight_GetLightNumber(*args)
    def AddAffectedLightingInfo(*args): return _cspace.iLight_AddAffectedLightingInfo(*args)
    def RemoveAffectedLightingInfo(*args): return _cspace.iLight_RemoveAffectedLightingInfo(*args)
    def Setup(*args): return _cspace.iLight_Setup(*args)
    def __del__(self, destroy=_cspace.delete_iLight):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iLight_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iLight_scfGetVersion)

class iLightPtr(iLight):
    def __init__(self, this):
        _swig_setattr(self, iLight, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLight, 'thisown', 0)
        _swig_setattr(self, iLight,self.__class__,iLight)
_cspace.iLight_swigregister(iLightPtr)

iLight_scfGetVersion = _cspace.iLight_scfGetVersion

class iLightList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLightList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLightList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLightList instance at %s>" % (self.this,)
    def GetCount(*args): return _cspace.iLightList_GetCount(*args)
    def Get(*args): return _cspace.iLightList_Get(*args)
    def Add(*args): return _cspace.iLightList_Add(*args)
    def Remove(*args): return _cspace.iLightList_Remove(*args)
    def RemoveAll(*args): return _cspace.iLightList_RemoveAll(*args)
    def Find(*args): return _cspace.iLightList_Find(*args)
    def FindByName(*args): return _cspace.iLightList_FindByName(*args)
    def FindByID(*args): return _cspace.iLightList_FindByID(*args)
    def __del__(self, destroy=_cspace.delete_iLightList):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iLightList_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iLightList_scfGetVersion)

class iLightListPtr(iLightList):
    def __init__(self, this):
        _swig_setattr(self, iLightList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLightList, 'thisown', 0)
        _swig_setattr(self, iLightList,self.__class__,iLightList)
_cspace.iLightList_swigregister(iLightListPtr)

iLightList_scfGetVersion = _cspace.iLightList_scfGetVersion

class iLightingProcessData(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLightingProcessData, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLightingProcessData, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLightingProcessData instance at %s>" % (self.this,)
    def FinalizeLighting(*args): return _cspace.iLightingProcessData_FinalizeLighting(*args)
    def __del__(self, destroy=_cspace.delete_iLightingProcessData):
        try:
            if self.thisown: destroy(self)
        except: pass

class iLightingProcessDataPtr(iLightingProcessData):
    def __init__(self, this):
        _swig_setattr(self, iLightingProcessData, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLightingProcessData, 'thisown', 0)
        _swig_setattr(self, iLightingProcessData,self.__class__,iLightingProcessData)
_cspace.iLightingProcessData_swigregister(iLightingProcessDataPtr)

class iLightingProcessInfo(iFrustumViewUserdata):
    __swig_setmethods__ = {}
    for _s in [iFrustumViewUserdata]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLightingProcessInfo, name, value)
    __swig_getmethods__ = {}
    for _s in [iFrustumViewUserdata]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLightingProcessInfo, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLightingProcessInfo instance at %s>" % (self.this,)
    def GetLight(*args): return _cspace.iLightingProcessInfo_GetLight(*args)
    def IsDynamic(*args): return _cspace.iLightingProcessInfo_IsDynamic(*args)
    def SetColor(*args): return _cspace.iLightingProcessInfo_SetColor(*args)
    def GetColor(*args): return _cspace.iLightingProcessInfo_GetColor(*args)
    def AttachUserdata(*args): return _cspace.iLightingProcessInfo_AttachUserdata(*args)
    def QueryUserdata(*args): return _cspace.iLightingProcessInfo_QueryUserdata(*args)
    def FinalizeLighting(*args): return _cspace.iLightingProcessInfo_FinalizeLighting(*args)
    def __del__(self, destroy=_cspace.delete_iLightingProcessInfo):
        try:
            if self.thisown: destroy(self)
        except: pass

class iLightingProcessInfoPtr(iLightingProcessInfo):
    def __init__(self, this):
        _swig_setattr(self, iLightingProcessInfo, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLightingProcessInfo, 'thisown', 0)
        _swig_setattr(self, iLightingProcessInfo,self.__class__,iLightingProcessInfo)
_cspace.iLightingProcessInfo_swigregister(iLightingProcessInfoPtr)

class iLightIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLightIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLightIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLightIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iLightIterator_HasNext(*args)
    def Next(*args): return _cspace.iLightIterator_Next(*args)
    def GetLastSector(*args): return _cspace.iLightIterator_GetLastSector(*args)
    def Reset(*args): return _cspace.iLightIterator_Reset(*args)
    def __del__(self, destroy=_cspace.delete_iLightIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iLightIteratorPtr(iLightIterator):
    def __init__(self, this):
        _swig_setattr(self, iLightIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLightIterator, 'thisown', 0)
        _swig_setattr(self, iLightIterator,self.__class__,iLightIterator)
_cspace.iLightIterator_swigregister(iLightIteratorPtr)

class iSectorCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSectorCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSectorCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSectorCallback instance at %s>" % (self.this,)
    def Traverse(*args): return _cspace.iSectorCallback_Traverse(*args)
    def __del__(self, destroy=_cspace.delete_iSectorCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSectorCallbackPtr(iSectorCallback):
    def __init__(self, this):
        _swig_setattr(self, iSectorCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSectorCallback, 'thisown', 0)
        _swig_setattr(self, iSectorCallback,self.__class__,iSectorCallback)
_cspace.iSectorCallback_swigregister(iSectorCallbackPtr)

class iSectorMeshCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSectorMeshCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSectorMeshCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSectorMeshCallback instance at %s>" % (self.this,)
    def NewMesh(*args): return _cspace.iSectorMeshCallback_NewMesh(*args)
    def RemoveMesh(*args): return _cspace.iSectorMeshCallback_RemoveMesh(*args)
    def __del__(self, destroy=_cspace.delete_iSectorMeshCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSectorMeshCallbackPtr(iSectorMeshCallback):
    def __init__(self, this):
        _swig_setattr(self, iSectorMeshCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSectorMeshCallback, 'thisown', 0)
        _swig_setattr(self, iSectorMeshCallback,self.__class__,iSectorMeshCallback)
_cspace.iSectorMeshCallback_swigregister(iSectorMeshCallbackPtr)

class iSector(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSector, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSector, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSector instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iSector_QueryObject(*args)
    def SetRenderLoop(*args): return _cspace.iSector_SetRenderLoop(*args)
    def GetRenderLoop(*args): return _cspace.iSector_GetRenderLoop(*args)
    def HasFog(*args): return _cspace.iSector_HasFog(*args)
    def GetFog(*args): return _cspace.iSector_GetFog(*args)
    def SetFog(*args): return _cspace.iSector_SetFog(*args)
    def DisableFog(*args): return _cspace.iSector_DisableFog(*args)
    def GetMeshes(*args): return _cspace.iSector_GetMeshes(*args)
    def GetLights(*args): return _cspace.iSector_GetLights(*args)
    def ShineLights(*args): return _cspace.iSector_ShineLights(*args)
    def SetDynamicAmbientLight(*args): return _cspace.iSector_SetDynamicAmbientLight(*args)
    def GetDynamicAmbientLight(*args): return _cspace.iSector_GetDynamicAmbientLight(*args)
    def CalculateSectorBBox(*args): return _cspace.iSector_CalculateSectorBBox(*args)
    def SetVisibilityCullerPlugin(*args): return _cspace.iSector_SetVisibilityCullerPlugin(*args)
    def GetVisibilityCuller(*args): return _cspace.iSector_GetVisibilityCuller(*args)
    def GetRecLevel(*args): return _cspace.iSector_GetRecLevel(*args)
    def IncRecLevel(*args): return _cspace.iSector_IncRecLevel(*args)
    def DecRecLevel(*args): return _cspace.iSector_DecRecLevel(*args)
    def HitBeamPortals(*args): return _cspace.iSector_HitBeamPortals(*args)
    def HitBeam(*args): return _cspace.iSector_HitBeam(*args)
    def FollowSegment(*args): return _cspace.iSector_FollowSegment(*args)
    def Draw(*args): return _cspace.iSector_Draw(*args)
    def PrepareDraw(*args): return _cspace.iSector_PrepareDraw(*args)
    def GetVisibleMeshes(*args): return _cspace.iSector_GetVisibleMeshes(*args)
    def SetSectorCallback(*args): return _cspace.iSector_SetSectorCallback(*args)
    def RemoveSectorCallback(*args): return _cspace.iSector_RemoveSectorCallback(*args)
    def GetSectorCallbackCount(*args): return _cspace.iSector_GetSectorCallbackCount(*args)
    def GetSectorCallback(*args): return _cspace.iSector_GetSectorCallback(*args)
    def AddSectorMeshCallback(*args): return _cspace.iSector_AddSectorMeshCallback(*args)
    def RemoveSectorMeshCallback(*args): return _cspace.iSector_RemoveSectorMeshCallback(*args)
    def CheckFrustum(*args): return _cspace.iSector_CheckFrustum(*args)
    def GetPortalMeshes(*args): return _cspace.iSector_GetPortalMeshes(*args)
    def RegisterPortalMesh(*args): return _cspace.iSector_RegisterPortalMesh(*args)
    def UnregisterPortalMesh(*args): return _cspace.iSector_UnregisterPortalMesh(*args)
    def UnlinkObjects(*args): return _cspace.iSector_UnlinkObjects(*args)
    def __del__(self, destroy=_cspace.delete_iSector):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSector_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSector_scfGetVersion)

class iSectorPtr(iSector):
    def __init__(self, this):
        _swig_setattr(self, iSector, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSector, 'thisown', 0)
        _swig_setattr(self, iSector,self.__class__,iSector)
_cspace.iSector_swigregister(iSectorPtr)

iSector_scfGetVersion = _cspace.iSector_scfGetVersion

class iSectorList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSectorList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSectorList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSectorList instance at %s>" % (self.this,)
    def GetCount(*args): return _cspace.iSectorList_GetCount(*args)
    def Get(*args): return _cspace.iSectorList_Get(*args)
    def Add(*args): return _cspace.iSectorList_Add(*args)
    def Remove(*args): return _cspace.iSectorList_Remove(*args)
    def RemoveAll(*args): return _cspace.iSectorList_RemoveAll(*args)
    def Find(*args): return _cspace.iSectorList_Find(*args)
    def FindByName(*args): return _cspace.iSectorList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iSectorList):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSectorList_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSectorList_scfGetVersion)

class iSectorListPtr(iSectorList):
    def __init__(self, this):
        _swig_setattr(self, iSectorList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSectorList, 'thisown', 0)
        _swig_setattr(self, iSectorList,self.__class__,iSectorList)
_cspace.iSectorList_swigregister(iSectorListPtr)

iSectorList_scfGetVersion = _cspace.iSectorList_scfGetVersion

class iSectorIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSectorIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSectorIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSectorIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iSectorIterator_HasNext(*args)
    def Next(*args): return _cspace.iSectorIterator_Next(*args)
    def GetLastPosition(*args): return _cspace.iSectorIterator_GetLastPosition(*args)
    def Reset(*args): return _cspace.iSectorIterator_Reset(*args)
    def __del__(self, destroy=_cspace.delete_iSectorIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSectorIteratorPtr(iSectorIterator):
    def __init__(self, this):
        _swig_setattr(self, iSectorIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSectorIterator, 'thisown', 0)
        _swig_setattr(self, iSectorIterator,self.__class__,iSectorIterator)
_cspace.iSectorIterator_swigregister(iSectorIteratorPtr)

CS_ENGINE_CACHE_READ = _cspace.CS_ENGINE_CACHE_READ
CS_ENGINE_CACHE_WRITE = _cspace.CS_ENGINE_CACHE_WRITE
CS_ENGINE_CACHE_NOUPDATE = _cspace.CS_ENGINE_CACHE_NOUPDATE
CS_RENDPRI_NONE = _cspace.CS_RENDPRI_NONE
CS_RENDPRI_BACK2FRONT = _cspace.CS_RENDPRI_BACK2FRONT
CS_RENDPRI_FRONT2BACK = _cspace.CS_RENDPRI_FRONT2BACK
class iEngineSectorCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEngineSectorCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEngineSectorCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEngineSectorCallback instance at %s>" % (self.this,)
    def NewSector(*args): return _cspace.iEngineSectorCallback_NewSector(*args)
    def RemoveSector(*args): return _cspace.iEngineSectorCallback_RemoveSector(*args)
    def __del__(self, destroy=_cspace.delete_iEngineSectorCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEngineSectorCallbackPtr(iEngineSectorCallback):
    def __init__(self, this):
        _swig_setattr(self, iEngineSectorCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEngineSectorCallback, 'thisown', 0)
        _swig_setattr(self, iEngineSectorCallback,self.__class__,iEngineSectorCallback)
_cspace.iEngineSectorCallback_swigregister(iEngineSectorCallbackPtr)

class iEngine(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEngine, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEngine, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEngine instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iEngine_QueryObject(*args)
    def Prepare(*args): return _cspace.iEngine_Prepare(*args)
    def ForceRelight(*args): return _cspace.iEngine_ForceRelight(*args)
    def RemoveLight(*args): return _cspace.iEngine_RemoveLight(*args)
    def PrepareTextures(*args): return _cspace.iEngine_PrepareTextures(*args)
    def PrepareMeshes(*args): return _cspace.iEngine_PrepareMeshes(*args)
    def ShineLights(*args): return _cspace.iEngine_ShineLights(*args)
    def GetTextureFormat(*args): return _cspace.iEngine_GetTextureFormat(*args)
    def DeleteAll(*args): return _cspace.iEngine_DeleteAll(*args)
    def RegisterRenderPriority(*args): return _cspace.iEngine_RegisterRenderPriority(*args)
    def GetRenderPriority(*args): return _cspace.iEngine_GetRenderPriority(*args)
    def GetRenderPrioritySorting(*args): return _cspace.iEngine_GetRenderPrioritySorting(*args)
    def GetSkyRenderPriority(*args): return _cspace.iEngine_GetSkyRenderPriority(*args)
    def GetPortalRenderPriority(*args): return _cspace.iEngine_GetPortalRenderPriority(*args)
    def GetWallRenderPriority(*args): return _cspace.iEngine_GetWallRenderPriority(*args)
    def GetObjectRenderPriority(*args): return _cspace.iEngine_GetObjectRenderPriority(*args)
    def GetAlphaRenderPriority(*args): return _cspace.iEngine_GetAlphaRenderPriority(*args)
    def ClearRenderPriorities(*args): return _cspace.iEngine_ClearRenderPriorities(*args)
    def GetRenderPriorityCount(*args): return _cspace.iEngine_GetRenderPriorityCount(*args)
    def GetRenderPriorityName(*args): return _cspace.iEngine_GetRenderPriorityName(*args)
    def CreateBaseMaterial(*args): return _cspace.iEngine_CreateBaseMaterial(*args)
    def CreateTexture(*args): return _cspace.iEngine_CreateTexture(*args)
    def CreateBlackTexture(*args): return _cspace.iEngine_CreateBlackTexture(*args)
    def CreateMaterial(*args): return _cspace.iEngine_CreateMaterial(*args)
    def CreateSector(*args): return _cspace.iEngine_CreateSector(*args)
    def AddEngineSectorCallback(*args): return _cspace.iEngine_AddEngineSectorCallback(*args)
    def RemoveEngineSectorCallback(*args): return _cspace.iEngine_RemoveEngineSectorCallback(*args)
    def CreateSectorWallsMesh(*args): return _cspace.iEngine_CreateSectorWallsMesh(*args)
    def CreateThingMesh(*args): return _cspace.iEngine_CreateThingMesh(*args)
    def GetSectors(*args): return _cspace.iEngine_GetSectors(*args)
    def GetMeshFactories(*args): return _cspace.iEngine_GetMeshFactories(*args)
    def GetMeshes(*args): return _cspace.iEngine_GetMeshes(*args)
    def GetCollections(*args): return _cspace.iEngine_GetCollections(*args)
    def GetCameraPositions(*args): return _cspace.iEngine_GetCameraPositions(*args)
    def GetTextureList(*args): return _cspace.iEngine_GetTextureList(*args)
    def GetMaterialList(*args): return _cspace.iEngine_GetMaterialList(*args)
    def GetVariableList(*args): return _cspace.iEngine_GetVariableList(*args)
    def AddMeshAndChildren(*args): return _cspace.iEngine_AddMeshAndChildren(*args)
    def CreateRegion(*args): return _cspace.iEngine_CreateRegion(*args)
    def GetRegions(*args): return _cspace.iEngine_GetRegions(*args)
    def FindMaterial(*args): return _cspace.iEngine_FindMaterial(*args)
    def FindTexture(*args): return _cspace.iEngine_FindTexture(*args)
    def FindSector(*args): return _cspace.iEngine_FindSector(*args)
    def FindMeshObject(*args): return _cspace.iEngine_FindMeshObject(*args)
    def FindMeshFactory(*args): return _cspace.iEngine_FindMeshFactory(*args)
    def FindCameraPosition(*args): return _cspace.iEngine_FindCameraPosition(*args)
    def FindCollection(*args): return _cspace.iEngine_FindCollection(*args)
    def SetLightingCacheMode(*args): return _cspace.iEngine_SetLightingCacheMode(*args)
    def GetLightingCacheMode(*args): return _cspace.iEngine_GetLightingCacheMode(*args)
    def SetClearZBuf(*args): return _cspace.iEngine_SetClearZBuf(*args)
    def GetClearZBuf(*args): return _cspace.iEngine_GetClearZBuf(*args)
    def GetDefaultClearZBuf(*args): return _cspace.iEngine_GetDefaultClearZBuf(*args)
    def SetClearScreen(*args): return _cspace.iEngine_SetClearScreen(*args)
    def GetClearScreen(*args): return _cspace.iEngine_GetClearScreen(*args)
    def GetDefaultClearScreen(*args): return _cspace.iEngine_GetDefaultClearScreen(*args)
    def SetMaxLightmapSize(*args): return _cspace.iEngine_SetMaxLightmapSize(*args)
    def GetMaxLightmapSize(*args): return _cspace.iEngine_GetMaxLightmapSize(*args)
    def GetDefaultMaxLightmapSize(*args): return _cspace.iEngine_GetDefaultMaxLightmapSize(*args)
    def GetMaxLightmapAspectRatio(*args): return _cspace.iEngine_GetMaxLightmapAspectRatio(*args)
    def ResetWorldSpecificSettings(*args): return _cspace.iEngine_ResetWorldSpecificSettings(*args)
    def CreateCamera(*args): return _cspace.iEngine_CreateCamera(*args)
    def CreateLight(*args): return _cspace.iEngine_CreateLight(*args)
    def FindLight(*args): return _cspace.iEngine_FindLight(*args)
    def FindLightID(*args): return _cspace.iEngine_FindLightID(*args)
    def GetLightIterator(*args): return _cspace.iEngine_GetLightIterator(*args)
    def GetBeginDrawFlags(*args): return _cspace.iEngine_GetBeginDrawFlags(*args)
    def GetTopLevelClipper(*args): return _cspace.iEngine_GetTopLevelClipper(*args)
    def CreateMeshFactory(*args): return _cspace.iEngine_CreateMeshFactory(*args)
    def CreateLoaderContext(*args): return _cspace.iEngine_CreateLoaderContext(*args)
    def LoadMeshFactory(*args): return _cspace.iEngine_LoadMeshFactory(*args)
    def CreateMeshWrapper(*args): return _cspace.iEngine_CreateMeshWrapper(*args)
    def LoadMeshWrapper(*args): return _cspace.iEngine_LoadMeshWrapper(*args)
    def CreatePortalContainer(*args): return _cspace.iEngine_CreatePortalContainer(*args)
    def CreatePortal(*args): return _cspace.iEngine_CreatePortal(*args)
    def PrecacheDraw(*args): return _cspace.iEngine_PrecacheDraw(*args)
    def Draw(*args): return _cspace.iEngine_Draw(*args)
    def SetContext(*args): return _cspace.iEngine_SetContext(*args)
    def GetContext(*args): return _cspace.iEngine_GetContext(*args)
    def SetAmbientLight(*args): return _cspace.iEngine_SetAmbientLight(*args)
    def GetAmbientLight(*args): return _cspace.iEngine_GetAmbientLight(*args)
    def GetNearbyLights(*args): return _cspace.iEngine_GetNearbyLights(*args)
    def GetNearbySectors(*args): return _cspace.iEngine_GetNearbySectors(*args)
    def GetNearbyObjects(*args): return _cspace.iEngine_GetNearbyObjects(*args)
    def GetNearbyMeshes(*args): return _cspace.iEngine_GetNearbyMeshes(*args)
    def GetVisibleObjects(*args): return _cspace.iEngine_GetVisibleObjects(*args)
    def GetVisibleMeshes(*args): return _cspace.iEngine_GetVisibleMeshes(*args)
    def RemoveObject(*args): return _cspace.iEngine_RemoveObject(*args)
    def SetCacheManager(*args): return _cspace.iEngine_SetCacheManager(*args)
    def GetCacheManager(*args): return _cspace.iEngine_GetCacheManager(*args)
    def GetDefaultAmbientLight(*args): return _cspace.iEngine_GetDefaultAmbientLight(*args)
    def CreateFrustumView(*args): return _cspace.iEngine_CreateFrustumView(*args)
    def CreateObjectWatcher(*args): return _cspace.iEngine_CreateObjectWatcher(*args)
    def WantToDie(*args): return _cspace.iEngine_WantToDie(*args)
    def GetRenderLoopManager(*args): return _cspace.iEngine_GetRenderLoopManager(*args)
    def GetCurrentDefaultRenderloop(*args): return _cspace.iEngine_GetCurrentDefaultRenderloop(*args)
    def SetCurrentDefaultRenderloop(*args): return _cspace.iEngine_SetCurrentDefaultRenderloop(*args)
    def GetCurrentFrameNumber(*args): return _cspace.iEngine_GetCurrentFrameNumber(*args)
    def SetSaveableFlag(*args): return _cspace.iEngine_SetSaveableFlag(*args)
    def GetSaveableFlag(*args): return _cspace.iEngine_GetSaveableFlag(*args)
    def __del__(self, destroy=_cspace.delete_iEngine):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iEngine_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iEngine_scfGetVersion)

class iEnginePtr(iEngine):
    def __init__(self, this):
        _swig_setattr(self, iEngine, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEngine, 'thisown', 0)
        _swig_setattr(self, iEngine,self.__class__,iEngine)
_cspace.iEngine_swigregister(iEnginePtr)

iEngine_scfGetVersion = _cspace.iEngine_scfGetVersion

class iCameraSectorListener(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCameraSectorListener, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCameraSectorListener, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCameraSectorListener instance at %s>" % (self.this,)
    def NewSector(*args): return _cspace.iCameraSectorListener_NewSector(*args)
    def __del__(self, destroy=_cspace.delete_iCameraSectorListener):
        try:
            if self.thisown: destroy(self)
        except: pass

class iCameraSectorListenerPtr(iCameraSectorListener):
    def __init__(self, this):
        _swig_setattr(self, iCameraSectorListener, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCameraSectorListener, 'thisown', 0)
        _swig_setattr(self, iCameraSectorListener,self.__class__,iCameraSectorListener)
_cspace.iCameraSectorListener_swigregister(iCameraSectorListenerPtr)

class iCamera(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCamera, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCamera, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCamera instance at %s>" % (self.this,)
    def Clone(*args): return _cspace.iCamera_Clone(*args)
    def GetFOV(*args): return _cspace.iCamera_GetFOV(*args)
    def GetInvFOV(*args): return _cspace.iCamera_GetInvFOV(*args)
    def GetFOVAngle(*args): return _cspace.iCamera_GetFOVAngle(*args)
    def SetFOV(*args): return _cspace.iCamera_SetFOV(*args)
    def SetFOVAngle(*args): return _cspace.iCamera_SetFOVAngle(*args)
    def GetShiftX(*args): return _cspace.iCamera_GetShiftX(*args)
    def GetShiftY(*args): return _cspace.iCamera_GetShiftY(*args)
    def SetPerspectiveCenter(*args): return _cspace.iCamera_SetPerspectiveCenter(*args)
    def GetTransform(*args): return _cspace.iCamera_GetTransform(*args)
    def SetTransform(*args): return _cspace.iCamera_SetTransform(*args)
    def MoveWorld(*args): return _cspace.iCamera_MoveWorld(*args)
    def Move(*args): return _cspace.iCamera_Move(*args)
    def MoveWorldUnrestricted(*args): return _cspace.iCamera_MoveWorldUnrestricted(*args)
    def MoveUnrestricted(*args): return _cspace.iCamera_MoveUnrestricted(*args)
    def GetSector(*args): return _cspace.iCamera_GetSector(*args)
    def SetSector(*args): return _cspace.iCamera_SetSector(*args)
    def Correct(*args): return _cspace.iCamera_Correct(*args)
    def IsMirrored(*args): return _cspace.iCamera_IsMirrored(*args)
    def SetMirrored(*args): return _cspace.iCamera_SetMirrored(*args)
    def GetFarPlane(*args): return _cspace.iCamera_GetFarPlane(*args)
    def SetFarPlane(*args): return _cspace.iCamera_SetFarPlane(*args)
    def GetCameraNumber(*args): return _cspace.iCamera_GetCameraNumber(*args)
    def Perspective(*args): return _cspace.iCamera_Perspective(*args)
    def InvPerspective(*args): return _cspace.iCamera_InvPerspective(*args)
    def OnlyPortals(*args): return _cspace.iCamera_OnlyPortals(*args)
    def GetOnlyPortals(*args): return _cspace.iCamera_GetOnlyPortals(*args)
    def AddCameraSectorListener(*args): return _cspace.iCamera_AddCameraSectorListener(*args)
    def RemoveCameraSectorListener(*args): return _cspace.iCamera_RemoveCameraSectorListener(*args)
    def __del__(self, destroy=_cspace.delete_iCamera):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iCamera_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iCamera_scfGetVersion)

class iCameraPtr(iCamera):
    def __init__(self, this):
        _swig_setattr(self, iCamera, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCamera, 'thisown', 0)
        _swig_setattr(self, iCamera,self.__class__,iCamera)
_cspace.iCamera_swigregister(iCameraPtr)

iCamera_scfGetVersion = _cspace.iCamera_scfGetVersion

class iCameraPosition(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCameraPosition, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCameraPosition, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCameraPosition instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iCameraPosition_QueryObject(*args)
    def Clone(*args): return _cspace.iCameraPosition_Clone(*args)
    def GetSector(*args): return _cspace.iCameraPosition_GetSector(*args)
    def SetSector(*args): return _cspace.iCameraPosition_SetSector(*args)
    def GetPosition(*args): return _cspace.iCameraPosition_GetPosition(*args)
    def SetPosition(*args): return _cspace.iCameraPosition_SetPosition(*args)
    def GetUpwardVector(*args): return _cspace.iCameraPosition_GetUpwardVector(*args)
    def SetUpwardVector(*args): return _cspace.iCameraPosition_SetUpwardVector(*args)
    def GetForwardVector(*args): return _cspace.iCameraPosition_GetForwardVector(*args)
    def SetForwardVector(*args): return _cspace.iCameraPosition_SetForwardVector(*args)
    def Set(*args): return _cspace.iCameraPosition_Set(*args)
    def Load(*args): return _cspace.iCameraPosition_Load(*args)
    def SetFarPlane(*args): return _cspace.iCameraPosition_SetFarPlane(*args)
    def ClearFarPlane(*args): return _cspace.iCameraPosition_ClearFarPlane(*args)
    def GetFarPlane(*args): return _cspace.iCameraPosition_GetFarPlane(*args)
    def __del__(self, destroy=_cspace.delete_iCameraPosition):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iCameraPosition_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iCameraPosition_scfGetVersion)

class iCameraPositionPtr(iCameraPosition):
    def __init__(self, this):
        _swig_setattr(self, iCameraPosition, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCameraPosition, 'thisown', 0)
        _swig_setattr(self, iCameraPosition,self.__class__,iCameraPosition)
_cspace.iCameraPosition_swigregister(iCameraPositionPtr)

iCameraPosition_scfGetVersion = _cspace.iCameraPosition_scfGetVersion

class iCameraPositionList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCameraPositionList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCameraPositionList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCameraPositionList instance at %s>" % (self.this,)
    def NewCameraPosition(*args): return _cspace.iCameraPositionList_NewCameraPosition(*args)
    def GetCount(*args): return _cspace.iCameraPositionList_GetCount(*args)
    def Get(*args): return _cspace.iCameraPositionList_Get(*args)
    def Add(*args): return _cspace.iCameraPositionList_Add(*args)
    def Remove(*args): return _cspace.iCameraPositionList_Remove(*args)
    def RemoveAll(*args): return _cspace.iCameraPositionList_RemoveAll(*args)
    def Find(*args): return _cspace.iCameraPositionList_Find(*args)
    def FindByName(*args): return _cspace.iCameraPositionList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iCameraPositionList):
        try:
            if self.thisown: destroy(self)
        except: pass

class iCameraPositionListPtr(iCameraPositionList):
    def __init__(self, this):
        _swig_setattr(self, iCameraPositionList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCameraPositionList, 'thisown', 0)
        _swig_setattr(self, iCameraPositionList,self.__class__,iCameraPositionList)
_cspace.iCameraPositionList_swigregister(iCameraPositionListPtr)

class iTextureCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTextureCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTextureCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTextureCallback instance at %s>" % (self.this,)
    def UseTexture(*args): return _cspace.iTextureCallback_UseTexture(*args)
    def __del__(self, destroy=_cspace.delete_iTextureCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iTextureCallbackPtr(iTextureCallback):
    def __init__(self, this):
        _swig_setattr(self, iTextureCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTextureCallback, 'thisown', 0)
        _swig_setattr(self, iTextureCallback,self.__class__,iTextureCallback)
_cspace.iTextureCallback_swigregister(iTextureCallbackPtr)

class iTextureWrapper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTextureWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTextureWrapper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTextureWrapper instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iTextureWrapper_QueryObject(*args)
    def Clone(*args): return _cspace.iTextureWrapper_Clone(*args)
    def SetImageFile(*args): return _cspace.iTextureWrapper_SetImageFile(*args)
    def GetImageFile(*args): return _cspace.iTextureWrapper_GetImageFile(*args)
    def SetTextureHandle(*args): return _cspace.iTextureWrapper_SetTextureHandle(*args)
    def GetTextureHandle(*args): return _cspace.iTextureWrapper_GetTextureHandle(*args)
    def SetKeyColor(*args): return _cspace.iTextureWrapper_SetKeyColor(*args)
    def GetKeyColor(*args): return _cspace.iTextureWrapper_GetKeyColor(*args)
    def SetFlags(*args): return _cspace.iTextureWrapper_SetFlags(*args)
    def GetFlags(*args): return _cspace.iTextureWrapper_GetFlags(*args)
    def Register(*args): return _cspace.iTextureWrapper_Register(*args)
    def SetUseCallback(*args): return _cspace.iTextureWrapper_SetUseCallback(*args)
    def GetUseCallback(*args): return _cspace.iTextureWrapper_GetUseCallback(*args)
    def Visit(*args): return _cspace.iTextureWrapper_Visit(*args)
    def IsVisitRequired(*args): return _cspace.iTextureWrapper_IsVisitRequired(*args)
    def SetKeepImage(*args): return _cspace.iTextureWrapper_SetKeepImage(*args)
    def KeepImage(*args): return _cspace.iTextureWrapper_KeepImage(*args)
    def SetTextureClass(*args): return _cspace.iTextureWrapper_SetTextureClass(*args)
    def GetTextureClass(*args): return _cspace.iTextureWrapper_GetTextureClass(*args)
    def __del__(self, destroy=_cspace.delete_iTextureWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTextureWrapper_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTextureWrapper_scfGetVersion)

class iTextureWrapperPtr(iTextureWrapper):
    def __init__(self, this):
        _swig_setattr(self, iTextureWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTextureWrapper, 'thisown', 0)
        _swig_setattr(self, iTextureWrapper,self.__class__,iTextureWrapper)
_cspace.iTextureWrapper_swigregister(iTextureWrapperPtr)

iTextureWrapper_scfGetVersion = _cspace.iTextureWrapper_scfGetVersion

class iTextureList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTextureList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTextureList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTextureList instance at %s>" % (self.this,)
    def NewTexture(*args): return _cspace.iTextureList_NewTexture(*args)
    def GetCount(*args): return _cspace.iTextureList_GetCount(*args)
    def Get(*args): return _cspace.iTextureList_Get(*args)
    def Add(*args): return _cspace.iTextureList_Add(*args)
    def Remove(*args): return _cspace.iTextureList_Remove(*args)
    def RemoveAll(*args): return _cspace.iTextureList_RemoveAll(*args)
    def Find(*args): return _cspace.iTextureList_Find(*args)
    def FindByName(*args): return _cspace.iTextureList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iTextureList):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTextureList_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTextureList_scfGetVersion)

class iTextureListPtr(iTextureList):
    def __init__(self, this):
        _swig_setattr(self, iTextureList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTextureList, 'thisown', 0)
        _swig_setattr(self, iTextureList,self.__class__,iTextureList)
_cspace.iTextureList_swigregister(iTextureListPtr)

iTextureList_scfGetVersion = _cspace.iTextureList_scfGetVersion

class iMaterialWrapper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMaterialWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMaterialWrapper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMaterialWrapper instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iMaterialWrapper_QueryObject(*args)
    def Clone(*args): return _cspace.iMaterialWrapper_Clone(*args)
    def SetMaterialHandle(*args): return _cspace.iMaterialWrapper_SetMaterialHandle(*args)
    def GetMaterialHandle(*args): return _cspace.iMaterialWrapper_GetMaterialHandle(*args)
    def SetMaterial(*args): return _cspace.iMaterialWrapper_SetMaterial(*args)
    def GetMaterial(*args): return _cspace.iMaterialWrapper_GetMaterial(*args)
    def Register(*args): return _cspace.iMaterialWrapper_Register(*args)
    def Visit(*args): return _cspace.iMaterialWrapper_Visit(*args)
    def IsVisitRequired(*args): return _cspace.iMaterialWrapper_IsVisitRequired(*args)
    def __del__(self, destroy=_cspace.delete_iMaterialWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMaterialWrapper_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMaterialWrapper_scfGetVersion)

class iMaterialWrapperPtr(iMaterialWrapper):
    def __init__(self, this):
        _swig_setattr(self, iMaterialWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMaterialWrapper, 'thisown', 0)
        _swig_setattr(self, iMaterialWrapper,self.__class__,iMaterialWrapper)
_cspace.iMaterialWrapper_swigregister(iMaterialWrapperPtr)

iMaterialWrapper_scfGetVersion = _cspace.iMaterialWrapper_scfGetVersion

class iMaterialEngine(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMaterialEngine, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMaterialEngine, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMaterialEngine instance at %s>" % (self.this,)
    def GetTextureWrapper(*args): return _cspace.iMaterialEngine_GetTextureWrapper(*args)
    def Visit(*args): return _cspace.iMaterialEngine_Visit(*args)
    def IsVisitRequired(*args): return _cspace.iMaterialEngine_IsVisitRequired(*args)
    def __del__(self, destroy=_cspace.delete_iMaterialEngine):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMaterialEnginePtr(iMaterialEngine):
    def __init__(self, this):
        _swig_setattr(self, iMaterialEngine, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMaterialEngine, 'thisown', 0)
        _swig_setattr(self, iMaterialEngine,self.__class__,iMaterialEngine)
_cspace.iMaterialEngine_swigregister(iMaterialEnginePtr)

class iMaterialList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMaterialList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMaterialList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMaterialList instance at %s>" % (self.this,)
    def NewMaterial(*args): return _cspace.iMaterialList_NewMaterial(*args)
    def GetCount(*args): return _cspace.iMaterialList_GetCount(*args)
    def Get(*args): return _cspace.iMaterialList_Get(*args)
    def Add(*args): return _cspace.iMaterialList_Add(*args)
    def Remove(*args): return _cspace.iMaterialList_Remove(*args)
    def RemoveAll(*args): return _cspace.iMaterialList_RemoveAll(*args)
    def Find(*args): return _cspace.iMaterialList_Find(*args)
    def FindByName(*args): return _cspace.iMaterialList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iMaterialList):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMaterialListPtr(iMaterialList):
    def __init__(self, this):
        _swig_setattr(self, iMaterialList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMaterialList, 'thisown', 0)
        _swig_setattr(self, iMaterialList,self.__class__,iMaterialList)
_cspace.iMaterialList_swigregister(iMaterialListPtr)

CS_ENTITY_DETAIL = _cspace.CS_ENTITY_DETAIL
CS_ENTITY_CAMERA = _cspace.CS_ENTITY_CAMERA
CS_ENTITY_INVISIBLEMESH = _cspace.CS_ENTITY_INVISIBLEMESH
CS_ENTITY_NOSHADOWS = _cspace.CS_ENTITY_NOSHADOWS
CS_ENTITY_NOLIGHTING = _cspace.CS_ENTITY_NOLIGHTING
CS_ENTITY_NOHITBEAM = _cspace.CS_ENTITY_NOHITBEAM
CS_LIGHTINGUPDATE_SORTRELEVANCE = _cspace.CS_LIGHTINGUPDATE_SORTRELEVANCE
CS_LIGHTINGUPDATE_ALWAYSUPDATE = _cspace.CS_LIGHTINGUPDATE_ALWAYSUPDATE
class iMeshDrawCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshDrawCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshDrawCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshDrawCallback instance at %s>" % (self.this,)
    def BeforeDrawing(*args): return _cspace.iMeshDrawCallback_BeforeDrawing(*args)
    def __del__(self, destroy=_cspace.delete_iMeshDrawCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMeshDrawCallbackPtr(iMeshDrawCallback):
    def __init__(self, this):
        _swig_setattr(self, iMeshDrawCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshDrawCallback, 'thisown', 0)
        _swig_setattr(self, iMeshDrawCallback,self.__class__,iMeshDrawCallback)
_cspace.iMeshDrawCallback_swigregister(iMeshDrawCallbackPtr)

class iMeshWrapper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshWrapper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshWrapper instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iMeshWrapper_QueryObject(*args)
    def GetMeshObject(*args): return _cspace.iMeshWrapper_GetMeshObject(*args)
    def SetMeshObject(*args): return _cspace.iMeshWrapper_SetMeshObject(*args)
    def GetPortalContainer(*args): return _cspace.iMeshWrapper_GetPortalContainer(*args)
    def GetLightingInfo(*args): return _cspace.iMeshWrapper_GetLightingInfo(*args)
    def GetShadowReceiver(*args): return _cspace.iMeshWrapper_GetShadowReceiver(*args)
    def GetShadowCaster(*args): return _cspace.iMeshWrapper_GetShadowCaster(*args)
    def GetVisibilityNumber(*args): return _cspace.iMeshWrapper_GetVisibilityNumber(*args)
    def GetFactory(*args): return _cspace.iMeshWrapper_GetFactory(*args)
    def SetFactory(*args): return _cspace.iMeshWrapper_SetFactory(*args)
    def SetLightingUpdate(*args): return _cspace.iMeshWrapper_SetLightingUpdate(*args)
    def GetMovable(*args): return _cspace.iMeshWrapper_GetMovable(*args)
    def PlaceMesh(*args): return _cspace.iMeshWrapper_PlaceMesh(*args)
    def HitBeamBBox(*args): return _cspace.iMeshWrapper_HitBeamBBox(*args)
    def HitBeamOutline(*args): return _cspace.iMeshWrapper_HitBeamOutline(*args)
    def HitBeamObject(*args): return _cspace.iMeshWrapper_HitBeamObject(*args)
    def HitBeam(*args): return _cspace.iMeshWrapper_HitBeam(*args)
    def SetDrawCallback(*args): return _cspace.iMeshWrapper_SetDrawCallback(*args)
    def RemoveDrawCallback(*args): return _cspace.iMeshWrapper_RemoveDrawCallback(*args)
    def GetDrawCallbackCount(*args): return _cspace.iMeshWrapper_GetDrawCallbackCount(*args)
    def GetDrawCallback(*args): return _cspace.iMeshWrapper_GetDrawCallback(*args)
    def SetRenderPriority(*args): return _cspace.iMeshWrapper_SetRenderPriority(*args)
    def GetRenderPriority(*args): return _cspace.iMeshWrapper_GetRenderPriority(*args)
    def SetRenderPriorityRecursive(*args): return _cspace.iMeshWrapper_SetRenderPriorityRecursive(*args)
    def GetFlags(*args): return _cspace.iMeshWrapper_GetFlags(*args)
    def SetFlagsRecursive(*args): return _cspace.iMeshWrapper_SetFlagsRecursive(*args)
    def SetZBufMode(*args): return _cspace.iMeshWrapper_SetZBufMode(*args)
    def GetZBufMode(*args): return _cspace.iMeshWrapper_GetZBufMode(*args)
    def SetZBufModeRecursive(*args): return _cspace.iMeshWrapper_SetZBufModeRecursive(*args)
    def HardTransform(*args): return _cspace.iMeshWrapper_HardTransform(*args)
    def GetWorldBoundingBox(*args): return _cspace.iMeshWrapper_GetWorldBoundingBox(*args)
    def GetTransformedBoundingBox(*args): return _cspace.iMeshWrapper_GetTransformedBoundingBox(*args)
    def GetScreenBoundingBox(*args): return _cspace.iMeshWrapper_GetScreenBoundingBox(*args)
    def GetChildren(*args): return _cspace.iMeshWrapper_GetChildren(*args)
    def GetParentContainer(*args): return _cspace.iMeshWrapper_GetParentContainer(*args)
    def SetParentContainer(*args): return _cspace.iMeshWrapper_SetParentContainer(*args)
    def GetRadius(*args): return _cspace.iMeshWrapper_GetRadius(*args)
    def CreateStaticLOD(*args): return _cspace.iMeshWrapper_CreateStaticLOD(*args)
    def DestroyStaticLOD(*args): return _cspace.iMeshWrapper_DestroyStaticLOD(*args)
    def GetStaticLOD(*args): return _cspace.iMeshWrapper_GetStaticLOD(*args)
    def AddMeshToStaticLOD(*args): return _cspace.iMeshWrapper_AddMeshToStaticLOD(*args)
    def RemoveMeshFromStaticLOD(*args): return _cspace.iMeshWrapper_RemoveMeshFromStaticLOD(*args)
    def DrawShadow(*args): return _cspace.iMeshWrapper_DrawShadow(*args)
    def DrawLight(*args): return _cspace.iMeshWrapper_DrawLight(*args)
    def CastHardwareShadow(*args): return _cspace.iMeshWrapper_CastHardwareShadow(*args)
    def SetDrawAfterShadow(*args): return _cspace.iMeshWrapper_SetDrawAfterShadow(*args)
    def GetDrawAfterShadow(*args): return _cspace.iMeshWrapper_GetDrawAfterShadow(*args)
    def GetSVContext(*args): return _cspace.iMeshWrapper_GetSVContext(*args)
    def __del__(self, destroy=_cspace.delete_iMeshWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMeshWrapper_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMeshWrapper_scfGetVersion)

class iMeshWrapperPtr(iMeshWrapper):
    def __init__(self, this):
        _swig_setattr(self, iMeshWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshWrapper, 'thisown', 0)
        _swig_setattr(self, iMeshWrapper,self.__class__,iMeshWrapper)
_cspace.iMeshWrapper_swigregister(iMeshWrapperPtr)

iMeshWrapper_scfGetVersion = _cspace.iMeshWrapper_scfGetVersion

class iMeshFactoryWrapper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshFactoryWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshFactoryWrapper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshFactoryWrapper instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iMeshFactoryWrapper_QueryObject(*args)
    def GetMeshObjectFactory(*args): return _cspace.iMeshFactoryWrapper_GetMeshObjectFactory(*args)
    def SetMeshObjectFactory(*args): return _cspace.iMeshFactoryWrapper_SetMeshObjectFactory(*args)
    def HardTransform(*args): return _cspace.iMeshFactoryWrapper_HardTransform(*args)
    def CreateMeshWrapper(*args): return _cspace.iMeshFactoryWrapper_CreateMeshWrapper(*args)
    def GetParentContainer(*args): return _cspace.iMeshFactoryWrapper_GetParentContainer(*args)
    def SetParentContainer(*args): return _cspace.iMeshFactoryWrapper_SetParentContainer(*args)
    def GetChildren(*args): return _cspace.iMeshFactoryWrapper_GetChildren(*args)
    def GetTransform(*args): return _cspace.iMeshFactoryWrapper_GetTransform(*args)
    def SetTransform(*args): return _cspace.iMeshFactoryWrapper_SetTransform(*args)
    def CreateStaticLOD(*args): return _cspace.iMeshFactoryWrapper_CreateStaticLOD(*args)
    def DestroyStaticLOD(*args): return _cspace.iMeshFactoryWrapper_DestroyStaticLOD(*args)
    def SetStaticLOD(*args): return _cspace.iMeshFactoryWrapper_SetStaticLOD(*args)
    def GetStaticLOD(*args): return _cspace.iMeshFactoryWrapper_GetStaticLOD(*args)
    def AddFactoryToStaticLOD(*args): return _cspace.iMeshFactoryWrapper_AddFactoryToStaticLOD(*args)
    def RemoveFactoryFromStaticLOD(*args): return _cspace.iMeshFactoryWrapper_RemoveFactoryFromStaticLOD(*args)
    def SetZBufMode(*args): return _cspace.iMeshFactoryWrapper_SetZBufMode(*args)
    def GetZBufMode(*args): return _cspace.iMeshFactoryWrapper_GetZBufMode(*args)
    def SetZBufModeRecursive(*args): return _cspace.iMeshFactoryWrapper_SetZBufModeRecursive(*args)
    def SetRenderPriority(*args): return _cspace.iMeshFactoryWrapper_SetRenderPriority(*args)
    def GetRenderPriority(*args): return _cspace.iMeshFactoryWrapper_GetRenderPriority(*args)
    def SetRenderPriorityRecursive(*args): return _cspace.iMeshFactoryWrapper_SetRenderPriorityRecursive(*args)
    def GetSVContext(*args): return _cspace.iMeshFactoryWrapper_GetSVContext(*args)
    def __del__(self, destroy=_cspace.delete_iMeshFactoryWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMeshFactoryWrapper_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMeshFactoryWrapper_scfGetVersion)

class iMeshFactoryWrapperPtr(iMeshFactoryWrapper):
    def __init__(self, this):
        _swig_setattr(self, iMeshFactoryWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshFactoryWrapper, 'thisown', 0)
        _swig_setattr(self, iMeshFactoryWrapper,self.__class__,iMeshFactoryWrapper)
_cspace.iMeshFactoryWrapper_swigregister(iMeshFactoryWrapperPtr)

iMeshFactoryWrapper_scfGetVersion = _cspace.iMeshFactoryWrapper_scfGetVersion

class iMeshList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshList instance at %s>" % (self.this,)
    def GetCount(*args): return _cspace.iMeshList_GetCount(*args)
    def Get(*args): return _cspace.iMeshList_Get(*args)
    def Add(*args): return _cspace.iMeshList_Add(*args)
    def Remove(*args): return _cspace.iMeshList_Remove(*args)
    def RemoveAll(*args): return _cspace.iMeshList_RemoveAll(*args)
    def Find(*args): return _cspace.iMeshList_Find(*args)
    def FindByName(*args): return _cspace.iMeshList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iMeshList):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMeshListPtr(iMeshList):
    def __init__(self, this):
        _swig_setattr(self, iMeshList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshList, 'thisown', 0)
        _swig_setattr(self, iMeshList,self.__class__,iMeshList)
_cspace.iMeshList_swigregister(iMeshListPtr)

class iMeshFactoryList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshFactoryList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshFactoryList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshFactoryList instance at %s>" % (self.this,)
    def GetCount(*args): return _cspace.iMeshFactoryList_GetCount(*args)
    def Get(*args): return _cspace.iMeshFactoryList_Get(*args)
    def Add(*args): return _cspace.iMeshFactoryList_Add(*args)
    def Remove(*args): return _cspace.iMeshFactoryList_Remove(*args)
    def RemoveAll(*args): return _cspace.iMeshFactoryList_RemoveAll(*args)
    def Find(*args): return _cspace.iMeshFactoryList_Find(*args)
    def FindByName(*args): return _cspace.iMeshFactoryList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iMeshFactoryList):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMeshFactoryListPtr(iMeshFactoryList):
    def __init__(self, this):
        _swig_setattr(self, iMeshFactoryList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshFactoryList, 'thisown', 0)
        _swig_setattr(self, iMeshFactoryList,self.__class__,iMeshFactoryList)
_cspace.iMeshFactoryList_swigregister(iMeshFactoryListPtr)

class iMeshWrapperIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshWrapperIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshWrapperIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshWrapperIterator instance at %s>" % (self.this,)
    def Next(*args): return _cspace.iMeshWrapperIterator_Next(*args)
    def Reset(*args): return _cspace.iMeshWrapperIterator_Reset(*args)
    def HasNext(*args): return _cspace.iMeshWrapperIterator_HasNext(*args)
    def __del__(self, destroy=_cspace.delete_iMeshWrapperIterator):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMeshWrapperIterator_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMeshWrapperIterator_scfGetVersion)

class iMeshWrapperIteratorPtr(iMeshWrapperIterator):
    def __init__(self, this):
        _swig_setattr(self, iMeshWrapperIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshWrapperIterator, 'thisown', 0)
        _swig_setattr(self, iMeshWrapperIterator,self.__class__,iMeshWrapperIterator)
_cspace.iMeshWrapperIterator_swigregister(iMeshWrapperIteratorPtr)

iMeshWrapperIterator_scfGetVersion = _cspace.iMeshWrapperIterator_scfGetVersion

class iMovableListener(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMovableListener, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMovableListener, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMovableListener instance at %s>" % (self.this,)
    def MovableChanged(*args): return _cspace.iMovableListener_MovableChanged(*args)
    def MovableDestroyed(*args): return _cspace.iMovableListener_MovableDestroyed(*args)
    def __del__(self, destroy=_cspace.delete_iMovableListener):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMovableListener_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMovableListener_scfGetVersion)

class iMovableListenerPtr(iMovableListener):
    def __init__(self, this):
        _swig_setattr(self, iMovableListener, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMovableListener, 'thisown', 0)
        _swig_setattr(self, iMovableListener,self.__class__,iMovableListener)
_cspace.iMovableListener_swigregister(iMovableListenerPtr)

iMovableListener_scfGetVersion = _cspace.iMovableListener_scfGetVersion

class iMovable(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMovable, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMovable, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMovable instance at %s>" % (self.this,)
    def GetParent(*args): return _cspace.iMovable_GetParent(*args)
    def SetParent(*args): return _cspace.iMovable_SetParent(*args)
    def SetSector(*args): return _cspace.iMovable_SetSector(*args)
    def ClearSectors(*args): return _cspace.iMovable_ClearSectors(*args)
    def GetSectors(*args): return _cspace.iMovable_GetSectors(*args)
    def InSector(*args): return _cspace.iMovable_InSector(*args)
    def SetPosition(*args): return _cspace.iMovable_SetPosition(*args)
    def GetPosition(*args): return _cspace.iMovable_GetPosition(*args)
    def GetFullPosition(*args): return _cspace.iMovable_GetFullPosition(*args)
    def SetTransform(*args): return _cspace.iMovable_SetTransform(*args)
    def GetTransform(*args): return _cspace.iMovable_GetTransform(*args)
    def GetFullTransform(*args): return _cspace.iMovable_GetFullTransform(*args)
    def MovePosition(*args): return _cspace.iMovable_MovePosition(*args)
    def Transform(*args): return _cspace.iMovable_Transform(*args)
    def AddListener(*args): return _cspace.iMovable_AddListener(*args)
    def RemoveListener(*args): return _cspace.iMovable_RemoveListener(*args)
    def UpdateMove(*args): return _cspace.iMovable_UpdateMove(*args)
    def GetUpdateNumber(*args): return _cspace.iMovable_GetUpdateNumber(*args)
    def IsTransformIdentity(*args): return _cspace.iMovable_IsTransformIdentity(*args)
    def IsFullTransformIdentity(*args): return _cspace.iMovable_IsFullTransformIdentity(*args)
    def TransformIdentity(*args): return _cspace.iMovable_TransformIdentity(*args)
    def __del__(self, destroy=_cspace.delete_iMovable):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMovable_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMovable_scfGetVersion)

class iMovablePtr(iMovable):
    def __init__(self, this):
        _swig_setattr(self, iMovable, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMovable, 'thisown', 0)
        _swig_setattr(self, iMovable,self.__class__,iMovable)
_cspace.iMovable_swigregister(iMovablePtr)

iMovable_scfGetVersion = _cspace.iMovable_scfGetVersion

class iRegion(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iRegion, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iRegion, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iRegion instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iRegion_QueryObject(*args)
    def Add(*args): return _cspace.iRegion_Add(*args)
    def Remove(*args): return _cspace.iRegion_Remove(*args)
    def Clear(*args): return _cspace.iRegion_Clear(*args)
    def DeleteAll(*args): return _cspace.iRegion_DeleteAll(*args)
    def PrepareTextures(*args): return _cspace.iRegion_PrepareTextures(*args)
    def ShineLights(*args): return _cspace.iRegion_ShineLights(*args)
    def Prepare(*args): return _cspace.iRegion_Prepare(*args)
    def FindSector(*args): return _cspace.iRegion_FindSector(*args)
    def FindMeshObject(*args): return _cspace.iRegion_FindMeshObject(*args)
    def FindMeshFactory(*args): return _cspace.iRegion_FindMeshFactory(*args)
    def FindTexture(*args): return _cspace.iRegion_FindTexture(*args)
    def FindMaterial(*args): return _cspace.iRegion_FindMaterial(*args)
    def FindCameraPosition(*args): return _cspace.iRegion_FindCameraPosition(*args)
    def FindCollection(*args): return _cspace.iRegion_FindCollection(*args)
    def IsInRegion(*args): return _cspace.iRegion_IsInRegion(*args)
    def __del__(self, destroy=_cspace.delete_iRegion):
        try:
            if self.thisown: destroy(self)
        except: pass

class iRegionPtr(iRegion):
    def __init__(self, this):
        _swig_setattr(self, iRegion, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iRegion, 'thisown', 0)
        _swig_setattr(self, iRegion,self.__class__,iRegion)
_cspace.iRegion_swigregister(iRegionPtr)

class iRegionList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iRegionList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iRegionList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iRegionList instance at %s>" % (self.this,)
    def GetCount(*args): return _cspace.iRegionList_GetCount(*args)
    def Get(*args): return _cspace.iRegionList_Get(*args)
    def Add(*args): return _cspace.iRegionList_Add(*args)
    def Remove(*args): return _cspace.iRegionList_Remove(*args)
    def RemoveAll(*args): return _cspace.iRegionList_RemoveAll(*args)
    def Find(*args): return _cspace.iRegionList_Find(*args)
    def FindByName(*args): return _cspace.iRegionList_FindByName(*args)
    def __del__(self, destroy=_cspace.delete_iRegionList):
        try:
            if self.thisown: destroy(self)
        except: pass

class iRegionListPtr(iRegionList):
    def __init__(self, this):
        _swig_setattr(self, iRegionList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iRegionList, 'thisown', 0)
        _swig_setattr(self, iRegionList,self.__class__,iRegionList)
_cspace.iRegionList_swigregister(iRegionListPtr)

class iVisibilityObjectIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVisibilityObjectIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVisibilityObjectIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVisibilityObjectIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iVisibilityObjectIterator_HasNext(*args)
    def Next(*args): return _cspace.iVisibilityObjectIterator_Next(*args)
    def Reset(*args): return _cspace.iVisibilityObjectIterator_Reset(*args)
    def __del__(self, destroy=_cspace.delete_iVisibilityObjectIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iVisibilityObjectIteratorPtr(iVisibilityObjectIterator):
    def __init__(self, this):
        _swig_setattr(self, iVisibilityObjectIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVisibilityObjectIterator, 'thisown', 0)
        _swig_setattr(self, iVisibilityObjectIterator,self.__class__,iVisibilityObjectIterator)
_cspace.iVisibilityObjectIterator_swigregister(iVisibilityObjectIteratorPtr)

class iVisibilityCullerListener(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVisibilityCullerListener, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVisibilityCullerListener, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVisibilityCullerListener instance at %s>" % (self.this,)
    def ObjectVisible(*args): return _cspace.iVisibilityCullerListener_ObjectVisible(*args)
    def __del__(self, destroy=_cspace.delete_iVisibilityCullerListener):
        try:
            if self.thisown: destroy(self)
        except: pass

class iVisibilityCullerListenerPtr(iVisibilityCullerListener):
    def __init__(self, this):
        _swig_setattr(self, iVisibilityCullerListener, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVisibilityCullerListener, 'thisown', 0)
        _swig_setattr(self, iVisibilityCullerListener,self.__class__,iVisibilityCullerListener)
_cspace.iVisibilityCullerListener_swigregister(iVisibilityCullerListenerPtr)

class iVisibilityCuller(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVisibilityCuller, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVisibilityCuller, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVisibilityCuller instance at %s>" % (self.this,)
    def Setup(*args): return _cspace.iVisibilityCuller_Setup(*args)
    def RegisterVisObject(*args): return _cspace.iVisibilityCuller_RegisterVisObject(*args)
    def UnregisterVisObject(*args): return _cspace.iVisibilityCuller_UnregisterVisObject(*args)
    def PrecacheCulling(*args): return _cspace.iVisibilityCuller_PrecacheCulling(*args)
    def VisTest(*args): return _cspace.iVisibilityCuller_VisTest(*args)
    def IntersectSegmentSloppy(*args): return _cspace.iVisibilityCuller_IntersectSegmentSloppy(*args)
    def IntersectSegment(*args): return _cspace.iVisibilityCuller_IntersectSegment(*args)
    def CastShadows(*args): return _cspace.iVisibilityCuller_CastShadows(*args)
    def ParseCullerParameters(*args): return _cspace.iVisibilityCuller_ParseCullerParameters(*args)
    def __del__(self, destroy=_cspace.delete_iVisibilityCuller):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iVisibilityCuller_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iVisibilityCuller_scfGetVersion)

class iVisibilityCullerPtr(iVisibilityCuller):
    def __init__(self, this):
        _swig_setattr(self, iVisibilityCuller, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVisibilityCuller, 'thisown', 0)
        _swig_setattr(self, iVisibilityCuller,self.__class__,iVisibilityCuller)
_cspace.iVisibilityCuller_swigregister(iVisibilityCullerPtr)

iVisibilityCuller_scfGetVersion = _cspace.iVisibilityCuller_scfGetVersion

CS_CULLER_HINT_GOODOCCLUDER = _cspace.CS_CULLER_HINT_GOODOCCLUDER
CS_CULLER_HINT_BADOCCLUDER = _cspace.CS_CULLER_HINT_BADOCCLUDER
class iVisibilityObject(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVisibilityObject, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVisibilityObject, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVisibilityObject instance at %s>" % (self.this,)
    def GetMovable(*args): return _cspace.iVisibilityObject_GetMovable(*args)
    def GetMeshWrapper(*args): return _cspace.iVisibilityObject_GetMeshWrapper(*args)
    def SetVisibilityNumber(*args): return _cspace.iVisibilityObject_SetVisibilityNumber(*args)
    def GetVisibilityNumber(*args): return _cspace.iVisibilityObject_GetVisibilityNumber(*args)
    def GetObjectModel(*args): return _cspace.iVisibilityObject_GetObjectModel(*args)
    def GetCullerFlags(*args): return _cspace.iVisibilityObject_GetCullerFlags(*args)
    def __del__(self, destroy=_cspace.delete_iVisibilityObject):
        try:
            if self.thisown: destroy(self)
        except: pass

class iVisibilityObjectPtr(iVisibilityObject):
    def __init__(self, this):
        _swig_setattr(self, iVisibilityObject, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVisibilityObject, 'thisown', 0)
        _swig_setattr(self, iVisibilityObject,self.__class__,iVisibilityObject)
_cspace.iVisibilityObject_swigregister(iVisibilityObjectPtr)

CS_PORTAL_CLIPDEST = _cspace.CS_PORTAL_CLIPDEST
CS_PORTAL_CLIPSTRADDLING = _cspace.CS_PORTAL_CLIPSTRADDLING
CS_PORTAL_ZFILL = _cspace.CS_PORTAL_ZFILL
CS_PORTAL_WARP = _cspace.CS_PORTAL_WARP
CS_PORTAL_MIRROR = _cspace.CS_PORTAL_MIRROR
CS_PORTAL_STATICDEST = _cspace.CS_PORTAL_STATICDEST
CS_PORTAL_FLOAT = _cspace.CS_PORTAL_FLOAT
CS_PORTAL_COLLDET = _cspace.CS_PORTAL_COLLDET
CS_PORTAL_VISCULL = _cspace.CS_PORTAL_VISCULL
class iPortalCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPortalCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPortalCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPortalCallback instance at %s>" % (self.this,)
    def Traverse(*args): return _cspace.iPortalCallback_Traverse(*args)
    def __del__(self, destroy=_cspace.delete_iPortalCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iPortalCallbackPtr(iPortalCallback):
    def __init__(self, this):
        _swig_setattr(self, iPortalCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPortalCallback, 'thisown', 0)
        _swig_setattr(self, iPortalCallback,self.__class__,iPortalCallback)
_cspace.iPortalCallback_swigregister(iPortalCallbackPtr)

class iPortal(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPortal, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPortal, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPortal instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iPortal_QueryObject(*args)
    def SetName(*args): return _cspace.iPortal_SetName(*args)
    def GetName(*args): return _cspace.iPortal_GetName(*args)
    def GetSector(*args): return _cspace.iPortal_GetSector(*args)
    def GetVertices(*args): return _cspace.iPortal_GetVertices(*args)
    def GetWorldVertices(*args): return _cspace.iPortal_GetWorldVertices(*args)
    def GetVertexIndices(*args): return _cspace.iPortal_GetVertexIndices(*args)
    def GetVertexIndicesCount(*args): return _cspace.iPortal_GetVertexIndicesCount(*args)
    def GetObjectPlane(*args): return _cspace.iPortal_GetObjectPlane(*args)
    def GetWorldPlane(*args): return _cspace.iPortal_GetWorldPlane(*args)
    def ComputeCameraPlane(*args): return _cspace.iPortal_ComputeCameraPlane(*args)
    def PointOnPolygon(*args): return _cspace.iPortal_PointOnPolygon(*args)
    def SetSector(*args): return _cspace.iPortal_SetSector(*args)
    def GetFlags(*args): return _cspace.iPortal_GetFlags(*args)
    def SetMaximumSectorVisit(*args): return _cspace.iPortal_SetMaximumSectorVisit(*args)
    def GetMaximumSectorVisit(*args): return _cspace.iPortal_GetMaximumSectorVisit(*args)
    def SetPortalCallback(*args): return _cspace.iPortal_SetPortalCallback(*args)
    def RemovePortalCallback(*args): return _cspace.iPortal_RemovePortalCallback(*args)
    def GetPortalCallbackCount(*args): return _cspace.iPortal_GetPortalCallbackCount(*args)
    def GetPortalCallback(*args): return _cspace.iPortal_GetPortalCallback(*args)
    def SetMissingSectorCallback(*args): return _cspace.iPortal_SetMissingSectorCallback(*args)
    def RemoveMissingSectorCallback(*args): return _cspace.iPortal_RemoveMissingSectorCallback(*args)
    def GetMissingSectorCallbackCount(*args): return _cspace.iPortal_GetMissingSectorCallbackCount(*args)
    def GetMissingSectorCallback(*args): return _cspace.iPortal_GetMissingSectorCallback(*args)
    def GetTextureFilter(*args): return _cspace.iPortal_GetTextureFilter(*args)
    def SetFilter(*args): return _cspace.iPortal_SetFilter(*args)
    def GetColorFilter(*args): return _cspace.iPortal_GetColorFilter(*args)
    def SetWarp(*args): return _cspace.iPortal_SetWarp(*args)
    def SetMirror(*args): return _cspace.iPortal_SetMirror(*args)
    def GetWarp(*args): return _cspace.iPortal_GetWarp(*args)
    def HardTransform(*args): return _cspace.iPortal_HardTransform(*args)
    def ObjectToWorld(*args): return _cspace.iPortal_ObjectToWorld(*args)
    def Warp(*args): return _cspace.iPortal_Warp(*args)
    def WarpSpace(*args): return _cspace.iPortal_WarpSpace(*args)
    def CompleteSector(*args): return _cspace.iPortal_CompleteSector(*args)
    def CheckFrustum(*args): return _cspace.iPortal_CheckFrustum(*args)
    def HitBeamPortals(*args): return _cspace.iPortal_HitBeamPortals(*args)
    def __del__(self, destroy=_cspace.delete_iPortal):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iPortal_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iPortal_scfGetVersion)

class iPortalPtr(iPortal):
    def __init__(self, this):
        _swig_setattr(self, iPortal, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPortal, 'thisown', 0)
        _swig_setattr(self, iPortal,self.__class__,iPortal)
_cspace.iPortal_swigregister(iPortalPtr)

iPortal_scfGetVersion = _cspace.iPortal_scfGetVersion

class iPortalContainer(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPortalContainer, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPortalContainer, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPortalContainer instance at %s>" % (self.this,)
    def GetPortalCount(*args): return _cspace.iPortalContainer_GetPortalCount(*args)
    def GetPortal(*args): return _cspace.iPortalContainer_GetPortal(*args)
    def CreatePortal(*args): return _cspace.iPortalContainer_CreatePortal(*args)
    def RemovePortal(*args): return _cspace.iPortalContainer_RemovePortal(*args)
    def Draw(*args): return _cspace.iPortalContainer_Draw(*args)
    def __del__(self, destroy=_cspace.delete_iPortalContainer):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iPortalContainer_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iPortalContainer_scfGetVersion)

class iPortalContainerPtr(iPortalContainer):
    def __init__(self, this):
        _swig_setattr(self, iPortalContainer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPortalContainer, 'thisown', 0)
        _swig_setattr(self, iPortalContainer,self.__class__,iPortalContainer)
_cspace.iPortalContainer_swigregister(iPortalContainerPtr)

iPortalContainer_scfGetVersion = _cspace.iPortalContainer_scfGetVersion

class iGeneralMeshCommonState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGeneralMeshCommonState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGeneralMeshCommonState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGeneralMeshCommonState instance at %s>" % (self.this,)
    def SetMaterialWrapper(*args): return _cspace.iGeneralMeshCommonState_SetMaterialWrapper(*args)
    def GetMaterialWrapper(*args): return _cspace.iGeneralMeshCommonState_GetMaterialWrapper(*args)
    def SetMixMode(*args): return _cspace.iGeneralMeshCommonState_SetMixMode(*args)
    def GetMixMode(*args): return _cspace.iGeneralMeshCommonState_GetMixMode(*args)
    def SetLighting(*args): return _cspace.iGeneralMeshCommonState_SetLighting(*args)
    def IsLighting(*args): return _cspace.iGeneralMeshCommonState_IsLighting(*args)
    def SetColor(*args): return _cspace.iGeneralMeshCommonState_SetColor(*args)
    def GetColor(*args): return _cspace.iGeneralMeshCommonState_GetColor(*args)
    def SetManualColors(*args): return _cspace.iGeneralMeshCommonState_SetManualColors(*args)
    def IsManualColors(*args): return _cspace.iGeneralMeshCommonState_IsManualColors(*args)
    def SetShadowCasting(*args): return _cspace.iGeneralMeshCommonState_SetShadowCasting(*args)
    def IsShadowCasting(*args): return _cspace.iGeneralMeshCommonState_IsShadowCasting(*args)
    def SetShadowReceiving(*args): return _cspace.iGeneralMeshCommonState_SetShadowReceiving(*args)
    def IsShadowReceiving(*args): return _cspace.iGeneralMeshCommonState_IsShadowReceiving(*args)
    def AddRenderBuffer(*args): return _cspace.iGeneralMeshCommonState_AddRenderBuffer(*args)
    def RemoveRenderBuffer(*args): return _cspace.iGeneralMeshCommonState_RemoveRenderBuffer(*args)
    def __del__(self, destroy=_cspace.delete_iGeneralMeshCommonState):
        try:
            if self.thisown: destroy(self)
        except: pass

class iGeneralMeshCommonStatePtr(iGeneralMeshCommonState):
    def __init__(self, this):
        _swig_setattr(self, iGeneralMeshCommonState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGeneralMeshCommonState, 'thisown', 0)
        _swig_setattr(self, iGeneralMeshCommonState,self.__class__,iGeneralMeshCommonState)
_cspace.iGeneralMeshCommonState_swigregister(iGeneralMeshCommonStatePtr)

class iGeneralMeshState(iGeneralMeshCommonState):
    __swig_setmethods__ = {}
    for _s in [iGeneralMeshCommonState]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGeneralMeshState, name, value)
    __swig_getmethods__ = {}
    for _s in [iGeneralMeshCommonState]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGeneralMeshState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGeneralMeshState instance at %s>" % (self.this,)
    def SetAnimationControl(*args): return _cspace.iGeneralMeshState_SetAnimationControl(*args)
    def GetAnimationControl(*args): return _cspace.iGeneralMeshState_GetAnimationControl(*args)
    def ClearSubMeshes(*args): return _cspace.iGeneralMeshState_ClearSubMeshes(*args)
    def AddSubMesh(*args): return _cspace.iGeneralMeshState_AddSubMesh(*args)
    def __del__(self, destroy=_cspace.delete_iGeneralMeshState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iGeneralMeshState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iGeneralMeshState_scfGetVersion)

class iGeneralMeshStatePtr(iGeneralMeshState):
    def __init__(self, this):
        _swig_setattr(self, iGeneralMeshState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGeneralMeshState, 'thisown', 0)
        _swig_setattr(self, iGeneralMeshState,self.__class__,iGeneralMeshState)
_cspace.iGeneralMeshState_swigregister(iGeneralMeshStatePtr)

iGeneralMeshState_scfGetVersion = _cspace.iGeneralMeshState_scfGetVersion

class iGeneralFactoryState(iGeneralMeshCommonState):
    __swig_setmethods__ = {}
    for _s in [iGeneralMeshCommonState]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGeneralFactoryState, name, value)
    __swig_getmethods__ = {}
    for _s in [iGeneralMeshCommonState]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGeneralFactoryState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGeneralFactoryState instance at %s>" % (self.this,)
    def SetVertexCount(*args): return _cspace.iGeneralFactoryState_SetVertexCount(*args)
    def GetVertexCount(*args): return _cspace.iGeneralFactoryState_GetVertexCount(*args)
    def GetVertices(self):
      return CSMutableArrayHelper(self.GetVertexByIndex, self.GetVertexCount)


    def GetTexels(self):
      return CSMutableArrayHelper(self.GetTexelByIndex, self.GetVertexCount)


    def GetNormals(self):
      # iGeneralFactoryState::GetVertices()
      return CSMutableArrayHelper(self.GetNormalByIndex, self.GetVertexCount)


    def SetTriangleCount(*args): return _cspace.iGeneralFactoryState_SetTriangleCount(*args)
    def GetTriangleCount(*args): return _cspace.iGeneralFactoryState_GetTriangleCount(*args)
    def GetTriangles(self):
      return CSMutableArrayHelper(self.GetTriangleByIndex, self.GetTriangleCount)


    def GetColors(self):
      return CSMutableArrayHelper(self.GetNormalByIndex, self.GetVertexCount)


    def Invalidate(*args): return _cspace.iGeneralFactoryState_Invalidate(*args)
    def CalculateNormals(*args): return _cspace.iGeneralFactoryState_CalculateNormals(*args)
    def GenerateBox(*args): return _cspace.iGeneralFactoryState_GenerateBox(*args)
    def SetBack2Front(*args): return _cspace.iGeneralFactoryState_SetBack2Front(*args)
    def IsAutoNormals(*args): return _cspace.iGeneralFactoryState_IsAutoNormals(*args)
    def IsBack2Front(*args): return _cspace.iGeneralFactoryState_IsBack2Front(*args)
    def SetAnimationControlFactory(*args): return _cspace.iGeneralFactoryState_SetAnimationControlFactory(*args)
    def GetAnimationControlFactory(*args): return _cspace.iGeneralFactoryState_GetAnimationControlFactory(*args)
    def __del__(self, destroy=_cspace.delete_iGeneralFactoryState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iGeneralFactoryState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iGeneralFactoryState_scfGetVersion)
    def GetVertexByIndex(*args): return _cspace.iGeneralFactoryState_GetVertexByIndex(*args)
    def GetTexelByIndex(*args): return _cspace.iGeneralFactoryState_GetTexelByIndex(*args)
    def GetNormalByIndex(*args): return _cspace.iGeneralFactoryState_GetNormalByIndex(*args)
    def GetTriangleByIndex(*args): return _cspace.iGeneralFactoryState_GetTriangleByIndex(*args)
    def GetColorByIndex(*args): return _cspace.iGeneralFactoryState_GetColorByIndex(*args)

class iGeneralFactoryStatePtr(iGeneralFactoryState):
    def __init__(self, this):
        _swig_setattr(self, iGeneralFactoryState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGeneralFactoryState, 'thisown', 0)
        _swig_setattr(self, iGeneralFactoryState,self.__class__,iGeneralFactoryState)
_cspace.iGeneralFactoryState_swigregister(iGeneralFactoryStatePtr)

iGeneralFactoryState_scfGetVersion = _cspace.iGeneralFactoryState_scfGetVersion

class iGenMeshAnimationControl(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGenMeshAnimationControl, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGenMeshAnimationControl, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGenMeshAnimationControl instance at %s>" % (self.this,)
    def AnimatesVertices(*args): return _cspace.iGenMeshAnimationControl_AnimatesVertices(*args)
    def AnimatesTexels(*args): return _cspace.iGenMeshAnimationControl_AnimatesTexels(*args)
    def AnimatesNormals(*args): return _cspace.iGenMeshAnimationControl_AnimatesNormals(*args)
    def AnimatesColors(*args): return _cspace.iGenMeshAnimationControl_AnimatesColors(*args)
    def UpdateVertices(*args): return _cspace.iGenMeshAnimationControl_UpdateVertices(*args)
    def UpdateTexels(*args): return _cspace.iGenMeshAnimationControl_UpdateTexels(*args)
    def UpdateNormals(*args): return _cspace.iGenMeshAnimationControl_UpdateNormals(*args)
    def UpdateColors(*args): return _cspace.iGenMeshAnimationControl_UpdateColors(*args)
    def __del__(self, destroy=_cspace.delete_iGenMeshAnimationControl):
        try:
            if self.thisown: destroy(self)
        except: pass

class iGenMeshAnimationControlPtr(iGenMeshAnimationControl):
    def __init__(self, this):
        _swig_setattr(self, iGenMeshAnimationControl, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGenMeshAnimationControl, 'thisown', 0)
        _swig_setattr(self, iGenMeshAnimationControl,self.__class__,iGenMeshAnimationControl)
_cspace.iGenMeshAnimationControl_swigregister(iGenMeshAnimationControlPtr)

class iGenMeshAnimationControlFactory(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGenMeshAnimationControlFactory, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGenMeshAnimationControlFactory, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGenMeshAnimationControlFactory instance at %s>" % (self.this,)
    def CreateAnimationControl(*args): return _cspace.iGenMeshAnimationControlFactory_CreateAnimationControl(*args)
    def Load(*args): return _cspace.iGenMeshAnimationControlFactory_Load(*args)
    def Save(*args): return _cspace.iGenMeshAnimationControlFactory_Save(*args)
    def __del__(self, destroy=_cspace.delete_iGenMeshAnimationControlFactory):
        try:
            if self.thisown: destroy(self)
        except: pass

class iGenMeshAnimationControlFactoryPtr(iGenMeshAnimationControlFactory):
    def __init__(self, this):
        _swig_setattr(self, iGenMeshAnimationControlFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGenMeshAnimationControlFactory, 'thisown', 0)
        _swig_setattr(self, iGenMeshAnimationControlFactory,self.__class__,iGenMeshAnimationControlFactory)
_cspace.iGenMeshAnimationControlFactory_swigregister(iGenMeshAnimationControlFactoryPtr)

class iGenMeshAnimationControlType(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGenMeshAnimationControlType, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGenMeshAnimationControlType, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGenMeshAnimationControlType instance at %s>" % (self.this,)
    def CreateAnimationControlFactory(*args): return _cspace.iGenMeshAnimationControlType_CreateAnimationControlFactory(*args)
    def __del__(self, destroy=_cspace.delete_iGenMeshAnimationControlType):
        try:
            if self.thisown: destroy(self)
        except: pass

class iGenMeshAnimationControlTypePtr(iGenMeshAnimationControlType):
    def __init__(self, this):
        _swig_setattr(self, iGenMeshAnimationControlType, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGenMeshAnimationControlType, 'thisown', 0)
        _swig_setattr(self, iGenMeshAnimationControlType,self.__class__,iGenMeshAnimationControlType)
_cspace.iGenMeshAnimationControlType_swigregister(iGenMeshAnimationControlTypePtr)

class csSprite2DVertex(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSprite2DVertex, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSprite2DVertex, name)
    def __repr__(self):
        return "<C csSprite2DVertex instance at %s>" % (self.this,)
    __swig_setmethods__["pos"] = _cspace.csSprite2DVertex_pos_set
    __swig_getmethods__["pos"] = _cspace.csSprite2DVertex_pos_get
    if _newclass:pos = property(_cspace.csSprite2DVertex_pos_get, _cspace.csSprite2DVertex_pos_set)
    __swig_setmethods__["color_init"] = _cspace.csSprite2DVertex_color_init_set
    __swig_getmethods__["color_init"] = _cspace.csSprite2DVertex_color_init_get
    if _newclass:color_init = property(_cspace.csSprite2DVertex_color_init_get, _cspace.csSprite2DVertex_color_init_set)
    __swig_setmethods__["color"] = _cspace.csSprite2DVertex_color_set
    __swig_getmethods__["color"] = _cspace.csSprite2DVertex_color_get
    if _newclass:color = property(_cspace.csSprite2DVertex_color_get, _cspace.csSprite2DVertex_color_set)
    __swig_setmethods__["u"] = _cspace.csSprite2DVertex_u_set
    __swig_getmethods__["u"] = _cspace.csSprite2DVertex_u_get
    if _newclass:u = property(_cspace.csSprite2DVertex_u_get, _cspace.csSprite2DVertex_u_set)
    __swig_setmethods__["v"] = _cspace.csSprite2DVertex_v_set
    __swig_getmethods__["v"] = _cspace.csSprite2DVertex_v_get
    if _newclass:v = property(_cspace.csSprite2DVertex_v_get, _cspace.csSprite2DVertex_v_set)
    def __init__(self, *args):
        _swig_setattr(self, csSprite2DVertex, 'this', _cspace.new_csSprite2DVertex(*args))
        _swig_setattr(self, csSprite2DVertex, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csSprite2DVertex):
        try:
            if self.thisown: destroy(self)
        except: pass

class csSprite2DVertexPtr(csSprite2DVertex):
    def __init__(self, this):
        _swig_setattr(self, csSprite2DVertex, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSprite2DVertex, 'thisown', 0)
        _swig_setattr(self, csSprite2DVertex,self.__class__,csSprite2DVertex)
_cspace.csSprite2DVertex_swigregister(csSprite2DVertexPtr)

class iSprite2DUVAnimationFrame(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSprite2DUVAnimationFrame, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSprite2DUVAnimationFrame, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSprite2DUVAnimationFrame instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iSprite2DUVAnimationFrame_SetName(*args)
    def GetName(*args): return _cspace.iSprite2DUVAnimationFrame_GetName(*args)
    def GetUVCoo(*args): return _cspace.iSprite2DUVAnimationFrame_GetUVCoo(*args)
    def GetUVCount(*args): return _cspace.iSprite2DUVAnimationFrame_GetUVCount(*args)
    def SetUV(*args): return _cspace.iSprite2DUVAnimationFrame_SetUV(*args)
    def SetFrameData(*args): return _cspace.iSprite2DUVAnimationFrame_SetFrameData(*args)
    def RemoveUV(*args): return _cspace.iSprite2DUVAnimationFrame_RemoveUV(*args)
    def GetDuration(*args): return _cspace.iSprite2DUVAnimationFrame_GetDuration(*args)
    def SetDuration(*args): return _cspace.iSprite2DUVAnimationFrame_SetDuration(*args)
    def __del__(self, destroy=_cspace.delete_iSprite2DUVAnimationFrame):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSprite2DUVAnimationFramePtr(iSprite2DUVAnimationFrame):
    def __init__(self, this):
        _swig_setattr(self, iSprite2DUVAnimationFrame, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSprite2DUVAnimationFrame, 'thisown', 0)
        _swig_setattr(self, iSprite2DUVAnimationFrame,self.__class__,iSprite2DUVAnimationFrame)
_cspace.iSprite2DUVAnimationFrame_swigregister(iSprite2DUVAnimationFramePtr)

class iSprite2DUVAnimation(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSprite2DUVAnimation, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSprite2DUVAnimation, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSprite2DUVAnimation instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iSprite2DUVAnimation_SetName(*args)
    def GetName(*args): return _cspace.iSprite2DUVAnimation_GetName(*args)
    def GetFrameCount(*args): return _cspace.iSprite2DUVAnimation_GetFrameCount(*args)
    def GetFrame(*args): return _cspace.iSprite2DUVAnimation_GetFrame(*args)
    def CreateFrame(*args): return _cspace.iSprite2DUVAnimation_CreateFrame(*args)
    def MoveFrame(*args): return _cspace.iSprite2DUVAnimation_MoveFrame(*args)
    def RemoveFrame(*args): return _cspace.iSprite2DUVAnimation_RemoveFrame(*args)
    def __del__(self, destroy=_cspace.delete_iSprite2DUVAnimation):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSprite2DUVAnimationPtr(iSprite2DUVAnimation):
    def __init__(self, this):
        _swig_setattr(self, iSprite2DUVAnimation, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSprite2DUVAnimation, 'thisown', 0)
        _swig_setattr(self, iSprite2DUVAnimation,self.__class__,iSprite2DUVAnimation)
_cspace.iSprite2DUVAnimation_swigregister(iSprite2DUVAnimationPtr)

class iSprite2DFactoryState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSprite2DFactoryState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSprite2DFactoryState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSprite2DFactoryState instance at %s>" % (self.this,)
    def SetMaterialWrapper(*args): return _cspace.iSprite2DFactoryState_SetMaterialWrapper(*args)
    def GetMaterialWrapper(*args): return _cspace.iSprite2DFactoryState_GetMaterialWrapper(*args)
    def SetMixMode(*args): return _cspace.iSprite2DFactoryState_SetMixMode(*args)
    def GetMixMode(*args): return _cspace.iSprite2DFactoryState_GetMixMode(*args)
    def SetLighting(*args): return _cspace.iSprite2DFactoryState_SetLighting(*args)
    def HasLighting(*args): return _cspace.iSprite2DFactoryState_HasLighting(*args)
    def GetUVAnimationCount(*args): return _cspace.iSprite2DFactoryState_GetUVAnimationCount(*args)
    def CreateUVAnimation(*args): return _cspace.iSprite2DFactoryState_CreateUVAnimation(*args)
    def RemoveUVAnimation(*args): return _cspace.iSprite2DFactoryState_RemoveUVAnimation(*args)
    def GetUVAnimation(*args): return _cspace.iSprite2DFactoryState_GetUVAnimation(*args)
    def __del__(self, destroy=_cspace.delete_iSprite2DFactoryState):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSprite2DFactoryStatePtr(iSprite2DFactoryState):
    def __init__(self, this):
        _swig_setattr(self, iSprite2DFactoryState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSprite2DFactoryState, 'thisown', 0)
        _swig_setattr(self, iSprite2DFactoryState,self.__class__,iSprite2DFactoryState)
_cspace.iSprite2DFactoryState_swigregister(iSprite2DFactoryStatePtr)

class iSprite2DState(iSprite2DFactoryState):
    __swig_setmethods__ = {}
    for _s in [iSprite2DFactoryState]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSprite2DState, name, value)
    __swig_getmethods__ = {}
    for _s in [iSprite2DFactoryState]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSprite2DState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSprite2DState instance at %s>" % (self.this,)
    def CreateRegularVertices(*args): return _cspace.iSprite2DState_CreateRegularVertices(*args)
    def SetUVAnimation(*args): return _cspace.iSprite2DState_SetUVAnimation(*args)
    def GetUVAnimation(*args): return _cspace.iSprite2DState_GetUVAnimation(*args)
    def StopUVAnimation(*args): return _cspace.iSprite2DState_StopUVAnimation(*args)
    def PlayUVAnimation(*args): return _cspace.iSprite2DState_PlayUVAnimation(*args)
    def __del__(self, destroy=_cspace.delete_iSprite2DState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSprite2DState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSprite2DState_scfGetVersion)
    def GetVertexByIndex(*args): return _cspace.iSprite2DState_GetVertexByIndex(*args)
    def GetVertexCount(*args): return _cspace.iSprite2DState_GetVertexCount(*args)

class iSprite2DStatePtr(iSprite2DState):
    def __init__(self, this):
        _swig_setattr(self, iSprite2DState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSprite2DState, 'thisown', 0)
        _swig_setattr(self, iSprite2DState,self.__class__,iSprite2DState)
_cspace.iSprite2DState_swigregister(iSprite2DStatePtr)

iSprite2DState_scfGetVersion = _cspace.iSprite2DState_scfGetVersion

CS_SPR_LIGHTING_HQ = _cspace.CS_SPR_LIGHTING_HQ
CS_SPR_LIGHTING_LQ = _cspace.CS_SPR_LIGHTING_LQ
CS_SPR_LIGHTING_FAST = _cspace.CS_SPR_LIGHTING_FAST
CS_SPR_LIGHTING_RANDOM = _cspace.CS_SPR_LIGHTING_RANDOM
CS_SPR_LIGHT_GLOBAL = _cspace.CS_SPR_LIGHT_GLOBAL
CS_SPR_LIGHT_TEMPLATE = _cspace.CS_SPR_LIGHT_TEMPLATE
CS_SPR_LIGHT_LOCAL = _cspace.CS_SPR_LIGHT_LOCAL
CS_SPR_LOD_GLOBAL = _cspace.CS_SPR_LOD_GLOBAL
CS_SPR_LOD_TEMPLATE = _cspace.CS_SPR_LOD_TEMPLATE
CS_SPR_LOD_LOCAL = _cspace.CS_SPR_LOD_LOCAL
class iSpriteFrame(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSpriteFrame, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSpriteFrame, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSpriteFrame instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iSpriteFrame_SetName(*args)
    def GetName(*args): return _cspace.iSpriteFrame_GetName(*args)
    def GetAnmIndex(*args): return _cspace.iSpriteFrame_GetAnmIndex(*args)
    def GetTexIndex(*args): return _cspace.iSpriteFrame_GetTexIndex(*args)
    def __del__(self, destroy=_cspace.delete_iSpriteFrame):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSpriteFramePtr(iSpriteFrame):
    def __init__(self, this):
        _swig_setattr(self, iSpriteFrame, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSpriteFrame, 'thisown', 0)
        _swig_setattr(self, iSpriteFrame,self.__class__,iSpriteFrame)
_cspace.iSpriteFrame_swigregister(iSpriteFramePtr)

class iSpriteAction(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSpriteAction, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSpriteAction, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSpriteAction instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iSpriteAction_SetName(*args)
    def GetName(*args): return _cspace.iSpriteAction_GetName(*args)
    def GetFrameCount(*args): return _cspace.iSpriteAction_GetFrameCount(*args)
    def GetFrame(*args): return _cspace.iSpriteAction_GetFrame(*args)
    def GetNextFrame(*args): return _cspace.iSpriteAction_GetNextFrame(*args)
    def GetFrameDelay(*args): return _cspace.iSpriteAction_GetFrameDelay(*args)
    def GetFrameDisplacement(*args): return _cspace.iSpriteAction_GetFrameDisplacement(*args)
    def AddFrame(*args): return _cspace.iSpriteAction_AddFrame(*args)
    def __del__(self, destroy=_cspace.delete_iSpriteAction):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSpriteActionPtr(iSpriteAction):
    def __init__(self, this):
        _swig_setattr(self, iSpriteAction, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSpriteAction, 'thisown', 0)
        _swig_setattr(self, iSpriteAction,self.__class__,iSpriteAction)
_cspace.iSpriteAction_swigregister(iSpriteActionPtr)

class iSpriteSocket(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSpriteSocket, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSpriteSocket, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSpriteSocket instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iSpriteSocket_SetName(*args)
    def GetName(*args): return _cspace.iSpriteSocket_GetName(*args)
    def SetMeshWrapper(*args): return _cspace.iSpriteSocket_SetMeshWrapper(*args)
    def GetMeshWrapper(*args): return _cspace.iSpriteSocket_GetMeshWrapper(*args)
    def SetTriangleIndex(*args): return _cspace.iSpriteSocket_SetTriangleIndex(*args)
    def GetTriangleIndex(*args): return _cspace.iSpriteSocket_GetTriangleIndex(*args)
    def __del__(self, destroy=_cspace.delete_iSpriteSocket):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSpriteSocketPtr(iSpriteSocket):
    def __init__(self, this):
        _swig_setattr(self, iSpriteSocket, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSpriteSocket, 'thisown', 0)
        _swig_setattr(self, iSpriteSocket,self.__class__,iSpriteSocket)
_cspace.iSpriteSocket_swigregister(iSpriteSocketPtr)

class iSprite3DFactoryState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSprite3DFactoryState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSprite3DFactoryState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSprite3DFactoryState instance at %s>" % (self.this,)
    def SetMaterialWrapper(*args): return _cspace.iSprite3DFactoryState_SetMaterialWrapper(*args)
    def GetMaterialWrapper(*args): return _cspace.iSprite3DFactoryState_GetMaterialWrapper(*args)
    def AddVertices(*args): return _cspace.iSprite3DFactoryState_AddVertices(*args)
    def GetVertexCount(*args): return _cspace.iSprite3DFactoryState_GetVertexCount(*args)
    def GetVertex(*args): return _cspace.iSprite3DFactoryState_GetVertex(*args)
    def SetVertex(*args): return _cspace.iSprite3DFactoryState_SetVertex(*args)
    def GetVertices(*args): return _cspace.iSprite3DFactoryState_GetVertices(*args)
    def SetVertices(*args): return _cspace.iSprite3DFactoryState_SetVertices(*args)
    def GetTexel(*args): return _cspace.iSprite3DFactoryState_GetTexel(*args)
    def SetTexel(*args): return _cspace.iSprite3DFactoryState_SetTexel(*args)
    def GetTexels(*args): return _cspace.iSprite3DFactoryState_GetTexels(*args)
    def SetTexels(*args): return _cspace.iSprite3DFactoryState_SetTexels(*args)
    def GetNormal(*args): return _cspace.iSprite3DFactoryState_GetNormal(*args)
    def SetNormal(*args): return _cspace.iSprite3DFactoryState_SetNormal(*args)
    def GetNormals(*args): return _cspace.iSprite3DFactoryState_GetNormals(*args)
    def SetNormals(*args): return _cspace.iSprite3DFactoryState_SetNormals(*args)
    def AddTriangle(*args): return _cspace.iSprite3DFactoryState_AddTriangle(*args)
    def GetTriangle(*args): return _cspace.iSprite3DFactoryState_GetTriangle(*args)
    def GetTriangles(*args): return _cspace.iSprite3DFactoryState_GetTriangles(*args)
    def GetTriangleCount(*args): return _cspace.iSprite3DFactoryState_GetTriangleCount(*args)
    def SetTriangleCount(*args): return _cspace.iSprite3DFactoryState_SetTriangleCount(*args)
    def SetTriangles(*args): return _cspace.iSprite3DFactoryState_SetTriangles(*args)
    def AddFrame(*args): return _cspace.iSprite3DFactoryState_AddFrame(*args)
    def FindFrame(*args): return _cspace.iSprite3DFactoryState_FindFrame(*args)
    def GetFrameCount(*args): return _cspace.iSprite3DFactoryState_GetFrameCount(*args)
    def GetFrame(*args): return _cspace.iSprite3DFactoryState_GetFrame(*args)
    def AddAction(*args): return _cspace.iSprite3DFactoryState_AddAction(*args)
    def FindAction(*args): return _cspace.iSprite3DFactoryState_FindAction(*args)
    def GetFirstAction(*args): return _cspace.iSprite3DFactoryState_GetFirstAction(*args)
    def GetActionCount(*args): return _cspace.iSprite3DFactoryState_GetActionCount(*args)
    def GetAction(*args): return _cspace.iSprite3DFactoryState_GetAction(*args)
    def AddSocket(*args): return _cspace.iSprite3DFactoryState_AddSocket(*args)
    def FindSocket(*args): return _cspace.iSprite3DFactoryState_FindSocket(*args)
    def GetSocketCount(*args): return _cspace.iSprite3DFactoryState_GetSocketCount(*args)
    def GetSocket(*args): return _cspace.iSprite3DFactoryState_GetSocket(*args)
    def EnableTweening(*args): return _cspace.iSprite3DFactoryState_EnableTweening(*args)
    def IsTweeningEnabled(*args): return _cspace.iSprite3DFactoryState_IsTweeningEnabled(*args)
    def SetLightingQuality(*args): return _cspace.iSprite3DFactoryState_SetLightingQuality(*args)
    def GetLightingQuality(*args): return _cspace.iSprite3DFactoryState_GetLightingQuality(*args)
    def SetLightingQualityConfig(*args): return _cspace.iSprite3DFactoryState_SetLightingQualityConfig(*args)
    def GetLightingQualityConfig(*args): return _cspace.iSprite3DFactoryState_GetLightingQualityConfig(*args)
    def SetLodLevelConfig(*args): return _cspace.iSprite3DFactoryState_SetLodLevelConfig(*args)
    def GetLodLevelConfig(*args): return _cspace.iSprite3DFactoryState_GetLodLevelConfig(*args)
    def MergeNormals(*args): return _cspace.iSprite3DFactoryState_MergeNormals(*args)
    def SetMixMode(*args): return _cspace.iSprite3DFactoryState_SetMixMode(*args)
    def GetMixMode(*args): return _cspace.iSprite3DFactoryState_GetMixMode(*args)
    def __del__(self, destroy=_cspace.delete_iSprite3DFactoryState):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSprite3DFactoryStatePtr(iSprite3DFactoryState):
    def __init__(self, this):
        _swig_setattr(self, iSprite3DFactoryState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSprite3DFactoryState, 'thisown', 0)
        _swig_setattr(self, iSprite3DFactoryState,self.__class__,iSprite3DFactoryState)
_cspace.iSprite3DFactoryState_swigregister(iSprite3DFactoryStatePtr)

class iSprite3DState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSprite3DState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSprite3DState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSprite3DState instance at %s>" % (self.this,)
    def SetMaterialWrapper(*args): return _cspace.iSprite3DState_SetMaterialWrapper(*args)
    def GetMaterialWrapper(*args): return _cspace.iSprite3DState_GetMaterialWrapper(*args)
    def SetMixMode(*args): return _cspace.iSprite3DState_SetMixMode(*args)
    def GetMixMode(*args): return _cspace.iSprite3DState_GetMixMode(*args)
    def SetLighting(*args): return _cspace.iSprite3DState_SetLighting(*args)
    def IsLighting(*args): return _cspace.iSprite3DState_IsLighting(*args)
    def SetFrame(*args): return _cspace.iSprite3DState_SetFrame(*args)
    def GetCurFrame(*args): return _cspace.iSprite3DState_GetCurFrame(*args)
    def GetFrameCount(*args): return _cspace.iSprite3DState_GetFrameCount(*args)
    def SetAction(*args): return _cspace.iSprite3DState_SetAction(*args)
    def SetReverseAction(*args): return _cspace.iSprite3DState_SetReverseAction(*args)
    def SetSingleStepAction(*args): return _cspace.iSprite3DState_SetSingleStepAction(*args)
    def SetOverrideAction(*args): return _cspace.iSprite3DState_SetOverrideAction(*args)
    def PropagateAction(*args): return _cspace.iSprite3DState_PropagateAction(*args)
    def GetCurAction(*args): return _cspace.iSprite3DState_GetCurAction(*args)
    def GetReverseAction(*args): return _cspace.iSprite3DState_GetReverseAction(*args)
    def EnableTweening(*args): return _cspace.iSprite3DState_EnableTweening(*args)
    def IsTweeningEnabled(*args): return _cspace.iSprite3DState_IsTweeningEnabled(*args)
    def UnsetTexture(*args): return _cspace.iSprite3DState_UnsetTexture(*args)
    def GetLightingQuality(*args): return _cspace.iSprite3DState_GetLightingQuality(*args)
    def SetLocalLightingQuality(*args): return _cspace.iSprite3DState_SetLocalLightingQuality(*args)
    def SetLightingQualityConfig(*args): return _cspace.iSprite3DState_SetLightingQualityConfig(*args)
    def GetLightingQualityConfig(*args): return _cspace.iSprite3DState_GetLightingQualityConfig(*args)
    def SetLodLevelConfig(*args): return _cspace.iSprite3DState_SetLodLevelConfig(*args)
    def GetLodLevelConfig(*args): return _cspace.iSprite3DState_GetLodLevelConfig(*args)
    def IsLodEnabled(*args): return _cspace.iSprite3DState_IsLodEnabled(*args)
    def SetBaseColor(*args): return _cspace.iSprite3DState_SetBaseColor(*args)
    def GetBaseColor(*args): return _cspace.iSprite3DState_GetBaseColor(*args)
    def FindSocket(*args): return _cspace.iSprite3DState_FindSocket(*args)
    def __del__(self, destroy=_cspace.delete_iSprite3DState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSprite3DState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSprite3DState_scfGetVersion)

class iSprite3DStatePtr(iSprite3DState):
    def __init__(self, this):
        _swig_setattr(self, iSprite3DState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSprite3DState, 'thisown', 0)
        _swig_setattr(self, iSprite3DState,self.__class__,iSprite3DState)
_cspace.iSprite3DState_swigregister(iSprite3DStatePtr)

iSprite3DState_scfGetVersion = _cspace.iSprite3DState_scfGetVersion

class iSpriteCal3DSocket(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSpriteCal3DSocket, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSpriteCal3DSocket, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSpriteCal3DSocket instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iSpriteCal3DSocket_SetName(*args)
    def GetName(*args): return _cspace.iSpriteCal3DSocket_GetName(*args)
    def SetMeshWrapper(*args): return _cspace.iSpriteCal3DSocket_SetMeshWrapper(*args)
    def GetMeshWrapper(*args): return _cspace.iSpriteCal3DSocket_GetMeshWrapper(*args)
    def SetTriangleIndex(*args): return _cspace.iSpriteCal3DSocket_SetTriangleIndex(*args)
    def GetTriangleIndex(*args): return _cspace.iSpriteCal3DSocket_GetTriangleIndex(*args)
    def SetSubmeshIndex(*args): return _cspace.iSpriteCal3DSocket_SetSubmeshIndex(*args)
    def GetSubmeshIndex(*args): return _cspace.iSpriteCal3DSocket_GetSubmeshIndex(*args)
    def SetMeshIndex(*args): return _cspace.iSpriteCal3DSocket_SetMeshIndex(*args)
    def GetMeshIndex(*args): return _cspace.iSpriteCal3DSocket_GetMeshIndex(*args)
    def SetTransform(*args): return _cspace.iSpriteCal3DSocket_SetTransform(*args)
    def GetTransform(*args): return _cspace.iSpriteCal3DSocket_GetTransform(*args)
    def GetSecondaryCount(*args): return _cspace.iSpriteCal3DSocket_GetSecondaryCount(*args)
    def GetSecondaryMesh(*args): return _cspace.iSpriteCal3DSocket_GetSecondaryMesh(*args)
    def GetSecondaryTransform(*args): return _cspace.iSpriteCal3DSocket_GetSecondaryTransform(*args)
    def SetSecondaryTransform(*args): return _cspace.iSpriteCal3DSocket_SetSecondaryTransform(*args)
    def AttachSecondary(*args): return _cspace.iSpriteCal3DSocket_AttachSecondary(*args)
    def DetachSecondary(*args): return _cspace.iSpriteCal3DSocket_DetachSecondary(*args)
    def FindSecondary(*args): return _cspace.iSpriteCal3DSocket_FindSecondary(*args)
    def __del__(self, destroy=_cspace.delete_iSpriteCal3DSocket):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSpriteCal3DSocketPtr(iSpriteCal3DSocket):
    def __init__(self, this):
        _swig_setattr(self, iSpriteCal3DSocket, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSpriteCal3DSocket, 'thisown', 0)
        _swig_setattr(self, iSpriteCal3DSocket,self.__class__,iSpriteCal3DSocket)
_cspace.iSpriteCal3DSocket_swigregister(iSpriteCal3DSocketPtr)

class iSpriteCal3DFactoryState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSpriteCal3DFactoryState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSpriteCal3DFactoryState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSpriteCal3DFactoryState instance at %s>" % (self.this,)
    def Create(*args): return _cspace.iSpriteCal3DFactoryState_Create(*args)
    def ReportLastError(*args): return _cspace.iSpriteCal3DFactoryState_ReportLastError(*args)
    def SetLoadFlags(*args): return _cspace.iSpriteCal3DFactoryState_SetLoadFlags(*args)
    def SetBasePath(*args): return _cspace.iSpriteCal3DFactoryState_SetBasePath(*args)
    def LoadCoreSkeleton(*args): return _cspace.iSpriteCal3DFactoryState_LoadCoreSkeleton(*args)
    def RescaleFactory(*args): return _cspace.iSpriteCal3DFactoryState_RescaleFactory(*args)
    def LoadCoreAnimation(*args): return _cspace.iSpriteCal3DFactoryState_LoadCoreAnimation(*args)
    def LoadCoreMesh(*args): return _cspace.iSpriteCal3DFactoryState_LoadCoreMesh(*args)
    def LoadCoreMorphTarget(*args): return _cspace.iSpriteCal3DFactoryState_LoadCoreMorphTarget(*args)
    def AddMorphAnimation(*args): return _cspace.iSpriteCal3DFactoryState_AddMorphAnimation(*args)
    def AddMorphTarget(*args): return _cspace.iSpriteCal3DFactoryState_AddMorphTarget(*args)
    def AddCoreMaterial(*args): return _cspace.iSpriteCal3DFactoryState_AddCoreMaterial(*args)
    def CalculateAllBoneBoundingBoxes(*args): return _cspace.iSpriteCal3DFactoryState_CalculateAllBoneBoundingBoxes(*args)
    def BindMaterials(*args): return _cspace.iSpriteCal3DFactoryState_BindMaterials(*args)
    def GetMeshCount(*args): return _cspace.iSpriteCal3DFactoryState_GetMeshCount(*args)
    def GetMorphAnimationCount(*args): return _cspace.iSpriteCal3DFactoryState_GetMorphAnimationCount(*args)
    def GetMorphTargetCount(*args): return _cspace.iSpriteCal3DFactoryState_GetMorphTargetCount(*args)
    def GetMeshName(*args): return _cspace.iSpriteCal3DFactoryState_GetMeshName(*args)
    def FindMeshName(*args): return _cspace.iSpriteCal3DFactoryState_FindMeshName(*args)
    def GetDefaultMaterial(*args): return _cspace.iSpriteCal3DFactoryState_GetDefaultMaterial(*args)
    def GetMorphAnimationName(*args): return _cspace.iSpriteCal3DFactoryState_GetMorphAnimationName(*args)
    def FindMorphAnimationName(*args): return _cspace.iSpriteCal3DFactoryState_FindMorphAnimationName(*args)
    def IsMeshDefault(*args): return _cspace.iSpriteCal3DFactoryState_IsMeshDefault(*args)
    def AddSocket(*args): return _cspace.iSpriteCal3DFactoryState_AddSocket(*args)
    def FindSocket(*args): return _cspace.iSpriteCal3DFactoryState_FindSocket(*args)
    def GetSocketCount(*args): return _cspace.iSpriteCal3DFactoryState_GetSocketCount(*args)
    def GetSocket(*args): return _cspace.iSpriteCal3DFactoryState_GetSocket(*args)
    def GetCal3DCoreModel(*args): return _cspace.iSpriteCal3DFactoryState_GetCal3DCoreModel(*args)
    def RegisterAnimCallback(*args): return _cspace.iSpriteCal3DFactoryState_RegisterAnimCallback(*args)
    def RemoveAnimCallback(*args): return _cspace.iSpriteCal3DFactoryState_RemoveAnimCallback(*args)
    def __del__(self, destroy=_cspace.delete_iSpriteCal3DFactoryState):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSpriteCal3DFactoryStatePtr(iSpriteCal3DFactoryState):
    def __init__(self, this):
        _swig_setattr(self, iSpriteCal3DFactoryState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSpriteCal3DFactoryState, 'thisown', 0)
        _swig_setattr(self, iSpriteCal3DFactoryState,self.__class__,iSpriteCal3DFactoryState)
_cspace.iSpriteCal3DFactoryState_swigregister(iSpriteCal3DFactoryStatePtr)

class iAnimTimeUpdateHandler(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAnimTimeUpdateHandler, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAnimTimeUpdateHandler, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAnimTimeUpdateHandler instance at %s>" % (self.this,)
    def UpdatePosition(*args): return _cspace.iAnimTimeUpdateHandler_UpdatePosition(*args)
    def __del__(self, destroy=_cspace.delete_iAnimTimeUpdateHandler):
        try:
            if self.thisown: destroy(self)
        except: pass

class iAnimTimeUpdateHandlerPtr(iAnimTimeUpdateHandler):
    def __init__(self, this):
        _swig_setattr(self, iAnimTimeUpdateHandler, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAnimTimeUpdateHandler, 'thisown', 0)
        _swig_setattr(self, iAnimTimeUpdateHandler,self.__class__,iAnimTimeUpdateHandler)
_cspace.iAnimTimeUpdateHandler_swigregister(iAnimTimeUpdateHandlerPtr)

class iSpriteCal3DState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSpriteCal3DState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSpriteCal3DState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSpriteCal3DState instance at %s>" % (self.this,)
    C3D_ANIM_TYPE_NONE = _cspace.iSpriteCal3DState_C3D_ANIM_TYPE_NONE
    C3D_ANIM_TYPE_IDLE = _cspace.iSpriteCal3DState_C3D_ANIM_TYPE_IDLE
    C3D_ANIM_TYPE_TRAVEL = _cspace.iSpriteCal3DState_C3D_ANIM_TYPE_TRAVEL
    C3D_ANIM_TYPE_CYCLE = _cspace.iSpriteCal3DState_C3D_ANIM_TYPE_CYCLE
    C3D_ANIM_TYPE_STYLE_CYCLE = _cspace.iSpriteCal3DState_C3D_ANIM_TYPE_STYLE_CYCLE
    C3D_ANIM_TYPE_ACTION = _cspace.iSpriteCal3DState_C3D_ANIM_TYPE_ACTION
    def GetAnimCount(*args): return _cspace.iSpriteCal3DState_GetAnimCount(*args)
    def GetAnimName(*args): return _cspace.iSpriteCal3DState_GetAnimName(*args)
    def GetAnimType(*args): return _cspace.iSpriteCal3DState_GetAnimType(*args)
    def FindAnim(*args): return _cspace.iSpriteCal3DState_FindAnim(*args)
    def ClearAllAnims(*args): return _cspace.iSpriteCal3DState_ClearAllAnims(*args)
    def SetAnimCycle(*args): return _cspace.iSpriteCal3DState_SetAnimCycle(*args)
    def AddAnimCycle(*args): return _cspace.iSpriteCal3DState_AddAnimCycle(*args)
    def ClearAnimCycle(*args): return _cspace.iSpriteCal3DState_ClearAnimCycle(*args)
    def GetActiveAnimCount(*args): return _cspace.iSpriteCal3DState_GetActiveAnimCount(*args)
    def GetActiveAnims(*args): return _cspace.iSpriteCal3DState_GetActiveAnims(*args)
    def SetActiveAnims(*args): return _cspace.iSpriteCal3DState_SetActiveAnims(*args)
    def SetAnimAction(*args): return _cspace.iSpriteCal3DState_SetAnimAction(*args)
    def SetVelocity(*args): return _cspace.iSpriteCal3DState_SetVelocity(*args)
    def SetDefaultIdleAnim(*args): return _cspace.iSpriteCal3DState_SetDefaultIdleAnim(*args)
    def SetLOD(*args): return _cspace.iSpriteCal3DState_SetLOD(*args)
    def AttachCoreMesh(*args): return _cspace.iSpriteCal3DState_AttachCoreMesh(*args)
    def DetachCoreMesh(*args): return _cspace.iSpriteCal3DState_DetachCoreMesh(*args)
    def BlendMorphTarget(*args): return _cspace.iSpriteCal3DState_BlendMorphTarget(*args)
    def ClearMorphTarget(*args): return _cspace.iSpriteCal3DState_ClearMorphTarget(*args)
    def FindSocket(*args): return _cspace.iSpriteCal3DState_FindSocket(*args)
    def SetMaterial(*args): return _cspace.iSpriteCal3DState_SetMaterial(*args)
    def SetTimeFactor(*args): return _cspace.iSpriteCal3DState_SetTimeFactor(*args)
    def GetTimeFactor(*args): return _cspace.iSpriteCal3DState_GetTimeFactor(*args)
    def GetAnimationTime(*args): return _cspace.iSpriteCal3DState_GetAnimationTime(*args)
    def GetAnimationDuration(*args): return _cspace.iSpriteCal3DState_GetAnimationDuration(*args)
    def SetAnimationTime(*args): return _cspace.iSpriteCal3DState_SetAnimationTime(*args)
    def GetCal3DModel(*args): return _cspace.iSpriteCal3DState_GetCal3DModel(*args)
    def SetUserData(*args): return _cspace.iSpriteCal3DState_SetUserData(*args)
    def SetAnimTimeUpdateHandler(*args): return _cspace.iSpriteCal3DState_SetAnimTimeUpdateHandler(*args)
    def __del__(self, destroy=_cspace.delete_iSpriteCal3DState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSpriteCal3DState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSpriteCal3DState_scfGetVersion)

class iSpriteCal3DStatePtr(iSpriteCal3DState):
    def __init__(self, this):
        _swig_setattr(self, iSpriteCal3DState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSpriteCal3DState, 'thisown', 0)
        _swig_setattr(self, iSpriteCal3DState,self.__class__,iSpriteCal3DState)
_cspace.iSpriteCal3DState_swigregister(iSpriteCal3DStatePtr)

iSpriteCal3DState_scfGetVersion = _cspace.iSpriteCal3DState_scfGetVersion

class csModelConverterFormat(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csModelConverterFormat, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csModelConverterFormat, name)
    def __repr__(self):
        return "<C csModelConverterFormat instance at %s>" % (self.this,)
    __swig_setmethods__["Name"] = _cspace.csModelConverterFormat_Name_set
    __swig_getmethods__["Name"] = _cspace.csModelConverterFormat_Name_get
    if _newclass:Name = property(_cspace.csModelConverterFormat_Name_get, _cspace.csModelConverterFormat_Name_set)
    __swig_setmethods__["CanLoad"] = _cspace.csModelConverterFormat_CanLoad_set
    __swig_getmethods__["CanLoad"] = _cspace.csModelConverterFormat_CanLoad_get
    if _newclass:CanLoad = property(_cspace.csModelConverterFormat_CanLoad_get, _cspace.csModelConverterFormat_CanLoad_set)
    __swig_setmethods__["CanSave"] = _cspace.csModelConverterFormat_CanSave_set
    __swig_getmethods__["CanSave"] = _cspace.csModelConverterFormat_CanSave_get
    if _newclass:CanSave = property(_cspace.csModelConverterFormat_CanSave_get, _cspace.csModelConverterFormat_CanSave_set)
    def __init__(self, *args):
        _swig_setattr(self, csModelConverterFormat, 'this', _cspace.new_csModelConverterFormat(*args))
        _swig_setattr(self, csModelConverterFormat, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csModelConverterFormat):
        try:
            if self.thisown: destroy(self)
        except: pass

class csModelConverterFormatPtr(csModelConverterFormat):
    def __init__(self, this):
        _swig_setattr(self, csModelConverterFormat, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csModelConverterFormat, 'thisown', 0)
        _swig_setattr(self, csModelConverterFormat,self.__class__,csModelConverterFormat)
_cspace.csModelConverterFormat_swigregister(csModelConverterFormatPtr)

class iModelConverter(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iModelConverter, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iModelConverter, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iModelConverter instance at %s>" % (self.this,)
    def GetFormatCount(*args): return _cspace.iModelConverter_GetFormatCount(*args)
    def GetFormat(*args): return _cspace.iModelConverter_GetFormat(*args)
    def Load(*args): return _cspace.iModelConverter_Load(*args)
    def Save(*args): return _cspace.iModelConverter_Save(*args)
    def __del__(self, destroy=_cspace.delete_iModelConverter):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iModelConverter_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iModelConverter_scfGetVersion)

class iModelConverterPtr(iModelConverter):
    def __init__(self, this):
        _swig_setattr(self, iModelConverter, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iModelConverter, 'thisown', 0)
        _swig_setattr(self, iModelConverter,self.__class__,iModelConverter)
_cspace.iModelConverter_swigregister(iModelConverterPtr)

iModelConverter_scfGetVersion = _cspace.iModelConverter_scfGetVersion

CS_MESH_STATICPOS = _cspace.CS_MESH_STATICPOS
CS_MESH_STATICSHAPE = _cspace.CS_MESH_STATICSHAPE
CS_FACTORY_STATICSHAPE = _cspace.CS_FACTORY_STATICSHAPE
class iMeshObjectDrawCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshObjectDrawCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshObjectDrawCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshObjectDrawCallback instance at %s>" % (self.this,)
    def BeforeDrawing(*args): return _cspace.iMeshObjectDrawCallback_BeforeDrawing(*args)
    def __del__(self, destroy=_cspace.delete_iMeshObjectDrawCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMeshObjectDrawCallbackPtr(iMeshObjectDrawCallback):
    def __init__(self, this):
        _swig_setattr(self, iMeshObjectDrawCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshObjectDrawCallback, 'thisown', 0)
        _swig_setattr(self, iMeshObjectDrawCallback,self.__class__,iMeshObjectDrawCallback)
_cspace.iMeshObjectDrawCallback_swigregister(iMeshObjectDrawCallbackPtr)

class iMeshObject(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshObject, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshObject, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshObject instance at %s>" % (self.this,)
    def GetFactory(*args): return _cspace.iMeshObject_GetFactory(*args)
    def GetFlags(*args): return _cspace.iMeshObject_GetFlags(*args)
    def Clone(*args): return _cspace.iMeshObject_Clone(*args)
    def GetRenderMeshes(*args): return _cspace.iMeshObject_GetRenderMeshes(*args)
    def SetVisibleCallback(*args): return _cspace.iMeshObject_SetVisibleCallback(*args)
    def GetVisibleCallback(*args): return _cspace.iMeshObject_GetVisibleCallback(*args)
    def NextFrame(*args): return _cspace.iMeshObject_NextFrame(*args)
    def HardTransform(*args): return _cspace.iMeshObject_HardTransform(*args)
    def SupportsHardTransform(*args): return _cspace.iMeshObject_SupportsHardTransform(*args)
    def HitBeamOutline(*args): return _cspace.iMeshObject_HitBeamOutline(*args)
    def HitBeamObject(*args): return _cspace.iMeshObject_HitBeamObject(*args)
    def SetLogicalParent(*args): return _cspace.iMeshObject_SetLogicalParent(*args)
    def GetLogicalParent(*args): return _cspace.iMeshObject_GetLogicalParent(*args)
    def GetObjectModel(*args): return _cspace.iMeshObject_GetObjectModel(*args)
    def SetColor(*args): return _cspace.iMeshObject_SetColor(*args)
    def GetColor(*args): return _cspace.iMeshObject_GetColor(*args)
    def SetMaterialWrapper(*args): return _cspace.iMeshObject_SetMaterialWrapper(*args)
    def GetMaterialWrapper(*args): return _cspace.iMeshObject_GetMaterialWrapper(*args)
    def InvalidateMaterialHandles(*args): return _cspace.iMeshObject_InvalidateMaterialHandles(*args)
    def PositionChild(*args): return _cspace.iMeshObject_PositionChild(*args)
    def __del__(self, destroy=_cspace.delete_iMeshObject):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMeshObject_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMeshObject_scfGetVersion)

class iMeshObjectPtr(iMeshObject):
    def __init__(self, this):
        _swig_setattr(self, iMeshObject, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshObject, 'thisown', 0)
        _swig_setattr(self, iMeshObject,self.__class__,iMeshObject)
_cspace.iMeshObject_swigregister(iMeshObjectPtr)

iMeshObject_scfGetVersion = _cspace.iMeshObject_scfGetVersion

class iMeshObjectFactory(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshObjectFactory, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshObjectFactory, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshObjectFactory instance at %s>" % (self.this,)
    def GetFlags(*args): return _cspace.iMeshObjectFactory_GetFlags(*args)
    def NewInstance(*args): return _cspace.iMeshObjectFactory_NewInstance(*args)
    def Clone(*args): return _cspace.iMeshObjectFactory_Clone(*args)
    def HardTransform(*args): return _cspace.iMeshObjectFactory_HardTransform(*args)
    def SupportsHardTransform(*args): return _cspace.iMeshObjectFactory_SupportsHardTransform(*args)
    def SetLogicalParent(*args): return _cspace.iMeshObjectFactory_SetLogicalParent(*args)
    def GetLogicalParent(*args): return _cspace.iMeshObjectFactory_GetLogicalParent(*args)
    def GetMeshObjectType(*args): return _cspace.iMeshObjectFactory_GetMeshObjectType(*args)
    def GetObjectModel(*args): return _cspace.iMeshObjectFactory_GetObjectModel(*args)
    def __del__(self, destroy=_cspace.delete_iMeshObjectFactory):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMeshObjectFactory_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMeshObjectFactory_scfGetVersion)

class iMeshObjectFactoryPtr(iMeshObjectFactory):
    def __init__(self, this):
        _swig_setattr(self, iMeshObjectFactory, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshObjectFactory, 'thisown', 0)
        _swig_setattr(self, iMeshObjectFactory,self.__class__,iMeshObjectFactory)
_cspace.iMeshObjectFactory_swigregister(iMeshObjectFactoryPtr)

iMeshObjectFactory_scfGetVersion = _cspace.iMeshObjectFactory_scfGetVersion

class iMeshObjectType(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMeshObjectType, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMeshObjectType, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMeshObjectType instance at %s>" % (self.this,)
    def NewFactory(*args): return _cspace.iMeshObjectType_NewFactory(*args)
    def __del__(self, destroy=_cspace.delete_iMeshObjectType):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMeshObjectType_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMeshObjectType_scfGetVersion)

class iMeshObjectTypePtr(iMeshObjectType):
    def __init__(self, this):
        _swig_setattr(self, iMeshObjectType, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMeshObjectType, 'thisown', 0)
        _swig_setattr(self, iMeshObjectType,self.__class__,iMeshObjectType)
_cspace.iMeshObjectType_swigregister(iMeshObjectTypePtr)

iMeshObjectType_scfGetVersion = _cspace.iMeshObjectType_scfGetVersion

class iBallState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iBallState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iBallState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iBallState instance at %s>" % (self.this,)
    def SetRadius(*args): return _cspace.iBallState_SetRadius(*args)
    def GetRadius(*args): return _cspace.iBallState_GetRadius(*args)
    def SetShift(*args): return _cspace.iBallState_SetShift(*args)
    def GetShift(*args): return _cspace.iBallState_GetShift(*args)
    def SetRimVertices(*args): return _cspace.iBallState_SetRimVertices(*args)
    def GetRimVertices(*args): return _cspace.iBallState_GetRimVertices(*args)
    def SetMaterialWrapper(*args): return _cspace.iBallState_SetMaterialWrapper(*args)
    def GetMaterialWrapper(*args): return _cspace.iBallState_GetMaterialWrapper(*args)
    def SetMixMode(*args): return _cspace.iBallState_SetMixMode(*args)
    def GetMixMode(*args): return _cspace.iBallState_GetMixMode(*args)
    def SetReversed(*args): return _cspace.iBallState_SetReversed(*args)
    def IsReversed(*args): return _cspace.iBallState_IsReversed(*args)
    def SetTopOnly(*args): return _cspace.iBallState_SetTopOnly(*args)
    def IsTopOnly(*args): return _cspace.iBallState_IsTopOnly(*args)
    def SetLighting(*args): return _cspace.iBallState_SetLighting(*args)
    def IsLighting(*args): return _cspace.iBallState_IsLighting(*args)
    def SetColor(*args): return _cspace.iBallState_SetColor(*args)
    def GetColor(*args): return _cspace.iBallState_GetColor(*args)
    def SetCylindricalMapping(*args): return _cspace.iBallState_SetCylindricalMapping(*args)
    def IsCylindricalMapping(*args): return _cspace.iBallState_IsCylindricalMapping(*args)
    def ApplyVertGradient(*args): return _cspace.iBallState_ApplyVertGradient(*args)
    def ApplyLightSpot(*args): return _cspace.iBallState_ApplyLightSpot(*args)
    def PaintSky(*args): return _cspace.iBallState_PaintSky(*args)
    def __del__(self, destroy=_cspace.delete_iBallState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iBallState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iBallState_scfGetVersion)

class iBallStatePtr(iBallState):
    def __init__(self, this):
        _swig_setattr(self, iBallState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iBallState, 'thisown', 0)
        _swig_setattr(self, iBallState,self.__class__,iBallState)
_cspace.iBallState_swigregister(iBallStatePtr)

iBallState_scfGetVersion = _cspace.iBallState_scfGetVersion

class csPolygonRange(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPolygonRange, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPolygonRange, name)
    def __repr__(self):
        return "<C csPolygonRange instance at %s>" % (self.this,)
    __swig_setmethods__["start"] = _cspace.csPolygonRange_start_set
    __swig_getmethods__["start"] = _cspace.csPolygonRange_start_get
    if _newclass:start = property(_cspace.csPolygonRange_start_get, _cspace.csPolygonRange_start_set)
    __swig_setmethods__["end"] = _cspace.csPolygonRange_end_set
    __swig_getmethods__["end"] = _cspace.csPolygonRange_end_get
    if _newclass:end = property(_cspace.csPolygonRange_end_get, _cspace.csPolygonRange_end_set)
    def __init__(self, *args):
        _swig_setattr(self, csPolygonRange, 'this', _cspace.new_csPolygonRange(*args))
        _swig_setattr(self, csPolygonRange, 'thisown', 1)
    def Set(*args): return _cspace.csPolygonRange_Set(*args)
    def __del__(self, destroy=_cspace.delete_csPolygonRange):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPolygonRangePtr(csPolygonRange):
    def __init__(self, this):
        _swig_setattr(self, csPolygonRange, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPolygonRange, 'thisown', 0)
        _swig_setattr(self, csPolygonRange,self.__class__,csPolygonRange)
_cspace.csPolygonRange_swigregister(csPolygonRangePtr)

CS_POLY_LIGHTING = _cspace.CS_POLY_LIGHTING
CS_POLY_COLLDET = _cspace.CS_POLY_COLLDET
CS_POLY_VISCULL = _cspace.CS_POLY_VISCULL
CS_POLYINDEX_LAST = _cspace.CS_POLYINDEX_LAST
CS_THING_NOCOMPRESS = _cspace.CS_THING_NOCOMPRESS
CS_THING_MOVE_NEVER = _cspace.CS_THING_MOVE_NEVER
CS_THING_MOVE_OCCASIONAL = _cspace.CS_THING_MOVE_OCCASIONAL
class iPolygonHandle(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPolygonHandle, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPolygonHandle, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPolygonHandle instance at %s>" % (self.this,)
    def GetThingFactoryState(*args): return _cspace.iPolygonHandle_GetThingFactoryState(*args)
    def GetMeshObjectFactory(*args): return _cspace.iPolygonHandle_GetMeshObjectFactory(*args)
    def GetThingState(*args): return _cspace.iPolygonHandle_GetThingState(*args)
    def GetMeshObject(*args): return _cspace.iPolygonHandle_GetMeshObject(*args)
    def GetIndex(*args): return _cspace.iPolygonHandle_GetIndex(*args)
    def __del__(self, destroy=_cspace.delete_iPolygonHandle):
        try:
            if self.thisown: destroy(self)
        except: pass

class iPolygonHandlePtr(iPolygonHandle):
    def __init__(self, this):
        _swig_setattr(self, iPolygonHandle, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPolygonHandle, 'thisown', 0)
        _swig_setattr(self, iPolygonHandle,self.__class__,iPolygonHandle)
_cspace.iPolygonHandle_swigregister(iPolygonHandlePtr)

class iThingFactoryState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iThingFactoryState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iThingFactoryState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iThingFactoryState instance at %s>" % (self.this,)
    def CompressVertices(*args): return _cspace.iThingFactoryState_CompressVertices(*args)
    def GetPolygonCount(*args): return _cspace.iThingFactoryState_GetPolygonCount(*args)
    def RemovePolygon(*args): return _cspace.iThingFactoryState_RemovePolygon(*args)
    def RemovePolygons(*args): return _cspace.iThingFactoryState_RemovePolygons(*args)
    def FindPolygonByName(*args): return _cspace.iThingFactoryState_FindPolygonByName(*args)
    def AddEmptyPolygon(*args): return _cspace.iThingFactoryState_AddEmptyPolygon(*args)
    def AddTriangle(*args): return _cspace.iThingFactoryState_AddTriangle(*args)
    def AddQuad(*args): return _cspace.iThingFactoryState_AddQuad(*args)
    def AddPolygon(*args): return _cspace.iThingFactoryState_AddPolygon(*args)
    def AddOutsideBox(*args): return _cspace.iThingFactoryState_AddOutsideBox(*args)
    def AddInsideBox(*args): return _cspace.iThingFactoryState_AddInsideBox(*args)
    def SetPolygonName(*args): return _cspace.iThingFactoryState_SetPolygonName(*args)
    def GetPolygonName(*args): return _cspace.iThingFactoryState_GetPolygonName(*args)
    def CreatePolygonHandle(*args): return _cspace.iThingFactoryState_CreatePolygonHandle(*args)
    def SetPolygonMaterial(*args): return _cspace.iThingFactoryState_SetPolygonMaterial(*args)
    def GetPolygonMaterial(*args): return _cspace.iThingFactoryState_GetPolygonMaterial(*args)
    def AddPolygonVertex(*args): return _cspace.iThingFactoryState_AddPolygonVertex(*args)
    def SetPolygonVertexIndices(*args): return _cspace.iThingFactoryState_SetPolygonVertexIndices(*args)
    def GetPolygonVertexCount(*args): return _cspace.iThingFactoryState_GetPolygonVertexCount(*args)
    def GetPolygonVertex(*args): return _cspace.iThingFactoryState_GetPolygonVertex(*args)
    def GetPolygonVertexIndices(*args): return _cspace.iThingFactoryState_GetPolygonVertexIndices(*args)
    def SetPolygonTextureMapping(*args): return _cspace.iThingFactoryState_SetPolygonTextureMapping(*args)
    def GetPolygonTextureMapping(*args): return _cspace.iThingFactoryState_GetPolygonTextureMapping(*args)
    def SetPolygonTextureMappingEnabled(*args): return _cspace.iThingFactoryState_SetPolygonTextureMappingEnabled(*args)
    def IsPolygonTextureMappingEnabled(*args): return _cspace.iThingFactoryState_IsPolygonTextureMappingEnabled(*args)
    def SetPolygonFlags(*args): return _cspace.iThingFactoryState_SetPolygonFlags(*args)
    def ResetPolygonFlags(*args): return _cspace.iThingFactoryState_ResetPolygonFlags(*args)
    def GetPolygonFlags(*args): return _cspace.iThingFactoryState_GetPolygonFlags(*args)
    def GetPolygonObjectPlane(*args): return _cspace.iThingFactoryState_GetPolygonObjectPlane(*args)
    def IsPolygonTransparent(*args): return _cspace.iThingFactoryState_IsPolygonTransparent(*args)
    def PointOnPolygon(*args): return _cspace.iThingFactoryState_PointOnPolygon(*args)
    def GetVertexCount(*args): return _cspace.iThingFactoryState_GetVertexCount(*args)
    def GetVertex(*args): return _cspace.iThingFactoryState_GetVertex(*args)
    def GetVertices(*args): return _cspace.iThingFactoryState_GetVertices(*args)
    def CreateVertex(*args): return _cspace.iThingFactoryState_CreateVertex(*args)
    def SetVertex(*args): return _cspace.iThingFactoryState_SetVertex(*args)
    def DeleteVertex(*args): return _cspace.iThingFactoryState_DeleteVertex(*args)
    def DeleteVertices(*args): return _cspace.iThingFactoryState_DeleteVertices(*args)
    def SetSmoothingFlag(*args): return _cspace.iThingFactoryState_SetSmoothingFlag(*args)
    def GetSmoothingFlag(*args): return _cspace.iThingFactoryState_GetSmoothingFlag(*args)
    def GetNormals(*args): return _cspace.iThingFactoryState_GetNormals(*args)
    def GetCosinusFactor(*args): return _cspace.iThingFactoryState_GetCosinusFactor(*args)
    def SetCosinusFactor(*args): return _cspace.iThingFactoryState_SetCosinusFactor(*args)
    def AddPolygonRenderBuffer(*args): return _cspace.iThingFactoryState_AddPolygonRenderBuffer(*args)
    def __del__(self, destroy=_cspace.delete_iThingFactoryState):
        try:
            if self.thisown: destroy(self)
        except: pass

class iThingFactoryStatePtr(iThingFactoryState):
    def __init__(self, this):
        _swig_setattr(self, iThingFactoryState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iThingFactoryState, 'thisown', 0)
        _swig_setattr(self, iThingFactoryState,self.__class__,iThingFactoryState)
_cspace.iThingFactoryState_swigregister(iThingFactoryStatePtr)

class iThingState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iThingState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iThingState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iThingState instance at %s>" % (self.this,)
    def GetFactory(*args): return _cspace.iThingState_GetFactory(*args)
    def GetVertexW(*args): return _cspace.iThingState_GetVertexW(*args)
    def GetVerticesW(*args): return _cspace.iThingState_GetVerticesW(*args)
    def GetMovingOption(*args): return _cspace.iThingState_GetMovingOption(*args)
    def SetMovingOption(*args): return _cspace.iThingState_SetMovingOption(*args)
    def Prepare(*args): return _cspace.iThingState_Prepare(*args)
    def Unprepare(*args): return _cspace.iThingState_Unprepare(*args)
    def ReplaceMaterial(*args): return _cspace.iThingState_ReplaceMaterial(*args)
    def ClearReplacedMaterials(*args): return _cspace.iThingState_ClearReplacedMaterials(*args)
    def SetMixMode(*args): return _cspace.iThingState_SetMixMode(*args)
    def GetMixMode(*args): return _cspace.iThingState_GetMixMode(*args)
    def CreatePolygonHandle(*args): return _cspace.iThingState_CreatePolygonHandle(*args)
    def GetPolygonWorldPlane(*args): return _cspace.iThingState_GetPolygonWorldPlane(*args)
    def __del__(self, destroy=_cspace.delete_iThingState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iThingState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iThingState_scfGetVersion)

class iThingStatePtr(iThingState):
    def __init__(self, this):
        _swig_setattr(self, iThingState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iThingState, 'thisown', 0)
        _swig_setattr(self, iThingState,self.__class__,iThingState)
_cspace.iThingState_swigregister(iThingStatePtr)

iThingState_scfGetVersion = _cspace.iThingState_scfGetVersion

class iThingEnvironment(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iThingEnvironment, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iThingEnvironment, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iThingEnvironment instance at %s>" % (self.this,)
    def Clear(*args): return _cspace.iThingEnvironment_Clear(*args)
    def GetLightmapCellSize(*args): return _cspace.iThingEnvironment_GetLightmapCellSize(*args)
    def SetLightmapCellSize(*args): return _cspace.iThingEnvironment_SetLightmapCellSize(*args)
    def GetDefaultLightmapCellSize(*args): return _cspace.iThingEnvironment_GetDefaultLightmapCellSize(*args)
    def __del__(self, destroy=_cspace.delete_iThingEnvironment):
        try:
            if self.thisown: destroy(self)
        except: pass

class iThingEnvironmentPtr(iThingEnvironment):
    def __init__(self, this):
        _swig_setattr(self, iThingEnvironment, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iThingEnvironment, 'thisown', 0)
        _swig_setattr(self, iThingEnvironment,self.__class__,iThingEnvironment)
_cspace.iThingEnvironment_swigregister(iThingEnvironmentPtr)

class iTerrainObjectState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTerrainObjectState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTerrainObjectState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTerrainObjectState instance at %s>" % (self.this,)
    def SetMaterialPalette(*args): return _cspace.iTerrainObjectState_SetMaterialPalette(*args)
    def GetMaterialPalette(*args): return _cspace.iTerrainObjectState_GetMaterialPalette(*args)
    def SetMaterialMap(*args): return _cspace.iTerrainObjectState_SetMaterialMap(*args)
    def SetLODValue(*args): return _cspace.iTerrainObjectState_SetLODValue(*args)
    def GetLODValue(*args): return _cspace.iTerrainObjectState_GetLODValue(*args)
    def SaveState(*args): return _cspace.iTerrainObjectState_SaveState(*args)
    def RestoreState(*args): return _cspace.iTerrainObjectState_RestoreState(*args)
    def CollisionDetect(*args): return _cspace.iTerrainObjectState_CollisionDetect(*args)
    def SetStaticLighting(*args): return _cspace.iTerrainObjectState_SetStaticLighting(*args)
    def GetStaticLighting(*args): return _cspace.iTerrainObjectState_GetStaticLighting(*args)
    def SetCastShadows(*args): return _cspace.iTerrainObjectState_SetCastShadows(*args)
    def GetCastShadows(*args): return _cspace.iTerrainObjectState_GetCastShadows(*args)
    def __del__(self, destroy=_cspace.delete_iTerrainObjectState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTerrainObjectState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTerrainObjectState_scfGetVersion)

class iTerrainObjectStatePtr(iTerrainObjectState):
    def __init__(self, this):
        _swig_setattr(self, iTerrainObjectState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTerrainObjectState, 'thisown', 0)
        _swig_setattr(self, iTerrainObjectState,self.__class__,iTerrainObjectState)
_cspace.iTerrainObjectState_swigregister(iTerrainObjectStatePtr)

iTerrainObjectState_scfGetVersion = _cspace.iTerrainObjectState_scfGetVersion

class iTerrainFactoryState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTerrainFactoryState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTerrainFactoryState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTerrainFactoryState instance at %s>" % (self.this,)
    def SetTerraFormer(*args): return _cspace.iTerrainFactoryState_SetTerraFormer(*args)
    def GetTerraFormer(*args): return _cspace.iTerrainFactoryState_GetTerraFormer(*args)
    def SetSamplerRegion(*args): return _cspace.iTerrainFactoryState_SetSamplerRegion(*args)
    def GetSamplerRegion(*args): return _cspace.iTerrainFactoryState_GetSamplerRegion(*args)
    def SaveState(*args): return _cspace.iTerrainFactoryState_SaveState(*args)
    def RestoreState(*args): return _cspace.iTerrainFactoryState_RestoreState(*args)
    def __del__(self, destroy=_cspace.delete_iTerrainFactoryState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTerrainFactoryState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTerrainFactoryState_scfGetVersion)

class iTerrainFactoryStatePtr(iTerrainFactoryState):
    def __init__(self, this):
        _swig_setattr(self, iTerrainFactoryState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTerrainFactoryState, 'thisown', 0)
        _swig_setattr(self, iTerrainFactoryState,self.__class__,iTerrainFactoryState)
_cspace.iTerrainFactoryState_swigregister(iTerrainFactoryStatePtr)

iTerrainFactoryState_scfGetVersion = _cspace.iTerrainFactoryState_scfGetVersion

class iLoaderStatus(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLoaderStatus, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLoaderStatus, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLoaderStatus instance at %s>" % (self.this,)
    def IsReady(*args): return _cspace.iLoaderStatus_IsReady(*args)
    def IsError(*args): return _cspace.iLoaderStatus_IsError(*args)
    def __del__(self, destroy=_cspace.delete_iLoaderStatus):
        try:
            if self.thisown: destroy(self)
        except: pass

class iLoaderStatusPtr(iLoaderStatus):
    def __init__(self, this):
        _swig_setattr(self, iLoaderStatus, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLoaderStatus, 'thisown', 0)
        _swig_setattr(self, iLoaderStatus,self.__class__,iLoaderStatus)
_cspace.iLoaderStatus_swigregister(iLoaderStatusPtr)

class iLoader(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLoader, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLoader, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLoader instance at %s>" % (self.this,)
    def LoadImage(*args): return _cspace.iLoader_LoadImage(*args)
    def LoadTexture(*args): return _cspace.iLoader_LoadTexture(*args)
    def LoadSoundData(*args): return _cspace.iLoader_LoadSoundData(*args)
    def LoadSound(*args): return _cspace.iLoader_LoadSound(*args)
    def ThreadedLoadMapFile(*args): return _cspace.iLoader_ThreadedLoadMapFile(*args)
    def LoadMapFile(*args): return _cspace.iLoader_LoadMapFile(*args)
    def LoadLibraryFile(*args): return _cspace.iLoader_LoadLibraryFile(*args)
    def LoadMeshObjectFactory(*args): return _cspace.iLoader_LoadMeshObjectFactory(*args)
    def LoadMeshObject(*args): return _cspace.iLoader_LoadMeshObject(*args)
    def Load(*args): return _cspace.iLoader_Load(*args)
    def __del__(self, destroy=_cspace.delete_iLoader):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iLoader_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iLoader_scfGetVersion)

class iLoaderPtr(iLoader):
    def __init__(self, this):
        _swig_setattr(self, iLoader, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLoader, 'thisown', 0)
        _swig_setattr(self, iLoader,self.__class__,iLoader)
_cspace.iLoader_swigregister(iLoaderPtr)

iLoader_scfGetVersion = _cspace.iLoader_scfGetVersion

class iLoaderPlugin(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iLoaderPlugin, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iLoaderPlugin, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iLoaderPlugin instance at %s>" % (self.this,)
    def Parse(*args): return _cspace.iLoaderPlugin_Parse(*args)
    def __del__(self, destroy=_cspace.delete_iLoaderPlugin):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iLoaderPlugin_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iLoaderPlugin_scfGetVersion)

class iLoaderPluginPtr(iLoaderPlugin):
    def __init__(self, this):
        _swig_setattr(self, iLoaderPlugin, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iLoaderPlugin, 'thisown', 0)
        _swig_setattr(self, iLoaderPlugin,self.__class__,iLoaderPlugin)
_cspace.iLoaderPlugin_swigregister(iLoaderPluginPtr)

iLoaderPlugin_scfGetVersion = _cspace.iLoaderPlugin_scfGetVersion

class iBinaryLoaderPlugin(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iBinaryLoaderPlugin, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iBinaryLoaderPlugin, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iBinaryLoaderPlugin instance at %s>" % (self.this,)
    def Parse(*args): return _cspace.iBinaryLoaderPlugin_Parse(*args)
    def __del__(self, destroy=_cspace.delete_iBinaryLoaderPlugin):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iBinaryLoaderPlugin_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iBinaryLoaderPlugin_scfGetVersion)

class iBinaryLoaderPluginPtr(iBinaryLoaderPlugin):
    def __init__(self, this):
        _swig_setattr(self, iBinaryLoaderPlugin, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iBinaryLoaderPlugin, 'thisown', 0)
        _swig_setattr(self, iBinaryLoaderPlugin,self.__class__,iBinaryLoaderPlugin)
_cspace.iBinaryLoaderPlugin_swigregister(iBinaryLoaderPluginPtr)

iBinaryLoaderPlugin_scfGetVersion = _cspace.iBinaryLoaderPlugin_scfGetVersion

class iSaver(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSaver, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSaver, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSaver instance at %s>" % (self.this,)
    def SaveMapFile(*args): return _cspace.iSaver_SaveMapFile(*args)
    def __del__(self, destroy=_cspace.delete_iSaver):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSaverPtr(iSaver):
    def __init__(self, this):
        _swig_setattr(self, iSaver, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSaver, 'thisown', 0)
        _swig_setattr(self, iSaver,self.__class__,iSaver)
_cspace.iSaver_swigregister(iSaverPtr)

class iSoundHandle(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSoundHandle, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSoundHandle, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSoundHandle instance at %s>" % (self.this,)
    def IsStatic(*args): return _cspace.iSoundHandle_IsStatic(*args)
    def Play(*args): return _cspace.iSoundHandle_Play(*args)
    def CreateSource(*args): return _cspace.iSoundHandle_CreateSource(*args)
    def StartStream(*args): return _cspace.iSoundHandle_StartStream(*args)
    def StopStream(*args): return _cspace.iSoundHandle_StopStream(*args)
    def ResetStream(*args): return _cspace.iSoundHandle_ResetStream(*args)
    def __del__(self, destroy=_cspace.delete_iSoundHandle):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSoundHandle_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSoundHandle_scfGetVersion)

class iSoundHandlePtr(iSoundHandle):
    def __init__(self, this):
        _swig_setattr(self, iSoundHandle, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSoundHandle, 'thisown', 0)
        _swig_setattr(self, iSoundHandle,self.__class__,iSoundHandle)
_cspace.iSoundHandle_swigregister(iSoundHandlePtr)

iSoundHandle_scfGetVersion = _cspace.iSoundHandle_scfGetVersion

class iSoundLoader(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSoundLoader, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSoundLoader, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSoundLoader instance at %s>" % (self.this,)
    def LoadSound(*args): return _cspace.iSoundLoader_LoadSound(*args)
    def __del__(self, destroy=_cspace.delete_iSoundLoader):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSoundLoader_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSoundLoader_scfGetVersion)

class iSoundLoaderPtr(iSoundLoader):
    def __init__(self, this):
        _swig_setattr(self, iSoundLoader, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSoundLoader, 'thisown', 0)
        _swig_setattr(self, iSoundLoader,self.__class__,iSoundLoader)
_cspace.iSoundLoader_swigregister(iSoundLoaderPtr)

iSoundLoader_scfGetVersion = _cspace.iSoundLoader_scfGetVersion

class iSoundRender(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSoundRender, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSoundRender, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSoundRender instance at %s>" % (self.this,)
    def SetVolume(*args): return _cspace.iSoundRender_SetVolume(*args)
    def GetVolume(*args): return _cspace.iSoundRender_GetVolume(*args)
    def RegisterSound(*args): return _cspace.iSoundRender_RegisterSound(*args)
    def UnregisterSound(*args): return _cspace.iSoundRender_UnregisterSound(*args)
    def GetListener(*args): return _cspace.iSoundRender_GetListener(*args)
    def MixingFunction(*args): return _cspace.iSoundRender_MixingFunction(*args)
    def __del__(self, destroy=_cspace.delete_iSoundRender):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSoundRender_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSoundRender_scfGetVersion)

class iSoundRenderPtr(iSoundRender):
    def __init__(self, this):
        _swig_setattr(self, iSoundRender, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSoundRender, 'thisown', 0)
        _swig_setattr(self, iSoundRender,self.__class__,iSoundRender)
_cspace.iSoundRender_swigregister(iSoundRenderPtr)

iSoundRender_scfGetVersion = _cspace.iSoundRender_scfGetVersion

class iSoundWrapper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSoundWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSoundWrapper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSoundWrapper instance at %s>" % (self.this,)
    def GetSound(*args): return _cspace.iSoundWrapper_GetSound(*args)
    def QueryObject(*args): return _cspace.iSoundWrapper_QueryObject(*args)
    def __del__(self, destroy=_cspace.delete_iSoundWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSoundWrapper_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSoundWrapper_scfGetVersion)

class iSoundWrapperPtr(iSoundWrapper):
    def __init__(self, this):
        _swig_setattr(self, iSoundWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSoundWrapper, 'thisown', 0)
        _swig_setattr(self, iSoundWrapper,self.__class__,iSoundWrapper)
_cspace.iSoundWrapper_swigregister(iSoundWrapperPtr)

iSoundWrapper_scfGetVersion = _cspace.iSoundWrapper_scfGetVersion

class iSoundDriver(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSoundDriver, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSoundDriver, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSoundDriver instance at %s>" % (self.this,)
    def Open(*args): return _cspace.iSoundDriver_Open(*args)
    def Close(*args): return _cspace.iSoundDriver_Close(*args)
    def LockMemory(*args): return _cspace.iSoundDriver_LockMemory(*args)
    def UnlockMemory(*args): return _cspace.iSoundDriver_UnlockMemory(*args)
    def IsBackground(*args): return _cspace.iSoundDriver_IsBackground(*args)
    def Is16Bits(*args): return _cspace.iSoundDriver_Is16Bits(*args)
    def IsStereo(*args): return _cspace.iSoundDriver_IsStereo(*args)
    def GetFrequency(*args): return _cspace.iSoundDriver_GetFrequency(*args)
    def IsHandleVoidSound(*args): return _cspace.iSoundDriver_IsHandleVoidSound(*args)
    def ThreadAware(*args): return _cspace.iSoundDriver_ThreadAware(*args)
    def __del__(self, destroy=_cspace.delete_iSoundDriver):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSoundDriver_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSoundDriver_scfGetVersion)

class iSoundDriverPtr(iSoundDriver):
    def __init__(self, this):
        _swig_setattr(self, iSoundDriver, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSoundDriver, 'thisown', 0)
        _swig_setattr(self, iSoundDriver,self.__class__,iSoundDriver)
_cspace.iSoundDriver_swigregister(iSoundDriverPtr)

iSoundDriver_scfGetVersion = _cspace.iSoundDriver_scfGetVersion

SOUND_RESTART = _cspace.SOUND_RESTART
SOUND_LOOP = _cspace.SOUND_LOOP
SOUND3D_DISABLE = _cspace.SOUND3D_DISABLE
SOUND3D_RELATIVE = _cspace.SOUND3D_RELATIVE
SOUND3D_ABSOLUTE = _cspace.SOUND3D_ABSOLUTE
SOUND_DISTANCE_INFINITE = _cspace.SOUND_DISTANCE_INFINITE
class iSoundSource(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSoundSource, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSoundSource, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSoundSource instance at %s>" % (self.this,)
    def Play(*args): return _cspace.iSoundSource_Play(*args)
    def Stop(*args): return _cspace.iSoundSource_Stop(*args)
    def SetVolume(*args): return _cspace.iSoundSource_SetVolume(*args)
    def GetVolume(*args): return _cspace.iSoundSource_GetVolume(*args)
    def SetFrequencyFactor(*args): return _cspace.iSoundSource_SetFrequencyFactor(*args)
    def GetFrequencyFactor(*args): return _cspace.iSoundSource_GetFrequencyFactor(*args)
    def GetMode3D(*args): return _cspace.iSoundSource_GetMode3D(*args)
    def SetMode3D(*args): return _cspace.iSoundSource_SetMode3D(*args)
    def SetPosition(*args): return _cspace.iSoundSource_SetPosition(*args)
    def GetPosition(*args): return _cspace.iSoundSource_GetPosition(*args)
    def SetVelocity(*args): return _cspace.iSoundSource_SetVelocity(*args)
    def GetVelocity(*args): return _cspace.iSoundSource_GetVelocity(*args)
    def SetMinimumDistance(*args): return _cspace.iSoundSource_SetMinimumDistance(*args)
    def SetMaximumDistance(*args): return _cspace.iSoundSource_SetMaximumDistance(*args)
    def GetMinimumDistance(*args): return _cspace.iSoundSource_GetMinimumDistance(*args)
    def GetMaximumDistance(*args): return _cspace.iSoundSource_GetMaximumDistance(*args)
    def __del__(self, destroy=_cspace.delete_iSoundSource):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSoundSource_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSoundSource_scfGetVersion)

class iSoundSourcePtr(iSoundSource):
    def __init__(self, this):
        _swig_setattr(self, iSoundSource, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSoundSource, 'thisown', 0)
        _swig_setattr(self, iSoundSource,self.__class__,iSoundSource)
_cspace.iSoundSource_swigregister(iSoundSourcePtr)

iSoundSource_scfGetVersion = _cspace.iSoundSource_scfGetVersion

class iComponent(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iComponent, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iComponent, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iComponent instance at %s>" % (self.this,)
    def Initialize(*args): return _cspace.iComponent_Initialize(*args)
    def __del__(self, destroy=_cspace.delete_iComponent):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iComponent_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iComponent_scfGetVersion)

class iComponentPtr(iComponent):
    def __init__(self, this):
        _swig_setattr(self, iComponent, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iComponent, 'thisown', 0)
        _swig_setattr(self, iComponent,self.__class__,iComponent)
_cspace.iComponent_swigregister(iComponentPtr)

iComponent_scfGetVersion = _cspace.iComponent_scfGetVersion

class iCacheManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCacheManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCacheManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCacheManager instance at %s>" % (self.this,)
    def SetCurrentType(*args): return _cspace.iCacheManager_SetCurrentType(*args)
    def GetCurrentType(*args): return _cspace.iCacheManager_GetCurrentType(*args)
    def SetCurrentScope(*args): return _cspace.iCacheManager_SetCurrentScope(*args)
    def GetCurrentScope(*args): return _cspace.iCacheManager_GetCurrentScope(*args)
    def CacheData(*args): return _cspace.iCacheManager_CacheData(*args)
    def ReadCache(*args): return _cspace.iCacheManager_ReadCache(*args)
    def ClearCache(*args): return _cspace.iCacheManager_ClearCache(*args)
    def Flush(*args): return _cspace.iCacheManager_Flush(*args)
    def __del__(self, destroy=_cspace.delete_iCacheManager):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iCacheManager_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iCacheManager_scfGetVersion)

class iCacheManagerPtr(iCacheManager):
    def __init__(self, this):
        _swig_setattr(self, iCacheManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCacheManager, 'thisown', 0)
        _swig_setattr(self, iCacheManager,self.__class__,iCacheManager)
_cspace.iCacheManager_swigregister(iCacheManagerPtr)

iCacheManager_scfGetVersion = _cspace.iCacheManager_scfGetVersion

class csFileTime(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csFileTime, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csFileTime, name)
    def __repr__(self):
        return "<C csFileTime instance at %s>" % (self.this,)
    __swig_setmethods__["sec"] = _cspace.csFileTime_sec_set
    __swig_getmethods__["sec"] = _cspace.csFileTime_sec_get
    if _newclass:sec = property(_cspace.csFileTime_sec_get, _cspace.csFileTime_sec_set)
    __swig_setmethods__["min"] = _cspace.csFileTime_min_set
    __swig_getmethods__["min"] = _cspace.csFileTime_min_get
    if _newclass:min = property(_cspace.csFileTime_min_get, _cspace.csFileTime_min_set)
    __swig_setmethods__["hour"] = _cspace.csFileTime_hour_set
    __swig_getmethods__["hour"] = _cspace.csFileTime_hour_get
    if _newclass:hour = property(_cspace.csFileTime_hour_get, _cspace.csFileTime_hour_set)
    __swig_setmethods__["day"] = _cspace.csFileTime_day_set
    __swig_getmethods__["day"] = _cspace.csFileTime_day_get
    if _newclass:day = property(_cspace.csFileTime_day_get, _cspace.csFileTime_day_set)
    __swig_setmethods__["mon"] = _cspace.csFileTime_mon_set
    __swig_getmethods__["mon"] = _cspace.csFileTime_mon_get
    if _newclass:mon = property(_cspace.csFileTime_mon_get, _cspace.csFileTime_mon_set)
    __swig_setmethods__["year"] = _cspace.csFileTime_year_set
    __swig_getmethods__["year"] = _cspace.csFileTime_year_get
    if _newclass:year = property(_cspace.csFileTime_year_get, _cspace.csFileTime_year_set)
    def __init__(self, *args):
        _swig_setattr(self, csFileTime, 'this', _cspace.new_csFileTime(*args))
        _swig_setattr(self, csFileTime, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csFileTime):
        try:
            if self.thisown: destroy(self)
        except: pass

class csFileTimePtr(csFileTime):
    def __init__(self, this):
        _swig_setattr(self, csFileTime, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csFileTime, 'thisown', 0)
        _swig_setattr(self, csFileTime,self.__class__,csFileTime)
_cspace.csFileTime_swigregister(csFileTimePtr)

VFS_PATH_DIVIDER = _cspace.VFS_PATH_DIVIDER
VFS_PATH_SEPARATOR = _cspace.VFS_PATH_SEPARATOR
VFS_MAX_PATH_LEN = _cspace.VFS_MAX_PATH_LEN
VFS_FILE_MODE = _cspace.VFS_FILE_MODE
VFS_FILE_READ = _cspace.VFS_FILE_READ
VFS_FILE_WRITE = _cspace.VFS_FILE_WRITE
VFS_FILE_APPEND = _cspace.VFS_FILE_APPEND
VFS_FILE_UNCOMPRESSED = _cspace.VFS_FILE_UNCOMPRESSED
VFS_STATUS_OK = _cspace.VFS_STATUS_OK
VFS_STATUS_OTHER = _cspace.VFS_STATUS_OTHER
VFS_STATUS_NOSPACE = _cspace.VFS_STATUS_NOSPACE
VFS_STATUS_RESOURCES = _cspace.VFS_STATUS_RESOURCES
VFS_STATUS_ACCESSDENIED = _cspace.VFS_STATUS_ACCESSDENIED
VFS_STATUS_IOERROR = _cspace.VFS_STATUS_IOERROR
class iFile(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFile, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFile, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFile instance at %s>" % (self.this,)
    def GetName(*args): return _cspace.iFile_GetName(*args)
    def GetSize(*args): return _cspace.iFile_GetSize(*args)
    def GetStatus(*args): return _cspace.iFile_GetStatus(*args)
    def Read(*args): return _cspace.iFile_Read(*args)
    def Write(*args): return _cspace.iFile_Write(*args)
    def Flush(*args): return _cspace.iFile_Flush(*args)
    def AtEOF(*args): return _cspace.iFile_AtEOF(*args)
    def GetPos(*args): return _cspace.iFile_GetPos(*args)
    def SetPos(*args): return _cspace.iFile_SetPos(*args)
    def GetAllData(*args): return _cspace.iFile_GetAllData(*args)
    def __del__(self, destroy=_cspace.delete_iFile):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iFile_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iFile_scfGetVersion)

class iFilePtr(iFile):
    def __init__(self, this):
        _swig_setattr(self, iFile, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFile, 'thisown', 0)
        _swig_setattr(self, iFile,self.__class__,iFile)
_cspace.iFile_swigregister(iFilePtr)

iFile_scfGetVersion = _cspace.iFile_scfGetVersion

class iVFS(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVFS, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVFS, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVFS instance at %s>" % (self.this,)
    def ChDir(*args): return _cspace.iVFS_ChDir(*args)
    def GetCwd(*args): return _cspace.iVFS_GetCwd(*args)
    def PushDir(*args): return _cspace.iVFS_PushDir(*args)
    def PopDir(*args): return _cspace.iVFS_PopDir(*args)
    def ExpandPath(*args): return _cspace.iVFS_ExpandPath(*args)
    def Exists(*args): return _cspace.iVFS_Exists(*args)
    def FindFiles(*args): return _cspace.iVFS_FindFiles(*args)
    def Open(*args): return _cspace.iVFS_Open(*args)
    def ReadFile(*args): return _cspace.iVFS_ReadFile(*args)
    def WriteFile(*args): return _cspace.iVFS_WriteFile(*args)
    def DeleteFile(*args): return _cspace.iVFS_DeleteFile(*args)
    def Sync(*args): return _cspace.iVFS_Sync(*args)
    def Mount(*args): return _cspace.iVFS_Mount(*args)
    def Unmount(*args): return _cspace.iVFS_Unmount(*args)
    def MountRoot(*args): return _cspace.iVFS_MountRoot(*args)
    def SaveMounts(*args): return _cspace.iVFS_SaveMounts(*args)
    def LoadMountsFromFile(*args): return _cspace.iVFS_LoadMountsFromFile(*args)
    def ChDirAuto(*args): return _cspace.iVFS_ChDirAuto(*args)
    def GetFileTime(*args): return _cspace.iVFS_GetFileTime(*args)
    def SetFileTime(*args): return _cspace.iVFS_SetFileTime(*args)
    def GetFileSize(*args): return _cspace.iVFS_GetFileSize(*args)
    def GetRealPath(*args): return _cspace.iVFS_GetRealPath(*args)
    def GetMounts(*args): return _cspace.iVFS_GetMounts(*args)
    def GetRealMountPaths(*args): return _cspace.iVFS_GetRealMountPaths(*args)
    def __del__(self, destroy=_cspace.delete_iVFS):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iVFS_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iVFS_scfGetVersion)

class iVFSPtr(iVFS):
    def __init__(self, this):
        _swig_setattr(self, iVFS, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVFS, 'thisown', 0)
        _swig_setattr(self, iVFS,self.__class__,iVFS)
_cspace.iVFS_swigregister(iVFSPtr)

iVFS_scfGetVersion = _cspace.iVFS_scfGetVersion

class iObject(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iObject, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iObject, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iObject instance at %s>" % (self.this,)
    def SetName(*args): return _cspace.iObject_SetName(*args)
    def GetName(*args): return _cspace.iObject_GetName(*args)
    def GetID(*args): return _cspace.iObject_GetID(*args)
    def SetObjectParent(*args): return _cspace.iObject_SetObjectParent(*args)
    def GetObjectParent(*args): return _cspace.iObject_GetObjectParent(*args)
    def ObjAdd(*args): return _cspace.iObject_ObjAdd(*args)
    def ObjRemove(*args): return _cspace.iObject_ObjRemove(*args)
    def ObjRemoveAll(*args): return _cspace.iObject_ObjRemoveAll(*args)
    def ObjAddChildren(*args): return _cspace.iObject_ObjAddChildren(*args)
    def GetChild(*args): return _cspace.iObject_GetChild(*args)
    def GetIterator(*args): return _cspace.iObject_GetIterator(*args)
    def ObjReleaseOld(*args): return _cspace.iObject_ObjReleaseOld(*args)
    def __del__(self, destroy=_cspace.delete_iObject):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iObject_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iObject_scfGetVersion)

class iObjectPtr(iObject):
    def __init__(self, this):
        _swig_setattr(self, iObject, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iObject, 'thisown', 0)
        _swig_setattr(self, iObject,self.__class__,iObject)
_cspace.iObject_swigregister(iObjectPtr)

iObject_scfGetVersion = _cspace.iObject_scfGetVersion

class iObjectIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iObjectIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iObjectIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iObjectIterator instance at %s>" % (self.this,)
    def Next(*args): return _cspace.iObjectIterator_Next(*args)
    def Reset(*args): return _cspace.iObjectIterator_Reset(*args)
    def GetParentObj(*args): return _cspace.iObjectIterator_GetParentObj(*args)
    def HasNext(*args): return _cspace.iObjectIterator_HasNext(*args)
    def FindName(*args): return _cspace.iObjectIterator_FindName(*args)
    def __del__(self, destroy=_cspace.delete_iObjectIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iObjectIteratorPtr(iObjectIterator):
    def __init__(self, this):
        _swig_setattr(self, iObjectIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iObjectIterator, 'thisown', 0)
        _swig_setattr(self, iObjectIterator,self.__class__,iObjectIterator)
_cspace.iObjectIterator_swigregister(iObjectIteratorPtr)

class iObjectRegistry(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iObjectRegistry, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iObjectRegistry, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iObjectRegistry instance at %s>" % (self.this,)
    def Clear(*args): return _cspace.iObjectRegistry_Clear(*args)
    def Register(*args): return _cspace.iObjectRegistry_Register(*args)
    def Unregister(*args): return _cspace.iObjectRegistry_Unregister(*args)
    def Get(*args): return _cspace.iObjectRegistry_Get(*args)
    def __del__(self, destroy=_cspace.delete_iObjectRegistry):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iObjectRegistry_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iObjectRegistry_scfGetVersion)

class iObjectRegistryPtr(iObjectRegistry):
    def __init__(self, this):
        _swig_setattr(self, iObjectRegistry, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iObjectRegistry, 'thisown', 0)
        _swig_setattr(self, iObjectRegistry,self.__class__,iObjectRegistry)
_cspace.iObjectRegistry_swigregister(iObjectRegistryPtr)

iObjectRegistry_scfGetVersion = _cspace.iObjectRegistry_scfGetVersion

class iObjectRegistryIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iObjectRegistryIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iObjectRegistryIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iObjectRegistryIterator instance at %s>" % (self.this,)
    def Reset(*args): return _cspace.iObjectRegistryIterator_Reset(*args)
    def GetCurrentTag(*args): return _cspace.iObjectRegistryIterator_GetCurrentTag(*args)
    def HasNext(*args): return _cspace.iObjectRegistryIterator_HasNext(*args)
    def Next(*args): return _cspace.iObjectRegistryIterator_Next(*args)
    def __del__(self, destroy=_cspace.delete_iObjectRegistryIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iObjectRegistryIteratorPtr(iObjectRegistryIterator):
    def __init__(self, this):
        _swig_setattr(self, iObjectRegistryIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iObjectRegistryIterator, 'thisown', 0)
        _swig_setattr(self, iObjectRegistryIterator,self.__class__,iObjectRegistryIterator)
_cspace.iObjectRegistryIterator_swigregister(iObjectRegistryIteratorPtr)

class iVirtualClock(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVirtualClock, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVirtualClock, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVirtualClock instance at %s>" % (self.this,)
    def Advance(*args): return _cspace.iVirtualClock_Advance(*args)
    def Suspend(*args): return _cspace.iVirtualClock_Suspend(*args)
    def Resume(*args): return _cspace.iVirtualClock_Resume(*args)
    def GetElapsedTicks(*args): return _cspace.iVirtualClock_GetElapsedTicks(*args)
    def GetCurrentTicks(*args): return _cspace.iVirtualClock_GetCurrentTicks(*args)
    def __del__(self, destroy=_cspace.delete_iVirtualClock):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iVirtualClock_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iVirtualClock_scfGetVersion)

class iVirtualClockPtr(iVirtualClock):
    def __init__(self, this):
        _swig_setattr(self, iVirtualClock, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVirtualClock, 'thisown', 0)
        _swig_setattr(self, iVirtualClock,self.__class__,iVirtualClock)
_cspace.iVirtualClock_swigregister(iVirtualClockPtr)

iVirtualClock_scfGetVersion = _cspace.iVirtualClock_scfGetVersion

class iEventAttributeIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEventAttributeIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEventAttributeIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEventAttributeIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iEventAttributeIterator_HasNext(*args)
    def Next(*args): return _cspace.iEventAttributeIterator_Next(*args)
    def Reset(*args): return _cspace.iEventAttributeIterator_Reset(*args)
    def __del__(self, destroy=_cspace.delete_iEventAttributeIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEventAttributeIteratorPtr(iEventAttributeIterator):
    def __init__(self, this):
        _swig_setattr(self, iEventAttributeIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEventAttributeIterator, 'thisown', 0)
        _swig_setattr(self, iEventAttributeIterator,self.__class__,iEventAttributeIterator)
_cspace.iEventAttributeIterator_swigregister(iEventAttributeIteratorPtr)

class csKeyEventData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csKeyEventData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csKeyEventData, name)
    def __repr__(self):
        return "<C csKeyEventData instance at %s>" % (self.this,)
    __swig_setmethods__["eventType"] = _cspace.csKeyEventData_eventType_set
    __swig_getmethods__["eventType"] = _cspace.csKeyEventData_eventType_get
    if _newclass:eventType = property(_cspace.csKeyEventData_eventType_get, _cspace.csKeyEventData_eventType_set)
    __swig_setmethods__["codeRaw"] = _cspace.csKeyEventData_codeRaw_set
    __swig_getmethods__["codeRaw"] = _cspace.csKeyEventData_codeRaw_get
    if _newclass:codeRaw = property(_cspace.csKeyEventData_codeRaw_get, _cspace.csKeyEventData_codeRaw_set)
    __swig_setmethods__["codeCooked"] = _cspace.csKeyEventData_codeCooked_set
    __swig_getmethods__["codeCooked"] = _cspace.csKeyEventData_codeCooked_get
    if _newclass:codeCooked = property(_cspace.csKeyEventData_codeCooked_get, _cspace.csKeyEventData_codeCooked_set)
    __swig_setmethods__["modifiers"] = _cspace.csKeyEventData_modifiers_set
    __swig_getmethods__["modifiers"] = _cspace.csKeyEventData_modifiers_get
    if _newclass:modifiers = property(_cspace.csKeyEventData_modifiers_get, _cspace.csKeyEventData_modifiers_set)
    __swig_setmethods__["autoRepeat"] = _cspace.csKeyEventData_autoRepeat_set
    __swig_getmethods__["autoRepeat"] = _cspace.csKeyEventData_autoRepeat_get
    if _newclass:autoRepeat = property(_cspace.csKeyEventData_autoRepeat_get, _cspace.csKeyEventData_autoRepeat_set)
    __swig_setmethods__["charType"] = _cspace.csKeyEventData_charType_set
    __swig_getmethods__["charType"] = _cspace.csKeyEventData_charType_get
    if _newclass:charType = property(_cspace.csKeyEventData_charType_get, _cspace.csKeyEventData_charType_set)
    def __init__(self, *args):
        _swig_setattr(self, csKeyEventData, 'this', _cspace.new_csKeyEventData(*args))
        _swig_setattr(self, csKeyEventData, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csKeyEventData):
        try:
            if self.thisown: destroy(self)
        except: pass

class csKeyEventDataPtr(csKeyEventData):
    def __init__(self, this):
        _swig_setattr(self, csKeyEventData, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csKeyEventData, 'thisown', 0)
        _swig_setattr(self, csKeyEventData,self.__class__,csKeyEventData)
_cspace.csKeyEventData_swigregister(csKeyEventDataPtr)

csmbLeft = _cspace.csmbLeft
csmbRight = _cspace.csmbRight
csmbMiddle = _cspace.csmbMiddle
csmbWheelUp = _cspace.csmbWheelUp
csmbWheelDown = _cspace.csmbWheelDown
csmbExtra1 = _cspace.csmbExtra1
csmbExtra2 = _cspace.csmbExtra2
class csEventMouseData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csEventMouseData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csEventMouseData, name)
    def __repr__(self):
        return "<C csEventMouseData instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cspace.csEventMouseData_x_set
    __swig_getmethods__["x"] = _cspace.csEventMouseData_x_get
    if _newclass:x = property(_cspace.csEventMouseData_x_get, _cspace.csEventMouseData_x_set)
    __swig_setmethods__["y"] = _cspace.csEventMouseData_y_set
    __swig_getmethods__["y"] = _cspace.csEventMouseData_y_get
    if _newclass:y = property(_cspace.csEventMouseData_y_get, _cspace.csEventMouseData_y_set)
    __swig_setmethods__["Button"] = _cspace.csEventMouseData_Button_set
    __swig_getmethods__["Button"] = _cspace.csEventMouseData_Button_get
    if _newclass:Button = property(_cspace.csEventMouseData_Button_get, _cspace.csEventMouseData_Button_set)
    __swig_setmethods__["Modifiers"] = _cspace.csEventMouseData_Modifiers_set
    __swig_getmethods__["Modifiers"] = _cspace.csEventMouseData_Modifiers_get
    if _newclass:Modifiers = property(_cspace.csEventMouseData_Modifiers_get, _cspace.csEventMouseData_Modifiers_set)
    def __init__(self, *args):
        _swig_setattr(self, csEventMouseData, 'this', _cspace.new_csEventMouseData(*args))
        _swig_setattr(self, csEventMouseData, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csEventMouseData):
        try:
            if self.thisown: destroy(self)
        except: pass

class csEventMouseDataPtr(csEventMouseData):
    def __init__(self, this):
        _swig_setattr(self, csEventMouseData, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csEventMouseData, 'thisown', 0)
        _swig_setattr(self, csEventMouseData,self.__class__,csEventMouseData)
_cspace.csEventMouseData_swigregister(csEventMouseDataPtr)

class csEventJoystickData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csEventJoystickData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csEventJoystickData, name)
    def __repr__(self):
        return "<C csEventJoystickData instance at %s>" % (self.this,)
    __swig_setmethods__["number"] = _cspace.csEventJoystickData_number_set
    __swig_getmethods__["number"] = _cspace.csEventJoystickData_number_get
    if _newclass:number = property(_cspace.csEventJoystickData_number_get, _cspace.csEventJoystickData_number_set)
    __swig_setmethods__["x"] = _cspace.csEventJoystickData_x_set
    __swig_getmethods__["x"] = _cspace.csEventJoystickData_x_get
    if _newclass:x = property(_cspace.csEventJoystickData_x_get, _cspace.csEventJoystickData_x_set)
    __swig_setmethods__["y"] = _cspace.csEventJoystickData_y_set
    __swig_getmethods__["y"] = _cspace.csEventJoystickData_y_get
    if _newclass:y = property(_cspace.csEventJoystickData_y_get, _cspace.csEventJoystickData_y_set)
    __swig_setmethods__["Button"] = _cspace.csEventJoystickData_Button_set
    __swig_getmethods__["Button"] = _cspace.csEventJoystickData_Button_get
    if _newclass:Button = property(_cspace.csEventJoystickData_Button_get, _cspace.csEventJoystickData_Button_set)
    __swig_setmethods__["Modifiers"] = _cspace.csEventJoystickData_Modifiers_set
    __swig_getmethods__["Modifiers"] = _cspace.csEventJoystickData_Modifiers_get
    if _newclass:Modifiers = property(_cspace.csEventJoystickData_Modifiers_get, _cspace.csEventJoystickData_Modifiers_set)
    def __init__(self, *args):
        _swig_setattr(self, csEventJoystickData, 'this', _cspace.new_csEventJoystickData(*args))
        _swig_setattr(self, csEventJoystickData, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csEventJoystickData):
        try:
            if self.thisown: destroy(self)
        except: pass

class csEventJoystickDataPtr(csEventJoystickData):
    def __init__(self, this):
        _swig_setattr(self, csEventJoystickData, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csEventJoystickData, 'thisown', 0)
        _swig_setattr(self, csEventJoystickData,self.__class__,csEventJoystickData)
_cspace.csEventJoystickData_swigregister(csEventJoystickDataPtr)

class csEventCommandData(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csEventCommandData, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csEventCommandData, name)
    def __repr__(self):
        return "<C csEventCommandData instance at %s>" % (self.this,)
    __swig_setmethods__["Code"] = _cspace.csEventCommandData_Code_set
    __swig_getmethods__["Code"] = _cspace.csEventCommandData_Code_get
    if _newclass:Code = property(_cspace.csEventCommandData_Code_get, _cspace.csEventCommandData_Code_set)
    __swig_setmethods__["Info"] = _cspace.csEventCommandData_Info_set
    __swig_getmethods__["Info"] = _cspace.csEventCommandData_Info_get
    if _newclass:Info = property(_cspace.csEventCommandData_Info_get, _cspace.csEventCommandData_Info_set)
    def __init__(self, *args):
        _swig_setattr(self, csEventCommandData, 'this', _cspace.new_csEventCommandData(*args))
        _swig_setattr(self, csEventCommandData, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csEventCommandData):
        try:
            if self.thisown: destroy(self)
        except: pass

class csEventCommandDataPtr(csEventCommandData):
    def __init__(self, this):
        _swig_setattr(self, csEventCommandData, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csEventCommandData, 'thisown', 0)
        _swig_setattr(self, csEventCommandData,self.__class__,csEventCommandData)
_cspace.csEventCommandData_swigregister(csEventCommandDataPtr)

csEventErrNone = _cspace.csEventErrNone
csEventErrLossy = _cspace.csEventErrLossy
csEventErrNotFound = _cspace.csEventErrNotFound
csEventErrMismatchInt = _cspace.csEventErrMismatchInt
csEventErrMismatchUInt = _cspace.csEventErrMismatchUInt
csEventErrMismatchFloat = _cspace.csEventErrMismatchFloat
csEventErrMismatchBuffer = _cspace.csEventErrMismatchBuffer
csEventErrMismatchEvent = _cspace.csEventErrMismatchEvent
csEventErrMismatchIBase = _cspace.csEventErrMismatchIBase
csEventErrUhOhUnknown = _cspace.csEventErrUhOhUnknown
csEventAttrUnknown = _cspace.csEventAttrUnknown
csEventAttrInt = _cspace.csEventAttrInt
csEventAttrUInt = _cspace.csEventAttrUInt
csEventAttrFloat = _cspace.csEventAttrFloat
csEventAttrDatabuffer = _cspace.csEventAttrDatabuffer
csEventAttrEvent = _cspace.csEventAttrEvent
csEventAttriBase = _cspace.csEventAttriBase
class iEvent(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEvent, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEvent, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEvent instance at %s>" % (self.this,)
    __swig_setmethods__["Type"] = _cspace.iEvent_Type_set
    __swig_getmethods__["Type"] = _cspace.iEvent_Type_get
    if _newclass:Type = property(_cspace.iEvent_Type_get, _cspace.iEvent_Type_set)
    __swig_setmethods__["Category"] = _cspace.iEvent_Category_set
    __swig_getmethods__["Category"] = _cspace.iEvent_Category_get
    if _newclass:Category = property(_cspace.iEvent_Category_get, _cspace.iEvent_Category_set)
    __swig_setmethods__["SubCategory"] = _cspace.iEvent_SubCategory_set
    __swig_getmethods__["SubCategory"] = _cspace.iEvent_SubCategory_get
    if _newclass:SubCategory = property(_cspace.iEvent_SubCategory_get, _cspace.iEvent_SubCategory_set)
    __swig_setmethods__["Flags"] = _cspace.iEvent_Flags_set
    __swig_getmethods__["Flags"] = _cspace.iEvent_Flags_get
    if _newclass:Flags = property(_cspace.iEvent_Flags_get, _cspace.iEvent_Flags_set)
    __swig_setmethods__["Time"] = _cspace.iEvent_Time_set
    __swig_getmethods__["Time"] = _cspace.iEvent_Time_get
    if _newclass:Time = property(_cspace.iEvent_Time_get, _cspace.iEvent_Time_set)
    def AddInt8(*args): return _cspace.iEvent_AddInt8(*args)
    def AddUInt8(*args): return _cspace.iEvent_AddUInt8(*args)
    def AddInt16(*args): return _cspace.iEvent_AddInt16(*args)
    def AddUInt16(*args): return _cspace.iEvent_AddUInt16(*args)
    def AddInt32(*args): return _cspace.iEvent_AddInt32(*args)
    def AddUInt32(*args): return _cspace.iEvent_AddUInt32(*args)
    def AddFloat(*args): return _cspace.iEvent_AddFloat(*args)
    def AddDouble(*args): return _cspace.iEvent_AddDouble(*args)
    def AddBool(*args): return _cspace.iEvent_AddBool(*args)
    def Add(*args): return _cspace.iEvent_Add(*args)
    def RetrieveInt8(*args): return _cspace.iEvent_RetrieveInt8(*args)
    def RetrieveUInt8(*args): return _cspace.iEvent_RetrieveUInt8(*args)
    def RetrieveInt16(*args): return _cspace.iEvent_RetrieveInt16(*args)
    def RetrieveUInt16(*args): return _cspace.iEvent_RetrieveUInt16(*args)
    def RetrieveUInt32(*args): return _cspace.iEvent_RetrieveUInt32(*args)
    def RetrieveFloat(*args): return _cspace.iEvent_RetrieveFloat(*args)
    def RetrieveDouble(*args): return _cspace.iEvent_RetrieveDouble(*args)
    def RetrieveBool(*args): return _cspace.iEvent_RetrieveBool(*args)
    def Retrieve(*args): return _cspace.iEvent_Retrieve(*args)
    def AttributeExists(*args): return _cspace.iEvent_AttributeExists(*args)
    def GetAttributeType(*args): return _cspace.iEvent_GetAttributeType(*args)
    def Remove(*args): return _cspace.iEvent_Remove(*args)
    def RemoveAll(*args): return _cspace.iEvent_RemoveAll(*args)
    def GetAttributeIterator(*args): return _cspace.iEvent_GetAttributeIterator(*args)
    def __del__(self, destroy=_cspace.delete_iEvent):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iEvent_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iEvent_scfGetVersion)
    __swig_getmethods__["Mouse"] = _cspace.iEvent_Mouse_get
    if _newclass:Mouse = property(_cspace.iEvent_Mouse_get)
    __swig_getmethods__["Joystick"] = _cspace.iEvent_Joystick_get
    if _newclass:Joystick = property(_cspace.iEvent_Joystick_get)
    __swig_getmethods__["Command"] = _cspace.iEvent_Command_get
    if _newclass:Command = property(_cspace.iEvent_Command_get)

class iEventPtr(iEvent):
    def __init__(self, this):
        _swig_setattr(self, iEvent, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEvent, 'thisown', 0)
        _swig_setattr(self, iEvent,self.__class__,iEvent)
_cspace.iEvent_swigregister(iEventPtr)

iEvent_scfGetVersion = _cspace.iEvent_scfGetVersion

class iEventPlug(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEventPlug, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEventPlug, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEventPlug instance at %s>" % (self.this,)
    def GetPotentiallyConflictingEvents(*args): return _cspace.iEventPlug_GetPotentiallyConflictingEvents(*args)
    def QueryEventPriority(*args): return _cspace.iEventPlug_QueryEventPriority(*args)
    def EnableEvents(*args): return _cspace.iEventPlug_EnableEvents(*args)
    def __del__(self, destroy=_cspace.delete_iEventPlug):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEventPlugPtr(iEventPlug):
    def __init__(self, this):
        _swig_setattr(self, iEventPlug, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEventPlug, 'thisown', 0)
        _swig_setattr(self, iEventPlug,self.__class__,iEventPlug)
_cspace.iEventPlug_swigregister(iEventPlugPtr)

class iEventOutlet(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEventOutlet, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEventOutlet, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEventOutlet instance at %s>" % (self.this,)
    def CreateEvent(*args): return _cspace.iEventOutlet_CreateEvent(*args)
    def Post(*args): return _cspace.iEventOutlet_Post(*args)
    def Key(*args): return _cspace.iEventOutlet_Key(*args)
    def Mouse(*args): return _cspace.iEventOutlet_Mouse(*args)
    def Joystick(*args): return _cspace.iEventOutlet_Joystick(*args)
    def Broadcast(*args): return _cspace.iEventOutlet_Broadcast(*args)
    def ImmediateBroadcast(*args): return _cspace.iEventOutlet_ImmediateBroadcast(*args)
    def __del__(self, destroy=_cspace.delete_iEventOutlet):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEventOutletPtr(iEventOutlet):
    def __init__(self, this):
        _swig_setattr(self, iEventOutlet, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEventOutlet, 'thisown', 0)
        _swig_setattr(self, iEventOutlet,self.__class__,iEventOutlet)
_cspace.iEventOutlet_swigregister(iEventOutletPtr)

class iEventCord(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEventCord, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEventCord, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEventCord instance at %s>" % (self.this,)
    def Insert(*args): return _cspace.iEventCord_Insert(*args)
    def Remove(*args): return _cspace.iEventCord_Remove(*args)
    def GetPass(*args): return _cspace.iEventCord_GetPass(*args)
    def SetPass(*args): return _cspace.iEventCord_SetPass(*args)
    def GetCategory(*args): return _cspace.iEventCord_GetCategory(*args)
    def GetSubcategory(*args): return _cspace.iEventCord_GetSubcategory(*args)
    def __del__(self, destroy=_cspace.delete_iEventCord):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEventCordPtr(iEventCord):
    def __init__(self, this):
        _swig_setattr(self, iEventCord, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEventCord, 'thisown', 0)
        _swig_setattr(self, iEventCord,self.__class__,iEventCord)
_cspace.iEventCord_swigregister(iEventCordPtr)

class csKeyEventHelper(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csKeyEventHelper, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csKeyEventHelper, name)
    def __repr__(self):
        return "<C csKeyEventHelper instance at %s>" % (self.this,)
    __swig_getmethods__["GetRawCode"] = lambda x: _cspace.csKeyEventHelper_GetRawCode
    if _newclass:GetRawCode = staticmethod(_cspace.csKeyEventHelper_GetRawCode)
    __swig_getmethods__["GetCookedCode"] = lambda x: _cspace.csKeyEventHelper_GetCookedCode
    if _newclass:GetCookedCode = staticmethod(_cspace.csKeyEventHelper_GetCookedCode)
    __swig_getmethods__["GetModifiers"] = lambda x: _cspace.csKeyEventHelper_GetModifiers
    if _newclass:GetModifiers = staticmethod(_cspace.csKeyEventHelper_GetModifiers)
    __swig_getmethods__["GetEventType"] = lambda x: _cspace.csKeyEventHelper_GetEventType
    if _newclass:GetEventType = staticmethod(_cspace.csKeyEventHelper_GetEventType)
    __swig_getmethods__["GetAutoRepeat"] = lambda x: _cspace.csKeyEventHelper_GetAutoRepeat
    if _newclass:GetAutoRepeat = staticmethod(_cspace.csKeyEventHelper_GetAutoRepeat)
    __swig_getmethods__["GetCharacterType"] = lambda x: _cspace.csKeyEventHelper_GetCharacterType
    if _newclass:GetCharacterType = staticmethod(_cspace.csKeyEventHelper_GetCharacterType)
    __swig_getmethods__["GetEventData"] = lambda x: _cspace.csKeyEventHelper_GetEventData
    if _newclass:GetEventData = staticmethod(_cspace.csKeyEventHelper_GetEventData)
    __swig_getmethods__["GetModifiersBits"] = lambda x: _cspace.csKeyEventHelper_GetModifiersBits
    if _newclass:GetModifiersBits = staticmethod(_cspace.csKeyEventHelper_GetModifiersBits)
    __swig_getmethods__["GetModifiersBits"] = lambda x: _cspace.csKeyEventHelper_GetModifiersBits
    if _newclass:GetModifiersBits = staticmethod(_cspace.csKeyEventHelper_GetModifiersBits)
    __swig_getmethods__["GetModifiers"] = lambda x: _cspace.csKeyEventHelper_GetModifiers
    if _newclass:GetModifiers = staticmethod(_cspace.csKeyEventHelper_GetModifiers)
    def __init__(self, *args):
        _swig_setattr(self, csKeyEventHelper, 'this', _cspace.new_csKeyEventHelper(*args))
        _swig_setattr(self, csKeyEventHelper, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csKeyEventHelper):
        try:
            if self.thisown: destroy(self)
        except: pass

class csKeyEventHelperPtr(csKeyEventHelper):
    def __init__(self, this):
        _swig_setattr(self, csKeyEventHelper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csKeyEventHelper, 'thisown', 0)
        _swig_setattr(self, csKeyEventHelper,self.__class__,csKeyEventHelper)
_cspace.csKeyEventHelper_swigregister(csKeyEventHelperPtr)

csKeyEventHelper_GetRawCode = _cspace.csKeyEventHelper_GetRawCode

csKeyEventHelper_GetCookedCode = _cspace.csKeyEventHelper_GetCookedCode

csKeyEventHelper_GetEventType = _cspace.csKeyEventHelper_GetEventType

csKeyEventHelper_GetAutoRepeat = _cspace.csKeyEventHelper_GetAutoRepeat

csKeyEventHelper_GetCharacterType = _cspace.csKeyEventHelper_GetCharacterType

csKeyEventHelper_GetEventData = _cspace.csKeyEventHelper_GetEventData

csKeyEventHelper_GetModifiersBits = _cspace.csKeyEventHelper_GetModifiersBits

csKeyEventHelper_GetModifiers = _cspace.csKeyEventHelper_GetModifiers

csevNothing = _cspace.csevNothing
csevKeyboard = _cspace.csevKeyboard
csevMouseMove = _cspace.csevMouseMove
csevMouseDown = _cspace.csevMouseDown
csevMouseUp = _cspace.csevMouseUp
csevMouseClick = _cspace.csevMouseClick
csevMouseDoubleClick = _cspace.csevMouseDoubleClick
csevJoystickMove = _cspace.csevJoystickMove
csevJoystickDown = _cspace.csevJoystickDown
csevJoystickUp = _cspace.csevJoystickUp
csevCommand = _cspace.csevCommand
csevBroadcast = _cspace.csevBroadcast
csevMouseEnter = _cspace.csevMouseEnter
csevMouseExit = _cspace.csevMouseExit
csevLostFocus = _cspace.csevLostFocus
csevGainFocus = _cspace.csevGainFocus
csevGroupOff = _cspace.csevGroupOff
csevFrameStart = _cspace.csevFrameStart
csKeyEventTypeUp = _cspace.csKeyEventTypeUp
csKeyEventTypeDown = _cspace.csKeyEventTypeDown
CSEF_BROADCAST = _cspace.CSEF_BROADCAST
csKeyModifierTypeShift = _cspace.csKeyModifierTypeShift
csKeyModifierTypeCtrl = _cspace.csKeyModifierTypeCtrl
csKeyModifierTypeAlt = _cspace.csKeyModifierTypeAlt
csKeyModifierTypeCapsLock = _cspace.csKeyModifierTypeCapsLock
csKeyModifierTypeNumLock = _cspace.csKeyModifierTypeNumLock
csKeyModifierTypeScrollLock = _cspace.csKeyModifierTypeScrollLock
csKeyModifierTypeLast = _cspace.csKeyModifierTypeLast
csKeyModifierNumLeft = _cspace.csKeyModifierNumLeft
csKeyModifierNumRight = _cspace.csKeyModifierNumRight
csKeyModifierNumAny = _cspace.csKeyModifierNumAny
class csKeyModifiers(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csKeyModifiers, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csKeyModifiers, name)
    def __repr__(self):
        return "<C csKeyModifiers instance at %s>" % (self.this,)
    __swig_setmethods__["modifiers"] = _cspace.csKeyModifiers_modifiers_set
    __swig_getmethods__["modifiers"] = _cspace.csKeyModifiers_modifiers_get
    if _newclass:modifiers = property(_cspace.csKeyModifiers_modifiers_get, _cspace.csKeyModifiers_modifiers_set)
    def __init__(self, *args):
        _swig_setattr(self, csKeyModifiers, 'this', _cspace.new_csKeyModifiers(*args))
        _swig_setattr(self, csKeyModifiers, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csKeyModifiers):
        try:
            if self.thisown: destroy(self)
        except: pass

class csKeyModifiersPtr(csKeyModifiers):
    def __init__(self, this):
        _swig_setattr(self, csKeyModifiers, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csKeyModifiers, 'thisown', 0)
        _swig_setattr(self, csKeyModifiers,self.__class__,csKeyModifiers)
_cspace.csKeyModifiers_swigregister(csKeyModifiersPtr)

CSKEY_ESC = _cspace.CSKEY_ESC
CSKEY_ENTER = _cspace.CSKEY_ENTER
CSKEY_TAB = _cspace.CSKEY_TAB
CSKEY_BACKSPACE = _cspace.CSKEY_BACKSPACE
CSKEY_SPACE = _cspace.CSKEY_SPACE
CSKEY_SPECIAL_FIRST = _cspace.CSKEY_SPECIAL_FIRST
CSKEY_SPECIAL_LAST = _cspace.CSKEY_SPECIAL_LAST
CSKEY_UP = _cspace.CSKEY_UP
CSKEY_DOWN = _cspace.CSKEY_DOWN
CSKEY_LEFT = _cspace.CSKEY_LEFT
CSKEY_RIGHT = _cspace.CSKEY_RIGHT
CSKEY_PGUP = _cspace.CSKEY_PGUP
CSKEY_PGDN = _cspace.CSKEY_PGDN
CSKEY_HOME = _cspace.CSKEY_HOME
CSKEY_END = _cspace.CSKEY_END
CSKEY_INS = _cspace.CSKEY_INS
CSKEY_DEL = _cspace.CSKEY_DEL
CSKEY_CONTEXT = _cspace.CSKEY_CONTEXT
CSKEY_PRINTSCREEN = _cspace.CSKEY_PRINTSCREEN
CSKEY_PAUSE = _cspace.CSKEY_PAUSE
CSKEY_F1 = _cspace.CSKEY_F1
CSKEY_F2 = _cspace.CSKEY_F2
CSKEY_F3 = _cspace.CSKEY_F3
CSKEY_F4 = _cspace.CSKEY_F4
CSKEY_F5 = _cspace.CSKEY_F5
CSKEY_F6 = _cspace.CSKEY_F6
CSKEY_F7 = _cspace.CSKEY_F7
CSKEY_F8 = _cspace.CSKEY_F8
CSKEY_F9 = _cspace.CSKEY_F9
CSKEY_F10 = _cspace.CSKEY_F10
CSKEY_F11 = _cspace.CSKEY_F11
CSKEY_F12 = _cspace.CSKEY_F12
CSKEY_MODIFIER_FIRST = _cspace.CSKEY_MODIFIER_FIRST
CSKEY_MODIFIER_LAST = _cspace.CSKEY_MODIFIER_LAST
CSKEY_MODIFIERTYPE_SHIFT = _cspace.CSKEY_MODIFIERTYPE_SHIFT
CSKEY_PAD_FLAG = _cspace.CSKEY_PAD_FLAG
csKeyCharTypeNormal = _cspace.csKeyCharTypeNormal
csKeyCharTypeDead = _cspace.csKeyCharTypeDead
CSEVTYPE_Keyboard = _cspace.CSEVTYPE_Keyboard
CSEVTYPE_Mouse = _cspace.CSEVTYPE_Mouse
CSEVTYPE_Joystick = _cspace.CSEVTYPE_Joystick
cscmdNothing = _cspace.cscmdNothing
cscmdQuit = _cspace.cscmdQuit
cscmdFocusChanged = _cspace.cscmdFocusChanged
cscmdSystemOpen = _cspace.cscmdSystemOpen
cscmdSystemClose = _cspace.cscmdSystemClose
cscmdContextResize = _cspace.cscmdContextResize
cscmdContextClose = _cspace.cscmdContextClose
cscmdCommandLineHelp = _cspace.cscmdCommandLineHelp
cscmdPreProcess = _cspace.cscmdPreProcess
cscmdProcess = _cspace.cscmdProcess
cscmdPostProcess = _cspace.cscmdPostProcess
cscmdFinalProcess = _cspace.cscmdFinalProcess
cscmdCanvasHidden = _cspace.cscmdCanvasHidden
cscmdCanvasExposed = _cspace.cscmdCanvasExposed
class iEventQueue(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEventQueue, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEventQueue, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEventQueue instance at %s>" % (self.this,)
    def Process(*args): return _cspace.iEventQueue_Process(*args)
    def Dispatch(*args): return _cspace.iEventQueue_Dispatch(*args)
    def RegisterListener(*args): return _cspace.iEventQueue_RegisterListener(*args)
    def RemoveListener(*args): return _cspace.iEventQueue_RemoveListener(*args)
    def ChangeListenerTrigger(*args): return _cspace.iEventQueue_ChangeListenerTrigger(*args)
    def CreateEventOutlet(*args): return _cspace.iEventQueue_CreateEventOutlet(*args)
    def GetEventOutlet(*args): return _cspace.iEventQueue_GetEventOutlet(*args)
    def GetEventCord(*args): return _cspace.iEventQueue_GetEventCord(*args)
    def CreateEvent(*args): return _cspace.iEventQueue_CreateEvent(*args)
    def Post(*args): return _cspace.iEventQueue_Post(*args)
    def Get(*args): return _cspace.iEventQueue_Get(*args)
    def Clear(*args): return _cspace.iEventQueue_Clear(*args)
    def IsEmpty(*args): return _cspace.iEventQueue_IsEmpty(*args)
    def RemoveAllListeners(*args): return _cspace.iEventQueue_RemoveAllListeners(*args)
    def __del__(self, destroy=_cspace.delete_iEventQueue):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iEventQueue_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iEventQueue_scfGetVersion)

class iEventQueuePtr(iEventQueue):
    def __init__(self, this):
        _swig_setattr(self, iEventQueue, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEventQueue, 'thisown', 0)
        _swig_setattr(self, iEventQueue,self.__class__,iEventQueue)
_cspace.iEventQueue_swigregister(iEventQueuePtr)

iEventQueue_scfGetVersion = _cspace.iEventQueue_scfGetVersion

class iEventHandler(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEventHandler, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEventHandler, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEventHandler instance at %s>" % (self.this,)
    def HandleEvent(*args): return _cspace.iEventHandler_HandleEvent(*args)
    def __del__(self, destroy=_cspace.delete_iEventHandler):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iEventHandler_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iEventHandler_scfGetVersion)

class iEventHandlerPtr(iEventHandler):
    def __init__(self, this):
        _swig_setattr(self, iEventHandler, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEventHandler, 'thisown', 0)
        _swig_setattr(self, iEventHandler,self.__class__,iEventHandler)
_cspace.iEventHandler_swigregister(iEventHandlerPtr)

iEventHandler_scfGetVersion = _cspace.iEventHandler_scfGetVersion

class iPluginIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPluginIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPluginIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPluginIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iPluginIterator_HasNext(*args)
    def Next(*args): return _cspace.iPluginIterator_Next(*args)
    def __del__(self, destroy=_cspace.delete_iPluginIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iPluginIteratorPtr(iPluginIterator):
    def __init__(self, this):
        _swig_setattr(self, iPluginIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPluginIterator, 'thisown', 0)
        _swig_setattr(self, iPluginIterator,self.__class__,iPluginIterator)
_cspace.iPluginIterator_swigregister(iPluginIteratorPtr)

class iPluginManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iPluginManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iPluginManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iPluginManager instance at %s>" % (self.this,)
    def LoadPlugin(*args): return _cspace.iPluginManager_LoadPlugin(*args)
    def QueryPlugin(*args): return _cspace.iPluginManager_QueryPlugin(*args)
    def UnloadPlugin(*args): return _cspace.iPluginManager_UnloadPlugin(*args)
    def RegisterPlugin(*args): return _cspace.iPluginManager_RegisterPlugin(*args)
    def GetPlugins(*args): return _cspace.iPluginManager_GetPlugins(*args)
    def Clear(*args): return _cspace.iPluginManager_Clear(*args)
    def QueryOptions(*args): return _cspace.iPluginManager_QueryOptions(*args)
    def __del__(self, destroy=_cspace.delete_iPluginManager):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iPluginManager_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iPluginManager_scfGetVersion)

class iPluginManagerPtr(iPluginManager):
    def __init__(self, this):
        _swig_setattr(self, iPluginManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iPluginManager, 'thisown', 0)
        _swig_setattr(self, iPluginManager,self.__class__,iPluginManager)
_cspace.iPluginManager_swigregister(iPluginManagerPtr)

iPluginManager_scfGetVersion = _cspace.iPluginManager_scfGetVersion

CS_MAX_MOUSE_BUTTONS = _cspace.CS_MAX_MOUSE_BUTTONS
CS_MAX_JOYSTICK_COUNT = _cspace.CS_MAX_JOYSTICK_COUNT
CS_MAX_JOYSTICK_BUTTONS = _cspace.CS_MAX_JOYSTICK_BUTTONS
csComposeNoChar = _cspace.csComposeNoChar
csComposeNormalChar = _cspace.csComposeNormalChar
csComposeComposedChar = _cspace.csComposeComposedChar
csComposeUncomposeable = _cspace.csComposeUncomposeable
class iKeyComposer(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iKeyComposer, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iKeyComposer, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iKeyComposer instance at %s>" % (self.this,)
    def HandleKey(*args): return _cspace.iKeyComposer_HandleKey(*args)
    def ResetState(*args): return _cspace.iKeyComposer_ResetState(*args)
    def __del__(self, destroy=_cspace.delete_iKeyComposer):
        try:
            if self.thisown: destroy(self)
        except: pass

class iKeyComposerPtr(iKeyComposer):
    def __init__(self, this):
        _swig_setattr(self, iKeyComposer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iKeyComposer, 'thisown', 0)
        _swig_setattr(self, iKeyComposer,self.__class__,iKeyComposer)
_cspace.iKeyComposer_swigregister(iKeyComposerPtr)

class iKeyboardDriver(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iKeyboardDriver, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iKeyboardDriver, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iKeyboardDriver instance at %s>" % (self.this,)
    def Reset(*args): return _cspace.iKeyboardDriver_Reset(*args)
    def DoKey(*args): return _cspace.iKeyboardDriver_DoKey(*args)
    def GetModifierState(*args): return _cspace.iKeyboardDriver_GetModifierState(*args)
    def CreateKeyComposer(*args): return _cspace.iKeyboardDriver_CreateKeyComposer(*args)
    def SynthesizeCooked(*args): return _cspace.iKeyboardDriver_SynthesizeCooked(*args)
    def __del__(self, destroy=_cspace.delete_iKeyboardDriver):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iKeyboardDriver_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iKeyboardDriver_scfGetVersion)
    def GetKeyState(*args): return _cspace.iKeyboardDriver_GetKeyState(*args)

class iKeyboardDriverPtr(iKeyboardDriver):
    def __init__(self, this):
        _swig_setattr(self, iKeyboardDriver, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iKeyboardDriver, 'thisown', 0)
        _swig_setattr(self, iKeyboardDriver,self.__class__,iKeyboardDriver)
_cspace.iKeyboardDriver_swigregister(iKeyboardDriverPtr)

iKeyboardDriver_scfGetVersion = _cspace.iKeyboardDriver_scfGetVersion

class iMouseDriver(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMouseDriver, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMouseDriver, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMouseDriver instance at %s>" % (self.this,)
    def SetDoubleClickTime(*args): return _cspace.iMouseDriver_SetDoubleClickTime(*args)
    def Reset(*args): return _cspace.iMouseDriver_Reset(*args)
    def GetLastX(*args): return _cspace.iMouseDriver_GetLastX(*args)
    def GetLastY(*args): return _cspace.iMouseDriver_GetLastY(*args)
    def GetLastButton(*args): return _cspace.iMouseDriver_GetLastButton(*args)
    def DoButton(*args): return _cspace.iMouseDriver_DoButton(*args)
    def DoMotion(*args): return _cspace.iMouseDriver_DoMotion(*args)
    def __del__(self, destroy=_cspace.delete_iMouseDriver):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMouseDriver_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMouseDriver_scfGetVersion)

class iMouseDriverPtr(iMouseDriver):
    def __init__(self, this):
        _swig_setattr(self, iMouseDriver, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMouseDriver, 'thisown', 0)
        _swig_setattr(self, iMouseDriver,self.__class__,iMouseDriver)
_cspace.iMouseDriver_swigregister(iMouseDriverPtr)

iMouseDriver_scfGetVersion = _cspace.iMouseDriver_scfGetVersion

class iJoystickDriver(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iJoystickDriver, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iJoystickDriver, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iJoystickDriver instance at %s>" % (self.this,)
    def Reset(*args): return _cspace.iJoystickDriver_Reset(*args)
    def GetLastX(*args): return _cspace.iJoystickDriver_GetLastX(*args)
    def GetLastY(*args): return _cspace.iJoystickDriver_GetLastY(*args)
    def GetLastButton(*args): return _cspace.iJoystickDriver_GetLastButton(*args)
    def DoButton(*args): return _cspace.iJoystickDriver_DoButton(*args)
    def DoMotion(*args): return _cspace.iJoystickDriver_DoMotion(*args)
    def __del__(self, destroy=_cspace.delete_iJoystickDriver):
        try:
            if self.thisown: destroy(self)
        except: pass

class iJoystickDriverPtr(iJoystickDriver):
    def __init__(self, this):
        _swig_setattr(self, iJoystickDriver, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iJoystickDriver, 'thisown', 0)
        _swig_setattr(self, iJoystickDriver,self.__class__,iJoystickDriver)
_cspace.iJoystickDriver_swigregister(iJoystickDriverPtr)

class iConfigFile(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iConfigFile, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iConfigFile, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iConfigFile instance at %s>" % (self.this,)
    def GetFileName(*args): return _cspace.iConfigFile_GetFileName(*args)
    def GetVFS(*args): return _cspace.iConfigFile_GetVFS(*args)
    def SetFileName(*args): return _cspace.iConfigFile_SetFileName(*args)
    def Load(*args): return _cspace.iConfigFile_Load(*args)
    def Save(*args): return _cspace.iConfigFile_Save(*args)
    def Clear(*args): return _cspace.iConfigFile_Clear(*args)
    def Enumerate(*args): return _cspace.iConfigFile_Enumerate(*args)
    def KeyExists(*args): return _cspace.iConfigFile_KeyExists(*args)
    def SubsectionExists(*args): return _cspace.iConfigFile_SubsectionExists(*args)
    def GetInt(*args): return _cspace.iConfigFile_GetInt(*args)
    def GetFloat(*args): return _cspace.iConfigFile_GetFloat(*args)
    def GetStr(*args): return _cspace.iConfigFile_GetStr(*args)
    def GetBool(*args): return _cspace.iConfigFile_GetBool(*args)
    def GetComment(*args): return _cspace.iConfigFile_GetComment(*args)
    def SetStr(*args): return _cspace.iConfigFile_SetStr(*args)
    def SetInt(*args): return _cspace.iConfigFile_SetInt(*args)
    def SetFloat(*args): return _cspace.iConfigFile_SetFloat(*args)
    def SetBool(*args): return _cspace.iConfigFile_SetBool(*args)
    def SetComment(*args): return _cspace.iConfigFile_SetComment(*args)
    def DeleteKey(*args): return _cspace.iConfigFile_DeleteKey(*args)
    def GetEOFComment(*args): return _cspace.iConfigFile_GetEOFComment(*args)
    def SetEOFComment(*args): return _cspace.iConfigFile_SetEOFComment(*args)
    def __del__(self, destroy=_cspace.delete_iConfigFile):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iConfigFile_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iConfigFile_scfGetVersion)

class iConfigFilePtr(iConfigFile):
    def __init__(self, this):
        _swig_setattr(self, iConfigFile, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iConfigFile, 'thisown', 0)
        _swig_setattr(self, iConfigFile,self.__class__,iConfigFile)
_cspace.iConfigFile_swigregister(iConfigFilePtr)

iConfigFile_scfGetVersion = _cspace.iConfigFile_scfGetVersion

class iConfigIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iConfigIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iConfigIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iConfigIterator instance at %s>" % (self.this,)
    def GetConfigFile(*args): return _cspace.iConfigIterator_GetConfigFile(*args)
    def GetSubsection(*args): return _cspace.iConfigIterator_GetSubsection(*args)
    def Rewind(*args): return _cspace.iConfigIterator_Rewind(*args)
    def Next(*args): return _cspace.iConfigIterator_Next(*args)
    def GetKey(*args): return _cspace.iConfigIterator_GetKey(*args)
    def GetInt(*args): return _cspace.iConfigIterator_GetInt(*args)
    def GetFloat(*args): return _cspace.iConfigIterator_GetFloat(*args)
    def GetStr(*args): return _cspace.iConfigIterator_GetStr(*args)
    def GetBool(*args): return _cspace.iConfigIterator_GetBool(*args)
    def GetComment(*args): return _cspace.iConfigIterator_GetComment(*args)
    def __del__(self, destroy=_cspace.delete_iConfigIterator):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iConfigIterator_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iConfigIterator_scfGetVersion)

class iConfigIteratorPtr(iConfigIterator):
    def __init__(self, this):
        _swig_setattr(self, iConfigIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iConfigIterator, 'thisown', 0)
        _swig_setattr(self, iConfigIterator,self.__class__,iConfigIterator)
_cspace.iConfigIterator_swigregister(iConfigIteratorPtr)

iConfigIterator_scfGetVersion = _cspace.iConfigIterator_scfGetVersion

class iConfigManager(iConfigFile):
    __swig_setmethods__ = {}
    for _s in [iConfigFile]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iConfigManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iConfigFile]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iConfigManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iConfigManager instance at %s>" % (self.this,)
    PriorityMin = _cspace.iConfigManager_PriorityMin
    PriorityVeryLow = _cspace.iConfigManager_PriorityVeryLow
    PriorityLow = _cspace.iConfigManager_PriorityLow
    PriorityMedium = _cspace.iConfigManager_PriorityMedium
    PriorityHigh = _cspace.iConfigManager_PriorityHigh
    PriorityVeryHigh = _cspace.iConfigManager_PriorityVeryHigh
    PriorityMax = _cspace.iConfigManager_PriorityMax
    ConfigPriorityPlugin = _cspace.iConfigManager_ConfigPriorityPlugin
    ConfigPriorityApplication = _cspace.iConfigManager_ConfigPriorityApplication
    ConfigPriorityUserGlobal = _cspace.iConfigManager_ConfigPriorityUserGlobal
    ConfigPriorityUserApp = _cspace.iConfigManager_ConfigPriorityUserApp
    ConfigPriorityCmdLine = _cspace.iConfigManager_ConfigPriorityCmdLine
    def AddDomain(*args): return _cspace.iConfigManager_AddDomain(*args)
    def RemoveDomain(*args): return _cspace.iConfigManager_RemoveDomain(*args)
    def LookupDomain(*args): return _cspace.iConfigManager_LookupDomain(*args)
    def SetDomainPriority(*args): return _cspace.iConfigManager_SetDomainPriority(*args)
    def GetDomainPriority(*args): return _cspace.iConfigManager_GetDomainPriority(*args)
    def SetDynamicDomain(*args): return _cspace.iConfigManager_SetDynamicDomain(*args)
    def GetDynamicDomain(*args): return _cspace.iConfigManager_GetDynamicDomain(*args)
    def SetDynamicDomainPriority(*args): return _cspace.iConfigManager_SetDynamicDomainPriority(*args)
    def GetDynamicDomainPriority(*args): return _cspace.iConfigManager_GetDynamicDomainPriority(*args)
    def FlushRemoved(*args): return _cspace.iConfigManager_FlushRemoved(*args)
    def __del__(self, destroy=_cspace.delete_iConfigManager):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iConfigManager_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iConfigManager_scfGetVersion)

class iConfigManagerPtr(iConfigManager):
    def __init__(self, this):
        _swig_setattr(self, iConfigManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iConfigManager, 'thisown', 0)
        _swig_setattr(self, iConfigManager,self.__class__,iConfigManager)
_cspace.iConfigManager_swigregister(iConfigManagerPtr)

iConfigManager_scfGetVersion = _cspace.iConfigManager_scfGetVersion

class iStringArray(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iStringArray, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iStringArray, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iStringArray instance at %s>" % (self.this,)
    def Length(*args): return _cspace.iStringArray_Length(*args)
    def Push(*args): return _cspace.iStringArray_Push(*args)
    def Pop(*args): return _cspace.iStringArray_Pop(*args)
    def Get(*args): return _cspace.iStringArray_Get(*args)
    def Find(*args): return _cspace.iStringArray_Find(*args)
    def FindCaseInsensitive(*args): return _cspace.iStringArray_FindCaseInsensitive(*args)
    def FindSortedKey(*args): return _cspace.iStringArray_FindSortedKey(*args)
    def Sort(*args): return _cspace.iStringArray_Sort(*args)
    def DeleteIndex(*args): return _cspace.iStringArray_DeleteIndex(*args)
    def Insert(*args): return _cspace.iStringArray_Insert(*args)
    def DeleteAll(*args): return _cspace.iStringArray_DeleteAll(*args)
    def __del__(self, destroy=_cspace.delete_iStringArray):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iStringArray_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iStringArray_scfGetVersion)

class iStringArrayPtr(iStringArray):
    def __init__(self, this):
        _swig_setattr(self, iStringArray, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iStringArray, 'thisown', 0)
        _swig_setattr(self, iStringArray,self.__class__,iStringArray)
_cspace.iStringArray_swigregister(iStringArrayPtr)

iStringArray_scfGetVersion = _cspace.iStringArray_scfGetVersion

CS_NODE_DOCUMENT = _cspace.CS_NODE_DOCUMENT
CS_NODE_ELEMENT = _cspace.CS_NODE_ELEMENT
CS_NODE_COMMENT = _cspace.CS_NODE_COMMENT
CS_NODE_UNKNOWN = _cspace.CS_NODE_UNKNOWN
CS_NODE_TEXT = _cspace.CS_NODE_TEXT
CS_NODE_DECLARATION = _cspace.CS_NODE_DECLARATION
CS_CHANGEABLE_NEVER = _cspace.CS_CHANGEABLE_NEVER
CS_CHANGEABLE_NEWROOT = _cspace.CS_CHANGEABLE_NEWROOT
CS_CHANGEABLE_YES = _cspace.CS_CHANGEABLE_YES
class iDocumentAttributeIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDocumentAttributeIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDocumentAttributeIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDocumentAttributeIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iDocumentAttributeIterator_HasNext(*args)
    def Next(*args): return _cspace.iDocumentAttributeIterator_Next(*args)
    def __del__(self, destroy=_cspace.delete_iDocumentAttributeIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iDocumentAttributeIteratorPtr(iDocumentAttributeIterator):
    def __init__(self, this):
        _swig_setattr(self, iDocumentAttributeIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDocumentAttributeIterator, 'thisown', 0)
        _swig_setattr(self, iDocumentAttributeIterator,self.__class__,iDocumentAttributeIterator)
_cspace.iDocumentAttributeIterator_swigregister(iDocumentAttributeIteratorPtr)

class iDocumentAttribute(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDocumentAttribute, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDocumentAttribute, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDocumentAttribute instance at %s>" % (self.this,)
    def GetName(*args): return _cspace.iDocumentAttribute_GetName(*args)
    def GetValue(*args): return _cspace.iDocumentAttribute_GetValue(*args)
    def GetValueAsInt(*args): return _cspace.iDocumentAttribute_GetValueAsInt(*args)
    def GetValueAsFloat(*args): return _cspace.iDocumentAttribute_GetValueAsFloat(*args)
    def GetValueAsBool(*args): return _cspace.iDocumentAttribute_GetValueAsBool(*args)
    def SetName(*args): return _cspace.iDocumentAttribute_SetName(*args)
    def SetValue(*args): return _cspace.iDocumentAttribute_SetValue(*args)
    def SetValueAsInt(*args): return _cspace.iDocumentAttribute_SetValueAsInt(*args)
    def SetValueAsFloat(*args): return _cspace.iDocumentAttribute_SetValueAsFloat(*args)
    def __del__(self, destroy=_cspace.delete_iDocumentAttribute):
        try:
            if self.thisown: destroy(self)
        except: pass

class iDocumentAttributePtr(iDocumentAttribute):
    def __init__(self, this):
        _swig_setattr(self, iDocumentAttribute, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDocumentAttribute, 'thisown', 0)
        _swig_setattr(self, iDocumentAttribute,self.__class__,iDocumentAttribute)
_cspace.iDocumentAttribute_swigregister(iDocumentAttributePtr)

class iDocumentNodeIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDocumentNodeIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDocumentNodeIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDocumentNodeIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iDocumentNodeIterator_HasNext(*args)
    def Next(*args): return _cspace.iDocumentNodeIterator_Next(*args)
    def __del__(self, destroy=_cspace.delete_iDocumentNodeIterator):
        try:
            if self.thisown: destroy(self)
        except: pass

class iDocumentNodeIteratorPtr(iDocumentNodeIterator):
    def __init__(self, this):
        _swig_setattr(self, iDocumentNodeIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDocumentNodeIterator, 'thisown', 0)
        _swig_setattr(self, iDocumentNodeIterator,self.__class__,iDocumentNodeIterator)
_cspace.iDocumentNodeIterator_swigregister(iDocumentNodeIteratorPtr)

class iDocumentNode(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDocumentNode, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDocumentNode, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDocumentNode instance at %s>" % (self.this,)
    def GetType(*args): return _cspace.iDocumentNode_GetType(*args)
    def Equals(*args): return _cspace.iDocumentNode_Equals(*args)
    def GetValue(*args): return _cspace.iDocumentNode_GetValue(*args)
    def SetValue(*args): return _cspace.iDocumentNode_SetValue(*args)
    def SetValueAsInt(*args): return _cspace.iDocumentNode_SetValueAsInt(*args)
    def SetValueAsFloat(*args): return _cspace.iDocumentNode_SetValueAsFloat(*args)
    def GetParent(*args): return _cspace.iDocumentNode_GetParent(*args)
    def GetNodes(*args): return _cspace.iDocumentNode_GetNodes(*args)
    def GetNode(*args): return _cspace.iDocumentNode_GetNode(*args)
    def RemoveNode(*args): return _cspace.iDocumentNode_RemoveNode(*args)
    def RemoveNodes(*args): return _cspace.iDocumentNode_RemoveNodes(*args)
    def CreateNodeBefore(*args): return _cspace.iDocumentNode_CreateNodeBefore(*args)
    def GetContentsValue(*args): return _cspace.iDocumentNode_GetContentsValue(*args)
    def GetContentsValueAsInt(*args): return _cspace.iDocumentNode_GetContentsValueAsInt(*args)
    def GetContentsValueAsFloat(*args): return _cspace.iDocumentNode_GetContentsValueAsFloat(*args)
    def GetAttributes(*args): return _cspace.iDocumentNode_GetAttributes(*args)
    def GetAttribute(*args): return _cspace.iDocumentNode_GetAttribute(*args)
    def GetAttributeValue(*args): return _cspace.iDocumentNode_GetAttributeValue(*args)
    def GetAttributeValueAsInt(*args): return _cspace.iDocumentNode_GetAttributeValueAsInt(*args)
    def GetAttributeValueAsFloat(*args): return _cspace.iDocumentNode_GetAttributeValueAsFloat(*args)
    def GetAttributeValueAsBool(*args): return _cspace.iDocumentNode_GetAttributeValueAsBool(*args)
    def RemoveAttribute(*args): return _cspace.iDocumentNode_RemoveAttribute(*args)
    def RemoveAttributes(*args): return _cspace.iDocumentNode_RemoveAttributes(*args)
    def SetAttribute(*args): return _cspace.iDocumentNode_SetAttribute(*args)
    def SetAttributeAsInt(*args): return _cspace.iDocumentNode_SetAttributeAsInt(*args)
    def SetAttributeAsFloat(*args): return _cspace.iDocumentNode_SetAttributeAsFloat(*args)
    def __del__(self, destroy=_cspace.delete_iDocumentNode):
        try:
            if self.thisown: destroy(self)
        except: pass

class iDocumentNodePtr(iDocumentNode):
    def __init__(self, this):
        _swig_setattr(self, iDocumentNode, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDocumentNode, 'thisown', 0)
        _swig_setattr(self, iDocumentNode,self.__class__,iDocumentNode)
_cspace.iDocumentNode_swigregister(iDocumentNodePtr)

class iDocument(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDocument, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDocument, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDocument instance at %s>" % (self.this,)
    def Clear(*args): return _cspace.iDocument_Clear(*args)
    def CreateRoot(*args): return _cspace.iDocument_CreateRoot(*args)
    def GetRoot(*args): return _cspace.iDocument_GetRoot(*args)
    def Parse(*args): return _cspace.iDocument_Parse(*args)
    def Write(*args): return _cspace.iDocument_Write(*args)
    def Changeable(*args): return _cspace.iDocument_Changeable(*args)
    def __del__(self, destroy=_cspace.delete_iDocument):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iDocument_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iDocument_scfGetVersion)

class iDocumentPtr(iDocument):
    def __init__(self, this):
        _swig_setattr(self, iDocument, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDocument, 'thisown', 0)
        _swig_setattr(self, iDocument,self.__class__,iDocument)
_cspace.iDocument_swigregister(iDocumentPtr)

iDocument_scfGetVersion = _cspace.iDocument_scfGetVersion

class iDocumentSystem(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDocumentSystem, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDocumentSystem, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDocumentSystem instance at %s>" % (self.this,)
    def CreateDocument(*args): return _cspace.iDocumentSystem_CreateDocument(*args)
    def __del__(self, destroy=_cspace.delete_iDocumentSystem):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iDocumentSystem_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iDocumentSystem_scfGetVersion)

class iDocumentSystemPtr(iDocumentSystem):
    def __init__(self, this):
        _swig_setattr(self, iDocumentSystem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDocumentSystem, 'thisown', 0)
        _swig_setattr(self, iDocumentSystem,self.__class__,iDocumentSystem)
_cspace.iDocumentSystem_swigregister(iDocumentSystemPtr)

iDocumentSystem_scfGetVersion = _cspace.iDocumentSystem_scfGetVersion

class csTinyDocumentSystem(iDocumentSystem):
    __swig_setmethods__ = {}
    for _s in [iDocumentSystem]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csTinyDocumentSystem, name, value)
    __swig_getmethods__ = {}
    for _s in [iDocumentSystem]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csTinyDocumentSystem, name)
    def __repr__(self):
        return "<C csTinyDocumentSystem instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csTinyDocumentSystem, 'this', _cspace.new_csTinyDocumentSystem(*args))
        _swig_setattr(self, csTinyDocumentSystem, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csTinyDocumentSystem):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_setmethods__["scfRefCount"] = _cspace.csTinyDocumentSystem_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace.csTinyDocumentSystem_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace.csTinyDocumentSystem_scfRefCount_get, _cspace.csTinyDocumentSystem_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace.csTinyDocumentSystem_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace.csTinyDocumentSystem_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace.csTinyDocumentSystem_scfWeakRefOwners_get, _cspace.csTinyDocumentSystem_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace.csTinyDocumentSystem_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace.csTinyDocumentSystem_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace.csTinyDocumentSystem_scfParent_get
    if _newclass:scfParent = property(_cspace.csTinyDocumentSystem_scfParent_get, _cspace.csTinyDocumentSystem_scfParent_set)
    def IncRef(*args): return _cspace.csTinyDocumentSystem_IncRef(*args)
    def DecRef(*args): return _cspace.csTinyDocumentSystem_DecRef(*args)
    def GetRefCount(*args): return _cspace.csTinyDocumentSystem_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csTinyDocumentSystem_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csTinyDocumentSystem_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csTinyDocumentSystem_QueryInterface(*args)
    def CreateDocument(*args): return _cspace.csTinyDocumentSystem_CreateDocument(*args)

class csTinyDocumentSystemPtr(csTinyDocumentSystem):
    def __init__(self, this):
        _swig_setattr(self, csTinyDocumentSystem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csTinyDocumentSystem, 'thisown', 0)
        _swig_setattr(self, csTinyDocumentSystem,self.__class__,csTinyDocumentSystem)
_cspace.csTinyDocumentSystem_swigregister(csTinyDocumentSystemPtr)

class iDataBuffer(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDataBuffer, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDataBuffer, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDataBuffer instance at %s>" % (self.this,)
    def GetSize(*args): return _cspace.iDataBuffer_GetSize(*args)
    def GetData(*args): return _cspace.iDataBuffer_GetData(*args)
    def asString(*args): return _cspace.iDataBuffer_asString(*args)
    def GetUint8(*args): return _cspace.iDataBuffer_GetUint8(*args)
    def __del__(self, destroy=_cspace.delete_iDataBuffer):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iDataBuffer_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iDataBuffer_scfGetVersion)

class iDataBufferPtr(iDataBuffer):
    def __init__(self, this):
        _swig_setattr(self, iDataBuffer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDataBuffer, 'thisown', 0)
        _swig_setattr(self, iDataBuffer,self.__class__,iDataBuffer)
_cspace.iDataBuffer_swigregister(iDataBufferPtr)

iDataBuffer_scfGetVersion = _cspace.iDataBuffer_scfGetVersion

CS_WRITE_BASELINE = _cspace.CS_WRITE_BASELINE
CS_WRITE_NOANTIALIAS = _cspace.CS_WRITE_NOANTIALIAS
class csPixelCoord(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPixelCoord, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPixelCoord, name)
    def __repr__(self):
        return "<C csPixelCoord instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cspace.csPixelCoord_x_set
    __swig_getmethods__["x"] = _cspace.csPixelCoord_x_get
    if _newclass:x = property(_cspace.csPixelCoord_x_get, _cspace.csPixelCoord_x_set)
    __swig_setmethods__["y"] = _cspace.csPixelCoord_y_set
    __swig_getmethods__["y"] = _cspace.csPixelCoord_y_get
    if _newclass:y = property(_cspace.csPixelCoord_y_get, _cspace.csPixelCoord_y_set)
    def __init__(self, *args):
        _swig_setattr(self, csPixelCoord, 'this', _cspace.new_csPixelCoord(*args))
        _swig_setattr(self, csPixelCoord, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPixelCoord):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPixelCoordPtr(csPixelCoord):
    def __init__(self, this):
        _swig_setattr(self, csPixelCoord, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPixelCoord, 'thisown', 0)
        _swig_setattr(self, csPixelCoord,self.__class__,csPixelCoord)
_cspace.csPixelCoord_swigregister(csPixelCoordPtr)

class csPixelFormat(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPixelFormat, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPixelFormat, name)
    def __repr__(self):
        return "<C csPixelFormat instance at %s>" % (self.this,)
    __swig_setmethods__["RedMask"] = _cspace.csPixelFormat_RedMask_set
    __swig_getmethods__["RedMask"] = _cspace.csPixelFormat_RedMask_get
    if _newclass:RedMask = property(_cspace.csPixelFormat_RedMask_get, _cspace.csPixelFormat_RedMask_set)
    __swig_setmethods__["GreenMask"] = _cspace.csPixelFormat_GreenMask_set
    __swig_getmethods__["GreenMask"] = _cspace.csPixelFormat_GreenMask_get
    if _newclass:GreenMask = property(_cspace.csPixelFormat_GreenMask_get, _cspace.csPixelFormat_GreenMask_set)
    __swig_setmethods__["BlueMask"] = _cspace.csPixelFormat_BlueMask_set
    __swig_getmethods__["BlueMask"] = _cspace.csPixelFormat_BlueMask_get
    if _newclass:BlueMask = property(_cspace.csPixelFormat_BlueMask_get, _cspace.csPixelFormat_BlueMask_set)
    __swig_setmethods__["AlphaMask"] = _cspace.csPixelFormat_AlphaMask_set
    __swig_getmethods__["AlphaMask"] = _cspace.csPixelFormat_AlphaMask_get
    if _newclass:AlphaMask = property(_cspace.csPixelFormat_AlphaMask_get, _cspace.csPixelFormat_AlphaMask_set)
    __swig_setmethods__["RedShift"] = _cspace.csPixelFormat_RedShift_set
    __swig_getmethods__["RedShift"] = _cspace.csPixelFormat_RedShift_get
    if _newclass:RedShift = property(_cspace.csPixelFormat_RedShift_get, _cspace.csPixelFormat_RedShift_set)
    __swig_setmethods__["GreenShift"] = _cspace.csPixelFormat_GreenShift_set
    __swig_getmethods__["GreenShift"] = _cspace.csPixelFormat_GreenShift_get
    if _newclass:GreenShift = property(_cspace.csPixelFormat_GreenShift_get, _cspace.csPixelFormat_GreenShift_set)
    __swig_setmethods__["BlueShift"] = _cspace.csPixelFormat_BlueShift_set
    __swig_getmethods__["BlueShift"] = _cspace.csPixelFormat_BlueShift_get
    if _newclass:BlueShift = property(_cspace.csPixelFormat_BlueShift_get, _cspace.csPixelFormat_BlueShift_set)
    __swig_setmethods__["AlphaShift"] = _cspace.csPixelFormat_AlphaShift_set
    __swig_getmethods__["AlphaShift"] = _cspace.csPixelFormat_AlphaShift_get
    if _newclass:AlphaShift = property(_cspace.csPixelFormat_AlphaShift_get, _cspace.csPixelFormat_AlphaShift_set)
    __swig_setmethods__["RedBits"] = _cspace.csPixelFormat_RedBits_set
    __swig_getmethods__["RedBits"] = _cspace.csPixelFormat_RedBits_get
    if _newclass:RedBits = property(_cspace.csPixelFormat_RedBits_get, _cspace.csPixelFormat_RedBits_set)
    __swig_setmethods__["GreenBits"] = _cspace.csPixelFormat_GreenBits_set
    __swig_getmethods__["GreenBits"] = _cspace.csPixelFormat_GreenBits_get
    if _newclass:GreenBits = property(_cspace.csPixelFormat_GreenBits_get, _cspace.csPixelFormat_GreenBits_set)
    __swig_setmethods__["BlueBits"] = _cspace.csPixelFormat_BlueBits_set
    __swig_getmethods__["BlueBits"] = _cspace.csPixelFormat_BlueBits_get
    if _newclass:BlueBits = property(_cspace.csPixelFormat_BlueBits_get, _cspace.csPixelFormat_BlueBits_set)
    __swig_setmethods__["AlphaBits"] = _cspace.csPixelFormat_AlphaBits_set
    __swig_getmethods__["AlphaBits"] = _cspace.csPixelFormat_AlphaBits_get
    if _newclass:AlphaBits = property(_cspace.csPixelFormat_AlphaBits_get, _cspace.csPixelFormat_AlphaBits_set)
    __swig_setmethods__["PalEntries"] = _cspace.csPixelFormat_PalEntries_set
    __swig_getmethods__["PalEntries"] = _cspace.csPixelFormat_PalEntries_get
    if _newclass:PalEntries = property(_cspace.csPixelFormat_PalEntries_get, _cspace.csPixelFormat_PalEntries_set)
    __swig_setmethods__["PixelBytes"] = _cspace.csPixelFormat_PixelBytes_set
    __swig_getmethods__["PixelBytes"] = _cspace.csPixelFormat_PixelBytes_get
    if _newclass:PixelBytes = property(_cspace.csPixelFormat_PixelBytes_get, _cspace.csPixelFormat_PixelBytes_set)
    def complete(*args): return _cspace.csPixelFormat_complete(*args)
    def __init__(self, *args):
        _swig_setattr(self, csPixelFormat, 'this', _cspace.new_csPixelFormat(*args))
        _swig_setattr(self, csPixelFormat, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPixelFormat):
        try:
            if self.thisown: destroy(self)
        except: pass

class csPixelFormatPtr(csPixelFormat):
    def __init__(self, this):
        _swig_setattr(self, csPixelFormat, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPixelFormat, 'thisown', 0)
        _swig_setattr(self, csPixelFormat,self.__class__,csPixelFormat)
_cspace.csPixelFormat_swigregister(csPixelFormatPtr)

class csImageArea(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csImageArea, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csImageArea, name)
    def __repr__(self):
        return "<C csImageArea instance at %s>" % (self.this,)
    __swig_setmethods__["x"] = _cspace.csImageArea_x_set
    __swig_getmethods__["x"] = _cspace.csImageArea_x_get
    if _newclass:x = property(_cspace.csImageArea_x_get, _cspace.csImageArea_x_set)
    __swig_setmethods__["y"] = _cspace.csImageArea_y_set
    __swig_getmethods__["y"] = _cspace.csImageArea_y_get
    if _newclass:y = property(_cspace.csImageArea_y_get, _cspace.csImageArea_y_set)
    __swig_setmethods__["w"] = _cspace.csImageArea_w_set
    __swig_getmethods__["w"] = _cspace.csImageArea_w_get
    if _newclass:w = property(_cspace.csImageArea_w_get, _cspace.csImageArea_w_set)
    __swig_setmethods__["h"] = _cspace.csImageArea_h_set
    __swig_getmethods__["h"] = _cspace.csImageArea_h_get
    if _newclass:h = property(_cspace.csImageArea_h_get, _cspace.csImageArea_h_set)
    __swig_setmethods__["data"] = _cspace.csImageArea_data_set
    __swig_getmethods__["data"] = _cspace.csImageArea_data_get
    if _newclass:data = property(_cspace.csImageArea_data_get, _cspace.csImageArea_data_set)
    def __init__(self, *args):
        _swig_setattr(self, csImageArea, 'this', _cspace.new_csImageArea(*args))
        _swig_setattr(self, csImageArea, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csImageArea):
        try:
            if self.thisown: destroy(self)
        except: pass

class csImageAreaPtr(csImageArea):
    def __init__(self, this):
        _swig_setattr(self, csImageArea, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csImageArea, 'thisown', 0)
        _swig_setattr(self, csImageArea,self.__class__,csImageArea)
_cspace.csImageArea_swigregister(csImageAreaPtr)

class iOffscreenCanvasCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iOffscreenCanvasCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iOffscreenCanvasCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iOffscreenCanvasCallback instance at %s>" % (self.this,)
    def FinishDraw(*args): return _cspace.iOffscreenCanvasCallback_FinishDraw(*args)
    def SetRGB(*args): return _cspace.iOffscreenCanvasCallback_SetRGB(*args)
    def __del__(self, destroy=_cspace.delete_iOffscreenCanvasCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iOffscreenCanvasCallbackPtr(iOffscreenCanvasCallback):
    def __init__(self, this):
        _swig_setattr(self, iOffscreenCanvasCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iOffscreenCanvasCallback, 'thisown', 0)
        _swig_setattr(self, iOffscreenCanvasCallback,self.__class__,iOffscreenCanvasCallback)
_cspace.iOffscreenCanvasCallback_swigregister(iOffscreenCanvasCallbackPtr)

class iGraphics2D(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGraphics2D, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGraphics2D, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGraphics2D instance at %s>" % (self.this,)
    def Open(*args): return _cspace.iGraphics2D_Open(*args)
    def Close(*args): return _cspace.iGraphics2D_Close(*args)
    def GetWidth(*args): return _cspace.iGraphics2D_GetWidth(*args)
    def GetHeight(*args): return _cspace.iGraphics2D_GetHeight(*args)
    def GetPage(*args): return _cspace.iGraphics2D_GetPage(*args)
    def DoubleBuffer(*args): return _cspace.iGraphics2D_DoubleBuffer(*args)
    def GetDoubleBufferState(*args): return _cspace.iGraphics2D_GetDoubleBufferState(*args)
    def GetPixelFormat(*args): return _cspace.iGraphics2D_GetPixelFormat(*args)
    def GetPixelBytes(*args): return _cspace.iGraphics2D_GetPixelBytes(*args)
    def GetPalEntryCount(*args): return _cspace.iGraphics2D_GetPalEntryCount(*args)
    def GetPalette(*args): return _cspace.iGraphics2D_GetPalette(*args)
    def SetRGB(*args): return _cspace.iGraphics2D_SetRGB(*args)
    def FindRGB(*args): return _cspace.iGraphics2D_FindRGB(*args)
    def GetRGB(*args): return _cspace.iGraphics2D_GetRGB(*args)
    def GetRGBA(*args): return _cspace.iGraphics2D_GetRGBA(*args)
    def SetClipRect(*args): return _cspace.iGraphics2D_SetClipRect(*args)
    def GetClipRect(*args): return _cspace.iGraphics2D_GetClipRect(*args)
    def BeginDraw(*args): return _cspace.iGraphics2D_BeginDraw(*args)
    def FinishDraw(*args): return _cspace.iGraphics2D_FinishDraw(*args)
    def Print(*args): return _cspace.iGraphics2D_Print(*args)
    def Clear(*args): return _cspace.iGraphics2D_Clear(*args)
    def ClearAll(*args): return _cspace.iGraphics2D_ClearAll(*args)
    def DrawLine(*args): return _cspace.iGraphics2D_DrawLine(*args)
    def DrawBox(*args): return _cspace.iGraphics2D_DrawBox(*args)
    def ClipLine(*args): return _cspace.iGraphics2D_ClipLine(*args)
    def DrawPixel(*args): return _cspace.iGraphics2D_DrawPixel(*args)
    def DrawPixels(*args): return _cspace.iGraphics2D_DrawPixels(*args)
    def Blit(*args): return _cspace.iGraphics2D_Blit(*args)
    def GetPixelAt(*args): return _cspace.iGraphics2D_GetPixelAt(*args)
    def GetPixel(*args): return _cspace.iGraphics2D_GetPixel(*args)
    def SaveArea(*args): return _cspace.iGraphics2D_SaveArea(*args)
    def RestoreArea(*args): return _cspace.iGraphics2D_RestoreArea(*args)
    def FreeArea(*args): return _cspace.iGraphics2D_FreeArea(*args)
    def Write(*args): return _cspace.iGraphics2D_Write(*args)
    def WriteBaseline(*args): return _cspace.iGraphics2D_WriteBaseline(*args)
    def AllowResize(*args): return _cspace.iGraphics2D_AllowResize(*args)
    def Resize(*args): return _cspace.iGraphics2D_Resize(*args)
    def GetFontServer(*args): return _cspace.iGraphics2D_GetFontServer(*args)
    def PerformExtension(*args): return _cspace.iGraphics2D_PerformExtension(*args)
    def ScreenShot(*args): return _cspace.iGraphics2D_ScreenShot(*args)
    def GetNativeWindow(*args): return _cspace.iGraphics2D_GetNativeWindow(*args)
    def GetFullScreen(*args): return _cspace.iGraphics2D_GetFullScreen(*args)
    def SetFullScreen(*args): return _cspace.iGraphics2D_SetFullScreen(*args)
    def SetMousePosition(*args): return _cspace.iGraphics2D_SetMousePosition(*args)
    def SetMouseCursor(*args): return _cspace.iGraphics2D_SetMouseCursor(*args)
    def SetGamma(*args): return _cspace.iGraphics2D_SetGamma(*args)
    def GetGamma(*args): return _cspace.iGraphics2D_GetGamma(*args)
    def CreateOffscreenCanvas(*args): return _cspace.iGraphics2D_CreateOffscreenCanvas(*args)
    def __del__(self, destroy=_cspace.delete_iGraphics2D):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iGraphics2D_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iGraphics2D_scfGetVersion)
    def _PerformExtension(*args): return _cspace.iGraphics2D__PerformExtension(*args)
    def PerformExtension (self, command, *args):
      self._PerformExtension(self.__class__.__name__, command, args);


class iGraphics2DPtr(iGraphics2D):
    def __init__(self, this):
        _swig_setattr(self, iGraphics2D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGraphics2D, 'thisown', 0)
        _swig_setattr(self, iGraphics2D,self.__class__,iGraphics2D)
_cspace.iGraphics2D_swigregister(iGraphics2DPtr)

iGraphics2D_scfGetVersion = _cspace.iGraphics2D_scfGetVersion

CSDRAW_2DGRAPHICS = _cspace.CSDRAW_2DGRAPHICS
CSDRAW_3DGRAPHICS = _cspace.CSDRAW_3DGRAPHICS
CSDRAW_CLEARZBUFFER = _cspace.CSDRAW_CLEARZBUFFER
CSDRAW_CLEARSCREEN = _cspace.CSDRAW_CLEARSCREEN
CS_CLIPPER_NONE = _cspace.CS_CLIPPER_NONE
CS_CLIPPER_OPTIONAL = _cspace.CS_CLIPPER_OPTIONAL
CS_CLIPPER_TOPLEVEL = _cspace.CS_CLIPPER_TOPLEVEL
CS_CLIPPER_REQUIRED = _cspace.CS_CLIPPER_REQUIRED
CS_CLIP_NOT = _cspace.CS_CLIP_NOT
CS_CLIP_NEEDED = _cspace.CS_CLIP_NEEDED
class csFog(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csFog, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csFog, name)
    def __repr__(self):
        return "<C csFog instance at %s>" % (self.this,)
    __swig_setmethods__["enabled"] = _cspace.csFog_enabled_set
    __swig_getmethods__["enabled"] = _cspace.csFog_enabled_get
    if _newclass:enabled = property(_cspace.csFog_enabled_get, _cspace.csFog_enabled_set)
    __swig_setmethods__["density"] = _cspace.csFog_density_set
    __swig_getmethods__["density"] = _cspace.csFog_density_get
    if _newclass:density = property(_cspace.csFog_density_get, _cspace.csFog_density_set)
    __swig_setmethods__["red"] = _cspace.csFog_red_set
    __swig_getmethods__["red"] = _cspace.csFog_red_get
    if _newclass:red = property(_cspace.csFog_red_get, _cspace.csFog_red_set)
    __swig_setmethods__["green"] = _cspace.csFog_green_set
    __swig_getmethods__["green"] = _cspace.csFog_green_get
    if _newclass:green = property(_cspace.csFog_green_get, _cspace.csFog_green_set)
    __swig_setmethods__["blue"] = _cspace.csFog_blue_set
    __swig_getmethods__["blue"] = _cspace.csFog_blue_get
    if _newclass:blue = property(_cspace.csFog_blue_get, _cspace.csFog_blue_set)
    def __init__(self, *args):
        _swig_setattr(self, csFog, 'this', _cspace.new_csFog(*args))
        _swig_setattr(self, csFog, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csFog):
        try:
            if self.thisown: destroy(self)
        except: pass

class csFogPtr(csFog):
    def __init__(self, this):
        _swig_setattr(self, csFog, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csFog, 'thisown', 0)
        _swig_setattr(self, csFog,self.__class__,csFog)
_cspace.csFog_swigregister(csFogPtr)

CS_ZBUF_NONE = _cspace.CS_ZBUF_NONE
CS_ZBUF_FILL = _cspace.CS_ZBUF_FILL
CS_ZBUF_TEST = _cspace.CS_ZBUF_TEST
CS_ZBUF_USE = _cspace.CS_ZBUF_USE
CS_ZBUF_FILLONLY = _cspace.CS_ZBUF_FILLONLY
CS_ZBUF_EQUAL = _cspace.CS_ZBUF_EQUAL
CS_ZBUF_INVERT = _cspace.CS_ZBUF_INVERT
CS_ZBUF_MESH = _cspace.CS_ZBUF_MESH
CS_ZBUF_MESH2 = _cspace.CS_ZBUF_MESH2
CS_VATTRIB_SPECIFIC_FIRST = _cspace.CS_VATTRIB_SPECIFIC_FIRST
CS_VATTRIB_SPECIFIC_LAST = _cspace.CS_VATTRIB_SPECIFIC_LAST
CS_VATTRIB_GENERIC_FIRST = _cspace.CS_VATTRIB_GENERIC_FIRST
CS_VATTRIB_GENERIC_LAST = _cspace.CS_VATTRIB_GENERIC_LAST
CS_VATTRIB_POSITION = _cspace.CS_VATTRIB_POSITION
CS_VATTRIB_WEIGHT = _cspace.CS_VATTRIB_WEIGHT
CS_VATTRIB_NORMAL = _cspace.CS_VATTRIB_NORMAL
CS_VATTRIB_COLOR = _cspace.CS_VATTRIB_COLOR
CS_VATTRIB_PRIMARY_COLOR = _cspace.CS_VATTRIB_PRIMARY_COLOR
CS_VATTRIB_SECONDARY_COLOR = _cspace.CS_VATTRIB_SECONDARY_COLOR
CS_VATTRIB_FOGCOORD = _cspace.CS_VATTRIB_FOGCOORD
CS_VATTRIB_TEXCOORD = _cspace.CS_VATTRIB_TEXCOORD
CS_VATTRIB_TEXCOORD0 = _cspace.CS_VATTRIB_TEXCOORD0
CS_VATTRIB_TEXCOORD1 = _cspace.CS_VATTRIB_TEXCOORD1
CS_VATTRIB_TEXCOORD2 = _cspace.CS_VATTRIB_TEXCOORD2
CS_VATTRIB_TEXCOORD3 = _cspace.CS_VATTRIB_TEXCOORD3
CS_VATTRIB_TEXCOORD4 = _cspace.CS_VATTRIB_TEXCOORD4
CS_VATTRIB_TEXCOORD5 = _cspace.CS_VATTRIB_TEXCOORD5
CS_VATTRIB_TEXCOORD6 = _cspace.CS_VATTRIB_TEXCOORD6
CS_VATTRIB_TEXCOORD7 = _cspace.CS_VATTRIB_TEXCOORD7
CS_VATTRIB_0 = _cspace.CS_VATTRIB_0
CS_VATTRIB_1 = _cspace.CS_VATTRIB_1
CS_VATTRIB_2 = _cspace.CS_VATTRIB_2
CS_VATTRIB_3 = _cspace.CS_VATTRIB_3
CS_VATTRIB_4 = _cspace.CS_VATTRIB_4
CS_VATTRIB_5 = _cspace.CS_VATTRIB_5
CS_VATTRIB_6 = _cspace.CS_VATTRIB_6
CS_VATTRIB_7 = _cspace.CS_VATTRIB_7
CS_VATTRIB_8 = _cspace.CS_VATTRIB_8
CS_VATTRIB_9 = _cspace.CS_VATTRIB_9
CS_VATTRIB_10 = _cspace.CS_VATTRIB_10
CS_VATTRIB_11 = _cspace.CS_VATTRIB_11
CS_VATTRIB_12 = _cspace.CS_VATTRIB_12
CS_VATTRIB_13 = _cspace.CS_VATTRIB_13
CS_VATTRIB_14 = _cspace.CS_VATTRIB_14
CS_VATTRIB_15 = _cspace.CS_VATTRIB_15
G3DFOGMETHOD_NONE = _cspace.G3DFOGMETHOD_NONE
G3DFOGMETHOD_ZBUFFER = _cspace.G3DFOGMETHOD_ZBUFFER
G3DFOGMETHOD_VERTEX = _cspace.G3DFOGMETHOD_VERTEX
CS_FX_MASK_MIXMODE = _cspace.CS_FX_MASK_MIXMODE
CS_FX_COPY = _cspace.CS_FX_COPY
CS_FX_MULTIPLY = _cspace.CS_FX_MULTIPLY
CS_FX_MULTIPLY2 = _cspace.CS_FX_MULTIPLY2
CS_FX_ADD = _cspace.CS_FX_ADD
CS_FX_ALPHA = _cspace.CS_FX_ALPHA
CS_FX_TRANSPARENT = _cspace.CS_FX_TRANSPARENT
CS_FX_DESTALPHAADD = _cspace.CS_FX_DESTALPHAADD
CS_FX_SRCALPHAADD = _cspace.CS_FX_SRCALPHAADD
CS_FX_PREMULTALPHA = _cspace.CS_FX_PREMULTALPHA
CS_FX_MESH = _cspace.CS_FX_MESH
CS_FX_KEYCOLOR = _cspace.CS_FX_KEYCOLOR
CS_FX_FLAT = _cspace.CS_FX_FLAT
CS_FX_TILING = _cspace.CS_FX_TILING
CS_FX_MASK_ALPHA = _cspace.CS_FX_MASK_ALPHA
class csAlphaMode(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csAlphaMode, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csAlphaMode, name)
    def __repr__(self):
        return "<C csAlphaMode instance at %s>" % (self.this,)
    alphaNone = _cspace.csAlphaMode_alphaNone
    alphaBinary = _cspace.csAlphaMode_alphaBinary
    alphaSmooth = _cspace.csAlphaMode_alphaSmooth
    __swig_setmethods__["autoAlphaMode"] = _cspace.csAlphaMode_autoAlphaMode_set
    __swig_getmethods__["autoAlphaMode"] = _cspace.csAlphaMode_autoAlphaMode_get
    if _newclass:autoAlphaMode = property(_cspace.csAlphaMode_autoAlphaMode_get, _cspace.csAlphaMode_autoAlphaMode_set)
    def __init__(self, *args):
        _swig_setattr(self, csAlphaMode, 'this', _cspace.new_csAlphaMode(*args))
        _swig_setattr(self, csAlphaMode, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csAlphaMode):
        try:
            if self.thisown: destroy(self)
        except: pass

class csAlphaModePtr(csAlphaMode):
    def __init__(self, this):
        _swig_setattr(self, csAlphaMode, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csAlphaMode, 'thisown', 0)
        _swig_setattr(self, csAlphaMode,self.__class__,csAlphaMode)
_cspace.csAlphaMode_swigregister(csAlphaModePtr)

CS_LIGHTPARAM_POSITION = _cspace.CS_LIGHTPARAM_POSITION
CS_LIGHTPARAM_DIFFUSE = _cspace.CS_LIGHTPARAM_DIFFUSE
CS_LIGHTPARAM_SPECULAR = _cspace.CS_LIGHTPARAM_SPECULAR
CS_LIGHTPARAM_ATTENUATION = _cspace.CS_LIGHTPARAM_ATTENUATION
CS_SHADOW_VOLUME_BEGIN = _cspace.CS_SHADOW_VOLUME_BEGIN
CS_SHADOW_VOLUME_PASS1 = _cspace.CS_SHADOW_VOLUME_PASS1
CS_SHADOW_VOLUME_PASS2 = _cspace.CS_SHADOW_VOLUME_PASS2
CS_SHADOW_VOLUME_FAIL1 = _cspace.CS_SHADOW_VOLUME_FAIL1
CS_SHADOW_VOLUME_FAIL2 = _cspace.CS_SHADOW_VOLUME_FAIL2
CS_SHADOW_VOLUME_USE = _cspace.CS_SHADOW_VOLUME_USE
CS_SHADOW_VOLUME_FINISH = _cspace.CS_SHADOW_VOLUME_FINISH
G3DRENDERSTATE_ZBUFFERMODE = _cspace.G3DRENDERSTATE_ZBUFFERMODE
G3DRENDERSTATE_DITHERENABLE = _cspace.G3DRENDERSTATE_DITHERENABLE
G3DRENDERSTATE_BILINEARMAPPINGENABLE = _cspace.G3DRENDERSTATE_BILINEARMAPPINGENABLE
G3DRENDERSTATE_TRILINEARMAPPINGENABLE = _cspace.G3DRENDERSTATE_TRILINEARMAPPINGENABLE
G3DRENDERSTATE_TRANSPARENCYENABLE = _cspace.G3DRENDERSTATE_TRANSPARENCYENABLE
G3DRENDERSTATE_MIPMAPENABLE = _cspace.G3DRENDERSTATE_MIPMAPENABLE
G3DRENDERSTATE_TEXTUREMAPPINGENABLE = _cspace.G3DRENDERSTATE_TEXTUREMAPPINGENABLE
G3DRENDERSTATE_LIGHTINGENABLE = _cspace.G3DRENDERSTATE_LIGHTINGENABLE
G3DRENDERSTATE_INTERLACINGENABLE = _cspace.G3DRENDERSTATE_INTERLACINGENABLE
G3DRENDERSTATE_MMXENABLE = _cspace.G3DRENDERSTATE_MMXENABLE
G3DRENDERSTATE_INTERPOLATIONSTEP = _cspace.G3DRENDERSTATE_INTERPOLATIONSTEP
G3DRENDERSTATE_MAXPOLYGONSTODRAW = _cspace.G3DRENDERSTATE_MAXPOLYGONSTODRAW
G3DRENDERSTATE_GOURAUDENABLE = _cspace.G3DRENDERSTATE_GOURAUDENABLE
G3DRENDERSTATE_EDGES = _cspace.G3DRENDERSTATE_EDGES
class csGraphics3DCaps(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csGraphics3DCaps, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csGraphics3DCaps, name)
    def __repr__(self):
        return "<C csGraphics3DCaps instance at %s>" % (self.this,)
    __swig_setmethods__["CanClip"] = _cspace.csGraphics3DCaps_CanClip_set
    __swig_getmethods__["CanClip"] = _cspace.csGraphics3DCaps_CanClip_get
    if _newclass:CanClip = property(_cspace.csGraphics3DCaps_CanClip_get, _cspace.csGraphics3DCaps_CanClip_set)
    __swig_setmethods__["minTexHeight"] = _cspace.csGraphics3DCaps_minTexHeight_set
    __swig_getmethods__["minTexHeight"] = _cspace.csGraphics3DCaps_minTexHeight_get
    if _newclass:minTexHeight = property(_cspace.csGraphics3DCaps_minTexHeight_get, _cspace.csGraphics3DCaps_minTexHeight_set)
    __swig_setmethods__["minTexWidth"] = _cspace.csGraphics3DCaps_minTexWidth_set
    __swig_getmethods__["minTexWidth"] = _cspace.csGraphics3DCaps_minTexWidth_get
    if _newclass:minTexWidth = property(_cspace.csGraphics3DCaps_minTexWidth_get, _cspace.csGraphics3DCaps_minTexWidth_set)
    __swig_setmethods__["maxTexHeight"] = _cspace.csGraphics3DCaps_maxTexHeight_set
    __swig_getmethods__["maxTexHeight"] = _cspace.csGraphics3DCaps_maxTexHeight_get
    if _newclass:maxTexHeight = property(_cspace.csGraphics3DCaps_maxTexHeight_get, _cspace.csGraphics3DCaps_maxTexHeight_set)
    __swig_setmethods__["maxTexWidth"] = _cspace.csGraphics3DCaps_maxTexWidth_set
    __swig_getmethods__["maxTexWidth"] = _cspace.csGraphics3DCaps_maxTexWidth_get
    if _newclass:maxTexWidth = property(_cspace.csGraphics3DCaps_maxTexWidth_get, _cspace.csGraphics3DCaps_maxTexWidth_set)
    __swig_setmethods__["fog"] = _cspace.csGraphics3DCaps_fog_set
    __swig_getmethods__["fog"] = _cspace.csGraphics3DCaps_fog_get
    if _newclass:fog = property(_cspace.csGraphics3DCaps_fog_get, _cspace.csGraphics3DCaps_fog_set)
    __swig_setmethods__["NeedsPO2Maps"] = _cspace.csGraphics3DCaps_NeedsPO2Maps_set
    __swig_getmethods__["NeedsPO2Maps"] = _cspace.csGraphics3DCaps_NeedsPO2Maps_get
    if _newclass:NeedsPO2Maps = property(_cspace.csGraphics3DCaps_NeedsPO2Maps_get, _cspace.csGraphics3DCaps_NeedsPO2Maps_set)
    __swig_setmethods__["MaxAspectRatio"] = _cspace.csGraphics3DCaps_MaxAspectRatio_set
    __swig_getmethods__["MaxAspectRatio"] = _cspace.csGraphics3DCaps_MaxAspectRatio_get
    if _newclass:MaxAspectRatio = property(_cspace.csGraphics3DCaps_MaxAspectRatio_get, _cspace.csGraphics3DCaps_MaxAspectRatio_set)
    __swig_setmethods__["SupportsPointSprites"] = _cspace.csGraphics3DCaps_SupportsPointSprites_set
    __swig_getmethods__["SupportsPointSprites"] = _cspace.csGraphics3DCaps_SupportsPointSprites_get
    if _newclass:SupportsPointSprites = property(_cspace.csGraphics3DCaps_SupportsPointSprites_get, _cspace.csGraphics3DCaps_SupportsPointSprites_set)
    __swig_setmethods__["DestinationAlpha"] = _cspace.csGraphics3DCaps_DestinationAlpha_set
    __swig_getmethods__["DestinationAlpha"] = _cspace.csGraphics3DCaps_DestinationAlpha_get
    if _newclass:DestinationAlpha = property(_cspace.csGraphics3DCaps_DestinationAlpha_get, _cspace.csGraphics3DCaps_DestinationAlpha_set)
    __swig_setmethods__["StencilShadows"] = _cspace.csGraphics3DCaps_StencilShadows_set
    __swig_getmethods__["StencilShadows"] = _cspace.csGraphics3DCaps_StencilShadows_get
    if _newclass:StencilShadows = property(_cspace.csGraphics3DCaps_StencilShadows_get, _cspace.csGraphics3DCaps_StencilShadows_set)
    def __init__(self, *args):
        _swig_setattr(self, csGraphics3DCaps, 'this', _cspace.new_csGraphics3DCaps(*args))
        _swig_setattr(self, csGraphics3DCaps, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csGraphics3DCaps):
        try:
            if self.thisown: destroy(self)
        except: pass

class csGraphics3DCapsPtr(csGraphics3DCaps):
    def __init__(self, this):
        _swig_setattr(self, csGraphics3DCaps, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csGraphics3DCaps, 'thisown', 0)
        _swig_setattr(self, csGraphics3DCaps,self.__class__,csGraphics3DCaps)
_cspace.csGraphics3DCaps_swigregister(csGraphics3DCapsPtr)

CS_MESHTYPE_TRIANGLES = _cspace.CS_MESHTYPE_TRIANGLES
CS_MESHTYPE_QUADS = _cspace.CS_MESHTYPE_QUADS
CS_MESHTYPE_TRIANGLESTRIP = _cspace.CS_MESHTYPE_TRIANGLESTRIP
CS_MESHTYPE_TRIANGLEFAN = _cspace.CS_MESHTYPE_TRIANGLEFAN
CS_MESHTYPE_POINTS = _cspace.CS_MESHTYPE_POINTS
CS_MESHTYPE_POINT_SPRITES = _cspace.CS_MESHTYPE_POINT_SPRITES
CS_MESHTYPE_LINES = _cspace.CS_MESHTYPE_LINES
CS_MESHTYPE_LINESTRIP = _cspace.CS_MESHTYPE_LINESTRIP
CS_MESHTYPE_POLYGON = _cspace.CS_MESHTYPE_POLYGON
csSimpleMeshScreenspace = _cspace.csSimpleMeshScreenspace
class csSimpleRenderMesh(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csSimpleRenderMesh, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csSimpleRenderMesh, name)
    def __repr__(self):
        return "<C csSimpleRenderMesh instance at %s>" % (self.this,)
    __swig_setmethods__["meshtype"] = _cspace.csSimpleRenderMesh_meshtype_set
    __swig_getmethods__["meshtype"] = _cspace.csSimpleRenderMesh_meshtype_get
    if _newclass:meshtype = property(_cspace.csSimpleRenderMesh_meshtype_get, _cspace.csSimpleRenderMesh_meshtype_set)
    __swig_setmethods__["indexCount"] = _cspace.csSimpleRenderMesh_indexCount_set
    __swig_getmethods__["indexCount"] = _cspace.csSimpleRenderMesh_indexCount_get
    if _newclass:indexCount = property(_cspace.csSimpleRenderMesh_indexCount_get, _cspace.csSimpleRenderMesh_indexCount_set)
    __swig_setmethods__["indices"] = _cspace.csSimpleRenderMesh_indices_set
    __swig_getmethods__["indices"] = _cspace.csSimpleRenderMesh_indices_get
    if _newclass:indices = property(_cspace.csSimpleRenderMesh_indices_get, _cspace.csSimpleRenderMesh_indices_set)
    __swig_setmethods__["vertexCount"] = _cspace.csSimpleRenderMesh_vertexCount_set
    __swig_getmethods__["vertexCount"] = _cspace.csSimpleRenderMesh_vertexCount_get
    if _newclass:vertexCount = property(_cspace.csSimpleRenderMesh_vertexCount_get, _cspace.csSimpleRenderMesh_vertexCount_set)
    __swig_setmethods__["vertices"] = _cspace.csSimpleRenderMesh_vertices_set
    __swig_getmethods__["vertices"] = _cspace.csSimpleRenderMesh_vertices_get
    if _newclass:vertices = property(_cspace.csSimpleRenderMesh_vertices_get, _cspace.csSimpleRenderMesh_vertices_set)
    __swig_setmethods__["texcoords"] = _cspace.csSimpleRenderMesh_texcoords_set
    __swig_getmethods__["texcoords"] = _cspace.csSimpleRenderMesh_texcoords_get
    if _newclass:texcoords = property(_cspace.csSimpleRenderMesh_texcoords_get, _cspace.csSimpleRenderMesh_texcoords_set)
    __swig_setmethods__["colors"] = _cspace.csSimpleRenderMesh_colors_set
    __swig_getmethods__["colors"] = _cspace.csSimpleRenderMesh_colors_get
    if _newclass:colors = property(_cspace.csSimpleRenderMesh_colors_get, _cspace.csSimpleRenderMesh_colors_set)
    __swig_setmethods__["texture"] = _cspace.csSimpleRenderMesh_texture_set
    __swig_getmethods__["texture"] = _cspace.csSimpleRenderMesh_texture_get
    if _newclass:texture = property(_cspace.csSimpleRenderMesh_texture_get, _cspace.csSimpleRenderMesh_texture_set)
    __swig_setmethods__["shader"] = _cspace.csSimpleRenderMesh_shader_set
    __swig_getmethods__["shader"] = _cspace.csSimpleRenderMesh_shader_get
    if _newclass:shader = property(_cspace.csSimpleRenderMesh_shader_get, _cspace.csSimpleRenderMesh_shader_set)
    __swig_setmethods__["dynDomain"] = _cspace.csSimpleRenderMesh_dynDomain_set
    __swig_getmethods__["dynDomain"] = _cspace.csSimpleRenderMesh_dynDomain_get
    if _newclass:dynDomain = property(_cspace.csSimpleRenderMesh_dynDomain_get, _cspace.csSimpleRenderMesh_dynDomain_set)
    __swig_setmethods__["alphaType"] = _cspace.csSimpleRenderMesh_alphaType_set
    __swig_getmethods__["alphaType"] = _cspace.csSimpleRenderMesh_alphaType_get
    if _newclass:alphaType = property(_cspace.csSimpleRenderMesh_alphaType_get, _cspace.csSimpleRenderMesh_alphaType_set)
    __swig_setmethods__["z_buf_mode"] = _cspace.csSimpleRenderMesh_z_buf_mode_set
    __swig_getmethods__["z_buf_mode"] = _cspace.csSimpleRenderMesh_z_buf_mode_get
    if _newclass:z_buf_mode = property(_cspace.csSimpleRenderMesh_z_buf_mode_get, _cspace.csSimpleRenderMesh_z_buf_mode_set)
    __swig_setmethods__["mixmode"] = _cspace.csSimpleRenderMesh_mixmode_set
    __swig_getmethods__["mixmode"] = _cspace.csSimpleRenderMesh_mixmode_get
    if _newclass:mixmode = property(_cspace.csSimpleRenderMesh_mixmode_get, _cspace.csSimpleRenderMesh_mixmode_set)
    __swig_setmethods__["object2camera"] = _cspace.csSimpleRenderMesh_object2camera_set
    __swig_getmethods__["object2camera"] = _cspace.csSimpleRenderMesh_object2camera_get
    if _newclass:object2camera = property(_cspace.csSimpleRenderMesh_object2camera_get, _cspace.csSimpleRenderMesh_object2camera_set)
    def __init__(self, *args):
        _swig_setattr(self, csSimpleRenderMesh, 'this', _cspace.new_csSimpleRenderMesh(*args))
        _swig_setattr(self, csSimpleRenderMesh, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csSimpleRenderMesh):
        try:
            if self.thisown: destroy(self)
        except: pass

class csSimpleRenderMeshPtr(csSimpleRenderMesh):
    def __init__(self, this):
        _swig_setattr(self, csSimpleRenderMesh, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csSimpleRenderMesh, 'thisown', 0)
        _swig_setattr(self, csSimpleRenderMesh,self.__class__,csSimpleRenderMesh)
_cspace.csSimpleRenderMesh_swigregister(csSimpleRenderMeshPtr)

class iGraphics3D(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iGraphics3D, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iGraphics3D, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iGraphics3D instance at %s>" % (self.this,)
    def Open(*args): return _cspace.iGraphics3D_Open(*args)
    def Close(*args): return _cspace.iGraphics3D_Close(*args)
    def GetDriver2D(*args): return _cspace.iGraphics3D_GetDriver2D(*args)
    def GetTextureManager(*args): return _cspace.iGraphics3D_GetTextureManager(*args)
    def SetDimensions(*args): return _cspace.iGraphics3D_SetDimensions(*args)
    def GetWidth(*args): return _cspace.iGraphics3D_GetWidth(*args)
    def GetHeight(*args): return _cspace.iGraphics3D_GetHeight(*args)
    def GetCaps(*args): return _cspace.iGraphics3D_GetCaps(*args)
    def SetPerspectiveCenter(*args): return _cspace.iGraphics3D_SetPerspectiveCenter(*args)
    def GetPerspectiveCenter(*args): return _cspace.iGraphics3D_GetPerspectiveCenter(*args)
    def SetPerspectiveAspect(*args): return _cspace.iGraphics3D_SetPerspectiveAspect(*args)
    def GetPerspectiveAspect(*args): return _cspace.iGraphics3D_GetPerspectiveAspect(*args)
    def SetObjectToCamera(*args): return _cspace.iGraphics3D_SetObjectToCamera(*args)
    def GetObjectToCamera(*args): return _cspace.iGraphics3D_GetObjectToCamera(*args)
    def SetRenderTarget(*args): return _cspace.iGraphics3D_SetRenderTarget(*args)
    def GetRenderTarget(*args): return _cspace.iGraphics3D_GetRenderTarget(*args)
    def BeginDraw(*args): return _cspace.iGraphics3D_BeginDraw(*args)
    def FinishDraw(*args): return _cspace.iGraphics3D_FinishDraw(*args)
    def Print(*args): return _cspace.iGraphics3D_Print(*args)
    def DrawPixmap(*args): return _cspace.iGraphics3D_DrawPixmap(*args)
    def DrawLine(*args): return _cspace.iGraphics3D_DrawLine(*args)
    def SetClipper(*args): return _cspace.iGraphics3D_SetClipper(*args)
    def GetClipper(*args): return _cspace.iGraphics3D_GetClipper(*args)
    def GetClipType(*args): return _cspace.iGraphics3D_GetClipType(*args)
    def SetNearPlane(*args): return _cspace.iGraphics3D_SetNearPlane(*args)
    def ResetNearPlane(*args): return _cspace.iGraphics3D_ResetNearPlane(*args)
    def GetNearPlane(*args): return _cspace.iGraphics3D_GetNearPlane(*args)
    def HasNearPlane(*args): return _cspace.iGraphics3D_HasNearPlane(*args)
    def SetRenderState(*args): return _cspace.iGraphics3D_SetRenderState(*args)
    def GetRenderState(*args): return _cspace.iGraphics3D_GetRenderState(*args)
    def SetOption(*args): return _cspace.iGraphics3D_SetOption(*args)
    def ActivateBuffers(*args): return _cspace.iGraphics3D_ActivateBuffers(*args)
    def DeactivateBuffers(*args): return _cspace.iGraphics3D_DeactivateBuffers(*args)
    def SetTextureState(*args): return _cspace.iGraphics3D_SetTextureState(*args)
    def DrawMesh(*args): return _cspace.iGraphics3D_DrawMesh(*args)
    def SetWriteMask(*args): return _cspace.iGraphics3D_SetWriteMask(*args)
    def GetWriteMask(*args): return _cspace.iGraphics3D_GetWriteMask(*args)
    def SetZMode(*args): return _cspace.iGraphics3D_SetZMode(*args)
    def EnableZOffset(*args): return _cspace.iGraphics3D_EnableZOffset(*args)
    def DisableZOffset(*args): return _cspace.iGraphics3D_DisableZOffset(*args)
    def SetShadowState(*args): return _cspace.iGraphics3D_SetShadowState(*args)
    def GetZBuffValue(*args): return _cspace.iGraphics3D_GetZBuffValue(*args)
    def OpenPortal(*args): return _cspace.iGraphics3D_OpenPortal(*args)
    def ClosePortal(*args): return _cspace.iGraphics3D_ClosePortal(*args)
    def CreateHalo(*args): return _cspace.iGraphics3D_CreateHalo(*args)
    def RemoveFromCache(*args): return _cspace.iGraphics3D_RemoveFromCache(*args)
    def IsLightmapOK(*args): return _cspace.iGraphics3D_IsLightmapOK(*args)
    def CreatePolygonRenderer(*args): return _cspace.iGraphics3D_CreatePolygonRenderer(*args)
    def SetWorldToCamera(*args): return _cspace.iGraphics3D_SetWorldToCamera(*args)
    def DrawSimpleMesh(*args): return _cspace.iGraphics3D_DrawSimpleMesh(*args)
    def GetZMode(*args): return _cspace.iGraphics3D_GetZMode(*args)
    def __del__(self, destroy=_cspace.delete_iGraphics3D):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iGraphics3D_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iGraphics3D_scfGetVersion)

class iGraphics3DPtr(iGraphics3D):
    def __init__(self, this):
        _swig_setattr(self, iGraphics3D, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iGraphics3D, 'thisown', 0)
        _swig_setattr(self, iGraphics3D,self.__class__,iGraphics3D)
_cspace.iGraphics3D_swigregister(iGraphics3DPtr)

iGraphics3D_scfGetVersion = _cspace.iGraphics3D_scfGetVersion

csmcNone = _cspace.csmcNone
csmcArrow = _cspace.csmcArrow
csmcLens = _cspace.csmcLens
csmcCross = _cspace.csmcCross
csmcPen = _cspace.csmcPen
csmcMove = _cspace.csmcMove
csmcSizeNWSE = _cspace.csmcSizeNWSE
csmcSizeNESW = _cspace.csmcSizeNESW
csmcSizeNS = _cspace.csmcSizeNS
csmcSizeEW = _cspace.csmcSizeEW
csmcStop = _cspace.csmcStop
csmcWait = _cspace.csmcWait
CS_ALERT_ERROR = _cspace.CS_ALERT_ERROR
CS_ALERT_WARNING = _cspace.CS_ALERT_WARNING
CS_ALERT_NOTE = _cspace.CS_ALERT_NOTE
class iNativeWindowManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iNativeWindowManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iNativeWindowManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iNativeWindowManager instance at %s>" % (self.this,)
    def Alert(*args): return _cspace.iNativeWindowManager_Alert(*args)
    def __del__(self, destroy=_cspace.delete_iNativeWindowManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class iNativeWindowManagerPtr(iNativeWindowManager):
    def __init__(self, this):
        _swig_setattr(self, iNativeWindowManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iNativeWindowManager, 'thisown', 0)
        _swig_setattr(self, iNativeWindowManager,self.__class__,iNativeWindowManager)
_cspace.iNativeWindowManager_swigregister(iNativeWindowManagerPtr)

class iNativeWindow(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iNativeWindow, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iNativeWindow, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iNativeWindow instance at %s>" % (self.this,)
    def SetTitle(*args): return _cspace.iNativeWindow_SetTitle(*args)
    def __del__(self, destroy=_cspace.delete_iNativeWindow):
        try:
            if self.thisown: destroy(self)
        except: pass

class iNativeWindowPtr(iNativeWindow):
    def __init__(self, this):
        _swig_setattr(self, iNativeWindow, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iNativeWindow, 'thisown', 0)
        _swig_setattr(self, iNativeWindow,self.__class__,iNativeWindow)
_cspace.iNativeWindow_swigregister(iNativeWindowPtr)

CSFONT_LARGE = _cspace.CSFONT_LARGE
CSFONT_ITALIC = _cspace.CSFONT_ITALIC
CSFONT_COURIER = _cspace.CSFONT_COURIER
CSFONT_SMALL = _cspace.CSFONT_SMALL
CS_FONT_DEFAULT_GLYPH = _cspace.CS_FONT_DEFAULT_GLYPH
class iFontDeleteNotify(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFontDeleteNotify, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFontDeleteNotify, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFontDeleteNotify instance at %s>" % (self.this,)
    def BeforeDelete(*args): return _cspace.iFontDeleteNotify_BeforeDelete(*args)
    def __del__(self, destroy=_cspace.delete_iFontDeleteNotify):
        try:
            if self.thisown: destroy(self)
        except: pass

class iFontDeleteNotifyPtr(iFontDeleteNotify):
    def __init__(self, this):
        _swig_setattr(self, iFontDeleteNotify, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFontDeleteNotify, 'thisown', 0)
        _swig_setattr(self, iFontDeleteNotify,self.__class__,iFontDeleteNotify)
_cspace.iFontDeleteNotify_swigregister(iFontDeleteNotifyPtr)

class csBitmapMetrics(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csBitmapMetrics, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csBitmapMetrics, name)
    def __repr__(self):
        return "<C csBitmapMetrics instance at %s>" % (self.this,)
    __swig_setmethods__["width"] = _cspace.csBitmapMetrics_width_set
    __swig_getmethods__["width"] = _cspace.csBitmapMetrics_width_get
    if _newclass:width = property(_cspace.csBitmapMetrics_width_get, _cspace.csBitmapMetrics_width_set)
    __swig_setmethods__["height"] = _cspace.csBitmapMetrics_height_set
    __swig_getmethods__["height"] = _cspace.csBitmapMetrics_height_get
    if _newclass:height = property(_cspace.csBitmapMetrics_height_get, _cspace.csBitmapMetrics_height_set)
    __swig_setmethods__["left"] = _cspace.csBitmapMetrics_left_set
    __swig_getmethods__["left"] = _cspace.csBitmapMetrics_left_get
    if _newclass:left = property(_cspace.csBitmapMetrics_left_get, _cspace.csBitmapMetrics_left_set)
    __swig_setmethods__["top"] = _cspace.csBitmapMetrics_top_set
    __swig_getmethods__["top"] = _cspace.csBitmapMetrics_top_get
    if _newclass:top = property(_cspace.csBitmapMetrics_top_get, _cspace.csBitmapMetrics_top_set)
    def __init__(self, *args):
        _swig_setattr(self, csBitmapMetrics, 'this', _cspace.new_csBitmapMetrics(*args))
        _swig_setattr(self, csBitmapMetrics, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csBitmapMetrics):
        try:
            if self.thisown: destroy(self)
        except: pass

class csBitmapMetricsPtr(csBitmapMetrics):
    def __init__(self, this):
        _swig_setattr(self, csBitmapMetrics, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csBitmapMetrics, 'thisown', 0)
        _swig_setattr(self, csBitmapMetrics,self.__class__,csBitmapMetrics)
_cspace.csBitmapMetrics_swigregister(csBitmapMetricsPtr)

class csGlyphMetrics(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csGlyphMetrics, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csGlyphMetrics, name)
    def __repr__(self):
        return "<C csGlyphMetrics instance at %s>" % (self.this,)
    __swig_setmethods__["advance"] = _cspace.csGlyphMetrics_advance_set
    __swig_getmethods__["advance"] = _cspace.csGlyphMetrics_advance_get
    if _newclass:advance = property(_cspace.csGlyphMetrics_advance_get, _cspace.csGlyphMetrics_advance_set)
    def __init__(self, *args):
        _swig_setattr(self, csGlyphMetrics, 'this', _cspace.new_csGlyphMetrics(*args))
        _swig_setattr(self, csGlyphMetrics, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csGlyphMetrics):
        try:
            if self.thisown: destroy(self)
        except: pass

class csGlyphMetricsPtr(csGlyphMetrics):
    def __init__(self, this):
        _swig_setattr(self, csGlyphMetrics, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csGlyphMetrics, 'thisown', 0)
        _swig_setattr(self, csGlyphMetrics,self.__class__,csGlyphMetrics)
_cspace.csGlyphMetrics_swigregister(csGlyphMetricsPtr)

class iFont(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFont, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFont, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFont instance at %s>" % (self.this,)
    def AddDeleteCallback(*args): return _cspace.iFont_AddDeleteCallback(*args)
    def RemoveDeleteCallback(*args): return _cspace.iFont_RemoveDeleteCallback(*args)
    def GetSize(*args): return _cspace.iFont_GetSize(*args)
    def GetMaxSize(*args): return _cspace.iFont_GetMaxSize(*args)
    def GetGlyphMetrics(*args): return _cspace.iFont_GetGlyphMetrics(*args)
    def GetGlyphBitmap(*args): return _cspace.iFont_GetGlyphBitmap(*args)
    def GetGlyphAlphaBitmap(*args): return _cspace.iFont_GetGlyphAlphaBitmap(*args)
    def GetDimensions(*args): return _cspace.iFont_GetDimensions(*args)
    def GetLength(*args): return _cspace.iFont_GetLength(*args)
    def GetDescent(*args): return _cspace.iFont_GetDescent(*args)
    def GetAscent(*args): return _cspace.iFont_GetAscent(*args)
    def HasGlyph(*args): return _cspace.iFont_HasGlyph(*args)
    def GetTextHeight(*args): return _cspace.iFont_GetTextHeight(*args)
    def GetUnderlinePosition(*args): return _cspace.iFont_GetUnderlinePosition(*args)
    def GetUnderlineThickness(*args): return _cspace.iFont_GetUnderlineThickness(*args)
    def __del__(self, destroy=_cspace.delete_iFont):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iFont_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iFont_scfGetVersion)

class iFontPtr(iFont):
    def __init__(self, this):
        _swig_setattr(self, iFont, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFont, 'thisown', 0)
        _swig_setattr(self, iFont,self.__class__,iFont)
_cspace.iFont_swigregister(iFontPtr)

iFont_scfGetVersion = _cspace.iFont_scfGetVersion

class iFontServer(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iFontServer, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iFontServer, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iFontServer instance at %s>" % (self.this,)
    def LoadFont(*args): return _cspace.iFontServer_LoadFont(*args)
    def __del__(self, destroy=_cspace.delete_iFontServer):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iFontServer_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iFontServer_scfGetVersion)

class iFontServerPtr(iFontServer):
    def __init__(self, this):
        _swig_setattr(self, iFontServer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iFontServer, 'thisown', 0)
        _swig_setattr(self, iFontServer,self.__class__,iFontServer)
_cspace.iFontServer_swigregister(iFontServerPtr)

iFontServer_scfGetVersion = _cspace.iFontServer_scfGetVersion

class iHalo(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iHalo, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iHalo, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iHalo instance at %s>" % (self.this,)
    def GetWidth(*args): return _cspace.iHalo_GetWidth(*args)
    def GetHeight(*args): return _cspace.iHalo_GetHeight(*args)
    def SetColor(*args): return _cspace.iHalo_SetColor(*args)
    def GetColor(*args): return _cspace.iHalo_GetColor(*args)
    def Draw(*args): return _cspace.iHalo_Draw(*args)
    def __del__(self, destroy=_cspace.delete_iHalo):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iHalo_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iHalo_scfGetVersion)

class iHaloPtr(iHalo):
    def __init__(self, this):
        _swig_setattr(self, iHalo, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iHalo, 'thisown', 0)
        _swig_setattr(self, iHalo,self.__class__,iHalo)
_cspace.iHalo_swigregister(iHaloPtr)

iHalo_scfGetVersion = _cspace.iHalo_scfGetVersion

class iShaderVariableContext(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iShaderVariableContext, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iShaderVariableContext, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iShaderVariableContext instance at %s>" % (self.this,)
    def AddVariable(*args): return _cspace.iShaderVariableContext_AddVariable(*args)
    def GetVariable(*args): return _cspace.iShaderVariableContext_GetVariable(*args)
    def GetVariableAdd(*args): return _cspace.iShaderVariableContext_GetVariableAdd(*args)
    def GetShaderVariables(*args): return _cspace.iShaderVariableContext_GetShaderVariables(*args)
    def PushVariables(*args): return _cspace.iShaderVariableContext_PushVariables(*args)
    def PopVariables(*args): return _cspace.iShaderVariableContext_PopVariables(*args)
    def __del__(self, destroy=_cspace.delete_iShaderVariableContext):
        try:
            if self.thisown: destroy(self)
        except: pass

class iShaderVariableContextPtr(iShaderVariableContext):
    def __init__(self, this):
        _swig_setattr(self, iShaderVariableContext, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iShaderVariableContext, 'thisown', 0)
        _swig_setattr(self, iShaderVariableContext,self.__class__,iShaderVariableContext)
_cspace.iShaderVariableContext_swigregister(iShaderVariableContextPtr)

TagNeutral = _cspace.TagNeutral
TagForbidden = _cspace.TagForbidden
TagRequired = _cspace.TagRequired
class iShaderManager(iShaderVariableContext):
    __swig_setmethods__ = {}
    for _s in [iShaderVariableContext]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iShaderManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iShaderVariableContext]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iShaderManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iShaderManager instance at %s>" % (self.this,)
    def RegisterShader(*args): return _cspace.iShaderManager_RegisterShader(*args)
    def UnregisterShader(*args): return _cspace.iShaderManager_UnregisterShader(*args)
    def GetShader(*args): return _cspace.iShaderManager_GetShader(*args)
    def GetShaders(*args): return _cspace.iShaderManager_GetShaders(*args)
    def RegisterCompiler(*args): return _cspace.iShaderManager_RegisterCompiler(*args)
    def GetCompiler(*args): return _cspace.iShaderManager_GetCompiler(*args)
    def GetShaderVariableStack(*args): return _cspace.iShaderManager_GetShaderVariableStack(*args)
    def SetTagOptions(*args): return _cspace.iShaderManager_SetTagOptions(*args)
    def GetTagOptions(*args): return _cspace.iShaderManager_GetTagOptions(*args)
    def GetTags(*args): return _cspace.iShaderManager_GetTags(*args)
    def __del__(self, destroy=_cspace.delete_iShaderManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class iShaderManagerPtr(iShaderManager):
    def __init__(self, this):
        _swig_setattr(self, iShaderManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iShaderManager, 'thisown', 0)
        _swig_setattr(self, iShaderManager,self.__class__,iShaderManager)
_cspace.iShaderManager_swigregister(iShaderManagerPtr)

class iShaderRenderInterface(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iShaderRenderInterface, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iShaderRenderInterface, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iShaderRenderInterface instance at %s>" % (self.this,)
    def GetPrivateObject(*args): return _cspace.iShaderRenderInterface_GetPrivateObject(*args)
    def __del__(self, destroy=_cspace.delete_iShaderRenderInterface):
        try:
            if self.thisown: destroy(self)
        except: pass

class iShaderRenderInterfacePtr(iShaderRenderInterface):
    def __init__(self, this):
        _swig_setattr(self, iShaderRenderInterface, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iShaderRenderInterface, 'thisown', 0)
        _swig_setattr(self, iShaderRenderInterface,self.__class__,iShaderRenderInterface)
_cspace.iShaderRenderInterface_swigregister(iShaderRenderInterfacePtr)

class iShader(iShaderVariableContext):
    __swig_setmethods__ = {}
    for _s in [iShaderVariableContext]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iShader, name, value)
    __swig_getmethods__ = {}
    for _s in [iShaderVariableContext]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iShader, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iShader instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iShader_QueryObject(*args)
    def GetFileName(*args): return _cspace.iShader_GetFileName(*args)
    def SetFileName(*args): return _cspace.iShader_SetFileName(*args)
    def GetTicket(*args): return _cspace.iShader_GetTicket(*args)
    def GetNumberOfPasses(*args): return _cspace.iShader_GetNumberOfPasses(*args)
    def ActivatePass(*args): return _cspace.iShader_ActivatePass(*args)
    def SetupPass(*args): return _cspace.iShader_SetupPass(*args)
    def TeardownPass(*args): return _cspace.iShader_TeardownPass(*args)
    def DeactivatePass(*args): return _cspace.iShader_DeactivatePass(*args)
    def __del__(self, destroy=_cspace.delete_iShader):
        try:
            if self.thisown: destroy(self)
        except: pass

class iShaderPtr(iShader):
    def __init__(self, this):
        _swig_setattr(self, iShader, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iShader, 'thisown', 0)
        _swig_setattr(self, iShader,self.__class__,iShader)
_cspace.iShader_swigregister(iShaderPtr)

class iShaderPriorityList(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iShaderPriorityList, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iShaderPriorityList, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iShaderPriorityList instance at %s>" % (self.this,)
    def GetCount(*args): return _cspace.iShaderPriorityList_GetCount(*args)
    def GetPriority(*args): return _cspace.iShaderPriorityList_GetPriority(*args)
    def __del__(self, destroy=_cspace.delete_iShaderPriorityList):
        try:
            if self.thisown: destroy(self)
        except: pass

class iShaderPriorityListPtr(iShaderPriorityList):
    def __init__(self, this):
        _swig_setattr(self, iShaderPriorityList, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iShaderPriorityList, 'thisown', 0)
        _swig_setattr(self, iShaderPriorityList,self.__class__,iShaderPriorityList)
_cspace.iShaderPriorityList_swigregister(iShaderPriorityListPtr)

class iShaderCompiler(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iShaderCompiler, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iShaderCompiler, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iShaderCompiler instance at %s>" % (self.this,)
    def GetName(*args): return _cspace.iShaderCompiler_GetName(*args)
    def CompileShader(*args): return _cspace.iShaderCompiler_CompileShader(*args)
    def ValidateTemplate(*args): return _cspace.iShaderCompiler_ValidateTemplate(*args)
    def IsTemplateToCompiler(*args): return _cspace.iShaderCompiler_IsTemplateToCompiler(*args)
    def GetPriorities(*args): return _cspace.iShaderCompiler_GetPriorities(*args)
    def __del__(self, destroy=_cspace.delete_iShaderCompiler):
        try:
            if self.thisown: destroy(self)
        except: pass

class iShaderCompilerPtr(iShaderCompiler):
    def __init__(self, this):
        _swig_setattr(self, iShaderCompiler, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iShaderCompiler, 'thisown', 0)
        _swig_setattr(self, iShaderCompiler,self.__class__,iShaderCompiler)
_cspace.iShaderCompiler_swigregister(iShaderCompilerPtr)

class iTextureHandle(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTextureHandle, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTextureHandle, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTextureHandle instance at %s>" % (self.this,)
    def GetFlags(*args): return _cspace.iTextureHandle_GetFlags(*args)
    def SetKeyColor(*args): return _cspace.iTextureHandle_SetKeyColor(*args)
    def GetKeyColorStatus(*args): return _cspace.iTextureHandle_GetKeyColorStatus(*args)
    def GetKeyColor(*args): return _cspace.iTextureHandle_GetKeyColor(*args)
    CS_TEX_IMG_1D = _cspace.iTextureHandle_CS_TEX_IMG_1D
    CS_TEX_IMG_2D = _cspace.iTextureHandle_CS_TEX_IMG_2D
    CS_TEX_IMG_3D = _cspace.iTextureHandle_CS_TEX_IMG_3D
    CS_TEX_IMG_CUBEMAP = _cspace.iTextureHandle_CS_TEX_IMG_CUBEMAP
    CS_TEXTURE_CUBE_POS_X = _cspace.iTextureHandle_CS_TEXTURE_CUBE_POS_X
    CS_TEXTURE_CUBE_NEG_X = _cspace.iTextureHandle_CS_TEXTURE_CUBE_NEG_X
    CS_TEXTURE_CUBE_POS_Y = _cspace.iTextureHandle_CS_TEXTURE_CUBE_POS_Y
    CS_TEXTURE_CUBE_NEG_Y = _cspace.iTextureHandle_CS_TEXTURE_CUBE_NEG_Y
    CS_TEXTURE_CUBE_POS_Z = _cspace.iTextureHandle_CS_TEXTURE_CUBE_POS_Z
    CS_TEXTURE_CUBE_NEG_Z = _cspace.iTextureHandle_CS_TEXTURE_CUBE_NEG_Z
    def GetRendererDimensions(*args): return _cspace.iTextureHandle_GetRendererDimensions(*args)
    def GetOriginalDimensions(*args): return _cspace.iTextureHandle_GetOriginalDimensions(*args)
    def GetTextureTarget(*args): return _cspace.iTextureHandle_GetTextureTarget(*args)
    def Blit(*args): return _cspace.iTextureHandle_Blit(*args)
    def GetImageName(*args): return _cspace.iTextureHandle_GetImageName(*args)
    def GetMeanColor(*args): return _cspace.iTextureHandle_GetMeanColor(*args)
    def GetCacheData(*args): return _cspace.iTextureHandle_GetCacheData(*args)
    def SetCacheData(*args): return _cspace.iTextureHandle_SetCacheData(*args)
    def GetPrivateObject(*args): return _cspace.iTextureHandle_GetPrivateObject(*args)
    def GetAlphaMap(*args): return _cspace.iTextureHandle_GetAlphaMap(*args)
    def GetAlphaType(*args): return _cspace.iTextureHandle_GetAlphaType(*args)
    def Precache(*args): return _cspace.iTextureHandle_Precache(*args)
    def SetTextureClass(*args): return _cspace.iTextureHandle_SetTextureClass(*args)
    def GetTextureClass(*args): return _cspace.iTextureHandle_GetTextureClass(*args)
    def SetAlphaType(*args): return _cspace.iTextureHandle_SetAlphaType(*args)
    def __del__(self, destroy=_cspace.delete_iTextureHandle):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTextureHandle_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTextureHandle_scfGetVersion)

class iTextureHandlePtr(iTextureHandle):
    def __init__(self, this):
        _swig_setattr(self, iTextureHandle, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTextureHandle, 'thisown', 0)
        _swig_setattr(self, iTextureHandle,self.__class__,iTextureHandle)
_cspace.iTextureHandle_swigregister(iTextureHandlePtr)

iTextureHandle_scfGetVersion = _cspace.iTextureHandle_scfGetVersion

CS_TEXTURE_2D = _cspace.CS_TEXTURE_2D
CS_TEXTURE_3D = _cspace.CS_TEXTURE_3D
CS_TEXTURE_DITHER = _cspace.CS_TEXTURE_DITHER
CS_TEXTURE_NOMIPMAPS = _cspace.CS_TEXTURE_NOMIPMAPS
CS_TEXTURE_CLAMP = _cspace.CS_TEXTURE_CLAMP
CS_TEXTURE_NOFILTER = _cspace.CS_TEXTURE_NOFILTER
class iRendererLightmap(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iRendererLightmap, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iRendererLightmap, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iRendererLightmap instance at %s>" % (self.this,)
    def GetSLMCoords(*args): return _cspace.iRendererLightmap_GetSLMCoords(*args)
    def SetData(*args): return _cspace.iRendererLightmap_SetData(*args)
    def SetLightCellSize(*args): return _cspace.iRendererLightmap_SetLightCellSize(*args)
    def __del__(self, destroy=_cspace.delete_iRendererLightmap):
        try:
            if self.thisown: destroy(self)
        except: pass

class iRendererLightmapPtr(iRendererLightmap):
    def __init__(self, this):
        _swig_setattr(self, iRendererLightmap, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iRendererLightmap, 'thisown', 0)
        _swig_setattr(self, iRendererLightmap,self.__class__,iRendererLightmap)
_cspace.iRendererLightmap_swigregister(iRendererLightmapPtr)

class iSuperLightmap(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSuperLightmap, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSuperLightmap, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSuperLightmap instance at %s>" % (self.this,)
    def RegisterLightmap(*args): return _cspace.iSuperLightmap_RegisterLightmap(*args)
    def Dump(*args): return _cspace.iSuperLightmap_Dump(*args)
    def GetTexture(*args): return _cspace.iSuperLightmap_GetTexture(*args)
    def __del__(self, destroy=_cspace.delete_iSuperLightmap):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSuperLightmapPtr(iSuperLightmap):
    def __init__(self, this):
        _swig_setattr(self, iSuperLightmap, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSuperLightmap, 'thisown', 0)
        _swig_setattr(self, iSuperLightmap,self.__class__,iSuperLightmap)
_cspace.iSuperLightmap_swigregister(iSuperLightmapPtr)

class iTextureManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTextureManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTextureManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTextureManager instance at %s>" % (self.this,)
    def RegisterTexture(*args): return _cspace.iTextureManager_RegisterTexture(*args)
    def RegisterMaterial(*args): return _cspace.iTextureManager_RegisterMaterial(*args)
    def FreeMaterials(*args): return _cspace.iTextureManager_FreeMaterials(*args)
    def GetTextureFormat(*args): return _cspace.iTextureManager_GetTextureFormat(*args)
    def CreateSuperLightmap(*args): return _cspace.iTextureManager_CreateSuperLightmap(*args)
    def GetMaxTextureSize(*args): return _cspace.iTextureManager_GetMaxTextureSize(*args)
    def GetLightmapRendererCoords(*args): return _cspace.iTextureManager_GetLightmapRendererCoords(*args)
    def __del__(self, destroy=_cspace.delete_iTextureManager):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTextureManager_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTextureManager_scfGetVersion)

class iTextureManagerPtr(iTextureManager):
    def __init__(self, this):
        _swig_setattr(self, iTextureManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTextureManager, 'thisown', 0)
        _swig_setattr(self, iTextureManager,self.__class__,iTextureManager)
_cspace.iTextureManager_swigregister(iTextureManagerPtr)

iTextureManager_scfGetVersion = _cspace.iTextureManager_scfGetVersion

CS_DEFMAT_DIFFUSE = _cspace.CS_DEFMAT_DIFFUSE
CS_DEFMAT_AMBIENT = _cspace.CS_DEFMAT_AMBIENT
CS_DEFMAT_REFLECTION = _cspace.CS_DEFMAT_REFLECTION
CS_MATERIAL_VARNAME_DIFFUSE = _cspace.CS_MATERIAL_VARNAME_DIFFUSE
CS_MATERIAL_VARNAME_AMBIENT = _cspace.CS_MATERIAL_VARNAME_AMBIENT
CS_MATERIAL_VARNAME_REFLECTION = _cspace.CS_MATERIAL_VARNAME_REFLECTION
CS_MATERIAL_VARNAME_FLATCOLOR = _cspace.CS_MATERIAL_VARNAME_FLATCOLOR
CS_MATERIAL_TEXTURE_DIFFUSE = _cspace.CS_MATERIAL_TEXTURE_DIFFUSE
CS_MATERIAL_TEXTURE_LAYER1 = _cspace.CS_MATERIAL_TEXTURE_LAYER1
CS_MATERIAL_TEXTURE_LAYER2 = _cspace.CS_MATERIAL_TEXTURE_LAYER2
CS_MATERIAL_TEXTURE_LAYER3 = _cspace.CS_MATERIAL_TEXTURE_LAYER3
CS_MATERIAL_TEXTURE_LAYER4 = _cspace.CS_MATERIAL_TEXTURE_LAYER4
class csTextureLayer(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csTextureLayer, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csTextureLayer, name)
    def __repr__(self):
        return "<C csTextureLayer instance at %s>" % (self.this,)
    __swig_setmethods__["txt_handle"] = _cspace.csTextureLayer_txt_handle_set
    __swig_getmethods__["txt_handle"] = _cspace.csTextureLayer_txt_handle_get
    if _newclass:txt_handle = property(_cspace.csTextureLayer_txt_handle_get, _cspace.csTextureLayer_txt_handle_set)
    __swig_setmethods__["mode"] = _cspace.csTextureLayer_mode_set
    __swig_getmethods__["mode"] = _cspace.csTextureLayer_mode_get
    if _newclass:mode = property(_cspace.csTextureLayer_mode_get, _cspace.csTextureLayer_mode_set)
    __swig_setmethods__["uscale"] = _cspace.csTextureLayer_uscale_set
    __swig_getmethods__["uscale"] = _cspace.csTextureLayer_uscale_get
    if _newclass:uscale = property(_cspace.csTextureLayer_uscale_get, _cspace.csTextureLayer_uscale_set)
    __swig_setmethods__["vscale"] = _cspace.csTextureLayer_vscale_set
    __swig_getmethods__["vscale"] = _cspace.csTextureLayer_vscale_get
    if _newclass:vscale = property(_cspace.csTextureLayer_vscale_get, _cspace.csTextureLayer_vscale_set)
    __swig_setmethods__["ushift"] = _cspace.csTextureLayer_ushift_set
    __swig_getmethods__["ushift"] = _cspace.csTextureLayer_ushift_get
    if _newclass:ushift = property(_cspace.csTextureLayer_ushift_get, _cspace.csTextureLayer_ushift_set)
    __swig_setmethods__["vshift"] = _cspace.csTextureLayer_vshift_set
    __swig_getmethods__["vshift"] = _cspace.csTextureLayer_vshift_get
    if _newclass:vshift = property(_cspace.csTextureLayer_vshift_get, _cspace.csTextureLayer_vshift_set)
    def __init__(self, *args):
        _swig_setattr(self, csTextureLayer, 'this', _cspace.new_csTextureLayer(*args))
        _swig_setattr(self, csTextureLayer, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csTextureLayer):
        try:
            if self.thisown: destroy(self)
        except: pass

class csTextureLayerPtr(csTextureLayer):
    def __init__(self, this):
        _swig_setattr(self, csTextureLayer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csTextureLayer, 'thisown', 0)
        _swig_setattr(self, csTextureLayer,self.__class__,csTextureLayer)
_cspace.csTextureLayer_swigregister(csTextureLayerPtr)

class iMaterial(iShaderVariableContext):
    __swig_setmethods__ = {}
    for _s in [iShaderVariableContext]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMaterial, name, value)
    __swig_getmethods__ = {}
    for _s in [iShaderVariableContext]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMaterial, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMaterial instance at %s>" % (self.this,)
    def SetShader(*args): return _cspace.iMaterial_SetShader(*args)
    def GetShader(*args): return _cspace.iMaterial_GetShader(*args)
    def GetShaders(*args): return _cspace.iMaterial_GetShaders(*args)
    def GetTexture(*args): return _cspace.iMaterial_GetTexture(*args)
    def GetTextureLayerCount(*args): return _cspace.iMaterial_GetTextureLayerCount(*args)
    def GetTextureLayer(*args): return _cspace.iMaterial_GetTextureLayer(*args)
    def GetFlatColor(*args): return _cspace.iMaterial_GetFlatColor(*args)
    def SetFlatColor(*args): return _cspace.iMaterial_SetFlatColor(*args)
    def GetReflection(*args): return _cspace.iMaterial_GetReflection(*args)
    def SetReflection(*args): return _cspace.iMaterial_SetReflection(*args)
    def __del__(self, destroy=_cspace.delete_iMaterial):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iMaterial_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iMaterial_scfGetVersion)

class iMaterialPtr(iMaterial):
    def __init__(self, this):
        _swig_setattr(self, iMaterial, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMaterial, 'thisown', 0)
        _swig_setattr(self, iMaterial,self.__class__,iMaterial)
_cspace.iMaterial_swigregister(iMaterialPtr)

iMaterial_scfGetVersion = _cspace.iMaterial_scfGetVersion

class iMaterialHandle(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iMaterialHandle, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iMaterialHandle, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iMaterialHandle instance at %s>" % (self.this,)
    def GetShader(*args): return _cspace.iMaterialHandle_GetShader(*args)
    def GetTexture(*args): return _cspace.iMaterialHandle_GetTexture(*args)
    def GetFlatColor(*args): return _cspace.iMaterialHandle_GetFlatColor(*args)
    def GetReflection(*args): return _cspace.iMaterialHandle_GetReflection(*args)
    def __del__(self, destroy=_cspace.delete_iMaterialHandle):
        try:
            if self.thisown: destroy(self)
        except: pass

class iMaterialHandlePtr(iMaterialHandle):
    def __init__(self, this):
        _swig_setattr(self, iMaterialHandle, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iMaterialHandle, 'thisown', 0)
        _swig_setattr(self, iMaterialHandle,self.__class__,iMaterialHandle)
_cspace.iMaterialHandle_swigregister(iMaterialHandlePtr)

CS_POS_BY_FRAME = _cspace.CS_POS_BY_FRAME
CS_POS_BY_TIME = _cspace.CS_POS_BY_TIME
CS_DECODE_SPAN = _cspace.CS_DECODE_SPAN
CS_DYNAMIC_FRAMESIZE = _cspace.CS_DYNAMIC_FRAMESIZE
CS_STREAMTYPE_AUDIO = _cspace.CS_STREAMTYPE_AUDIO
CS_STREAMTYPE_VIDEO = _cspace.CS_STREAMTYPE_VIDEO
CS_STREAMTYPE_MIDI = _cspace.CS_STREAMTYPE_MIDI
CS_STREAMTYPE_TEXT = _cspace.CS_STREAMTYPE_TEXT
class csStreamDescription(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csStreamDescription, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csStreamDescription, name)
    def __repr__(self):
        return "<C csStreamDescription instance at %s>" % (self.this,)
    __swig_setmethods__["type"] = _cspace.csStreamDescription_type_set
    __swig_getmethods__["type"] = _cspace.csStreamDescription_type_get
    if _newclass:type = property(_cspace.csStreamDescription_type_get, _cspace.csStreamDescription_type_set)
    __swig_setmethods__["codec"] = _cspace.csStreamDescription_codec_set
    __swig_getmethods__["codec"] = _cspace.csStreamDescription_codec_get
    if _newclass:codec = property(_cspace.csStreamDescription_codec_get, _cspace.csStreamDescription_codec_set)
    __swig_getmethods__["name"] = _cspace.csStreamDescription_name_get
    if _newclass:name = property(_cspace.csStreamDescription_name_get)
    def __init__(self, *args):
        _swig_setattr(self, csStreamDescription, 'this', _cspace.new_csStreamDescription(*args))
        _swig_setattr(self, csStreamDescription, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csStreamDescription):
        try:
            if self.thisown: destroy(self)
        except: pass

class csStreamDescriptionPtr(csStreamDescription):
    def __init__(self, this):
        _swig_setattr(self, csStreamDescription, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csStreamDescription, 'thisown', 0)
        _swig_setattr(self, csStreamDescription,self.__class__,csStreamDescription)
_cspace.csStreamDescription_swigregister(csStreamDescriptionPtr)

class csVideoStreamDescription(csStreamDescription):
    __swig_setmethods__ = {}
    for _s in [csStreamDescription]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csVideoStreamDescription, name, value)
    __swig_getmethods__ = {}
    for _s in [csStreamDescription]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csVideoStreamDescription, name)
    def __repr__(self):
        return "<C csVideoStreamDescription instance at %s>" % (self.this,)
    __swig_setmethods__["colordepth"] = _cspace.csVideoStreamDescription_colordepth_set
    __swig_getmethods__["colordepth"] = _cspace.csVideoStreamDescription_colordepth_get
    if _newclass:colordepth = property(_cspace.csVideoStreamDescription_colordepth_get, _cspace.csVideoStreamDescription_colordepth_set)
    __swig_setmethods__["framecount"] = _cspace.csVideoStreamDescription_framecount_set
    __swig_getmethods__["framecount"] = _cspace.csVideoStreamDescription_framecount_get
    if _newclass:framecount = property(_cspace.csVideoStreamDescription_framecount_get, _cspace.csVideoStreamDescription_framecount_set)
    __swig_setmethods__["width"] = _cspace.csVideoStreamDescription_width_set
    __swig_getmethods__["width"] = _cspace.csVideoStreamDescription_width_get
    if _newclass:width = property(_cspace.csVideoStreamDescription_width_get, _cspace.csVideoStreamDescription_width_set)
    __swig_setmethods__["height"] = _cspace.csVideoStreamDescription_height_set
    __swig_getmethods__["height"] = _cspace.csVideoStreamDescription_height_get
    if _newclass:height = property(_cspace.csVideoStreamDescription_height_get, _cspace.csVideoStreamDescription_height_set)
    __swig_setmethods__["framerate"] = _cspace.csVideoStreamDescription_framerate_set
    __swig_getmethods__["framerate"] = _cspace.csVideoStreamDescription_framerate_get
    if _newclass:framerate = property(_cspace.csVideoStreamDescription_framerate_get, _cspace.csVideoStreamDescription_framerate_set)
    __swig_setmethods__["duration"] = _cspace.csVideoStreamDescription_duration_set
    __swig_getmethods__["duration"] = _cspace.csVideoStreamDescription_duration_get
    if _newclass:duration = property(_cspace.csVideoStreamDescription_duration_get, _cspace.csVideoStreamDescription_duration_set)
    def __init__(self, *args):
        _swig_setattr(self, csVideoStreamDescription, 'this', _cspace.new_csVideoStreamDescription(*args))
        _swig_setattr(self, csVideoStreamDescription, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csVideoStreamDescription):
        try:
            if self.thisown: destroy(self)
        except: pass

class csVideoStreamDescriptionPtr(csVideoStreamDescription):
    def __init__(self, this):
        _swig_setattr(self, csVideoStreamDescription, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csVideoStreamDescription, 'thisown', 0)
        _swig_setattr(self, csVideoStreamDescription,self.__class__,csVideoStreamDescription)
_cspace.csVideoStreamDescription_swigregister(csVideoStreamDescriptionPtr)

class csAudioStreamDescription(csStreamDescription):
    __swig_setmethods__ = {}
    for _s in [csStreamDescription]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csAudioStreamDescription, name, value)
    __swig_getmethods__ = {}
    for _s in [csStreamDescription]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csAudioStreamDescription, name)
    def __repr__(self):
        return "<C csAudioStreamDescription instance at %s>" % (self.this,)
    __swig_setmethods__["formattag"] = _cspace.csAudioStreamDescription_formattag_set
    __swig_getmethods__["formattag"] = _cspace.csAudioStreamDescription_formattag_get
    if _newclass:formattag = property(_cspace.csAudioStreamDescription_formattag_get, _cspace.csAudioStreamDescription_formattag_set)
    __swig_setmethods__["channels"] = _cspace.csAudioStreamDescription_channels_set
    __swig_getmethods__["channels"] = _cspace.csAudioStreamDescription_channels_get
    if _newclass:channels = property(_cspace.csAudioStreamDescription_channels_get, _cspace.csAudioStreamDescription_channels_set)
    __swig_setmethods__["samplespersecond"] = _cspace.csAudioStreamDescription_samplespersecond_set
    __swig_getmethods__["samplespersecond"] = _cspace.csAudioStreamDescription_samplespersecond_get
    if _newclass:samplespersecond = property(_cspace.csAudioStreamDescription_samplespersecond_get, _cspace.csAudioStreamDescription_samplespersecond_set)
    __swig_setmethods__["bitspersample"] = _cspace.csAudioStreamDescription_bitspersample_set
    __swig_getmethods__["bitspersample"] = _cspace.csAudioStreamDescription_bitspersample_get
    if _newclass:bitspersample = property(_cspace.csAudioStreamDescription_bitspersample_get, _cspace.csAudioStreamDescription_bitspersample_set)
    __swig_setmethods__["duration"] = _cspace.csAudioStreamDescription_duration_set
    __swig_getmethods__["duration"] = _cspace.csAudioStreamDescription_duration_get
    if _newclass:duration = property(_cspace.csAudioStreamDescription_duration_get, _cspace.csAudioStreamDescription_duration_set)
    def __init__(self, *args):
        _swig_setattr(self, csAudioStreamDescription, 'this', _cspace.new_csAudioStreamDescription(*args))
        _swig_setattr(self, csAudioStreamDescription, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csAudioStreamDescription):
        try:
            if self.thisown: destroy(self)
        except: pass

class csAudioStreamDescriptionPtr(csAudioStreamDescription):
    def __init__(self, this):
        _swig_setattr(self, csAudioStreamDescription, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csAudioStreamDescription, 'thisown', 0)
        _swig_setattr(self, csAudioStreamDescription,self.__class__,csAudioStreamDescription)
_cspace.csAudioStreamDescription_swigregister(csAudioStreamDescriptionPtr)

class iStreamIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iStreamIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iStreamIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iStreamIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iStreamIterator_HasNext(*args)
    def Next(*args): return _cspace.iStreamIterator_Next(*args)
    def __del__(self, destroy=_cspace.delete_iStreamIterator):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iStreamIterator_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iStreamIterator_scfGetVersion)

class iStreamIteratorPtr(iStreamIterator):
    def __init__(self, this):
        _swig_setattr(self, iStreamIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iStreamIterator, 'thisown', 0)
        _swig_setattr(self, iStreamIterator,self.__class__,iStreamIterator)
_cspace.iStreamIterator_swigregister(iStreamIteratorPtr)

iStreamIterator_scfGetVersion = _cspace.iStreamIterator_scfGetVersion

class iStreamFormat(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iStreamFormat, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iStreamFormat, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iStreamFormat instance at %s>" % (self.this,)
    def GetCaps(*args): return _cspace.iStreamFormat_GetCaps(*args)
    def GetStreamIterator(*args): return _cspace.iStreamFormat_GetStreamIterator(*args)
    def Select(*args): return _cspace.iStreamFormat_Select(*args)
    def NextFrame(*args): return _cspace.iStreamFormat_NextFrame(*args)
    def Load(*args): return _cspace.iStreamFormat_Load(*args)
    def Unload(*args): return _cspace.iStreamFormat_Unload(*args)
    def __del__(self, destroy=_cspace.delete_iStreamFormat):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iStreamFormat_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iStreamFormat_scfGetVersion)

class iStreamFormatPtr(iStreamFormat):
    def __init__(self, this):
        _swig_setattr(self, iStreamFormat, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iStreamFormat, 'thisown', 0)
        _swig_setattr(self, iStreamFormat,self.__class__,iStreamFormat)
_cspace.iStreamFormat_swigregister(iStreamFormatPtr)

iStreamFormat_scfGetVersion = _cspace.iStreamFormat_scfGetVersion

class iStream(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iStream, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iStream, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iStream instance at %s>" % (self.this,)
    def GetStreamDescription(*args): return _cspace.iStream_GetStreamDescription(*args)
    def GotoFrame(*args): return _cspace.iStream_GotoFrame(*args)
    def GotoTime(*args): return _cspace.iStream_GotoTime(*args)
    def SetPlayMethod(*args): return _cspace.iStream_SetPlayMethod(*args)
    def NextFrame(*args): return _cspace.iStream_NextFrame(*args)
    def __del__(self, destroy=_cspace.delete_iStream):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iStream_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iStream_scfGetVersion)

class iStreamPtr(iStream):
    def __init__(self, this):
        _swig_setattr(self, iStream, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iStream, 'thisown', 0)
        _swig_setattr(self, iStream,self.__class__,iStream)
_cspace.iStream_swigregister(iStreamPtr)

iStream_scfGetVersion = _cspace.iStream_scfGetVersion

class iVideoStream(iStream):
    __swig_setmethods__ = {}
    for _s in [iStream]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iVideoStream, name, value)
    __swig_getmethods__ = {}
    for _s in [iStream]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iVideoStream, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iVideoStream instance at %s>" % (self.this,)
    def GetStreamDescription(*args): return _cspace.iVideoStream_GetStreamDescription(*args)
    def SetRect(*args): return _cspace.iVideoStream_SetRect(*args)
    def SetFXMode(*args): return _cspace.iVideoStream_SetFXMode(*args)
    def NextFrameGetMaterial(*args): return _cspace.iVideoStream_NextFrameGetMaterial(*args)
    def __del__(self, destroy=_cspace.delete_iVideoStream):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iVideoStream_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iVideoStream_scfGetVersion)

class iVideoStreamPtr(iVideoStream):
    def __init__(self, this):
        _swig_setattr(self, iVideoStream, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iVideoStream, 'thisown', 0)
        _swig_setattr(self, iVideoStream,self.__class__,iVideoStream)
_cspace.iVideoStream_swigregister(iVideoStreamPtr)

iVideoStream_scfGetVersion = _cspace.iVideoStream_scfGetVersion

class iAudioStream(iStream):
    __swig_setmethods__ = {}
    for _s in [iStream]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iAudioStream, name, value)
    __swig_getmethods__ = {}
    for _s in [iStream]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iAudioStream, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iAudioStream instance at %s>" % (self.this,)
    def GetStreamDescription(*args): return _cspace.iAudioStream_GetStreamDescription(*args)
    def __del__(self, destroy=_cspace.delete_iAudioStream):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iAudioStream_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iAudioStream_scfGetVersion)

class iAudioStreamPtr(iAudioStream):
    def __init__(self, this):
        _swig_setattr(self, iAudioStream, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iAudioStream, 'thisown', 0)
        _swig_setattr(self, iAudioStream,self.__class__,iAudioStream)
_cspace.iAudioStream_swigregister(iAudioStreamPtr)

iAudioStream_scfGetVersion = _cspace.iAudioStream_scfGetVersion

CS_CODECFORMAT_RGB_CHANNEL = _cspace.CS_CODECFORMAT_RGB_CHANNEL
CS_CODECFORMAT_RGBA_CHANNEL = _cspace.CS_CODECFORMAT_RGBA_CHANNEL
CS_CODECFORMAT_YUV_CHANNEL = _cspace.CS_CODECFORMAT_YUV_CHANNEL
CS_CODECFORMAT_RGB_INTERLEAVED = _cspace.CS_CODECFORMAT_RGB_INTERLEAVED
CS_CODECFORMAT_RGBA_INTERLEAVED = _cspace.CS_CODECFORMAT_RGBA_INTERLEAVED
CS_CODECFORMAT_YUV_INTERLEAVED = _cspace.CS_CODECFORMAT_YUV_INTERLEAVED
class csCodecDescription(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csCodecDescription, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csCodecDescription, name)
    def __repr__(self):
        return "<C csCodecDescription instance at %s>" % (self.this,)
    __swig_setmethods__["codec"] = _cspace.csCodecDescription_codec_set
    __swig_getmethods__["codec"] = _cspace.csCodecDescription_codec_get
    if _newclass:codec = property(_cspace.csCodecDescription_codec_get, _cspace.csCodecDescription_codec_set)
    __swig_setmethods__["bEncode"] = _cspace.csCodecDescription_bEncode_set
    __swig_getmethods__["bEncode"] = _cspace.csCodecDescription_bEncode_get
    if _newclass:bEncode = property(_cspace.csCodecDescription_bEncode_get, _cspace.csCodecDescription_bEncode_set)
    __swig_setmethods__["bDecode"] = _cspace.csCodecDescription_bDecode_set
    __swig_getmethods__["bDecode"] = _cspace.csCodecDescription_bDecode_get
    if _newclass:bDecode = property(_cspace.csCodecDescription_bDecode_get, _cspace.csCodecDescription_bDecode_set)
    __swig_setmethods__["decodeoutput"] = _cspace.csCodecDescription_decodeoutput_set
    __swig_getmethods__["decodeoutput"] = _cspace.csCodecDescription_decodeoutput_get
    if _newclass:decodeoutput = property(_cspace.csCodecDescription_decodeoutput_get, _cspace.csCodecDescription_decodeoutput_set)
    __swig_setmethods__["encodeinput"] = _cspace.csCodecDescription_encodeinput_set
    __swig_getmethods__["encodeinput"] = _cspace.csCodecDescription_encodeinput_get
    if _newclass:encodeinput = property(_cspace.csCodecDescription_encodeinput_get, _cspace.csCodecDescription_encodeinput_set)
    def __init__(self, *args):
        _swig_setattr(self, csCodecDescription, 'this', _cspace.new_csCodecDescription(*args))
        _swig_setattr(self, csCodecDescription, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csCodecDescription):
        try:
            if self.thisown: destroy(self)
        except: pass

class csCodecDescriptionPtr(csCodecDescription):
    def __init__(self, this):
        _swig_setattr(self, csCodecDescription, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csCodecDescription, 'thisown', 0)
        _swig_setattr(self, csCodecDescription,self.__class__,csCodecDescription)
_cspace.csCodecDescription_swigregister(csCodecDescriptionPtr)

CS_IMGFMT_MASK = _cspace.CS_IMGFMT_MASK
CS_IMGFMT_NONE = _cspace.CS_IMGFMT_NONE
CS_IMGFMT_TRUECOLOR = _cspace.CS_IMGFMT_TRUECOLOR
CS_IMGFMT_PALETTED8 = _cspace.CS_IMGFMT_PALETTED8
CS_IMGFMT_ANY = _cspace.CS_IMGFMT_ANY
CS_IMGFMT_ALPHA = _cspace.CS_IMGFMT_ALPHA
CS_IMGFMT_INVALID = _cspace.CS_IMGFMT_INVALID
csimg2D = _cspace.csimg2D
csimg3D = _cspace.csimg3D
csimgCube = _cspace.csimgCube
class iImage(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iImage, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iImage, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iImage instance at %s>" % (self.this,)
    def GetImageData(*args): return _cspace.iImage_GetImageData(*args)
    def GetWidth(*args): return _cspace.iImage_GetWidth(*args)
    def GetHeight(*args): return _cspace.iImage_GetHeight(*args)
    def GetDepth(*args): return _cspace.iImage_GetDepth(*args)
    def SetName(*args): return _cspace.iImage_SetName(*args)
    def GetName(*args): return _cspace.iImage_GetName(*args)
    def GetFormat(*args): return _cspace.iImage_GetFormat(*args)
    def GetPalette(*args): return _cspace.iImage_GetPalette(*args)
    def GetAlpha(*args): return _cspace.iImage_GetAlpha(*args)
    def HasKeyColor(*args): return _cspace.iImage_HasKeyColor(*args)
    def HasKeycolor(*args): return _cspace.iImage_HasKeycolor(*args)
    def GetKeyColor(*args): return _cspace.iImage_GetKeyColor(*args)
    def GetKeycolor(*args): return _cspace.iImage_GetKeycolor(*args)
    def HasMipmaps(*args): return _cspace.iImage_HasMipmaps(*args)
    def GetMipmap(*args): return _cspace.iImage_GetMipmap(*args)
    def GetRawFormat(*args): return _cspace.iImage_GetRawFormat(*args)
    def GetRawData(*args): return _cspace.iImage_GetRawData(*args)
    def GetImageType(*args): return _cspace.iImage_GetImageType(*args)
    def HasSubImages(*args): return _cspace.iImage_HasSubImages(*args)
    def GetSubImage(*args): return _cspace.iImage_GetSubImage(*args)
    def __del__(self, destroy=_cspace.delete_iImage):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iImage_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iImage_scfGetVersion)

class iImagePtr(iImage):
    def __init__(self, this):
        _swig_setattr(self, iImage, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iImage, 'thisown', 0)
        _swig_setattr(self, iImage,self.__class__,iImage)
_cspace.iImage_swigregister(iImagePtr)

iImage_scfGetVersion = _cspace.iImage_scfGetVersion

CS_IMAGEIO_LOAD = _cspace.CS_IMAGEIO_LOAD
CS_IMAGEIO_SAVE = _cspace.CS_IMAGEIO_SAVE
class csImageIOFileFormatDescription(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csImageIOFileFormatDescription, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csImageIOFileFormatDescription, name)
    def __repr__(self):
        return "<C csImageIOFileFormatDescription instance at %s>" % (self.this,)
    __swig_getmethods__["mime"] = _cspace.csImageIOFileFormatDescription_mime_get
    if _newclass:mime = property(_cspace.csImageIOFileFormatDescription_mime_get)
    __swig_getmethods__["subtype"] = _cspace.csImageIOFileFormatDescription_subtype_get
    if _newclass:subtype = property(_cspace.csImageIOFileFormatDescription_subtype_get)
    __swig_setmethods__["cap"] = _cspace.csImageIOFileFormatDescription_cap_set
    __swig_getmethods__["cap"] = _cspace.csImageIOFileFormatDescription_cap_get
    if _newclass:cap = property(_cspace.csImageIOFileFormatDescription_cap_get, _cspace.csImageIOFileFormatDescription_cap_set)
    def __init__(self, *args):
        _swig_setattr(self, csImageIOFileFormatDescription, 'this', _cspace.new_csImageIOFileFormatDescription(*args))
        _swig_setattr(self, csImageIOFileFormatDescription, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csImageIOFileFormatDescription):
        try:
            if self.thisown: destroy(self)
        except: pass

class csImageIOFileFormatDescriptionPtr(csImageIOFileFormatDescription):
    def __init__(self, this):
        _swig_setattr(self, csImageIOFileFormatDescription, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csImageIOFileFormatDescription, 'thisown', 0)
        _swig_setattr(self, csImageIOFileFormatDescription,self.__class__,csImageIOFileFormatDescription)
_cspace.csImageIOFileFormatDescription_swigregister(csImageIOFileFormatDescriptionPtr)

class iImageIO(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iImageIO, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iImageIO, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iImageIO instance at %s>" % (self.this,)
    def GetDescription(*args): return _cspace.iImageIO_GetDescription(*args)
    def Load(*args): return _cspace.iImageIO_Load(*args)
    def SetDithering(*args): return _cspace.iImageIO_SetDithering(*args)
    def Save(*args): return _cspace.iImageIO_Save(*args)
    def __del__(self, destroy=_cspace.delete_iImageIO):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iImageIO_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iImageIO_scfGetVersion)

class iImageIOPtr(iImageIO):
    def __init__(self, this):
        _swig_setattr(self, iImageIO, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iImageIO, 'thisown', 0)
        _swig_setattr(self, iImageIO,self.__class__,iImageIO)
_cspace.iImageIO_swigregister(iImageIOPtr)

iImageIO_scfGetVersion = _cspace.iImageIO_scfGetVersion

CS_REPORTER_SEVERITY_BUG = _cspace.CS_REPORTER_SEVERITY_BUG
CS_REPORTER_SEVERITY_ERROR = _cspace.CS_REPORTER_SEVERITY_ERROR
CS_REPORTER_SEVERITY_WARNING = _cspace.CS_REPORTER_SEVERITY_WARNING
CS_REPORTER_SEVERITY_NOTIFY = _cspace.CS_REPORTER_SEVERITY_NOTIFY
CS_REPORTER_SEVERITY_DEBUG = _cspace.CS_REPORTER_SEVERITY_DEBUG
class iReporterListener(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iReporterListener, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iReporterListener, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iReporterListener instance at %s>" % (self.this,)
    def Report(*args): return _cspace.iReporterListener_Report(*args)
    def __del__(self, destroy=_cspace.delete_iReporterListener):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iReporterListener_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iReporterListener_scfGetVersion)

class iReporterListenerPtr(iReporterListener):
    def __init__(self, this):
        _swig_setattr(self, iReporterListener, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iReporterListener, 'thisown', 0)
        _swig_setattr(self, iReporterListener,self.__class__,iReporterListener)
_cspace.iReporterListener_swigregister(iReporterListenerPtr)

iReporterListener_scfGetVersion = _cspace.iReporterListener_scfGetVersion

class iReporterIterator(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iReporterIterator, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iReporterIterator, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iReporterIterator instance at %s>" % (self.this,)
    def HasNext(*args): return _cspace.iReporterIterator_HasNext(*args)
    def Next(*args): return _cspace.iReporterIterator_Next(*args)
    def GetMessageSeverity(*args): return _cspace.iReporterIterator_GetMessageSeverity(*args)
    def GetMessageId(*args): return _cspace.iReporterIterator_GetMessageId(*args)
    def GetMessageDescription(*args): return _cspace.iReporterIterator_GetMessageDescription(*args)
    def __del__(self, destroy=_cspace.delete_iReporterIterator):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iReporterIterator_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iReporterIterator_scfGetVersion)

class iReporterIteratorPtr(iReporterIterator):
    def __init__(self, this):
        _swig_setattr(self, iReporterIterator, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iReporterIterator, 'thisown', 0)
        _swig_setattr(self, iReporterIterator,self.__class__,iReporterIterator)
_cspace.iReporterIterator_swigregister(iReporterIteratorPtr)

iReporterIterator_scfGetVersion = _cspace.iReporterIterator_scfGetVersion

class iReporter(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iReporter, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iReporter, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iReporter instance at %s>" % (self.this,)
    def Report(*args): return _cspace.iReporter_Report(*args)
    def Clear(*args): return _cspace.iReporter_Clear(*args)
    def GetMessageIterator(*args): return _cspace.iReporter_GetMessageIterator(*args)
    def AddReporterListener(*args): return _cspace.iReporter_AddReporterListener(*args)
    def RemoveReporterListener(*args): return _cspace.iReporter_RemoveReporterListener(*args)
    def FindReporterListener(*args): return _cspace.iReporter_FindReporterListener(*args)
    def ReportError(*args): return _cspace.iReporter_ReportError(*args)
    def ReportWarning(*args): return _cspace.iReporter_ReportWarning(*args)
    def ReportNotify(*args): return _cspace.iReporter_ReportNotify(*args)
    def ReportBug(*args): return _cspace.iReporter_ReportBug(*args)
    def ReportDebug(*args): return _cspace.iReporter_ReportDebug(*args)
    def __del__(self, destroy=_cspace.delete_iReporter):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iReporter_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iReporter_scfGetVersion)

class iReporterPtr(iReporter):
    def __init__(self, this):
        _swig_setattr(self, iReporter, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iReporter, 'thisown', 0)
        _swig_setattr(self, iReporter,self.__class__,iReporter)
_cspace.iReporter_swigregister(iReporterPtr)

iReporter_scfGetVersion = _cspace.iReporter_scfGetVersion

class csReporterHelper(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csReporterHelper, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csReporterHelper, name)
    def __repr__(self):
        return "<C csReporterHelper instance at %s>" % (self.this,)
    __swig_getmethods__["Report"] = lambda x: _cspace.csReporterHelper_Report
    if _newclass:Report = staticmethod(_cspace.csReporterHelper_Report)
    def __init__(self, *args):
        _swig_setattr(self, csReporterHelper, 'this', _cspace.new_csReporterHelper(*args))
        _swig_setattr(self, csReporterHelper, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csReporterHelper):
        try:
            if self.thisown: destroy(self)
        except: pass

class csReporterHelperPtr(csReporterHelper):
    def __init__(self, this):
        _swig_setattr(self, csReporterHelper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csReporterHelper, 'thisown', 0)
        _swig_setattr(self, csReporterHelper,self.__class__,csReporterHelper)
_cspace.csReporterHelper_swigregister(csReporterHelperPtr)

csReporterHelper_Report = _cspace.csReporterHelper_Report

csConPageUp = _cspace.csConPageUp
csConPageDown = _cspace.csConPageDown
csConVeryTop = _cspace.csConVeryTop
csConVeryBottom = _cspace.csConVeryBottom
csConNoCursor = _cspace.csConNoCursor
csConNormalCursor = _cspace.csConNormalCursor
csConInsertCursor = _cspace.csConInsertCursor
class iConsoleWatcher(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iConsoleWatcher, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iConsoleWatcher, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iConsoleWatcher instance at %s>" % (self.this,)
    def ConsoleVisibilityChanged(*args): return _cspace.iConsoleWatcher_ConsoleVisibilityChanged(*args)
    def __del__(self, destroy=_cspace.delete_iConsoleWatcher):
        try:
            if self.thisown: destroy(self)
        except: pass

class iConsoleWatcherPtr(iConsoleWatcher):
    def __init__(self, this):
        _swig_setattr(self, iConsoleWatcher, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iConsoleWatcher, 'thisown', 0)
        _swig_setattr(self, iConsoleWatcher,self.__class__,iConsoleWatcher)
_cspace.iConsoleWatcher_swigregister(iConsoleWatcherPtr)

class iConsoleOutput(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iConsoleOutput, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iConsoleOutput, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iConsoleOutput instance at %s>" % (self.this,)
    def PutText(*args): return _cspace.iConsoleOutput_PutText(*args)
    def GetLine(*args): return _cspace.iConsoleOutput_GetLine(*args)
    def Draw2D(*args): return _cspace.iConsoleOutput_Draw2D(*args)
    def Draw3D(*args): return _cspace.iConsoleOutput_Draw3D(*args)
    def Clear(*args): return _cspace.iConsoleOutput_Clear(*args)
    def SetBufferSize(*args): return _cspace.iConsoleOutput_SetBufferSize(*args)
    def GetTransparency(*args): return _cspace.iConsoleOutput_GetTransparency(*args)
    def SetTransparency(*args): return _cspace.iConsoleOutput_SetTransparency(*args)
    def GetFont(*args): return _cspace.iConsoleOutput_GetFont(*args)
    def SetFont(*args): return _cspace.iConsoleOutput_SetFont(*args)
    def GetTopLine(*args): return _cspace.iConsoleOutput_GetTopLine(*args)
    def ScrollTo(*args): return _cspace.iConsoleOutput_ScrollTo(*args)
    def GetCursorStyle(*args): return _cspace.iConsoleOutput_GetCursorStyle(*args)
    def SetCursorStyle(*args): return _cspace.iConsoleOutput_SetCursorStyle(*args)
    def SetVisible(*args): return _cspace.iConsoleOutput_SetVisible(*args)
    def GetVisible(*args): return _cspace.iConsoleOutput_GetVisible(*args)
    def AutoUpdate(*args): return _cspace.iConsoleOutput_AutoUpdate(*args)
    def SetCursorPos(*args): return _cspace.iConsoleOutput_SetCursorPos(*args)
    def GetMaxLineWidth(*args): return _cspace.iConsoleOutput_GetMaxLineWidth(*args)
    def RegisterWatcher(*args): return _cspace.iConsoleOutput_RegisterWatcher(*args)
    def PerformExtension(*args): return _cspace.iConsoleOutput_PerformExtension(*args)
    def __del__(self, destroy=_cspace.delete_iConsoleOutput):
        try:
            if self.thisown: destroy(self)
        except: pass

class iConsoleOutputPtr(iConsoleOutput):
    def __init__(self, this):
        _swig_setattr(self, iConsoleOutput, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iConsoleOutput, 'thisown', 0)
        _swig_setattr(self, iConsoleOutput,self.__class__,iConsoleOutput)
_cspace.iConsoleOutput_swigregister(iConsoleOutputPtr)

class iStandardReporterListener(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iStandardReporterListener, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iStandardReporterListener, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iStandardReporterListener instance at %s>" % (self.this,)
    def SetOutputConsole(*args): return _cspace.iStandardReporterListener_SetOutputConsole(*args)
    def SetNativeWindowManager(*args): return _cspace.iStandardReporterListener_SetNativeWindowManager(*args)
    def SetReporter(*args): return _cspace.iStandardReporterListener_SetReporter(*args)
    def SetDebugFile(*args): return _cspace.iStandardReporterListener_SetDebugFile(*args)
    def SetDefaults(*args): return _cspace.iStandardReporterListener_SetDefaults(*args)
    def SetMessageDestination(*args): return _cspace.iStandardReporterListener_SetMessageDestination(*args)
    def RemoveMessages(*args): return _cspace.iStandardReporterListener_RemoveMessages(*args)
    def ShowMessageID(*args): return _cspace.iStandardReporterListener_ShowMessageID(*args)
    def GetDebugFile(*args): return _cspace.iStandardReporterListener_GetDebugFile(*args)
    def __del__(self, destroy=_cspace.delete_iStandardReporterListener):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iStandardReporterListener_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iStandardReporterListener_scfGetVersion)

class iStandardReporterListenerPtr(iStandardReporterListener):
    def __init__(self, this):
        _swig_setattr(self, iStandardReporterListener, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iStandardReporterListener, 'thisown', 0)
        _swig_setattr(self, iStandardReporterListener,self.__class__,iStandardReporterListener)
_cspace.iStandardReporterListener_swigregister(iStandardReporterListenerPtr)

iStandardReporterListener_scfGetVersion = _cspace.iStandardReporterListener_scfGetVersion

class iView(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iView, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iView, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iView instance at %s>" % (self.this,)
    def GetEngine(*args): return _cspace.iView_GetEngine(*args)
    def SetEngine(*args): return _cspace.iView_SetEngine(*args)
    def GetCamera(*args): return _cspace.iView_GetCamera(*args)
    def SetCamera(*args): return _cspace.iView_SetCamera(*args)
    def GetContext(*args): return _cspace.iView_GetContext(*args)
    def SetContext(*args): return _cspace.iView_SetContext(*args)
    def SetRectangle(*args): return _cspace.iView_SetRectangle(*args)
    def ClearView(*args): return _cspace.iView_ClearView(*args)
    def AddViewVertex(*args): return _cspace.iView_AddViewVertex(*args)
    def RestrictClipperToScreen(*args): return _cspace.iView_RestrictClipperToScreen(*args)
    def UpdateClipper(*args): return _cspace.iView_UpdateClipper(*args)
    def GetClipper(*args): return _cspace.iView_GetClipper(*args)
    def Draw(*args): return _cspace.iView_Draw(*args)
    def SetAutoResize(*args): return _cspace.iView_SetAutoResize(*args)
    def __del__(self, destroy=_cspace.delete_iView):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iView_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iView_scfGetVersion)

class iViewPtr(iView):
    def __init__(self, this):
        _swig_setattr(self, iView, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iView, 'thisown', 0)
        _swig_setattr(self, iView,self.__class__,iView)
_cspace.iView_swigregister(iViewPtr)

iView_scfGetVersion = _cspace.iView_scfGetVersion

class csCollisionPair(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csCollisionPair, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csCollisionPair, name)
    def __repr__(self):
        return "<C csCollisionPair instance at %s>" % (self.this,)
    __swig_setmethods__["a1"] = _cspace.csCollisionPair_a1_set
    __swig_getmethods__["a1"] = _cspace.csCollisionPair_a1_get
    if _newclass:a1 = property(_cspace.csCollisionPair_a1_get, _cspace.csCollisionPair_a1_set)
    __swig_setmethods__["b1"] = _cspace.csCollisionPair_b1_set
    __swig_getmethods__["b1"] = _cspace.csCollisionPair_b1_get
    if _newclass:b1 = property(_cspace.csCollisionPair_b1_get, _cspace.csCollisionPair_b1_set)
    __swig_setmethods__["c1"] = _cspace.csCollisionPair_c1_set
    __swig_getmethods__["c1"] = _cspace.csCollisionPair_c1_get
    if _newclass:c1 = property(_cspace.csCollisionPair_c1_get, _cspace.csCollisionPair_c1_set)
    __swig_setmethods__["a2"] = _cspace.csCollisionPair_a2_set
    __swig_getmethods__["a2"] = _cspace.csCollisionPair_a2_get
    if _newclass:a2 = property(_cspace.csCollisionPair_a2_get, _cspace.csCollisionPair_a2_set)
    __swig_setmethods__["b2"] = _cspace.csCollisionPair_b2_set
    __swig_getmethods__["b2"] = _cspace.csCollisionPair_b2_get
    if _newclass:b2 = property(_cspace.csCollisionPair_b2_get, _cspace.csCollisionPair_b2_set)
    __swig_setmethods__["c2"] = _cspace.csCollisionPair_c2_set
    __swig_getmethods__["c2"] = _cspace.csCollisionPair_c2_get
    if _newclass:c2 = property(_cspace.csCollisionPair_c2_get, _cspace.csCollisionPair_c2_set)
    def __init__(self, *args):
        _swig_setattr(self, csCollisionPair, 'this', _cspace.new_csCollisionPair(*args))
        _swig_setattr(self, csCollisionPair, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csCollisionPair):
        try:
            if self.thisown: destroy(self)
        except: pass

class csCollisionPairPtr(csCollisionPair):
    def __init__(self, this):
        _swig_setattr(self, csCollisionPair, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csCollisionPair, 'thisown', 0)
        _swig_setattr(self, csCollisionPair,self.__class__,csCollisionPair)
_cspace.csCollisionPair_swigregister(csCollisionPairPtr)

class csIntersectingTriangle(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csIntersectingTriangle, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csIntersectingTriangle, name)
    def __repr__(self):
        return "<C csIntersectingTriangle instance at %s>" % (self.this,)
    __swig_setmethods__["a"] = _cspace.csIntersectingTriangle_a_set
    __swig_getmethods__["a"] = _cspace.csIntersectingTriangle_a_get
    if _newclass:a = property(_cspace.csIntersectingTriangle_a_get, _cspace.csIntersectingTriangle_a_set)
    __swig_setmethods__["b"] = _cspace.csIntersectingTriangle_b_set
    __swig_getmethods__["b"] = _cspace.csIntersectingTriangle_b_get
    if _newclass:b = property(_cspace.csIntersectingTriangle_b_get, _cspace.csIntersectingTriangle_b_set)
    __swig_setmethods__["c"] = _cspace.csIntersectingTriangle_c_set
    __swig_getmethods__["c"] = _cspace.csIntersectingTriangle_c_get
    if _newclass:c = property(_cspace.csIntersectingTriangle_c_get, _cspace.csIntersectingTriangle_c_set)
    def __init__(self, *args):
        _swig_setattr(self, csIntersectingTriangle, 'this', _cspace.new_csIntersectingTriangle(*args))
        _swig_setattr(self, csIntersectingTriangle, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csIntersectingTriangle):
        try:
            if self.thisown: destroy(self)
        except: pass

class csIntersectingTrianglePtr(csIntersectingTriangle):
    def __init__(self, this):
        _swig_setattr(self, csIntersectingTriangle, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csIntersectingTriangle, 'thisown', 0)
        _swig_setattr(self, csIntersectingTriangle,self.__class__,csIntersectingTriangle)
_cspace.csIntersectingTriangle_swigregister(csIntersectingTrianglePtr)

class iCollider(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCollider, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCollider, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCollider instance at %s>" % (self.this,)
    def __del__(self, destroy=_cspace.delete_iCollider):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iCollider_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iCollider_scfGetVersion)

class iColliderPtr(iCollider):
    def __init__(self, this):
        _swig_setattr(self, iCollider, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCollider, 'thisown', 0)
        _swig_setattr(self, iCollider,self.__class__,iCollider)
_cspace.iCollider_swigregister(iColliderPtr)

iCollider_scfGetVersion = _cspace.iCollider_scfGetVersion

class iCollideSystem(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iCollideSystem, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iCollideSystem, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iCollideSystem instance at %s>" % (self.this,)
    def CreateCollider(*args): return _cspace.iCollideSystem_CreateCollider(*args)
    def Collide(*args): return _cspace.iCollideSystem_Collide(*args)
    def GetCollisionPairs(*args): return _cspace.iCollideSystem_GetCollisionPairs(*args)
    def GetCollisionPairCount(*args): return _cspace.iCollideSystem_GetCollisionPairCount(*args)
    def ResetCollisionPairs(*args): return _cspace.iCollideSystem_ResetCollisionPairs(*args)
    def CollideRay(*args): return _cspace.iCollideSystem_CollideRay(*args)
    def GetIntersectingTriangles(*args): return _cspace.iCollideSystem_GetIntersectingTriangles(*args)
    def SetOneHitOnly(*args): return _cspace.iCollideSystem_SetOneHitOnly(*args)
    def GetOneHitOnly(*args): return _cspace.iCollideSystem_GetOneHitOnly(*args)
    def __del__(self, destroy=_cspace.delete_iCollideSystem):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iCollideSystem_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iCollideSystem_scfGetVersion)
    def GetCollisionPairByIndex(*args): return _cspace.iCollideSystem_GetCollisionPairByIndex(*args)
    def GetCollisionPairs (self):
      num = self.GetCollisionPairCount()
      pairs = []
      for i in range(num):
        pairs.append(self.GetCollisionPairByIndex(i))
      return pairs


class iCollideSystemPtr(iCollideSystem):
    def __init__(self, this):
        _swig_setattr(self, iCollideSystem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iCollideSystem, 'thisown', 0)
        _swig_setattr(self, iCollideSystem,self.__class__,iCollideSystem)
_cspace.iCollideSystem_swigregister(iCollideSystemPtr)

iCollideSystem_scfGetVersion = _cspace.iCollideSystem_scfGetVersion

class iDynamics(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDynamics, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDynamics, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDynamics instance at %s>" % (self.this,)
    def CreateSystem(*args): return _cspace.iDynamics_CreateSystem(*args)
    def RemoveSystem(*args): return _cspace.iDynamics_RemoveSystem(*args)
    def FindSystem(*args): return _cspace.iDynamics_FindSystem(*args)
    def Step(*args): return _cspace.iDynamics_Step(*args)
    def __del__(self, destroy=_cspace.delete_iDynamics):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iDynamics_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iDynamics_scfGetVersion)

class iDynamicsPtr(iDynamics):
    def __init__(self, this):
        _swig_setattr(self, iDynamics, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDynamics, 'thisown', 0)
        _swig_setattr(self, iDynamics,self.__class__,iDynamics)
_cspace.iDynamics_swigregister(iDynamicsPtr)

iDynamics_scfGetVersion = _cspace.iDynamics_scfGetVersion

class iDynamicSystem(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDynamicSystem, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDynamicSystem, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDynamicSystem instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iDynamicSystem_QueryObject(*args)
    def SetGravity(*args): return _cspace.iDynamicSystem_SetGravity(*args)
    def GetGravity(*args): return _cspace.iDynamicSystem_GetGravity(*args)
    def SetLinearDampener(*args): return _cspace.iDynamicSystem_SetLinearDampener(*args)
    def GetLinearDampener(*args): return _cspace.iDynamicSystem_GetLinearDampener(*args)
    def SetRollingDampener(*args): return _cspace.iDynamicSystem_SetRollingDampener(*args)
    def GetRollingDampener(*args): return _cspace.iDynamicSystem_GetRollingDampener(*args)
    def EnableAutoDisable(*args): return _cspace.iDynamicSystem_EnableAutoDisable(*args)
    def AutoDisableEnabled(*args): return _cspace.iDynamicSystem_AutoDisableEnabled(*args)
    def SetAutoDisableParams(*args): return _cspace.iDynamicSystem_SetAutoDisableParams(*args)
    def Step(*args): return _cspace.iDynamicSystem_Step(*args)
    def CreateBody(*args): return _cspace.iDynamicSystem_CreateBody(*args)
    def RemoveBody(*args): return _cspace.iDynamicSystem_RemoveBody(*args)
    def FindBody(*args): return _cspace.iDynamicSystem_FindBody(*args)
    def CreateGroup(*args): return _cspace.iDynamicSystem_CreateGroup(*args)
    def RemoveGroup(*args): return _cspace.iDynamicSystem_RemoveGroup(*args)
    def CreateJoint(*args): return _cspace.iDynamicSystem_CreateJoint(*args)
    def RemoveJoint(*args): return _cspace.iDynamicSystem_RemoveJoint(*args)
    def GetDefaultMoveCallback(*args): return _cspace.iDynamicSystem_GetDefaultMoveCallback(*args)
    def AttachColliderMesh(*args): return _cspace.iDynamicSystem_AttachColliderMesh(*args)
    def AttachColliderCylinder(*args): return _cspace.iDynamicSystem_AttachColliderCylinder(*args)
    def AttachColliderBox(*args): return _cspace.iDynamicSystem_AttachColliderBox(*args)
    def AttachColliderSphere(*args): return _cspace.iDynamicSystem_AttachColliderSphere(*args)
    def AttachColliderPlane(*args): return _cspace.iDynamicSystem_AttachColliderPlane(*args)
    def __del__(self, destroy=_cspace.delete_iDynamicSystem):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iDynamicSystem_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iDynamicSystem_scfGetVersion)

class iDynamicSystemPtr(iDynamicSystem):
    def __init__(self, this):
        _swig_setattr(self, iDynamicSystem, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDynamicSystem, 'thisown', 0)
        _swig_setattr(self, iDynamicSystem,self.__class__,iDynamicSystem)
_cspace.iDynamicSystem_swigregister(iDynamicSystemPtr)

iDynamicSystem_scfGetVersion = _cspace.iDynamicSystem_scfGetVersion

class iDynamicsMoveCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDynamicsMoveCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDynamicsMoveCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDynamicsMoveCallback instance at %s>" % (self.this,)
    def Execute(*args): return _cspace.iDynamicsMoveCallback_Execute(*args)
    def __del__(self, destroy=_cspace.delete_iDynamicsMoveCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iDynamicsMoveCallbackPtr(iDynamicsMoveCallback):
    def __init__(self, this):
        _swig_setattr(self, iDynamicsMoveCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDynamicsMoveCallback, 'thisown', 0)
        _swig_setattr(self, iDynamicsMoveCallback,self.__class__,iDynamicsMoveCallback)
_cspace.iDynamicsMoveCallback_swigregister(iDynamicsMoveCallbackPtr)

class iDynamicsCollisionCallback(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iDynamicsCollisionCallback, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iDynamicsCollisionCallback, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iDynamicsCollisionCallback instance at %s>" % (self.this,)
    def Execute(*args): return _cspace.iDynamicsCollisionCallback_Execute(*args)
    def __del__(self, destroy=_cspace.delete_iDynamicsCollisionCallback):
        try:
            if self.thisown: destroy(self)
        except: pass

class iDynamicsCollisionCallbackPtr(iDynamicsCollisionCallback):
    def __init__(self, this):
        _swig_setattr(self, iDynamicsCollisionCallback, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iDynamicsCollisionCallback, 'thisown', 0)
        _swig_setattr(self, iDynamicsCollisionCallback,self.__class__,iDynamicsCollisionCallback)
_cspace.iDynamicsCollisionCallback_swigregister(iDynamicsCollisionCallbackPtr)

class iBodyGroup(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iBodyGroup, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iBodyGroup, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iBodyGroup instance at %s>" % (self.this,)
    def AddBody(*args): return _cspace.iBodyGroup_AddBody(*args)
    def RemoveBody(*args): return _cspace.iBodyGroup_RemoveBody(*args)
    def BodyInGroup(*args): return _cspace.iBodyGroup_BodyInGroup(*args)
    def __del__(self, destroy=_cspace.delete_iBodyGroup):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iBodyGroup_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iBodyGroup_scfGetVersion)

class iBodyGroupPtr(iBodyGroup):
    def __init__(self, this):
        _swig_setattr(self, iBodyGroup, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iBodyGroup, 'thisown', 0)
        _swig_setattr(self, iBodyGroup,self.__class__,iBodyGroup)
_cspace.iBodyGroup_swigregister(iBodyGroupPtr)

iBodyGroup_scfGetVersion = _cspace.iBodyGroup_scfGetVersion

class iRigidBody(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iRigidBody, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iRigidBody, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iRigidBody instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iRigidBody_QueryObject(*args)
    def MakeStatic(*args): return _cspace.iRigidBody_MakeStatic(*args)
    def MakeDynamic(*args): return _cspace.iRigidBody_MakeDynamic(*args)
    def IsStatic(*args): return _cspace.iRigidBody_IsStatic(*args)
    def Disable(*args): return _cspace.iRigidBody_Disable(*args)
    def Enable(*args): return _cspace.iRigidBody_Enable(*args)
    def IsEnabled(*args): return _cspace.iRigidBody_IsEnabled(*args)
    def GetGroup(*args): return _cspace.iRigidBody_GetGroup(*args)
    def AttachColliderMesh(*args): return _cspace.iRigidBody_AttachColliderMesh(*args)
    def AttachColliderCylinder(*args): return _cspace.iRigidBody_AttachColliderCylinder(*args)
    def AttachColliderBox(*args): return _cspace.iRigidBody_AttachColliderBox(*args)
    def AttachColliderSphere(*args): return _cspace.iRigidBody_AttachColliderSphere(*args)
    def AttachColliderPlane(*args): return _cspace.iRigidBody_AttachColliderPlane(*args)
    def SetPosition(*args): return _cspace.iRigidBody_SetPosition(*args)
    def GetPosition(*args): return _cspace.iRigidBody_GetPosition(*args)
    def SetOrientation(*args): return _cspace.iRigidBody_SetOrientation(*args)
    def GetOrientation(*args): return _cspace.iRigidBody_GetOrientation(*args)
    def SetTransform(*args): return _cspace.iRigidBody_SetTransform(*args)
    def GetTransform(*args): return _cspace.iRigidBody_GetTransform(*args)
    def SetLinearVelocity(*args): return _cspace.iRigidBody_SetLinearVelocity(*args)
    def GetLinearVelocity(*args): return _cspace.iRigidBody_GetLinearVelocity(*args)
    def SetAngularVelocity(*args): return _cspace.iRigidBody_SetAngularVelocity(*args)
    def GetAngularVelocity(*args): return _cspace.iRigidBody_GetAngularVelocity(*args)
    def SetProperties(*args): return _cspace.iRigidBody_SetProperties(*args)
    def GetProperties(*args): return _cspace.iRigidBody_GetProperties(*args)
    def AdjustTotalMass(*args): return _cspace.iRigidBody_AdjustTotalMass(*args)
    def AddForce(*args): return _cspace.iRigidBody_AddForce(*args)
    def AddTorque(*args): return _cspace.iRigidBody_AddTorque(*args)
    def AddRelForce(*args): return _cspace.iRigidBody_AddRelForce(*args)
    def AddRelTorque(*args): return _cspace.iRigidBody_AddRelTorque(*args)
    def AddForceAtPos(*args): return _cspace.iRigidBody_AddForceAtPos(*args)
    def AddForceAtRelPos(*args): return _cspace.iRigidBody_AddForceAtRelPos(*args)
    def AddRelForceAtPos(*args): return _cspace.iRigidBody_AddRelForceAtPos(*args)
    def AddRelForceAtRelPos(*args): return _cspace.iRigidBody_AddRelForceAtRelPos(*args)
    def GetForce(*args): return _cspace.iRigidBody_GetForce(*args)
    def GetTorque(*args): return _cspace.iRigidBody_GetTorque(*args)
    def AttachMesh(*args): return _cspace.iRigidBody_AttachMesh(*args)
    def GetAttachedMesh(*args): return _cspace.iRigidBody_GetAttachedMesh(*args)
    def SetMoveCallback(*args): return _cspace.iRigidBody_SetMoveCallback(*args)
    def SetCollisionCallback(*args): return _cspace.iRigidBody_SetCollisionCallback(*args)
    def Collision(*args): return _cspace.iRigidBody_Collision(*args)
    def Update(*args): return _cspace.iRigidBody_Update(*args)
    def __del__(self, destroy=_cspace.delete_iRigidBody):
        try:
            if self.thisown: destroy(self)
        except: pass

class iRigidBodyPtr(iRigidBody):
    def __init__(self, this):
        _swig_setattr(self, iRigidBody, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iRigidBody, 'thisown', 0)
        _swig_setattr(self, iRigidBody,self.__class__,iRigidBody)
_cspace.iRigidBody_swigregister(iRigidBodyPtr)

class iJoint(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iJoint, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iJoint, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iJoint instance at %s>" % (self.this,)
    def Attach(*args): return _cspace.iJoint_Attach(*args)
    def GetAttachedBody(*args): return _cspace.iJoint_GetAttachedBody(*args)
    def SetTransform(*args): return _cspace.iJoint_SetTransform(*args)
    def GetTransform(*args): return _cspace.iJoint_GetTransform(*args)
    def SetTransConstraints(*args): return _cspace.iJoint_SetTransConstraints(*args)
    def IsXTransConstrained(*args): return _cspace.iJoint_IsXTransConstrained(*args)
    def IsYTransConstrained(*args): return _cspace.iJoint_IsYTransConstrained(*args)
    def IsZTransConstrained(*args): return _cspace.iJoint_IsZTransConstrained(*args)
    def SetMinimumDistance(*args): return _cspace.iJoint_SetMinimumDistance(*args)
    def GetMinimumDistance(*args): return _cspace.iJoint_GetMinimumDistance(*args)
    def SetMaximumDistance(*args): return _cspace.iJoint_SetMaximumDistance(*args)
    def GetMaximumDistance(*args): return _cspace.iJoint_GetMaximumDistance(*args)
    def SetRotConstraints(*args): return _cspace.iJoint_SetRotConstraints(*args)
    def IsXRotConstrained(*args): return _cspace.iJoint_IsXRotConstrained(*args)
    def IsYRotConstrained(*args): return _cspace.iJoint_IsYRotConstrained(*args)
    def IsZRotConstrained(*args): return _cspace.iJoint_IsZRotConstrained(*args)
    def SetMinimumAngle(*args): return _cspace.iJoint_SetMinimumAngle(*args)
    def GetMinimumAngle(*args): return _cspace.iJoint_GetMinimumAngle(*args)
    def SetMaximumAngle(*args): return _cspace.iJoint_SetMaximumAngle(*args)
    def GetMaximumAngle(*args): return _cspace.iJoint_GetMaximumAngle(*args)
    def SetBounce(*args): return _cspace.iJoint_SetBounce(*args)
    def GetBounce(*args): return _cspace.iJoint_GetBounce(*args)
    def SetDesiredVelocity(*args): return _cspace.iJoint_SetDesiredVelocity(*args)
    def GetDesiredVelocity(*args): return _cspace.iJoint_GetDesiredVelocity(*args)
    def SetMaxForce(*args): return _cspace.iJoint_SetMaxForce(*args)
    def GetMaxForce(*args): return _cspace.iJoint_GetMaxForce(*args)
    def __del__(self, destroy=_cspace.delete_iJoint):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iJoint_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iJoint_scfGetVersion)

class iJointPtr(iJoint):
    def __init__(self, this):
        _swig_setattr(self, iJoint, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iJoint, 'thisown', 0)
        _swig_setattr(self, iJoint,self.__class__,iJoint)
_cspace.iJoint_swigregister(iJointPtr)

iJoint_scfGetVersion = _cspace.iJoint_scfGetVersion

CS_SEQUENCE_LIGHTCHANGE_NONE = _cspace.CS_SEQUENCE_LIGHTCHANGE_NONE
CS_SEQUENCE_LIGHTCHANGE_LESS = _cspace.CS_SEQUENCE_LIGHTCHANGE_LESS
CS_SEQUENCE_LIGHTCHANGE_GREATER = _cspace.CS_SEQUENCE_LIGHTCHANGE_GREATER
class iParameterESM(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iParameterESM, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iParameterESM, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iParameterESM instance at %s>" % (self.this,)
    def GetValue(*args): return _cspace.iParameterESM_GetValue(*args)
    def IsConstant(*args): return _cspace.iParameterESM_IsConstant(*args)
    def __del__(self, destroy=_cspace.delete_iParameterESM):
        try:
            if self.thisown: destroy(self)
        except: pass

class iParameterESMPtr(iParameterESM):
    def __init__(self, this):
        _swig_setattr(self, iParameterESM, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iParameterESM, 'thisown', 0)
        _swig_setattr(self, iParameterESM,self.__class__,iParameterESM)
_cspace.iParameterESM_swigregister(iParameterESMPtr)

class iEngineSequenceParameters(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEngineSequenceParameters, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEngineSequenceParameters, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEngineSequenceParameters instance at %s>" % (self.this,)
    def GetParameterCount(*args): return _cspace.iEngineSequenceParameters_GetParameterCount(*args)
    def GetParameter(*args): return _cspace.iEngineSequenceParameters_GetParameter(*args)
    def GetParameterIdx(*args): return _cspace.iEngineSequenceParameters_GetParameterIdx(*args)
    def GetParameterName(*args): return _cspace.iEngineSequenceParameters_GetParameterName(*args)
    def AddParameter(*args): return _cspace.iEngineSequenceParameters_AddParameter(*args)
    def SetParameter(*args): return _cspace.iEngineSequenceParameters_SetParameter(*args)
    def CreateParameterESM(*args): return _cspace.iEngineSequenceParameters_CreateParameterESM(*args)
    def __del__(self, destroy=_cspace.delete_iEngineSequenceParameters):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEngineSequenceParametersPtr(iEngineSequenceParameters):
    def __init__(self, this):
        _swig_setattr(self, iEngineSequenceParameters, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEngineSequenceParameters, 'thisown', 0)
        _swig_setattr(self, iEngineSequenceParameters,self.__class__,iEngineSequenceParameters)
_cspace.iEngineSequenceParameters_swigregister(iEngineSequenceParametersPtr)

class iSequenceWrapper(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSequenceWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSequenceWrapper, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSequenceWrapper instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iSequenceWrapper_QueryObject(*args)
    def GetSequence(*args): return _cspace.iSequenceWrapper_GetSequence(*args)
    def CreateBaseParameterBlock(*args): return _cspace.iSequenceWrapper_CreateBaseParameterBlock(*args)
    def GetBaseParameterBlock(*args): return _cspace.iSequenceWrapper_GetBaseParameterBlock(*args)
    def CreateParameterBlock(*args): return _cspace.iSequenceWrapper_CreateParameterBlock(*args)
    def AddOperationSetVariable(*args): return _cspace.iSequenceWrapper_AddOperationSetVariable(*args)
    def AddOperationSetMaterial(*args): return _cspace.iSequenceWrapper_AddOperationSetMaterial(*args)
    def AddOperationSetPolygonMaterial(*args): return _cspace.iSequenceWrapper_AddOperationSetPolygonMaterial(*args)
    def AddOperationSetLight(*args): return _cspace.iSequenceWrapper_AddOperationSetLight(*args)
    def AddOperationFadeLight(*args): return _cspace.iSequenceWrapper_AddOperationFadeLight(*args)
    def AddOperationSetAmbient(*args): return _cspace.iSequenceWrapper_AddOperationSetAmbient(*args)
    def AddOperationFadeAmbient(*args): return _cspace.iSequenceWrapper_AddOperationFadeAmbient(*args)
    def AddOperationRandomDelay(*args): return _cspace.iSequenceWrapper_AddOperationRandomDelay(*args)
    def AddOperationSetMeshColor(*args): return _cspace.iSequenceWrapper_AddOperationSetMeshColor(*args)
    def AddOperationFadeMeshColor(*args): return _cspace.iSequenceWrapper_AddOperationFadeMeshColor(*args)
    def AddOperationSetFog(*args): return _cspace.iSequenceWrapper_AddOperationSetFog(*args)
    def AddOperationFadeFog(*args): return _cspace.iSequenceWrapper_AddOperationFadeFog(*args)
    def AddOperationRotateDuration(*args): return _cspace.iSequenceWrapper_AddOperationRotateDuration(*args)
    def AddOperationMoveDuration(*args): return _cspace.iSequenceWrapper_AddOperationMoveDuration(*args)
    def AddOperationTriggerState(*args): return _cspace.iSequenceWrapper_AddOperationTriggerState(*args)
    def AddOperationCheckTrigger(*args): return _cspace.iSequenceWrapper_AddOperationCheckTrigger(*args)
    def AddOperationTestTrigger(*args): return _cspace.iSequenceWrapper_AddOperationTestTrigger(*args)
    def __del__(self, destroy=_cspace.delete_iSequenceWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSequenceWrapperPtr(iSequenceWrapper):
    def __init__(self, this):
        _swig_setattr(self, iSequenceWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSequenceWrapper, 'thisown', 0)
        _swig_setattr(self, iSequenceWrapper,self.__class__,iSequenceWrapper)
_cspace.iSequenceWrapper_swigregister(iSequenceWrapperPtr)

class iSequenceTrigger(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSequenceTrigger, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSequenceTrigger, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSequenceTrigger instance at %s>" % (self.this,)
    def QueryObject(*args): return _cspace.iSequenceTrigger_QueryObject(*args)
    def AddConditionInSector(*args): return _cspace.iSequenceTrigger_AddConditionInSector(*args)
    def AddConditionSectorVisible(*args): return _cspace.iSequenceTrigger_AddConditionSectorVisible(*args)
    def AddConditionMeshClick(*args): return _cspace.iSequenceTrigger_AddConditionMeshClick(*args)
    def AddConditionLightChange(*args): return _cspace.iSequenceTrigger_AddConditionLightChange(*args)
    def AddConditionManual(*args): return _cspace.iSequenceTrigger_AddConditionManual(*args)
    def SetEnabled(*args): return _cspace.iSequenceTrigger_SetEnabled(*args)
    def IsEnabled(*args): return _cspace.iSequenceTrigger_IsEnabled(*args)
    def ClearConditions(*args): return _cspace.iSequenceTrigger_ClearConditions(*args)
    def Trigger(*args): return _cspace.iSequenceTrigger_Trigger(*args)
    def SetParameters(*args): return _cspace.iSequenceTrigger_SetParameters(*args)
    def GetParameters(*args): return _cspace.iSequenceTrigger_GetParameters(*args)
    def FireSequence(*args): return _cspace.iSequenceTrigger_FireSequence(*args)
    def GetFiredSequence(*args): return _cspace.iSequenceTrigger_GetFiredSequence(*args)
    def TestConditions(*args): return _cspace.iSequenceTrigger_TestConditions(*args)
    def CheckState(*args): return _cspace.iSequenceTrigger_CheckState(*args)
    def ForceFire(*args): return _cspace.iSequenceTrigger_ForceFire(*args)
    def __del__(self, destroy=_cspace.delete_iSequenceTrigger):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSequenceTriggerPtr(iSequenceTrigger):
    def __init__(self, this):
        _swig_setattr(self, iSequenceTrigger, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSequenceTrigger, 'thisown', 0)
        _swig_setattr(self, iSequenceTrigger,self.__class__,iSequenceTrigger)
_cspace.iSequenceTrigger_swigregister(iSequenceTriggerPtr)

class iSequenceTimedOperation(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSequenceTimedOperation, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSequenceTimedOperation, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSequenceTimedOperation instance at %s>" % (self.this,)
    def Do(*args): return _cspace.iSequenceTimedOperation_Do(*args)
    def __del__(self, destroy=_cspace.delete_iSequenceTimedOperation):
        try:
            if self.thisown: destroy(self)
        except: pass

class iSequenceTimedOperationPtr(iSequenceTimedOperation):
    def __init__(self, this):
        _swig_setattr(self, iSequenceTimedOperation, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSequenceTimedOperation, 'thisown', 0)
        _swig_setattr(self, iSequenceTimedOperation,self.__class__,iSequenceTimedOperation)
_cspace.iSequenceTimedOperation_swigregister(iSequenceTimedOperationPtr)

class iEngineSequenceManager(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iEngineSequenceManager, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iEngineSequenceManager, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iEngineSequenceManager instance at %s>" % (self.this,)
    def GetSequenceManager(*args): return _cspace.iEngineSequenceManager_GetSequenceManager(*args)
    def SetCamera(*args): return _cspace.iEngineSequenceManager_SetCamera(*args)
    def GetCamera(*args): return _cspace.iEngineSequenceManager_GetCamera(*args)
    def CreateParameterESM(*args): return _cspace.iEngineSequenceManager_CreateParameterESM(*args)
    def CreateTrigger(*args): return _cspace.iEngineSequenceManager_CreateTrigger(*args)
    def RemoveTrigger(*args): return _cspace.iEngineSequenceManager_RemoveTrigger(*args)
    def RemoveTriggers(*args): return _cspace.iEngineSequenceManager_RemoveTriggers(*args)
    def GetTriggerCount(*args): return _cspace.iEngineSequenceManager_GetTriggerCount(*args)
    def GetTrigger(*args): return _cspace.iEngineSequenceManager_GetTrigger(*args)
    def FindTriggerByName(*args): return _cspace.iEngineSequenceManager_FindTriggerByName(*args)
    def FireTriggerByName(*args): return _cspace.iEngineSequenceManager_FireTriggerByName(*args)
    def CreateSequence(*args): return _cspace.iEngineSequenceManager_CreateSequence(*args)
    def RemoveSequence(*args): return _cspace.iEngineSequenceManager_RemoveSequence(*args)
    def RemoveSequences(*args): return _cspace.iEngineSequenceManager_RemoveSequences(*args)
    def GetSequenceCount(*args): return _cspace.iEngineSequenceManager_GetSequenceCount(*args)
    def GetSequence(*args): return _cspace.iEngineSequenceManager_GetSequence(*args)
    def FindSequenceByName(*args): return _cspace.iEngineSequenceManager_FindSequenceByName(*args)
    def RunSequenceByName(*args): return _cspace.iEngineSequenceManager_RunSequenceByName(*args)
    def FireTimedOperation(*args): return _cspace.iEngineSequenceManager_FireTimedOperation(*args)
    def __del__(self, destroy=_cspace.delete_iEngineSequenceManager):
        try:
            if self.thisown: destroy(self)
        except: pass

class iEngineSequenceManagerPtr(iEngineSequenceManager):
    def __init__(self, this):
        _swig_setattr(self, iEngineSequenceManager, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iEngineSequenceManager, 'thisown', 0)
        _swig_setattr(self, iEngineSequenceManager,self.__class__,iEngineSequenceManager)
_cspace.iEngineSequenceManager_swigregister(iEngineSequenceManagerPtr)

class iScriptObject(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iScriptObject, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iScriptObject, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iScriptObject instance at %s>" % (self.this,)
    def IsType(*args): return _cspace.iScriptObject_IsType(*args)
    def GetPointer(*args): return _cspace.iScriptObject_GetPointer(*args)
    def SetPointer(*args): return _cspace.iScriptObject_SetPointer(*args)
    def IntCall(*args): return _cspace.iScriptObject_IntCall(*args)
    def FloatCall(*args): return _cspace.iScriptObject_FloatCall(*args)
    def DoubleCall(*args): return _cspace.iScriptObject_DoubleCall(*args)
    def Call(*args): return _cspace.iScriptObject_Call(*args)
    def ObjectCall(*args): return _cspace.iScriptObject_ObjectCall(*args)
    def SetInt(*args): return _cspace.iScriptObject_SetInt(*args)
    def SetFloat(*args): return _cspace.iScriptObject_SetFloat(*args)
    def SetDouble(*args): return _cspace.iScriptObject_SetDouble(*args)
    def SetString(*args): return _cspace.iScriptObject_SetString(*args)
    def Set(*args): return _cspace.iScriptObject_Set(*args)
    def SetTruth(*args): return _cspace.iScriptObject_SetTruth(*args)
    def GetFloat(*args): return _cspace.iScriptObject_GetFloat(*args)
    def Get(*args): return _cspace.iScriptObject_Get(*args)
    def GetTruth(*args): return _cspace.iScriptObject_GetTruth(*args)
    def __del__(self, destroy=_cspace.delete_iScriptObject):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iScriptObject_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iScriptObject_scfGetVersion)

class iScriptObjectPtr(iScriptObject):
    def __init__(self, this):
        _swig_setattr(self, iScriptObject, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iScriptObject, 'thisown', 0)
        _swig_setattr(self, iScriptObject,self.__class__,iScriptObject)
_cspace.iScriptObject_swigregister(iScriptObjectPtr)

iScriptObject_scfGetVersion = _cspace.iScriptObject_scfGetVersion

class iScript(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iScript, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iScript, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iScript instance at %s>" % (self.this,)
    def Initialize(*args): return _cspace.iScript_Initialize(*args)
    def RunText(*args): return _cspace.iScript_RunText(*args)
    def LoadModule(*args): return _cspace.iScript_LoadModule(*args)
    def IntCall(*args): return _cspace.iScript_IntCall(*args)
    def FloatCall(*args): return _cspace.iScript_FloatCall(*args)
    def DoubleCall(*args): return _cspace.iScript_DoubleCall(*args)
    def Call(*args): return _cspace.iScript_Call(*args)
    def ObjectCall(*args): return _cspace.iScript_ObjectCall(*args)
    def NewObject(*args): return _cspace.iScript_NewObject(*args)
    def StoreInt(*args): return _cspace.iScript_StoreInt(*args)
    def StoreFloat(*args): return _cspace.iScript_StoreFloat(*args)
    def StoreDouble(*args): return _cspace.iScript_StoreDouble(*args)
    def StoreString(*args): return _cspace.iScript_StoreString(*args)
    def Store(*args): return _cspace.iScript_Store(*args)
    def SetTruth(*args): return _cspace.iScript_SetTruth(*args)
    def RetrieveFloat(*args): return _cspace.iScript_RetrieveFloat(*args)
    def Retrieve(*args): return _cspace.iScript_Retrieve(*args)
    def GetTruth(*args): return _cspace.iScript_GetTruth(*args)
    def Remove(*args): return _cspace.iScript_Remove(*args)
    def __del__(self, destroy=_cspace.delete_iScript):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iScript_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iScript_scfGetVersion)

class iScriptPtr(iScript):
    def __init__(self, this):
        _swig_setattr(self, iScript, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iScript, 'thisown', 0)
        _swig_setattr(self, iScript,self.__class__,iScript)
_cspace.iScript_swigregister(iScriptPtr)

iScript_scfGetVersion = _cspace.iScript_scfGetVersion

class iSimpleFormerState(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iSimpleFormerState, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iSimpleFormerState, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iSimpleFormerState instance at %s>" % (self.this,)
    def SetHeightmap(*args): return _cspace.iSimpleFormerState_SetHeightmap(*args)
    def SetScale(*args): return _cspace.iSimpleFormerState_SetScale(*args)
    def SetOffset(*args): return _cspace.iSimpleFormerState_SetOffset(*args)
    def SetIntegerMap(*args): return _cspace.iSimpleFormerState_SetIntegerMap(*args)
    def SetFloatMap(*args): return _cspace.iSimpleFormerState_SetFloatMap(*args)
    def __del__(self, destroy=_cspace.delete_iSimpleFormerState):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iSimpleFormerState_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iSimpleFormerState_scfGetVersion)

class iSimpleFormerStatePtr(iSimpleFormerState):
    def __init__(self, this):
        _swig_setattr(self, iSimpleFormerState, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iSimpleFormerState, 'thisown', 0)
        _swig_setattr(self, iSimpleFormerState,self.__class__,iSimpleFormerState)
_cspace.iSimpleFormerState_swigregister(iSimpleFormerStatePtr)

iSimpleFormerState_scfGetVersion = _cspace.iSimpleFormerState_scfGetVersion

class iTerraFormer(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTerraFormer, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTerraFormer, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTerraFormer instance at %s>" % (self.this,)
    def GetSampler(*args): return _cspace.iTerraFormer_GetSampler(*args)
    def SampleFloat(*args): return _cspace.iTerraFormer_SampleFloat(*args)
    def SampleVector2(*args): return _cspace.iTerraFormer_SampleVector2(*args)
    def SampleVector3(*args): return _cspace.iTerraFormer_SampleVector3(*args)
    def SampleInteger(*args): return _cspace.iTerraFormer_SampleInteger(*args)
    def __del__(self, destroy=_cspace.delete_iTerraFormer):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTerraFormer_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTerraFormer_scfGetVersion)

class iTerraFormerPtr(iTerraFormer):
    def __init__(self, this):
        _swig_setattr(self, iTerraFormer, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTerraFormer, 'thisown', 0)
        _swig_setattr(self, iTerraFormer,self.__class__,iTerraFormer)
_cspace.iTerraFormer_swigregister(iTerraFormerPtr)

iTerraFormer_scfGetVersion = _cspace.iTerraFormer_scfGetVersion

class iTerraSampler(iBase):
    __swig_setmethods__ = {}
    for _s in [iBase]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, iTerraSampler, name, value)
    __swig_getmethods__ = {}
    for _s in [iBase]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, iTerraSampler, name)
    def __init__(self): raise RuntimeError, "No constructor defined"
    def __repr__(self):
        return "<C iTerraSampler instance at %s>" % (self.this,)
    def SampleFloat(*args): return _cspace.iTerraSampler_SampleFloat(*args)
    def SampleVector2(*args): return _cspace.iTerraSampler_SampleVector2(*args)
    def SampleVector3(*args): return _cspace.iTerraSampler_SampleVector3(*args)
    def SampleInteger(*args): return _cspace.iTerraSampler_SampleInteger(*args)
    def GetMaterialPalette(*args): return _cspace.iTerraSampler_GetMaterialPalette(*args)
    def GetRegion(*args): return _cspace.iTerraSampler_GetRegion(*args)
    def GetResolution(*args): return _cspace.iTerraSampler_GetResolution(*args)
    def GetVersion(*args): return _cspace.iTerraSampler_GetVersion(*args)
    def Cleanup(*args): return _cspace.iTerraSampler_Cleanup(*args)
    def __del__(self, destroy=_cspace.delete_iTerraSampler):
        try:
            if self.thisown: destroy(self)
        except: pass
    __swig_getmethods__["scfGetVersion"] = lambda x: _cspace.iTerraSampler_scfGetVersion
    if _newclass:scfGetVersion = staticmethod(_cspace.iTerraSampler_scfGetVersion)

class iTerraSamplerPtr(iTerraSampler):
    def __init__(self, this):
        _swig_setattr(self, iTerraSampler, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, iTerraSampler, 'thisown', 0)
        _swig_setattr(self, iTerraSampler,self.__class__,iTerraSampler)
_cspace.iTerraSampler_swigregister(iTerraSamplerPtr)

iTerraSampler_scfGetVersion = _cspace.iTerraSampler_scfGetVersion

class csObject(iObject):
    __swig_setmethods__ = {}
    for _s in [iObject]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csObject, name, value)
    __swig_getmethods__ = {}
    for _s in [iObject]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csObject, name)
    def __repr__(self):
        return "<C csObject instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csObject, 'this', _cspace.new_csObject(*args))
        _swig_setattr(self, csObject, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csObject):
        try:
            if self.thisown: destroy(self)
        except: pass
    def SetName(*args): return _cspace.csObject_SetName(*args)
    def GetName(*args): return _cspace.csObject_GetName(*args)
    def GetID(*args): return _cspace.csObject_GetID(*args)
    def SetObjectParent(*args): return _cspace.csObject_SetObjectParent(*args)
    def GetObjectParent(*args): return _cspace.csObject_GetObjectParent(*args)
    def ObjAdd(*args): return _cspace.csObject_ObjAdd(*args)
    def ObjRemove(*args): return _cspace.csObject_ObjRemove(*args)
    def ObjRemoveAll(*args): return _cspace.csObject_ObjRemoveAll(*args)
    def ObjAddChildren(*args): return _cspace.csObject_ObjAddChildren(*args)
    def GetChild(*args): return _cspace.csObject_GetChild(*args)
    def GetIterator(*args): return _cspace.csObject_GetIterator(*args)
    __swig_setmethods__["scfRefCount"] = _cspace.csObject_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace.csObject_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace.csObject_scfRefCount_get, _cspace.csObject_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace.csObject_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace.csObject_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace.csObject_scfWeakRefOwners_get, _cspace.csObject_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace.csObject_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace.csObject_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace.csObject_scfParent_get
    if _newclass:scfParent = property(_cspace.csObject_scfParent_get, _cspace.csObject_scfParent_set)
    def IncRef(*args): return _cspace.csObject_IncRef(*args)
    def DecRef(*args): return _cspace.csObject_DecRef(*args)
    def GetRefCount(*args): return _cspace.csObject_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csObject_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csObject_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csObject_QueryInterface(*args)
    def ObjReleaseOld(*args): return _cspace.csObject_ObjReleaseOld(*args)

class csObjectPtr(csObject):
    def __init__(self, this):
        _swig_setattr(self, csObject, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csObject, 'thisown', 0)
        _swig_setattr(self, csObject,self.__class__,csObject)
_cspace.csObject_swigregister(csObjectPtr)

class csView(iView):
    __swig_setmethods__ = {}
    for _s in [iView]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csView, name, value)
    __swig_getmethods__ = {}
    for _s in [iView]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csView, name)
    def __repr__(self):
        return "<C csView instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csView, 'this', _cspace.new_csView(*args))
        _swig_setattr(self, csView, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csView):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetEngine(*args): return _cspace.csView_GetEngine(*args)
    def SetEngine(*args): return _cspace.csView_SetEngine(*args)
    def GetCamera(*args): return _cspace.csView_GetCamera(*args)
    def SetCamera(*args): return _cspace.csView_SetCamera(*args)
    def GetContext(*args): return _cspace.csView_GetContext(*args)
    def SetContext(*args): return _cspace.csView_SetContext(*args)
    def SetRectangle(*args): return _cspace.csView_SetRectangle(*args)
    def ClearView(*args): return _cspace.csView_ClearView(*args)
    def AddViewVertex(*args): return _cspace.csView_AddViewVertex(*args)
    def RestrictClipperToScreen(*args): return _cspace.csView_RestrictClipperToScreen(*args)
    def SetAutoResize(*args): return _cspace.csView_SetAutoResize(*args)
    def UpdateClipper(*args): return _cspace.csView_UpdateClipper(*args)
    def GetClipper(*args): return _cspace.csView_GetClipper(*args)
    def Draw(*args): return _cspace.csView_Draw(*args)
    __swig_setmethods__["scfRefCount"] = _cspace.csView_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace.csView_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace.csView_scfRefCount_get, _cspace.csView_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace.csView_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace.csView_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace.csView_scfWeakRefOwners_get, _cspace.csView_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace.csView_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace.csView_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace.csView_scfParent_get
    if _newclass:scfParent = property(_cspace.csView_scfParent_get, _cspace.csView_scfParent_set)
    def IncRef(*args): return _cspace.csView_IncRef(*args)
    def DecRef(*args): return _cspace.csView_DecRef(*args)
    def GetRefCount(*args): return _cspace.csView_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csView_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csView_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csView_QueryInterface(*args)

class csViewPtr(csView):
    def __init__(self, this):
        _swig_setattr(self, csView, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csView, 'thisown', 0)
        _swig_setattr(self, csView,self.__class__,csView)
_cspace.csView_swigregister(csViewPtr)

class csColliderWrapper(csObject):
    __swig_setmethods__ = {}
    for _s in [csObject]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, csColliderWrapper, name, value)
    __swig_getmethods__ = {}
    for _s in [csObject]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, csColliderWrapper, name)
    def __repr__(self):
        return "<C csColliderWrapper instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csColliderWrapper, 'this', _cspace.new_csColliderWrapper(*args))
        _swig_setattr(self, csColliderWrapper, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csColliderWrapper):
        try:
            if self.thisown: destroy(self)
        except: pass
    def GetCollider(*args): return _cspace.csColliderWrapper_GetCollider(*args)
    def GetCollideSystem(*args): return _cspace.csColliderWrapper_GetCollideSystem(*args)
    def Collide(*args): return _cspace.csColliderWrapper_Collide(*args)
    __swig_getmethods__["GetColliderWrapper"] = lambda x: _cspace.csColliderWrapper_GetColliderWrapper
    if _newclass:GetColliderWrapper = staticmethod(_cspace.csColliderWrapper_GetColliderWrapper)
    __swig_getmethods__["GetColliderWrapper"] = lambda x: _cspace.csColliderWrapper_GetColliderWrapper
    if _newclass:GetColliderWrapper = staticmethod(_cspace.csColliderWrapper_GetColliderWrapper)
    def IncRef(*args): return _cspace.csColliderWrapper_IncRef(*args)
    def DecRef(*args): return _cspace.csColliderWrapper_DecRef(*args)
    def GetRefCount(*args): return _cspace.csColliderWrapper_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace.csColliderWrapper_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace.csColliderWrapper_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace.csColliderWrapper_QueryInterface(*args)

class csColliderWrapperPtr(csColliderWrapper):
    def __init__(self, this):
        _swig_setattr(self, csColliderWrapper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csColliderWrapper, 'thisown', 0)
        _swig_setattr(self, csColliderWrapper,self.__class__,csColliderWrapper)
_cspace.csColliderWrapper_swigregister(csColliderWrapperPtr)

csColliderWrapper_GetColliderWrapper = _cspace.csColliderWrapper_GetColliderWrapper

class csColliderHelper(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csColliderHelper, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csColliderHelper, name)
    def __repr__(self):
        return "<C csColliderHelper instance at %s>" % (self.this,)
    __swig_getmethods__["InitializeCollisionWrapper"] = lambda x: _cspace.csColliderHelper_InitializeCollisionWrapper
    if _newclass:InitializeCollisionWrapper = staticmethod(_cspace.csColliderHelper_InitializeCollisionWrapper)
    __swig_getmethods__["InitializeCollisionWrappers"] = lambda x: _cspace.csColliderHelper_InitializeCollisionWrappers
    if _newclass:InitializeCollisionWrappers = staticmethod(_cspace.csColliderHelper_InitializeCollisionWrappers)
    __swig_getmethods__["CollideArray"] = lambda x: _cspace.csColliderHelper_CollideArray
    if _newclass:CollideArray = staticmethod(_cspace.csColliderHelper_CollideArray)
    __swig_getmethods__["CollidePath"] = lambda x: _cspace.csColliderHelper_CollidePath
    if _newclass:CollidePath = staticmethod(_cspace.csColliderHelper_CollidePath)
    __swig_getmethods__["TraceBeam"] = lambda x: _cspace.csColliderHelper_TraceBeam
    if _newclass:TraceBeam = staticmethod(_cspace.csColliderHelper_TraceBeam)
    def __init__(self, *args):
        _swig_setattr(self, csColliderHelper, 'this', _cspace.new_csColliderHelper(*args))
        _swig_setattr(self, csColliderHelper, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csColliderHelper):
        try:
            if self.thisown: destroy(self)
        except: pass

class csColliderHelperPtr(csColliderHelper):
    def __init__(self, this):
        _swig_setattr(self, csColliderHelper, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csColliderHelper, 'thisown', 0)
        _swig_setattr(self, csColliderHelper,self.__class__,csColliderHelper)
_cspace.csColliderHelper_swigregister(csColliderHelperPtr)

csColliderHelper_InitializeCollisionWrapper = _cspace.csColliderHelper_InitializeCollisionWrapper

csColliderHelper_InitializeCollisionWrappers = _cspace.csColliderHelper_InitializeCollisionWrappers

csColliderHelper_CollideArray = _cspace.csColliderHelper_CollideArray

csColliderHelper_CollidePath = _cspace.csColliderHelper_CollidePath

csColliderHelper_TraceBeam = _cspace.csColliderHelper_TraceBeam


csfxInterference = _cspace.csfxInterference

csfxFadeOut = _cspace.csfxFadeOut

csfxFadeTo = _cspace.csfxFadeTo

csfxFadeToColor = _cspace.csfxFadeToColor

csfxGreenScreen = _cspace.csfxGreenScreen

csfxRedScreen = _cspace.csfxRedScreen

csfxBlueScreen = _cspace.csfxBlueScreen

csfxWhiteOut = _cspace.csfxWhiteOut

csfxShadeVert = _cspace.csfxShadeVert

csfxScreenDPFX = _cspace.csfxScreenDPFX

csfxScreenDPFXPartial = _cspace.csfxScreenDPFXPartial
PIXMAP_TOP = _cspace.PIXMAP_TOP
PIXMAP_LEFT = _cspace.PIXMAP_LEFT
PIXMAP_CENTER = _cspace.PIXMAP_CENTER
PIXMAP_BOTTOM = _cspace.PIXMAP_BOTTOM
PIXMAP_RIGHT = _cspace.PIXMAP_RIGHT
class csPixmap(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, csPixmap, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, csPixmap, name)
    def __repr__(self):
        return "<C csSimplePixmap instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, csPixmap, 'this', _cspace.new_csPixmap(*args))
        _swig_setattr(self, csPixmap, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete_csPixmap):
        try:
            if self.thisown: destroy(self)
        except: pass
    def SetTextureHandle(*args): return _cspace.csPixmap_SetTextureHandle(*args)
    def SetTextureRectangle(*args): return _cspace.csPixmap_SetTextureRectangle(*args)
    def DrawScaled(*args): return _cspace.csPixmap_DrawScaled(*args)
    def DrawTiled(*args): return _cspace.csPixmap_DrawTiled(*args)
    def Width(*args): return _cspace.csPixmap_Width(*args)
    def Height(*args): return _cspace.csPixmap_Height(*args)
    def Advance(*args): return _cspace.csPixmap_Advance(*args)
    def GetTextureHandle(*args): return _cspace.csPixmap_GetTextureHandle(*args)

class csPixmapPtr(csPixmap):
    def __init__(self, this):
        _swig_setattr(self, csPixmap, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, csPixmap, 'thisown', 0)
        _swig_setattr(self, csPixmap,self.__class__,csPixmap)
_cspace.csPixmap_swigregister(csPixmapPtr)


CS_IS_KEYBOARD_EVENT = _cspace.CS_IS_KEYBOARD_EVENT

CS_IS_MOUSE_EVENT = _cspace.CS_IS_MOUSE_EVENT

CS_IS_JOYSTICK_EVENT = _cspace.CS_IS_JOYSTICK_EVENT

CS_IS_INPUT_EVENT = _cspace.CS_IS_INPUT_EVENT

CS_QUERY_REGISTRY_TAG = _cspace.CS_QUERY_REGISTRY_TAG

CS_LOAD_PLUGIN_ALWAYS = _cspace.CS_LOAD_PLUGIN_ALWAYS

CS_FX_SETALPHA = _cspace.CS_FX_SETALPHA

CS_FX_SETALPHA_INT = _cspace.CS_FX_SETALPHA_INT
class _csPyEventHandler(iEventHandler):
    __swig_setmethods__ = {}
    for _s in [iEventHandler]: __swig_setmethods__.update(_s.__swig_setmethods__)
    __setattr__ = lambda self, name, value: _swig_setattr(self, _csPyEventHandler, name, value)
    __swig_getmethods__ = {}
    for _s in [iEventHandler]: __swig_getmethods__.update(_s.__swig_getmethods__)
    __getattr__ = lambda self, name: _swig_getattr(self, _csPyEventHandler, name)
    def __repr__(self):
        return "<C _csPyEventHandler instance at %s>" % (self.this,)
    __swig_setmethods__["scfRefCount"] = _cspace._csPyEventHandler_scfRefCount_set
    __swig_getmethods__["scfRefCount"] = _cspace._csPyEventHandler_scfRefCount_get
    if _newclass:scfRefCount = property(_cspace._csPyEventHandler_scfRefCount_get, _cspace._csPyEventHandler_scfRefCount_set)
    __swig_setmethods__["scfWeakRefOwners"] = _cspace._csPyEventHandler_scfWeakRefOwners_set
    __swig_getmethods__["scfWeakRefOwners"] = _cspace._csPyEventHandler_scfWeakRefOwners_get
    if _newclass:scfWeakRefOwners = property(_cspace._csPyEventHandler_scfWeakRefOwners_get, _cspace._csPyEventHandler_scfWeakRefOwners_set)
    def scfRemoveRefOwners(*args): return _cspace._csPyEventHandler_scfRemoveRefOwners(*args)
    __swig_setmethods__["scfParent"] = _cspace._csPyEventHandler_scfParent_set
    __swig_getmethods__["scfParent"] = _cspace._csPyEventHandler_scfParent_get
    if _newclass:scfParent = property(_cspace._csPyEventHandler_scfParent_get, _cspace._csPyEventHandler_scfParent_set)
    def IncRef(*args): return _cspace._csPyEventHandler_IncRef(*args)
    def DecRef(*args): return _cspace._csPyEventHandler_DecRef(*args)
    def GetRefCount(*args): return _cspace._csPyEventHandler_GetRefCount(*args)
    def AddRefOwner(*args): return _cspace._csPyEventHandler_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cspace._csPyEventHandler_RemoveRefOwner(*args)
    def QueryInterface(*args): return _cspace._csPyEventHandler_QueryInterface(*args)
    def __init__(self, *args):
        _swig_setattr(self, _csPyEventHandler, 'this', _cspace.new__csPyEventHandler(*args))
        _swig_setattr(self, _csPyEventHandler, 'thisown', 1)
    def __del__(self, destroy=_cspace.delete__csPyEventHandler):
        try:
            if self.thisown: destroy(self)
        except: pass
    def HandleEvent(*args): return _cspace._csPyEventHandler_HandleEvent(*args)

class _csPyEventHandlerPtr(_csPyEventHandler):
    def __init__(self, this):
        _swig_setattr(self, _csPyEventHandler, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, _csPyEventHandler, 'thisown', 0)
        _swig_setattr(self, _csPyEventHandler,self.__class__,_csPyEventHandler)
_cspace._csPyEventHandler_swigregister(_csPyEventHandlerPtr)

class csPyEventHandler (_csPyEventHandler):
  """Python version of iEventHandler implementation.
     This class can be used as base class for event handlers in Python.
     Call csPyEventHandler.__init__(self) in __init__ of derived class.
  """
  def __init__ (self):
    _csPyEventHandler.__init__(self, self)

class _EventHandlerFuncWrapper (csPyEventHandler):
  def __init__ (self, func):
    csPyEventHandler.__init__(self)
    self._func = func
    # Make sure a reference keeps to this wrapper instance.
    self._func._cs_event_handler_wrapper = self
  def HandleEvent (self, event):
    return self._func(event)

def _csInitializer_SetupEventHandler (reg, obj,
    mask=(CSMASK_FrameProcess|CSMASK_Input|CSMASK_Broadcast)):
  """Replacement of C++ versions."""
  if callable(obj):
    # obj is a function
    hdlr = _EventHandlerFuncWrapper(obj)
    hdlr.thisown = 1
  else:
    # assume it is a iEventHandler
    hdlr = obj
  return csInitializer._SetupEventHandler(reg, hdlr, mask)

csInitializer.SetupEventHandler = \
  staticmethod(_csInitializer_SetupEventHandler)


def _csInitializer_RequestPlugins (reg, plugins):
  """Replacement of C++ version."""
  def _get_tuple (x):
    if callable(x):
      return tuple(x())
    else:
      return tuple(x)
  requests = csPluginRequestArray()
  for cls, intf, ident, ver in map(
      lambda x: _get_tuple(x), plugins):
    requests.Push(csPluginRequest(
      csString(cls), csString(intf), ident, ver))
  return csInitializer._RequestPlugins(reg, requests)

csInitializer.RequestPlugins = staticmethod(_csInitializer_RequestPlugins)



_CS_QUERY_REGISTRY = _cspace._CS_QUERY_REGISTRY

_CS_QUERY_REGISTRY_TAG_INTERFACE = _cspace._CS_QUERY_REGISTRY_TAG_INTERFACE

_SCF_QUERY_INTERFACE = _cspace._SCF_QUERY_INTERFACE

_SCF_QUERY_INTERFACE_SAFE = _cspace._SCF_QUERY_INTERFACE_SAFE

_CS_QUERY_PLUGIN_CLASS = _cspace._CS_QUERY_PLUGIN_CLASS

_CS_LOAD_PLUGIN = _cspace._CS_LOAD_PLUGIN

_CS_GET_CHILD_OBJECT = _cspace._CS_GET_CHILD_OBJECT

_CS_GET_NAMED_CHILD_OBJECT = _cspace._CS_GET_NAMED_CHILD_OBJECT

_CS_GET_FIRST_NAMED_CHILD_OBJECT = _cspace._CS_GET_FIRST_NAMED_CHILD_OBJECT
csReport = csReporterHelper.Report

def _GetIntfId (intf):
  return cvar.iSCF_SCF.GetInterfaceID(intf.__name__)
def _GetIntfVersion (intf):
  return eval('%s_scfGetVersion()' % intf.__name__, locals(), globals())

def CS_QUERY_REGISTRY (reg, intf):
  return _CS_QUERY_REGISTRY (reg, intf.__name__, _GetIntfVersion(intf))

def CS_QUERY_REGISTRY_TAG_INTERFACE (reg, tag, intf):
  return _CS_QUERY_REGISTRY_TAG_INTERFACE (reg, tag, intf.__name__,
    _GetIntfVersion(intf))

def SCF_QUERY_INTERFACE (obj, intf):
  return _SCF_QUERY_INTERFACE (obj, intf.__name__, _GetIntfVersion(intf))

def SCF_QUERY_INTERFACE_SAFE (obj, intf):
  return _SCF_QUERY_INTERFACE_SAFE(obj, intf.__name__,
    _GetIntfVersion(intf))

def CS_GET_CHILD_OBJECT (obj, intf):
  return _CS_GET_CHILD_OBJECT(obj, intf.__name__, _GetIntfVersion(intf))

def CS_GET_NAMED_CHILD_OBJECT (obj, intf, name):
  return _CS_GET_NAMED_CHILD_OBJECT(obj, intf.__name__,
    _GetIntfVersion(intf), name)

def CS_GET_FIRST_NAMED_CHILD_OBJECT (obj, intf, name):
  return CS_GET_FIRST_NAMED_CHILD_OBJECT (obj, intf.__name__,
    _GetIntfVersion(intf), name)

def CS_QUERY_PLUGIN_CLASS (obj, class_id, intf):
  return _CS_QUERY_PLUGIN_CLASS(obj, class_id, intf.__name__,
    _GetIntfVersion(intf))

def CS_LOAD_PLUGIN (obj, class_id, intf):
  return _CS_LOAD_PLUGIN(obj, class_id, intf.__name__,
    _GetIntfVersion(intf))

def CS_REQUEST_PLUGIN (name, intf):
  return (name, intf.__name__, cvar.iSCF_SCF.GetInterfaceID(intf.__name__),
    eval('%s_scfGetVersion()' % intf.__name__, locals(), globals()))

def CS_REQUEST_VFS ():
  return CS_REQUEST_PLUGIN("crystalspace.kernel.vfs", iVFS)

def CS_REQUEST_FONTSERVER ():
  return CS_REQUEST_PLUGIN("crystalspace.font.server.default", iFontServer)

def CS_REQUEST_IMAGELOADER ():
  return CS_REQUEST_PLUGIN("crystalspace.graphic.image.io.multiplex",
    iImageIO)

def CS_REQUEST_NULL3D ():
  return CS_REQUEST_PLUGIN("crystalspace.graphics3d.null", iGraphics3D)

def CS_REQUEST_SOFTWARE3D ():
  return CS_REQUEST_PLUGIN("crystalspace.graphics3d.software", iGraphics3D)

def CS_REQUEST_OPENGL3D ():
  return CS_REQUEST_PLUGIN("crystalspace.graphics3d.opengl", iGraphics3D)

def CS_REQUEST_ENGINE ():
  return CS_REQUEST_PLUGIN("crystalspace.engine.3d", iEngine)

def CS_REQUEST_LEVELLOADER ():
  return CS_REQUEST_PLUGIN("crystalspace.level.loader", iLoader)

def CS_REQUEST_LEVELSAVER ():
  return CS_REQUEST_PLUGIN("crystalspace.level.saver", iSaver)

def CS_REQUEST_REPORTER ():
  return CS_REQUEST_PLUGIN("crystalspace.utilities.reporter", iReporter)

def CS_REQUEST_REPORTERLISTENER ():
  return CS_REQUEST_PLUGIN("crystalspace.utilities.stdrep",
    iStandardReporterListener)

def CS_REQUEST_CONSOLEOUT ():
  return CS_REQUEST_PLUGIN("crystalspace.console.output.simple",
    iConsoleOutput)


class CSMutableArrayHelper:
  def __init__(self, getFunc, lenFunc):
    self.getFunc = getFunc
    self.lenFunc = lenFunc

  def __len__(self):
    return self.lenFunc()

  def __getitem__(self, key):
    if type(key) != type(0):
      raise TypeError()
    arrlen = self.lenFunc()
    if key < 0 or key >= arrlen:
      raise IndexError('Length is ' + str(arrlen) + ', you asked for ' +
        str(key))
    return self.getFunc(key)

  # We do not implement __setitem__ because the only legal action is to
  #  overwrite the object at the given location.  (The contents of the
  #  array are mutable, but the array is a single allocation of a single
  #  type.)  Callers should be using the contained objects' own
  #  __setitem__ or mutation methods.

  # We do not implement __delitem__ because we cannot delete items.

CS_VEC_FORWARD = csVector3(0,0,1)
CS_VEC_BACKWARD = csVector3(0,0,-1)
CS_VEC_RIGHT = csVector3(1,0,0)
CS_VEC_LEFT = csVector3(-1,0,0)
CS_VEC_UP = csVector3(0,1,0)
CS_VEC_DOWN = csVector3(0,-1,0)
CS_VEC_ROT_RIGHT = csVector3(0,1,0)
CS_VEC_ROT_LEFT = csVector3(0,-1,0)
CS_VEC_TILT_RIGHT = -csVector3(0,0,1)
CS_VEC_TILT_LEFT = -csVector3(0,0,-1) 
CS_VEC_TILT_UP = -csVector3(1,0,0)
CS_VEC_TILT_DOWN = -csVector3(-1,0,0)


CS_POLYRANGE_LAST = csPolygonRange (-1, -1)


