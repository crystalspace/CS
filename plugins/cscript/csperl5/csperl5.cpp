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

#include "cssysdef.h"
#include "csperl5.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/databuff.h"

extern "C" void xs_init (pTHX); // defined in csperlxs.c


SCF_IMPLEMENT_FACTORY (csPerl5)

//------------------------------------------------------- csPerl5 ----//

csPerl5::csPerl5 (iBase *parent)
: scfImplementationType (this, parent), my_perl (0)
{
}

bool csPerl5::Initialize (iObjectRegistry *objreg)
{
  reporter = csQueryRegistry<iReporter> (objreg);
  if (! reporter.IsValid ()) return false;

  vfs = csQueryRegistry<iVFS> (objreg);
  if (! vfs.IsValid ())
  {
    reporter->ReportError ("crystalspace.script.perl5.init",
      "Can't find VFS plugin");
    return false;
  }

  object_reg = objreg;

  const char *vfsinc = "/scripts/perl5";
  csRef<iDataBuffer> incbuff = vfs->GetRealPath (vfsinc);
  if (! incbuff.IsValid ())
  {
    reporter->ReportError ("crystalspace.script.perl5.init",
      "Can't find directory %s in VFS", vfsinc);
    return false;
  }

  my_perl = perl_alloc ();
  if (my_perl == 0)
  {
    reporter->ReportError ("crystalspace.script.perl5.init",
      "Can't allocate memory for perl interpreter");
    return false;
  }
  perl_construct (my_perl);

  char *realinc = (char *) incbuff->GetData ();
  char *argv [] = { "perl5", "-I", realinc, "-T", "-e", "1" };
  int const argc = sizeof(argv) / sizeof(argv[0]);
  perl_parse (my_perl, xs_init, argc, argv, 0);
  perl_run (my_perl);
  return true;
}

csPerl5::~csPerl5 ()
{
  if (my_perl != 0)
  {
    perl_destruct (my_perl);
    perl_free (my_perl);
  }
}

csPerl5::Value* csPerl5::Query (iScriptValue *val) const
{
  csRef<csPerl5Value> priv = scfQueryInterface<csPerl5Value> (val);
  if (priv.IsValid ())
    return static_cast<csPerl5::Value*> ((csPerl5Value*)priv);

  reporter->Report (CS_REPORTER_SEVERITY_ERROR,
    "crystalspace.script.perl5", "This iScriptValue is empty or not from Perl");
  return 0;
}

csPerl5::Object* csPerl5::Query (iScriptObject *obj) const
{
  csRef<csPerl5Object> priv = scfQueryInterface<csPerl5Object> (obj);
  if (priv.IsValid ())
    return static_cast<csPerl5::Object*> ((csPerl5Object*)priv);

  reporter->Report (CS_REPORTER_SEVERITY_ERROR,
    "crystalspace.script.perl5", "This iScriptObject is not from Perl");
  return 0;
}

bool csPerl5::CheckError (const char *caller) const
{
  SV* error = get_sv ("@", FALSE);

  if (SvTRUE (error))
  {
    reporter->ReportError ("crystalspace.script.perl5.fatal",
      "perl: %s: %s", caller, SvPV_nolen (error));
    return false;
  }
  return true;
}

bool csPerl5::RunText (const char *text)
{
  eval_pv (text, FALSE);
  return CheckError ("eval");
}

bool csPerl5::LoadModule (const char *name)
{
  csString text;
  text << "use " << name;
  bool ok = RunText (text.GetData ());

  if (! ok) return false;
  if (csString ("cspace") == name) StoreObjectReg ();
  return true;
}

bool csPerl5::LoadModule (const char *path, const char *filename)
{
  csRef<iDataBuffer> nativepath (vfs->GetRealPath (path));
  if (nativepath.IsValid ())
    return LoadModuleNative (nativepath->GetData (), filename);
  return false;
}

bool csPerl5::LoadModuleNative (const char *path, const char *filename)
{
  csString text;
  text <<
  "{"
    "local @INC = ('" << path << "', @INC);"
    "require '" << filename << "';"
  "}";
  bool ok = RunText (text.GetData ());

  if (! ok) return false;
  if (csString ("cspace.pm") == filename) StoreObjectReg ();
  return true;
}

bool csPerl5::Store (const char *name, iScriptValue *value)
{
  SV *sv = get_sv (name, TRUE);
  if (! sv) return false;
  SvSetMagicSV (sv, Query (value)->self);
  return true;
}

csPtr<iScriptValue> csPerl5::Retrieve (const char *name)
{
  SV *sv = get_sv (name, FALSE);
  if (SvOK (sv)) return new Value (this, newSVsv (sv), false);
  return 0;
}

bool csPerl5::Remove (const char *name)
{
  SV *sv = get_sv (name, FALSE);
  if (! SvOK (sv)) return false;
  SvSetMagicSV (sv, &PL_sv_undef);
  return true;
}

csPtr<iScriptValue> csPerl5::Call (const char *name,
  const csRefArray<iScriptValue> &args)
{
  csString name2;
  name2 << "cspace::" << name;

  csRef<iScriptValue> ret (CallBody (name2.GetData (), args));
  if (! ret.IsValid()) ret.AttachNew (CallBody (name, args));

  return csPtr<iScriptValue> (ret);
}

csPtr<iScriptValue> csPerl5::CallBody (const char *name,
  const csRefArray<iScriptValue> &args, SV *obj)
{
  dSP;
  ENTER;
  SAVETMPS;

  PUSHMARK(SP);

  if (obj) XPUSHs (obj);

  csRefArray<iScriptValue>::ConstIterator iter (args.GetIterator ());
  while (iter.HasNext ())
    XPUSHs (Query (iter.Next ())->self);

  PUTBACK;

  int n = obj ? call_method (name, G_SCALAR | G_EVAL | G_KEEPERR)
		: call_pv (name, G_SCALAR | G_EVAL | G_KEEPERR);

  SPAGAIN;

  csRef<iScriptValue> ret;
  if (n >= 1)
  {
    SV *sv = POPs;
    if (SvOK (sv))
      ret = csPtr<iScriptValue> (new Value (this, sv));
    else
      ret = csPtr<iScriptValue> (new EmptyValue (this));
  }
  else ret = csPtr<iScriptValue> (new EmptyValue (this));

  PUTBACK;
  FREETMPS;
  LEAVE;

  if (! CheckError ("call")) return 0;

  return csPtr<iScriptValue> (ret);
}

csPtr<iScriptObject> csPerl5::New(const char *name,
  const csRefArray<iScriptValue> &args)
{
  csString name2;
  name2 << "cspace::" << name;
  csRef<iScriptValue> type1 (RValue (name));
  csRef<iScriptValue> type2 (RValue (name2.GetData ()));

  csRef<iScriptValue> ret (CallBody ("new", args, Query (type2)->self));
  if (! ret.IsValid ())
	ret.AttachNew (CallBody ("new", args, Query (type1)->self));

  if (ret.IsValid ()) return new Object (this, Query (ret)->self);
  return 0;
}

void csPerl5::StoreObjectReg()
{
  csRefArray<iScriptValue> args;
  args.Push (csRef<iScriptValue> (RValue ((iObjectRegistry*) object_reg)));

  csRef<iScriptValue> discard (Call ("_SetObjectReg", args));
}

//----------------------------------------------- csPerl5::Object ----//

csPtr<iScriptValue> csPerl5::Object::Call(const char *name,
  const csRefArray<iScriptValue> &args)
{
  return parent->CallBody (name, args, self);
}

bool csPerl5::Object::Set (const char *name, iScriptValue *value)
{
  csRefArray<iScriptValue> args;
  args.Push (value);
  csRef<iScriptValue> ret (Call (name, args));
  return ret.IsValid ();

//Alternate implementation:
#if 0
  csRefArray<iScriptValue> args;
  args.Push (csRef<iScriptValue> (parent->RValue (name)));
  args.Push (value);
  csRef<iScriptValue> ret (Call ("STORE", args));
  return ret.IsValid ();
#endif
}

csPtr<iScriptValue> csPerl5::Object::Get (const char *name)
{
  return Call (name);

// Alternate implementation:
#if 0
  csRefArray<iScriptValue> args;
  args.Push (csRef<iScriptValue> (parent->RValue (name)));
  return Call ("FETCH", args);
#endif
}

const csRef<iString> csPerl5::Object::GetClass () const
{
  csRef<iString> c (csPtr<iString>
    (new scfString (HvNAME (SvSTASH (SvRV (self))))));

  if (c->StartsWith ("cspace::")) c->DeleteAt (0, 8);
  return c;
}

bool csPerl5::Object::IsA (const char *type) const
{
  if (sv_derived_from (self, type)) return true;

  csString type2;
  type2 << "cspace::" << type;
  const char *type2d = type2.GetData ();
  if (sv_derived_from (self, type2d)) return true;

  return false;
}

void* csPerl5::Object::GetPointer ()
{
  csRef<iScriptValue> val (Call ("_GetCPointer"));
  if (! val.IsValid ()) return 0;
  return (void *) val->GetInt ();
}
