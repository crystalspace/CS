/*
    Copyright (C) 2002 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CSPERL5_CSPERL5_H__
#define __CSPERL5_CSPERL5_H__

#include "ivaria/script.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"
#include "csutil/scfstr.h"

#include <string.h>
#include <stdarg.h>

#undef MIN
#undef MAX
#include <EXTERN.h>
#include <perl.h>
#undef free
#undef malloc
#undef Copy
#undef MAXXCOUNT
#undef MAXY_SIZE
#undef MAXYCOUNT
#undef Move
#undef New
#undef Newc
#undef Newz
#undef Renew
#undef Renewc
#undef Safefree
#undef StructCopy
#undef Zero

struct iObjectRegistry;

SCF_VERSION (csPerl5Object, 0, 0, 1);
struct csPerl5Object : public iScriptObject {};

class csPerl5 : public iScript
{
  class Object : public csPerl5Object
  {
    csPerl5 *parent;
    PerlInterpreter *my_perl;

  protected:
    SV *self;
    friend class csPerl5;

  public:
    SCF_DECLARE_IBASE;

    Object (csPerl5 *p, SV *s);
    virtual ~Object ();

    bool IsType (const char *type) const
    { return sv_isa (self, type) || sv_derived_from (self, type); }

    bool Call (const char *name, const char *fmt, ...)
    {
      va_list va;
      va_start (va, fmt);
      SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va);
      if (sv)
	SvREFCNT_dec (sv);
      return (sv != 0);
    }
    bool Call (const char *name, int &ret, const char *fmt, ...)
    {
      va_list va;
      va_start (va, fmt);
      SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va);
      if (sv)
      {
	ret = SvIV (sv);
	SvREFCNT_dec (sv);
      }
      return (sv != 0);
    }
    bool Call (const char *name, float &ret, const char *fmt, ...)
    {
      va_list va;
      va_start (va, fmt);
      SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va);
      if (sv)
      {
	ret = SvNV (sv);
	SvREFCNT_dec (sv);
      }
      return sv;
    }
    bool Call (const char *name, double &ret, const char *fmt, ...)
    {
      va_list va;
      va_start (va, fmt);
      SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va);
      if (sv)
      {
	ret = SvNV (sv);
	SvREFCNT_dec (sv);
      }
      return sv;
    }
    bool Call (const char *name, csRef<iString>& ret, const char *fmt, ...)
    {
      va_list va;
      va_start (va, fmt);
      SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va);
      if (sv)
      {
	ret.AttachNew(new scfString(SvPV_nolen (sv)));
	SvREFCNT_dec (sv);
      }
      return sv;
    }
    bool Call(const char *name, csRef<iScriptObject> &ret, const char *fmt,...)
    {
      va_list va;
      va_start (va, fmt);
      SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va);
      if (sv)
      {
	ret.AttachNew(new Object (parent, sv));
	SvREFCNT_dec (sv);
      }
      return sv;
    }

    bool Set (const char *name, int data)
    { return Call ("STORE", "%s%d", name, data); }
    bool Set (const char *name, float data)
    { return Call ("STORE", "%s%f", name, data); }
    bool Set (const char *name, double data)
    { return Call ("STORE", "%s%lf", name, data); }
    bool Set (const char *name, char const *data)
    { return Call ("STORE", "%s%s", name, data); }
    bool Set (const char *name, iScriptObject *data)
    { return Call ("STORE", "%s%p", name, data); }
    bool SetTruth (const char *name, bool data)
    { return Call ("STORE", "%s%d", name, data ? 1 : 0); }
    bool SetPointer (iBase *data)
    {
      SV *p = SvRV (self);
      if (!SvOK (p))
	return false;
      sv_setiv (p, (int) data);
      return true;
    }

    bool Get (const char *name, int &data) const
    { return ((Object *) this)->Call ("FETCH", data, "%s", name); }
    bool Get (const char *name, float &data) const
    { return ((Object *) this)->Call ("FETCH", data, "%s", name); }
    bool Get (const char *name, double &data) const
    { return ((Object *) this)->Call ("FETCH", data, "%s", name); }
    bool Get (const char *name, csRef<iString> &data) const
    { return ((Object *) this)->Call ("FETCH", data, "%s", name); }
    bool Get (const char *name, csRef<iScriptObject> &data) const
    { return ((Object *) this)->Call ("FETCH", data, "%s", name); }
    bool GetTruth (const char *name, bool &data) const
    {
      int n;
      bool ok = ((Object *) this)->Call ("FETCH", n, "%s", name);
      if (ok)
	data = (bool)n;
      return ok;
    }
    iBase* GetPointer () const
    {
      SV *p = SvRV (self);
      if (SvOK (p))
	return (iBase *) SvIV (p);
      return 0;
    }
  };
  friend class Object;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPerl5);
    bool Initialize (iObjectRegistry *o) { return scfParent->Init (o); }
  } scfiComponent;
  friend struct eiComponent;

protected:
  PerlInterpreter *my_perl;

  csRef<iReporter> reporter;

  bool Init (iObjectRegistry *);
  bool CheckError (const char *caller) const;
  Object* Query (iScriptObject *obj) const;
  // Caller is responsible for decref'ing SV returned from CallV(0.
  SV* CallV (const char *name, const char *fmt, va_list va, SV *self = 0);
  SV* CallV (const char *name, const char *fmt, va_list va, const char *self)
  { return CallV (name, fmt, va, sv_2mortal (newSVpv (self, 0))); }

public:
  SCF_DECLARE_IBASE;

  csPerl5 (iBase *);
  virtual ~csPerl5 ();

  bool Initialize (iObjectRegistry *);
  bool Store (const char *name, void *data, void *tag);

  bool RunText (const char *);
  bool LoadModule (const char *);

  csRef<iScriptObject> NewObject (const char *type, const char *fmt, ...);

  bool Call (const char *name, const char *fmt, ...)
  {
    va_list va;
    va_start (va, fmt);
    SV *sv = CallV (name, fmt, va);
    va_end (va);
    if (sv)
      SvREFCNT_dec (sv);
    return sv;
  }
  bool Call (const char *name, int &ret, const char *fmt, ...)
  {
    va_list va;
    va_start (va, fmt);
    SV *sv = CallV (name, fmt, va);
    va_end (va);
    if (sv)
    {
      ret = SvIV (sv);
      SvREFCNT_dec (sv);
    }
    return sv;
  }
  bool Call (const char *name, float &ret, const char *fmt, ...)
  {
    va_list va;
    va_start (va, fmt);
    SV *sv = CallV (name, fmt, va);
    va_end (va);
    if (sv)
    {
      ret = SvNV (sv);
      SvREFCNT_dec (sv);
    }
    return sv;
  }
  bool Call (const char *name, double &ret, const char *fmt, ...)
  {
    va_list va;
    va_start (va, fmt);
    SV *sv = CallV (name, fmt, va);
    va_end (va);
    if (sv)
    {
      ret = SvNV (sv);
      SvREFCNT_dec (sv);
    }
    return sv;
  }
  bool Call (const char *name, csRef<iString> &ret, const char *fmt, ...)
  {
    va_list va;
    va_start (va, fmt);
    SV *sv = CallV (name, fmt, va);
    va_end (va);
    if (sv)
    {
      ret.AttachNew(new scfString(SvPV_nolen (sv)));
      SvREFCNT_dec (sv);
    }
    return sv;
  }
  bool Call (const char *name, csRef<iScriptObject> &ret, const char *fmt, ...)
  {
    va_list va;
    va_start (va, fmt);
    SV *sv = CallV (name, fmt, va);
    va_end (va);
    if (sv)
    {
      ret.AttachNew(new Object (this, sv));
      SvREFCNT_dec (sv);
    }
    return sv;
  }

  bool Store (const char *name, int data)
  { SV *sv = get_sv (name, TRUE); sv_setiv (sv, data); return true; }
  bool Store (const char *name, float data)
  { SV *sv = get_sv (name, TRUE); sv_setnv (sv, data); return true; }
  bool Store (const char *name, double data)
  { SV *sv = get_sv (name, TRUE); sv_setnv (sv, data); return true; }
  bool Store (const char *name, char const *data)
  { SV *sv = get_sv (name, TRUE); sv_setpv (sv, data); return true; }
  bool Store (const char *name, iScriptObject *data)
  {
    SV *sv = get_sv (name, TRUE);
    sv_setsv (sv, Query(data)->self);
    return true;
  }
  bool SetTruth (const char *name, bool data)
  { SV *sv = get_sv (name, TRUE); sv_setiv (sv, data ? 1 : 0); return true; }

  bool Retrieve (const char *name, int &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvIV (sv); return sv; }
  bool Retrieve (const char *name, float &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvNV (sv); return sv; }
  bool Retrieve (const char *name, double &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvNV (sv); return sv; }
  bool Retrieve (const char *name, csRef<iString> &data) const
  {
    SV *sv = get_sv (name, FALSE);
    if (sv)
      data.AttachNew(new scfString(SvPV_nolen (sv)));
    return sv;
  }
  bool Retrieve (const char *name, csRef<iScriptObject> &data) const
  {
    SV *sv = get_sv (name, FALSE);
    if (sv)
      data.AttachNew(new Object (CS_CONST_CAST(csPerl5*,this), sv));
    return sv;
  }
  bool GetTruth (const char *name, bool &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvTRUE (sv); return sv; }

  bool Remove (const char *name)
  {
    SV *sv = get_sv (name, FALSE);
    if (sv)
      sv_setsv(sv, &PL_sv_undef);
    return sv;
  }
};

#endif
