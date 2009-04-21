/*
    Copyright (C) 2002, 2007 by Mat Sutcliffe <oktal@gmx.co.uk>

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
#include "csplugincommon/script/scriptcommon.h"
#include "iutil/comp.h"
#include "ivaria/reporter.h"
#include "csutil/scfstr.h"
#include "iutil/vfs.h"

/*
 perl.h includes dirent.h on some configurations which conflicts with
 the CS definitions of dirent, opendir, etc.  So define _DIRENT_H_ to 
 make dirent.h skip its body.  This is hackish but no better solution
 has been found.
*/
#ifndef _DIRENT_H_
#define _DIRENT_H_
#endif

/*
 The perl headers define several macros which go unused and conflict with
 names used in CS. Therefore we undef these after including the perl headers.
 MIN and MAX are already defined so we undef these first. We reasonably
 assume perl's versions of MIN and MAX have the same semantics as ours.
*/
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

struct csPerl5Value : public virtual iBase
{
  SCF_INTERFACE (csPerl5Value, 1, 0, 0);
  virtual ~csPerl5Value () {}
};

struct csPerl5Object : public virtual iBase
{
  SCF_INTERFACE (csPerl5Object, 1, 0, 0);
  virtual ~csPerl5Object () {}
};

class csPerl5 : public scfImplementationExt1<csPerl5, csScriptCommon, iComponent>
{
protected:
  class Object;

  class Value : public scfImplementation2<csPerl5::Value, iScriptValue, csPerl5Value>
  {
    csPerl5 *parent;
    PerlInterpreter *my_perl;

  protected:
    friend class csPerl5;
    friend class csPerl5::Object;

    SV *self;

  public:
    Value (csPerl5 *p, SV *s, bool incref = true)
    : scfImplementationType (this), parent (p), my_perl (p->my_perl), self (s)
    { if (incref && self) SvREFCNT_inc (self); }
    virtual ~Value () { if (self) SvREFCNT_dec (self); }

    iScript* GetScript () { return parent; }

    unsigned GetTypes () const { return tInt | tFloat | tDouble | tBool
      | tString | (sv_isobject(self) ? tObject : 0); }

    int GetInt () const { return SvIV (self); }
    double GetDouble () const { return SvNV (self); }
    float GetFloat () const { return (float) SvNV (self); }
    bool GetBool () const { return SvTRUE (self); }
    const csRef<iString> GetString () const {
      return csPtr<iString> (new scfString (SvPV_nolen (self))); }
    csRef<iScriptObject> GetObject () const { CS_ASSERT (sv_isobject (self));
      return csPtr<iScriptObject> (new csPerl5::Object (parent, self)); }
  };

  class EmptyValue : public scfImplementation1<csPerl5::EmptyValue,iScriptValue>
  {
    csPerl5 *parent;

  public:
    EmptyValue (csPerl5 *p) : scfImplementationType (this), parent (p) {}
    virtual ~EmptyValue () {}

    iScript* GetScript () { return parent; }
    unsigned GetTypes () const { return 0; }
    int GetInt () const { return 0; }
    double GetDouble () const { return 0; }
    float GetFloat () const { return 0; }
    bool GetBool () const { return false; }
    const csRef<iString> GetString () const { return 0; }
    csRef<iScriptObject> GetObject () const { return 0; }
  };

  class Object : public scfImplementationExt1<csPerl5::Object, csScriptObjectCommon, csPerl5Object>
  {
    csPerl5 *parent;
    PerlInterpreter *my_perl;

  protected:
    friend class csPerl5;
    friend class csPerl5::Value;

    SV *self;

  public:
    Object (csPerl5 *p, SV *s, bool incref = true)
    : scfImplementationType (this), parent (p), my_perl (p->my_perl), self (s)
    { CS_ASSERT (self && sv_isobject (self)); if (incref) SvREFCNT_inc (self); }
    virtual ~Object () { SvREFCNT_dec (self); }

    iScript* GetScript () { return parent; }

    const csRef<iString> GetClass () const;
    bool IsA (const char *type) const;

    void* GetPointer ();

    csPtr<iScriptValue> Call (const char *name,
      const csRefArray<iScriptValue> &args = csRefArray<iScriptValue> ());

    bool Set (const char *name, iScriptValue *value);
    csPtr<iScriptValue> Get (const char *name);
  };

  friend class Value;
  friend class Object;

  iObjectRegistry *object_reg;
  void StoreObjectReg ();

  csRef<iReporter> reporter;
  csRef<iVFS> vfs;

  PerlInterpreter *my_perl;

  bool CheckError (const char *caller) const;

  csPtr<iScriptValue> CallBody (const char *name,
    const csRefArray<iScriptValue> &args, SV *obj = 0);

  Object* Query (iScriptObject *obj) const;
  Value* Query (iScriptValue *val) const;

public:
  csPerl5 (iBase *parent);
  virtual ~csPerl5 ();

  bool Initialize (iObjectRegistry *);

  bool RunText (const char *);

  bool LoadModule (const char *name);
  bool LoadModule (const char *path, const char *filename);
  bool LoadModuleNative (const char *path, const char *filename);

  csPtr<iScriptObject> New (const char *, const csRefArray<iScriptValue> &);
  csPtr<iScriptValue> Call (const char *, const csRefArray<iScriptValue> &);

  bool Store (const char *, iScriptValue *);
  csPtr<iScriptValue> Retrieve (const char *);
  bool Remove (const char *);

  csPtr<iScriptValue> RValue (int v)
  { return csPtr<iScriptValue> (new Value (this, newSViv (v), false)); }
  csPtr<iScriptValue> RValue (void* v)
  { return csPtr<iScriptValue> (new Value (this, newSViv (INT2PTR(PTRV, v)), false)); }
  csPtr<iScriptValue> RValue (float v)
  { return csPtr<iScriptValue> (new Value (this, newSVnv (v), false)); }
  csPtr<iScriptValue> RValue (double v)
  { return csPtr<iScriptValue> (new Value (this, newSVnv (v), false)); }
  csPtr<iScriptValue> RValue (const char *v)
  { return csPtr<iScriptValue> (new Value (this, newSVpv (v, 0), false)); }
  csPtr<iScriptValue> RValue (bool v)
  { return csPtr<iScriptValue> (new Value (this, newSViv (v ? 1 : 0), false)); }
  csPtr<iScriptValue> RValue (iScriptObject *v)
  { return csPtr<iScriptValue> (new Value (this, Query (v)->self)); }
};

#endif
