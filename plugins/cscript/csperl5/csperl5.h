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

#include <string.h>
#include <stdarg.h>

#undef MIN
#undef MAX
#include <EXTERN.h>
#include <perl.h>
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

SCF_VERSION (Object, 0, 0, 1); // so we can query for our private object

struct iObjectRegistry;

class csPerl5 : public iScript
{
  PerlInterpreter *my_perl;

  class Object : public iScriptObject
  {
    csPerl5 *parent;
    char *type;
    HV *stash;

  protected:
    SV *self;
    friend class csPerl5;

  public:
    SCF_DECLARE_IBASE;

    Object (const csPerl5 *p, const char *t, SV *s);
    virtual ~Object ();

    virtual const char* GetType () const { return type; }

    bool Call (const char *name, const char *fmt, ...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) SvREFCNT_dec (sv); return sv; }
    bool Call (const char *name, int &ret, const char *fmt, ...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) ret = SvIV (sv); SvREFCNT_dec (sv); return sv; }
    bool Call (const char *name, float &ret, const char *fmt, ...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) ret = SvNV (sv); SvREFCNT_dec (sv); return sv; }
    bool Call (const char *name, double &ret, const char *fmt, ...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) ret = SvNV (sv); SvREFCNT_dec (sv); return sv; }
    bool Call (const char *name, char **ret, const char *fmt, ...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) *ret=SvPV_nolen (sv); SvREFCNT_dec (sv); return sv;}
    bool Call (const char *name, void **ret, const char *fmt, ...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) *ret = SvPV_nolen (sv); return sv; }
    bool Call (const char *name, csRef<iScriptObject> &ret, const char *fmt,...)
    { va_list va; va_start (va, fmt); SV *sv = parent->CallV(name,fmt,va,self);
      va_end (va); if (sv) ret = new Object (parent, "", sv); return sv; }

    bool Set (const char *name, int data)
    { hv_store (stash, name, strlen (name), newSViv (data), 0); return true; }
    bool Set (const char *name, float data)
    { hv_store (stash, name, strlen (name), newSVnv (data), 0); return true; }
    bool Set (const char *name, double data)
    { hv_store (stash, name, strlen (name), newSVnv (data), 0); return true; }
    bool Set (const char *name, char *data)
    { hv_store (stash, name, strlen (name), newSVpv (data,0), 0); return true; }
    bool Set (const char *name, void *data, const char *type)
    { SV *sv = newSViv (0); sv_setref_pv (sv, type, data);
      hv_store (stash, name, strlen (name), sv, 0); return true; }
    bool Set (const char *name, iScriptObject *data)
    { hv_store (stash, name, strlen (name),
      newSVsv (parent->Query (data)->self), 0); return true; }
    bool SetTruth (const char *name, bool data)
    { hv_store (stash, name, strlen (name), newSViv (data), 0); return true; }

    bool Get (const char *name, int &data) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      if (sv) data = SvIV (*sv); return sv; }
    bool Get (const char *name, float &data) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      if (sv) data = SvNV (*sv); return sv; }
    bool Get (const char *name, double &data) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      if (sv) data = SvNV (*sv); return sv; }
    bool Get (const char *name, char **data) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      if (sv) *data = SvPV_nolen (*sv); return sv; }
    bool Get (const char *name, void **data, const char *type) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      bool ok = sv && (sv_isa (*sv, type) || sv_derived_from (*sv, type));
      if (ok) *data = SvPV_nolen (SvRV (*sv)); return sv; }
    bool Get (const char *name, csRef<iScriptObject> &data) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      if (sv) data = new Object (parent,"",SvRV(*sv)); return sv; }
    bool GetTruth (const char *name, bool &data) const
    { SV **sv = hv_fetch (stash, name, strlen (name), 0);
      if (sv) data = SvTRUE (*sv); return sv; }
  };
  friend class Object;

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE (csPerl5);
    virtual bool Initialize (iObjectRegistry *o) { return scfParent->Init (o); }
  } scfiComponent;
  friend struct eiComponent;

protected:
  csRef<iReporter> reporter;

  bool Init (iObjectRegistry *);
  bool CheckError (const char *caller) const;
  Object* Query (iScriptObject *obj) const;
  SV* CallV (const char *name, const char *fmt, va_list va, SV *self = NULL);
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

  csPtr<iScriptObject> NewObject (const char *type, const char *fmt, ...) const;

  bool Call (const char *name, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) SvREFCNT_dec (sv); return sv; }
  bool Call (const char *name, int &ret, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) ret = SvIV (sv); SvREFCNT_dec (sv); return sv; }
  bool Call (const char *name, float &ret, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) ret = SvNV (sv); SvREFCNT_dec (sv); return sv; }
  bool Call (const char *name, double &ret, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) ret = SvNV (sv); SvREFCNT_dec (sv); return sv; }
  bool Call (const char *name, char **ret, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) *ret = SvPV_nolen (sv); SvREFCNT_dec (sv); return sv; }
  bool Call (const char *name, void **ret, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) *ret = (iBase *) SvPV_nolen (sv); return sv; }
  bool Call (const char *name, csRef<iScriptObject> &ret, const char *fmt, ...)
  { va_list va; va_start (va, fmt); SV *sv = CallV (name, fmt, va); va_end (va);
    if (sv) ret = new Object (this, ""/*FIXME*/, sv); return sv; }

  bool Store (const char *name, int data)
  { SV *sv = get_sv (name, TRUE); sv_setiv (sv, data); return true; }
  bool Store (const char *name, float data)
  { SV *sv = get_sv (name, TRUE); sv_setnv (sv, data); return true; }
  bool Store (const char *name, double data)
  { SV *sv = get_sv (name, TRUE); sv_setnv (sv, data); return true; }
  bool Store (const char *name, char *data)
  { SV *sv = get_sv (name, TRUE); sv_setpv (sv, data); return true; }
  bool Store (const char *name, void *data, const char *type)
  { SV *sv = get_sv (name, TRUE); sv_setref_pv (sv, type, data); return true; }
  bool Store (const char *name, iScriptObject *data)
  { SV *sv = get_sv (name, TRUE); sv_setsv (sv,Query (data)->self);return true;}
  bool SetTruth (const char *name, bool data)
  { SV *sv = get_sv (name, TRUE); sv_setiv (sv, data); return true; }

  bool Retrieve (const char *name, int &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvIV (sv); return sv; }
  bool Retrieve (const char *name, float &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvNV (sv); return sv; }
  bool Retrieve (const char *name, double &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvNV (sv); return sv; }
  bool Retrieve (const char *name, char **data) const
  { SV *sv = get_sv (name, FALSE); if (sv) *data = SvPV_nolen (sv); return sv; }
  bool Retrieve (const char *name, void **data, const char *type) const
  { SV *sv = get_sv (name, FALSE); bool ok = sv && sv_isa (sv, type);
    if (ok) *data = SvPV_nolen (SvRV (sv)); return ok; }
  bool Retrieve (const char *name, csRef<iScriptObject> &data) const
  { SV *sv = get_sv (name, FALSE);
    if (sv) data = new Object (this, ""/*FIXME*/, sv); return sv; }
  bool GetTruth (const char *name, bool &data) const
  { SV *sv = get_sv (name, FALSE); if (sv) data = SvTRUE (sv); return sv; }

  bool Remove (const char *name)
  { SV *sv = get_sv (name, FALSE);
    if (sv) sv_setsv(sv, &PL_sv_undef); return sv; }
};

#endif
