/*
    Copyright (C) 2003, 2007 Mat Sutcliffe <oktal@gmx.co.uk>

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

#ifdef SWIGPERL5

#include <csutil/csstring.h>

/****************************************************************************
 * Fill the following to extend csArray, csSet and iFooList interfaces.
 ****************************************************************************/
#undef ARRAY_OBJECT_FUNCTIONS
%define ARRAY_OBJECT_FUNCTIONS(classname,typename)
%enddef
#undef SET_OBJECT_FUNCTIONS
%define SET_OBJECT_FUNCTIONS(classname,typename)
%enddef
#undef LIST_OBJECT_FUNCTIONS
%define LIST_OBJECT_FUNCTIONS(classname,typename)
%enddef

/****************************************************************************
 * Ignore the operator overloads that Swig cannot handle.
 * Swig can only wrap +, -, *, /, %, ++, --, ==, !=, <, >, && and ||.
 ****************************************************************************/
%ignore operator<<;
%ignore operator>>;
%ignore operator=;
%ignore operator+=;
%ignore operator-=;
%ignore operator*=;
%ignore operator/=;
%ignore operator%=;
%ignore operator* ();
%ignore operator[];
%ignore operator();
%ignore operator<=;
%ignore operator>=;
%ignore operator&;
%ignore operator|;
%ignore operator^;
%ignore operator!;
%ignore operator~;
%ignore operator<<=;
%ignore operator>>=;
%ignore operator&=;
%ignore operator|=;
%ignore operator^=;
// SWIG 1.3.33 reports a syntax error here
//%ignore operator&&=;
//%ignore operator||=;
%ignore operator->;
%ignore operator->*;
%ignore operator,;
%ignore operator new;
%ignore operator new[];
%ignore operator delete;
%ignore operator delete[];

// csgeom/vector3.h -- ignore (float); (double) overloads still visible
%ignore operator/ (const csVector3&, float);
%ignore operator* (const csVector3&, float);
%ignore operator* (float, const csVector3&);

// Swig cannot wrap operator overloads that are friend functions, so we ignore
// warnings about them.
%warnfilter(503) operator+;
%warnfilter(503) operator-;
%warnfilter(503) operator*;
%warnfilter(503) operator/;
%warnfilter(503) operator%;
%warnfilter(503) operator++;
%warnfilter(503) operator--;
%warnfilter(503) operator==;
%warnfilter(503) operator!=;
%warnfilter(503) operator<;
%warnfilter(503) operator>;
%warnfilter(503) operator&&;
%warnfilter(503) operator||;

/****************************************************************************
 * Wrap int8 so that Perl uses an int instead of a one-character string.
 ****************************************************************************/
%typemap(in) int8
{
  $1 = SvIV ($input);
}
%typemap(out) int8
{
  $result = sv_2mortal (newSViv ($1));
}

/****************************************************************************
 * Allow characters for keyboard events to be given as one-character strings.
 * Otherwise, they would have to be passed as integer ASCII codes.
 ****************************************************************************/
%typemap(out) int Char
{
  static char event_key_char[2] = " ";
  event_key_char[0] = (char) $1;
  $result = newSVpv (event_key_char, 0);
}
%typemap(in) int Char
{
  $1 = (int) * SvPV_nolen ($input);
}
%apply(int Char) { int iChar };
%apply(int Char) { int kchar };

/****************************************************************************
 * Define typemaps to get the pointers out of csRef, csPtr and csWrapPtr.
 *
 * Note that in actual practice in TYPEMAP_OUT_csRef and TYPEMAP_OUT_csPtr, $1
 * is really of type SwigValueWrapper<csRef<T>> or SwigValueWrapper<csPtr<T>>.
 * The SwigValueWrapper wrapper is added automatically by Swig and is outside
 * of our control.  SwigValueWrapper has a cast operator which will return
 * csRef<T>& (or csPtr<T>&), which should allow us to assign a SwigValueWrapper
 * to a csRef.  Unfortunately, unlike other compilers, however, MSVC considers
 * assignment of SwigValueWrapper to csRef<> ambiguous, so we must manually
 * invoke SwigValueWrapper's operator T& before actually assigning the value to
 * a csRef.  This hack is noted by "* explicit cast *".
 ****************************************************************************/
%define TYPEMAP_OUT_csRef_BODY(result, T)
  if (rf.IsValid ())
  {
    // SWIG_SHADOW | SWIG_OWNER tells Swig that it is taking ownership of a
    // reference to the object and should therefore DecRef it when its finished
    SWIG_MakePtr (result, (T*) rf, $descriptor(T*), SWIG_SHADOW | SWIG_OWNER);
    rf->IncRef ();
  }
  else SvSetSV (result, & PL_sv_undef);
%enddef

#undef TYPEMAP_OUT_csRef
%define TYPEMAP_OUT_csRef(T)
  %typemap(out) csRef<T>
  {
    csRef<T> rf ((csRef<T>&)$1); /* explicit cast */
    $result = sv_newmortal ();
    TYPEMAP_OUT_csRef_BODY ($result, T)
    argvi++;
  }
%enddef

#undef TYPEMAP_OUT_csPtr
%define TYPEMAP_OUT_csPtr(T)
  %typemap(out) csPtr<T>
  {
    csRef<T> rf ((csPtr<T>&)$1); /* explicit cast */
    $result = sv_newmortal ();
    TYPEMAP_OUT_csRef_BODY ($result, T)
    argvi++;
  }
%enddef

#undef TYPEMAP_OUT_csWrapPtr
%define TYPEMAP_OUT_csWrapPtr
  %typemap(out) csWrapPtr
  {
    $result = sv_newmortal();
    csRef<iBase> rf ($1.Ref);
    if (rf.IsValid ())
    {
      csString pT; pT << $1.Type << " *";
      void* ptr = rf->QueryInterface(iSCF::SCF->GetInterfaceID($1.Type), $1.Version);
      SWIG_MakePtr ($result, ptr, SWIG_TypeQuery (pT.GetData ()),
	SWIG_SHADOW | SWIG_OWNER);
    }
    else SvSetSV ($result, & PL_sv_undef);
    argvi++;
  }
%enddef

/****************************************************************************
 * Typemap to get the pointers out of csRefArray and convert to a Perl array.
 ****************************************************************************/
#undef TYPEMAP_OUT_csRefArray
%define TYPEMAP_OUT_csRefArray(T)
  %typemap(out) csRefArray<T>
  {
    AV *av = newAV ();
    for (unsigned i = 0, size = $1.GetSize (); i < size; i++)
    {
      csRef<T> rf ($1.Get (i));
      SV *ae = newSViv (0);
      TYPEMAP_OUT_csRef_BODY (ae, T)
      av_push (av, ae);
    }
    $result = sv_2mortal (newRV_noinc ((SV *) av));
    argvi++;
  }
%enddef

/****************************************************************************
 * Typemaps to convert csArray to a Perl array and vice versa.
 ****************************************************************************/
%define _TYPEMAP_csArray(T, toSV, fromSV)
  %typemap(out) csArray<T>
  {
    AV *av = newAV ();
    for (unsigned i = 0, size = $1.GetSize (); i < size; i++)
    {
      SV *ae = toSV ($1.Get (i));
      av_push (av, ae);
    }
    $result = sv_2mortal (newRV_noinc ((SV *) av));
    argvi++;
  }
  %typemap(in) csArray<T>
  {
    if (! SvROK ($input)) croak ("Argument must be an array reference");
    AV *av = (AV *) SvRV ($input);
    if (SvTYPE (av) != SVt_PVAV) croak ("Argument must be an array reference");
    if (av_len (av) >= 0)
    {
      for (int i = 0, last = av_len (av); i <= last; i++)
      {
        SV *sv = * av_fetch (av, i, 0);
        $1.Push (fromSV (sv));
      }
    }
  }
%enddef

_TYPEMAP_csArray(int,			newSViv,	SvIV)
_TYPEMAP_csArray(short,			newSViv,	SvIV)
_TYPEMAP_csArray(long,			newSViv,	SvIV)
_TYPEMAP_csArray(unsigned int,		newSVuv,	SvUV)
_TYPEMAP_csArray(unsigned long,		newSVuv,	SvUV)
_TYPEMAP_csArray(unsigned short,	newSVuv,	SvUV)
_TYPEMAP_csArray(float,			newSVnv,	SvNV)
_TYPEMAP_csArray(double,		newSVnv,	SvNV)

#undef TYPEMAP_csArray
%define TYPEMAP_csArray(T)
  %typemap(out) csArray<T>
  {
    AV *av = newAV ();
    for (unsigned i = 0, size = $1.GetSize (); i < size; i++)
    {
      SV *ae = newSViv (0);
      void *ptr = & $1.Get (i);
      SWIG_MakePtr (ae, ptr, $descriptor(T*), 0);
      av_push (av, ae);
    }
    $result = sv_2mortal (newRV_noinc ((SV *) av));
    argvi++;
  }
  %typemap(in) csArray<T>
  {
    if (! SvROK ($input)) croak ("Argument must be an array reference");
    AV *av = (AV *) SvRV ($input);
    if (SvTYPE (av) != SVt_PVAV) croak ("Argument must be an array reference");
    if (av_len (av) >= 0)
      for (int i = 0, last = av_len (av); i <= last; i++)
      {
        SV *sv = * av_fetch (av, i, 0);
        void *ptr;
        int status = SWIG_ConvertPtr (sv, &ptr, $input_descriptor, 0);
        if (! SWIG_IsOK (status))
          croak ("All elements of array must be instances of cspace::%s", #T);
        $1.Push (* (T*) ptr);
      }
  }
%enddef

/****************************************************************************
 * Get the version number of an SCF interface named in a string, by calling
 * the class method scfGetVersion in the corresponding Perl package.
 * Used by the typemap below.
 ****************************************************************************/
%{
  int scfGetVersion (const char *iface)
  {
    csString sub;
    sub << "cspace::" << iface << "::scfGetVersion";

    dSP;
    ENTER;
    SAVETMPS;

    PUSHMARK (SP);
    PUTBACK;

    int n = call_pv (sub.GetData (), G_SCALAR);

    SPAGAIN;
    if (n != 1) croak ("Expected 1 result from %s::scfGetVersion", iface);
    int version = POPi;
    PUTBACK;

    FREETMPS;
    LEAVE;

    return version;
  }
%}

/****************************************************************************
 * Typemaps to convert an interface and version from an interface name.
 * Used by wrapped versions of scfQueryInterface, csQueryRegistry, etc.
 ****************************************************************************/
%typemap(in) (const char *iface, int iface_ver)
{
  char *inputString;
  inputString = SvPV_nolen ($input);
  $1 = inputString;
  $2 = scfGetVersion (inputString);
}

/****************************************************************************
 * Typemaps to convert an argc/argv pair to a Perl array.
 ****************************************************************************/
%typemap(in) (int argc, char const* const argv[])
{
  if (! SvROK ($input)) croak ("Argument must be an array reference");
  AV *av = (AV *) SvRV ($input);
  if (SvTYPE (av) != SVt_PVAV) croak ("Argument must be an array reference");
  $1 = av_len (av) + 1;
  $2 = new (char*)[$1];
  for (int i = 0; i < $1; i++)
  {
    SV *sv = * av_fetch (av, i, 0);
    $2[i] = SvPV_nolen (sv);
  }
}

%typemap(freearg) (int argc, char const* const argv[])
{
  delete[] $2;

  // Let caller know that we consumed the entire array (i.e. we 'shifted'
  // all elements). It is safe to do this only after all usage of $2[]
  // since $2[] contains live references to strings in $input.
  av_clear((AV*)SvRV($input));
}

/****************************************************************************
 * Typemap to handle format/vararg input functions.
 ****************************************************************************/
%typemap(in) (const char * description, ...)
{
  $1 = "%s";
  $2 = SvPV_nolen ($input);
}
%apply(const char * description, ...) { (const char * format, ...) };
%apply(const char * description, ...) { (const char * fmt, ...) };

/****************************************************************************
 * Typemaps to handle arrays.
 ****************************************************************************/
#undef TYPEMAP_OUTARG_ARRAY_BODY
%define TYPEMAP_OUTARG_ARRAY_BODY(array_type, base_type, cnt, ptr, to_item)
  AV *av = newAV ();
  for (int i = 0; i < cnt; i++)
  {
    base_type *item = new base_type (to_item (ptr[i]));
    SV *sv = newSViv (0);
    // SWIG_SHADOW | SWIG_OWNER tells Swig that it is taking ownership of the
    // object and should therefore delete it when Perl releases the SV.
    SWIG_MakePtr (sv, item, $descriptor(base_type*), SWIG_SHADOW | SWIG_OWNER);
    av_push (av, sv);
  }
  $result = sv_2mortal (newRV_noinc ((SV *) av));
  argvi++;
%enddef

#undef TYPEMAP_OUTARG_ARRAY_CNT_PTR
%define TYPEMAP_OUTARG_ARRAY_CNT_PTR(pattern, ptr_init, to_item)
  %typemap(in, numinputs = 0) pattern ($2_type ptr, $1_basetype cnt)
  {
    $1 = cnt;
    $2 = ($2_type) ptr_init;
  }
  %typemap(outarg) pattern
  {
    TYPEMAP_OUTARG_ARRAY_BODY ($*2_type, $2_basetype, $1, $2, to_item)
  }
  %typemap(freearg) pattern
  {
    delete [] $2;
  }
%enddef

#undef TYPEMAP_OUTARG_ARRAY_PTR_CNT
%define TYPEMAP_OUTARG_ARRAY_PTR_CNT(pattern, ptr_init, to_item)
  %typemap(in, numinputs = 0) pattern ($1_type ptr, $2_basetype cnt)
  {
    $1 = ($1_type) ptr_init;
    $2 = cnt;
  }
  %typemap(outarg) pattern
  {
    TYPEMAP_OUTARG_ARRAY_BODY ($*1_type, $1_basetype, $2, $1, to_item)
  }
  %typemap(freearg) pattern
  {
    delete [] $1;
  }
%enddef

#undef TYPEMAP_IN_ARRAY_BODY
%define TYPEMAP_IN_ARRAY_BODY(array_type, base_type, cnt, ptr, to_item)
  if (! SvROK ($input)) croak ("Argument must be an array reference");
  AV *av = (AV *) SvRV ($input);
  if (SvTYPE (av) != SVt_PVAV) croak ("Argument must be an array reference");
  cnt = av_len (av) + 1;
  ptr = new array_type [cnt];
  for (int i = 0; i < cnt; i++)
  {
    SV *ae = * av_fetch (av, i, 0);
    void *p;
    int status = SWIG_ConvertPtr (ae, &p, $descriptor(base_type*), 0);
    if (! SWIG_IsOK (status))
    {
      croak ("All elements of array must be instances of %s", #base_type);
      delete [] ptr;
      return;
    }
    ptr [i] = to_item (base_type *) p;
  }
%enddef

#undef TYPEMAP_IN_ARRAY_CNT_PTR
%define TYPEMAP_IN_ARRAY_CNT_PTR(pattern, to_item)
  %typemap(in) pattern
  {
    TYPEMAP_IN_ARRAY_BODY ($*2_type, $2_basetype, $1, $2, to_item)
  }
  %typemap(freearg) pattern
  {
    delete [] $2;
  }
%enddef

#undef TYPEMAP_IN_ARRAY_PTR_CNT
%define TYPEMAP_IN_ARRAY_PTR_CNT(pattern, to_item)
  %typemap(in) pattern
  {
    TYPEMAP_IN_ARRAY_BODY ($*1_type, $1_basetype, $2, $1, to_item)
  }
  %typemap(freearg) pattern
  {
    delete [] $1;
  }
%enddef

/****************************************************************************
 * Modern versions of Swig generate garbage Perl C interface code when a class
 * has both static and instance methods with the same name. In addition to
 * referencing non-existent variables (such as argv[n]), the flow and logic of
 * the generated code is entirely quite corrupt.  It will also generate garbage
 * code if one overload of an instance method accepts a variable number of
 * arguments.  We work around the problem by renaming one of the methods; in
 * the case of class versus instance method, we choose always to rename the
 * class method.
 ****************************************************************************/
%rename (ClassifyPolygon) Classify(const csPlane3&, csVector3*, int);
%rename (ComputePolygonNormal) ComputeNormal(csVector3*, int);
%rename (ComputePolygonPlane) ComputePlane(csVector3*, int);

#endif // SWIGPERL5
