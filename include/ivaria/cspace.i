/*
    Copyright (C) 2002 by Rene Jager (renej.frog@yucom.be)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
	SWIG interface for Crystal Space

	Used for the new "cspython" plugin based on python 2.2 including
	almost complete access to CS.

	Additionally, provides CS as a python 2.2 module name "cspace".
	To use this, make sure cspace.py and _cspace.so in directory
	$CRYSTAL/scripts/python can be found by python. And don't forget
	to set the CRYSTAL environment variable.
	See $CRYSTAL/scripts/python/testpymod.py for an example of usage.
*/

%module cspace
%{

#include "css.h"

#include "imap/saver.h"
#include "ivaria/reporter.h"
#include "ivaria/dynamics.h"
#include "ivaria/engseq.h"
//OBSOLETE#include "ivideo/rndbuf.h"
//OBSOLETE#include "ivideo/render3d.h"
#include "ivideo/shader/shader.h"
#include "iutil/cache.h"

%}

%include "cstypes.h"

%rename(assign) operator=;

typedef unsigned scfInterfaceID;

%ignore G3DTriangleMesh::vertex_mode;
%ignore G3DTriangleMesh::VM_WORLDSPACE;
%ignore G3DTriangleMesh::VM_VIEWSPACE;
%ignore G3DPolygonMesh::vertex_mode;
%ignore G3DPolygonMesh::VM_WORLDSPACE;
%ignore G3DPolygonMesh::VM_VIEWSPACE;
%ignore csInitializer::RequestPlugins;
%ignore csInitializer::SetupEventHandler;

%include "csutil/ref.h"

%define HANDLE_INTERFACE(T)

%nodefault T;

%typemap(out) csPtr<T>
{
	$result = SWIG_NewPointerObj(
		(void *) new csRef<T>($1), $descriptor(csRef<T> *), 1
	);
}

%template(csRef_ ## T) csRef<T>;
%template(csPtr_ ## T) csPtr<T>;

%extend csPtr<T>
{
	csRef<T> ref () const
	{
		return csRef<T>(*self);
	}
}

%extend csRef<T>
{
	bool operator == (const csRef<T> & r) const
		{ return *self == r; }
	bool operator != (const csRef<T> & r) const
		{ return *self != r; }
	bool operator == (T * p) const
		{ return *self == p; }
	bool operator != (T * p) const
		{ return *self != p; }
}

%inline %{

	csRef<T> _CS_QUERY_REGISTRY_ ## T (iObjectRegistry * reg)
	{
		return csRef<T>(CS_QUERY_REGISTRY(reg, T));
	}

	csRef<T> _CS_QUERY_REGISTRY_TAG_INTERFACE_ ## T (
		iObjectRegistry * reg, const char * tag)
	{
		return csRef<T>(CS_QUERY_REGISTRY_TAG_INTERFACE(reg, tag, T));
	}

	csRef<T> _SCF_QUERY_INTERFACE_ ## T (iBase * obj)
	{
		return csRef<T>(SCF_QUERY_INTERFACE(obj, T));
	}

	csRef<T> _SCF_QUERY_INTERFACE_SAFE_ ## T (iBase * obj)
	{
		return csRef<T>(SCF_QUERY_INTERFACE_SAFE(obj, T));
	}

	csRef<T> _CS_QUERY_PLUGIN_CLASS_ ## T (iPluginManager * obj,
		const char * class_id)
	{
		return csRef<T>(CS_QUERY_PLUGIN_CLASS(obj, class_id, T));
	}

	csRef<T> _CS_LOAD_PLUGIN_ ## T (iPluginManager * obj,
		const char * class_id)
	{
		return csRef<T>(CS_LOAD_PLUGIN(obj, class_id, T));
	}

	csRef<T> _CS_GET_CHILD_OBJECT_ ## T (iObject * object)
	{
		return csRef<T>(CS_GET_CHILD_OBJECT(object, T));
	}

	csRef<T> _CS_GET_NAMED_CHILD_OBJECT_ ## T (iObject * object,
		const char *name)
	{
		return csRef<T>(CS_GET_NAMED_CHILD_OBJECT(object, T, name));
	}

	csRef<T> _CS_GET_FIRST_NAMED_CHILD_OBJECT_ ## T (iObject * object,
		const char * name)
	{
		return csRef<T>(CS_GET_FIRST_NAMED_CHILD_OBJECT(object, T, name));
	}

%}

%enddef

HANDLE_INTERFACE(iBase)
HANDLE_INTERFACE(iBinaryLoaderPlugin)
HANDLE_INTERFACE(iCamera)
HANDLE_INTERFACE(iCameraPosition)
HANDLE_INTERFACE(iCacheManager)
HANDLE_INTERFACE(iCollider)
HANDLE_INTERFACE(iCollideSystem)
HANDLE_INTERFACE(iComponent)
HANDLE_INTERFACE(iConfigFile)
HANDLE_INTERFACE(iDebugHelper)
HANDLE_INTERFACE(iDynamics)
HANDLE_INTERFACE(iDynamicSystem)
HANDLE_INTERFACE(iEngine)
HANDLE_INTERFACE(iEvent)
HANDLE_INTERFACE(iEventHandler)
HANDLE_INTERFACE(iEventQueue)
HANDLE_INTERFACE(iFile)
HANDLE_INTERFACE(iFont)
HANDLE_INTERFACE(iFontServer)
HANDLE_INTERFACE(iGraphics3D)
HANDLE_INTERFACE(iGraphics2D)
HANDLE_INTERFACE(iImage)
HANDLE_INTERFACE(iImageIO)
HANDLE_INTERFACE(iKeyboardDriver)
HANDLE_INTERFACE(iLightList)
HANDLE_INTERFACE(iLoader)
HANDLE_INTERFACE(iLoaderPlugin)
HANDLE_INTERFACE(iMaterial)
HANDLE_INTERFACE(iMeshFactoryWrapper)
HANDLE_INTERFACE(iMeshObject)
HANDLE_INTERFACE(iMeshObjectFactory)
HANDLE_INTERFACE(iMeshObjectType)
HANDLE_INTERFACE(iMeshWrapper)
HANDLE_INTERFACE(iModelConverter)
HANDLE_INTERFACE(iMovable)
HANDLE_INTERFACE(iMovableListener)
HANDLE_INTERFACE(iObject)
HANDLE_INTERFACE(iObjectRegistry)
HANDLE_INTERFACE(iPluginManager)
HANDLE_INTERFACE(iPolygon3D)
HANDLE_INTERFACE(iPolygonMesh)
HANDLE_INTERFACE(iPolygonTexture)
//OBSOLETEHANDLE_INTERFACE(iRenderBufferManager)
HANDLE_INTERFACE(iSCF)
HANDLE_INTERFACE(iScript)
HANDLE_INTERFACE(iSector)
HANDLE_INTERFACE(iSectorList)
HANDLE_INTERFACE(iSoundHandle)
HANDLE_INTERFACE(iSoundLoader)
HANDLE_INTERFACE(iSoundRender)
HANDLE_INTERFACE(iSprite3DState)
HANDLE_INTERFACE(iStatLight)
HANDLE_INTERFACE(iTextureHandle)
HANDLE_INTERFACE(iTextureManager)
HANDLE_INTERFACE(iTextureWrapper)
HANDLE_INTERFACE(iThingState)
HANDLE_INTERFACE(iVFS)
HANDLE_INTERFACE(iView)
HANDLE_INTERFACE(iVirtualClock)
HANDLE_INTERFACE(iVisibilityCuller)

#ifdef SWIGPYTHON

%define WRAP_PTR_RETURNING_METHOD(intf, method)

	%rename(_csPtr_returning_ ## method) intf::method;

	%extend intf
	{
	%pythoncode %{
		def method (self, *args):
			return self._csPtr_returning_ ## method (*args)
	%}
	}

	%extend csRef<intf>
	{
	%pythoncode %{
		def method (self, *args):
			return self.__deref__()._csPtr_returning_ ## method (*args)
	%}
	}

%enddef

#else // SWIGPYTHON

%define WRAP_PTR_RETURNING_METHOD(int, method)
	// TODO
%enddef

#endif // SWIGPYTHON

// iengine/engine.h
WRAP_PTR_RETURNING_METHOD(iEngine, CreateSectorWallsMesh)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateThingMesh)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateLight)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateBaseMaterial)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateCamera)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateLight)
WRAP_PTR_RETURNING_METHOD(iEngine, GetLightIterator)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateDynLight)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateMeshFactory)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateLoaderContext)
WRAP_PTR_RETURNING_METHOD(iEngine, LoadMeshFactory)
WRAP_PTR_RETURNING_METHOD(iEngine, CreateMeshWrapper)
WRAP_PTR_RETURNING_METHOD(iEngine, LoadMeshWrapper)
WRAP_PTR_RETURNING_METHOD(iEngine, GetNearbySectors)
WRAP_PTR_RETURNING_METHOD(iEngine, GetNearbyObjects)
WRAP_PTR_RETURNING_METHOD(iEngine, GetVisibleObjects)

// iengine/engine.h
WRAP_PTR_RETURNING_METHOD(iPolygonMesh, CreateLowerDetailPolygonMesh)

// iengine/viscull.h
WRAP_PTR_RETURNING_METHOD(iVisibilityCuller, VisTest)

// igraphics/image.h
WRAP_PTR_RETURNING_METHOD(iImage, MipMap)
WRAP_PTR_RETURNING_METHOD(iImage, Clone)
WRAP_PTR_RETURNING_METHOD(iImage, Crop)
WRAP_PTR_RETURNING_METHOD(iImage, Sharpen)

// igraphics/imageio.h
WRAP_PTR_RETURNING_METHOD(iImageIO, Load)
WRAP_PTR_RETURNING_METHOD(iImageIO, Save)

// imap/loader.h
WRAP_PTR_RETURNING_METHOD(iLoader, LoadImage)
WRAP_PTR_RETURNING_METHOD(iLoader, LoadTexture)
WRAP_PTR_RETURNING_METHOD(iLoader, LoadSoundData)
WRAP_PTR_RETURNING_METHOD(iLoader, LoadSound)
WRAP_PTR_RETURNING_METHOD(iLoader, LoadMeshObjectFactory)
WRAP_PTR_RETURNING_METHOD(iLoader, LoadMeshObject)

// imap/reader.h
WRAP_PTR_RETURNING_METHOD(iLoaderPlugin, Parse)
WRAP_PTR_RETURNING_METHOD(iBinaryLoaderPlugin, Parse)

// imesh/mdlconv.h
WRAP_PTR_RETURNING_METHOD(iModelConverter, Load)
WRAP_PTR_RETURNING_METHOD(iModelConverter, Save)

// imesh/object.h
WRAP_PTR_RETURNING_METHOD(iMeshObjectFactory, NewInstance)
WRAP_PTR_RETURNING_METHOD(iMeshObjectType, NewFactory)

// isound/handle.h
WRAP_PTR_RETURNING_METHOD(iSoundHandle, Play)
WRAP_PTR_RETURNING_METHOD(iSoundHandle, CreateSource)

// isound/loader.h
WRAP_PTR_RETURNING_METHOD(iSoundLoader, LoadSound)

// isound/renderer.h
WRAP_PTR_RETURNING_METHOD(iSoundRender, RegisterSound)

// iutil/cache.h
WRAP_PTR_RETURNING_METHOD(iCacheManager, ReadCache)

// iutil/cfgfile.h
WRAP_PTR_RETURNING_METHOD(iConfigFile, Enumerate)

// iutil/dbghelp.h
WRAP_PTR_RETURNING_METHOD(iDebugHelper, UnitTest)
WRAP_PTR_RETURNING_METHOD(iDebugHelper, StateTest)
WRAP_PTR_RETURNING_METHOD(iDebugHelper, Dump)

// iutil/eventq.h
WRAP_PTR_RETURNING_METHOD(iEventQueue, CreateEventOutlet)
WRAP_PTR_RETURNING_METHOD(iEventQueue, Get)

// iutil/object.h
WRAP_PTR_RETURNING_METHOD(iObject, GetIterator)

// iutil/vfs.h
WRAP_PTR_RETURNING_METHOD(iFile, GetAllData)
WRAP_PTR_RETURNING_METHOD(iVFS, ExpandPath)
WRAP_PTR_RETURNING_METHOD(iVFS, FindFiles)
WRAP_PTR_RETURNING_METHOD(iVFS, Open)
WRAP_PTR_RETURNING_METHOD(iVFS, ReadFile)
WRAP_PTR_RETURNING_METHOD(iVFS, GetRealPath)

// ivaria/collider.h
WRAP_PTR_RETURNING_METHOD(iCollider, CreateCollider)
WRAP_PTR_RETURNING_METHOD(iCollider, CreateSphereCollider)
WRAP_PTR_RETURNING_METHOD(iCollider, CreateBoxCollider)

// ivaria/dynamics.h
WRAP_PTR_RETURNING_METHOD(iDynamics, CreateSystem)
WRAP_PTR_RETURNING_METHOD(iDynamicSystem, CreateBody)
WRAP_PTR_RETURNING_METHOD(iDynamicSystem, CreateGroup)
WRAP_PTR_RETURNING_METHOD(iDynamicSystem, CreateJoint)

// ivaria/engseq.h
WRAP_PTR_RETURNING_METHOD(iEngineSequenceParameters, CreateParameterESM)
WRAP_PTR_RETURNING_METHOD(iSequenceWrapper, CreateParameterBlock)
WRAP_PTR_RETURNING_METHOD(iEngineSequenceManager, CreateParameterESM)
WRAP_PTR_RETURNING_METHOD(iEngineSequenceManager, CreateTrigger)
WRAP_PTR_RETURNING_METHOD(iEngineSequenceManager, CreateSequence)

// ivideo/fontserv.h
WRAP_PTR_RETURNING_METHOD(iFontServer, LoadFont)

// ivideo/graph2d.h
WRAP_PTR_RETURNING_METHOD(iGraphics2D, ScreenShot)
WRAP_PTR_RETURNING_METHOD(iGraphics2D, CreateOffScreenCanvas)

// ivideo/rndbuf.h
//OBSOLETEWRAP_PTR_RETURNING_METHOD(iRenderBufferManager, GetBuffer)

// ivideo/txtmgr.h
WRAP_PTR_RETURNING_METHOD(iTextureManager, RegisterTexture)
WRAP_PTR_RETURNING_METHOD(iTextureManager, RegisterMaterial)

// ivideo/vbufmgr.h
WRAP_PTR_RETURNING_METHOD(iVertexBufferManager, CreateBuffer)

// ivideo/shader/shader.h
WRAP_PTR_RETURNING_METHOD(iShaderManager, CreateShader)
WRAP_PTR_RETURNING_METHOD(iShaderManager, CreateVariable)
WRAP_PTR_RETURNING_METHOD(iShaderManager, CreateShaderProgramFromFile)
WRAP_PTR_RETURNING_METHOD(iShaderManager, CreateShaderProgramFromString)
WRAP_PTR_RETURNING_METHOD(iShaderRenderInterface, CreateShaderProgram)
WRAP_PTR_RETURNING_METHOD(iShader, CreateTechnique)
WRAP_PTR_RETURNING_METHOD(iShaderTechnique, CreatePass)
WRAP_PTR_RETURNING_METHOD(iShaderProgramPlugin, CreateShaderProgram)

#define CS_STRUCT_ALIGN_4BYTE_BEGIN
#define CS_STRUCT_ALIGN_4BYTE_END

#define CS_GNUC_PRINTF(format_idx, arg_idx)
#define CS_GNUC_SCANF(format_idx, arg_idx)

#ifdef SWIGPYTHON

%typemap(in) (int argc, char const* const argv[])
{
	if (PyList_Check($input)) {
		int i;
		$1 = PyList_Size($input);
		$2 = (char **) malloc(($1+1)*sizeof(char *));
		for (i = 0; i < $1; i++) {
			PyObject *o = PyList_GetItem($input,i);
			if (PyString_Check(o))
				$2[i] = PyString_AsString(PyList_GetItem($input,i));
			else {
				PyErr_SetString(PyExc_TypeError,"list must contain strings");
				free($2);
				return NULL;
			}
		}
		$2[i] = 0;
	} else {
		PyErr_SetString(PyExc_TypeError, "not a list");
		return NULL;
	}
}

%typemap(freearg) (int argc, char const* const argv[])
{
	free((char *) $2);
}

%typemap(in) (const char* description, ...)
{
	$1 = "%s";
	$2 = (void *) PyString_AsString($input);
}

%pythoncode %{

	def csInitializer_RequestPlugins (reg, plugins):
		"""Replacement of C++ version with variable argument list."""
		def _get_tuple (x):
			if callable(x):
				return tuple(x())
			else:
				return tuple(x)
		ok = 1
		for plugName, intName, scfId, version in map(
				lambda x: _get_tuple(x), plugins):
			res = csInitializer._RequestPlugin(
				reg, plugName, intName, scfId, version
			)
			if not res:
				ok = 0
		return ok

%}

#else // SWIGPYTHON

%ignore iReporter::Report;
%ignore csReporterHelper::Report;

#endif // SWIGPYTHON

%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;

%include "csutil/scf.h"
%include "csutil/cscolor.h"
%include "csutil/cmdhelp.h"
%include "csgeom/vector2.h"
%include "csgeom/vector3.h"
%include "csgeom/matrix3.h"
%include "csgeom/transfrm.h"
%include "csgfx/rgbpixel.h"
%include "cssys/sysfunc.h"

%typemap(default) const char * configName {
	$1 = NULL;
}
%include "cstool/initapp.h"
%typemap(default) const char * configName;

%include "igeom/polymesh.h"
%include "iengine/light.h"
%include "iengine/statlght.h"
%include "iengine/sector.h"
%include "iengine/engine.h"
%include "iengine/camera.h"
%include "iengine/campos.h"
%include "iengine/texture.h"
%include "iengine/material.h"
%include "iengine/mesh.h"
%include "iengine/movable.h"
%include "iengine/viscull.h"
%include "imesh/mdlconv.h"
%include "imesh/object.h"
%include "imesh/sprite3d.h"
%include "imesh/thing/thing.h"
%include "imesh/thing/lightmap.h"
%include "imesh/thing/polygon.h"
%include "imap/parser.h"
%include "imap/loader.h"
%include "imap/reader.h"
%include "imap/saver.h"
%include "isound/handle.h"
%include "isound/loader.h"
%include "isound/renderer.h"
%include "iutil/comp.h"
%include "iutil/cache.h"
%include "iutil/vfs.h"
%include "iutil/object.h"
%include "iutil/dbghelp.h"
%include "iutil/objreg.h"
%include "iutil/virtclk.h"
%include "iutil/event.h"
%include "iutil/evdefs.h"
%include "iutil/eventq.h"
%include "iutil/eventh.h"
%include "iutil/plugin.h"
%include "iutil/csinput.h"
%include "iutil/cfgfile.h"
%include "ivideo/graph2d.h"
%include "ivideo/graph3d.h"
//OBSOLETE%include "ivideo/render3d.h"	// After ivideo/graph3d.h, csZBufMode ignored.
%include "ivideo/fontserv.h"
%include "ivideo/halo.h"
%include "ivideo/texture.h"
%include "ivideo/txtmgr.h"
%include "ivideo/vbufmgr.h"
//OBSOLETE%include "ivideo/rndbuf.h"
%include "ivideo/material.h"
%include "ivideo/natwin.h"
%include "ivideo/shader/shader.h"
%include "igraphic/image.h"
%include "igraphic/imageio.h"
%include "ivaria/reporter.h"
%include "ivaria/stdrep.h"
%include "ivaria/view.h"
%include "ivaria/collider.h"
%include "ivaria/dynamics.h"
%include "ivaria/conout.h"
%include "ivaria/script.h"
%include "ivaria/engseq.h"

%include "cstool/csview.h"
%include "cstool/collider.h"
//%include "csutil/csinput.h"

#ifdef SWIGPYTHON

%native(_csInitializer_SetupEventHandler) _py_csInitializer_SetupEventHandler;

%{
	static PyObject * _py_csInitializer_EventHandler = 0;

	bool _csInitializer_EventHandler (iEvent & event)
	{
		if (!_py_csInitializer_EventHandler)
		{
			return false;
		}
		PyObject * event_obj = SWIG_NewPointerObj(
			(void *) &event, SWIG_TypeQuery("iEvent *"), 0
		);
		PyObject * result = PyObject_CallFunction(
			_py_csInitializer_EventHandler, "(O)", event_obj
		);
		Py_DECREF(event_obj);
		if (!result)
		{
			return false;
		}
		bool res = PyInt_AsLong(result);
		Py_DECREF(result);
		return res;
	}

	PyObject * _py_csInitializer_SetupEventHandler (PyObject * self,
		PyObject * args)
	{
		PyObject * reg_obj;
		PyObject * func_obj;
		unsigned int mask;
		if (!PyArg_ParseTuple(args, "OOi", &reg_obj, &func_obj, &mask))
		{
			PyErr_SetString(PyExc_TypeError, "Wrong arguments!");
			return NULL;
		}
		iObjectRegistry * reg = 0;
		swig_type_info * ti = SWIG_TypeQuery("iObjectRegistry *");
		if (SWIG_ConvertPtr(reg_obj, (void **) &reg, ti, SWIG_POINTER_EXCEPTION)
			== -1)
		{
			PyErr_SetString(PyExc_TypeError, "Not a iObjectRegistry object!");
			return NULL;
		}
		if (!reg)
		{
			PyErr_SetString(PyExc_TypeError, "Nil iObjectRegistry!");
			return NULL;
		}
		if (!func_obj || !PyCallable_Check(func_obj))
		{
			PyErr_SetString(PyExc_TypeError, "Need a callable object!");
			return NULL;
		}
		_py_csInitializer_EventHandler = func_obj;
		Py_XINCREF(_py_csInitializer_EventHandler);
		bool res = csInitializer::SetupEventHandler(
			reg, &_csInitializer_EventHandler, mask
		);
		return PyInt_FromLong((long) res);
	}

%}

%pythoncode %{

	def CS_REQUEST_REGISTRY (object_reg, intf_class):
		intf_name = intf_class.__name__
		intf_id = eval(intf_name + '_scfGetID()', locals(), globals())
		intf_version = eval(intf_name + '_VERSION', locals(), globals())
		base_obj = object_reg.Get(intf_name, intf_id, intf_version)
		obj = _CastBasePointerToRef(base_obj, intf_name)
		return obj

	CSMASK_Nothing = int(1 << csevNothing)
	CSMASK_FrameProcess = CSMASK_Nothing
	CSMASK_KeyDown = (1 << csevKeyDown)
	CSMASK_KeyUp = (1 << csevKeyUp)
	CSMASK_MouseMove = (1 << csevMouseMove)
	CSMASK_MouseDown = (1 << csevMouseDown)
	CSMASK_MouseUp = (1 << csevMouseUp)
	CSMASK_MouseClick = (1 << csevMouseClick)
	CSMASK_MouseDoubleClick = (1 << csevMouseDoubleClick)
	CSMASK_JoystickMove = (1 << csevJoystickMove)
	CSMASK_JoystickDown = (1 << csevJoystickDown)
	CSMASK_JoystickUp = (1 << csevJoystickUp)
	CSMASK_Command = (1 << csevCommand)
	CSMASK_Broadcast = (1 << csevBroadcast)
	CSMASK_Network = (1 << csevNetwork)

	CSMASK_Keyboard = (CSMASK_KeyDown | CSMASK_KeyUp)
	CSMASK_Mouse = (CSMASK_MouseMove | CSMASK_MouseDown | CSMASK_MouseUp
		| CSMASK_MouseClick | CSMASK_MouseDoubleClick)
	CSMASK_Joystick = (CSMASK_JoystickMove | CSMASK_JoystickDown
		| CSMASK_JoystickUp)
	CSMASK_Input = (CSMASK_Keyboard | CSMASK_Mouse | CSMASK_Joystick)

	def CS_IS_KEYBOARD_EVENT (e):
		return ((1 << (e).Type) & CSMASK_Keyboard)

	def CS_IS_MOUSE_EVENT (e):
		return ((1 << (e).Type) & CSMASK_Mouse)

	def CS_IS_JOYSTICK_EVENT (e):
		return ((1 << (e).Type) & CSMASK_Joystick)

	def CS_IS_INPUT_EVENT (e):
		return ((1 << (e).Type) & CSMASK_Input)

	def CS_IS_NETWORK_EVENT (e):
		return ((1 << (e).Type) & CSMASK_Network)

%}

%pythoncode %{

	_csInitializer_SetupEventHandler_DefaultMask = \
		CSMASK_Nothing			| \
		CSMASK_Broadcast		| \
		CSMASK_MouseUp			| \
		CSMASK_MouseDown		| \
		CSMASK_MouseMove		| \
		CSMASK_KeyDown			| \
		CSMASK_KeyUp			| \
		CSMASK_MouseClick		| \
		CSMASK_MouseDoubleClick	| \
		CSMASK_JoystickMove		| \
		CSMASK_JoystickDown		| \
		CSMASK_JoystickUp		| \
		0

	def csInitializer_SetupEventHandler (reg, func,
			mask=_csInitializer_SetupEventHandler_DefaultMask):
		"""Replacement of C++ version."""
		return _csInitializer_SetupEventHandler (reg, func, mask)

	csInitializer.RequestPlugins = staticmethod(csInitializer_RequestPlugins)

	csInitializer.SetupEventHandler = staticmethod(csInitializer_SetupEventHandler)

%}

#endif // SWIGPYTHON

%inline %{

	struct _Struct_iEvent_Key
	{
		int Code;
		int Char;
		int Modifiers;
	};

	struct _Struct_iEvent_Mouse
	{
		int x,y;
		int Button;
		int Modifiers;
	};

	struct _Struct_iEvent_Joystick
	{
		int number;
		int x, y;
		int Button;
		int Modifiers;
	};

	struct _Struct_iEvent_Command
	{
		uint Code;
		void *Info;
	};

	struct _Struct_iEvent_Network
	{
		iNetworkSocket2 *From;
		iNetworkPacket *Data;
	};

%}

%{

	_Struct_iEvent_Key * iEvent_Key_get (iEvent * event)
	{
		return (_Struct_iEvent_Key *) (void *) &event->Key;
	}

	_Struct_iEvent_Mouse * iEvent_Mouse_get (iEvent * event)
	{
		return (_Struct_iEvent_Mouse *) (void *) &event->Mouse;
	}

	_Struct_iEvent_Joystick * iEvent_Joystick_get (iEvent * event)
	{
		return (_Struct_iEvent_Joystick *) (void *) &event->Joystick;
	}

	_Struct_iEvent_Command * iEvent_Command_get (iEvent * event)
	{
		return (_Struct_iEvent_Command *) (void *) &event->Command;
	}

	_Struct_iEvent_Network * iEvent_Network_get (iEvent * event)
	{
		return (_Struct_iEvent_Network *) (void *) &event->Network;
	}

%}

%extend iEvent
{
	const _Struct_iEvent_Key Key;
	const _Struct_iEvent_Mouse Mouse;
	const _Struct_iEvent_Joystick Joystick;
	const _Struct_iEvent_Command Command;
	const _Struct_iEvent_Network Network;
}

%inline %{

	csRef<iBase> _CS_QUERY_REGISTRY_TAG (iObjectRegistry * reg,
		const char * tag)
	{
		return CS_QUERY_REGISTRY_TAG(reg, tag);
	}

	csRef<iBase> _CS_LOAD_PLUGIN_ALWAYS (iPluginManager * plugin_mgr,
		const char * class_id)
	{
		return csRef<iBase>(CS_LOAD_PLUGIN_ALWAYS(plugin_mgr, class_id));
	}

%}

#ifdef SWIGPYTHON

%pythoncode %{

	csReport = csReporterHelper.Report

	def CS_QUERY_REGISTRY_TAG (reg, tag):
		return _CS_QUERY_REGISTRY_TAG(reg, tag)

	def CS_QUERY_REGISTRY (reg, intf):
		return eval(
			'_CS_QUERY_REGISTRY_%s(reg)' % intf.__name__, locals(), globals()
		)

	def CS_QUERY_REGISTRY_TAG_INTERFACE (reg, tag, intf):
		return eval(
			'_CS_QUERY_REGISTRY_TAG_INTERFACE_%s(reg, tag)' % intf.__name__,
			locals(), globals()
		)

	def SCF_QUERY_INTERFACE (obj, intf):
		return eval(
			'_SCF_QUERY_INTERFACE_%s(obj)' % intf.__name__, locals(), globals()
		)

	def SCF_QUERY_INTERFACE_SAFE (obj, intf):
		return eval(
			'_SCF_QUERY_INTERFACE_SAFE_%s(obj)' % intf.__name__, locals(),
			globals()
		)

	def CS_QUERY_PLUGIN_CLASS (obj, class_id, intf):
		return eval(
			'_CS_QUERY_PLUGIN_CLASS_%s(obj, class_id)' % intf.__name__,
			locals(), globals()
		)

	def CS_LOAD_PLUGIN (obj, class_id, intf):
		return eval(
			'_CS_LOAD_PLUGIN_%s(obj, class_id)' % intf.__name__,
			locals(), globals()
		)

	def CS_REQUEST_PLUGIN (name, intf):
		return (
			name, intf.__name__, cvar.iSCF_SCF.GetInterfaceID(intf.__name__),
			eval('%s_VERSION' % intf.__name__, locals(), globals())
		)

	def CS_REQUEST_VFS ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.kernel.vfs", iVFS
		)

	def CS_REQUEST_FONTSERVER ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.font.server.default", iFontServer
		)

	def CS_REQUEST_IMAGELOADER ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.graphic.image.io.multiplex", iImageIO
		)

	def CS_REQUEST_NULL3D ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.graphics3d.null", iGraphics3D
		)

	def CS_REQUEST_SOFTWARE3D ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.graphics3d.software", iGraphics3D
		)

	def CS_REQUEST_OPENGL3D ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.graphics3d.opengl", iGraphics3D
		)

	def CS_REQUEST_ENGINE ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.engine.3d", iEngine
		)

	def CS_REQUEST_LEVELLOADER ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.level.loader", iLoader
		)

	def CS_REQUEST_LEVELSAVER ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.level.saver", iSaver
		)

	def CS_REQUEST_REPORTER ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.utilities.reporter", iReporter
		)

	def CS_REQUEST_REPORTERLISTENER ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.utilities.stdrep", iStandardReporterListener
		)

	def CS_REQUEST_CONSOLEOUT ():
		return CS_REQUEST_PLUGIN(
			"crystalspace.console.output.simple", iConsoleOutput
		)

	def CS_GET_CHILD_OBJECT (object, intf):
		return eval(
			'_CS_GET_CHILD_OBJECT_%s(object)' % intf.__name__,
			locals(), globals()
		)

	def CS_GET_NAMED_CHILD_OBJECT (object, intf, name):
		return eval(
			'_CS_GET_NAMED_CHILD_OBJECT_%s(object, name)' % intf.__name__,
			locals(), globals()
		)

	def CS_GET_FIRST_NAMED_CHILD_OBJECT (object, intf, name):
		return eval(
			'_CS_GET_FIRST_NAMED_CHILD_OBJECT_%s(object, name)' % intf.__name__,
			locals(), globals()
		)

%}

%extend iCollideSystem
{
	csCollisionPair * _GetCollisionPairByIndex (int index)
	{
		return self->GetCollisionPairs() + index;
	}

	%rename(_GetCollisionPairs) GetCollisionPairs;

	%pythoncode %{
		def GetCollisionPairs (self):
			num = self.GetCollisionPairCount()
			pairs = []
			for i in range(num):
				pairs.append(self._GetCollisionPairByIndex(i))
			return pairs
	%}

}

#endif // SWIGPYTHON

%extend csInitializer
{
	static bool _RequestPlugin (iObjectRegistry * object_reg,
		const char * plugName, const char * intName, int scfId, int version)
	{
		return csInitializer::RequestPlugins(
			object_reg, plugName, intName, scfId, version, CS_REQUEST_END
		);
	}
}

#ifdef SWIGPYTHON

%define VECTOR_PYTHON_OBJECT_FUNCTIONS(V)
	V __add__ (const V & v) const
		{ return *self + v; }
	V __sub__ (const V & v) const
		{ return *self - v; }
	float __mul__ (const V & v) const
		{ return *self * v; }
	V __mul__ (float f) const
		{ return *self * f; }
	V __rmul__ (float f) const
		{ return f * *self; }
	V __div__ (float f) const
		{ return *self * f; }
	bool __eq__ (const V & v) const
		{ return *self == v; }
	bool __ne__ (const V & v) const
		{ return *self != v; }
	bool __lt__ (float f) const
		{ return *self < f; }
	bool __ge__ (float f) const
		{ return f > *self; }
	float __abs__ () const
		{ return self->Norm(); }
%enddef

%extend csVector2
{
	VECTOR_PYTHON_OBJECT_FUNCTIONS(csVector2)

	float __getitem__ (int i) const
		{ return i ? self->y : self->x; }
	void __setitem__ (int i, float v)
		{ if (i) self->y = v; else self->x = v; }
}

%extend csVector3
{
	VECTOR_PYTHON_OBJECT_FUNCTIONS(csVector3)

	float __getitem__ (int i) const
		{ return self->operator[](i); }
	void __setitem__ (int i, float v)
		{ self->operator[](i) = v; }
	bool __nonzero__ () const
		{ return !self->IsZero(); }
	csVector3 & __imul__ (const csTransform & t)
		{ return *self *= t; }
	csVector3 & __idiv__ (const csReversibleTransform & t)
		{ return *self /= t; }
	csVector3 __div__ (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csPlane3
{
	csPlane3 & __imul__ (const csTransform & t)
		{ return *self *= t; }
	csPlane3 & __idiv__ (const csReversibleTransform & t)
		{ return *self /= t; }
	csPlane3 __div__ (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csSphere
{
	csSphere & __imul__ (const csTransform & t)
		{ return *self *= t; }
	csSphere __div__ (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csMatrix3
{
	csMatrix3 __add__ (const csMatrix3 & m)
		{ return *self + m; }
	csMatrix3 __sub__ (const csMatrix3 & m)
		{ return *self - m; }
	csMatrix3 __mul__ (const csMatrix3 & m)
		{ return *self * m; }
	csVector3 __mul__ (const csVector3 & v)
		{ return *self * v; }
	csMatrix3 __mul__ (float f)
		{ return *self * f; }
	csMatrix3 __rmul__ (float f)
		{ return f * *self; }
	csMatrix3 __div__ (float f)
		{ return *self * f; }
	bool __eq__ (const csMatrix3 & m) const
		{ return *self == m; }
	bool __ne__ (const csMatrix3 & m) const
		{ return *self != m; }
	bool __lt__ (float f) const
		{ return *self < f; }
	csMatrix3 __mul__ (const csTransform & t) const
		{ return *self * t; }
	csMatrix3 & __imul__ (const csTransform & t)
		{ return *self *= t; }
}

%extend csTransform
{
	csVector3 __rmul__ (const csVector3 & v) const
		{ return v * *self; } 
	csVector3 __mul__ (const csVector3 & v) const
		{ return *self * v; } 
	csPlane3 __rmul__ (const csPlane3 & p) const
		{ return p * *self; } 
	csPlane3 __mul__ (const csPlane3 & p) const
		{ return *self * p; } 
	csSphere __rmul__ (const csSphere & s) const
		{ return s * *self; } 
	csSphere __mul__ (const csSphere & s) const
		{ return *self * s; } 
	csMatrix3 __mul__ (const csMatrix3 & m) const
		{ return *self * m; } 
	csTransform __mul__ (const csReversibleTransform & t) const
		{ return *self * t; } 
}

%extend csReversibleTransform
{
	csReversibleTransform & __imul__ (const csReversibleTransform & t)
		{ return *self *= t; }
	csReversibleTransform __mul__ (const csReversibleTransform & t)
		{ return *self * t; }
	csReversibleTransform & __idiv__ (const csReversibleTransform & t)
		{ return *self /= t; }
	csReversibleTransform __div__ (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csColor
{
	csColor __add__ (const csColor & c) const
		{ return *self + c; }
	csColor __sub__ (const csColor & c) const
		{ return *self - c; }
	csColor __mul__ (float f) const
		{ return *self * f; }
	csColor __rmul__ (float f) const
		{ return f * *self; }
}

#endif // SWIGPYTHON

%extend csVector3
{
	csVector3 project (const csVector3 & what) const
		{ return what << *self; }
}

%extend csRGBpixel
{
	csRGBcolor Color () const
		{ return *self; }
}

%extend G3DTriangleMesh
{
	enum { VM_WORLDSPACE, VM_VIEWSPACE };
	int vertex_mode;
}

%extend G3DPolygonMesh
{
	enum { VM_WORLDSPACE, VM_VIEWSPACE };
	int vertex_mode;
}

#ifdef SWIGPYTHON

%pythoncode %{

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

%}

#endif // SWIGPYTHON

template <class T> bool operator == (const csRef<T>& r1, const csRef<T>& r2);
template <class T> bool operator != (const csRef<T>& r1, const csRef<T>& r2);
template <class T> bool operator == (const csRef<T>& r1, T* obj);
template <class T> bool operator != (const csRef<T>& r1, T* obj);
template <class T> bool operator == (T* obj, const csRef<T>& r1);
template <class T> bool operator != (T* obj, const csRef<T>& r1);

