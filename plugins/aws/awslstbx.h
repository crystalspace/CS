/*
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_AWS_LSTBX_H__
#define __CS_AWS_LSTBX_H__

#include "awsPanel.h"
#include "awsscr.h"
#include "awsscrbr.h"
#include "csutil/parray.h"

class awsListRowVector;

/**
 * Knows how to draw items that have several properties.
 */
struct awsListItem
{
  /// An image, if it contains one.
  iTextureHandle *image;

  /// The text string, if it has one.
  iString *text;

  /// User supplied parameter.
  int param;
  
  /// The state of the item, if it has one.
  bool state;

  /// Whether or not the item is stateful.
  bool has_state;

  /**
   * Whether this is a group state item or not (like radio-buttons vs
   * checkboxes).
   */
  bool group_state;

  /// Alignment of text (left or right) and stateful box (if one).
  int txt_align;

  /// Alignment of image (left or right).
  int img_align;

  /// Draws item.
  void DrawItem (iAws *wmgr, csRect frame);

  /// Tells what the minimum height of this item needs to be.
  int GetHeight (iAwsPrefManager *pm);

  /// Destroys this particular list item.
  ~awsListItem();
};

/**
 * Manages a row of items.
 */
struct awsListRow
{
  /// Pointer to parent (if it has one).
  awsListRow *parent;

  /// Pointer to list of children (if there are some).
  awsListRowVector *children;

  /// Pointer to columns of items in this row.
  awsListItem *cols;

  /// If this is selectable.
  bool selectable;

  /// If this is expanded (tree control).
  bool expanded;

  /// Gets the minimum height of the row, does NOT recurse for children.
  int GetHeight (iAwsPrefManager *pm, int colcount);

  /// Destroys a list row properly (but not it's children).
  ~awsListRow ();
};

/**
 * Holds a vector of awsListRows.
 */
class awsListRowVector : public csPDelArray<awsListRow>
{
public:
  int local_sortcol;
  static int sortcol;

  awsListRowVector () : local_sortcol (0) { }

  /// Compare two array elements in given Mode.
  static int Compare (awsListRow* const&, awsListRow* const&);

  /// Compare entry with a key.
  static int CompareKey (awsListRow* const&, iString* const& Key);

  /// Return a functor wrapping CompareKey() for a given string.
  static csArrayCmp<awsListRow*,iString*> KeyCmp(iString* s)
  { return csArrayCmp<awsListRow*,iString*>(s, CompareKey); }

  /// Set the sort column.
  void SetSortCol (int sc) { local_sortcol = sc; }
};

/**
 * Manages a column of items.
 */
struct awsListColumn
{
  /// The image for the header, if it has one.
  iTextureHandle *image;

  /// The background for the header, if it has one.
  iTextureHandle *bkg;

  /// The caption of the header, if it has one.
  iString *caption;

  /// Alignment.
  int align;

  /// The width of this column.
  int width;
};

/**
 * Manages clickable parts of the listbox.
 */
struct awsListHotspot
{
  /// The region that contains the hotspot.
  csRect r;

  /// The owner of the hotspot (either an item or row).
  void *obj;

  /// The type of hotspot (tree box, stateful item, etc.).
  int type;
};

/**
 * The actual listbox control that puts all this stuff together.
 */
class awsListBox : public awsPanel
{
private:
  /// True when button is down, false if up.
  bool is_down;

  /// True if the component has the mouse over it.
  bool mouse_is_over;

  /// True if this acts as a push-button switch (like tool-bar mode).
  bool is_switch;

  /// True if button was down, and button is in switch mode (toggle = yes).
  bool was_down;

  /// Holds the highlight overlay texture.
  iTextureHandle *highlight;

  /// Image of a collapsed tree box.
  iTextureHandle *tree_collapsed;

  /// Image of an expanded tree box.
  iTextureHandle *tree_expanded;

  /// Image of horizontal "line".
  iTextureHandle *tree_hline;

  /// Image of vertical "line".
  iTextureHandle *tree_vline;

  /// Image of checkbox type item (unmarked).
  iTextureHandle *tree_chke;

  /// Image of checkbox type item (marked).
  iTextureHandle *tree_chkf;

  /// Image of radio button type item (unmarked).
  iTextureHandle *tree_grpe;

  /// Image of radio button type item (marked).
  iTextureHandle *tree_grpf;

  /// Alpha level for highlight bitmap.
  int hi_alpha_level;

  /// Type of control (list/tree/etc).
  int control_type;

  /// Column to sort by.
  int sortcol;

  /// Number of columns to deal with.
  int ncolumns;

  /// Column container (always static).
  awsListColumn *columns;

  /// Row container (grows and shrinks as items are manipulated).
  awsListRowVector rows;

  /// Container of hotspots.
  csPDelArray<awsListHotspot> hotspots;

  /// Currently selected row.
  awsListRow *sel;

  /// Row to item-number mapping.
  awsListRow **map;

  /// Size of item-number map.
  int map_size;

  /// True if item-number map needs refreshed.
  bool map_dirty;

  /// Position of where we start drawing.
  int scroll_start;

  /// Number of items that can be drawn.
  int drawable_count;

  /// The sink that gets scroll notifications.
  iAwsSink *sink;

  /// The slot that handles scroll notifications.
  iAwsSlot *slot;

  /// Our embedded scrollbar.
  awsScrollBar *scrollbar;

  /// Action dispatcher.
  awsActionDispatcher* actions;

  /// Trigger for catching scroll signals.
  static void ScrollChanged (void *sk, iAwsSource *source);

  /// Inserts an item.
  static void InsertItem (void *owner, iAwsParmList* parmlist);

  /// Deletes an item.
  static void DeleteItem (void *owner, iAwsParmList* parmlist);

  /// Get the selected item.
  static void GetSelectedItem (void *owner, iAwsParmList* parmlist);

  /// Get an item by row number.
  static void GetItem (void *owner, iAwsParmList* parmlist);

  /// Clears the entire list.
  static void ClearList (void *owner, iAwsParmList* parmlist);

  /**
   * Counts the number of visible items recursively, given the starting vector.
   */
  static int CountVisibleItems (awsListRowVector *v);

  /// Maps the visible items in the list recursively onto a flat index.
  static void MapVisibleItems (
    awsListRowVector *v,
    int &start,
    awsListRow **map);
protected:
  /// Updates the row-map if it's dirty.
  void UpdateMap ();

  /// Used, but shouldn't be (remnant of radio-button code).
  void ClearGroup ();

  /// Used internally to clear group items, called by ClearPeers.
  bool RecursiveClearPeers (awsListItem *itm, awsListRow *row);

  /// Used internally to clear peers of some item.
  void ClearPeers (awsListItem *itm);

  /// Used internally to reset the hotspots list.
  void ClearHotspots ();

  /// Find out how depp this row is.
  int GetRowDepth (awsListRow *row);

  /// get requested items for this row.
  bool GetItems (awsListRow *row, iAwsParmList* parmlist);

  /// Find out if this row is the last child in it's parent's list.
  bool IsLastChild (awsListRow *row);

  /**
   * Used internally to redraw the list recursively (support tree/hierarchical
   * drawing).
   */
  bool DrawItemsRecursively (
    awsListRow *row,
    int &x,
    int &y,
    int border,
    int depth,
    bool last_child);
public:
  awsListBox ();
  virtual ~awsListBox ();

  /// A multi-column list.
  static const int ctList;

  /// A multi-column tree.
  static const int ctTree;

  /// An item was selected.
  static const int signalSelected;

  /// The box was scrolled programmatically, or by a keypress.
  static const int signalScrolled;

  /// Component becomes focused.
  static const int signalFocused;

  /// The state of a stateful column was changed.
  static const int signalStateChanged;

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Get properties.
  bool GetProperty (const char *name, void **parm);

  /// Set properties.
  bool SetProperty (const char *name, void *parm);

  /// Executes some actions.
  bool Execute (const char *action, iAwsParmList* parmlist);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  bool HandleEvent (iEvent &Event);

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down.
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when this component loses mouse focus.
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus.
  virtual bool OnMouseEnter ();

  /// Triggered when the user presses a key.
  virtual bool OnKeyboard (const csKeyEventData& eventData);

  /// Triggered when the component becomes focused.
  virtual void OnSetFocus ();

  /**
   * Updates the scrollbar so that it's in the right place and has the
   * right stuff.
   */
  virtual void OnAdded ();

  /// Updates the scrollbar so that it's in the right place, etc.
  virtual void OnResized ();
};

class awsListBoxFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with
   * the window manager.
   */
  awsListBoxFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsListBoxFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_LSTBX_H__
