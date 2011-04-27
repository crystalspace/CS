/*
    Copyright (C) 2005 Seth Yastrov

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
#include "iutil/databuff.h"
#include "iutil/objreg.h"
#include "ceguiimports.h"
#include "ceguiscriptmodule.h"

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  //----------------------------------------------------------------------------//
  CEGUIScriptModule::CEGUIScriptModule (iScript* script, iObjectRegistry* reg)
  {
    d_identifierString = "Crystal Space iScript Scripting Module";

    obj_reg = reg;
    vfs = csQueryRegistry<iVFS> (obj_reg);
    scripting = script;
  }

  //----------------------------------------------------------------------------//
  void CEGUIScriptModule::executeScriptFile (const CEGUI::String &filename,
                                               const CEGUI::String &resourceGroup)
  {
    csRef<iDataBuffer> buffer = vfs->ReadFile (filename.c_str());

    // Reading failed
    if (!buffer.IsValid ())
    {
      CEGUI::String msg= (uint8*)"CEGUIScriptModule::executeScriptFile - "
        "Filename supplied for script execution must be valid";
      msg += (uint8*)" ["+filename+(uint8*)"]";
      throw CEGUI::InvalidRequestException(msg);
    }

    scripting->RunText (buffer->GetData ());
  }

  //----------------------------------------------------------------------------//
  int CEGUIScriptModule::executeScriptGlobal (
    const CEGUI::String &function_name)
  {
    csRef<iScriptValue> ret = scripting->Call (function_name.c_str());
    return ret->GetInt();
  }

  //----------------------------------------------------------------------------//
  bool CEGUIScriptModule::executeScriptedEventHandler (
    const CEGUI::String & /*handler_name*/,
    const CEGUI::EventArgs & /*e*/)
  {
    // @@@: Not implemented

    /*
    csRef<iScriptObject> obj = scripting->NewObject ("CEGUI::EventArgs", " ");
    return scripting->Call (handler_name.c_str() , "%p", (iScriptObject*) obj);
    */
    return false;
  }

  //----------------------------------------------------------------------------//
  void CEGUIScriptModule::executeString (const CEGUI::String &str)
  {
    scripting->RunText (str.c_str ());
  }

  //----------------------------------------------------------------------------//
  CEGUI::Event::Connection CEGUIScriptModule::subscribeEvent(
    CEGUI::EventSet* target, const CEGUI::String& name,
    const CEGUI::String& subscriber_name)
  {
    // @@@: Not implemented
    return 0;
  }

  //----------------------------------------------------------------------------//
  CEGUI::Event::Connection CEGUIScriptModule::subscribeEvent(
    CEGUI::EventSet* target,
    const CEGUI::String& name, CEGUI::Event::Group group,
    const CEGUI::String& subscriber_name)
  {
    // @@@: Not implemented
    return 0;
  }

} CS_PLUGIN_NAMESPACE_END(cegui)
