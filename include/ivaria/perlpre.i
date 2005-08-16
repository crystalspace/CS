/*
    Copyright (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

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
 * Renaming operators is the first stage of wrapping them.
 * We ignore operator [] and () and unary *
 * since these will have to be wrapped manually.
 * We ignore operator &&= and ||=
 * since they have nothing to which to be wrapped
 ****************************************************************************/
%ignore			operator[];
%ignore			operator();
%ignore			operator* ();

%rename(__add__)	operator+;
%rename(__subtr__)	operator-;
%ignore			operator+ ();
%rename(__neg__)	operator- ();

%rename(__mult__)	operator*;
%rename(__div__)	operator/;
%rename(__modulo__)	operator%;
%rename(__lshift__)	operator<<;
%rename(__rshift__)	operator>>;
%rename(__and__)	operator&;
%rename(__or__)		operator|;
%rename(__xor__)	operator^;

%rename(__lt__)		operator<;
%rename(__le__)		operator<=;
%rename(__gt__)		operator>;
%rename(__ge__)		operator>=;
%rename(__eq__)		operator==;
%rename(__ne__)		operator!=;

%rename(__not__)	operator!;
%rename(__compl__)	operator~;
%rename(__inc__)	operator++;
%rename(__dec__)	operator--;

%rename(__copy__)	operator=;
%rename(__add_ass__)	operator+=;
%rename(__subtr_ass__)	operator-=;
%rename(__mult_ass__)	operator*=;
%rename(__divide_ass__)	operator/=;
%rename(__modulo_ass__)	operator%=;
%rename(__lshift_ass__)	operator<<=;
%rename(__rshift_ass__)	operator>>=;
%rename(__and_ass__)	operator&=;
%rename(__or_ass__)	operator|=;
%rename(__xor_ass__)	operator^=;

%rename(__land__)	operator&&;
%rename(__lor__)	operator||;

%ignore			operator&&=;
%ignore			operator||=;

// csgeom/vector3.h -- ignore (float); (double) overloads still visible
%ignore operator/ (const csVector3&, float);
%ignore operator* (const csVector3&, float);
%ignore operator* (float, const csVector3&);

/****************************************************************************
 * Applying this Perl code is the second and final stage of wrapping
 * operator overloads. It is commented out since Swig doesn't yet have a
 * %perlcode directive.
 ****************************************************************************/
#if 0
%perlcode %{
  use overload (
    'abs'	=> '__abs__',
    'bool'	=> '__bool__',
    '""'	=> '__string__',
    '0+'	=> '__numer__',

    '+'		=> '__add__',
    '-'		=> '__subtr__',
    '*'		=> '__mult__',
    '/'		=> '__div__',
    '%'		=> '__modulo__',
    '**'	=> '__pow__',

    '<<'	=> '__lshift__',
    '>>'	=> '__rshift__',
    '&'		=> '__and__',
    '|'		=> '__or__',
    '^'		=> '__xor__',

    '+='	=> '__add_ass__',
    '-='	=> '__subtr_ass__',
    '*='	=> '__mult_ass__',
    '/='	=> '__div_ass__',
    '%='	=> '__modulo_ass__',
    '**='	=> '__pow_ass__',
    '<<='	=> '__lshift_ass__',
    '>>='	=> '__rshift_ass__',
    '&='	=> '__and_ass__',
    '|='	=> '__or_ass__',
    '^='	=> '__xor_ass__',

    '<'		=> '__lt__',
    '<='	=> '__le__',
    '>'		=> '__gt__',
    '>='	=> '__ge__',
    '=='	=> '__eq__',
    '!='	=> '__ne__',

    'lt'	=> '__slt__',
    'le'	=> '__sle__',
    'gt'	=> '__sgt__',
    'ge'	=> '__sge__',
    'eq'	=> '__seq__',
    'ne'	=> '__sne__',

    '!'		=> '__not__',
    '~'		=> '__compl__',
    '++'	=> '__inc__',
    '--'	=> '__dec__',

    'x'		=> '__repeat__',
    '.'		=> '__concat__',
    'x='	=> '__repeat_ass__',
    '.='	=> '__concat_ass__',
    '='		=> '__copy__',

    'neg'	=> '__neg__',

    '${}'	=> '__sv__',
    '@{}'	=> '__av__',
    '%{}'	=> '__hv__',
    '*{}'	=> '__gv__',
    '&{}'	=> '__cv__',

    '<>'	=> '__iter__'
  );

  *and = *__land__;
  *or = *__lor__;
%}
#endif // 0

/****************************************************************************
 * Fix wrapping of int8 so that Perl uses an int instead of a length-1 string.
 ****************************************************************************/
%typemap(in) int8
{
  $1 = SvIV ($input);
}
%typemap(out) int8
{
  $result = newSViv ($1);
}

/****************************************************************************
 * Allow event key characters to be given as strings of length 1. Otherwise,
 * they would have to be passed as ASCII integers.
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
%define TYPEMAP_OUT_csRef_BODY(result, pT, cT)
  if (rf.IsValid ())
  {
    rf->IncRef ();
    SV* rv = sv_newmortal();
    SWIG_MakePtr(rv, (cT*)rf, SWIG_TypeQuery(pT), 0);
    result = rv;
  }
  else
  {
    SvREFCNT_inc (& PL_sv_undef);
    result = & PL_sv_undef;
  }
  argvi++;
%enddef

#undef TYPEMAP_OUT_csRef
%define TYPEMAP_OUT_csRef(T)
  %typemap(out) csRef<T>
  {
    csRef<T> rf ((csRef<T>&)$1); /* explicit cast */
    TYPEMAP_OUT_csRef_BODY ($result, "cspace::" #T, T)
  }
%enddef

#undef TYPEMAP_OUT_csPtr
%define TYPEMAP_OUT_csPtr(T)
  %typemap(out) csPtr<T>
  {
    csRef<T> rf ((csPtr<T>&)$1); /* explicit cast */
    TYPEMAP_OUT_csRef_BODY ($result, "cspace::" #T, T)
  }
%enddef

#undef TYPEMAP_OUT_csWrapPtr
%define TYPEMAP_OUT_csWrapPtr
  %typemap(out) csWrapPtr
  {
    csRef<iBase> rf ($1.Ref);
    if (rf.IsValid ())
    {
      csString pT; pT << "cspace::" << $1.Type;
      iBase* ibase = rf;
      void* ptr = ibase->QueryInterface(iSCF::SCF->GetInterfaceID($1.Type), $1.Version);
      SV* rv = sv_newmortal();
      SWIG_MakePtr(rv, ptr, SWIG_TypeQuery(pT.GetData()), 0);
      $result = rv;
    }
    else
    {
      SvREFCNT_inc (& PL_sv_undef);
      $result = & PL_sv_undef;
    }
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
    for (int i = 0; i < $1.Length (); i++)
    {
      SV *ae;
      TYPEMAP_OUT_csRef_BODY (ae, "cspace::" #T, T)
      av_push (av, ae);
      SvREFCNT_dec (ae);
    }
    $result = newRV_noinc ((SV *) av);
  }
%enddef

/****************************************************************************
 * Typemaps to convert csArray to a Perl array and vice versa.
 ****************************************************************************/
%define _TYPEMAP_csArray(T, toSV, fromSV)
  %typemap(out) csArray<T>
  {
    AV *av = newAV ();
    for (int i = 0; i < $1.Length (); i++)
    {
      SV *ae = toSV ($1.Get (i));
      av_push (av, ae);
      SvREFCNT_dec (ae);
    }
    $result = newRV_noinc ((SV *) av);
  }
  %typemap(in) csArray<T>
  {
    AV *av = (AV *) SvRV ($input);
    if (SvTYPE (av) != SVt_PVAV)
      croak ("%s", "Argument must be an array reference");
    if (av_len (av) >= 0)
    {
      for (int i = 0; i <= av_len (av); i++)
      {
        SV *sv = av_fetch (av, i, 0);
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
    for (int i = 0; i < $1.Length (); i++)
    {
      SV *rv = newSViv (0);
      sv_setref_iv (rv, "cspace::" #T, (int) (void *) & $1.Get (i));
      av_push (av, rv);
      SvREFCNT_dec (ae);
    }
    $result = newRV_noinc ((SV *) av);
  }
  %typemap(in) csArray<T>
  {
    AV *av = (AV *) SvRV ($input);
    if (SvTYPE (av) != SVt_PVAV)
      croak ("%s", "Argument must be an array reference");
    if (av_len (av) >= 0)
      for (int i = 0; i <= av_len (av); i++)
      {
        SV *sv = av_fetch (av, i, 0);
        if (! sv_isa (sv, "cspace::" #T))
          croak ("All elements of array must be cspace::%s", #T);
        T *v = (T *) SvIV (SvRV (sv));
        $1.Push (* v);
      }
  }
%enddef

/****************************************************************************
 * Retrieve the version number of an interface which is provided as a string.
 * Used by the typemap below.
 ****************************************************************************/
%{
  int scfGetVersion (const char *iface)
  {
    dSP;
    int ver;
    int nresult;
    csString var;
    var << "cspace::" << iface << "::scfGetVersion";

    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    PUTBACK;
    nresult = call_pv(var.GetData(), G_SCALAR);
    SPAGAIN;

    if (nresult != 1)
      croak("Expected exactly one result from scfGetVersion()\n");

    ver = POPi;

    PUTBACK;
    FREETMPS;
    LEAVE;

    return ver;
  }
%}

/****************************************************************************
 * Typemaps to convert an interface and version from an interface name.
 ****************************************************************************/
%typemap(in) (const char * iface, int iface_ver)
{
  // SvPV_nolen is a macro that can't be used in general expression so we
  // declare a temp variable.  The temp variable is non-const because SWIG
  // declares $1 to be non-const for some reason.
  char *inputString;
  inputString = SvPV_nolen($input);
  $1 = inputString;
  $2 = scfGetVersion(inputString);
}

/****************************************************************************
 * Typemaps to convert an argc/argv pair to a Perl array.
 ****************************************************************************/
%typemap(in) (int argc, char const* const argv[])
{
  // Convert incoming Perl array reference to argc/argv[].  Note that we
  // manually determine and prepend the script name to the incoming @ARGV array
  // since C functions expect argv[0] to be the program name, whereas the Perl
  // @ARGV array is filled only with script arguments.
  AV *av = (AV *) SvRV ($input);
  if (SvTYPE (av) != SVt_PVAV)
    croak ("%s", "Argument must be an array reference");
  $1 = av_len (av) + 2; // +1 to get actual array length; +1 for script name
  $2 = new (char*)[$1];
  $2[0] = SvPV_nolen(get_sv("0", 0));
  for (int i = 1; i < $1; i++)
  {
    SV **sv = av_fetch (av, i - 1, 0);
    $2[i] = SvPV_nolen (*sv);
  }
}

%typemap(freearg) (int argc, char const* const argv[])
{
  delete[] $2;

  // Let caller know that we consumed the entire array (i.e. we `shifted'
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
    base_type *item = new base_type (to_item ptr[i]);
    SV *o = newSVsv (& PL_sv_undef);
    SvREFCNT_dec (sv_setref_iv (o, #base_type, (int) item));
    av_push (av, o);
    SvREFCNT_dec (o);
  }
  $result = newRV ((SV *) av);
  AvREFCNT_dec (av);
%enddef

#undef TYPEMAP_OUTARG_ARRAY_CNT_PTR
%define TYPEMAP_OUTARG_ARRAY_CNT_PTR(pattern, ptr_init, to_item)
  %typemap(in, numinputs = 0) pattern ($2_type ptr, $1_basetype cnt)
  {
    $1 = & cnt;
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
    $2 = & cnt;
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
  AV *av = (AV *) SvRV ($input);
  if (SvTYPE (av) != SVt_PVAV)
    croak ("%s", "Argument must be an array reference");
  cnt = av_len (av) + 1;
  ptr = new array_type [cnt];
  for (int i = 0; i < cnt; i++)
  {
    SV *oref = av_shift (av);
    SV *o = SvRV (oref);
    SvREFCNT_dec (oref);
    if (! sv_isa (o, #base_type))
    {
      croak ("%s", "Array must contain " #base_type "'s");
      delete [] ptr;
      return;
    }
    base_type *p = (base_type *) SvIV (o);
    SvREFCNT_dec (o);
    ptr [i] = to_item p;
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
 * In perlpost.i, create an scfInitialize function in Perl which grabs the
 * program path automatically, and remove the C argc/argv version here.
 ****************************************************************************/
%ignore scfInitialize(int argc, const char * const argv []);

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
%ignore iThingFactoryState::AddPolygon(int, ...);

#endif // SWIGPERL5
