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

#ifdef SWIGJAVA

#include <stdio.h>
#include <string.h>

%javaconst(1);

// We still support Swig versions older than 1.3.22, so regress to the 'enum'
// style used in earlier versions.
#if (SWIG_VERSION >= 0x010322)
%include "enumsimple.swg"
#endif

// Following are declared as constants here to prevent javac complaining about
// finding a 'long' where an 'int' is expected.
%constant int CS_CRYSTAL_PROTOCOL = 0x43533030;
%constant int CS_MUSCLE_PROTOCOL = 0x504d3030;
%constant int CS_XML_PROTOCOL = 0x584d4d30;
%ignore CS_CRYSTAL_PROTOCOL;
%ignore CS_MUSCLE_PROTOCOL;
%ignore CS_XML_PROTOCOL;

%typemap(javabase) iBase "cspace";

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
%rename(and) operator&&;
%rename(or) operator||;
%rename(isLessThan) operator<;
%rename(equalsOrLess) operator<=;
%rename(isGreaterThen) operator>;
%rename(equalsOrGreater) operator>=;
%rename(equals) operator==;
%rename(equalsNot) operator!=;

%ignore operator+();
%rename(negate) operator-();
%rename(not) operator!;
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

// cstool/initapp.h
%extend csPluginRequest
{
  csPluginRequest(char const* cls, char const* intf)
  {
    return new csPluginRequest(cls, intf, iSCF::SCF->GetInterfaceID(intf), 0);
  }
}

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

%{
jobject _csRef_to_Java(const csRef<iBase>& ref, void* ptr, const char* name,
    const char* clazz, JNIEnv* jenv)
{
  if (!ref.IsValid())
    return 0;
  ref->IncRef();
  jlong cptr = 0;
  *(void **)&cptr = ptr; 
  jclass cls = jenv->FindClass(clazz);
  jmethodID mid = jenv->GetMethodID(cls, "<init>", "(JZ)V");
  return jenv->NewObject(cls, mid, cptr, false);
}
%}

/*
  ptr   : either a csRef<type> or csPtr<type>
  name  : type name, e.g. "iEngine *"
  type  : type of pointer
  clazz : class name, e.g. "org/crystalspace3d/iEngine"

  In actual practice, 'ptr' is really of type SwigValueWrapper<csRef<T>> or
  SwigValueWrapper<csPtr<T>>.  The SwigValueWrapper wrapper is added
  automatically by Swig and is outside of our control.  SwigValueWrapper has a
  cast operator which will return csRef<T>& (or csPtr<T>&), which should allow
  us to assign a SwigValueWrapper to a csRef.  Unfortunately, unlike other
  compilers, however, MSVC considers assignment of SwigValueWrapper to csRef
  ambiguous, so we must manually invoke SwigValueWrapper's operator T& before
  actually assigning the value to a csRef.  This hack is noted by "* explicit
  cast *".
*/
%define TYPEMAP_OUT_csRef_BODY(ptr, name, type, wrapper, clazz)
  csRef<type> ref((wrapper<type>&)ptr); /* explicit cast */
  $result = _csRef_to_Java(csRef<iBase>(
    (type*)ref), (void*)(type*)ref, name, clazz, jenv);
%enddef

#undef TYPEMAP_OUT_csRef
%define TYPEMAP_OUT_csRef(T)
  %typemap(out) csRef<T>
  {
    TYPEMAP_OUT_csRef_BODY($1, #T " *", T, csRef, "org/crystalspace3d/" #T)
  }
  %typemap(jni) csRef<T> "jobject";
  %typemap(jtype) csRef<T> #T;
  %typemap(jstype) csRef<T> #T;
  %typemap(javain) csRef<T> "$javainput";
  %typemap(javaout) csRef<T> { return $jnicall; }
%enddef

#undef TYPEMAP_OUT_csPtr
%define TYPEMAP_OUT_csPtr(T)
  %typemap(out) csPtr<T>
  {
    TYPEMAP_OUT_csRef_BODY($1, #T " *", T, csPtr, "org/crystalspace3d/" #T)
  }
  //%typemap(out) csPtr<T> %{ $result = $1; %}
  %typemap(jni) csPtr<T> "jobject";
  %typemap(jtype) csPtr<T> #T;
  %typemap(jstype) csPtr<T> #T;
  %typemap(javain) csPtr<T> "$javainput";
  %typemap(javaout) csPtr<T> { return $jnicall; }
%enddef

#undef TYPEMAP_OUT_csWrapPtr
%define TYPEMAP_OUT_csWrapPtr
  %typemap(out) csWrapPtr
  {
    iBase * ibase = (iBase *)$1.Ref;
    void * ptr = ibase->QueryInterface(iSCF::SCF->GetInterfaceID($1.Type), $1.Version);
    ibase->DecRef(); // Undo IncRef from QueryInterface
    if (ptr == 0)
      $result = 0;
    else
    {
      jlong cptr = 0;
      *(void **)&cptr = ptr;
      char cls_name[1024];
      strcat(strcpy(cls_name, "org/crystalspace3d/"), $1.Type);
      jclass cls = jenv->FindClass(cls_name);
      jmethodID mid = jenv->GetMethodID(cls, "<init>", "(JZ)V");
      $result = jenv->NewObject(cls, mid, cptr, false);
    }
  }
  //%typemap(out) csWrapPtr %{ $result = $1; %}
  %typemap(jni) csWrapPtr "jobject";
  %typemap(jtype) csWrapPtr "Object";
  %typemap(jstype) csWrapPtr "Object";
  %typemap(javain) csWrapPtr "$javainput";
  //%typemap(javaout) csWrapPtr { return $jnicall; }
  %typemap(javaout) csWrapPtr
  {
    Object _obj = $jnicall;
    iBase ibase = (iBase) _obj;
    if (ibase != null)
      ibase.IncRef();
    return _obj;
  }
%enddef

#undef INTERFACE_EQUALS
%define INTERFACE_EQUALS
  public boolean equals (Object obj)
  {
    boolean equal = false;
    if (obj instanceof $javaclassname)
      equal = ((($javaclassname)obj).swigCPtr == this.swigCPtr);
    return equal;
  }
%enddef
#undef INTERFACE_APPLY
#define INTERFACE_APPLY(T) %typemap(javacode) T %{ INTERFACE_EQUALS %}
APPLY_FOR_EACH_INTERFACE

// ivaria/event.h
// Swig 1.3.23 introduces support for default arguments, so it generates this
// automatically. Prior versions do not.
#if (SWIG_VERSION < 0x010323)
#define IEVENTOUTLET_BROADCAST \
public void Broadcast (int iCode) { Broadcast(iCode, 0); }
#else
#define IEVENTOUTLET_BROADCAST
#endif

#undef IEVENTOUTLET_JAVACODE
%define IEVENTOUTLET_JAVACODE
%typemap(javacode) iEventOutlet
%{
  INTERFACE_EQUALS
  IEVENTOUTLET_BROADCAST
%}
%enddef
IEVENTOUTLET_JAVACODE

// iutil/cfgmgr.h
// Swig 1.3.21 (and possibly earlier) have a bug where enums are emitted as
// illegal Java code:
//   public final static int ConfigPriorityFoo = iConfigManager::PriorityFoo;
// rather than the legal code:
//   public final static int ConfigPriorityFoo = iConfigManager.PriorityFoo;
// We work around this by %ignoring those constants and defining them maually.
%ignore iConfigManager::ConfigPriorityPlugin;
%ignore iConfigManager::ConfigPriorityApplication;
%ignore iConfigManager::ConfigPriorityUserGlobal;
%ignore iConfigManager::ConfigPriorityUserApp;
%ignore iConfigManager::ConfigPriorityCmdLine;
#undef ICONFIGMANAGER_JAVACODE
%define ICONFIGMANAGER_JAVACODE
%typemap(javacode) iConfigManager
%{
  INTERFACE_EQUALS
  public final static int ConfigPriorityPlugin =
    iConfigManager.PriorityVeryLow;
  public final static int ConfigPriorityApplication =
    iConfigManager.PriorityLow;
  public final static int ConfigPriorityUserGlobal =
    iConfigManager.PriorityMedium;
  public final static int ConfigPriorityUserApp =
    iConfigManager.PriorityHigh;
  public final static int ConfigPriorityCmdLine =
    iConfigManager.PriorityVeryHigh;
%}
%enddef
ICONFIGMANAGER_JAVACODE

// Should probably be rewritten using csString for className
%typemap(in) (const char * iface, int iface_ver) (char className[1024])
{
    const char * s = jenv->GetStringUTFChars($input, 0);
    const char * dot = strrchr(s, '.');
    strcpy(className, "org/crystalspace3d/");
    strcat(className, dot?dot+1:s);
    $1 = className + sizeof("org/crystalspace3d/") - 1;
    jenv->ReleaseStringUTFChars($input, s);
    jclass cls = jenv->FindClass(className);
    jmethodID mid = jenv->GetStaticMethodID(cls, "scfGetVersion", "()I");
    $2 = jenv->CallStaticIntMethod(cls, mid);
}
%typemap(jni) (const char * iface, int iface_ver) "jstring"
%typemap(jtype) (const char * iface, int iface_ver) "String"
%typemap(jstype) (const char * iface, int iface_ver) "Class"
%typemap(javain) (const char * iface, int iface_ver) "$javainput.getName()"

// argc-argv handling
%typemap(in) (int argc, char const * const argv[])
{
  $1 = jenv->GetArrayLength($input) + 1; // +1 for synthesized argv[0].
  $2 = (char **) malloc(($1 + 1) * sizeof(char *));
  /* C/C++ functions accepting argc/argv[] expect argv[0] to be the program
     or script name, but Java's `main(String[] args)' array contains only
     program arguments. We must, therefore, prepend our own argv[0] to the
     incoming array. Unfortunately, there does not seem to be any way of
     determining the location of the .class file in which main() was invoked,
     so we instead just use "./csjava" as argv[0]. We purposely use the "./"
     notation so that functions, such as csGetAppPath(), which interpret
     argv[0] will consider the "current working directory" as the location of
     the program. (This may not be the best solution for synthesizing
     argv[0], but it is better than sending a bogus argv[] array to the C/C++
     function.)
  */
  $2[0] = strdup("./csjava");
  /* make a copy of each string */
  int i;
  for (i = 1; i < $1; ++i) {
    jstring j_string = (jstring)jenv->GetObjectArrayElement($input, i - 1);
    const char * c_string = jenv->GetStringUTFChars(j_string, 0);
    $2[i] = strdup(c_string);
    jenv->ReleaseStringUTFChars(j_string, c_string);
    jenv->DeleteLocalRef(j_string);
  }
  $2[i] = 0;
}

%typemap(freearg) (int argc, char const * const argv[])
{
  for (int i = 0; i < $1 - 1; ++i)
    free($2[i]);
  free($2);
}

%typemap(jni) (int argc, char const * const argv []) "jobjectArray"
%typemap(jtype) (int argc, char const * const argv []) "String[]"
%typemap(jstype) (int argc, char const * const argv []) "String[]"
%typemap(javain) (int argc, char const * const argv []) "$javainput"

#endif // SWIGJAVA
