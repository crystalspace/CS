/*
    Crystal Space Windowing System: menu class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSMENU_H__
#define __CS_CSMENU_H__

/**\file
 * Crystal Space Windowing System: menu class
 */

/**
 * \addtogroup csws_comps_menu
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"

/**
 * \name Menu item styles
 * @{ */
/// Menu item has a checkmark at the left
#define CSMIS_CHECKED		0x00000001
/// Menu item is a separator
#define CSMIS_SEPARATOR		0x00000010
/// This menu item starts a new column (if frame style != csmfsBar)
#define CSMIS_NEWCOLUMN		0x00000020
/// Do not close menu when menu item is activated
#define CSMIS_NOCLOSE		0x00000040
/// Default menu item styles
#define CSMIS_DEFAULTVALUE	0
/** @} */

// Forward declaration (for usage in csMenuItem)
class csMenu;

/// This class encapsulates a menu item
class CS_CRYSTALSPACE_EXPORT csMenuItem : public csComponent
{
  /// Menu item info (if not 0) (placed to the right of menu item text)
  char *info;
  /// Character number that should be underlined (-1 == none)
  size_t underline_pos;
  /// Menu item styles
  int Style;
  /// Menu item command code
  int CommandCode;
  /// Sumbenu object
  csMenu *SubMenu;

public:
  /// Menu item constructor: text item with optional style
  csMenuItem (csComponent *iParent, const char *iText,
    int iCommandCode = 0, int iStyle = CSMIS_DEFAULTVALUE);

  /// Menu item constructor: construct a separator item
  csMenuItem (csComponent *iParent, int iStyle = CSMIS_DEFAULTVALUE);

  /// Menu item constructor: construct a submenu
  csMenuItem (csComponent *iParent, const char *iText, csMenu *iSubMenu,
    int iStyle = CSMIS_DEFAULTVALUE);

  /// Destroy menu item object
  virtual ~csMenuItem ();

  /// Report the minimal size of menu item
  virtual void SuggestSize (int &w, int &h);

  /// Set menu item text
  virtual void SetText (const char *iText);

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

  /// Draw the menu item
  virtual void Draw ();

  /// Move child menus when moved
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Get the Style bitmask for this menu item.
  virtual int GetStyle () { return Style; };

protected:
  /// Menu item 'checked' image
  static csPixmap *sprchecked;
  /// "Open submenu" arrow image
  static csPixmap *sprsubmenu;

  /// Common part of constructors
  void Init ();

  /// Activate this menu item
  virtual void Press ();
};

/// csMenu class messages
enum
{
  /**
   * Tell menu or menu item to deactivate.
   * <pre>
   * IN: (int)DismissCode;
   * </pre>
   */
  cscmdDeactivateMenu = 0x00000200,
  /**
   * Tell menu to place all its items on their places and that it should
   * re-size itself (if it is not a menu bar).
   */
  cscmdMenuPlaceItems,
  /**
   * Tell menu to capture the mouse, if its parent menu didn't so.
   * <pre>
   * IN: (csComponent *)Source;
   * OUT: 0 if mouse has been captured
   * </pre>
   */
  cscmdMenuCaptureMouse,
  /**
   * Set menu's current item to command argument.
   * <pre>
   * IN: (csComponent *)Item;
   * OUT: 0 if successfull
   * </pre>
   */
  cscmdMenuSetItem,
  /**
   * If menu has no current item, select the item that was last active.
   */
  cscmdMenuSetLastItem,
  /**
   * Query if submenus are dropped out automatically
   * <pre>
   * IN:  0
   * OUT: (bool)DropFlag
   * </pre>
   */
  cscmdMenuQueryDropFlag,
  /**
   * Query if submenus are dropped out automatically
   * <pre>
   * IN:  (bool)DropFlag
   * OUT: (csMenu *)menu if successful
   * </pre>
   */
  cscmdMenuSetDropFlag,
  /**
   * Set/remove 'checked' mark at the right of menu item.<p>
   * This command should be sent to a menu item object to set/unset the
   * check mark to the right of item. Usage example:
   * <pre>
   * menu->GetChild (cscmdQuit)->SendCommand (cscmdMenuItemCheck, true);
   * </pre>
   * <pre>
   * IN: (bool)true (set) or false (unset)
   * </pre>
   */
  cscmdMenuItemCheck,
  /**
   * Query menu item's style
   * <pre>
   * IN: 0
   * OUT: (int)ItemStyle
   * </pre>
   */
  cscmdMenuItemGetStyle,
  /**
   * Check if menu item's id or id of a menu item in submenu of this menu
   * item has given id.
   * <pre>
   * IN: (int)id;
   * OUT: (csComponent *)menuitem or 0
   * </pre>
   */
  cscmdMenuItemFindId
};

/// Possible menu frame styles
enum csMenuFrameStyle
{
  /// Menu has no frame
  csmfsNone,
  /// Menu has a single-colored one-pixel border
  csmfsThin,
  /// Menu is a horizontal menu
  csmfsBar,
  /// Normal menu with a 3D border
  csmfs3D
};

/**
 * \name Menu style flags
 * @{ */
/// hide menu when it deactivates
#define CSMS_HIDEINACTIVE	0x00000001
/// Default menu style value
#define CSMS_DEFAULTVALUE	CSMS_HIDEINACTIVE
/** @} */

/**
 * The Menu class represents two types of menu: vertical (popup)
 * menus and bar menus.
 *
 * In fact, menu class is a bit messy just now, and as soon as I will have time
 * it should be cleaned up. However, it works reasonably well, so it is just
 * a matter of taste.
 */
class CS_CRYSTALSPACE_EXPORT csMenu : public csComponent
{
  friend class csMenuItem;

  /// Menu border width and height
  int BorderWidth,BorderHeight;
  /// Menu frame style
  int FrameStyle;
  /// Menu style flags
  int MenuStyle;
  /// Remember the first menu item
  csComponent *first;
  /// Remember the last selected menu item
  csComponent *last;
  /// Old parent's focused component
  csComponent *oldparentfocus;
  /// Are submenus opened?
  bool SubMenuOpened;
  /// Flag: re-place items on first ::Draw()?
  bool fPlaceItems;

public:
  /// Current menu item
  csComponent *current;

  /// Create menu object
  csMenu (csComponent *iParent, csMenuFrameStyle iFrameStyle = csmfs3D,
    int iMenuStyle = CSMS_DEFAULTVALUE);

  /// Draw the menu
  virtual void Draw ();

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

  /// Pre-handle keyboard events to catch hotkeys
  virtual bool PreHandleEvent (iEvent &Event);

  /// Pass a event to current item
  bool CurrentHandleEvent (iEvent &Event);

  /// Recalculate menu size (called after each menu item insertion)
  virtual void PlaceItems ();

  /// Return true if menu is a menu bar
  bool IsMenuBar ()
  { return (FrameStyle == csmfsBar); }

  /// Set/clear given component state flags
  virtual void SetState (int mask, bool enable);

  /// Set a child as current menu item
  bool SetCurrent (csComponent *newCurrent, bool DropSubmenu = false);

  /// Re-position childs when rescaled
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Deactivate menu
  void Deactivate (int DismissCode);

  /// Find the item with given command code (even in submenus)
  csComponent *GetItem (int iCommandCode);

  /// Set/remove a checkmark left of the menu item
  void SetCheck (int iCommandCode, bool iState);

  /// Get the checked state of the specified menu item.
  bool GetCheck (int iCommandCode);

  /// Set fPlaceItems since a item has been inserted
  virtual void Insert (csComponent *comp);

  /// Set fPlaceItems since a item has been removed
  virtual void Delete (csComponent *comp);

  /// Suggest the size of the menu
  virtual void SuggestSize (int &w, int &h);

private:
  /// Set 'width' for 'count' items from 'start'
  void SetItemWidth (csComponent *start, int count, int width);

  /// Move from current item to next selectable item in one of four directions
  virtual bool ExecuteKey (int key);
};

/**
 * \page Example Example
 * Just a small example how menus can be defined and used:
 * <pre>
 * csComponent *window = new csWindow (app, "Window title");
 * csMenu *menu = (csMenu *)window->GetChild (CSWID_MENUBAR);
 * if (menu)
 * {
 *   submenu = new csMenu (0);
 *   (void)new csMenuItem (menu, "~File", submenu);
 *     (void)new csMenuItem (submenu, "~Open\tCtrl+O", cscmdNothing);
 *     (void)new csMenuItem (submenu, "~Save\tCtrl+S", cscmdNothing);
 *     (void)new csMenuItem (submenu, "~Close", cscmdNothing);
 *     (void)new csMenuItem (submenu);
 *     (void)new csMenuItem (submenu, "~Quit\tCtrl+Q", cscmdQuit);
 *   [...]
 * }
 * </pre>
 */

/** @} */

#endif // __CS_CSMENU_H__
