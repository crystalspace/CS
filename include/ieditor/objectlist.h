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

#ifndef __IEDITOR_OBJECTLIST_H__
#define __IEDITOR_OBJECTLIST_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include "ieditor/editorobject.h"

namespace CS {
namespace EditorApp {

struct iObjectList;

/**
 * Implement this if you're interested in knowing when an object is added or
 * removed from an object list.
 */
struct iObjectListListener : public virtual iBase
{
  SCF_INTERFACE (iObjectListListener, 0, 0, 1);

  /// Called just after an object has been added to the list.
  virtual void OnObjectAdded (iObjectList* list, iEditorObject* obj) = 0;

  /// Called just before an object is going to be removed from the list.
  virtual void OnObjectRemoved (iObjectList* list, iEditorObject* obj) = 0;
  
  /// Called just before all objects will be cleared from the list.
  virtual void OnObjectsCleared (iObjectList* list) = 0;
};

/**
 * A list of objects.
 */
struct iObjectList : public virtual iBase
{
  SCF_INTERFACE (iObjectList, 0, 0, 1);

  virtual void Add (iEditorObject* obj) = 0;

  virtual void Remove (iEditorObject* obj) = 0;

  virtual void Clear () = 0;
  
  virtual csPtr<iEditorObjectIterator> GetIterator () = 0;

  virtual void AddListener (iObjectListListener* listener) = 0;

  virtual void RemoveListener (iObjectListListener* listener) = 0;

  /**
   * Find the wrapper for the corresponding iBase*.
   * \returns The wrapper if the object is in the list, or NULL.
   */
  virtual iEditorObject* FindObject (iBase* obj) = 0;
};

} // namespace EditorApp
} // namespace CS

#endif
