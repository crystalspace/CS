# This file was created automatically by SWIG.
import cspacec
class iBasePtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C iBase instance>"
class iBase(iBasePtr):
    def __init__(self,this):
        self.this = this




class iPlugInPtr(iBasePtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C iPlugIn instance>"
class iPlugIn(iPlugInPtr):
    def __init__(self,this):
        self.this = this




class iGraphics3DPtr(iPlugInPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C iGraphics3D instance>"
class iGraphics3D(iGraphics3DPtr):
    def __init__(self,this):
        self.this = this




class iCameraPtr(iBasePtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C iCamera instance>"
class iCamera(iCameraPtr):
    def __init__(self,this):
        self.this = this




class csVector3Ptr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __str__(self):
        val = cspacec.csVector3___str__(self.this)
        return val
    def __setattr__(self,name,value):
        if name == "x" :
            cspacec.csVector3_x_set(self.this,value)
            return
        if name == "y" :
            cspacec.csVector3_y_set(self.this,value)
            return
        if name == "z" :
            cspacec.csVector3_z_set(self.this,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "x" : 
            return cspacec.csVector3_x_get(self.this)
        if name == "y" : 
            return cspacec.csVector3_y_get(self.this)
        if name == "z" : 
            return cspacec.csVector3_z_get(self.this)
        raise AttributeError,name
    def __repr__(self):
        return "<C csVector3 instance>"
class csVector3(csVector3Ptr):
    def __init__(self,arg0,arg1,arg2) :
        self.this = cspacec.new_csVector3(arg0,arg1,arg2)
        self.thisown = 1




class csBasePtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            cspacec.delete_csBase(self.this)
    def __repr__(self):
        return "<C csBase instance>"
class csBase(csBasePtr):
    def __init__(self,this):
        self.this = this




class csObjectPtr(csBasePtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def SetName(self,arg0):
        val = cspacec.csObject_SetName(self.this,arg0)
        return val
    def __repr__(self):
        return "<C csObject instance>"
class csObject(csObjectPtr):
    def __init__(self,this):
        self.this = this




class csTextureHandlePtr(csObjectPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C csTextureHandle instance>"
class csTextureHandle(csTextureHandlePtr):
    def __init__(self,this):
        self.this = this




class csPolygon3DPtr(csObjectPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def AddVertex(self,arg0,arg1,arg2):
        val = cspacec.csPolygon3D_AddVertex(self.this,arg0,arg1,arg2)
        return val
    def SetTextureSpace(self,arg0,arg1,arg2):
        val = cspacec.csPolygon3D_SetTextureSpace(self.this,arg0.this,arg1.this,arg2)
        return val
    def Vobj(self,arg0):
        val = cspacec.csPolygon3D_Vobj(self.this,arg0)
        val = csVector3Ptr(val)
        return val
    def __repr__(self):
        return "<C csPolygon3D instance>"
class csPolygon3D(csPolygon3DPtr):
    def __init__(self,this):
        self.this = this




class csPolygonSetPtr(csObjectPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def NewPolygon(self,arg0):
        val = cspacec.csPolygonSet_NewPolygon(self.this,arg0.this)
        val = csPolygon3DPtr(val)
        return val
    def __repr__(self):
        return "<C csPolygonSet instance>"
class csPolygonSet(csPolygonSetPtr):
    def __init__(self,this):
        self.this = this




class csSectorPtr(csPolygonSetPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C csSector instance>"
class csSector(csSectorPtr):
    def __init__(self,this):
        self.this = this




class csWorldPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def NewSector(self):
        val = cspacec.csWorld_NewSector(self.this)
        val = csSectorPtr(val)
        return val
    def __setattr__(self,name,value):
        if name == "view" :
            cspacec.csWorld_view_set(self.this,value.this)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "view" : 
            return csViewPtr(cspacec.csWorld_view_get(self.this))
        raise AttributeError,name
    def __repr__(self):
        return "<C csWorld instance>"
class csWorld(csWorldPtr):
    def __init__(self,this):
        self.this = this




class csLoaderPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            cspacec.delete_csLoader(self.this)
    def LoadTexture(self,arg0,arg1,arg2):
        val = cspacec.csLoader_LoadTexture(self.this,arg0.this,arg1,arg2)
        val = csTextureHandlePtr(val)
        return val
    def __repr__(self):
        return "<C csLoader instance>"
class csLoader(csLoaderPtr):
    def __init__(self) :
        self.this = cspacec.new_csLoader()
        self.thisown = 1




class csTransformPtr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C csTransform instance>"
class csTransform(csTransformPtr):
    def __init__(self,this):
        self.this = this




class csReversibleTransformPtr(csTransformPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C csReversibleTransform instance>"
class csReversibleTransform(csReversibleTransformPtr):
    def __init__(self,this):
        self.this = this




class csOrthoTransformPtr(csReversibleTransformPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __repr__(self):
        return "<C csOrthoTransform instance>"
class csOrthoTransform(csOrthoTransformPtr):
    def __init__(self,this):
        self.this = this




class csCameraPtr(csOrthoTransformPtr,iCameraPtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def SetPosition(self,arg0):
        val = cspacec.csCamera_SetPosition(self.this,arg0.this)
        return val
    def __repr__(self):
        return "<C csCamera instance>"
class csCamera(csCameraPtr):
    def __init__(self,this):
        self.this = this




class csViewPtr(csBasePtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __del__(self):
        if self.thisown == 1 :
            cspacec.delete_csView(self.this)
    def SetSector(self,arg0):
        val = cspacec.csView_SetSector(self.this,arg0.this)
        return val
    def GetCamera(self):
        val = cspacec.csView_GetCamera(self.this)
        val = csCameraPtr(val)
        return val
    def SetRectangle(self,arg0,arg1,arg2,arg3):
        val = cspacec.csView_SetRectangle(self.this,arg0,arg1,arg2,arg3)
        return val
    def __repr__(self):
        return "<C csView instance>"
class csView(csViewPtr):
    def __init__(self,arg0,arg1) :
        self.this = cspacec.new_csView(arg0.this,arg1.this)
        self.thisown = 1




class iSystemPtr(iBasePtr):
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def Query_csWorld(self):
        val = cspacec.iSystem_Query_csWorld(self.this)
        val = csWorldPtr(val)
        return val
    def Query_iGraphics3D(self):
        val = cspacec.iSystem_Query_iGraphics3D(self.this)
        val = iGraphics3DPtr(val)
        return val
    def __repr__(self):
        return "<C iSystem instance>"
class iSystem(iSystemPtr):
    def __init__(self,this):
        self.this = this






#-------------- FUNCTION WRAPPERS ------------------

def GetSystem():
    val = cspacec.GetSystem()
    val = iSystemPtr(val)
    return val

GetFrameHeight = cspacec.GetFrameHeight

GetFrameWidth = cspacec.GetFrameWidth



#-------------- VARIABLE WRAPPERS ------------------

