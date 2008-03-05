# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.31
#
# Don't modify this file, modify the SWIG interface instead.

import _cstool
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
import iengine
import ivideo
import csgfx
_SetSCFPointer = _cstool._SetSCFPointer
_GetSCFPointer = _cstool._GetSCFPointer
if not "core" in dir():
    core = __import__("cspace").__dict__["core"]
core.AddSCFLink(_SetSCFPointer)
CSMutableArrayHelper = core.CSMutableArrayHelper

class scfFakecsColliderWrapper(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_scfFakecsColliderWrapper(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_scfFakecsColliderWrapper
    __del__ = lambda self : None;
scfFakecsColliderWrapper_swigregister = _cstool.scfFakecsColliderWrapper_swigregister
scfFakecsColliderWrapper_swigregister(scfFakecsColliderWrapper)

class scfColliderWrapper(core.csObject,scfFakecsColliderWrapper):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def IncRef(*args): return _cstool.scfColliderWrapper_IncRef(*args)
    def DecRef(*args): return _cstool.scfColliderWrapper_DecRef(*args)
    def GetRefCount(*args): return _cstool.scfColliderWrapper_GetRefCount(*args)
    def QueryInterface(*args): return _cstool.scfColliderWrapper_QueryInterface(*args)
    def AddRefOwner(*args): return _cstool.scfColliderWrapper_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cstool.scfColliderWrapper_RemoveRefOwner(*args)
    def GetInterfaceMetadata(*args): return _cstool.scfColliderWrapper_GetInterfaceMetadata(*args)
scfColliderWrapper_swigregister = _cstool.scfColliderWrapper_swigregister
scfColliderWrapper_swigregister(scfColliderWrapper)

class csColliderWrapper(scfColliderWrapper):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csColliderWrapper(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csColliderWrapper
    __del__ = lambda self : None;
    def GetCollider(*args): return _cstool.csColliderWrapper_GetCollider(*args)
    def GetCollideSystem(*args): return _cstool.csColliderWrapper_GetCollideSystem(*args)
    def Collide(*args): return _cstool.csColliderWrapper_Collide(*args)
    GetColliderWrapper = staticmethod(_cstool.csColliderWrapper_GetColliderWrapper)
    def UpdateCollider(*args): return _cstool.csColliderWrapper_UpdateCollider(*args)
csColliderWrapper_swigregister = _cstool.csColliderWrapper_swigregister
csColliderWrapper_swigregister(csColliderWrapper)
csColliderWrapper_GetColliderWrapper = _cstool.csColliderWrapper_GetColliderWrapper

class csTraceBeamResult(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    closest_tri = _swig_property(_cstool.csTraceBeamResult_closest_tri_get, _cstool.csTraceBeamResult_closest_tri_set)
    closest_isect = _swig_property(_cstool.csTraceBeamResult_closest_isect_get, _cstool.csTraceBeamResult_closest_isect_set)
    closest_mesh = _swig_property(_cstool.csTraceBeamResult_closest_mesh_get, _cstool.csTraceBeamResult_closest_mesh_set)
    sqdistance = _swig_property(_cstool.csTraceBeamResult_sqdistance_get, _cstool.csTraceBeamResult_sqdistance_set)
    end_sector = _swig_property(_cstool.csTraceBeamResult_end_sector_get, _cstool.csTraceBeamResult_end_sector_set)
    def __init__(self, *args): 
        this = _cstool.new_csTraceBeamResult(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csTraceBeamResult
    __del__ = lambda self : None;
csTraceBeamResult_swigregister = _cstool.csTraceBeamResult_swigregister
csTraceBeamResult_swigregister(csTraceBeamResult)

class csColliderHelper(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    InitializeCollisionWrapper = staticmethod(_cstool.csColliderHelper_InitializeCollisionWrapper)
    InitializeCollisionWrappers = staticmethod(_cstool.csColliderHelper_InitializeCollisionWrappers)
    InitializeCollisionWrappersCollection = staticmethod(_cstool.csColliderHelper_InitializeCollisionWrappersCollection)
    InitializeCollisionWrappersRegion = staticmethod(_cstool.csColliderHelper_InitializeCollisionWrappersRegion)
    CollideArray = staticmethod(_cstool.csColliderHelper_CollideArray)
    CollidePath = staticmethod(_cstool.csColliderHelper_CollidePath)
    TraceBeam = staticmethod(_cstool.csColliderHelper_TraceBeam)
    def __init__(self, *args): 
        this = _cstool.new_csColliderHelper(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csColliderHelper
    __del__ = lambda self : None;
csColliderHelper_swigregister = _cstool.csColliderHelper_swigregister
csColliderHelper_swigregister(csColliderHelper)
csColliderHelper_InitializeCollisionWrapper = _cstool.csColliderHelper_InitializeCollisionWrapper
csColliderHelper_InitializeCollisionWrappers = _cstool.csColliderHelper_InitializeCollisionWrappers
csColliderHelper_InitializeCollisionWrappersCollection = _cstool.csColliderHelper_InitializeCollisionWrappersCollection
csColliderHelper_InitializeCollisionWrappersRegion = _cstool.csColliderHelper_InitializeCollisionWrappersRegion
csColliderHelper_CollideArray = _cstool.csColliderHelper_CollideArray
csColliderHelper_CollidePath = _cstool.csColliderHelper_CollidePath
csColliderHelper_TraceBeam = _cstool.csColliderHelper_TraceBeam

class csColliderActor(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csColliderActor(*args)
        try: self.this.append(this)
        except: self.this = this
    def SetCollideSystem(*args): return _cstool.csColliderActor_SetCollideSystem(*args)
    def SetEngine(*args): return _cstool.csColliderActor_SetEngine(*args)
    def InitializeColliders(*args): return _cstool.csColliderActor_InitializeColliders(*args)
    def SetCamera(*args): return _cstool.csColliderActor_SetCamera(*args)
    def SetGravity(*args): return _cstool.csColliderActor_SetGravity(*args)
    def GetGravity(*args): return _cstool.csColliderActor_GetGravity(*args)
    def IsOnGround(*args): return _cstool.csColliderActor_IsOnGround(*args)
    def SetOnGround(*args): return _cstool.csColliderActor_SetOnGround(*args)
    def HasCD(*args): return _cstool.csColliderActor_HasCD(*args)
    def SetCD(*args): return _cstool.csColliderActor_SetCD(*args)
    def CheckRevertMove(*args): return _cstool.csColliderActor_CheckRevertMove(*args)
    def EnableHitMeshes(*args): return _cstool.csColliderActor_EnableHitMeshes(*args)
    def CheckHitMeshes(*args): return _cstool.csColliderActor_CheckHitMeshes(*args)
    def GetHitMeshes(*args): return _cstool.csColliderActor_GetHitMeshes(*args)
    def Move(*args): return _cstool.csColliderActor_Move(*args)
    def GetRotation(*args): return _cstool.csColliderActor_GetRotation(*args)
    def SetRotation(*args): return _cstool.csColliderActor_SetRotation(*args)
    def AdjustForCollisions(*args): return _cstool.csColliderActor_AdjustForCollisions(*args)
    __swig_destroy__ = _cstool.delete_csColliderActor
    __del__ = lambda self : None;
csColliderActor_swigregister = _cstool.csColliderActor_swigregister
csColliderActor_swigregister(csColliderActor)

class csView(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csView(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csView
    __del__ = lambda self : None;
    def GetEngine(*args): return _cstool.csView_GetEngine(*args)
    def SetEngine(*args): return _cstool.csView_SetEngine(*args)
    def GetCamera(*args): return _cstool.csView_GetCamera(*args)
    def SetCamera(*args): return _cstool.csView_SetCamera(*args)
    def GetContext(*args): return _cstool.csView_GetContext(*args)
    def SetContext(*args): return _cstool.csView_SetContext(*args)
    def SetRectangle(*args): return _cstool.csView_SetRectangle(*args)
    def ClearView(*args): return _cstool.csView_ClearView(*args)
    def AddViewVertex(*args): return _cstool.csView_AddViewVertex(*args)
    def RestrictClipperToScreen(*args): return _cstool.csView_RestrictClipperToScreen(*args)
    def SetAutoResize(*args): return _cstool.csView_SetAutoResize(*args)
    def UpdateClipper(*args): return _cstool.csView_UpdateClipper(*args)
    def GetClipper(*args): return _cstool.csView_GetClipper(*args)
    def Draw(*args): return _cstool.csView_Draw(*args)
csView_swigregister = _cstool.csView_swigregister
csView_swigregister(csView)

csfxInterference = _cstool.csfxInterference
csfxFadeOut = _cstool.csfxFadeOut
csfxFadeTo = _cstool.csfxFadeTo
csfxFadeToColor = _cstool.csfxFadeToColor
csfxGreenScreen = _cstool.csfxGreenScreen
csfxRedScreen = _cstool.csfxRedScreen
csfxBlueScreen = _cstool.csfxBlueScreen
csfxWhiteOut = _cstool.csfxWhiteOut
csfxScreenDPFX = _cstool.csfxScreenDPFX
csfxScreenDPFXPartial = _cstool.csfxScreenDPFXPartial
class csPixmap(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _cstool.delete_csPixmap
    __del__ = lambda self : None;
    def Width(*args): return _cstool.csPixmap_Width(*args)
    def Height(*args): return _cstool.csPixmap_Height(*args)
    def Advance(*args): return _cstool.csPixmap_Advance(*args)
    def GetTextureHandle(*args): return _cstool.csPixmap_GetTextureHandle(*args)
    def DrawScaled(*args): return _cstool.csPixmap_DrawScaled(*args)
    def DrawScaledAlign(*args): return _cstool.csPixmap_DrawScaledAlign(*args)
    def Draw(*args): return _cstool.csPixmap_Draw(*args)
    def DrawAlign(*args): return _cstool.csPixmap_DrawAlign(*args)
    def DrawTiled(*args): return _cstool.csPixmap_DrawTiled(*args)
csPixmap_swigregister = _cstool.csPixmap_swigregister
csPixmap_swigregister(csPixmap)
csfxShadeVert = _cstool.csfxShadeVert

class csSimplePixmap(csPixmap):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csSimplePixmap(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csSimplePixmap
    __del__ = lambda self : None;
    def SetTextureHandle(*args): return _cstool.csSimplePixmap_SetTextureHandle(*args)
    def SetTextureRectangle(*args): return _cstool.csSimplePixmap_SetTextureRectangle(*args)
    def DrawScaled(*args): return _cstool.csSimplePixmap_DrawScaled(*args)
    def DrawTiled(*args): return _cstool.csSimplePixmap_DrawTiled(*args)
    def Width(*args): return _cstool.csSimplePixmap_Width(*args)
    def Height(*args): return _cstool.csSimplePixmap_Height(*args)
    def Advance(*args): return _cstool.csSimplePixmap_Advance(*args)
    def GetTextureHandle(*args): return _cstool.csSimplePixmap_GetTextureHandle(*args)
csSimplePixmap_swigregister = _cstool.csSimplePixmap_swigregister
csSimplePixmap_swigregister(csSimplePixmap)

class csShortestDistanceResult(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    sqdistance = _swig_property(_cstool.csShortestDistanceResult_sqdistance_get, _cstool.csShortestDistanceResult_sqdistance_set)
    direction = _swig_property(_cstool.csShortestDistanceResult_direction_get, _cstool.csShortestDistanceResult_direction_set)
    def __init__(self, *args): 
        this = _cstool.new_csShortestDistanceResult(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csShortestDistanceResult
    __del__ = lambda self : None;
csShortestDistanceResult_swigregister = _cstool.csShortestDistanceResult_swigregister
csShortestDistanceResult_swigregister(csShortestDistanceResult)

class csScreenTargetResult(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    mesh = _swig_property(_cstool.csScreenTargetResult_mesh_get, _cstool.csScreenTargetResult_mesh_set)
    isect = _swig_property(_cstool.csScreenTargetResult_isect_get, _cstool.csScreenTargetResult_isect_set)
    polygon_idx = _swig_property(_cstool.csScreenTargetResult_polygon_idx_get, _cstool.csScreenTargetResult_polygon_idx_set)
    def __init__(self, *args): 
        this = _cstool.new_csScreenTargetResult(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csScreenTargetResult
    __del__ = lambda self : None;
csScreenTargetResult_swigregister = _cstool.csScreenTargetResult_swigregister
csScreenTargetResult_swigregister(csScreenTargetResult)

class csEngineTools(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    FindShortestDistance = staticmethod(_cstool.csEngineTools_FindShortestDistance)
    FindScreenTarget = staticmethod(_cstool.csEngineTools_FindScreenTarget)
    def __init__(self, *args): 
        this = _cstool.new_csEngineTools(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csEngineTools
    __del__ = lambda self : None;
csEngineTools_swigregister = _cstool.csEngineTools_swigregister
csEngineTools_swigregister(csEngineTools)
csEngineTools_FindShortestDistance = _cstool.csEngineTools_FindShortestDistance
csEngineTools_FindScreenTarget = _cstool.csEngineTools_FindScreenTarget

class Primitive(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _cstool.delete_Primitive
    __del__ = lambda self : None;
    def Append(*args): return _cstool.Primitive_Append(*args)
Primitive_swigregister = _cstool.Primitive_swigregister
Primitive_swigregister(Primitive)

class TesselatedQuad(Primitive):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_TesselatedQuad(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_TesselatedQuad
    __del__ = lambda self : None;
    def SetLevel(*args): return _cstool.TesselatedQuad_SetLevel(*args)
    def GetLevel(*args): return _cstool.TesselatedQuad_GetLevel(*args)
    def SetMapper(*args): return _cstool.TesselatedQuad_SetMapper(*args)
    def Append(*args): return _cstool.TesselatedQuad_Append(*args)
TesselatedQuad_swigregister = _cstool.TesselatedQuad_swigregister
TesselatedQuad_swigregister(TesselatedQuad)

class TesselatedBox(Primitive):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_TesselatedBox(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_TesselatedBox
    __del__ = lambda self : None;
    def SetLevel(*args): return _cstool.TesselatedBox_SetLevel(*args)
    def GetLevel(*args): return _cstool.TesselatedBox_GetLevel(*args)
    def SetMapper(*args): return _cstool.TesselatedBox_SetMapper(*args)
    def SetFlags(*args): return _cstool.TesselatedBox_SetFlags(*args)
    def GetFlags(*args): return _cstool.TesselatedBox_GetFlags(*args)
    def Append(*args): return _cstool.TesselatedBox_Append(*args)
TesselatedBox_swigregister = _cstool.TesselatedBox_swigregister
TesselatedBox_swigregister(TesselatedBox)

class Box(Primitive):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_Box(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_Box
    __del__ = lambda self : None;
    def SetMapper(*args): return _cstool.Box_SetMapper(*args)
    def SetFlags(*args): return _cstool.Box_SetFlags(*args)
    def GetFlags(*args): return _cstool.Box_GetFlags(*args)
    def Append(*args): return _cstool.Box_Append(*args)
Box_swigregister = _cstool.Box_swigregister
Box_swigregister(Box)

class Capsule(Primitive):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_Capsule(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_Capsule
    __del__ = lambda self : None;
    def SetMapper(*args): return _cstool.Capsule_SetMapper(*args)
    def Append(*args): return _cstool.Capsule_Append(*args)
Capsule_swigregister = _cstool.Capsule_swigregister
Capsule_swigregister(Capsule)

class Sphere(Primitive):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_Sphere(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_Sphere
    __del__ = lambda self : None;
    def SetCylindricalMapping(*args): return _cstool.Sphere_SetCylindricalMapping(*args)
    def HasCylindricalMapping(*args): return _cstool.Sphere_HasCylindricalMapping(*args)
    def SetTopOnly(*args): return _cstool.Sphere_SetTopOnly(*args)
    def IsTopOnly(*args): return _cstool.Sphere_IsTopOnly(*args)
    def SetReversed(*args): return _cstool.Sphere_SetReversed(*args)
    def IsReversed(*args): return _cstool.Sphere_IsReversed(*args)
    def SetMapper(*args): return _cstool.Sphere_SetMapper(*args)
    def Append(*args): return _cstool.Sphere_Append(*args)
Sphere_swigregister = _cstool.Sphere_swigregister
Sphere_swigregister(Sphere)

class GeneralMeshBuilder(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    CreateFactory = staticmethod(_cstool.GeneralMeshBuilder_CreateFactory)
    CreateMesh = staticmethod(_cstool.GeneralMeshBuilder_CreateMesh)
    CreateFactoryAndMesh = staticmethod(_cstool.GeneralMeshBuilder_CreateFactoryAndMesh)
    def __init__(self, *args): 
        this = _cstool.new_GeneralMeshBuilder(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_GeneralMeshBuilder
    __del__ = lambda self : None;
GeneralMeshBuilder_swigregister = _cstool.GeneralMeshBuilder_swigregister
GeneralMeshBuilder_swigregister(GeneralMeshBuilder)
GeneralMeshBuilder_CreateFactory = _cstool.GeneralMeshBuilder_CreateFactory
GeneralMeshBuilder_CreateMesh = _cstool.GeneralMeshBuilder_CreateMesh
GeneralMeshBuilder_CreateFactoryAndMesh = _cstool.GeneralMeshBuilder_CreateFactoryAndMesh

CS_PEN_TA_TOP = _cstool.CS_PEN_TA_TOP
CS_PEN_TA_BOT = _cstool.CS_PEN_TA_BOT
CS_PEN_TA_LEFT = _cstool.CS_PEN_TA_LEFT
CS_PEN_TA_RIGHT = _cstool.CS_PEN_TA_RIGHT
CS_PEN_TA_CENTER = _cstool.CS_PEN_TA_CENTER
CS_PEN_FILL = _cstool.CS_PEN_FILL
CS_PEN_SWAPCOLORS = _cstool.CS_PEN_SWAPCOLORS
CS_PEN_TEXTURE_ONLY = _cstool.CS_PEN_TEXTURE_ONLY
CS_PEN_TEXTURE = _cstool.CS_PEN_TEXTURE
class iPen(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def SetFlag(*args): return _cstool.iPen_SetFlag(*args)
    def ClearFlag(*args): return _cstool.iPen_ClearFlag(*args)
    def SetMixMode(*args): return _cstool.iPen_SetMixMode(*args)
    def SetColor(*args): return _cstool.iPen_SetColor(*args)
    def SetTexture(*args): return _cstool.iPen_SetTexture(*args)
    def SwapColors(*args): return _cstool.iPen_SwapColors(*args)
    def SetPenWidth(*args): return _cstool.iPen_SetPenWidth(*args)
    def ClearTransform(*args): return _cstool.iPen_ClearTransform(*args)
    def PushTransform(*args): return _cstool.iPen_PushTransform(*args)
    def PopTransform(*args): return _cstool.iPen_PopTransform(*args)
    def SetOrigin(*args): return _cstool.iPen_SetOrigin(*args)
    def Translate(*args): return _cstool.iPen_Translate(*args)
    def DrawLine(*args): return _cstool.iPen_DrawLine(*args)
    def DrawPoint(*args): return _cstool.iPen_DrawPoint(*args)
    def DrawRect(*args): return _cstool.iPen_DrawRect(*args)
    def DrawMiteredRect(*args): return _cstool.iPen_DrawMiteredRect(*args)
    def DrawRoundedRect(*args): return _cstool.iPen_DrawRoundedRect(*args)
    def DrawArc(*args): return _cstool.iPen_DrawArc(*args)
    def DrawTriangle(*args): return _cstool.iPen_DrawTriangle(*args)
    def Write(*args): return _cstool.iPen_Write(*args)
    def WriteBoxed(*args): return _cstool.iPen_WriteBoxed(*args)
    def _Rotate(*args): return _cstool.iPen__Rotate(*args)
    def Rotate(self,a):
         return _cspace.iPen__Rotate(a)

iPen_swigregister = _cstool.iPen_swigregister
iPen_swigregister(iPen)

class csPen(iPen):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csPen(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csPen
    __del__ = lambda self : None;
    def SetFlag(*args): return _cstool.csPen_SetFlag(*args)
    def ClearFlag(*args): return _cstool.csPen_ClearFlag(*args)
    def SetMixMode(*args): return _cstool.csPen_SetMixMode(*args)
    def SetColor(*args): return _cstool.csPen_SetColor(*args)
    def SetTexture(*args): return _cstool.csPen_SetTexture(*args)
    def SwapColors(*args): return _cstool.csPen_SwapColors(*args)
    def SetPenWidth(*args): return _cstool.csPen_SetPenWidth(*args)
    def ClearTransform(*args): return _cstool.csPen_ClearTransform(*args)
    def PushTransform(*args): return _cstool.csPen_PushTransform(*args)
    def PopTransform(*args): return _cstool.csPen_PopTransform(*args)
    def SetOrigin(*args): return _cstool.csPen_SetOrigin(*args)
    def Translate(*args): return _cstool.csPen_Translate(*args)
    def DrawLine(*args): return _cstool.csPen_DrawLine(*args)
    def DrawThickLine(*args): return _cstool.csPen_DrawThickLine(*args)
    def DrawPoint(*args): return _cstool.csPen_DrawPoint(*args)
    def DrawRect(*args): return _cstool.csPen_DrawRect(*args)
    def DrawMiteredRect(*args): return _cstool.csPen_DrawMiteredRect(*args)
    def DrawRoundedRect(*args): return _cstool.csPen_DrawRoundedRect(*args)
    def DrawArc(*args): return _cstool.csPen_DrawArc(*args)
    def DrawTriangle(*args): return _cstool.csPen_DrawTriangle(*args)
    def Write(*args): return _cstool.csPen_Write(*args)
    def WriteBoxed(*args): return _cstool.csPen_WriteBoxed(*args)
csPen_swigregister = _cstool.csPen_swigregister
csPen_swigregister(csPen)

class csMemoryPen(iPen):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csMemoryPen(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csMemoryPen
    __del__ = lambda self : None;
    def Clear(*args): return _cstool.csMemoryPen_Clear(*args)
    def Draw(*args): return _cstool.csMemoryPen_Draw(*args)
    def SetFlag(*args): return _cstool.csMemoryPen_SetFlag(*args)
    def ClearFlag(*args): return _cstool.csMemoryPen_ClearFlag(*args)
    def SetMixMode(*args): return _cstool.csMemoryPen_SetMixMode(*args)
    def SetColor(*args): return _cstool.csMemoryPen_SetColor(*args)
    def SetTexture(*args): return _cstool.csMemoryPen_SetTexture(*args)
    def SwapColors(*args): return _cstool.csMemoryPen_SwapColors(*args)
    def SetPenWidth(*args): return _cstool.csMemoryPen_SetPenWidth(*args)
    def ClearTransform(*args): return _cstool.csMemoryPen_ClearTransform(*args)
    def PushTransform(*args): return _cstool.csMemoryPen_PushTransform(*args)
    def PopTransform(*args): return _cstool.csMemoryPen_PopTransform(*args)
    def SetOrigin(*args): return _cstool.csMemoryPen_SetOrigin(*args)
    def Translate(*args): return _cstool.csMemoryPen_Translate(*args)
    def DrawLine(*args): return _cstool.csMemoryPen_DrawLine(*args)
    def DrawPoint(*args): return _cstool.csMemoryPen_DrawPoint(*args)
    def DrawRect(*args): return _cstool.csMemoryPen_DrawRect(*args)
    def DrawMiteredRect(*args): return _cstool.csMemoryPen_DrawMiteredRect(*args)
    def DrawRoundedRect(*args): return _cstool.csMemoryPen_DrawRoundedRect(*args)
    def DrawArc(*args): return _cstool.csMemoryPen_DrawArc(*args)
    def DrawTriangle(*args): return _cstool.csMemoryPen_DrawTriangle(*args)
    def Write(*args): return _cstool.csMemoryPen_Write(*args)
    def WriteBoxed(*args): return _cstool.csMemoryPen_WriteBoxed(*args)
csMemoryPen_swigregister = _cstool.csMemoryPen_swigregister
csMemoryPen_swigregister(csMemoryPen)

class scfProcTexture(core.csObject,iengine.iTextureWrapper,core.iProcTexture):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def IncRef(*args): return _cstool.scfProcTexture_IncRef(*args)
    def DecRef(*args): return _cstool.scfProcTexture_DecRef(*args)
    def GetRefCount(*args): return _cstool.scfProcTexture_GetRefCount(*args)
    def QueryInterface(*args): return _cstool.scfProcTexture_QueryInterface(*args)
    def AddRefOwner(*args): return _cstool.scfProcTexture_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _cstool.scfProcTexture_RemoveRefOwner(*args)
    def GetInterfaceMetadata(*args): return _cstool.scfProcTexture_GetInterfaceMetadata(*args)
scfProcTexture_swigregister = _cstool.scfProcTexture_swigregister
scfProcTexture_swigregister(scfProcTexture)

class iProcTexCallback(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def GetProcTexture(*args): return _cstool.iProcTexCallback_GetProcTexture(*args)
    __swig_destroy__ = _cstool.delete_iProcTexCallback
    __del__ = lambda self : None;
iProcTexCallback_swigregister = _cstool.iProcTexCallback_swigregister
iProcTexCallback_swigregister(iProcTexCallback)

class csProcTexture(scfProcTexture):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    last_cur_time = _swig_property(_cstool.csProcTexture_last_cur_time_get, _cstool.csProcTexture_last_cur_time_set)
    __swig_destroy__ = _cstool.delete_csProcTexture
    __del__ = lambda self : None;
    def GetG3D(*args): return _cstool.csProcTexture_GetG3D(*args)
    def GetG2D(*args): return _cstool.csProcTexture_GetG2D(*args)
    def DisableAutoUpdate(*args): return _cstool.csProcTexture_DisableAutoUpdate(*args)
    def Initialize(*args): return _cstool.csProcTexture_Initialize(*args)
    def PrepareAnim(*args): return _cstool.csProcTexture_PrepareAnim(*args)
    def SetKeyColor(*args): return _cstool.csProcTexture_SetKeyColor(*args)
    def Animate(*args): return _cstool.csProcTexture_Animate(*args)
    def GetDimension(*args): return _cstool.csProcTexture_GetDimension(*args)
    GetRandom = staticmethod(_cstool.csProcTexture_GetRandom)
    def GetTextureWrapper(*args): return _cstool.csProcTexture_GetTextureWrapper(*args)
csProcTexture_swigregister = _cstool.csProcTexture_swigregister
csProcTexture_swigregister(csProcTexture)
csProcTexture_GetRandom = _cstool.csProcTexture_GetRandom

class csProcAnimated(csProcTexture):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _cstool.new_csProcAnimated(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _cstool.delete_csProcAnimated
    __del__ = lambda self : None;
    def PrepareAnim(*args): return _cstool.csProcAnimated_PrepareAnim(*args)
    def Animate(*args): return _cstool.csProcAnimated_Animate(*args)
csProcAnimated_swigregister = _cstool.csProcAnimated_swigregister
csProcAnimated_swigregister(csProcAnimated)



