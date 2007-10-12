/*
    Copyright (C) 2003 Rene Jager <renej_frog@users.sourceforge.net>

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

#ifdef SWIGCSHARP
#include <stdlib.h>
#include <stdio.h>

%csconst(1);

// Fill LIST_OBJECT_FUNCTIONS to extend list interfaces.
#undef LIST_OBJECT_FUNCTIONS
%define LIST_OBJECT_FUNCTIONS(classname,typename)
%enddef
// Fill SET_OBJECT_FUNCTIONS to extend csSet interfaces.
#undef SET_OBJECT_FUNCTIONS
%define SET_OBJECT_FUNCTIONS(classname,typename)
%enddef

%typemap(csbase) iBase "cspace";

#if 0
/****************************************************************************
 * Ignore the operator overloads that C# don't support.
 * C# only support +, -, !, ~, ++, --, true, or false
 * +, -, *, /, %, &, |, ^, <<, >>, ==, !=, >, <, >=, or <=
 * FIXME: SWIG complaints about the operator, even if C# support they.
 ****************************************************************************/

%ignore operator+=;
%ignore operator-=;
%ignore operator*=;
%ignore operator/=;
%ignore operator%=;
%ignore operator* ();
%ignore operator[];
%ignore operator<<=;
%ignore operator>>=;
%ignore operator&=;
%ignore operator|=;
%ignore operator^=;
%ignore operator&&=;
%ignore operator||=;
%ignore operator->;
%ignore operator->*;
%ignore operator,;
%ignore operator&&;
%ignore operator||;
%ignore operator new;
%ignore operator new[];
%ignore operator delete;
%ignore operator delete[];

#else

%ignore operator[];
%rename(get) operator[] const;
%ignore operator();
%rename(add) operator+;
%rename(subtract) operator-;
%rename(multiply) operator*;
%rename(divide) operator/;
%rename(modulo) operator%;
%rename(leftShift) operator<<;
%rename(rightShift) operator>>;
%rename(bitAnd) operator&;
%rename(bitOr) operator|;
%rename(bitXor) operator^;
%ignore operator&&;
%ignore operator||;
%rename(isLessThan) operator<;
%rename(equalsOrLess) operator<=;
%rename(isGreaterThen) operator>;
%rename(equalsOrGreater) operator>=;
%rename(equals) operator==;
%rename(equalsNot) operator!=;
%ignore operator+();
%rename(negate) operator-();
%ignore operator!;
//%rename(not) operator!;
%rename(bitComplement) operator~;
%rename(increment) operator++();
%rename(getAndIncrement) operator++(int);
%rename(decrement) operator--();
%rename(getAndDecrement) operator--(int);

%rename(assign) operator=;
%rename(addAssign) operator+=;
%rename(subtractAssign) operator-=;
%rename(multiplyAssign) operator*=;
%rename(divideAssign) operator/=;
%rename(moduloAssign) operator%=;
%rename(leftShiftAssign) operator<<=;
%rename(rightShiftAssign) operator>>=;
%rename(bitAssign) operator&=;
%rename(bitOrAssign) operator|=;
%rename(bitXorAssign) operator^=;

// csutil/cscolor.h
%rename(assign4) csColor4::operator=(const csColor&);
%rename(multiplyAssign4) csColor4::operator*=(float);
%rename(addAssign4) csColor4::operator+=(const csColor&);

#endif
// cstool/initapp.h
%extend csPluginRequest
{
  csPluginRequest(char const* cls, char const* intf)
  {
    return new csPluginRequest(cls, intf, iSCF::SCF->GetInterfaceID(intf), 0);
  }
}

%{
    //This structure is used for return a interface
    // pointer to the managed code
    struct csRetInterface
    {
	void * ptr;
	char *szClassName;
	int free;
    };

    struct csArgs
    {
	int free;
	int argc;
	const char **argv;
    };

    struct csInterfaceData
    {
	int free;
	int version;
	char *name;
    };
%}
// csgeom/transfrm.h
%ignore csTransform::operator*;
%ignore csReversibleTransform::operator*;

#ifndef CS_MINI_SWIG
%extend csTransform
{
  static csMatrix3 mulmat1 (const csMatrix3& m, const csTransform& t)
  { return m * t; }
  static csMatrix3 mulmat2 (const csTransform& t, const csMatrix3& m)
  { return t * m; }
}
%extend csReversibleTransform
{
  static csTransform mulrev (const csTransform& t1,
                             const csReversibleTransform& t2)
  { return t1 * t2; } 
}
#endif // CS_MINI_SWIG

// iutil/event.h
%{
    // Workaround for bug in SWIG 1.3.19: reversed accessor functions!
    #define iEvent_get_Key iEvent_Key_get
    #define iEvent_get_Mouse iEvent_Mouse_get
    #define iEvent_get_Joystick iEvent_Joystick_get
    #define iEvent_get_Command iEvent_Command_get
%}

%define TYPEMAP_OUT_csRef_BODY(ptr, type, wrapper)
  csRef<type> ref((wrapper<type>&)ptr); /* explicit cast */
  if(!ref.IsValid())
  {
    $result = NULL;
  }
  else
  {
  ref->IncRef();
  $result = (void*)((type*)ref);
  }
%enddef

#undef TYPEMAP_OUT_csRef
%define TYPEMAP_OUT_csRef(T)
  %typemap(out) csRef<T>
  {
    TYPEMAP_OUT_csRef_BODY($1, T, csRef)
  }
  %typemap(ctype) csRef<T> "void*";
  %typemap(imtype, out="IntPtr") csRef<T> "HandleRef";
  %typemap(cstype) csRef<T> #T;
  %typemap(csin) csRef<T> "T.getCPtr($csinput)";
  %typemap(csout, excode=SWIGEXCODE) csRef<T> 
  {
    IntPtr _retptr = $imcall;
    $excode;
    T _ret = new T(_retptr, false);
    return _ret;
  }
%enddef

#undef TYPEMAP_OUT_csPtr
%define TYPEMAP_OUT_csPtr(T)
  %typemap(out) csPtr<T>
  {
    TYPEMAP_OUT_csRef_BODY($1, T, csPtr)
  }
  //%typemap(out) csPtr<T> %{ $result = $1; %}
  %typemap(ctype) csPtr<T> "void *";
  %typemap(imtype, out="IntPtr") csPtr<T> "HandleRef";
  %typemap(cstype) csPtr<T> #T;
  %typemap(csin) csPtr<T> "T.getCPtr($csinput)";
  %typemap(csout, excode=SWIGEXCODE) csPtr<T>
  {
    IntPtr _retptr = $imcall;
    $excode;
    T _ret = new T(_retptr, false);
    return _ret;
  }
%enddef

#undef TYPEMAP_OUT_csWrapPtr
%define TYPEMAP_OUT_csWrapPtr
  %typemap(out) csWrapPtr
  {
    char __className[1024];
    iBase * ibase = (iBase *)$1.Ref;
    void * ptr = ibase->QueryInterface(iSCF::SCF->GetInterfaceID($1.Type), $1.Version);
    ibase->DecRef(); // Undo IncRef from QueryInterface
    if (ptr == 0)
    {
      $result.ptr = NULL;
      memset((void*)$result.szClassName, 0, sizeof($result.szClassName));
      $result.szClassName = strdup("<dummy>");
      $result.free = 1; //Should be freed the className?
    }
    else
    {
      sprintf(__className, "CrystalSpace.%s", $1.Type);
      $result.szClassName = strdup(__className);
      $result.ptr = ptr;
      $result.free = 1; //Should be freed the className?
    }
  }
  //%typemap(out) csWrapPtr %{ $result = $1; %}
  %typemap(ctype) csWrapPtr "csRetInterface";
  %typemap(imtype) csWrapPtr "CrystalSpace.InteropServices.csRetInterface";
  %typemap(cstype) csWrapPtr "Object";
  %typemap(csin) csWrapPtr "$csinput";
  //%typemap(csout) csWrapPtr { return $imcall; }
  %typemap(csout) csWrapPtr
  {
    CrystalSpace.InteropServices.csRetInterface _iret= $imcall;
    return CrystalSpace.InteropServices.csArgsUtils.CreateInterface(_iret);
  }
%enddef

#undef INTERFACE_EQUALS
%define INTERFACE_EQUALS
  public override bool Equals (object obj)
  {
    bool equal = false;
    if (obj is $csclassname)
      equal = (($csclassname)obj).swigCPtr.Equals(this.swigCPtr);
    return equal;
  }

  public override int GetHashCode()
  {
    return this.swigCPtr.GetHashCode();
  }

%enddef

#undef INTERFACE_APPLY
#define INTERFACE_APPLY(T) %typemap(cscode) T %{ INTERFACE_EQUALS %}

APPLY_FOR_EACH_INTERFACE

// ivaria/event.h
// Swig 1.3.23 introduces support for default arguments, so it generates this
// automatically. Prior versions do not.
#if (SWIG_VERSION < 0x010323)
#define IEVENTOUTLET_BROADCAST 		\
public void Broadcast (int iCode) { Broadcast(iCode, new HandleRef(this, IntPtr.Zero)); }
#else
#define IEVENTOUTLET_BROADCAST
#endif

#undef IEVENTOUTLET_CSHARPCODE
%define IEVENTOUTLET_CSHARPCODE
%typemap(cscode) iEventOutlet
%{
  INTERFACE_EQUALS
  IEVENTOUTLET_BROADCAST
%}
%enddef
IEVENTOUTLET_CSHARPCODE


// iutil/cfgmgr.h
// Swig 1.3.21 (and possibly earlier) have a bug where enums are emitted as
// illegal C# code:
//   public final static int ConfigPriorityFoo = iConfigManager::PriorityFoo;
// rather than the legal code:
//   public final static int ConfigPriorityFoo = iConfigManager.PriorityFoo;
// We work around this by %ignoring those constants and defining them maually.
%ignore iConfigManager::ConfigPriorityPlugin;
%ignore iConfigManager::ConfigPriorityApplication;
%ignore iConfigManager::ConfigPriorityUserGlobal;
%ignore iConfigManager::ConfigPriorityUserApp;
%ignore iConfigManager::ConfigPriorityCmdLine;

#undef ICONFIGMANAGER_CSHARPCODE
%define ICONFIGMANAGER_CSHARPCODE
%typemap(cscode) iConfigManager
%{
  INTERFACE_EQUALS
  public const int ConfigPriorityPlugin =
    iConfigManager.PriorityVeryLow;
  public const int ConfigPriorityApplication =
    iConfigManager.PriorityLow;
  public const int ConfigPriorityUserGlobal =
    iConfigManager.PriorityMedium;
  public const int ConfigPriorityUserApp =
    iConfigManager.PriorityHigh;
  public const int ConfigPriorityCmdLine =
    iConfigManager.PriorityVeryHigh;
%}
%enddef
ICONFIGMANAGER_CSHARPCODE

// We define directly somes constants
%ignore VFS_FILE_UNCOMPRESSED;
%ignore CS_ZBUF_MESH;
%ignore CS_ZBUF_MESH2;
%ignore CS_MIXMODE_TYPE_MESH;
%ignore CS_MIXMODE_TYPE_MASK;
%ignore CS_FX_MESH;
%ignore CS_FX_MASK_MIXMODE;
%ignore CS_LIGHT_ACTIVEHALO;
%ignore CS_IMGFMT_INVALID;
%ignore CSKEY_BACKSPACE;
%ignore CS_BOUNDINGBOX_MAXVALUE;

//TODO: fix the bug
%ignore csImageBase::GetKeyColor;
%ignore csProcTexture::SetKeyColor;

// Should probably be rewritten using csString for className
%typemap(in) (const char * iface, int iface_ver)
{
    $1 = strdup($input.name);
    $2 = $input.version;
}

%typemap(ctype) (const char * iface, int iface_ver) "csInterfaceData"
%typemap(imtype) (const char * iface, int iface_ver) "CrystalSpace.InteropServices.csInterfaceData"
%typemap(cstype) (const char * iface, int iface_ver) "Type"
%typemap(csin) (const char * iface, int iface_ver) "CrystalSpace.InteropServices.csArgsUtils.PackInterfaceData($csinput)"

%typemap(freearg) (const char * iface, int iface_ver)
{
    free($1);
    if($input.free)
    {
	free($input.name);
	$input.free=0;
    }
}

// argc-argv handling
%typemap(in) (int argc, char const * const argv[])
{
  $1 = $input.argc + 1; // +1 for synthesized argv[0].
  $2 = (char **) malloc(($1 + 1) * sizeof(char *));
  $2[0] = strdup("./cspacesharp");
  /* make a copy of each string */
  int i;
  for (i = 1; i < $1; i++) {
    $2[i] = strdup($input.argv[i-1]);
  }
}

%typemap(freearg) (int argc, char const * const argv[])
{
  for (int i = 0; i < $1; i++)
    free((void*)$2[i]);
  free((void*)$2);
  if($input.free)
  {
    for (int i = 0; i < $input.argc; i++)
      free((void*)$input.argv[i]);
    free((void*)$input.argv);
    $input.free = 0;
  }
}

%typemap(ctype) (int argc, char const * const argv []) "csArgs"
%typemap(imtype) (int argc, char const * const argv []) "CrystalSpace.InteropServices.csArgs"
%typemap(cstype) (int argc, char const * const argv []) "String[]"
%typemap(csin) (int argc, char const * const argv []) "CrystalSpace.InteropServices.csArgsUtils.FromString($csinput)"

// Rename somes methods
%rename(GetJointType) iODEJointState::GetType;
%rename(GetVFType) iParticleBuiltinEffectorVelocityField::GetType;
%rename(SetVFType) iParticleBuiltinEffectorVelocityField::SetType;
%rename(GetLightType) iLight::GetType;
%rename(SetLightType) iLight::SetType;
%rename(GetNodeType) iDocumentNode::GetType;
%rename(GetVariableType) csShaderVariable::GetType;
%rename(SetVariableType) csShaderVariable::SetType;

#endif SWIGCSHARP
