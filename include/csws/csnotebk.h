/*
    Crystal Space Windowing System: notebook class
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSNOTEBK_H__
#define __CS_CSNOTEBK_H__

/**\file
 * Crystal Space Windowing System: notebook class
 */

/**
 * \addtogroup csws_comps_notebook
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"
#include "csbutton.h"
#include "csutil/parray.h"

/**
 * \name Notebook styles
 * There are various styles of notebooks. The position of the tabs,
 * the border style and so on can be different. The masks below
 * can be ORed together to form a compound notebook style; only
 * one value from each group can be choosed.
 * @{ */

/// Notebook style: The mask for tab position bits
#define CSNBS_TABPOS_MASK	0x00000003
/// Notebook style: Use tabs along the top side of notebook (default value)
#define CSNBS_TABPOS_TOP	0x00000000
/// Notebook style: Use tabs along the bottom side of notebook
#define CSNBS_TABPOS_BOTTOM	0x00000001
/// Notebook style: Use tabs along the left side of notebook
#define CSNBS_TABPOS_LEFT	0x00000002
/// Notebook style: Use tabs along the right side of notebook
#define CSNBS_TABPOS_RIGHT	0x00000003
/// Notebook style: Draw a thin 3D frame around page dialogs
#define CSNBS_PAGEFRAME		0x00000010
/// Notebook style: Display page information on the top of notebook
#define CSNBS_PAGEINFO		0x00000020
/// Notebook style: Display a flat thin border around tabs instead of 3D look
#define CSNBS_THINTABS		0x00000040
/// Notebook style: default style
#define CSNBS_DEFAULTVALUE	\
	CSNBS_TABPOS_TOP | CSNBS_PAGEFRAME | CSNBS_PAGEINFO
/** @} */

/**
 * Notebook messages.
 */
enum
{
  /**
   * Sent to parent component to notify that the notebook is going
   * to leave a page. The parent can abort this process by returning
   * 0 in Event.Command.Info field.
   * <pre>
   * IN:  (csComponent *)page
   * OUT: unchanged or 0
   * </pre>
   */
  cscmdNotebookLeavePage = 0x00000c00,
  /**
   * Notify parent window that a page is going to be selected.
   * The parent can abort this process by returning 0 in the
   * Event.Command.Info field or set the address of another component
   * to be selected instead.
   * <pre>
   * IN:  (csComponent *)page
   * OUT: (csComponent *)page_to_select or 0
   * </pre>
   */
  cscmdNotebookSelectPage,
  /**
   * Tell notebook to switch to next page
   * <pre>
   * IN:  nothing
   * OUT: nothing
   * </pre>
   */
  cscmdNotebookNextPage,
  /**
   * Tell notebook to switch to previous page
   * <pre>
   * IN:  nothing
   * OUT: nothing
   * </pre>
   */
  cscmdNotebookPrevPage,
  /**
   * Tell notebook to scroll tab line forward
   * <pre>
   * IN:  nothing
   * OUT: nothing
   * </pre>
   */
  cscmdNotebookScrollTabsForward,
  /**
   * Tell notebook to scroll tab line backward
   * <pre>
   * IN:  nothing
   * OUT: nothing
   * </pre>
   */
  cscmdNotebookScrollTabsBackward
};

class cspNotebookButton;

/**
 * A notebook (also called "property sheet") is a collection
 * of dialogs enclosed in a single window. The dialogs are divided into
 * several groups, each group gets its own "tab" on one of the sides
 * of the notebook. By clicking the "tab" you switch to that group of
 * dialogs, every group can consist of several dialogs that are switched
 * with a minor arrow control in one of the corners of the notebook.
 */
class CS_CRYSTALSPACE_EXPORT csNotebook : public csComponent
{
  // Structure used to keep per-page private information
  struct cspPageData
  {
    // The page component
    csComponent *page;
    // Tab bounds (empty if not visible)
    csRect bound;
    // The Z-order of this tab (-1 if not visible)
    int zorder;
    // Notebook page flags (see NOTEBOOK_PAGE_XXX)
    unsigned char flags;

    // The image on this tab (only primary pages)
    csPixmap *image;
    // The text on this tab
    char *text;
    // The underline position
    size_t underline_pos;

    // Constructor
    cspPageData (csComponent *ipage, unsigned char iflags);
    // Destructor
    ~cspPageData ();
    // Check if this character is the hotkey for this tab
    bool IsHotKey (int iKey);
  };

  /// Notebook style
  int style;
  /// The next/previous page buttons
  csButton *nextpage, *prevpage;
  /// Tab line scroll forward/backward buttons
  cspNotebookButton *tabfor, *taback;
  /// Shared pixmaps on notebook buttons
  static csPixmap *sprites [12];
  /// Flag: true if we should re-position knobs
  bool fReposition;
  /// The first visible tab and the active tab
  size_t firsttab, activetab;
  /// This array contains the per-page information
  csPDelArray<cspPageData> pages;

public:
  /// Create a notebook component with given parent and style
  csNotebook (csComponent *iParent, int iStyle = CSNBS_DEFAULTVALUE);
  /// Destroy the notebook object
  virtual ~csNotebook ();
  /// Draw the notebook
  virtual void Draw ();
  /// Handle external events
  virtual bool HandleEvent (iEvent &Event);

  /// Reposition knobs on resize
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);

  /// Query notebook style
  int GetStyle () { return style; }
  /// Change notebook style on-the-fly
  void SetStyle (int iStyle);

  /// Add a primary tab: page info can differ from page title
  bool AddPrimaryTab (csComponent *iComponent, const char *iText,
    const char *iInfo = 0, csComponent *iBefore = 0);
  /// Add a secondary tab (no page title)
  bool AddSecondaryTab (csComponent *iComponent, const char *iInfo,
    csComponent *iBefore = 0);
  /// Same but use a pixmap as tab label rather than a text string
  bool AddPrimaryTab (csComponent *iComponent, csPixmap *iPixmap,
    bool iDeletePixmap = true, const char *iInfo = 0,
    csComponent *iBefore = 0);

  /// Delete a tab and associated component by index
  bool DeleteTab (size_t iIndex);
  /// Delete a tab and associated component
  bool DeleteTab (csComponent *iComponent);

  /// Select a particular tab (by index)
  bool SelectTab (size_t iIndex);
  /// Select a particular tab (by component)
  bool SelectTab (csComponent *iComponent);

protected:
  /// Secondary data (can be derived from other data but is cached for speed)
  int tabx, taby, tabw, tabh;
  size_t lasttab;
  int lastpos;
  size_t primary;

  /// Reposition current page, forward/backward buttons etc
  void PlaceGadgets ();
  /// Common part for AddXXXTab ()
  bool AddTab (cspPageData *iPageData, const char *iInfo,
    csComponent *iBefore);
  /// Compute client rectangle size
  void GetClientRect (csRect &oRect);
  /// Compute the height of information line
  int InfoHeight ();
  /// Compute the size of a single tab
  bool GetTabSize (size_t iTab, int &oW, int &oH);
  /// Check if given x,y lies inside a tab
  bool InsideTab (size_t iIndex, int x, int y);
  /// Find page index given page pointer
  size_t FindPage (csComponent *iComponent);
};

/** @} */

#endif // __CS_CSNOTEBK_H__
