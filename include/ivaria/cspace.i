/*
    Copyright (C) 2003 by Rene Jager (renej@frog.nl, renej.frog@yucom.be)

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

	This file includes scripting language specific SWIG files.
	Search for 'pythpre' (pre-include phase) and 'pythpost' (post-include
	phase) for places where to include files for new scripting languages.

	Python:
		Used for the new "cspython" plugin based on python 2.2 including
		almost complete access to CS.

		Additionally, provides CS as a python 2.2 module name "cspace".
		To use this, make sure cspace.py and _cspace.so in directory
		$CRYSTAL/scripts/python can be found by python; and don't forget
		to set the CRYSTAL environment variable.
		See $CRYSTAL/scripts/python/pysimpcd.py for an example of usage.

	Perl:
		Used for the "csperl5" plugin based on perl 5.8 including
		access to most parts of CS
		(although some ealier versions are probably supported).

		Additionally, provices CS as a perl 5.8 module name "cspace".
		Make sure the directory containing cspace.pm and cspace.so,
		$CRYSTAL/scripts/perl5, is in perl's @INC array.

	Note:
		Tested with swig 1.3.17, swig 1.1 won't work!

	Thanks to:
		Norman Kramer <norman@users.sourceforge.net> who made me think
			about sequence of declarations in SWIG file.
		Mark Gossage <mark@gossage.cjb.net> who made me think about
			preventing handling of smart pointers by SWIG to reduce
			size of generated code.
		Mat Sutcliffe <oktal@gmx.co.uk> for input, Perl includes,
			and wrapping SCF and other macros.
*/

/*
	SWIG's preprocessor can be used to "replace" a macro by a function
	in the target scripting language using the following:

	In C header file:

		#define X(arg) ...

	which as a function would have the prototype:

		int X (int i);

	Now in the SWIG *.i file do:

		#define _X(a) X(a)
		#undef X
		int _X (int i);

	and there will be a "X" function in the scripting language available.
*/

%module cspace

#ifdef SWIGPERL5
	%include "ivaria/perl1st.i"
#endif

/* @@@ The following is a kludge. In the future, all cspace modules for all
 *     scripting languages should include css.h in the same way Perl does.
 */
#ifndef SWIGPERL5
  %{
    #include "css.h"
  %}
#endif

%{

#include "imap/saver.h"
#include "ivaria/reporter.h"
#include "ivaria/dynamics.h"
#include "ivaria/engseq.h"
#include "iutil/cache.h"
#include "csutil/xmltiny.h"
#include "igeom/objmodel.h"
#include "inetwork/sockerr.h"
#include "inetwork/netman.h"
#include "inetwork/socket2.h"

// Mark Gossage: somewhere in winuser.h there are a couple of #defines to
// rename RegisterClass and UnregisterClass (which are windoze fns).
// They also accidentally rename the iSCF fns too.
#ifdef RegisterClass
#	undef RegisterClass
#endif
#ifdef UnregisterClass
#	undef UnregisterClass
#endif

%}

// The following list holds all the interfaces that are handled correctly.
// If you have problems, first check if the interface in question is in
// this list. Please keep the list sorted alphabetically.
%define APPLY_FOR_EACH_INTERFACE
	INTERFACE_APPLY(iAudioStream)
	INTERFACE_APPLY(iBase)
	INTERFACE_APPLY(iBinaryLoaderPlugin)
	INTERFACE_APPLY(iBodyGroup)
	INTERFACE_APPLY(iCamera)
	INTERFACE_APPLY(iCameraPosition)
	INTERFACE_APPLY(iCacheManager)
	INTERFACE_APPLY(iCollider)
	INTERFACE_APPLY(iCollideSystem)
	INTERFACE_APPLY(iComponent)
	INTERFACE_APPLY(iConfigFile)
	INTERFACE_APPLY(iConfigIterator)
	INTERFACE_APPLY(iConfigManager)
	INTERFACE_APPLY(iDataBuffer)
	INTERFACE_APPLY(iDebugHelper)
	INTERFACE_APPLY(iDocument)
	INTERFACE_APPLY(iDocumentSystem)
	INTERFACE_APPLY(iDynamics)
	INTERFACE_APPLY(iDynamicSystem)
	INTERFACE_APPLY(iEngine)
	INTERFACE_APPLY(iEvent)
	INTERFACE_APPLY(iEventHandler)
	INTERFACE_APPLY(iEventQueue)
	INTERFACE_APPLY(iFactory)
	INTERFACE_APPLY(iFile)
	INTERFACE_APPLY(iFont)
	INTERFACE_APPLY(iFontServer)
	INTERFACE_APPLY(iFrustumView)
	INTERFACE_APPLY(iFrustumViewUserdata)
	INTERFACE_APPLY(iGraphics3D)
	INTERFACE_APPLY(iGraphics2D)
	INTERFACE_APPLY(iHalo)
	INTERFACE_APPLY(iImage)
	INTERFACE_APPLY(iImageIO)
	INTERFACE_APPLY(iJoint)
	INTERFACE_APPLY(iKeyboardDriver)
	INTERFACE_APPLY(iLightList)
	INTERFACE_APPLY(iLoader)
	INTERFACE_APPLY(iLoaderPlugin)
	INTERFACE_APPLY(iMaterial)
	INTERFACE_APPLY(iMaterialWrapper)
	INTERFACE_APPLY(iMeshFactoryWrapper)
	INTERFACE_APPLY(iMeshObject)
	INTERFACE_APPLY(iMeshObjectFactory)
	INTERFACE_APPLY(iMeshObjectType)
	INTERFACE_APPLY(iMeshWrapper)
	INTERFACE_APPLY(iModelConverter)
	INTERFACE_APPLY(iMovable)
	INTERFACE_APPLY(iMovableListener)
	INTERFACE_APPLY(iNetworkConnection)
	INTERFACE_APPLY(iNetworkDriver)
	INTERFACE_APPLY(iNetworkEndPoint)
	INTERFACE_APPLY(iNetworkListener)
	INTERFACE_APPLY(iNetworkManager)
	INTERFACE_APPLY(iNetworkPacket)
	INTERFACE_APPLY(iNetworkSocket2)
	INTERFACE_APPLY(iObject)
	INTERFACE_APPLY(iObjectModel)
	INTERFACE_APPLY(iObjectModelListener)
	INTERFACE_APPLY(iObjectRegistry)
	INTERFACE_APPLY(iPluginManager)
	INTERFACE_APPLY(iPolygon3D)
	INTERFACE_APPLY(iPolygonMesh)
	INTERFACE_APPLY(iPolygonTexture)
	INTERFACE_APPLY(iSCF)
//	INTERFACE_APPLY(iScript)
	INTERFACE_APPLY(iSector)
	INTERFACE_APPLY(iSectorList)
	INTERFACE_APPLY(iSoundHandle)
	INTERFACE_APPLY(iSoundLoader)
	INTERFACE_APPLY(iSoundRender)
	INTERFACE_APPLY(iSoundWrapper)
	INTERFACE_APPLY(iSoundDriver)
	INTERFACE_APPLY(iSoundSource)
	INTERFACE_APPLY(iSprite2DState)
	INTERFACE_APPLY(iSprite3DState)
	INTERFACE_APPLY(iStatLight)
	INTERFACE_APPLY(iStream)
	INTERFACE_APPLY(iStreamIterator)
	INTERFACE_APPLY(iStreamFormat)
	INTERFACE_APPLY(iString)
	INTERFACE_APPLY(iStrVector)
	INTERFACE_APPLY(iTextureHandle)
	INTERFACE_APPLY(iTextureList)
	INTERFACE_APPLY(iTextureManager)
	INTERFACE_APPLY(iTextureWrapper)
	INTERFACE_APPLY(iThingState)
	INTERFACE_APPLY(iVFS)
	INTERFACE_APPLY(iVideoStream)
	INTERFACE_APPLY(iView)
	INTERFACE_APPLY(iVirtualClock)
	INTERFACE_APPLY(iVisibilityCuller)
%enddef

%include "typemaps.i"

// The following list all kown arguments that are actually (also) outputs.
// It is possible that some of the output arguments should be INOUT instead
// of OUTPUT.

// output arguments
%apply unsigned char * OUTPUT { uint8 & red };
%apply unsigned char * OUTPUT { uint8 & green };
%apply unsigned char * OUTPUT { uint8 & blue };
%apply unsigned char * OUTPUT { uint8 & oR };
%apply unsigned char * OUTPUT { uint8 & oG };
%apply unsigned char * OUTPUT { uint8 & oB };
%apply int * OUTPUT { int & red };
%apply int * OUTPUT { int & green };
%apply int * OUTPUT { int & blue };
%apply int * OUTPUT { int & r };
%apply int * OUTPUT { int & g };
%apply int * OUTPUT { int & b };
%apply int * OUTPUT { int & mw };
%apply int * OUTPUT { int & mh };
%apply int * OUTPUT { int & oW };
%apply int * OUTPUT { int & oH };
%apply int * OUTPUT { int & oR };
%apply int * OUTPUT { int & oG };
%apply int * OUTPUT { int & oB };
%apply int * OUTPUT { int & w };
%apply int * OUTPUT { int & h };
%apply int * OUTPUT { int & bw };
%apply int * OUTPUT { int & bh };
%apply int * OUTPUT { int & ClientW };
%apply int * OUTPUT { int & ClientH };
%apply int * OUTPUT { int & totw };
%apply int * OUTPUT { int & toth };
%apply int * OUTPUT { int & sugw };
%apply int * OUTPUT { int & sugh };
%apply int * OUTPUT { int & adv };
%apply int * OUTPUT { int & left };
%apply int * OUTPUT { int & top };
%apply int * OUTPUT { int & desc };
%apply int * OUTPUT { int & nr };
%apply int * OUTPUT { int & key };
%apply int * OUTPUT { int & shift };
%apply int * OUTPUT { int & button };
%apply int * OUTPUT { int & ver };
%apply int * OUTPUT { int & scfInterfaceID };
%apply int * OUTPUT { int & a };
%apply int * OUTPUT { int & x };
%apply int * OUTPUT { int & y };
%apply int * OUTPUT { int & oFontSize };
%apply int * OUTPUT { int & oUnderlinePos };
%apply int * OUTPUT { int & newX };
%apply int * OUTPUT { int & newY };
%apply int * OUTPUT { int & newH };
%apply int * OUTPUT { int & newW };
%apply int * OUTPUT { int & xmin };
%apply int * OUTPUT { int & ymin };
%apply int * OUTPUT { int & xmax };
%apply int * OUTPUT { int & ymax };
%apply int * OUTPUT { int & theRow };
%apply int * OUTPUT { int & theCol };
%apply int * OUTPUT { int & row };
%apply int * OUTPUT { int & col };
%apply int * OUTPUT { int & newHeight };
%apply int * OUTPUT { int & newWidth };
%apply int * OUTPUT { int & oColor };
%apply int * OUTPUT { int & val };
%apply float * OUTPUT { float & oDiffuse };
%apply float * OUTPUT { float & oAmbient };
%apply float * OUTPUT { float & oReflection };
%apply float * OUTPUT { float & red };
%apply float * OUTPUT { float & green };
%apply float * OUTPUT { float & blue };
%apply float * OUTPUT { float & a };
%apply float * OUTPUT { float & b };
%apply float * OUTPUT { float & x1 };
%apply float * OUTPUT { float & y1 };
%apply float * OUTPUT { float & x2 };
%apply float * OUTPUT { float & y2 };
%apply float * OUTPUT { float & oH };
%apply float * OUTPUT { float & oS };
%apply float * OUTPUT { float & oR };
%apply float * OUTPUT { float & oG };
%apply float * OUTPUT { float & oB };
%apply float * OUTPUT { float & r };
%apply float * OUTPUT { float & g };
%apply float * OUTPUT { float & b };
%apply float * OUTPUT { float & l };
%apply float * OUTPUT { float & h };
%apply float * OUTPUT { float & s };
%apply float * OUTPUT { float & start };
%apply float * OUTPUT { float & dist };
%apply float * OUTPUT { float & w };
%apply float * OUTPUT { float & ra };
%apply float * OUTPUT { float & rb };

// input/output arguments
%apply int * INOUT { int & maxcolors };
%apply float * INOUT { float & iR };
%apply float * INOUT { float & iG };
%apply float * INOUT { float & iB };

%include "cstypes.h"

%inline %{

	// This pointer wrapper can be used to prevent code-bloat by macro's acting
	// as template functions.
	// Examples are SCF_QUERY_INTERFACE and CS_QUERY_REGISTRY.
	// Thanks to Mat Sutcliffe <oktal@gmx.co.uk>.
	// Note that this works _only_ if you're _not_ using virtual inheritance!
	//
	// renej: The VoidPtr is added to handle case where the void pointer is
	// the actual pointer. Ref<iBase> is used for pointers that need casting
	// when transferred to the scripting language.

	struct csWrapPtr
	{
	  csRef<iBase> Ref;
	  void *VoidPtr;
	  const char *Type;
	  csWrapPtr (const char *t, iBase *r)
		: Ref (r), VoidPtr (0), Type (t) {}
	  csWrapPtr (const char *t, csPtr<iBase> r)
		: Ref (r), VoidPtr (0), Type (t) {}
	  csWrapPtr (const char *t, csRef<iBase> r)
		: Ref (r), VoidPtr(0), Type (t) {}
	  csWrapPtr (const char *t, void *p)
		: VoidPtr (p), Type (t) {}
	  csWrapPtr (const csWrapPtr &p)
		: Ref (p.Ref), VoidPtr (p.VoidPtr), Type (p.Type) {}
	};

%}

// Macro's expected in rest of this file: ignored by default.
// When overriding in language-specific files, first #undef and then
// re-%define them.
#define TYPEMAP_OUT_csRef(T)
#define TYPEMAP_OUT_csPtr(T)
#define TYPEMAP_OUT_csWrapPtr
#define TYPEMAP_IN_ARRAY_CNT_PTR(a,b)
#define TYPEMAP_IN_ARRAY_PTR_CNT(a,b) 
#define TYPEMAP_OUTARG_ARRAY_PTR_CNT(a,b,c)

#if defined(SWIGPYTHON)
	%include "ivaria/pythpre.i"
#elif defined(SWIGPERL5)
	%include "ivaria/perlpre.i"
#elif defined(SWIGRUBY)
	%include "ivaria/rubypre.i"
#elif defined(SWIGTCL8)
	%include "ivaria/tclpre.i"
#endif

// Handle arrays as input arguments.
TYPEMAP_IN_ARRAY_CNT_PTR(
	(int num_layers, iTextureWrapper ** wrappers), /**/
)
TYPEMAP_IN_ARRAY_PTR_CNT(
	(csVector2 * InPolygon, int InCount), *
)

// Handle arrays as output arguments.
TYPEMAP_OUTARG_ARRAY_PTR_CNT(
	(csVector2 * OutPolygon, int & OutCount),
	new csVector2[MAX_OUTPUT_VERTICES], *
)

%ignore csPtr::csPtr;
%ignore csRef::csRef;
%ignore csRef::~csRef;
%ignore csRef::operator =;
%ignore csRef::operator *;

%include "csutil/ref.h"

%define INTERFACE_PRE(T)

	%nodefault T;
	%ignore T##_scfGetID ();

	TYPEMAP_OUT_csRef(T)
	TYPEMAP_OUT_csPtr(T)

%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_PRE(x)
APPLY_FOR_EACH_INTERFACE

TYPEMAP_OUT_csWrapPtr

// Ignored macro's.
#define CS_STRUCT_ALIGN_4BYTE_BEGIN
#define CS_STRUCT_ALIGN_4BYTE_END
#define CS_GNUC_PRINTF(format_idx, arg_idx)
#define CS_GNUC_SCANF(format_idx, arg_idx)
#define CS_DECLARE_STATIC_CLASSVAR(a, b, c)

// Inclusion of CS headers.
// The sequence of %include-ing the CS headers can be crucial!
// The scheme is as follows: %ignore'd functions and types are placed
// before actual inclusion, as are "local" %typemap's (like default).
// After %include-ing the header resets can be done; for example
// resetting the %typemap(default). The %extend-ing and extra code takes
// place after all %include's are done, mentioning the header(s) it is
// related to.

%include "csutil/scf.h"

%include "iutil/dbghelp.h"

%ignore operator* (const csColor &, float);
%ignore operator* (float , const csColor &);
%ignore operator+ (const csColor &, const csColor &);
%ignore operator- (const csColor &, const csColor &);
%include "csutil/cscolor.h"

%include "csutil/cmdhelp.h"
%include "csutil/strset.h"

%include "iutil/string.h"
//%include "csutil/csstring.h"

%ignore csVector2::operator[];
%ignore csVector2::Norm (const csVector2 &);
%include "csgeom/vector2.h"

%ignore csVector3::operator[];
%ignore csVector3::Norm (const csVector3 &);
%ignore csVector3::Unit (const csVector3 &);
%include "csgeom/vector3.h"

%include "csgeom/matrix2.h"
%include "csgeom/matrix3.h"
%include "csgeom/transfrm.h"
%include "csgeom/sphere.h"

%ignore csPlane2::A ();
%ignore csPlane2::B ();
%ignore csPlane2::C ();
%include "csgeom/plane2.h"

%ignore csPlane3::A ();
%ignore csPlane3::B ();
%ignore csPlane3::C ();
%ignore csPlane3::D ();
%include "csgeom/plane3.h"

%include "csgeom/math2d.h"

%ignore csPoly2D::operator[];
%include "csgeom/poly2d.h"

%include "csgeom/math3d.h"

%ignore csPoly3D::operator[];
%include "csgeom/poly3d.h"

%include "csgeom/csrect.h"
%include "csgeom/csrectrg.h"
%include "csgeom/quaterni.h"
%include "csgeom/spline.h"
%include "csgeom/cspoint.h"

%rename(asRGBcolor) csRGBpixel::operator csRGBcolor;
%include "csgfx/rgbpixel.h"

%ignore csGetPlatformConfig;
%include "cssys/sysfunc.h"

%ignore csInitializer::RequestPlugins;
%ignoren csInitializer::SetupEventHandler(iObjectRegistry*, csEventHandlerFunc, unsigned int);
%rename(_SetupEventHandler) csInitializer::SetupEventHandler(iObjectRegistry*, iEventHandler *, unsigned int);
%typemap(default) const char * configName { $1 = 0; }
%include "cstool/initapp.h"
%typemap(default) const char * configName;

%include "igeom/polymesh.h"
%include "igeom/clip2d.h"
%include "igeom/objmodel.h"

%include "iengine/fview.h"
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
%include "imesh/sprite2d.h"
%include "imesh/sprite3d.h"
%include "imesh/thing/thing.h"
%include "imesh/thing/lightmap.h"
%include "imesh/thing/polygon.h"

%extend iSprite2DState
{
  csSprite2DVertex* GetVertexByIndex(int index)
  {
    return &(self->GetVertices()[index]);
  }
}

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
%include "iutil/vfs.h"
%include "iutil/object.h"
%include "iutil/dbghelp.h"
%include "iutil/objreg.h"
%include "iutil/virtclk.h"

%rename(AddInt8) iEvent::Add(const char *, int8);
%rename(AddInt16) iEvent::Add(const char *, int16);
%rename(AddInt32) iEvent::Add(const char *, int32, bool);
%rename(AddUInt8) iEvent::Add(const char *, uint8);
%rename(AddUInt16) iEvent::Add(const char *, uint16);
%rename(AddUInt32) iEvent::Add(const char *, uint32);
%rename(AddFloat) iEvent::Add(const char *, float);
%rename(AddDouble) iEvent::Add(const char *, double);
%rename(AddString) iEvent::Add(const char *, char *);
%rename(AddBool) iEvent::Add(const char *, bool, bool);
%rename(AddVoidPtr) iEvent::Add(const char *, void *, uint32);
%rename(FindInt8) iEvent::Find(const char *, int8 &, int);
%rename(FindInt16) iEvent::Find(const char *, int16 &, int);
%rename(FindInt32) iEvent::Find(const char *, int32 &, bool, int);
%rename(FindUInt8) iEvent::Find(const char *, uint8 &, int);
%rename(FindUInt16) iEvent::Find(const char *, uint16 &, int);
%rename(FindUInt32) iEvent::Find(const char *, uint32 &, int);
%rename(FindFloat) iEvent::Find(const char *, float &, int);
%rename(FindDouble) iEvent::Find(const char *, double &, int);
%rename(FindString) iEvent::Find(const char *, char **, int);
%rename(FindBool) iEvent::Find(const char *, bool &, int);
%rename(FindVoidPtr) iEvent::Find(const char *, void **, uint32 &, int);
%include "iutil/event.h"

%include "iutil/evdefs.h"
%include "iutil/eventq.h"
%include "iutil/eventh.h"
%include "iutil/plugin.h"
%include "iutil/csinput.h"
%include "iutil/cfgfile.h"
%include "iutil/cfgmgr.h"
%include "iutil/strvec.h"
%include "iutil/document.h"

%include "csutil/xmltiny.h"

%ignore iDataBuffer::GetInt8;
%include "iutil/databuff.h"

%include "ivideo/graph3d.h"
%include "ivideo/graph2d.h"

%ignore GetGlyphSize(uint8, int &, int &);
%ignore GetGlyphBitmap(uint8, int &, int &);
%ignore GetGlyphAlphaBitmap(uint8, int &, int &);
%ignore GetDimensions(char const *, int &, int &);
%include "ivideo/fontserv.h"

%include "ivideo/halo.h"

%rename(GetKeyColorStatus) iTextureHandle::GetKeyColor();
%include "ivideo/texture.h"

%include "ivideo/txtmgr.h"
%include "ivideo/vbufmgr.h"
%include "ivideo/material.h"
%include "ivideo/natwin.h"
%include "ivideo/codec.h"

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
//%include "ivaria/script.h"
%include "ivaria/engseq.h"

%include "inetwork/netman.h"
%include "inetwork/sockerr.h"
%include "inetwork/driver.h"
%include "inetwork/socket2.h"

%include "csutil/csobject.h"

%include "cstool/csview.h"
%include "cstool/collider.h"

%define INTERFACE_POST(T)

	%extend T
	{
		~ T () { SCF_DEC_REF(self); }
	}

%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) INTERFACE_POST(x)
APPLY_FOR_EACH_INTERFACE

// When time comes the classic cast "(T*)self" can be changed to
// "dynamic_cast<T*>(self)". For now, this is not necessary because
// classic C casts are used all over the place in CS. Note that for
// readability it might be better to use "static_cast<T*>(self)".
%define CAST_FROM_BASE(T)
	else if (!strcmp(to_name, #T)) ptr = (void*)(T*)self;
%enddef
#undef INTERFACE_APPLY
#define INTERFACE_APPLY(x) CAST_FROM_BASE(x)

%extend iBase
{
	csWrapPtr _DynamicCast (const char * to_name)
	{
		void * ptr;
		if (!to_name || !to_name[0] || !self) ptr = 0;
		APPLY_FOR_EACH_INTERFACE
		else ptr = 0;
		return csWrapPtr(to_name, ptr);
	}
}

#undef CAST_FROM_BASE

// iutil/event.h
%extend iEvent
{
	const csEventKeyData Key;
	const csEventMouseData Mouse;
	const csEventJoystickData Joystick;
	const csEventCommandData Command;
	const csEventNetworkData Network;
}

// iutil/event.h
%{
	csEventKeyData * iEvent_Key_get (iEvent * event)
		{ return &event->Key; }
	csEventMouseData * iEvent_Mouse_get (iEvent * event)
		{ return &event->Mouse; }
	csEventJoystickData * iEvent_Joystick_get (iEvent * event)
		{ return &event->Joystick; }
	csEventCommandData * iEvent_Command_get (iEvent * event)
		{ return &event->Command; }
	csEventNetworkData * iEvent_Network_get (iEvent * event)
		{ return &event->Network; }
%}

// iutil/evdefs.h
#define _CS_IS_KEYBOARD_EVENT(e) CS_IS_KEYBOARD_EVENT(e)
#undef CS_IS_KEYBOARD_EVENT
bool _CS_IS_KEYBOARD_EVENT (const iEvent &);
#define _CS_IS_MOUSE_EVENT(e) CS_IS_MOUSE_EVENT(e)
#undef CS_IS_MOUSE_EVENT
bool _CS_IS_MOUSE_EVENT (const iEvent &);
#define _CS_IS_JOYSTICK_EVENT(e) CS_IS_JOYSTICK_EVENT(e)
#undef CS_IS_JOYSTICK_EVENT
bool _CS_IS_JOYSTICK_EVENT (const iEvent &);
#define _CS_IS_INPUT_EVENT(e) CS_IS_INPUT_EVENT(e)
#undef CS_IS_INPUT_EVENT
bool _CS_IS_INPUT_EVENT (const iEvent &);
#define _CS_IS_NETWORK_EVENT(e) CS_IS_NETWORK_EVENT(e)
#undef CS_IS_NETWORK_EVENT
bool _CS_IS_NETWORK_EVENT (const iEvent &);

// iutil/objreg.h
#define _CS_QUERY_REGISTRY_TAG(a, b) CS_QUERY_REGISTRY_TAG(a, b)
#undef CS_QUERY_REGISTRY_TAG
csPtr<iBase> _CS_QUERY_REGISTRY_TAG (iObjectRegistry *, const char *);

// iutil/plugin.h
#define _CS_LOAD_PLUGIN_ALWAYS(a, b) CS_LOAD_PLUGIN_ALWAYS(a, b)
#undef CS_LOAD_PLUGIN_ALWAYS
csPtr<iBase> _CS_LOAD_PLUGIN_ALWAYS (iPluginManager *, const char *);

// ivaria/collider.h
%extend iCollideSystem
{
	csCollisionPair * GetCollisionPairByIndex (int index)
	{
		return self->GetCollisionPairs() + index;
	}
}

// cstool/initapp.h
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

// ivideo/graph3d.h
#define _CS_FX_SETALPHA(a) CS_FX_SETALPHA(a)
#undef CS_FX_SETALPHA
uint _CS_FX_SETALPHA (uint);
#define _CS_FX_SETALPHA_INT(a) CS_FX_SETALPHA_INT(a)
#undef CS_FX_SETALPHA_INT
uint _CS_FX_SETALPHA_INT (uint);

// csgeom/vector2.h csgeom/vector3.h
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

// csgeom/vector2.h
%extend csVector2
{
	VECTOR_OBJECT_FUNCTIONS(csVector2)
}

// csgeom/vector3.h
%extend csVector3
{
	VECTOR_OBJECT_FUNCTIONS(csVector3)

	csVector3 & operator *= (const csTransform & t)
		{ return *self *= t; }
	csVector3 & operator /= (const csReversibleTransform & t)
		{ return *self /= t; }
	csVector3 operator / (const csReversibleTransform & t)
		{ return *self / t; }
	csVector3 project (const csVector3 & what) const
		{ return what << *self; }
}

// csgeom/plane3.h
%extend csPlane3
{
	csPlane3 & operator *= (const csTransform & t)
		{ return *self *= t; }
	csPlane3 & operator /= (const csReversibleTransform & t)
		{ return *self /= t; }
	csPlane3 operator / (const csReversibleTransform & t)
		{ return *self / t; }
}

// csgeom/sphere.h
%extend csSphere
{
	csSphere & operator *= (const csTransform & t)
		{ return *self *= t; }
	csSphere operator / (const csReversibleTransform & t)
		{ return *self / t; }
}

// csgeom/matrix3.h
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

// csgeom/transfrm.h
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

// csgeom/transfrm.h
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

// csutil/cscolor.h
%extend csColor
{
	csColor operator + (const csColor & c) const
		{ return *self + c; }
	csColor operator - (const csColor & c) const
		{ return *self - c; }
	csColor operator * (float f) const
		{ return *self * f; }
}

// csgeom/quaterni.h
%extend csQuaternion
{
	csQuaternion operator + (const csQuaternion& q)
		{ return *self + q; }
	csQuaternion operator - (const csQuaternion& q)
		{ return *self - q; }
	csQuaternion operator * (const csQuaternion& q)
		{ return *self * q; }
}

#if defined(SWIGPYTHON)
	%include "ivaria/pythpost.i"
#elif defined(SWIGPERL5)
	%include "ivaria/perlpost.i"
#elif defined(SWIGRUBY)
	%include "ivaria/rubypost.i"
#elif defined(SWIGTCL8)
	%include "ivaria/tclpost.i"
#endif


