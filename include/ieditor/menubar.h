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

#ifndef __IEDITOR_MENUBAR_H__
#define __IEDITOR_MENUBAR_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include <wx/string.h>

class wxMenuBar;
class wxMenuItem;

namespace CS {
namespace EditorApp {

struct iMenuItem;

/**
 * A callback to listen to menu item clicks.
 */
struct iMenuItemEventListener : public virtual iBase
{
  SCF_INTERFACE (iMenuItemEventListener, 0, 0, 1);

  /// Implement this, the menuitem passed just got clicked, now do something!
  virtual void OnClick (iMenuItem*) = 0;
};


/**
 * A standard menu item (will also hold separators)
 *
 * This class is refcounted and will remove the wrapped wxWidget from the
 * GUI and disconnect from any events when it gets invalidated.
 */
struct iMenuItem : public virtual iBase
{
  SCF_INTERFACE (iMenuItem, 0, 0, 1);

  /// Get the wrapped wxMenuItem.
  virtual wxMenuItem* GetwxMenuItem () const = 0;
  
  virtual void AddListener (iMenuItemEventListener*) = 0;
  virtual void RemoveListener (iMenuItemEventListener*) = 0;
};

/**
 * A menu item with a checkbox.
 *
 * This class is refcounted and will remove the wrapped wxWidget from the
 * GUI and disconnect from any events when it gets invalidated.
 */
struct iMenuCheckItem : public iMenuItem
{
  SCF_INTERFACE (iMenuCheckItem, 0, 0, 1);

  virtual bool IsChecked () const = 0;
  
  virtual void Check (bool) = 0;
};

/**
 * A (sub)menu on the menu bar.
 *
 * This class is refcounted and will remove the wrapped wxWidget from the
 * GUI and disconnect from any events when it gets invalidated.
 */
struct iMenu : public virtual iBase
{
  SCF_INTERFACE (iMenu, 0, 0, 1);
  
  /// Get the wrapped wxMenu.
  virtual wxMenu* GetwxMenu () const = 0;
  
  virtual csPtr<iMenuItem> AppendItem (const char* item) = 0;
  virtual csPtr<iMenuCheckItem> AppendCheckItem (const char* item) = 0;
  virtual csPtr<iMenuItem> AppendSeparator () = 0;
  virtual csPtr<iMenu> AppendSubMenu (const char* item) = 0;
};


/**
 * The root of all menus.
 */
struct iMenuBar : public virtual iBase
{
  SCF_INTERFACE (iMenuBar, 0, 0, 1);

  /// Get the wrapped wxMenuBar.
  virtual wxMenuBar* GetwxMenuBar () const = 0;
  
  virtual csPtr<iMenu> Append (const char* item) = 0;
};

} // namespace EditorApp
} // namespace CS

#endif
