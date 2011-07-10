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

#ifndef __IEDITOR_EDITOR_H__
#define __IEDITOR_EDITOR_H__

#include "csutil/array.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

#include "ieditor/editorobject.h"

class wxWindow;

struct iCollection;
struct iProgressMeter;
struct iThreadReturn;
struct csSimpleRenderMesh;

namespace CS {
namespace EditorApp {

struct iObjectList;
struct iPanelManager;
struct iMenuBar;

/// Listens for map actions
struct iMapListener : public virtual iBase
{
  SCF_INTERFACE (iMapListener, 0, 0, 1);

  /// Called after map is loaded.
  virtual void OnMapLoaded (const char* path, const char* filename) = 0;

  /// Called after library is loaded.
  virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection) = 0;
};

/**
 * The main editor interface.
 */
struct iEditor : public virtual iBase
{
  SCF_INTERFACE (iEditor, 0, 0, 1);

  virtual bool StartEngine () = 0;
  virtual bool StartApplication () = 0;
  virtual bool LoadPlugin (const char* name) = 0;

  /// Get the underlying wxWindow content area of this editor.
  virtual wxWindow* GetWindow () = 0;

  /// Get the panel manager associated with this editor
  virtual iPanelManager* GetPanelManager () const = 0;

  /// Get the panel manager associated with this editor
  virtual iMenuBar* GetMenuBar () const = 0;

  /// Get a progress meter to be notified of the progress of an action
  virtual csPtr<iProgressMeter> GetProgressMeter () = 0;

  /// Load a map from the given VFS file.
  virtual iThreadReturn* LoadMapFile (const char* path, const char* filename,
				      bool clearEngine = true) = 0;

  /// Save the engine contents to the given file
  virtual void SaveMapFile (const char* path, const char* filename) = 0;
  
  /// Load a library file.
  virtual iThreadReturn* LoadLibraryFile (const char* path, const char* filename) = 0;

  /// Add a map listener.
  virtual void AddMapListener (iMapListener* listener) = 0;

  /// Remove a map listener.
  virtual void RemoveMapListener (iMapListener* listener) = 0;

  /// Create an editor object wrapping the specified object
  virtual csPtr<iEditorObject> CreateEditorObject (iBase* object, wxBitmap* icon) = 0;
  
  /// Get the active selection.
  virtual iObjectList* GetSelection () = 0;

  /// Get the list of objects registered with the editor.
  virtual iObjectList* GetObjects () = 0;

  // TODO: remove all of that
  virtual void SetHelperMeshes (csArray<csSimpleRenderMesh>* helpers) = 0;
  virtual csArray<csSimpleRenderMesh>* GetHelperMeshes () = 0;
  
  enum TransformStatus
  {
    NOTHING = 0,
  
    MOVING,
    MOVEX,
    MOVEY,
    MOVEZ,
  
    ROTATING,
    ROTATEX,
    ROTATEY,
    ROTATEZ,
  
    SCALING,
    SCALEX,
    SCALEY,
    SCALEZ
  };

  virtual void SetTransformStatus (TransformStatus status) = 0;
  virtual TransformStatus GetTransformStatus () = 0;
};

} // namespace EditorApp
} // namespace CS

#endif
