/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "csscript/scripts.h"
#include "csscript/intscri.h"
#include "csscript/primscri.h"
#include "csengine/csobjvec.h"
#include "csengine/sysitf.h"
#include "csobject/nameobj.h"

//---------------------------------------------------------------------------

csObjVector& Get_Script_List()
{
  static csObjVector scripts;
  return scripts;
}

csScript* csScriptList::GetScript(const char* name)
{
  return (csScript*) (Get_Script_List ().FindByName (name));
}

void csScriptList::NewScript (LanguageLayer* layer, char* name, char* params)
{
  if (!layer) return;

  csScript* s = NULL;
  char* par = strchr (params, ':');
  if (!par)
  {
    CsPrintf (MSG_FATAL_ERROR, "Missing language qualifier for script '%s'!\n", name);
    fatal_exit (0, false);
  }

  if (Get_Script_List().FindByName (name))
  {
    CsPrintf (MSG_FATAL_ERROR, "A script with the name '%s' is already defined!\n", name);
    fatal_exit (0, false);
  }

  // All different script languages should be recognized here.
  if (!strncmp (params, "prim", (int)(par-params)))
  {
    CHK (PrimScript* sc = new PrimScript (layer));
    csNameObject::AddName (*sc, name);
    par++;
    sc->load (&par);
    s = (csScript*)sc;
  }
  else if (!strncmp (params, "int", (int)(par-params)))
  {
    CHK (IntScript* sc = new IntScript (layer));
    csNameObject::AddName (*sc, name);
    par++;
    sc->load (par);
    s = (csScript*)sc;
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "Unknown script qualifier for script '%s'!\n", name);
    fatal_exit (0, false);
    return;
  }

  s->prepare ();
  Get_Script_List ().Push (s);
}

void csScriptList::ClearScript (char* name)
{
  csObject *o = Get_Script_List().FindByName (name);
  if (o)
  {
    int idx = Get_Script_List().Find (o);
    if (idx >= 0)
      Get_Script_List ().Delete (idx);
  }
}

void csScriptList::ClearScripts ()
{
  Get_Script_List ().DeleteAll ();
}
