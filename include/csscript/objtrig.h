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

#ifndef OBJTRIG_H
#define OBJTRIG_H

#include "csobject/pobject.h"
#include "csscript/trigger.h"

class csScript;

/**
 * The toplevel class for all objects in Crystal Space that
 * need support for triggers (activation in the engine using
 * the space button currently).
 */
class csObjectTrigger : public csPObject
{
private:
  /// List with all activation triggers for this object.
  TriggerList activate_triggers;

public:
  /// Create a new csObjectTrigger with the given name and type.
  csObjectTrigger ();

  /**
   * Add a new activate trigger to the list of triggers for the given
   * script and object.
   */
  void NewActivateTrigger (csScript* script, csObject* ob = NULL);

  /**
   * Activate all the active triggers. This will cause the respective
   * scripts to be run.
   */
  void DoActivateTriggers ();

  /**
   * Activate all the active triggers. This will cause the respective
   * scripts to be run.
   */
  static void DoActivateTriggers (csObject& obj);

  /// Retrieve the trigger object (if any) from the given csObject
  static csObjectTrigger* GetTrigger (csObject& obj);

  CSOBJTYPE;
};

#endif /*OBJTRIG_H*/
