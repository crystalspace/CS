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

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "csscript/csscript.h"

/**
 * A static class which contains functions to add scripts to a global list
 */
class csScriptList
{
public:
  /// Get the named script from the global list
  static csScript* GetScript(const char* name);

  /// Create a new script and add it to the global list
  static void NewScript(LanguageLayer* layer, char* name, char* params);

  /// Clear a particular script
  static void ClearScript (char* name);

  /// Clear all scripts
  static void ClearScripts ();
};

#endif /*TRIGGER_H*/

