# This file was created automatically by SWIG.
import cspacec
import new
class iBase:
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iBase instance at %s>" % (self.this,)
class iBasePtr(iBase):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iBase



class iSCF(iBase):
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iSCF instance at %s>" % (self.this,)
class iSCFPtr(iSCF):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iSCF


iSCF.scfCreateInstance = new.instancemethod(cspacec.iSCF_scfCreateInstance, None, iSCF)

class csVector3:
    def __init__(self,*args):
        self.this = apply(cspacec.new_csVector3,args)
        self.thisown = 1

    __setmethods__ = {
        "x" : cspacec.csVector3_x_set,
        "y" : cspacec.csVector3_y_set,
        "z" : cspacec.csVector3_z_set,
    }
    def __setattr__(self,name,value):
        if (name == "this") or (name == "thisown"): self.__dict__[name] = value; return
        method = csVector3.__setmethods__.get(name,None)
        if method: return method(self,value)
        self.__dict__[name] = value
    __getmethods__ = {
        "x" : cspacec.csVector3_x_get,
        "y" : cspacec.csVector3_y_get,
        "z" : cspacec.csVector3_z_get,
    }
    def __getattr__(self,name):
        method = csVector3.__getmethods__.get(name,None)
        if method: return method(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C csVector3 instance at %s>" % (self.this,)
class csVector3Ptr(csVector3):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = csVector3


csVector3.__str__ = new.instancemethod(cspacec.csVector3___str__, None, csVector3)

class csRGBpixel:
    def __init__(self,this):
        self.this = this

    __setmethods__ = {
        "red" : cspacec.csRGBpixel_red_set,
        "green" : cspacec.csRGBpixel_green_set,
        "blue" : cspacec.csRGBpixel_blue_set,
        "alpha" : cspacec.csRGBpixel_alpha_set,
    }
    def __setattr__(self,name,value):
        if (name == "this") or (name == "thisown"): self.__dict__[name] = value; return
        method = csRGBpixel.__setmethods__.get(name,None)
        if method: return method(self,value)
        self.__dict__[name] = value
    __getmethods__ = {
        "red" : cspacec.csRGBpixel_red_get,
        "green" : cspacec.csRGBpixel_green_get,
        "blue" : cspacec.csRGBpixel_blue_get,
        "alpha" : cspacec.csRGBpixel_alpha_get,
    }
    def __getattr__(self,name):
        method = csRGBpixel.__getmethods__.get(name,None)
        if method: return method(self)
        raise AttributeError,name
    def __repr__(self):
        return "<C csRGBpixel instance at %s>" % (self.this,)
class csRGBpixelPtr(csRGBpixel):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = csRGBpixel



class iPlugIn(iBase):
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iPlugIn instance at %s>" % (self.this,)
class iPlugInPtr(iPlugIn):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iPlugIn


iPlugIn.Initialize = new.instancemethod(cspacec.iPlugIn_Initialize, None, iPlugIn)
iPlugIn.HandleEvent = new.instancemethod(cspacec.iPlugIn_HandleEvent, None, iPlugIn)
iPlugIn.SuspendResume = new.instancemethod(cspacec.iPlugIn_SuspendResume, None, iPlugIn)

class iTextureHandle(iBase):
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iTextureHandle instance at %s>" % (self.this,)
class iTextureHandlePtr(iTextureHandle):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iTextureHandle


iTextureHandle.GetMipMapDimensions = new.instancemethod(cspacec.iTextureHandle_GetMipMapDimensions, None, iTextureHandle)
iTextureHandle.GetMeanColor = new.instancemethod(cspacec.iTextureHandle_GetMeanColor, None, iTextureHandle)
iTextureHandle.GetCacheData = new.instancemethod(cspacec.iTextureHandle_GetCacheData, None, iTextureHandle)
iTextureHandle.SetCacheData = new.instancemethod(cspacec.iTextureHandle_SetCacheData, None, iTextureHandle)
iTextureHandle.GetPrivateObject = new.instancemethod(cspacec.iTextureHandle_GetPrivateObject, None, iTextureHandle)

class iGraphics3D(iPlugIn):
    def __init__(self,this):
        self.this = this

    def GetTextureManager(*args):
        val = apply(cspacec.iGraphics3D_GetTextureManager,args)
        if val: val = iTextureManagerPtr(val) 
        return val
    def __repr__(self):
        return "<C iGraphics3D instance at %s>" % (self.this,)
class iGraphics3DPtr(iGraphics3D):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iGraphics3D


iGraphics3D.Initialize = new.instancemethod(cspacec.iGraphics3D_Initialize, None, iGraphics3D)
iGraphics3D.Open = new.instancemethod(cspacec.iGraphics3D_Open, None, iGraphics3D)
iGraphics3D.Close = new.instancemethod(cspacec.iGraphics3D_Close, None, iGraphics3D)
iGraphics3D.SetDimensions = new.instancemethod(cspacec.iGraphics3D_SetDimensions, None, iGraphics3D)
iGraphics3D.BeginDraw = new.instancemethod(cspacec.iGraphics3D_BeginDraw, None, iGraphics3D)
iGraphics3D.FinishDraw = new.instancemethod(cspacec.iGraphics3D_FinishDraw, None, iGraphics3D)
iGraphics3D.Print = new.instancemethod(cspacec.iGraphics3D_Print, None, iGraphics3D)
iGraphics3D.DrawPolygon = new.instancemethod(cspacec.iGraphics3D_DrawPolygon, None, iGraphics3D)
iGraphics3D.DrawPolygonDebug = new.instancemethod(cspacec.iGraphics3D_DrawPolygonDebug, None, iGraphics3D)
iGraphics3D.DrawLine = new.instancemethod(cspacec.iGraphics3D_DrawLine, None, iGraphics3D)
iGraphics3D.StartPolygonFX = new.instancemethod(cspacec.iGraphics3D_StartPolygonFX, None, iGraphics3D)
iGraphics3D.FinishPolygonFX = new.instancemethod(cspacec.iGraphics3D_FinishPolygonFX, None, iGraphics3D)
iGraphics3D.DrawPolygonFX = new.instancemethod(cspacec.iGraphics3D_DrawPolygonFX, None, iGraphics3D)
iGraphics3D.DrawTriangleMesh = new.instancemethod(cspacec.iGraphics3D_DrawTriangleMesh, None, iGraphics3D)
iGraphics3D.DrawPolygonMesh = new.instancemethod(cspacec.iGraphics3D_DrawPolygonMesh, None, iGraphics3D)
iGraphics3D.OpenFogObject = new.instancemethod(cspacec.iGraphics3D_OpenFogObject, None, iGraphics3D)
iGraphics3D.DrawFogPolygon = new.instancemethod(cspacec.iGraphics3D_DrawFogPolygon, None, iGraphics3D)
iGraphics3D.CloseFogObject = new.instancemethod(cspacec.iGraphics3D_CloseFogObject, None, iGraphics3D)
iGraphics3D.SetRenderState = new.instancemethod(cspacec.iGraphics3D_SetRenderState, None, iGraphics3D)
iGraphics3D.GetRenderState = new.instancemethod(cspacec.iGraphics3D_GetRenderState, None, iGraphics3D)
iGraphics3D.GetCaps = new.instancemethod(cspacec.iGraphics3D_GetCaps, None, iGraphics3D)
iGraphics3D.GetZBuffAt = new.instancemethod(cspacec.iGraphics3D_GetZBuffAt, None, iGraphics3D)
iGraphics3D.GetZBuffValue = new.instancemethod(cspacec.iGraphics3D_GetZBuffValue, None, iGraphics3D)
iGraphics3D.DumpCache = new.instancemethod(cspacec.iGraphics3D_DumpCache, None, iGraphics3D)
iGraphics3D.ClearCache = new.instancemethod(cspacec.iGraphics3D_ClearCache, None, iGraphics3D)
iGraphics3D.RemoveFromCache = new.instancemethod(cspacec.iGraphics3D_RemoveFromCache, None, iGraphics3D)
iGraphics3D.GetWidth = new.instancemethod(cspacec.iGraphics3D_GetWidth, None, iGraphics3D)
iGraphics3D.GetHeight = new.instancemethod(cspacec.iGraphics3D_GetHeight, None, iGraphics3D)
iGraphics3D.SetPerspectiveCenter = new.instancemethod(cspacec.iGraphics3D_SetPerspectiveCenter, None, iGraphics3D)
iGraphics3D.SetPerspectiveAspect = new.instancemethod(cspacec.iGraphics3D_SetPerspectiveAspect, None, iGraphics3D)
iGraphics3D.SetObjectToCamera = new.instancemethod(cspacec.iGraphics3D_SetObjectToCamera, None, iGraphics3D)
iGraphics3D.SetClipper = new.instancemethod(cspacec.iGraphics3D_SetClipper, None, iGraphics3D)
iGraphics3D.GetDriver2D = new.instancemethod(cspacec.iGraphics3D_GetDriver2D, None, iGraphics3D)
iGraphics3D.CreateHalo = new.instancemethod(cspacec.iGraphics3D_CreateHalo, None, iGraphics3D)
iGraphics3D.DrawPixmap = new.instancemethod(cspacec.iGraphics3D_DrawPixmap, None, iGraphics3D)

class iCamera(iBase):
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iCamera instance at %s>" % (self.this,)
class iCameraPtr(iCamera):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iCamera


iCamera.GetAspect = new.instancemethod(cspacec.iCamera_GetAspect, None, iCamera)
iCamera.GetInvAspect = new.instancemethod(cspacec.iCamera_GetInvAspect, None, iCamera)

class iPolygonSet(iBase):
    def __init__(self,this):
        self.this = this

    def GetPolygon(*args):
        val = apply(cspacec.iPolygonSet_GetPolygon,args)
        if val: val = iPolygon3DPtr(val) 
        return val
    def CreatePolygon(*args):
        val = apply(cspacec.iPolygonSet_CreatePolygon,args)
        if val: val = iPolygon3DPtr(val) 
        return val
    def GetVertex(*args):
        val = apply(cspacec.iPolygonSet_GetVertex,args)
        if val: val = csVector3Ptr(val) 
        return val
    def GetVertexW(*args):
        val = apply(cspacec.iPolygonSet_GetVertexW,args)
        if val: val = csVector3Ptr(val) 
        return val
    def GetVertexC(*args):
        val = apply(cspacec.iPolygonSet_GetVertexC,args)
        if val: val = csVector3Ptr(val) 
        return val
    def __repr__(self):
        return "<C iPolygonSet instance at %s>" % (self.this,)
class iPolygonSetPtr(iPolygonSet):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iPolygonSet


iPolygonSet.GetName = new.instancemethod(cspacec.iPolygonSet_GetName, None, iPolygonSet)
iPolygonSet.SetName = new.instancemethod(cspacec.iPolygonSet_SetName, None, iPolygonSet)
iPolygonSet.CompressVertices = new.instancemethod(cspacec.iPolygonSet_CompressVertices, None, iPolygonSet)
iPolygonSet.GetPolygonCount = new.instancemethod(cspacec.iPolygonSet_GetPolygonCount, None, iPolygonSet)
iPolygonSet.GetVertexCount = new.instancemethod(cspacec.iPolygonSet_GetVertexCount, None, iPolygonSet)
iPolygonSet.CreateVertex = new.instancemethod(cspacec.iPolygonSet_CreateVertex, None, iPolygonSet)
iPolygonSet.CreateKey = new.instancemethod(cspacec.iPolygonSet_CreateKey, None, iPolygonSet)

class iSector(iBase,iPolygonSet):
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iSector instance at %s>" % (self.this,)
class iSectorPtr(iSector):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iSector


iSector.CreateBSP = new.instancemethod(cspacec.iSector_CreateBSP, None, iSector)

class iThing(iPolygonSet):
    def __init__(self,this):
        self.this = this

    def __repr__(self):
        return "<C iThing instance at %s>" % (self.this,)
class iThingPtr(iThing):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iThing


iThing.SetPosition = new.instancemethod(cspacec.iThing_SetPosition, None, iThing)
iThing.SetSector = new.instancemethod(cspacec.iThing_SetSector, None, iThing)
iThing.SetTransform = new.instancemethod(cspacec.iThing_SetTransform, None, iThing)

class iPolygon3D(iBase):
    def __init__(self,this):
        self.this = this

    def GetContainer(*args):
        val = apply(cspacec.iPolygon3D_GetContainer,args)
        if val: val = iPolygonSetPtr(val) 
        return val
    def GetTexture(*args):
        val = apply(cspacec.iPolygon3D_GetTexture,args)
        if val: val = iPolygonTexturePtr(val) 
        return val
    def GetTextureHandle(*args):
        val = apply(cspacec.iPolygon3D_GetTextureHandle,args)
        if val: val = iTextureHandlePtr(val) 
        return val
    def GetVertex(*args):
        val = apply(cspacec.iPolygon3D_GetVertex,args)
        if val: val = csVector3Ptr(val) 
        return val
    def GetVertexW(*args):
        val = apply(cspacec.iPolygon3D_GetVertexW,args)
        if val: val = csVector3Ptr(val) 
        return val
    def GetVertexC(*args):
        val = apply(cspacec.iPolygon3D_GetVertexC,args)
        if val: val = csVector3Ptr(val) 
        return val
    def __repr__(self):
        return "<C iPolygon3D instance at %s>" % (self.this,)
class iPolygon3DPtr(iPolygon3D):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iPolygon3D


iPolygon3D.GetName = new.instancemethod(cspacec.iPolygon3D_GetName, None, iPolygon3D)
iPolygon3D.SetName = new.instancemethod(cspacec.iPolygon3D_SetName, None, iPolygon3D)
iPolygon3D.GetLightMap = new.instancemethod(cspacec.iPolygon3D_GetLightMap, None, iPolygon3D)
iPolygon3D.GetVertexCount = new.instancemethod(cspacec.iPolygon3D_GetVertexCount, None, iPolygon3D)
iPolygon3D.CreateVertexByIndex = new.instancemethod(cspacec.iPolygon3D_CreateVertexByIndex, None, iPolygon3D)
iPolygon3D.CreateVertex = new.instancemethod(cspacec.iPolygon3D_CreateVertex, None, iPolygon3D)
iPolygon3D.GetAlpha = new.instancemethod(cspacec.iPolygon3D_GetAlpha, None, iPolygon3D)
iPolygon3D.SetAlpha = new.instancemethod(cspacec.iPolygon3D_SetAlpha, None, iPolygon3D)
iPolygon3D.CreatePlane = new.instancemethod(cspacec.iPolygon3D_CreatePlane, None, iPolygon3D)
iPolygon3D.SetPlane = new.instancemethod(cspacec.iPolygon3D_SetPlane, None, iPolygon3D)

class iImage(iBase):
    def __init__(self,this):
        self.this = this

    def MipMap(*args):
        val = apply(cspacec.iImage_MipMap,args)
        if val: val = iImagePtr(val) 
        return val
    def GetPalette(*args):
        val = apply(cspacec.iImage_GetPalette,args)
        if val: val = csRGBpixelPtr(val) 
        return val
    def Clone(*args):
        val = apply(cspacec.iImage_Clone,args)
        if val: val = iImagePtr(val) 
        return val
    def Crop(*args):
        val = apply(cspacec.iImage_Crop,args)
        if val: val = iImagePtr(val) 
        return val
    def __repr__(self):
        return "<C iImage instance at %s>" % (self.this,)
class iImagePtr(iImage):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iImage


iImage.GetImageData = new.instancemethod(cspacec.iImage_GetImageData, None, iImage)
iImage.GetWidth = new.instancemethod(cspacec.iImage_GetWidth, None, iImage)
iImage.GetHeight = new.instancemethod(cspacec.iImage_GetHeight, None, iImage)
iImage.GetSize = new.instancemethod(cspacec.iImage_GetSize, None, iImage)
iImage.Rescale = new.instancemethod(cspacec.iImage_Rescale, None, iImage)
iImage.SetName = new.instancemethod(cspacec.iImage_SetName, None, iImage)
iImage.GetName = new.instancemethod(cspacec.iImage_GetName, None, iImage)
iImage.GetFormat = new.instancemethod(cspacec.iImage_GetFormat, None, iImage)
iImage.GetAlpha = new.instancemethod(cspacec.iImage_GetAlpha, None, iImage)
iImage.SetFormat = new.instancemethod(cspacec.iImage_SetFormat, None, iImage)

class iTextureManager(iBase):
    def __init__(self,this):
        self.this = this

    def RegisterTexture(*args):
        val = apply(cspacec.iTextureManager_RegisterTexture,args)
        if val: val = iTextureHandlePtr(val) 
        return val
    def __repr__(self):
        return "<C iTextureManager instance at %s>" % (self.this,)
class iTextureManagerPtr(iTextureManager):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iTextureManager


iTextureManager.UnregisterTexture = new.instancemethod(cspacec.iTextureManager_UnregisterTexture, None, iTextureManager)
iTextureManager.PrepareTexture = new.instancemethod(cspacec.iTextureManager_PrepareTexture, None, iTextureManager)
iTextureManager.PrepareTextures = new.instancemethod(cspacec.iTextureManager_PrepareTextures, None, iTextureManager)
iTextureManager.FreeImages = new.instancemethod(cspacec.iTextureManager_FreeImages, None, iTextureManager)
iTextureManager.ResetPalette = new.instancemethod(cspacec.iTextureManager_ResetPalette, None, iTextureManager)
iTextureManager.ReserveColor = new.instancemethod(cspacec.iTextureManager_ReserveColor, None, iTextureManager)
iTextureManager.FindRGB = new.instancemethod(cspacec.iTextureManager_FindRGB, None, iTextureManager)
iTextureManager.SetPalette = new.instancemethod(cspacec.iTextureManager_SetPalette, None, iTextureManager)
iTextureManager.SetVerbose = new.instancemethod(cspacec.iTextureManager_SetVerbose, None, iTextureManager)
iTextureManager.GetTextureFormat = new.instancemethod(cspacec.iTextureManager_GetTextureFormat, None, iTextureManager)

class iPolygonTexture(iBase):
    def __init__(self,this):
        self.this = this

    def GetTextureHandle(*args):
        val = apply(cspacec.iPolygonTexture_GetTextureHandle,args)
        if val: val = iTextureHandlePtr(val) 
        return val
    def GetPolygon(*args):
        val = apply(cspacec.iPolygonTexture_GetPolygon,args)
        if val: val = iPolygon3DPtr(val) 
        return val
    def __repr__(self):
        return "<C iPolygonTexture instance at %s>" % (self.this,)
class iPolygonTexturePtr(iPolygonTexture):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iPolygonTexture


iPolygonTexture.GetFDU = new.instancemethod(cspacec.iPolygonTexture_GetFDU, None, iPolygonTexture)
iPolygonTexture.GetFDV = new.instancemethod(cspacec.iPolygonTexture_GetFDV, None, iPolygonTexture)
iPolygonTexture.GetWidth = new.instancemethod(cspacec.iPolygonTexture_GetWidth, None, iPolygonTexture)
iPolygonTexture.GetHeight = new.instancemethod(cspacec.iPolygonTexture_GetHeight, None, iPolygonTexture)
iPolygonTexture.GetShiftU = new.instancemethod(cspacec.iPolygonTexture_GetShiftU, None, iPolygonTexture)
iPolygonTexture.GetIMinU = new.instancemethod(cspacec.iPolygonTexture_GetIMinU, None, iPolygonTexture)
iPolygonTexture.GetIMinV = new.instancemethod(cspacec.iPolygonTexture_GetIMinV, None, iPolygonTexture)
iPolygonTexture.GetTextureBox = new.instancemethod(cspacec.iPolygonTexture_GetTextureBox, None, iPolygonTexture)
iPolygonTexture.GetOriginalWidth = new.instancemethod(cspacec.iPolygonTexture_GetOriginalWidth, None, iPolygonTexture)
iPolygonTexture.DynamicLightsDirty = new.instancemethod(cspacec.iPolygonTexture_DynamicLightsDirty, None, iPolygonTexture)
iPolygonTexture.RecalculateDynamicLights = new.instancemethod(cspacec.iPolygonTexture_RecalculateDynamicLights, None, iPolygonTexture)
iPolygonTexture.GetLightMap = new.instancemethod(cspacec.iPolygonTexture_GetLightMap, None, iPolygonTexture)
iPolygonTexture.GetLightCellSize = new.instancemethod(cspacec.iPolygonTexture_GetLightCellSize, None, iPolygonTexture)
iPolygonTexture.GetLightCellShift = new.instancemethod(cspacec.iPolygonTexture_GetLightCellShift, None, iPolygonTexture)
iPolygonTexture.GetCacheData = new.instancemethod(cspacec.iPolygonTexture_GetCacheData, None, iPolygonTexture)
iPolygonTexture.SetCacheData = new.instancemethod(cspacec.iPolygonTexture_SetCacheData, None, iPolygonTexture)

class iWorld(iPlugIn):
    def __init__(self,this):
        self.this = this

    def CreateSector(*args):
        val = apply(cspacec.iWorld_CreateSector,args)
        if val: val = iSectorPtr(val) 
        return val
    def CreateThing(*args):
        val = apply(cspacec.iWorld_CreateThing,args)
        if val: val = iThingPtr(val) 
        return val
    def __repr__(self):
        return "<C iWorld instance at %s>" % (self.this,)
class iWorldPtr(iWorld):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iWorld


iWorld.GetTextureFormat = new.instancemethod(cspacec.iWorld_GetTextureFormat, None, iWorld)
iWorld.SelectLibrary = new.instancemethod(cspacec.iWorld_SelectLibrary, None, iWorld)
iWorld.DeleteLibrary = new.instancemethod(cspacec.iWorld_DeleteLibrary, None, iWorld)
iWorld.DeleteAll = new.instancemethod(cspacec.iWorld_DeleteAll, None, iWorld)
iWorld.CreateTexture = new.instancemethod(cspacec.iWorld_CreateTexture, None, iWorld)
iWorld.CreateCamera = new.instancemethod(cspacec.iWorld_CreateCamera, None, iWorld)
iWorld.CreateKey = new.instancemethod(cspacec.iWorld_CreateKey, None, iWorld)
iWorld.CreatePlane = new.instancemethod(cspacec.iWorld_CreatePlane, None, iWorld)

class iSystem(iBase):
    def __init__(self,this):
        self.this = this

    def GetSCF(*args):
        val = apply(cspacec.iSystem_GetSCF,args)
        if val: val = iSCFPtr(val) 
        return val
    def Query_iWorld(*args):
        val = apply(cspacec.iSystem_Query_iWorld,args)
        if val: val = iWorldPtr(val) 
        return val
    def Query_iGraphics3D(*args):
        val = apply(cspacec.iSystem_Query_iGraphics3D,args)
        if val: val = iGraphics3DPtr(val) 
        return val
    def __repr__(self):
        return "<C iSystem instance at %s>" % (self.this,)
class iSystemPtr(iSystem):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
        self.__class__ = iSystem





#-------------- FUNCTION WRAPPERS ------------------

ptrcast = cspacec.ptrcast

ptrvalue = cspacec.ptrvalue

ptrset = cspacec.ptrset

ptrcreate = cspacec.ptrcreate

ptrfree = cspacec.ptrfree

ptradd = cspacec.ptradd

ptrmap = cspacec.ptrmap

MakeVersion = cspacec.MakeVersion

def GetSystem(*args, **kwargs):
    val = apply(cspacec.GetSystem,args,kwargs)
    if val: val = iSystemPtr(val)
    return val

GetMyPtr = cspacec.GetMyPtr

def new_csSector(*args, **kwargs):
    val = apply(cspacec.new_csSector,args,kwargs)
    if val: val = iSectorPtr(val)
    return val

def new_csCamera(*args, **kwargs):
    val = apply(cspacec.new_csCamera,args,kwargs)
    if val: val = iCameraPtr(val)
    return val

def new_csPolygonSet(*args, **kwargs):
    val = apply(cspacec.new_csPolygonSet,args,kwargs)
    if val: val = iPolygonSetPtr(val)
    return val

def new_csPolygonTexture(*args, **kwargs):
    val = apply(cspacec.new_csPolygonTexture,args,kwargs)
    if val: val = iPolygonTexturePtr(val)
    return val



#-------------- VARIABLE WRAPPERS ------------------

