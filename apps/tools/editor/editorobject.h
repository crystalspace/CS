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

#ifndef __EDITOROBJECT_H__
#define __EDITOROBJECT_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include <csutil/refarr.h>
#include <csutil/hash.h>

#include "ieditor/objectlist.h"

struct iObjectRegistry;

namespace CS {
namespace EditorApp {

class EditorObject : public scfImplementation1<EditorObject, iEditorObject>
{
public:
  /// Construct an editor object wrapping the specified object.
  EditorObject (iObjectRegistry* obj_reg, iBase* object, wxBitmap* icon);
  
  virtual ~EditorObject ();
  
  virtual const char* GetName () const;

  virtual void SetName (const char* name);

  virtual iBase* GetParent ();

  virtual void SetParent (iBase* parent);

  virtual csPtr<iBaseIterator> GetIterator ();

  virtual EditorObjectType GetType () const;

  virtual wxBitmap* GetIcon () const;
  
  virtual iBase* GetIBase ();

  virtual bool HasInterface (scfInterfaceID id);

  virtual void AddListener (iEditorObjectChangeListener* listener);
  virtual void RemoveListener (iEditorObjectChangeListener* listener);

protected:
  /// Notify the listeners that this object has been changed.
  void NotifyListeners ();
  
  /// The wrapped object.
  iBase* object;
  
  /// The type (factory, instance, unknown)
  EditorObjectType type;

  /// Interface which holds the name attribute
  csRef<iInterfaceWrapper> nameAttribInterface;
  
  /// Interface which holds the parent attribute
  csRef<iInterfaceWrapper> parentAttribInterface;

  /// Wrappers around all interfaces this object implements
  csRefArray<iInterfaceWrapper> interfaces;

  /// Hash specifying which scfInterfaceId's are implemented by the object.
  csHash<bool, scfInterfaceID> hasInterfaceHash;

  /// Object change listeners
  csRefArray<iEditorObjectChangeListener> listeners;
  
  /// Icon for displaying in lists, for example
  wxBitmap* icon;
};

} // namespace EditorApp
} // namespace CS

#endif
