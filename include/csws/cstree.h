/*
    Crystal Space Windowing System: tree box class
    Copyright (C) 2000 by Norman Kraemer, based on the listbox code:
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

#ifndef __CS_CSTREE_H__
#define __CS_CSTREE_H__

/**\file
 * Crystal Space Windowing System: tree box class
 */

/**
 * \addtogroup csws_comps_treebox
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"
#include "csscrbar.h"

/**
 * Tree control - specific messages
 */
enum
{
  /**
   * Check if a component is indeed a tree item.
   * <pre>
   * IN:  0 (nothing)
   * OUT: CS_TREEITEM_MAGIC if it is a tree item.
   * </pre>
   */
  cscmdTreeItemCheck = 0x00000e00,
  /**
   * Toggle the tree item (expand/collapse)
   * <pre>
   * IN:  Action to perform: 0 - collapse, 1 - expand, other - toggle
   * OUT: current state (0 - collapsed, 1 - expanded)
   * </pre>
   */
  cscmdTreeItemToggle,
  /**
   * Toggle the tree item and all child items in the subtree.
   * <pre>
   * IN:  Action to perform: 0 - collapse, 1 - expand
   * </pre>
   */
  cscmdTreeItemToggleAll,
  /**
   * Tree item state change notification. This event is sent to parent
   * before the actual change happens; you still have a chance to forbid
   * the tree item to change its state. This event is re-sent by tree
   * items to their parents, so your dialog will always receive these
   * messages, even if they are emmited by deep tree items.
   * <pre>
   * IN:  (csTreeItem *)item;
   * OUT: CS_TREEITEM_MAGIC to forbid changing state
   * </pre>
   */
  cscmdTreeItemToggleNotify,
  /**
   * Notify parent treebox that the size (notably height) of the item
   * has been changed, and all the tree items should be re-positioned.
   * <pre>
   * IN:  (csTreeItem *)item;
   * </pre>
   */
  cscmdTreeItemSizeChangeNotify,
  /**
   * This message is sent by a tree to notify its parent that a item
   * has been focused.
   * <pre>
   * IN: (csTreeItem *)Item;
   * </pre>
   * To find the parent of the item, check Item->parent; this can be
   * either another tree item or the tree box. To clarify this, send
   * the cscmdTreeItemCheck message to the item in question.
   */
  cscmdTreeItemFocused,
  /**
   * Query what's the previous of a given item.
   * The 'previous' notion is defined as follows: if this is the first
   * child tree item, return its parent; otherwise if his previous neightbour
   * is closed, return it; otherwise return the last child tree item from the
   * "last->opened ? last->last : last" chain.
   * If there is no previous item (top of the tree), it returns 0.
   * <pre>
   * OUT: (csTreeItem *)PrevItem
   * </pre>
   */
  cscmdTreeItemGetPrev,
  /**
   * Query what's the next of a given item.
   * The 'next' notion is defined as follows: if the item is opened,
   * return its first child item; otherwise if it is the last item
   * in the subtree, return the item following its parent; otherwise
   * return the next item in the branch.
   * If there is no next item (bottom of the tree), it returns 0.
   * <pre>
   * OUT: (csTreeItem *)NextItem
   * </pre>
   */
  cscmdTreeItemGetNext,
  /**
   * Ask the parent for its first tree item.
   * <pre>
   * OUT: (csTreeItem *)Item
   * </pre>
   */
  cscmdTreeItemGetFirst,
  /**
   * Ask the parent for its last tree item.
   * <pre>
   * OUT: (csTreeItem *)Item
   * </pre>
   */
  cscmdTreeItemGetLast,
  /**
   * Sent to parent treebox to notify that a tree item is being deleted.
   * <pre>
   * IN: (csTreeItem *)item;
   * </pre>
   */
  cscmdTreeItemDeleteNotify,
  /**
   * Sent to treebox parent to notify it that a tree item has been clicked
   * with the right mouse button.
   * <pre>
   * IN: (csTreeItem *)item;
   * </pre>
   */
  cscmdTreeItemRightClick,
  /**
   * Query the selected tree item.Note that the parent of item
   * is not neccessarily the tree box (it can be a level N child).
   * <pre>
   * OUT: (csTreeItem *)item
   * </pre>
   */
  cscmdTreeQuerySelected,
  /**
   * Select first item that exactly matches the text.
   * <pre>
   * IN: (char *)text
   * OUT: (csTreeItem *)item (or 0 if not found)
   * </pre>
   */
  cscmdTreeSelectItem,
  /**
   * Set horizontal offset for all tree items (horizontal scrolling)
   * <pre>
   * IN: (int)deltaX
   * </pre>
   */
  cscmdTreeSetHorizOffset,
  /**
   * This message is sent by a tree item to its parent when it receives
   * a 'mouse down' event.
   * <pre>
   * IN:  (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeStartTracking,
  /**
   * This message is sent by a tree item to its parent when the mouse
   * is captured and moved over a unfocused tree item, so that the
   * csTreeBox component can check whenever the item in question should
   * be highlighted.
   * <pre>
   * IN:  (csTreeItem *)source;
   * </pre>
   */
  cscmdTreeTrack,
  /**
   * This command tells to a Tree control object to make given item visible
   * (and scroll the view if it is not inside the current viewport).
   * <pre>
   * IN: (csTreeItem *)item;
   * </pre>
   */
  cscmdTreeMakeVisible,
  /**
   * Same as cscmdTreeMakeVisible but tells to make visible the entire
   * branch (subtree) rather than just the branch, if possible. If the
   * branch is too high, the top margin will placed be at the top of the
   * view, and the bottom margin will be clipped.
   * <pre>
   * IN: (csTreeItem *)item;
   * </pre>
   */
  cscmdTreeMakeBranchVisible,
  /**
   * Completely clear a tree.
   */
  cscmdTreeClear
};

/**
 * \name Tree item state flags
 * @{ */
/// Additional state flag to mark open branches
#define CSS_TREEITEM_OPEN	0x00010000
/// Child tree items should be re-positioned
#define CSS_TREEITEM_PLACEITEMS	0x00020000
/** @} */

/// The magic answer that means that the component is indeed a tree item
#define CS_TREEITEM_MAGIC	(intptr_t)0xdeadface

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

/**
 * This class encapsulates a tree item. The tree item always contains
 * an "expand/collapse" button, optionally contains two pixmaps (shown
 * depending on the expanded state), and a text string. The button is
 * displayed only if tree has child items. Also the tree item can contain
 * any number of child (subordinate) tree items. Note that the tree item
 * can contain ONLY other csTreeItem's and their derivates; the only
 * exception is the button component which is handled specialy. Inserting
 * something other than a csTreeItem into another csTreeItem will crash,
 * since csTreeItem often does unconditional typecasts to csTreeItem.
 */
class CS_CSWS_EXPORT csTreeItem : public csComponent
{
  friend class csTreeBox;

  /// Tree item style
  csTreeItemStyle ItemStyle;
  /// Tree item images (for closed(0) and open(1) states)
  csPixmap *ItemBitmap [2];
  /// Delete bitmap on object deletion?
  bool DeleteBitmap;
  /// Horizontal offset of child items
  int hChildrenOffset;
  /// The expand/collapse button
  csButton *button;
  /// The tree box control
  csTreeBox *treebox;

  /// Place all child tree items
  void PlaceItems ();
  /// Return the next item after this one
  csTreeItem *NextItem ();
  /// Return the item preceeding this one
  csTreeItem *PrevItem ();

public:
  /// Tree item constructor: text item with optional style
  csTreeItem (csComponent *iParent, const char *iText, int iID = 0,
    csTreeItemStyle iStyle = cstisNormal);

  /// Tree item destructor
  virtual ~csTreeItem ();

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

  /// Draw the tree item
  virtual void Draw ();

  /// Handle additional state flags
  virtual void SetState (int mask, bool enable);

  /// Invalidate ourselves when the focus goes to one of our children
  virtual bool SetFocused (csComponent *comp);

  /// Tell parent to re-position items
  virtual void Insert (csComponent *comp);

  /// Tell parent to re-position items
  virtual void Delete (csComponent *comp);

  /// Report the minimal size of tree item
  virtual void SuggestSize (int &w, int &h);

  /// Report the minimal size of tree item and total size with children
  void SuggestTotalSize (int &w, int &h, int &totw, int &toth);

  /// Set tree item image (possibly for open state too)
  void SetBitmap (csPixmap *iBitmap, csPixmap *iBitmapOpen = 0,
    bool iDelete = true);

  /// Set horizontal offset of child items
  void SetChildOffset (int ihOffset)
  { hChildrenOffset = ihOffset; }

  /// Toggle the open state of the branch: 0: collapse, 1: expand, other: toggle
  int Toggle (int iAction = 2);

  /**
   * For each subtree item call a function with a optional arg
   * Function returns the first child on which func returnes 'true'
   * Optionally you can pass an "only for opened branches" flag,
   * so that only visible branches will be handled.
   */
  csTreeItem *ForEachItem (bool (*func) (csTreeItem *child, intptr_t param),
    intptr_t param = 0, bool iOnlyOpen = false);

  /// Force a reset of button size & position
  void ResetButton ()
  {
    button->SetRect (0, 0, -1, -1);
    parent->SendCommand (cscmdTreeItemSizeChangeNotify, (intptr_t)this);
  }
};

/**
 * \name Tree control styles
 * These are bit masks which can be ORed together to form a final value sent 
 * to the csTreeBox constructor.
 * @{ */
/// Tree has a horizontal scroll bar
#define CSTS_HSCROLL		0x00000001
/// Tree has a vertical scroll bar
#define CSTS_VSCROLL		0x00000002
/// Automatically hide scrollbars if they are not needed
#define CSTS_AUTOSCROLLBAR	0x00000004
/// Tree items have small expand/collapse buttons
#define CSTS_SMALLBUTTONS	0x00000008

/// default tree control style
#define CSTS_DEFAULTVALUE	CSTS_VSCROLL | CSTS_AUTOSCROLLBAR
/** @} */

/**
 * \name Tree box state flags
 * @{ */
/// Child tree items should be re-positioned
#define CSS_TREEBOX_PLACEITEMS	0x00010000
/** \internal
 * Temporarily ignore cscmdTreeMakeVisible commands (used internally)
 */
#define CSS_TREEBOX_LOCKVISIBLE	0x00020000
/** @} */

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
 * The csTreeBox class is a rectangle which contains a number of subrectangles,
 * each of which is recursively similar to its parent. In other words, every
 * tree item can contain a subtree itself. In very other words, the tree is
 * a graph, every vertex of which has one incoming and several outgoing edges.
 * The csTreeBox control can hold several trees at once (e.g. it can have
 * several "roots"). csTreeBox control ALWAYS contains just one selected
 * item at a time; multiple selection mode is not implemented (I don't see
 * why this may be useful anyway).
 *<p>
 * Every csTreeItem that is inserted into the tree is queried for its preferred
 * size (in particular its preffered height), and the next item is positioned
 * at the bottom of every previous item. Every csTreeItem asks in turn all his
 * child nodes (if the tree node is "open") or just returns its own height
 * without child nodes.
 *<p>
 * Example code how to create a tree:
 *<pre>
 * csTreeBox *tree = new csTreeBox (app);
 * tree->SetRect (0, 0, 200, 400);
 * csTreeItem *item1, *item2, *item3;
 * item1 = new csTreeItem (tree, "My Computer");
 *   item2 = new csTreeItem (item1, "C:\\");
 *     item3 = new csTreeItem (item2, "Blindows");
 *     item3 = new csTreeItem (item2, "Suxx");
 *   item2 = new csTreeItem (item1, "D:\\");
 *     item3 = new csTreeItem (item2, "My Documents");
 *     item3 = new csTreeItem (item2, "My Toys");
 *     item3 = new csTreeItem (item2, "My Mom Told Me - Dont Run Windows");
 *   item2 = new csTreeItem (item1, "\\\\SAMBA\\RULEZ\\FOREVER\\");
 *     item3 = new csTreeItem (item2, "Home directory");
 *     item3 = new csTreeItem (item2, "Public stuff");
 *</pre>
 * Keyboard navigation:
 * <ul>
 *   <li>Up/Down -- Select previous/next tree item</li>
 *   <li>Left/Right -- Scroll tree horizontally (if meaningful)</li>
 *   <li>Ctrl+Left/Right -- Scroll horizontally in big steps</li>
 *   <li>PgUp/PgDn -- Go to the previous/next page of the tree.</li>
 *   <li>Shift+PgUp/PgDn -- Show the previous/next page of the tree
 *     (does not move the caret).</li>
 *   <li>Ctrl+PgUp/PgDn -- Go to the first/last tree item</li>
 *   <li>Home/End -- Scroll to left/right margin.</li>
 *   <li>Ctrl+Home/End -- Scroll to the beginning/end of the tree
 *     (does not move the caret).</li>
 *   <li>Keypad PLUS/MINUS -- Expand/collapse current branch.</li>
 *   <li>Ctrl+Keypad PLUS/MINUS -- Expand/collapse all branches
 *     at once.</li>
 *   <li>Shift+Keypad PLUS/MINUS -- Expand/collapse all the items
 *     contained in the current branch.</li>
 *   <li>Space -- Toggle expand/collapse current branch</li>
 *   <li>Any other symbols -- Find the next item that starts with
 *     given character. For example, pressing 'a' will find the first
 *     item following current which starts with 'A' or 'a'; if there
 *     is no one until the end of tree, the search is restarted from
 *     top; if there is no such item at all, the cursor stays still.</li>
 * </ul>
 */
class CS_CSWS_EXPORT csTreeBox : public csComponent
{
  /**
   * A private class used to insert all tree items into.
   * This is used since we need to not allow any tree item
   * to paint on top of treebox border or scrollbars; thus
   * this class. The hierarchy of a treebox looks this way:
   * <pre>
   * csTreeBox
   *   +--- csTreeBox::csTreeView
   *          +--- Item 1
   *          |      +--- Item 1:1
   *          |      +--- Item 1:2
   *          |      +--- ...
   *          +--- Item 2
   *          ...
   * </pre>
   * This component should contain ONLY and EXCLUSIVELY csTreeItem's
   * and their derivates; inserting something other will lead to crash
   * since csTreeBox often uses unconditional typecasts to csTreeItem.
   */
  class csTreeView : public csComponent
  {
  public:
    /// Constructor
    csTreeView (csComponent *iParent);

    /// Resend all command events to parent (csTreeBox)
    virtual bool HandleEvent (iEvent &Event);

    /// Set parent's CSS_TREEBOX_PLACEITEMS since a item has been removed
    virtual void Delete (csComponent *comp);
  } *clipview;

  /// Tree style
  int TreeStyle;
  /// Tree frame style
  csTreeFrameStyle FrameStyle;
  /// Tree frame width and height
  int BorderWidth, BorderHeight;
  /// The timer
  csComponent *timer;
  /// The scroll bars
  csScrollBar *hscroll, *vscroll;
  /// Status of both scroll bars
  csScrollBarStatus hsbstatus, vsbstatus;
  /// Horizontal scrolling position & maximum width of all visible tree items
  int deltax, maxdeltax;
  /// Vertical scrolling position & total height of all the tree items
  int deltay, maxdeltay;
  /// Active tree item
  csTreeItem *active;

  /// Return the next item after this one
  csTreeItem *NextItem ();
  /// Return the item preceeding this one
  csTreeItem *PrevItem ();

public:
  /// Create input line object
  csTreeBox (csComponent *iParent, int iStyle = CSTS_DEFAULTVALUE,
    csTreeFrameStyle iFrameStyle = cstfsThickRect);

  /// Destroy the tree box
  virtual ~csTreeBox ();

  /// Handle external events and generate timeouts
  virtual bool HandleEvent (iEvent &Event);

  /// Draw the tree
  virtual void Draw ();

  /// Find a place for each tree item, and optionally set scrollbar parameters
  void PlaceItems (int sbFlags = CSTS_HSCROLL | CSTS_VSCROLL);

  /// Resize child items on parent window resize
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /**
   * For each tree item call a function with a optional arg
   * Function returns the first child on which func returnes 'true'
   */
  csTreeItem *ForEachItem (bool (*func) (csTreeItem *child, intptr_t param),
    intptr_t param = 0, bool iOnlyOpen = false);

  /// Override SetState method to toggle scrollbars together with CSS_SELECTED
  virtual void SetState (int mask, bool enable);

  /// Set fPlaceItems since a item has been inserted
  virtual void Insert (csComponent *comp);

  /// Expand all items
  virtual void ExpandAll ();

  /// Collapse all items
  virtual void CollapseAll ();

  /// Query tree box style
  int GetStyle () { return TreeStyle; }
  /// Query tree box frame style
  csTreeFrameStyle GetFrameStyle () { return FrameStyle; }
  /// Change tree box style
  void SetStyle (int iStyle, csTreeFrameStyle iFrameStyle);

protected:
  friend class csTreeItem;

  /// Set the imagess for expand/collapse button
  void PrepareButton (csButton *iButton, bool iOpen);

  /// Make a tree item visible (the functionality for cscmdTreeMakeVisible)
  void MakeItemVisible (csComponent *iItem, bool iChildren = false);

  /// Focus a item (and defocus all other items)
  void FocusItem (csTreeItem *iItem);

  /// Scroll vertically by iDelta pixels, possibly preserving caret's relative Y
  void VScroll (int iDelta, bool iMoveCaret);

  /// Place scrollbars and the csTreeView.
  void PlaceScrollbars ();
};

/** @} */

#endif // __CS_CSTREE_H__
