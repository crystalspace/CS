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

#ifndef __IEDITOR_VIEWMANAGER_H__
#define __IEDITOR_VIEWMANAGER_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include <wx/string.h>

class wxWindow;

namespace CS {
namespace EditorApp {
  
struct iContext;

enum ViewDockPosition
{
  DockPositionNone = 0,
  DockPositionTop = 1,
  DockPositionRight = 2,
  DockPositionBottom = 3,
  DockPositionLeft = 4,
  DockPositionCenter = 5,
};
  
/**
 * A view is a window in the editor.
 * 
 * This view interface allows plugins to implement custom views.
 */
struct iView : public virtual iBase
{
  SCF_INTERFACE (iView, 0, 0, 1);

  /// Get the underlying wxWindow content area of this view.
  virtual wxWindow* GetWindow () = 0;

  /// Get the caption that will be shown in the titlebar of this view.
  virtual const wxChar* GetCaption () const = 0;

  /// Get the default position to dock this view.
  virtual ViewDockPosition GetDefaultDockPosition () const = 0;
};


/**
 * Manages a set of views which make up the visible parts of the editor.
 */
struct iViewManager : public virtual iBase
{
  SCF_INTERFACE (iViewManager, 0, 1, 0);
  
  /**
   * Add a view to the view manager.
   */
  virtual void AddView (iView* view) = 0;

  /**
   * Remove a view from the view manager.
   * \remarks Only call this when you no longer need the view,
   *          e.g. when unloading a plugin.
   */
  virtual void RemoveView (iView* view) = 0;

  /// Set whether or not the given view should be visible.
  virtual void SetViewVisible (iView* view, bool visible) = 0;

  // TODO: Maybe a GetIterator() function?
};

} // namespace EditorApp
} // namespace CS

#endif
