/*
    Crystal Space Windowing System: Windowing System Component interface
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSCOMP_H__
#define __CSCOMP_H__

#include "csutil/csbase.h"
#include "csgeom/csrect.h"
#include "csengine/csobjvec.h"
#include "csengine/csspr2d.h"
#include "cswspal.h"

class csApp;
class csEvent;
class csStrVector;

/// Component state flags

/// Component is visible
#define CSS_VISIBLE		0x00000001
/// Component is focused in parent window child list
#define CSS_FOCUSED		0x00000002
/// Component is disabled
#define CSS_DISABLED		0x00000004
/// Component can be selected
#define CSS_SELECTABLE		0x00010000
/// Component is the beginning of a group of components
#define CSS_GROUP		0x00020000
/// Move component to top Z-order when selected
#define CSS_TOPSELECT		0x00040000
/// Excldude component from clipping process
#define CSS_TRANSPARENT		0x00080000
/// Component is modally executing
#define CSS_MODAL		0x00100000

/**
 * csApp contains a static array with indexes of all colors
 * used in windowing system. They are mapped to nearest closest
 * match in physical palette.
 */
enum
{
  cs_Color_Black = 0,
  cs_Color_White,
  cs_Color_Gray_D,
  cs_Color_Gray_M,
  cs_Color_Gray_L,
  cs_Color_Blue_D,
  cs_Color_Blue_M,
  cs_Color_Blue_L,
  cs_Color_Green_D,
  cs_Color_Green_M,
  cs_Color_Green_L,
  cs_Color_Red_D,
  cs_Color_Red_M,
  cs_Color_Red_L,
  cs_Color_Cyan_D,
  cs_Color_Cyan_M,
  cs_Color_Cyan_L,
  cs_Color_Brown_D,
  cs_Color_Brown_M,
  cs_Color_Brown_L,
  cs_Color_Last,			// This should always be the last one
  // now just the aliases
  cs_Color_Yellow = cs_Color_Brown_L
};

/**
 * Predefined Windowing System Command Codes<p>
 * The list below does not contain all defined messages; these are only the
 * most general ones. Any class which defines some class-specific messages
 * should ensure that no other command is using the integer value of its
 * proprietary command codes. To avoid this as much as possible, the following
 * ranges are reserved:<p>
 * <ul>
 *   <li>0x00000000 ... 0x7FFFFFFF: Reserved for CrystalSpace Windowing System
 *       <ul>
 *         <li>0x00000000 ... 0x000000FF: Non-class specific commands
 *         <li>0x00000100 ... 0x000001FF: csWindow class messages
 *         <li>0x00000200 ... 0x000002FF: csMenu class messages
 *         <li>0x00000300 ... 0x000003FF: csTimer class messages
 *         <li>0x00000400 ... 0x000004FF: csListBox class messages
 *         <li>0x00000500 ... 0x000005FF: csButton class messages
 *         <li>0x00000600 ... 0x000006FF: csScrollBar class messages
 *         <li>0x00000700 ... 0x000007FF: csStatic class messages
 *         <li>0x00000800 ... 0x000008FF: csCheckBox class messages
 *         <li>0x00000900 ... 0x000009FF: csRadioButton class messages
 *         <li>0x00000A00 ... 0x00000AFF: csSpinBox class messages
 *       </ul>
 *   <li>0x80000000 ... 0xFFFFFFFF: Reserved for user class-specific messages
 * </ul>
 * All commands receives a input parameter in the Command.Info field of csEvent
 * object. They can reply to the message by assigning to Command.Info a value.
 * In the description of messages below they are marked by 'IN' (the value
 * is initially passed to object) and 'OUT' (the value is expected to be filled
 * in by the object) labels. If no IN or OUT labels are present, the value of
 * Command.Info is ignored. Since Command.Info is of type (void *) it should
 * be casted to appropiate type before filling/after reading.
 */
enum
{
  /**
   * Broadcasted before csApp::Process () begins to process current messages
   * in application message queue.
   */
  cscmdPreProcess = 0x100,
  /**
   * Broadcasted after csApp::Process () finished to process messages
   * in application message queue.
   */
  cscmdPostProcess,
  /**
   * This event is broadcasted to refresh invalidated components.
   */
  cscmdRedraw,
  /**
   * Query a control if it would like to be the default control in a dialog.<p>
   * The control is 'default' if it has a 'default' attribute (this is
   * control-specific, for example buttons have the CSBSTY_DEFAULT style).
   * <pre>
   * IN: NULL
   * OUT: (csComponent *) or NULL;
   * </pre>
   */
  cscmdAreYouDefault,
  /**
   * This message is sent by parent to its active child to activate
   * whatever action it does. For example, this message is sent by a
   * dialog window to its active child when user presses Enter key.
   * <pre>
   * IN: NULL
   * OUT: (csComponent *)this if successful;
   * </pre>
   */
  cscmdActivate,
  /**
   * This broadcast message is posted after system palette has been changed.
   * If class has looked up any colors in palette, it should redo it.
   */
  cscmdPaletteChanged,
  /**
   * The "hide window" command
   */
  cscmdHide,
  /**
   * The "maximize window" command
   */
  cscmdMaximize,
  /**
   * The "close window" button
   */
  cscmdClose,
  /**
   * This messages gives a chance to parent window to limit child's
   * "maximized" rectangle.
   * <pre>
   * IN: (csRect *)MaximizedRectangle
   * OUT: modify rectangle if needed
   * </pre>
   */
  cscmdLimitMaximize,
  /**
   * These commands are used for message boxes. MessageBox (...) returns
   * cscmdOK, cscmdCancel and so on depending on which button user presses.
   */
  cscmdOK,
  ///
  cscmdCancel,
  ///
  cscmdAbort,
  ///
  cscmdRetry,
  ///
  cscmdIgnore
};

/**
 * Drag mode flags: these flags are used by csComp::Drag to compute
 * new window coordinates when dragging window with mouse
 */
/// Drag left window border
#define CS_DRAG_XMIN		0x00000001
/// Drag right window border
#define CS_DRAG_XMAX		0x00000002
/// Drag top window border
#define CS_DRAG_YMIN		0x00000004
/// Drag bottom window border
#define CS_DRAG_YMAX		0x00000008
/// Window is moveable
#define CS_DRAG_MOVEABLE	0x00000010
/// Window is sizeable
#define CS_DRAG_SIZEABLE	0x00000020
/// All flags above combined (used when dragging with titlebar)
#define CS_DRAG_ALL	(CS_DRAG_XMIN | CS_DRAG_XMAX | \
			 CS_DRAG_YMIN | CS_DRAG_YMAX)

/**
 * Graphics system component: a menu, window etc.<p>
 * This is an abstract base class: all windowing system classes should be
 * subclassed from csComponent. Each component can have a number of child
 * components. Child components are chained together in a ring list; the only
 * case when a NULL can be encountered in this list is when component has
 * no children. When a component has at least one child and if you traverse the
 * child list you should take care to avoid looping forever through them.<p>
 *
 * A csComponent object is a rectangle area of screen which can contain
 * absolutely any content. The object is responsible for filling all
 * pixels within that rectangle, i.e. underlying windows will never
 * touch other window's area (and partially transparent windows like in
 * X Windows are impossible).
 */
class csComponent : public csBase
{
protected:
  /// Object state flags (see CSS_XXX flags)
  int state;
  /// Rectangle that should be redrawn
  csRect dirty;
  /// Clipping rectangle (if not empty)
  csRect clip;
  /// Component palette and palette length
  int *palette, palettesize;
  /// false if palette points to static array, true if it is a copy
  bool originalpalette;
  /// Window drag style (see CS_DRAG_XXX above)
  int DragStyle;
  /// Used on drag operations
  static int dragX, dragY, dragMode;
  /// The component bound before drag started
  static csRect dragBound;
  /// Component against which this component is clipped<p>
  csComponent *clipparent;
  /// Most components contain a text string. Unify the interface.
  char *text;
  /// Current font index
  int Font;
  /// true if window is maximized, false if not (OrgBound invalid in this case)
  bool Maximized;
  /// Original bound when window is maximized
  csRect OrgBound;

public:
  /// The focused child window
  csComponent *focused;
  /// The top-Z child window
  csComponent *top;
  /// Next and previous neightbours
  csComponent *next, *prev;
  /// Parent component or NULL
  csComponent *parent;
  /// An array of 'clip children', i.e. components which are clipped inside our bounds
  csVector clipchildren;
  /// Top-level application object
  csApp *app;
  /// Component ID, unique within its parrent's child ring
  unsigned int id;
  /// Component size/position rectangle
  csRect bound;

  /// Create a component and insert it into parent's child list if parent != NULL
  csComponent (csComponent *iParent);
  /// Destroy component and remove it from parent't child list
  virtual ~csComponent ();

  /// Insert a child component
  virtual void Insert (csComponent *comp);

  /// Delete a child component
  virtual void Delete (csComponent *comp);

  /// Delete all children components
  void DeleteAll ();

  /// Insert a 'clip child'
  void InsertClipChild (csComponent *clipchild);

  /// Remove a 'clip child'
  void DeleteClipChild (csComponent *clipchild);

  /// Return the 'clip parent' component
  csComponent *GetClipParent ()
  { return clipparent; }

  /// Focus a child component
  bool SetFocused (csComponent *comp);

  /// Get the focused child window
  csComponent *GetFocused ()
  { return focused; }

  /// Select (focus) this component and return true if successful
  bool Select ();

  /// Return next selectable child window after 'start'
  virtual csComponent *NextChild (csComponent *start = NULL, bool disabled = false);

  /// Return previous selectable child window before 'start'
  virtual csComponent *PrevChild (csComponent *start = NULL, bool disabled = false);

  /// Return next control after 'start', looping through groups
  virtual csComponent *NextControl (csComponent *start = NULL);

  /// Return previous control before 'start', looping through groups
  virtual csComponent *PrevControl (csComponent *start = NULL);

  /// Return control in next group after 'start'
  virtual csComponent *NextGroup (csComponent *start = NULL);

  /// Return control in previous group before 'start'
  virtual csComponent *PrevGroup (csComponent *start = NULL);

  /**
   * Change Z-order of a child component above 'below' (can be NULL
   * for lowest Z-order) neightbour.
   */
  bool SetZorder (csComponent *comp, csComponent *below);

  /// Get the top Z-order child window
  csComponent *GetTop ()
  { return top; }

  /**
   * Set component logical palette.<p>
   * Each component has its own logical palette. A logical palette component
   * is a index into a global table which resides in csApp object. The table
   * in csApp maps a color (such as cs_Color_White) to a real index into
   * the physical palette.<p>
   * It is highly desirable for components to contain in their 1st element
   * their background color. This is sometimes used (for example by
   * irregularily-shaped buttons).
   */
  void SetPalette (int *iPalette, int iPaletteSize);

  /// Same, but accepts the index into cswsPalette[] array
  void SetPalette (int iPaletteID)
  { SetPalette (cswsPalette [iPaletteID].Palette, cswsPalette [iPaletteID].Length); }

  /// Set a color value in palette (makes a copy of *palette if not already)
  void SetColor (int Index, int Color);

  /// Get a color from logical palette
  int GetColor (int Index)
  { if (Index >= palettesize) return cs_Color_Red_L; else return palette[Index]; }

  /**
   * Most components have a text string field. For example, titlebars,
   * buttons, input lines, static components etc etc etc. The following
   * routines are used to access this field in a component-independent
   * manner.
   */
  virtual void SetText (const char *iText);
  /// Query component text
  virtual void GetText (char *oText, int iTextSize);
  /// Same, but you cannot change returned value
  virtual const char *GetText () { return text; }

  /**
   * For each child component call a function with a optional arg
   * Function returns the first child on which func returnes 'true'
   * Function can scan from top-Z child (Zorder == true) or from
   * focused child (Zorder == false).
   */
  csComponent *ForEach (bool (*func) (csComponent *child, void *param),
    void *param = NULL, bool Zorder = false);

  /// Find a child component by its ID
  csComponent *GetChild (int find_id);

  /// Set the application for this object and all its children
  void SetApp (csApp *newapp);

  /// Handle a event and return true if processed
  virtual bool HandleEvent (csEvent &Event);

  /**
   * Handle a event BEFORE all others. You will receive ALL events
   * including focused events. For example, a object will receive mouse events
   * even if object is not visible. A object should take care not to block
   * events expected by other objects.
   */
  virtual bool PreHandleEvent (csEvent &Event);

  /// Handle a event if nobody eaten it.
  virtual bool PostHandleEvent (csEvent &Event);

  /// Send a command to this window and returns the Info field of csEvent object
  void *SendCommand (int CommandCode, void *Info = NULL);

  /// Find the 'default' child
  csComponent *GetDefault ();

  /// Redraw the component if it has a dirty area
  void Redraw ();

  /// Draw the component (only dirty rectangle should be redrawn)
  virtual void Draw ();

  /// Show the component (and activate it if focused == true)
  virtual void Show (bool focused = false);

  /// Hide the component
  virtual void Hide ();

  /// Set component rectangle to given. Return false if not changed
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Same, but with csRect argument
  bool SetRect (csRect &rect)
  { return SetRect (rect.xmin, rect.ymin, rect.xmax, rect.ymax); }

  /// Calls SetRect after calling FixPosition and FixSize (used when dragging)
  bool SetDragRect (int xmin, int ymin, int xmax, int ymax);

  /// Set component position to given X and Y
  void SetPos (int x, int y)
  { SetRect (x, y, x + bound.xmax - bound.xmin, y + bound.ymax - bound.ymin); }

  /// Set component size to given Width and Height
  void SetSize (int w, int h)
  { SetRect (bound.xmin, bound.ymin, bound.xmin + w, bound.ymin + h); }

  /// Center window inside parent
  void Center (bool iHoriz = true, bool iVert = true);

  /// Maximize window if it is not already and if DragStyle has CS_DRAG_SIZEABLE
  virtual bool Maximize ();

  /// Restore window if it is maximized and if DragStyle has CS_DRAG_SIZEABLE
  virtual bool Restore ();

  /// Invalidate a area of component (force a redraw of this area)
  void Invalidate (csRect &area, bool IncludeChildren = false);

  /// Same, but with coordinates instead of rectangle
  void Invalidate (int xmin, int ymin, int xmax, int ymax,
    bool IncludeChildren = false)
  { csRect inv (xmin, ymin, xmax, ymax); Invalidate (inv, IncludeChildren); }

  /// Same, but invalidates entire component
  void Invalidate (bool IncludeChildren = false)
  { Invalidate (0, 0, bound.Width (), bound.Height (), IncludeChildren); }

  /// Set/clear given component state flags
  virtual void SetState (int mask, bool enable);

  /// Return component state flags
  int GetState (int mask)
  { return (state & mask); }

  /// Set drag style flags
  void SetDragStyle (int iDragStyle)
  { DragStyle = iDragStyle; }

  /// Query drag style flags
  int GetDragStyle ()
  { return DragStyle; }

  /// Convert a pair of X,Y coordinates from local to global coordinate system
  void LocalToGlobal (int &x, int &y);

  /// Convert a pair of X,Y coordinates from global to local coordinate system
  void GlobalToLocal (int &x, int &y);

  /// Drag a window with mouse located at (x,y) window-local coordinates
  void Drag (int x, int y, int DragMode);

  /// Set mouse cursor pointer
  void SetMouse (csMouseCursorID Cursor);

  /**
   * Destroy this component<p>
   * You should not call any methods or use any variables after this!
   * When this function returns, the object is already destroyed.
   */
  virtual void Close ();

  /**
   * Set mouse cursor to one of sizing cursors depending on drag mode
   * flags (dragtype should be any combination of CS_DRAG_XXX bits).
   */
  void SetSizingCursor (int dragtype);

  /// Query current mouse location (returns true if mouse is inside this component)
  bool GetMousePosition (int &x, int &y);

  /**
   * Handle a mouse drag event. Check if mouse cursor is within BorderW/BorderH
   * distance from window border; for mouse move events just sets the
   * corresponding mouse shape (if DragMode has CS_DRAG_SIZEABLE bit set).
   * If event is a mouse down event and mouse cursor falls within that distance
   * from window border either resize or move window mode is entered.
   * The function returns "true" if event has been processed, and false
   * if it can be processed by other handlers.
   * This function is handy to call from event handlers of all windows that
   * can be resized and/or moved, example:
   * <pre>
   *   switch (Event.Type)
   *   {
   *     case csevMouseDown:
   *     case csevMouseMove:
   *       if (HandleDragEvent (Event, BorderWidth, BorderHeight))
   *         return true;
   *       return csComponent::HandleEvent (Event);
   *     ...
   *   }
   * </pre>
   */
  bool HandleDragEvent (csEvent &Event, int BorderW, int BorderH);

  /// Fix new window position before assigning to component
  virtual void FixPosition (int &newX, int &newY);

  /// Fix new window size before assigning to component
  virtual void FixSize (int &newW, int &newH);

  /// Return the recommended minimal size of component
  virtual void SuggestSize (int &w, int &h);

  /// Set the size of component to minimal possible plus delta
  virtual void SetSuggestedSize (int dw, int dh);

  /// Find the maximal rectangle uncovered by child windows
  void FindMaxFreeRect (csRect &area);

/**
 * The following methods should be used for drawing.
 * All drawing should perform all expected clipping.
 * Some routines works somewhat kludgy for now, but its enough.
 */

  /**
   * Set clipping rectangle. All following drawing will be clipped
   * against given rectangle. Setting w=0 or h=0 will disable clipping
   * (although it will be anyway performed relative to component bound).
   */
  void SetClipRect (int xmin, int ymin, int xmax, int ymax)
  { clip.Set (xmin, ymin, xmax, ymax); }

  /// Disable clipping
  void SetClipRect ()
  { clip.MakeEmpty (); }

  /// Query current text font for this component
  int GetFont ();

  /// Set text font for this component and possibly its children
  void SetFont (int iFont, bool IncludeChildren = false);

  /// Draw a box
  void Box (int xmin, int ymin, int xmax, int ymax, int colindx);

  /// Draw a line
  void Line (float x1, float y1, float x2, float y2, int colindx);

  /// Draw a pixel
  void Pixel (int x, int y, int colindx);

  /// Draw a text string: if bg < 0 background is not drawn
  void Text (int x, int y, int fgindx, int bgindx, const char *s);

  /// Draw a (scaled) 2D sprite
  void Sprite2D (csSprite2D *s2d, int x, int y, int w, int h);
  /// Draw a (non-scaled) 2D sprite
  void Sprite2D (csSprite2D *s2d, int x, int y)
  { Sprite2D (s2d, x, y, s2d->Width (), s2d->Height ()); }

  /// Return the width of given text using currently selected font
  int TextWidth (const char *text);
  /// Return the height of currently selected font
  int TextHeight ();

  /// Draw a 3D-looking thin rectangle
  void Rect3D (int xmin, int ymin, int xmax, int ymax, int darkindx, int lightindx);

  /// Draw a 3D rectangle with two oblique corners (used for buttons, for example)
  void ObliqueRect3D (int xmin, int ymin, int xmax, int ymax, int cornersize,
    int darkindx, int lightindx);

protected:
  /**
   * Clip a set of rectangles so that they will contain only rectangles
   * uncovered by other windows. Initial rectangles are in local coordinates,
   * final rectangles are in global coordinates.
   */
  void Clip (csObjVector &rect, csComponent *last);

  /// Clip a set of rectangles against 'clip children'
  void ClipAlienChildren (csObjVector &rect, csComponent *child);

  /**
   * Prepare a label. Search for '~' in iLabel, and copy text without '~'
   * into oLabel. Former underline position is stored into oUnderlinePos.
   * This is used by labels, menuitems, static components etc.
   */
  static void PrepareLabel (const char *iLabel, char * &oLabel, int &oUnderlinePos);
  /// Draw a underline under iText drawn at iX,iY with iColor
  void DrawUnderline (int iX, int iY, const char *iText, int iUnderlinePos,
    int iColor);

  /// Utility functions: return position one word left from StartPos
  static int WordLeft (const char *iText, int StartPos);
  /// Return position one word right from StartPos
  static int WordRight (const char *iText, int StartPos);

private:
  static bool do_handle_event (csComponent *child, void *param);
};

#endif // __CSCOMP_H__
