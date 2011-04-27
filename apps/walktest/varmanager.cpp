/*
    Copyright (C) 2008 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "iengine/sharevar.h"
#include "iengine/engine.h"

#include "walktest.h"
#include "varmanager.h"

bool WalkTestVarManager::SetVariableColor (WalkTest* Sys, const char* arg)
{
  if (!arg)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Please give a name of a variable and a color.");
    return false;
  }
  char name[256];
  csColor c;
  csScanStr (arg, "%s,%f,%f,%f", name, &c.red, &c.green, &c.blue);

  iSharedVariableList* vl = Sys->Engine->GetVariableList ();
  iSharedVariable* v = vl->FindByName (name);
  if (!v)
  {
    csRef<iSharedVariable> nv = vl->New ();
    v = nv;
    v->SetName (name);
    vl->Add (nv);
  }
  v->SetColor (c);
  return true;
}

bool WalkTestVarManager::SetVariableVector (WalkTest* Sys, const char* arg)
{
  if (!arg)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Please give a name of a variable and a vector.");
    return false;
  }
  char name[256];
  csVector3 w;
  csScanStr (arg, "%s,%f,%f,%f", name, &w.x, &w.y, &w.z);

  iSharedVariableList* vl = Sys->Engine->GetVariableList ();
  iSharedVariable* v = vl->FindByName (name);
  if (!v)
  {
    csRef<iSharedVariable> nv = vl->New ();
    v = nv;
    v->SetName (name);
    vl->Add (nv);
  }
  v->SetVector (w);
  return true;
}

bool WalkTestVarManager::SetVariable (WalkTest* Sys, const char* arg)
{
  if (!arg)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Please give a name of a variable and a value.");
    return false;
  }
  char name[256];
  float value;
  csScanStr (arg, "%s,%f", name, &value);

  iSharedVariableList* vl = Sys->Engine->GetVariableList ();
  iSharedVariable* v = vl->FindByName (name);
  if (!v)
  {
    csRef<iSharedVariable> nv = vl->New ();
    v = nv;
    v->SetName (name);
    vl->Add (nv);
  }
  v->Set (value);
  return true;
}

bool WalkTestVarManager::ShowVariable (WalkTest* Sys, const char* arg)
{
  if (!arg)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Please give a name of a variable to show.");
    return false;
  }
  char name[256];
  csScanStr (arg, "%s", name);

  iSharedVariableList* vl = Sys->Engine->GetVariableList ();
  iSharedVariable* v = vl->FindByName (name);
  if (!v)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
	      "Couldn't find variable %s!", CS::Quote::Single (name));
    return false;
  }
  int t = v->GetType ();
  switch (t)
  {
    case iSharedVariable::SV_FLOAT:
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  float %s=%g",
	      CS::Quote::Single (v->GetName ()), v->Get ());
      break;
    case iSharedVariable::SV_COLOR:
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  color %s=%g,%g,%g",
	      CS::Quote::Single (v->GetName ()),
	      v->GetColor ().red, v->GetColor ().green, v->GetColor ().blue);
      break;
    case iSharedVariable::SV_VECTOR:
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  vector %s=%g,%g,%g",
	      CS::Quote::Single (v->GetName ()),
	      v->GetVector ().x, v->GetVector ().y, v->GetVector ().z);
      break;
    default:
      Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  unknown %s=?",
	      CS::Quote::Single (v->GetName ()));
      break;
  }
  return true;
}

bool WalkTestVarManager::ListVariables (WalkTest* Sys, const char* arg)
{
  iSharedVariableList* vl = Sys->Engine->GetVariableList ();
  int i;
  for (i = 0 ; i < vl->GetCount () ; i++)
  {
    iSharedVariable* v = vl->Get (i);
    int t = v->GetType ();
    switch (t)
    {
      case iSharedVariable::SV_FLOAT:
	Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  float %s=%g",
		CS::Quote::Single (v->GetName ()), v->Get ());
	break;
      case iSharedVariable::SV_COLOR:
	Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  color %s=%g,%g,%g",
		CS::Quote::Single (v->GetName ()),
		v->GetColor ().red, v->GetColor ().green, v->GetColor ().blue);
	break;
      case iSharedVariable::SV_VECTOR:
	Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  vector %s=%g,%g,%g",
		CS::Quote::Single (v->GetName ()),
		v->GetVector ().x, v->GetVector ().y, v->GetVector ().z);
	break;
      default:
	Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "  unknown %s=?",
		CS::Quote::Single (v->GetName ()));
	break;
    }
  }
  return true;
}

