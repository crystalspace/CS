# This file was created automatically by SWIG.
import cspacec
class Vector3Ptr :
    def __init__(self,this):
        self.this = this
        self.thisown = 0
    def __str__(self):
        val = cspacec.Vector3___str__(self.this)
        return val
    def __setattr__(self,name,value):
        if name == "x" :
            cspacec.Vector3_x_set(self.this,value)
            return
        if name == "y" :
            cspacec.Vector3_y_set(self.this,value)
            return
        if name == "z" :
            cspacec.Vector3_z_set(self.this,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if name == "x" : 
            return cspacec.Vector3_x_get(self.this)
        if name == "y" : 
            return cspacec.Vector3_y_get(self.this)
        if name == "z" : 
            return cspacec.Vector3_z_get(self.this)
        raise AttributeError,name
    def __repr__(self):
        return "<C Vector3 instance>"
class Vector3(Vector3Ptr):
    def __init__(self,arg0,arg1,arg2) :
        self.this = cspacec.new_Vector3(arg0,arg1,arg2)
        self.thisown = 1



def Vector3n() :
    val = Vector3Ptr(cspacec.new_Vector3n())
    val.thisown = 1
    return val

def Vector3s(arg0) :
    val = Vector3Ptr(cspacec.new_Vector3s(arg0))
    val.thisown = 1
    return val




#-------------- FUNCTION WRAPPERS ------------------



#-------------- VARIABLE WRAPPERS ------------------

