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

#ifndef __ACTIONMANAGER_H__
#define __ACTIONMANAGER_H__

#include "ieditor/actionmanager.h"

#include <csutil/refarr.h>

struct iObjectRegistry;

namespace CS {
namespace EditorApp {

class ActionManager : public scfImplementation1<ActionManager, iActionManager>
{
public:
  ActionManager (iObjectRegistry* obj_reg);
  virtual ~ActionManager ();

  virtual bool Do (iAction* action);

  virtual bool Undo ();
  virtual bool Redo ();

  virtual const iAction* PeekUndo () const;
  virtual const iAction* PeekRedo () const;

  virtual void AddListener (iActionListener* listener);
  virtual void RemoveListener (iActionListener* listener);
  
private:
  void NotifyListeners (iAction* listener);
  
  iObjectRegistry* object_reg;

  csRefArray<iAction> undoStack, redoStack;
  csRefArray<iActionListener> listeners;

};

} // namespace EditorApp
} // namespace CS

#endif
