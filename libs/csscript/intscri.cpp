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
#include "csengine/sysitf.h"
#include "csscript/intscri.h"
#include "csutil/scanstr.h"

//---------------------------------------------------------------------------

IntScriptRegister int_script_reg;

IntScriptRegister::IntScriptRegister ()
{
  first = NULL;
}

IntScriptRegister::~IntScriptRegister ()
{
  while (first)
  {
    Reg* n = first->next;
    if (first->name)
      CHKB (delete [] first->name);
    CHK (delete first);
    first = n;
  }
}

void IntScriptRegister::reg (char* name, IntScriptFunc* func)
{
  if (find_function (name))
  {
    CsPrintf (MSG_FATAL_ERROR, "There is already a function registered with the name '%s'!\n", name);
    fatal_exit (0, true);
    return; // if fatal_exit returns
  }
  CHK (Reg* r = new Reg);
  r->next = first;
  CHK (r->name = new char [strlen (name)+1]);
  strcpy (r->name, name);
  r->func = func;
  first = r;
}

IntScriptFunc* IntScriptRegister::find_function (char* name)
{
  Reg* r = first;
  while (r)
  {
    if (!strcmp (name, r->name)) return r->func;
    r = r->next;
  }
  return NULL;
}

//---------------------------------------------------------------------------

CSOBJTYPE_IMPL(IntScript,csScript);

IntScript::IntScript (LanguageLayer* layer) : csScript (layer)
{
  data = NULL;
}

IntScript::~IntScript ()
{
  if (data) CHKB (delete [] data);
}

void IntScript::load (char* buf)
{
  char str[255];
  char str2[255];
  ScanStr (buf, "%s,%s", str, str2);
  func = int_script_reg.find_function (str);
  if (!func)
  {
    CsPrintf (MSG_FATAL_ERROR, "There is no registered function with the name '%s'!\n", str);
    fatal_exit (0, true);
    return; // if fatal_exit returns
  }
  CHK (data = new char [strlen (str2)+1]);
  strcpy (data, str2);
}

void IntScript::prepare ()
{
}

csRunScript* IntScript::run_script (csObject* attached)
{
  CHK (IntRunScript* r = new IntRunScript (this, attached));
  layer->link_run (r);
  return (csRunScript*)r;
}

//---------------------------------------------------------------------------

IntRunScript::IntRunScript (IntScript* script, csObject* object) : csRunScript (script, object)
{
  init ();
}

IntRunScript::~IntRunScript ()
{
}

void IntRunScript::init ()
{
}

bool IntRunScript::step ()
{
  return ((IntScript*)script)->func (this, ((IntScript*)script)->data);
}

void IntRunScript::deliver_event (csScriptEvent event)
{
  (void)event;
}

//---------------------------------------------------------------------------
