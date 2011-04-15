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

#ifndef __IEDITOR_PANELMANAGER_H__
#define __IEDITOR_PANELMANAGER_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include <wx/string.h>

class wxWindow;

namespace CSE
{

enum PanelDockPosition
{
  DockPositionNone = 0,
  DockPositionTop = 1,
  DockPositionRight = 2,
  DockPositionBottom = 3,
  DockPositionLeft = 4,
  DockPositionCenter = 5,
};
  
/**
 * A panel is a window in the editor.
 * 
 * This panel interface allows plugins to implement custom panels.
 */
struct iPanel : public virtual iBase
{
  SCF_INTERFACE (iPanel, 0, 0, 1);

  /// Get the wxWindow for the content area of the panel.
  virtual wxWindow* GetWindow () = 0;

  /// Get the caption that will be shown in the titlebar of the panel.
  virtual const wxChar* GetCaption () const = 0;

  /// Get the default position to dock the panel.
  virtual int GetDefaultDockPosition () const = 0;
};


/**
 * Manages a set of panels which make up the visible parts of the editor.
 */
struct iPanelManager : public virtual iBase
{
  SCF_INTERFACE (iPanelManager, 0, 0, 1);
  
  /// Call when the managed window is being destroyed.
  virtual void Uninitialize () = 0;
  
  /// Get the window currently being managed by the panel manager.
  virtual wxWindow* GetManagedWindow () = 0;
  
  /// Set the window to be managed by the panel manager.
  virtual void SetManagedWindow (wxWindow* managedWindow) = 0;
  
  /**
   * Add a panel to the panel manager.
   */
  virtual void AddPanel (iPanel* panel) = 0;

  /**
   * Remove a panel from the panel manager.
   * \remarks Only call this when you no longer need the panel,
   *          e.g. when unloading a plugin.
   */
  virtual void RemovePanel (iPanel* panel) = 0;

  /// Set whether or not the panel should be visible.
  virtual void SetPanelVisible (iPanel* panel, bool visible) = 0;

  // TODO: Maybe a GetIterator() function?
};

}

#endif
