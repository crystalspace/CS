/*
    Crystal Space Windowing System: tree class
    Copyright (C) 2000 by Norman Krämer
    based on the listbox code:
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

#ifndef __CSTREECTRL_H__
#define __CSTREECTRL_H__

#include "csutil/csbase.h"
#include "csutil/cstreend.h"
#include "cscomp.h"
#include "csiline.h"
#include "csscrbar.h"

/// Additional state flag used to mark selected tree items
#define CSS_TREEITEM_SELECTED	0x00008000
/// is branch open ?
#define CSS_TREEITEM_OPEN	0x0000A000

/**
 * Tree items are divided into several subtypes which will be
 * shown in different colors.
 */
enum csTreeItemStyle
{
  /// Normal text
  cstisNormal,
  /// Emphasized text
  cstisEmphasized
};

/// This class encapsulates a tree item
class csTreeItem : public csComponent
{
  /// Tree item style
  csTreeItemStyle ItemStyle;
  /// Horizontal item offset in pixels
  int deltax;
  /// Tree item image
  csPixmap *ItemBitmap;
  /// Delete bitmap on object deletion?
  bool DeleteBitmap;
  /// Horizontal and vertical content offset
  int hOffset, vOffset;
public:
  /// Tree item constructor: text item with optional style
  csTreeItem (csComponent *iParent, const char *iText, int iID = 0,
    csTreeItemStyle iStyle = cstisNormal);

  /// Tree item destructor
  virtual ~csTreeItem ();

  /// Handle input events
  virtual bool HandleEvent (csEvent &Event);

  /// Draw the tree item
  virtual void Draw ();

  /// Handle additional state flags
  virtual void SetState (int mask, bool enable);

  /// Report the minimal size of tree item
  virtual void SuggestSize (int &w, int &h);

  /// Set tree item image
  void SetBitmap (csPixmap *iBitmap, bool iDelete = true);

  /// Set content offset
  void SetOffset (int ihOffset, int ivOffset)
  { hOffset = ihOffset; vOffset = ivOffset; Invalidate (); }

};

/**
 * Tree control styles
 */
/// Tree can have multiple items selected
#define CSTS_MULTIPLESEL	0x00000001
/// Tree has a horizontal scroll bar
#define CSTS_HSCROLL		0x00000002
/// Tree has a vertical scroll bar
#define CSTS_VSCROLL		0x00000004

/// default tree control style
#define CSTS_DEFAULTVALUE	CSTS_VSCROLL

/// Tree control frame styles
enum csTreeFrameStyle
{
  /// tree control has no frame
  cstfsNone,
  /// tree control has a thin 3D rectangular frame
  cstfsThinRect,
  /// tree control has a thick 3D rectangular frame
  cstfsThickRect
};

/**
 * Tree control - specific messages
 */
enum
{
  /**
   * This message is sent by a tree item to its parent when it receives
   * a 'mouse down' event.
   * <pre>
   * IN:  (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeStartTracking = 0x00000e00,
  /**
   * This message is sent by a tree item to its parent when parent
   * has captured the mouse and mouse is moved over a unfocused tree item
   * <pre>
   * IN:  (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeTrack,
  /**
   * This command tells to a Tree control object to make given item visible
   * <pre>
   * IN: (cstreeItem *)item;
   * </pre>
   */
  cscmdTreeMakeVisible,
  /**
   * Completely clear a tree
   */
  cscmdTreeClear,
  /**
   * Query state of a tree item
   * <pre>
   * IN:  NULL
   * OUT: (int)CS_TREEITEMCHECK_SELECTED or
   *      (int)CS_TREEITEMCHECK_UNSELECTED
   * </pre>
   */
  cscmdTreeItemCheck,
  /**
   * This command is sent to a tree item to set its state
   * (selected/unselected)
   * <pre>
   * IN: (bool)SelectedState;
   * </pre>
   */
  cscmdTreeItemSet,
  /**
   * The following command is sent by a tree item to notify
   * its owner of the fact that it has been selected. Tree
   * (usually owner of item is a tree control) resends this message
   * to its parent.
   * <pre>
   * IN: (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeItemSelected,
  /**
   * Same as above, except notifies owner that tree item has been
   * deselected.
   * <pre>
   * IN: (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeItemDeselected,
  /**
   * The following command is sent by a tree item to notify
   * its owner of the fact that it has been clicked. Tree
   * (usually owner of item is a tree control) resends this message
   * to its parent. Unlike cscmdTreeItemSelected message this
   * message is sent even if tree item has been already selected.
   * <pre>
   * IN: (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeItemClicked,
  /**
   * The following command is sent by a tree item to notify
   * its owner of the fact that it has been doubly clicked. Tree
   * (usually owner of item is a tree control) resends this message
   * to its parent.
   * <pre>
   * IN: (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeItemDoubleClicked,
  /**
   * This message is sent by a tree to notify its parent that a item
   * has been focused.
   * <pre>
   * IN: (csTreeItem *)Item;
   * </pre>
   * To find the parent tree of the item, use Item->parent;
   * Item->parent->parent is the trees parent.
   */
  cscmdTreeItemFocused,
  /**
   * Ask a tree item if it is entirely visible. If not, the tree control
   * will scroll vertically, if possible, until it will be entirely
   * visible.
   * <pre>
   * IN:  (bool)false
   * OUT: (bool)true if tree should scroll
   * </pre>
   */
  cscmdTreeItemScrollVertically,
  /**
   * Set horizontal offset for a tree item
   * <pre>
   * IN: (int)deltaX
   * </pre>
   */
  cscmdTreeItemSetHorizOffset,
  /**
   * Query first selected item ID. Handy for non-multiple-select tree controls.
   * <pre>
   * OUT: (csTReeItem *)item
   * </pre>
   */
  cscmdTreeQueryFirstSelected,
  /**
   * Select first item that exactly matches the text.
   * <pre>
   * IN: (char *)text
   * OUT: (csTreeItem *)item (or NULL if not found)
   * </pre>
   */
  cscmdTreeSelectItem,
  /**
   * Make Item2 child of Item1
   * <pre>
   * IN: csComponent* [2] = { Item1, Item2 }
   * OUT: true/false
   * </pre>
   */
  cscmdTreeAddChild,
  /**
   * Remove child item from its parent
   * <pre>
   * IN: (csComponent*)Item
   * OUT: NULL
   * </pre>
   */
  cscmdTreeRemoveChild
};

#define CS_TREEITEMCHECK_SELECTED	0xdeadface
#define CS_TREEITEMCHECK_UNSELECTED	0x0badf00d

/**
 * TRee control class is a rectangle which contains a number of tree
 * items. Tree can have only one selected item at a time (if
 * CSTS_MULTIPLESEL style is not specified) or have multiple selected
 * items at a time (if that style flag is set).
 */

class csTreeCtrl : public csComponent
{
  class TreeCtrlNode : public csTreeNode{
  public:
    TreeCtrlNode (csComponent *theItem, TreeCtrlNode *theParent, bool isOpen) : csTreeNode (theParent)
      { item = theItem; open = isOpen; }
    ~TreeCtrlNode () { csComponent *c=item; item=NULL; if (c) delete c;}
    
    TreeCtrlNode *FindItem (csComponent *theItem);
    
    TreeCtrlNode *Next (TreeCtrlNode *after=NULL);
    TreeCtrlNode *Prev (TreeCtrlNode *before=NULL);
    csComponent *item;
    bool open;
  };

  TreeCtrlNode *treeroot;
  /// Tree style
  int TreeStyle;
  /// Tree frame style
  csTreeFrameStyle FrameStyle;
  /// Tree frame width and height
  int BorderWidth, BorderHeight;
  /// First tree item
  csComponent *first;
  /// First visible tree item
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
  /// indentation size for subbranches
  int branchdeltax;

public:
  /// Create input line object
  csTreeCtrl (csComponent *iParent, int iStyle = CSTS_DEFAULTVALUE,
    csTreeFrameStyle iFrameStyle = cstfsThickRect);

  virtual ~csTreeCtrl () { delete treeroot; }
  /// Handle external events and generate timeouts
  virtual bool HandleEvent (csEvent &Event);

  /// Draw the tree
  virtual void Draw ();

  /// Find a place for each menu item
  void PlaceItems (bool setscrollbars = true);

  /// Resize child items on parent window resize
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /**
   * For each tree item call a function with a optional arg
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

  /// Open an item 
  virtual void OpenItem (csComponent *item);

  /// Open all items
  virtual void OpenAll ();

  /// Collapse an item 
  virtual void CollapseItem (csComponent *item);

  /// Collapse all
  virtual void CollapseAll ();

  /// Closes or opens the item
  void SwitchOpenState (csComponent *item);

  /// Set branch indentation
  void SetBranchIndent (int x);

  /// retrieve all selected tree items
  void FindAllSelected (csVector *list);

protected:
  /// Make a tree item visible (same as cscmdTreeMakeVisible)
  void MakeItemVisible (csComponent *item);

  /// Make item2 the child of item1, returns false if item1 was not in tree
  bool AddChild (csComponent *item1, csComponent *item2);

  /// Removes an item from the tree ( and all its children )
  void RemoveChild (csComponent *item);

  /// Get the last in the set of opened branches
  csComponent *GetLast ();
  
  csComponent *FindFirstSelected ();

  /// Draw the branches of the tree
  void DrawBranches ();


};

#endif
