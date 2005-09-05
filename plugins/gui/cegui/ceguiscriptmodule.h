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

#ifndef _CS_CEGUISCRIPTMODULE_H_
#define _CS_CEGUISCRIPTMODULE_H_

/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

#include "csutil/ref.h"
#include "iutil/vfs.h"
#include "ivaria/script.h"

#include "CEGUI.h"
#include "CEGUIScriptModule.h"

struct iObjectRegistry;

/**
 * An implementation of CEGUI::ScriptModule using the CS iScript facilities.
 */
class csCEGUIScriptModule : public CEGUI::ScriptModule
{
public:
  /// Constructor.
  csCEGUIScriptModule (iScript* script, iObjectRegistry* reg);

  /// Destructor.
  virtual ~csCEGUIScriptModule () {}

  virtual void createBindings () {}
  virtual void destroyBindings () {}

  /// Execute a script file.
  virtual void executeScriptFile (const CEGUI::String &filename,
    const CEGUI::String &resourceGroup="");

  /**
   * Execute a scripted global function.
   * The function should not take any parameters and should return an integer.
   */
  virtual int executeScriptGlobal (const CEGUI::String &function_name);

  /**
   * Execute a scripted global 'event handler' function. The function should
   * take some kind of EventArgs like parameter that the concrete
   * implementation of this function can create from the passed EventArgs based
   * object. The function should not return anything.
   */
  virtual bool executeScriptedEventHandler (const CEGUI::String &handler_name,
    const CEGUI::EventArgs &e);

  /// Execute script code contained in the given CEGUI::String object.
  virtual void executeString (const CEGUI::String &str);

protected:
  iObjectRegistry* obj_reg;
  csRef<iScript> scripting;
  csRef<iVFS> vfs;
};

#endif // _CS_CEGUISCRIPTMODULE_H_
