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
	SWIG interface for Crystal Space

	Used for the new "cspython" plugin based on python 2.2 including
	almost complete access to CS.

	Additionally, provides CS as a python 2.2 module name "cspace".
	To use this, make sure cspace.py and _cspace.so in directory
	$CRYSTAL/scripts/python can be found by python; and don't forget
	to set the CRYSTAL environment variable.
	See $CRYSTAL/scripts/python/pysimpcd.py for an example of usage.

	This file includes scripting language specific SWIG files (see bottom).

	Note: tested with swig 1.3.17, swig 1.1 won't work!

	Thanks to:
		Norman Kramer who made me think about sequence of declarations in
			SWIG file.
		Mark Gossage who made me think about preventing handling of smart
			pointers by SWIG to reduce size of generated code.
*/

%module cspace
%{

#include "css.h"

#include "imap/saver.h"
#include "ivaria/reporter.h"
#include "ivaria/dynamics.h"
#include "ivaria/engseq.h"
#include "iutil/cache.h"
#include "iutil/document.h"
#include "csutil/xmltiny.h"

%}

%include "cstypes.h"

%rename(assign) operator=;

typedef unsigned scfInterfaceID;

#if defined(SWIGPYTHON)

	%define TYPEMAP_OUT_csRef(T)
		%typemap(out) csRef<T>
		{
			csRef<T> ref($1);
			if (ref.IsValid()) {
				ref->IncRef();
				$result = SWIG_NewPointerObj(
					(void *) (T *) ref, $descriptor(T *), 1
				);
			} else {
				Py_INCREF(Py_None);
				$result = Py_None;
			}
		}
	%enddef

	%define TYPEMAP_OUT_csPtr(T)
		%typemap(out) csPtr<T>
		{
			csRef<T> ref($1);
			if (ref.IsValid()) {
				ref->IncRef();
				$result = SWIG_NewPointerObj(
					(void *) (T *) ref, $descriptor(T *), 1
				);
			} else {
				Py_INCREF(Py_None);
				$result = Py_None;
			}
		}
	%enddef

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
					PyErr_SetString(
						PyExc_TypeError, "list must contain strings"
					);
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

#elif defined(SWIGPERL5)

	// TODO help needed...

	%define TYPEMAP_OUT_csRef(T)
		// ignore
	%enddef

	%define TYPEMAP_OUT_csPtr(T)
		// ignore
	%enddef

#else

	%define TYPEMAP_OUT_csRef(T)
		// ignore
	%enddef

	%define TYPEMAP_OUT_csPtr(T)
		// ignore
	%enddef

#endif

%define INTERFACE_PRE(T)

	%nodefault T;

	TYPEMAP_OUT_csRef(T)
	TYPEMAP_OUT_csPtr(T)

%enddef

%ignore csPtr::csPtr;
%ignore csRef::csRef;
%ignore csRef::~csRef;
%ignore csRef::operator =;
%ignore csRef::operator *;

%include "csutil/ref.h"

INTERFACE_PRE(iBase)
INTERFACE_PRE(iBinaryLoaderPlugin)
INTERFACE_PRE(iCamera)
INTERFACE_PRE(iCameraPosition)
INTERFACE_PRE(iCacheManager)
INTERFACE_PRE(iCollider)
INTERFACE_PRE(iCollideSystem)
INTERFACE_PRE(iComponent)
INTERFACE_PRE(iConfigFile)
INTERFACE_PRE(iConfigIterator)
INTERFACE_PRE(iConfigManager)
INTERFACE_PRE(iDataBuffer)
INTERFACE_PRE(iDebugHelper)
INTERFACE_PRE(iDocumentAttributeIterator)
INTERFACE_PRE(iDocumentAttribute)
INTERFACE_PRE(iDocumentNodeIterator)
INTERFACE_PRE(iDocumentNode)
INTERFACE_PRE(iDocument)
INTERFACE_PRE(iDocumentSystem)
INTERFACE_PRE(iDynamics)
INTERFACE_PRE(iDynamicSystem)
INTERFACE_PRE(iEngine)
INTERFACE_PRE(iEvent)
INTERFACE_PRE(iEventHandler)
INTERFACE_PRE(iEventQueue)
INTERFACE_PRE(iFile)
INTERFACE_PRE(iFont)
INTERFACE_PRE(iFontServer)
INTERFACE_PRE(iGraphics3D)
INTERFACE_PRE(iGraphics2D)
INTERFACE_PRE(iImage)
INTERFACE_PRE(iImageIO)
INTERFACE_PRE(iKeyboardDriver)
INTERFACE_PRE(iLightList)
INTERFACE_PRE(iLoader)
INTERFACE_PRE(iLoaderPlugin)
INTERFACE_PRE(iMaterial)
INTERFACE_PRE(iMeshFactoryWrapper)
INTERFACE_PRE(iMeshObject)
INTERFACE_PRE(iMeshObjectFactory)
INTERFACE_PRE(iMeshObjectType)
INTERFACE_PRE(iMeshWrapper)
INTERFACE_PRE(iModelConverter)
INTERFACE_PRE(iMovable)
INTERFACE_PRE(iMovableListener)
INTERFACE_PRE(iObject)
INTERFACE_PRE(iObjectRegistry)
INTERFACE_PRE(iPluginManager)
INTERFACE_PRE(iPolygon3D)
INTERFACE_PRE(iPolygonMesh)
INTERFACE_PRE(iPolygonTexture)
INTERFACE_PRE(iSCF)
INTERFACE_PRE(iScript)
INTERFACE_PRE(iSector)
INTERFACE_PRE(iSectorList)
INTERFACE_PRE(iSoundHandle)
INTERFACE_PRE(iSoundLoader)
INTERFACE_PRE(iSoundRender)
INTERFACE_PRE(iSoundWrapper)
INTERFACE_PRE(iSoundDriver)
INTERFACE_PRE(iSoundSource)
INTERFACE_PRE(iSprite3DState)
INTERFACE_PRE(iStatLight)
INTERFACE_PRE(iStrVector)
INTERFACE_PRE(iTextureHandle)
INTERFACE_PRE(iTextureManager)
INTERFACE_PRE(iTextureWrapper)
INTERFACE_PRE(iThingState)
INTERFACE_PRE(iVFS)
INTERFACE_PRE(iView)
INTERFACE_PRE(iVirtualClock)
INTERFACE_PRE(iVisibilityCuller)

#define CS_STRUCT_ALIGN_4BYTE_BEGIN
#define CS_STRUCT_ALIGN_4BYTE_END

#define CS_GNUC_PRINTF(format_idx, arg_idx)
#define CS_GNUC_SCANF(format_idx, arg_idx)

#ifdef SWIGPYTHON

#endif // SWIGPYTHON

%include "csutil/scf.h"
%include "csutil/cscolor.h"
%include "csutil/cmdhelp.h"
%include "csgeom/vector2.h"

%ignore csVector3::operator[];
%include "csgeom/vector3.h"

%include "csgeom/matrix2.h"
%include "csgeom/matrix3.h"
%include "csgeom/transfrm.h"
%include "csgeom/plane2.h"
%include "csgeom/plane3.h"
%include "csgeom/sphere.h"
//NOTYET %include "csgeom/poly2d.h"
//NOTYET %include "csgeom/poly3d.h"
//NOTYET %include "csgeom/csrect.h"
//NOTYET %include "csgeom/csrectrg.h"
//NOTYET %include "csgeom/quaterni.h"
//NOTYET %include "csgeom/spline.h"
//NOTYET %include "csgeom/cspoint.h"
//NOTYET %include "csgeom/math3d.h"

%rename(asRGBpixel) csRGBpixel::operator csRGBpixel;
%include "csgfx/rgbpixel.h"

%include "cssys/sysfunc.h"

%ignore csInitializer::RequestPlugins;
%ignore csInitializer::SetupEventHandler;
%typemap(default) const char * configName { $1 = NULL; }
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
%include "isound/wrapper.h"
%include "isound/driver.h"
%include "isound/source.h"
%include "iutil/comp.h"
%include "iutil/cache.h"
%include "iutil/document.h"
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
%include "iutil/cfgmgr.h"
%include "iutil/strvec.h"

%rename(asString) iDataBuffer::operator * () const;
%include "iutil/databuff.h"

%ignore G3DTriangleMesh::vertex_mode;
%ignore G3DTriangleMesh::VM_WORLDSPACE;
%ignore G3DTriangleMesh::VM_VIEWSPACE;
%ignore G3DPolygonMesh::vertex_mode;
%ignore G3DPolygonMesh::VM_WORLDSPACE;
%ignore G3DPolygonMesh::VM_VIEWSPACE;
%include "ivideo/graph3d.h"

%include "ivideo/graph2d.h"
%include "ivideo/fontserv.h"
%include "ivideo/halo.h"
%include "ivideo/texture.h"
%include "ivideo/txtmgr.h"
%include "ivideo/vbufmgr.h"
%include "ivideo/material.h"
%include "ivideo/natwin.h"
%include "igraphic/image.h"
%include "igraphic/imageio.h"

%ignore iReporter::ReportV;
%ignore csReporterHelper::ReportV;
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
//NOTYET %include "csutil/csinput.h"

%define INTERFACE_POST(T)

	%extend T
	{
		~ T ()
			{ SCF_DEC_REF(self); }

		static csPtr<T> _CS_QUERY_REGISTRY (iObjectRegistry * reg)
			{ return CS_QUERY_REGISTRY(reg, T); }

		static csPtr<T> _CS_QUERY_REGISTRY_TAG_INTERFACE (
			iObjectRegistry * reg, const char * tag)
			{ return CS_QUERY_REGISTRY_TAG_INTERFACE(reg, tag, T); }

		static csPtr<T> _SCF_QUERY_INTERFACE (iBase * obj)
			{ return SCF_QUERY_INTERFACE(obj, T); }

		static csPtr<T> _SCF_QUERY_INTERFACE_SAFE (iBase * obj)
			{ return SCF_QUERY_INTERFACE_SAFE(obj, T); }

		static csPtr<T> _CS_QUERY_PLUGIN_CLASS (iPluginManager * obj,
			const char * class_id)
			{ return CS_QUERY_PLUGIN_CLASS(obj, class_id, T); }

		static csPtr<T> _CS_LOAD_PLUGIN (iPluginManager * obj,
			const char * class_id)
			{ return CS_LOAD_PLUGIN(obj, class_id, T); }

		static csPtr<T> _CS_GET_CHILD_OBJECT (iObject * object)
			{ return CS_GET_CHILD_OBJECT(object, T); }

		static csPtr<T> _CS_GET_NAMED_CHILD_OBJECT (iObject * object,
			const char *name)
			{ return CS_GET_NAMED_CHILD_OBJECT(object, T, name); }

		static csPtr<T> _CS_GET_FIRST_NAMED_CHILD_OBJECT (iObject * object,
			const char * name)
			{ return CS_GET_FIRST_NAMED_CHILD_OBJECT(object, T, name); }

	}

%enddef

INTERFACE_POST(iBase)
INTERFACE_POST(iBinaryLoaderPlugin)
INTERFACE_POST(iCamera)
INTERFACE_POST(iCameraPosition)
INTERFACE_POST(iCacheManager)
INTERFACE_POST(iCollider)
INTERFACE_POST(iCollideSystem)
INTERFACE_POST(iComponent)
INTERFACE_POST(iConfigFile)
INTERFACE_POST(iConfigIterator)
INTERFACE_POST(iConfigManager)
INTERFACE_POST(iDataBuffer)
INTERFACE_POST(iDebugHelper)
INTERFACE_POST(iDocumentAttributeIterator)
INTERFACE_POST(iDocumentAttribute)
INTERFACE_POST(iDocumentNodeIterator)
INTERFACE_POST(iDocumentNode)
INTERFACE_POST(iDocument)
INTERFACE_POST(iDocumentSystem)
INTERFACE_POST(iDynamics)
INTERFACE_POST(iDynamicSystem)
INTERFACE_POST(iEngine)
INTERFACE_POST(iEvent)
INTERFACE_POST(iEventHandler)
INTERFACE_POST(iEventQueue)
INTERFACE_POST(iFile)
INTERFACE_POST(iFont)
INTERFACE_POST(iFontServer)
INTERFACE_POST(iGraphics3D)
INTERFACE_POST(iGraphics2D)
INTERFACE_POST(iImage)
INTERFACE_POST(iImageIO)
INTERFACE_POST(iKeyboardDriver)
INTERFACE_POST(iLightList)
INTERFACE_POST(iLoader)
INTERFACE_POST(iLoaderPlugin)
INTERFACE_POST(iMaterial)
INTERFACE_POST(iMeshFactoryWrapper)
INTERFACE_POST(iMeshObject)
INTERFACE_POST(iMeshObjectFactory)
INTERFACE_POST(iMeshObjectType)
INTERFACE_POST(iMeshWrapper)
INTERFACE_POST(iModelConverter)
INTERFACE_POST(iMovable)
INTERFACE_POST(iMovableListener)
INTERFACE_POST(iObject)
INTERFACE_POST(iObjectRegistry)
INTERFACE_POST(iPluginManager)
INTERFACE_POST(iPolygon3D)
INTERFACE_POST(iPolygonMesh)
INTERFACE_POST(iPolygonTexture)
INTERFACE_POST(iSCF)
INTERFACE_POST(iScript)
INTERFACE_POST(iSector)
INTERFACE_POST(iSectorList)
INTERFACE_POST(iSoundHandle)
INTERFACE_POST(iSoundLoader)
INTERFACE_POST(iSoundRender)
INTERFACE_POST(iSoundWrapper)
INTERFACE_POST(iSoundDriver)
INTERFACE_POST(iSoundSource)
INTERFACE_POST(iSprite3DState)
INTERFACE_POST(iStatLight)
INTERFACE_POST(iStrVector)
INTERFACE_POST(iTextureHandle)
INTERFACE_POST(iTextureManager)
INTERFACE_POST(iTextureWrapper)
INTERFACE_POST(iThingState)
INTERFACE_POST(iVFS)
INTERFACE_POST(iView)
INTERFACE_POST(iVirtualClock)
INTERFACE_POST(iVisibilityCuller)

// The following isn't necessary if include/iutil/event.h would have
// named struct instead of anonymous ones...
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

	csPtr<iBase> _CS_QUERY_REGISTRY_TAG (iObjectRegistry * reg,
		const char * tag)
	{
		return CS_QUERY_REGISTRY_TAG(reg, tag);
	}

	csPtr<iBase> _CS_LOAD_PLUGIN_ALWAYS (iPluginManager * plugin_mgr,
		const char * class_id)
	{
		return CS_LOAD_PLUGIN_ALWAYS(plugin_mgr, class_id);
	}

%}

%extend iCollideSystem
{
	csCollisionPair * GetCollisionPairByIndex (int index)
	{
		return self->GetCollisionPairs() + index;
	}
}

%extend csInitializer
{
	static bool _RequestPlugin (iObjectRegistry * object_reg,
		const char * plugName, const char * intName, int scfId, int version)
	{
		return csInitializer::RequestPlugins(
			object_reg, plugName, intName, scfId, version, CS_REQUEST_END
		);
	}

	static bool _RegisterDocumentSystem (iObjectRegistry * object_reg)
	{
	  iDocumentSystem *ds = new csTinyDocumentSystem;
	  bool res = object_reg->Register (ds, "iDocumentSystem");
	  ds->DecRef ();
	  return res;
	}
}

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

%define VECTOR_OBJECT_FUNCTIONS(V)
	V operator + (const V & v) const
		{ return *self + v; }
	V operator - (const V & v) const
		{ return *self - v; }
	float operator * (const V & v) const
		{ return *self * v; }
	V operator * (float f) const
		{ return *self * f; }
	V operator / (float f) const
		{ return *self * f; }
	bool operator == (const V & v) const
		{ return *self == v; }
	bool operator != (const V & v) const
		{ return *self != v; }
	bool operator < (float f) const
		{ return *self < f; }
	bool operator > (float f) const
		{ return f > *self; }
%enddef

%extend csVector2
{
	VECTOR_OBJECT_FUNCTIONS(csVector2)
}

%extend csVector3
{
	VECTOR_OBJECT_FUNCTIONS(csVector3)

	csVector3 & operator *= (const csTransform & t)
		{ return *self *= t; }
	csVector3 & operator /= (const csReversibleTransform & t)
		{ return *self /= t; }
	csVector3 operator / (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csPlane3
{
	csPlane3 & operator *= (const csTransform & t)
		{ return *self *= t; }
	csPlane3 & operator /= (const csReversibleTransform & t)
		{ return *self /= t; }
	csPlane3 operator / (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csSphere
{
	csSphere & operator *= (const csTransform & t)
		{ return *self *= t; }
	csSphere operator / (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csMatrix3
{
	csMatrix3 operator + (const csMatrix3 & m)
		{ return *self + m; }
	csMatrix3 operator - (const csMatrix3 & m)
		{ return *self - m; }
	csMatrix3 operator * (const csMatrix3 & m)
		{ return *self * m; }
	csVector3 operator * (const csVector3 & v)
		{ return *self * v; }
	csMatrix3 operator * (float f)
		{ return *self * f; }
	csMatrix3 operator / (float f)
		{ return *self / f; }
	bool operator == (const csMatrix3 & m) const
		{ return *self == m; }
	bool operator != (const csMatrix3 & m) const
		{ return *self != m; }
	bool operator < (float f) const
		{ return *self < f; }
	csMatrix3 operator * (const csTransform & t) const
		{ return *self * t; }
	csMatrix3 & operator *= (const csTransform & t)
		{ return *self *= t; }
}

%extend csTransform
{
	csVector3 operator * (const csVector3 & v) const
		{ return *self * v; } 
	csPlane3 operator * (const csPlane3 & p) const
		{ return *self * p; } 
	csSphere operator * (const csSphere & s) const
		{ return *self * s; } 
	csMatrix3 operator * (const csMatrix3 & m) const
		{ return *self * m; } 
	csTransform operator * (const csReversibleTransform & t) const
		{ return *self * t; } 
}

%extend csReversibleTransform
{
	csReversibleTransform & operator *= (const csReversibleTransform & t)
		{ return *self *= t; }
	csReversibleTransform operator * (const csReversibleTransform & t)
		{ return *self * t; }
	csReversibleTransform & operator /= (const csReversibleTransform & t)
		{ return *self /= t; }
	csReversibleTransform operator / (const csReversibleTransform & t)
		{ return *self / t; }
}

%extend csColor
{
	csColor operator + (const csColor & c) const
		{ return *self + c; }
	csColor operator - (const csColor & c) const
		{ return *self - c; }
	csColor operator * (float f) const
		{ return *self * f; }
}

#ifdef SWIGPYTHON
	%include "ivaria/pythoncs.i"
#endif

#ifdef SWIGPERL5
// TODO help needed...
//	%include "ivaria/perl5cs.i"
#endif


