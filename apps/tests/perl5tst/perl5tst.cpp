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
#include "csutil/ref.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "iutil/objreg.h"
#include "iutil/string.h"
#include "ivaria/script.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"

CS_IMPLEMENT_APPLICATION

int main (int argc, char *argv[])
{
  iObjectRegistry* objreg = csInitializer::CreateEnvironment (argc, argv);
  if (! objreg)
  {
    csFPrintf (stderr, "Failed to create environment!\n");
    return 1;
  }

  bool ok = csInitializer::RequestPlugins (objreg,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.script.perl5", iScript),
    CS_REQUEST_END);
  if (! ok)
  {
    csFPrintf (stderr, "Failed to load plugins!\n");
    return 2;
  }

  if (csCommandLineHelper::CheckHelp (objreg))
  {
    csCommandLineHelper::Help (objreg);
    return 0;
  }

  {
    csRef<iScript> script = csQueryRegistry<iScript> (objreg);
    if (! script)
    {
      csFPrintf (stderr, "Failed to find perl5 plugin!\n");
      return 3;
    }

    ok = script->LoadModule ("cspace");
    //ok = script->LoadModule ("scripts/perl5", "cspace.pm");
    if (! ok)
    {
      csFPrintf (stderr, "Failed to load perl5 cspace module!\n");
      return 4;
    }

    csInitializer::OpenApplication (objreg);

    //====================================================================//
    csPrintf ("Testing RValue/Store/Retrieve:\n");

    int test_int = 3;
    float test_float = 3.0;
    double test_double = 3.0;
    bool test_bool = true;
    const char *test_str = "hello";

    csPrintf ("  Int: ");
    csRef<iScriptValue> int_value (script->RValue (test_int));
    ok = script->Store("i", int_value);
    int_value.AttachNew (script->Retrieve ("i"));
    csPrintf ("%d == %d\n", test_int, int_value->GetInt ());

    csPrintf ("  Float: ");
    csRef<iScriptValue> float_value (script->RValue (test_float));
    ok = script->Store("f", float_value);
    float_value.AttachNew (script->Retrieve ("f"));
    csPrintf ("%f == %f\n", test_float, float_value->GetFloat ());

    csPrintf ("  Double: ");
    csRef<iScriptValue> double_value (script->RValue (test_double));
    ok = script->Store("d", double_value);
    double_value.AttachNew (script->Retrieve ("d"));
    csPrintf ("%lf == %lf\n", test_double, double_value->GetDouble ());

    csPrintf ("  String: ");
    csRef<iScriptValue> str_value (script->RValue (test_str));
    ok = script->Store("s", str_value);
    str_value.AttachNew (script->Retrieve ("s"));
    csPrintf ("%s == %s\n", test_str, str_value->GetString ()->GetData ());

    csPrintf ("  Bool: ");
    csRef<iScriptValue> bool_value (script->RValue (test_bool));
    ok = script->Store("b", bool_value);
    bool_value.AttachNew (script->Retrieve ("b"));
    csPrintf ("%s == %s\n\n", test_bool ? "true" : "false",
			      bool_value->GetBool () ? "true" : "false");

    //====================================================================//
    csPrintf ("Testing Remove:\n");

    ok = script->Remove ("i") && script->Remove ("f") && script->Remove ("d")
      && script->Remove ("s") && script->Remove ("b");
    csPrintf ("  %s\n", ok ? "ok" : "failed");

    int_value.AttachNew (script->Retrieve ("i"));
    csPrintf ("  %s\n", int_value.IsValid () ? "failed" : "ok");

    //====================================================================//
    csPrintf ("Testing New(csVector3):\n");

    csRef<iScriptObject> obj (script->New ("csVector3"));
    csPrintf ("  %s\n", obj.IsValid () ? "ok" : "failed");

    //====================================================================//
    csPrintf ("Testing GetClass/IsA:\n");

    csRef<iString> classname (obj->GetClass ());
    csPrintf ("  %s\n", classname->GetData ());

    csPrintf ("  %s\n", obj->IsA ("csVector3") ? "ok" : "failed");

    //====================================================================//
    csPrintf ("Testing Set/Get:\n");

    csPrintf ("  %f == ", float_value->GetFloat ());

    ok = obj->Set ("x", float_value);
    float_value.AttachNew (obj->Get ("x"));

    csPrintf ("%f\n", float_value->GetFloat ());

    //====================================================================//
    csPrintf ("Testing Call(csVector3::Set):\n");

    csRefArray<iScriptValue> args;
    args.Push (float_value);

    csRef<iScriptValue> ret (obj->Call ("Set", args));
    csPrintf ("  %f\n", float_value->GetFloat ());

    //====================================================================//
    csPrintf ("Testing Call(csVector3::Norm):\n");

    ret.AttachNew (obj->Call ("Norm"));
    csPrintf ("  %f\n", ret->GetFloat ());

    //====================================================================//
    csPrintf ("Testing GetPointer:\n");

    csVector3 &vector = * (csVector3 *) obj->GetPointer ();

    vector.Normalize ();

    csPrintf ("  %f %f %f\n", vector[0], vector[1], vector[2]);
    csPrintf ("  ok\n");

    csPrintf ("All Done!\n");
  }

  csInitializer::DestroyApplication (objreg);
  return 0;
}
