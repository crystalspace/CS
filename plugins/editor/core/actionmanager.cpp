/*
    Copyright (C) 2007 by Seth Yastrov

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

#include <cssysdef.h>
#include "csutil/scf.h"

#include <csutil/objreg.h>

#include "actionmanager.h"

#include "ieditor/action.h"

namespace CS {
namespace EditorApp {

ActionManager::ActionManager (iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iActionManager");
}

ActionManager::~ActionManager ()
{
  object_reg->Unregister (this, "iActionManager");
}

bool ActionManager::Do (iAction* action)
{
  csRef<iAction> undoAction (action->Do ());
  if (!undoAction.IsValid ())
    return false;
  
  redoStack.Empty ();
  undoStack.Push (undoAction);

  NotifyListeners (action);
  
  return true;
}

bool ActionManager::Undo ()
{
  if (undoStack.IsEmpty ())
    return false;

  csRef<iAction> action (undoStack.Pop ());

  // Store redo action
  csRef<iAction> redoAction (action->Do ());
  if (redoAction.IsValid ())
    redoStack.Push (redoAction);

  NotifyListeners (action);

  return true;
}

bool ActionManager::Redo ()
{
  if (redoStack.IsEmpty ())
    return false;
  
  csRef<iAction> action (redoStack.Pop ());
  
  csRef<iAction> undoAction (action->Do ());
  if (undoAction.IsValid ())
    undoStack.Push (undoAction);

  NotifyListeners (action);

  return true;
}

const iAction* ActionManager::PeekUndo () const
{
  if (undoStack.IsEmpty ())
    return 0;
  
  return undoStack.Get (undoStack.GetSize () - 1);
}

const iAction* ActionManager::PeekRedo () const
{
  if (redoStack.IsEmpty ())
    return 0;
  
  return redoStack.Get (redoStack.GetSize () - 1);
}

void ActionManager::AddListener (iActionListener* listener)
{
  listeners.Push (listener);
}

void ActionManager::RemoveListener (iActionListener* listener)
{
  listeners.Delete (listener);
}

void ActionManager::NotifyListeners (iAction* action)
{
  csRefArray<iActionListener>::Iterator it = listeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnActionDone (action);
  }
}

} // namespace EditorApp
} // namespace CS
