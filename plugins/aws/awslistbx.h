#ifndef __AWS_LIST_BOX_H__
#define __AWS_LIST_BOX_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson

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
*****************************************************************************/
#include "awscomp.h"
#include "awsitmv.h"
#include "awsscr.h"
#include "awsscrbr.h"

/// Knows how to draw items that have several properties.
struct awsListItem
{
  /// An image, if it contains one.
  iTextureHandle *image;

  /// The text string, if it has one.
  iString *text;

  /// The state of the item, if it has one
  bool state;

  /// Whether or not the item is stateful
  bool has_state;

  /// Whether this is a group state item or not (like radio-buttons vs. checkboxes)
  bool group_state;

  /// Alignment of text (left or right) and stateful box (if one)
  int txt_align;

  /// Alignment of image (left or right)
  int img_align;

  /// Draws item
  void DrawItem(iAws *wmgr, csRect frame);

  /// Tells what the minimum height of this item needs to be
  int GetHeight(iAwsPrefManager *pm);

  /// Destroys this particular list item.
  ~awsListItem();
};

/// Manages a row of items
struct awsListRow
{
  /// Pointer to parent (if it has one)
  awsListRow *parent;

  /// Pointer to list of children (if there are some)
  awsListRowVector *children;

  /// Pointer to columns of items in this row
  awsListItem *cols;

  /// If this is selectable
  bool selectable;

  /// If this is expanded (tree control)
  bool expanded;

  /// Gets the minimum height of the row, does NOT recurse for children.
  int GetHeight(iAwsPrefManager *pm, int colcount);

  /// Destroys a list row properly ( but not it's children. )
  ~awsListRow();
};

/// Manages a column of items
struct awsListColumn
{
  /// The image for the header, if it has one.
  iTextureHandle *image;

  /// The background for the header, if it has one.
  iTextureHandle *bkg;

  /// The caption of the header, if it has one.
  iString *caption;

  /// Alignment
  int align;

  /// The width of this column
  int width;
};

/// Manages clickable parts of the listbox
struct awsListHotspot
{
  /// The region that contains the hotspot
  csRect r;

  /// The owner of the hotspot (either an item or row)
  void  *obj;

  /// The type of hotspot (tree box, stateful item, etc.)
  int type;
};


/// The actual listbox control that puts all this stuff together.
class awsListBox : public awsComponent
{
   /// True when button is down, false if up
   bool is_down;

   /// True if the component has the mouse over it
   bool mouse_is_over;

   /// True if this acts as a push-button switch (like tool-bar mode)
   bool is_switch;

   /// True if button was down, and button is in switch mode (toggle=yes)
   bool was_down;

   /// Holds the background texture (either the global one, or an overridden one
   iTextureHandle *bkg;

   /// Holds the highlight overlay texture
   iTextureHandle *highlight;

   /// Image of a collapsed tree box
   iTextureHandle *tree_collapsed;

   /// Image of an expanded tree box
   iTextureHandle *tree_expanded;

   /// Image of horizontal "line"
   iTextureHandle *tree_hline;

   /// Image of vertical "line"
   iTextureHandle *tree_vline;

   /// Image of checkbox type item (unmarked)
   iTextureHandle *tree_chke;

   /// Image of checkbox type item (marked)
   iTextureHandle *tree_chkf;

   /// Image of radio button type item (unmarked)
   iTextureHandle *tree_grpe;

   /// Image of radio button type item (marked)
   iTextureHandle *tree_grpf;

  //////////////////////////////////////////

   /// Flags for frame style.
   int frame_style;

   /// Alpha level for this component
   int alpha_level;

   /// Alpha level for highlight bitmap.
   int hi_alpha_level;

   /// Type of control (list/tree/etc)
   int control_type;

   /// Column to sort by
   int sortcol;

   /// Number of columns to deal with
   int ncolumns;

   /// Column container (always static)
   awsListColumn *columns;

   /// Row container (grows and shrinks as items are manipulated)
   awsListRowVector rows;

   /// Container of hotspots
   csBasicVector hotspots;

   /// Currently selected row
   awsListRow *sel;

   /// Row to item-number mapping
   awsListRow **map;

   /// Size of item-number map
   int map_size;

   /// True if item-number map needs refreshed
   bool map_dirty;

   /// Position of where we start drawing
   int scroll_start;

   /// Number of items that can be drawn
   int drawable_count;

private:
   ///////////////////////////// Embedded items

   /// The sink that gets scroll notifications
   iAwsSink *sink;

   /// The slot that handles scroll notifications
   iAwsSlot *slot;

   /// Our embedded scrollbar
   awsScrollBar *scrollbar;

   /// Trigger for catching scroll signals
   static void ScrollChanged(void *sk, iAwsSource *source);

   ///////////////////////////// Actions

   /// Action dispatcher
   awsActionDispatcher actions;

   /// Inserts an item
   static void InsertItem(void *owner, iAwsParmList &parmlist);

   /// Deletes an item
   static void DeleteItem(void *owner, iAwsParmList &parmlist);

   /// Inserts an item
   static void GetSelectedItem(void *owner, iAwsParmList &parmlist);

   /// Clears the entire list
   static void ClearList(void *owner, iAwsParmList &parmlist);

   //////////////////////////// Static member helper functions

   /// Counts the number of visible items recursively, given the starting vector.
   static int CountVisibleItems(awsListRowVector *v);

   /// Maps the visible items in the list recursively onto a flat index.
   static void MapVisibleItems(awsListRowVector *v, int &start, awsListRow **map);


protected:
   /// Updates the row-map if it's dirty
   void UpdateMap();

   /// Used, but shouldn't be (remnant of radio-button code)
   void ClearGroup();

   /// Used internally to clear group items, called by ClearPeers
   bool RecursiveClearPeers(awsListItem *itm, awsListRow *row);

   /// Used internally to clear peers of some item.
   void ClearPeers(awsListItem *itm);

   /// Used internally to reset the hotspots list.
   void ClearHotspots();

   /// Find out how depp this row is
   int GetRowDepth(awsListRow *row);

   /// Find out if this row is the last child in it's parent's list
   bool IsLastChild(awsListRow *row);

   /// Used internally to redraw the list recursively (support tree/hierarchical drawing)
   bool DrawItemsRecursively(awsListRow *row, int &x, int &y, int border, int depth, bool last_child);

public:
    awsListBox();
    virtual ~awsListBox();

   /******* Frame Styles **********************/

   /// A bumpy frame, like Group Frame
   static const int fsBump;

   /// A sunken frame
   static const int fsSunken;

   /// A raised frame
   static const int fsRaised;

   /// A simple frame
   static const int fsSimple;

   /// No frame is drawn
   static const int fsNone;

   /******* Control Types **********************/

   /// A multi-column list
   static const int ctList;

   /// A multi-column tree
   static const int ctTree;

   /******* Signals **********************/

   /// An item was selected
   static const int signalSelected;

   /// The box was scrolled programmatically, or by a keypress.
   static const int signalScrolled;


public:
    /// Get's the texture handle and the title, plus style if there is one.
    virtual bool Setup(iAws *wmgr, awsComponentNode *settings);

    /// Gets properties
    bool GetProperty(char *name, void **parm);

    /// Sets properties
    bool SetProperty(char *name, void *parm);

    /// Executes some actions
    bool Execute(char *action, iAwsParmList &parmlist);

    /// Returns the named TYPE of the component, like "Radio Button", etc.
    virtual char *Type();

public:
    SCF_DECLARE_IBASE;

    bool HandleEvent(iEvent& Event);

    /// Triggered when the component needs to draw
    virtual void OnDraw(csRect clip);

    /// Triggered when the user presses a mouse button down
    virtual bool OnMouseDown(int button, int x, int y);

    /// Triggered when the user unpresses a mouse button
    virtual bool OnMouseUp(int button, int x, int y);

    /// Triggered when the user moves the mouse
    virtual bool OnMouseMove(int button, int x, int y);

    /// Triggered when the user clicks the mouse
    virtual bool OnMouseClick(int button, int x, int y);

    /// Triggered when the user double clicks the mouse
    virtual bool OnMouseDoubleClick(int button, int x, int y);

    /// Triggered when this component loses mouse focus
    virtual bool OnMouseExit();

    /// Triggered when this component gains mouse focus
    virtual bool OnMouseEnter();

    /// Triggered when the user presses a key
    virtual bool OnKeypress(int key, int modifiers);

    /// Triggered when the keyboard focus is lost
    virtual bool OnLostFocus();

    /// Triggered when the keyboard focus is gained
    virtual bool OnGainFocus();

    /// Updates the scrollbar so that it's in the right place and has the right stuff.
    virtual void OnAdded();
};

class awsListBoxFactory : public awsComponentFactory
{
public:
    SCF_DECLARE_IBASE;

    /// Calls register to register the component that it builds with the window manager
    awsListBoxFactory(iAws *wmgr);

    /// Does nothing
    virtual ~awsListBoxFactory();

    /// Returns a newly created component of the type this factory handles.
    virtual iAwsComponent *Create();
};

#endif

