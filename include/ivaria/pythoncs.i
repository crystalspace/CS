/*
    Copyright (C) 2002-2003 by Rene Jager (renej.frog@yucom.be)

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
	Python specific stuff for SWIG interface.

	See include/ivaria/cspace.i
*/

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

	CSMASK_Nothing = (1 << csevNothing)
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

	def csInitializer_RegisterDocumentSystem (reg):
		"""Register default iDocumentSystem"""
		res = csInitializer._RegisterDocumentSystem (reg)
		return res

	csInitializer.RequestPlugins = staticmethod(csInitializer_RequestPlugins)
	csInitializer.RegisterDocumentSystem = staticmethod(csInitializer_RegisterDocumentSystem)

	csInitializer.SetupEventHandler = staticmethod(csInitializer_SetupEventHandler)

}


%}

%pythoncode %{

	csReport = csReporterHelper.Report

	def CS_QUERY_REGISTRY_TAG (reg, tag):
		return _CS_QUERY_REGISTRY_TAG(reg, tag)

	def CS_QUERY_REGISTRY (reg, intf):
		return intf._CS_QUERY_REGISTRY(reg)

	def CS_QUERY_REGISTRY_TAG_INTERFACE (reg, tag, intf):
		return intf._CS_QUERY_REGISTRY_TAG_INTERFACE(reg, tag)

	def SCF_QUERY_INTERFACE (obj, intf):
		return intf._SCF_QUERY_INTERFACE(obj)

	def SCF_QUERY_INTERFACE_SAFE (obj, intf):
		return intf._SCF_QUERY_INTERFACE_SAFE(obj)

	def CS_GET_CHILD_OBJECT (object, intf):
		return intf._CS_GET_CHILD_OBJECT(object)

	def CS_GET_NAMED_CHILD_OBJECT (object, intf, name):
		return intf._CS_GET_NAMED_CHILD_OBJECT(object, name)

	def CS_GET_FIRST_NAMED_CHILD_OBJECT (object, intf, name):
		return intf._CS_GET_FIRST_NAMED_CHILD_OBJECT(object, name)

	def CS_QUERY_PLUGIN_CLASS (obj, class_id, intf):
		return intf._CS_QUERY_PLUGIN_CLASS(obj, class_id)

	def CS_LOAD_PLUGIN (obj, class_id, intf):
		return intf._CS_LOAD_PLUGIN(obj, class_id)

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

%}

%extend iCollideSystem
{
	%rename(_GetCollisionPairs) GetCollisionPairs;

	%pythoncode %{
		def GetCollisionPairs (self):
			num = self.GetCollisionPairCount()
			pairs = []
			for i in range(num):
				pairs.append(self.GetCollisionPairByIndex(i))
			return pairs
	%}

}

%define VECTOR_PYTHON_OBJECT_FUNCTIONS(V)
	V __rmul__ (float f) const
		{ return f * *self; }
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
}

%extend csMatrix3
{
	csMatrix3 __rmul__ (float f)
		{ return f * *self; }
}

%extend csTransform
{
	csVector3 __rmul__ (const csVector3 & v) const
		{ return v * *self; } 
	csPlane3 __rmul__ (const csPlane3 & p) const
		{ return p * *self; } 
	csSphere __rmul__ (const csSphere & s) const
		{ return s * *self; } 
}

%extend csColor
{
	csColor __rmul__ (float f) const
		{ return f * *self; }
}

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

