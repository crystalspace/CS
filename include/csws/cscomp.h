/*
    Crystal Space Windowing System: Windowing System Component interface
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

#ifndef __CS_CSCOMP_H__
#define __CS_CSCOMP_H__

/**\file
 * Crystal Space Windowing System: Component interface
 */

/**
 * \addtogroup csws_comps
 * @{ */

#include "csextern.h"
 
#include "csgeom/csrect.h"
#include "csutil/parray.h"
#include "csutil/array.h"
#include "cstool/cspixmap.h"
#include "cswspal.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"

class csApp;
class csSkin;
class csSkinSlice;
struct iEvent;

// this is where we store all the csRect for calculating visible areas and clipping in
typedef csPDelArray<csRect> cswsRectVector;

/**
 * \name Component state flags
 * The component state is an (at least) 32-bit integer, which contains
 * up to 32 state flags. These flags marks various transitional states
 * which can change in time depending on the activity of the component
 * (and user actions). These 32 bits are split into several conventional
 * fields, every set of bits being reserved for different tasks:
 * <pre>
 *  31    24 23    16 15     8 7      0
 * +--------+--------+--------+--------+
 * |        |        |        |        |
 * +--------+--------+--------+--------+
 * \---+---/ \---+--/ \-------+-------/
 *     |         |            +--------* Reserved for the csComponent class
 *     |         |                       (masks for these bits are defined below)
 *     |         +---------------------* Reserved for internal use by CSWS
 *     |                                 components, derived from csComponent.
 *     +-------------------------------* Reserved for user-defined components.
 * </pre>
 * Thus, all values matching the mask 0x0000ffff are reserved for csComponent,
 * all values matching the mask 0x00ff0000 are reserved for other CSWS
 * components (see csListBoxItem, for example), and finally all the
 * bits matching the mask 0xff000000 are reserved for user.
 * Pretty generous, eh? ;-)
 * @{ */

/// Component state flag: Component is visible
#define CSS_VISIBLE		0x00000001
/// Component state flag: Component is focused in parent window child list
#define CSS_FOCUSED		0x00000002
/// Component state flag: Component is disabled
#define CSS_DISABLED		0x00000004
/// Component state flag: Component can be selected
#define CSS_SELECTABLE		0x00000008
/// Component state flag: Component is the beginning of a group of components
#define CSS_GROUP		0x00000010
/// Component state flag: Move component to top Z-order when selected
#define CSS_TOPSELECT		0x00000020
/// Component state flag: Exclude component from clipping process
#define CSS_TRANSPARENT		0x00000040
/// Component state flag: Component is modally executing
#define CSS_MODAL		0x00000080
/// Component state flag: Component is maximized (NEVER change this manually!)
#define CSS_MAXIMIZED		0x00000100
/// Component state flag: Component or (some of) his children components are dirty
#define CSS_DIRTY		0x00000200
/// Component state flag: Additional state flag used to decide when to finish CheckDirty(); ignore it
#define CSS_RESTART_DIRTY_CHECK	0x00000400
/**@}*/

/**
 * Predefined Windowing System Command Codes<p>
 * The list below does not contain all defined messages; these are only the
 * most general ones. Any class which defines some class-specific messages
 * should ensure that no other command is using the integer value of its
 * proprietary command codes. To avoid this as much as possible, the following
 * ranges are reserved:<p>
 * <ul>
 *   <li>0x00000000 ... 0x0FFFFFFF: Reserved for CrystalSpace Windowing System
 *     <ul>
 *       <li>0x00000000 ... 0x000000FF: Non-class specific commands
 *       <li>0x00000100 ... 0x000001FF: csWindow class messages
 *       <li>0x00000200 ... 0x000002FF: csMenu class messages
 *       <li>0x00000300 ... 0x000003FF: csTimer class messages
 *       <li>0x00000400 ... 0x000004FF: csListBox class messages
 *       <li>0x00000500 ... 0x000005FF: csButton class messages
 *       <li>0x00000600 ... 0x000006FF: csScrollBar class messages
 *       <li>0x00000700 ... 0x000007FF: csStatic class messages
 *       <li>0x00000800 ... 0x000008FF: csCheckBox class messages
 *       <li>0x00000900 ... 0x000009FF: csRadioButton class messages
 *       <li>0x00000A00 ... 0x00000AFF: csSpinBox class messages
 *       <li>0x00000B00 ... 0x00000BFF: csColorWheel class messages
 *       <li>0x00000C00 ... 0x00000CFF: csNotebook class messages
 *       <li>0x00000D00 ... 0x00000DFF: csSlider class messages
 *       <li>0x00000E00 ... 0x00000EFF: csTree class messages
 *       <li>0x00000F00 ... 0x00000FFF: csGrid class messages
 *     </ul>
 *   <li>0x10000000 ... 0xFFFFFFFF: Reserved for user class-specific messages
 * </ul>
 * All commands receives a input parameter in the Command.Info field of iEvent
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
   * Query a control if it would like to be the default control in a dialog.<p>
   * The control is 'default' if it has a 'default' attribute (this is
   * control-specific, for example buttons have the CSBSTY_DEFAULT style).
   * <pre>
   * IN: 0
   * OUT: (csComponent *) or 0;
   * </pre>
   */
  cscmdAreYouDefault = 0x80,
  /**
   * This message is sent by parent to its active child to activate
   * whatever action it does. For example, this message is sent by a
   * dialog window to its active child when user presses Enter key.
   * <pre>
   * IN: 0
   * OUT: (csComponent *)this if successful;
   * </pre>
   */
  cscmdActivate,
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
   * A component loses focus. This message is sent to the component owner
   * right before component loses focus (i.e. during this message the
   * component still owns the focus)
   * <pre>
   * IN: (csComponent *)Component
   * OUT: 0 if you prohibit the focus change operation
   * </pre>
   */
  cscmdLoseFocus,
  /**
   * A component receives focus. This message is sent to the component owner
   * right before component receives focus (i.e. during this message the
   * component still does not own the focus)
   * <pre>
   * IN: (csComponent *)Component
   * OUT: 0 if you prohibit the focus change operation
   * </pre>
   */
  cscmdReceiveFocus,
  /**
   * These commands are used for message boxes. csMessageBox (...) returns
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
  cscmdIgnore,
  /**
   * This command tells every component that Windowing System's
   * color scheme has been changed.
   * <pre>
   * IN:  nothing
   * OUT: nothing
   * </pre>
   */
  cscmdColorSchemeChanged,
  /**
   * Skin has changed (or some components of the skin).
   * If the component is skinnable, it should query the
   * respective skin slice from the skin object and store
   * it into the 'skinslice' member variable for later use.
   * Also the component is invalidated upon reception of
   * this message.
   * <pre>
   * IN:  (csSkin *)skinslice
   * OUT: nothing
   * </pre>
   */
  cscmdSkinChanged,
  /**
   * CSWS private command sent when a component is moved;
   * in this case we have to move all children who's clip
   * parent is not equal to their parent in a special way.
   * <pre>
   * IN:  (int *)deltaxy [2]
   * </pre>
   */
  cscmdMoveClipChildren,
  /**
   * This command is sent when this components get out of modality.
   */
  cscmdStopModal
};

/**
 * \name Drag mode flags
 * These flags are used by csComp::Drag to compute
 * new window coordinates when dragging window with mouse
 * @{ */
/// Drag left window border
#define CS_DRAG_XMIN		0x01
/// Drag right window border
#define CS_DRAG_XMAX		0x02
/// Drag top window border
#define CS_DRAG_YMIN		0x04
/// Drag bottom window border
#define CS_DRAG_YMAX		0x08
/// Window is moveable
#define CS_DRAG_MOVEABLE	0x10
/// Window is sizeable
#define CS_DRAG_SIZEABLE	0x20
/// All flags above combined (used when dragging with titlebar)
#define CS_DRAG_ALL		\
	(CS_DRAG_XMIN | CS_DRAG_XMAX | CS_DRAG_YMIN | CS_DRAG_YMAX)
/** @} */

/** 
 * \name Bound resize flags
 * When the size of certain component changes, it checks all his children
 * what they want to do with their own size. You can lock the distance
 * between certain margin of the component and respective margin of
 * the parent component to stay the same when this happens, using the
 * ResizeMode field (or SetResizeMode() method).
 * @{ */
/// Lock component's left margin with parent's left margin
#define CS_LOCK_XMIN		0x01
/// Lock component's right margin with parent's right margin
#define CS_LOCK_XMAX		0x02
/// Lock component's top margin with parent's top margin
#define CS_LOCK_YMIN		0x04
/// Lock component's bottom margin with parent's bottom margin
#define CS_LOCK_YMAX		0x08
/// Lock all four margins
#define CS_LOCK_ALL		\
	(CS_LOCK_XMIN | CS_LOCK_XMAX | CS_LOCK_YMIN | CS_LOCK_YMAX)
/** @} */

/**
 * \name Child repositioning
 * An alternative way for child repositioning consist of the following:
 * independently of each other in both X and Y direction the child can
 * keep its size, and be placed either at the start, center, or end strip
 * of the reserved for childs space of the parent component. These values
 * are stored also in the ResizeMode field and occupies same bits as the
 * CS_LOCK_XXX constants; thus these modes are reciprocaly exclusive.
 * You can still use locks for top and bottom margins and use automatic
 * repositioning for left and right margins or vice versa. That is,
 * CS_LOCK_XMIN and CS_LOCK_XMAX are reciprocally exclusive with
 * CS_REPOS_{LEFT|RIGHT|HCENTER} modes, but they are compatible with
 * CS_REPOS_{TOP|BOTTOM|VCENTER} modes.
 * @{ */
/// Used to distinguish when component is repositioned or bound locked horizontally
#define CS_REPOS_HORIZONTAL	0x10
/// Used to distinguish when component is repositioned or bound locked vertically
#define CS_REPOS_VERTICAL	0x20
/// The mask for extracting the horizontal automatic position
#define CS_REPOS_H_MASK		(CS_REPOS_HORIZONTAL | 0x3)
/// The mask for extracting the vertical automatic position
#define CS_REPOS_V_MASK		(CS_REPOS_VERTICAL | 0xc)
/// The component is automatically placed at the left of the parent window
#define CS_REPOS_LEFT		(CS_REPOS_HORIZONTAL | 0x0)
/// The component is automatically placed at the right of the parent window
#define CS_REPOS_RIGHT		(CS_REPOS_HORIZONTAL | 0x1)
/// The component is automatically horizontally centered in the parent window
#define CS_REPOS_HCENTER	(CS_REPOS_HORIZONTAL | 0x2)
/// Same as HCENTER but the component is resized by the same amount as parent
#define CS_REPOS_HCENTERSIZE	(CS_REPOS_HORIZONTAL | 0x3)
/// The component is automatically placed at the top of the parent window
#define CS_REPOS_TOP		(CS_REPOS_VERTICAL | 0x0)
/// The component is automatically placed at the bottom of the parent window
#define CS_REPOS_BOTTOM		(CS_REPOS_VERTICAL | 0x4)
/// The component is automatically vertically centered in the parent window
#define CS_REPOS_VCENTER	(CS_REPOS_VERTICAL | 0x8)
/// Same as VCENTER but the component is resized by the same amount as parent
#define CS_REPOS_VCENTERSIZE	(CS_REPOS_VERTICAL | 0xc)
/** @} */

/**
 * Graphics system component: a menu, window etc.<p>
 * This is an abstract base class: all windowing system classes should be
 * subclassed from csComponent. Each component can have a number of child
 * components. Child components are chained together in a ring list; the only
 * case when a 0 can be encountered in this list is when component has
 * no children. When a component has at least one child and if you traverse the
 * child list you should take care to avoid looping forever through them.<p>
 *
 * A csComponent object is a rectangle area of screen which can contain
 * absolutely any content. The object is responsible for filling all
 * pixels within that rectangle, i.e. underlying windows will never
 * touch other window's area. If the component's `state' field has the
 * CSS_TRANSPARENT bit set, the component can have partially transparent
 * parts (those are filled by underlying component(s)). In this case
 * the underlying components are painted before this component, and then
 * anything you draw inside the Draw() method is overlayed onto parent
 * component's image.
 */
class CS_CRYSTALSPACE_EXPORT csComponent
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
  /// Original component palette
  int *originalpalette;
  /// Original bound when window is maximized
  csRect OrgBound;
  /// Window drag style (see CS_DRAG_XXX above)
  char DragStyle;
  /// What to do when parent size changes (see CS_LOCK_XXX flags)
  char ResizeMode;
  /// Used on drag operations
  static int dragX;
  static int dragY;
  static int dragMode;
  /// The component bound before drag started
  static csRect *dragBound;
  /// Component against which this component is clipped<p>
  csComponent *clipparent;
  /// Most components contain a text string. Unify the interface.
  char *text;
  /// Current font (or 0 if should use parent font)
  iFont *Font;
  /// Current font size
  int FontSize;
  /// An array of 'clip children', i.e. components which are clipped inside our bounds
  csArray<csComponent*> clipchildren;
  /// This field is used to cache current clipping region during every Redraw()
  static cswsRectVector *visregion;

public:
#if (CS_PROCESSOR_SIZE == 32)
# if (_MSC_VER >= 1300)
  /*
   * Silence VC7 64bit warning.
   */
  typedef unsigned int __w64 ID;
# else
  /// An opaque hash key.
  typedef unsigned int ID;
# endif
#else
  /*
   * At some places, pointers are casted to csHashKey. Work around truncation
   * problems by forcing csHashKey to at least 64bit on 64bit machines.
   */
  typedef uint64 ID;
#endif


  /// The focused child window
  csComponent *focused;
  /// The top-Z child window
  csComponent *top;
  /// Next and previous neightbours
  csComponent *next, *prev;
  /// Parent component or 0
  csComponent *parent;
  /// Top-level application object
  csApp *app;
  /// Component skin slice
  csSkinSlice *skinslice;
  /// Abstract pointer for internal use by skin slice
  void** skindata;
  /// Component ID, unique within its parrent's child ring
  ID id;
  /// Component size/position rectangle
  csRect bound;

  /// Create a component and insert it into parent's child list if parent != 0
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
  virtual bool SetFocused (csComponent *comp);

  /// Get the focused child window
  csComponent *GetFocused ()
  { return focused; }

  /// Select (focus) this component and return true if successful
  bool Select ();

  /// Return next visible selectable child window after 'start'
  virtual csComponent *NextChild (csComponent *start = 0, bool disabled = false);

  /// Return previous visible selectable child window before 'start'
  virtual csComponent *PrevChild (csComponent *start = 0, bool disabled = false);

  /// Return next control after 'start', looping through groups
  virtual csComponent *NextControl (csComponent *start = 0);

  /// Return previous control before 'start', looping through groups
  virtual csComponent *PrevControl (csComponent *start = 0);

  /// Return control in next group after 'start'
  virtual csComponent *NextGroup (csComponent *start = 0);

  /// Return control in previous group before 'start'
  virtual csComponent *PrevGroup (csComponent *start = 0);

  /// Fix the focused child if it is not selectable (find another)
  bool FixFocused ();

  /**
   * Change Z-order of a child component above 'below' (can be 0
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
  { SetPalette (cswsPalette [iPaletteID].Palette, cswsPalette [iPaletteID].Size); }

  /// Reset the palette of the component to the palette it had at startup
  void ResetPalette ();

  /// Set a color value in palette (makes a copy of *palette if not already)
  void SetColor (int Index, int Color);

  /// Get a color from logical palette
  int GetColor (int Index)
   { if (Index & 0x80000000) return Index;
    if (Index >= palettesize) return cs_Color_Red_L; else return palette[Index]; }

  /**
   * Most components have a text string field. For example, titlebars,
   * buttons, input lines, static components etc etc etc. The following
   * routines are used to access this field in a component-independent
   * manner.
   */
  virtual void SetText (const char *iText);
  /// Query component text
  virtual void GetText (char *oText, size_t iTextSize) const;
  /// Same, but you cannot change returned value
  virtual const char *GetText () const { return text; }

  /**
   * For each child component call a function with a optional arg
   * Function returns the first child on which func returnes 'true'
   * Function can scan from top-Z child (Zorder == true) or from
   * focused child (Zorder == false).
   */
  csComponent *ForEach (bool (*func) (csComponent *child, intptr_t param),
    intptr_t param = 0, bool Zorder = false);

  /// Find a child component by its ID
  csComponent *GetChild (ID find_id) const;

  /// Set the application for this object and all its children
  void SetApp (csApp *newapp);

  /// Handle a event and return true if processed
  virtual bool HandleEvent (iEvent &Event);

  /**
   * Handle a event BEFORE all others. You will receive ALL events
   * including focused events. For example, a object will receive mouse events
   * even if object is not visible. A object should take care not to block
   * events expected by other objects.
   */
  virtual bool PreHandleEvent (iEvent &Event);

  /// Handle a event if nobody eaten it.
  virtual bool PostHandleEvent (iEvent &Event);

  /// Send a command to this window and returns the Info field of iEvent object
  intptr_t SendCommand (int CommandCode, intptr_t Info = 0);
  /// Send a broadcast to this window and returns the Info field of iEvent object
  intptr_t SendBroadcast (int CommandCode, intptr_t Info = 0);

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

  /**
   * Invalidate a area of component (force a redraw of this area).
   * If fIncludeChildren is true, all child components that covers
   * this area of parent will be partially invalidated as well.
   * Additionaly, if 'below' is not 0, only the child components
   * that are below 'below' in Z-order or CSS_TRANSPARENT components
   * that are 'above' in Z-order will be invalidated.
   */
  void Invalidate (csRect &area, bool IncludeChildren = false,
    csComponent *below = 0);

  /// Same, but with coordinates instead of rectangle
  void Invalidate (int xmin, int ymin, int xmax, int ymax,
    bool IncludeChildren = false, csComponent *below = 0)
  {
    csRect inv (xmin, ymin, xmax, ymax);
    Invalidate (inv, IncludeChildren, below);
  }

  /// Same, but invalidates entire component (and possibly all children)
  void Invalidate (bool IncludeChildren = false, csComponent *below = 0)
  { Invalidate (-99999, -99999, +99999, +99999, IncludeChildren, below); }

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

  /// Set resize mode flags
  void SetResizeMode (int iResizeMode)
  { ResizeMode = iResizeMode; }

  /// Query resize mode flags
  int GetResizeMode ()
  { return ResizeMode; }

  /// Convert a pair of X,Y coordinates from local to global coordinate system
  void LocalToGlobal (int &x, int &y);

  /// Convert a pair of X,Y coordinates from global to local coordinate system
  void GlobalToLocal (int &x, int &y);

  /**
   * Convert a X,Y pair from coordinate system of another window to this one.
   * This works faster than GlobalToLocal/LocalToGlobal pair in the case when
   * this component is a N-th level parent for the 'from' component.
   */
  void OtherToThis (csComponent *from, int &x, int &y);

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
   * Get the (possibly child) component that is topmost at given x,y
   * location. This is useful, for example, to get the component under
   * mouse cursor. You can provide a test function that will be called
   * for "transparent" childs; if the child has the CSS_TRANSPARENT
   * flag set, this routine will be called to determine whenever we
   * should go further below this child. If the routine is 0, it
   * will stop at the first transparent child. If the routine returns
   * true, GetChildAt() will return given child component.
   */
  csComponent *GetChildAt (int x, int y,
    bool (*func) (csComponent *, void *) = 0, void *data = 0);

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
  bool HandleDragEvent (iEvent &Event, int BorderW, int BorderH);

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

  /// Get the name of the skip slice for this component (if not 0)
  virtual char *GetSkinName ();

  /// Get the closest in window hierarchy skin object
  virtual csSkin *GetSkin ();

  /**
   * The following methods should be used for drawing.
   * All drawing routines below performs all required clipping.
   *<p>
   * You should avoid using other drawing routines
   * (such as accessing iGraphics2D/3D directly) because
   * first of all they aren't clipped to the bounds of current
   * component and possibly to all overlapped windows, and second
   * the graphics pipeline should take care to copy static portions
   * of screen in double-buffered environments.
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

  /// Set font for this component
  void SetFont (iFont *iNewFont);

  /// Query current text font for this component
  virtual void GetFont (iFont *&oFont);

  /// Draw a box
  void Box (int xmin, int ymin, int xmax, int ymax, int colindx);

  /// Draw a line
  void Line (float x1, float y1, float x2, float y2, int colindx);

  /// Draw a pixel
  void Pixel (int x, int y, int colindx);

  /// Draw a text string: if bg < 0 background is not drawn
  void Text (int x, int y, int fgindx, int bgindx, const char *s);

  /// Draw a (scaled) 2D sprite
  void Pixmap (csPixmap *s2d, int x, int y, int w, int h, uint8 Alpha = 0);
  /// Draw a (non-scaled) 2D sprite
  void Pixmap (csPixmap *s2d, int x, int y, uint8 Alpha = 0)
  { Pixmap (s2d, x, y, s2d->Width (), s2d->Height (), Alpha); }
  /// Draw a (tiled) pixmap (orgy stands for "origin y", not what you though of - shame!)
  void Pixmap (csPixmap *s2d, int x, int y, int w, int h, int orgx, int orgy,
    uint8 Alpha = 0);
  /// Draw a (possibly tiled) texture, possibly semi-transparent
  void Texture (iTextureHandle *tex, int x, int y, int w, int h,
    int orgx, int orgy, uint8 Alpha = 0);

  /// Return the width of given text using current font (and possibly height)
  int GetTextSize (const char *text, int *oHeight = 0);
  /// Return how many letters from given string fits in this number of pixels
  int GetTextChars (const char *text, int iWidth);

  /// Draw a 3D-looking thin rectangle
  void Rect3D (int xmin, int ymin, int xmax, int ymax, int darkindx, int lightindx);

  /// Draw a 3D rectangle with two oblique corners (used for buttons, for example)
  void ObliqueRect3D (int xmin, int ymin, int xmax, int ymax, int cornersize,
    int darkindx, int lightindx);

  /// Clear the entire component with given color
  void Clear (int colindx)
  { Box (0, 0, bound.Width (), bound.Height (), colindx); }

  /// Clear the Z-buffer in the given rectangle
  void ClearZbuffer (int xmin, int ymin, int xmax, int ymax);

  /// Clear the Z-buffer in the area covered by this component
  void ClearZbuffer ()
  { ClearZbuffer (0, 0, bound.Width (), bound.Height ()); }

  /// Draw a 3D polygon
  //@@@REIMPLEMENT THIS FOR NR
  //void Polygon3D (G3DPolygonDPFX &poly, uint mode);

  ///-------------------------------------- Utility drawing functions ----------

  /// Draw a underline under iText drawn at iX,iY with iColor
  void DrawUnderline (int iX, int iY, const char *iText, size_t iUnderlinePos,
    int iColor);

protected:
  /**
   * Clip a set of rectangles so that they will contain only rectangles
   * uncovered by other windows. Initial rectangles are in local coordinates,
   * final rectangles are in global coordinates.
   */
  void Clip (cswsRectVector &rect, csComponent *last, bool forchild = false);

  /// Clip the rectangle set against given child
  void ClipChild (cswsRectVector &rect, csComponent *child);

  /**
   * Perform fast clipping by using the pre-cached visible region
   * initialized at start of Redraw(). This has the side effect that
   * any drawing operation that happens outside the Draw() method
   * is effectively clipped away.
   */
  void FastClip (cswsRectVector &rect);

  /**
   * Prepare a label. Search for '~' in iLabel, and copy text without '~'
   * into oLabel. Former underline position is stored into oUnderlinePos.
   * This is used by labels, menuitems, static components etc.
   */
  static void PrepareLabel (const char *iLabel, char * &oLabel, size_t &oUnderlinePos);

  /// Check if the keyboard event fits given hot key
  bool CheckHotKey (iEvent &iEvent, char iHotKey);

  /// Utility functions: return position one word left from StartPos
  static size_t WordLeft (const char *iText, size_t StartPos);
  /// Return position one word right from StartPos
  static size_t WordRight (const char *iText, size_t StartPos);

  /// Apply a skin <b>only</b> to this component: returns true on success
  bool ApplySkin (csSkin *Skin);

  /**
   * Perform a check of this component and all dirty children'
   * dirty areas: if child component is transparent, unify his
   * dirty area with this component's dirty area. You will never
   * need to call this function manually; this is done automatically.
   * The `TD' prefix stands for `top-doen', that is the Z-order in
   * which the check is performed.
   */
  void CheckDirtyTD (csRect &ioR);
  /**
   * Same as CheckDirtyTD but the check is performed in the inverse
   * direction - from bottom up (`BU'). This routine checks if the
   * transparent child components are covered by this component's
   * dirty area; if so, the respective areas of child windows are
   * marked as dirty as well.
   */
  void CheckDirtyBU (csRect &ioR);

private:
  static bool do_handle_event (csComponent *child, intptr_t param);
};

/** @} */

#endif // __CS_CSCOMP_H__
