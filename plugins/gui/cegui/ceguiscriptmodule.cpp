/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cssysdef.h"

#include "iutil/databuff.h"
#include "iutil/objreg.h"

#include "ceguiscriptmodule.h"

csCEGUIScriptModule::csCEGUIScriptModule (iScript* script, iObjectRegistry* reg)
{
  obj_reg = reg;
  vfs = CS_QUERY_REGISTRY(obj_reg, iVFS);
  scripting = script;
}

void csCEGUIScriptModule::executeScriptFile (
  const CEGUI::String &filename,
  const CEGUI::String &resourceGroup)
{
  csRef<iDataBuffer> buffer = vfs->ReadFile (filename.c_str());

  // Reading failed
  if (!buffer.IsValid ())
  {
    CEGUI::String msg= (uint8*)"csCEGUIScriptModule::executeScriptFile - "
      "Filename supplied for script execution must be valid";
    msg += (uint8*)" ["+filename+(uint8*)"]";
    throw CEGUI::InvalidRequestException(msg);
  }

  scripting->RunText (buffer->GetData ());
}

int csCEGUIScriptModule::executeScriptGlobal (
  const CEGUI::String &function_name)
{
  int ret;
  scripting->Call (function_name.c_str() , ret, " ");
  return ret;
}

bool csCEGUIScriptModule::executeScriptedEventHandler (
  const CEGUI::String &handler_name,
  const CEGUI::EventArgs &e)
{
  // @@@: Not implemented

  /*
  csRef<iScriptObject> obj = scripting->NewObject ("CEGUI::EventArgs", " ");
  return scripting->Call (handler_name.c_str() , "%p", (iScriptObject*) obj);
  */
  return false;
}

/// Execute script code contained in the given CEGUI::String object.
void csCEGUIScriptModule::executeString (const CEGUI::String &str)
{
  scripting->RunText (str.c_str ());
}
