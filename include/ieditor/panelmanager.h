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
class wxMenu;
class wxMenuBar;

namespace CS {
namespace EditorApp {

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

  /// Get the underlying wxWindow content area of this panel.
  virtual wxWindow* GetWindow () = 0;

  /// Get the caption that will be shown in the titlebar of this panel.
  virtual const wxChar* GetCaption () const = 0;

  /// Get the default position to dock this panel.
  virtual PanelDockPosition GetDefaultDockPosition () const = 0;
};


/**
 * Manages a set of panels which make up the visible parts of the editor.
 */
struct iPanelManager : public virtual iBase
{
  SCF_INTERFACE (iPanelManager, 0, 1, 0);
  
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

  /// Set whether or not the given panel should be visible.
  virtual void SetPanelVisible (iPanel* panel, bool visible) = 0;

  // TODO: Maybe a GetIterator() function?
};

} // namespace EditorApp
} // namespace CS

#endif
