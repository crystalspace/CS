/* 
 * Some functionality for wrapping shadervar values in python objects.
 * for file csgfx/shadervar.h 
 */

%define SHADERVAR_GET(obj,data,datatype)
	{
	    datatype *res = new datatype ();
	    data->GetValue(* res);
	    obj = SWIG_NewPointerObj((void*)(res), SWIGTYPE_p_ ## datatype, 1);
	}
%enddef
%define SHADERVAR_GETREF(obj,data,datatype)
	{
	    datatype *res;
	    data->GetValue(res);
	    obj = SWIG_NewPointerObj((void*)(res), SWIGTYPE_p_ ## datatype, 0);
	}
%enddef
%define SHADERVAR_RETURN(data)
		PyObject *obj = Py_None;
		if (data)
		{
		    switch(data->GetType())
		    {
			case csShaderVariable::FLOAT:
				float fval;
				data->GetValue(fval);
				obj = PyFloat_FromDouble(fval);
				break;
			case csShaderVariable::VECTOR2:
				SHADERVAR_GET(obj,data,csVector2)
				break;
			case csShaderVariable::VECTOR3:
				SHADERVAR_GET(obj,data,csVector3)
				break;
			case csShaderVariable::INT:
				int ival;
				data->GetValue(ival);
				obj = SWIG_From_long((long)ival);
				break;
			case csShaderVariable::VECTOR4:
				SHADERVAR_GET(obj,data,csVector4)
				break;
			case csShaderVariable::MATRIX:
				SHADERVAR_GET(obj,data,csMatrix3)
				break;
			case csShaderVariable::TRANSFORM:
				SHADERVAR_GET(obj,data,csReversibleTransform)
				break;
			case csShaderVariable::TEXTURE:
				SHADERVAR_GETREF(obj,data,iTextureHandle)
				break;
			case csShaderVariable::RENDERBUFFER:
				SHADERVAR_GETREF(obj,data,iRenderBuffer)
				break;
			case csShaderVariable::ARRAY:
				//SHADERVAR_GET(obj,data,iArray)
			default:
				break;
		    }
		}
%enddef
%extend csShaderVariable {
	PyObject *GetValue() {
		SHADERVAR_RETURN(self);
		return obj;
	}
}

