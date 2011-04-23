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

#ifndef __IEDITOR_EDITOROBJECT_H__
#define __IEDITOR_EDITOROBJECT_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

class wxBitmap;

namespace CS {
namespace EditorApp {

/**
 * Iterator over iBase objects.
 */
struct iBaseIterator : public virtual iBase
{
  SCF_INTERFACE (iBaseIterator, 0, 0, 1);

  virtual bool HasNext () const = 0;

  virtual iBase* Next () = 0;

  virtual void Reset () = 0;
};

struct iEditorObject;

/**
 * Implement this if you're interested in knowing when an editor object
 * is changed. This is useful for views wanting to know when to
 * update themselves.
 */
struct iEditorObjectChangeListener : public virtual iBase
{
  SCF_INTERFACE (iEditorObjectChangeListener, 0, 0, 1);

  /// Called after an object has been changed.
  virtual void OnObjectChanged (iEditorObject* obj) = 0;
};

/**
 * The type of an object.
 */
enum EditorObjectType
{
  /// Factory type, e.g. mesh factory
  EditorObjectTypeFactory = 0,
  /// Instance type, e.g. mesh wrapper
  EditorObjectTypeInstance,
  /// Type is not known
  EditorObjectTypeUnknown,
};

/**
 * Iterator over editor objects.
 */
struct iEditorObjectIterator : public virtual iBase
{
  SCF_INTERFACE (iEditorObjectIterator, 0, 0, 1);

  virtual bool HasNext () const = 0;

  virtual iEditorObject* Next () = 0;

  virtual void Reset () = 0;
};

/**
 * Wrapper around objects that implement iBase.
 */
struct iEditorObject : public virtual iBase
{
  SCF_INTERFACE (iEditorObject, 0, 0, 1);

  /// Get the object's name.
  virtual const char* GetName () const = 0;

  /// Set the name of the object.
  virtual void SetName (const char* name) = 0;

  /// Get the object's parent in the hierarchy.
  virtual iBase* GetParent () = 0;

  /// Set the object's parent in the hierarchy.
  virtual void SetParent (iBase* parent) = 0;

  /// Get an iterator over the child objects.
  virtual csPtr<iBaseIterator> GetIterator () = 0;

  /// Get the type of the object.
  virtual EditorObjectType GetType () const = 0;
  
  /// Get an icon for the object.
  virtual wxBitmap* GetIcon () const = 0;
  
  // TODO: Add property get/set functions
  
  /// Get access to the raw iBase pointer
  virtual iBase* GetIBase () = 0;

  /**
   * Query whether the object implements the specified interface.
   */
  virtual bool HasInterface (scfInterfaceID id) = 0;

  /// Add an object change listener.
  virtual void AddListener (iEditorObjectChangeListener* listener) = 0;

  /// Remove an object change listener.
  virtual void RemoveListener (iEditorObjectChangeListener* listener) = 0;
};

} // namespace EditorApp
} // namespace CS

#endif
