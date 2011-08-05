/*
    Copyright (C) 2011 by Jelle Hellemans

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

#ifndef __CSEDITOR_CONTEXT_H__
#define __CSEDITOR_CONTEXT_H__

#include "ieditor/context.h"

#include <csutil/refarr.h>


struct iObjectRegistry;

namespace CS {
namespace EditorApp {

class Context : public scfImplementation1<Context, iContext>
{
public:
  Context (iObjectRegistry* obj_reg);
  virtual ~Context ();
  
  //iContext
  virtual iObject* GetActiveObject ();
  virtual const csWeakRefArray<iObject>& GetSelectedObjects ();
  virtual void AddSelectedObject (iObject*);
  virtual void RemoveSelectedObject (iObject*);
  virtual void ClearSelectedObjects ();
  
  virtual iCamera* GetCamera ();
  virtual void SetCamera (iCamera*);
  
  virtual void AddListener (iContextListener* listener);
  virtual void RemoveListener (iContextListener* listener);
  
private:
  void NotifyListeners ();
  iObjectRegistry* object_reg;
  csWeakRef<iObject> active;
  csWeakRefArray<iObject> selection;
  csWeakRef<iCamera> camera;
  csRefArray<iContextListener> listeners;
};

} // namespace EditorApp
} // namespace CS

#endif
