/*
    Crystal Space Windowing System: list box class
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

#ifndef __CSLISTBX_H__
#define __CSLISTBX_H__

#include "cscomp.h"
#include "csscrbar.h"

/**
 * List box - specific messages
 */
enum
{
  /**
   * This message is sent by a listbox item to its parent when it receives
   * a 'mouse down' event.
   * <pre>
   * IN:  (csListBoxItem *)source;
   * </pre>
   */
  cscmdListBoxStartTracking = 0x00000400,
  /**
   * This message is sent by a listbox item to its parent when parent
   * has captured the mouse and mouse is moved over a unfocused list box item
   * <pre>
   * IN:  (csListBoxItem *)source;
   * </pre>
   */
  cscmdListBoxTrack,
  /**
   * This command tells to a ListBox object to make given item visible
   * <pre>
   * IN: (csListBoxItem *)item;
   * </pre>
   */
  cscmdListBoxMakeVisible,
  /**
   * Completely clear a listbox
   */
  cscmdListBoxClear,
  /**
   * Query state of a listbox item
   * <pre>
   * IN:  NULL
   * OUT: (int)CS_LISTBOXITEMCHECK_SELECTED or
   *      (int)CS_LISTBOXITEMCHECK_UNSELECTED
   * </pre>
   */
  cscmdListBoxItemCheck,
  /**
   * This command is sent to a list box item to set its state
   * (selected/unselected)
   * <pre>
   * IN: (bool)SelectedState;
   * </pre>
   */
  cscmdListBoxItemSet,
  /**
   * The following command is sent by a list box item to notify
   * its owner of the fact that it has been selected. List box
   * (usually owner of item is a listbox) resends this message
   * to its parent.
   * <pre>
   * IN: (csListBoxItem *)source;
   * </pre>
   */
  cscmdListBoxItemSelected,
  /**
   * Same as above, except notifies owner that list box item has been
   * deselected.
   * <pre>
   * IN: (csListBoxItem *)source;
   * </pre>
   */
  cscmdListBoxItemDeselected,
  /**
   * The following command is sent by a list box item to notify
   * its owner of the fact that it has been clicked. List box
   * (usually owner of item is a listbox) resends this message
   * to its parent. Unlike cscmdListBoxItemSelected message this
   * message is sent even if listbox item has been already selected.
   * <pre>
   * IN: (csListBoxItem *)source;
   * </pre>
   */
  cscmdListBoxItemClicked,
  /**
   * The following command is sent by a list box item to notify
   * its owner of the fact that it has been doubly clicked. List box
   * (usually owner of item is a listbox) resends this message
   * to its parent.
   * <pre>
   * IN: (csListBoxItem *)source;
   * </pre>
   */
  cscmdListBoxItemDoubleClicked,
  /**
   * This message is sent by a listbox to notify its parent that a item
   * has been focused.
   * <pre>
   * IN: (csListBoxItem *)Item;
   * </pre>
   * To find the parent listbox of the item, use Item->parent;
   * Item->parent->parent is the listbox parent.
   */
  cscmdListBoxItemFocused,
  /**
   * Ask a listbox item if it is entirely visible. If not, the list
   * box will scroll vertically, if possible, until it will be entirely
   * visible.
   * <pre>
   * IN:  (bool)false
   * OUT: (bool)true if listbox should scroll
   * </pre>
   */
  cscmdListBoxItemScrollVertically,
  /**
   * Set horizontal offset for a listbox item
   * <pre>
   * IN: (int)deltaX
   * </pre>
   */
  cscmdListBoxItemSetHorizOffset,
  /**
   * Query first selected item ID. Handy for non-multiple-select list boxes.
   * <pre>
   * OUT: (csListBoxItem *)item
   * </pre>
   */
  cscmdListBoxQueryFirstSelected,
  /**
   * Select first item that exactly matches the text.
   * <pre>
   * IN: (char *)text
   * OUT: (csListBoxItem *)item (or NULL if not found)
   * </pre>
   */
  cscmdListBoxSelectItem
};

/// Additional state flag used to mark selected list box items
#define CSS_LISTBOXITEM_SELECTED	0x00010000

/// The magic answer that means that the listbox item is selected
#define CS_LISTBOXITEMCHECK_SELECTED	0xdeadface
/// The magic answer that means that the listbox item is not selected
#define CS_LISTBOXITEMCHECK_UNSELECTED	0x0badf00d

/**
 * List box items are divided into several subtypes which will be
 * shown in different colors.
 */
enum csListBoxItemStyle
{
  /// Normal text
  cslisNormal,
  /// Emphasized text
  cslisEmphasized
};

/// This class encapsulates a menu item
class csListBoxItem : public csComponent
{
  /// List box item style
  csListBoxItemStyle ItemStyle;
  /// Horizontal item offset in pixels
  int deltax;
  /// Listbox item image
  csPixmap *ItemBitmap;
  /// Delete bitmap on object deletion?
  bool DeleteBitmap;
  /// Horizontal contents offset
  int hOffset;

public:
  /// Listbox item constructor: text item with optional style
  csListBoxItem (csComponent *iParent, const char *iText, int iID = 0,
    csListBoxItemStyle iStyle = cslisNormal);

  /// Listbox item destructor
  virtual ~csListBoxItem ();

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

  /// Draw the menu item
  virtual void Draw ();

  /// Handle additional state flags
  virtual void SetState (int mask, bool enable);

  /// Report the minimal size of menu item
  virtual void SuggestSize (int &w, int &h);

  /// Set listbox item image
  void SetBitmap (csPixmap *iBitmap, bool iDelete = true);

  /// Set content offset
  void SetOffset (int ihOffset)
  { hOffset = ihOffset; Invalidate (); }
};

/**
 * List box styles. These are bit masks that can be ORed together
 * to form a value passed to csListBox constructor.
 */
/// List box can have multiple items selected
#define CSLBS_MULTIPLESEL	0x00000001
/// List box has a horizontal scroll bar
#define CSLBS_HSCROLL		0x00000002
/// List box has a vertical scroll bar
#define CSLBS_VSCROLL		0x00000004

/// Default list box style
#define CSLBS_DEFAULTVALUE	CSLBS_VSCROLL

/// List box frame styles
enum csListBoxFrameStyle
{
  /// List box has no frame
  cslfsNone,
  /// List box has a thin 3D rectangular frame
  cslfsThinRect,
  /// List box has a thick 3D rectangular frame
  cslfsThickRect
};

/**
 * List box class is a rectangle which contains a number of list box
 * items. List box can have only one selected item at a time (if
 * CSLBS_MULTIPLESEL style is not specified) or have multiple selected
 * items at a time (if that style flag is set).
 */
class csListBox : public csComponent
{
  /// List box style
  int ListBoxStyle;
  /// List box frame style
  csListBoxFrameStyle FrameStyle;
  /// List box frame width and height
  int BorderWidth, BorderHeight;
  /// First list box item
  csComponent *first;
  /// First visible list box item
  csComponent *firstvisible;
  /// Selection state in mouse capture mode (initialized on MouseDown)
  bool selstate;
  /// Number of items that fits vertically
  int vertcount;
  /// The scroll bars
  csScrollBar *hscroll, *vscroll;
  /// Status of both scroll bars
  csScrollBarStatus hsbstatus, vsbstatus;
  /// Horizontal scrolling position & maximum
  int deltax, maxdeltax;
  /// Flag: place items before redraw?
  bool fPlaceItems;

public:
  /// Create input line object
  csListBox (csComponent *iParent, int iStyle = CSLBS_DEFAULTVALUE,
    csListBoxFrameStyle iFrameStyle = cslfsThickRect);

  /// Handle external events and generate timeouts
  virtual bool HandleEvent (iEvent &Event);

  /// Draw the list box
  virtual void Draw ();

  /// Find a place for each menu item
  void PlaceItems (bool setscrollbars = true);

  /// Report the minimal size of listbox
  virtual void SuggestSize (int &w, int &h);

  /// Resize child items on parent window resize
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /**
   * For each listbox item call a function with a optional arg
   * Function returns the first child on which func returnes 'true'
   */
  csComponent *ForEachItem (bool (*func) (csComponent *child, void *param),
    void *param = NULL, bool iSelected = true);

  /// Override SetState method to disable scroll bars as well
  virtual void SetState (int mask, bool enable);

  /// Tell parent that a new item has been selected
  virtual bool SetFocused (csComponent *comp);

  /// Set fPlaceItems since a item has been inserted
  virtual void Insert (csComponent *comp);

  /// Set fPlaceItems since a item has been removed
  virtual void Delete (csComponent *comp);

protected:
  /// Make a listbox item visible (same as cscmdListBoxMakeVisible)
  void MakeItemVisible (csComponent *item);
};

#endif // __CSLISTBX_H__
