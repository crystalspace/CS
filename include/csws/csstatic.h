/*
    Crystal Space Windowing System: static control class
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

#ifndef __CS_CSSTATIC_H__
#define __CS_CSSTATIC_H__

/**\file
 * Crystal Space Windowing System: static control class
 */

/**
 * \addtogroup csws_comps_static
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"

/// Possible static component styles
enum csStaticStyle
{
  /// Empty component. No idea what it can be used for :-)
  csscsEmpty,
  /// A transparent rectangle with text
  csscsLabel,
  /// Same as csscsLabel but with a thin 3D frame around
  csscsFrameLabel,
  /// A rectangle of background color
  csscsRectangle,
  /// A bitmap
  csscsBitmap,
  /// Text
  csscsText
};

/**
 * \name Text alignment flags
 * @{ */
/// Horizontal text alignment mask
#define CSSTA_HALIGNMASK	0x00000003
/// Align text horizontally to the left (default)
#define CSSTA_LEFT		0x00000000
/// Align text horizontally to the right
#define CSSTA_RIGHT		0x00000001
/// Center text horizontally
#define CSSTA_HCENTER		0x00000002
/// Vertical text alignment mask
#define CSSTA_VALIGNMASK	0x0000000C
/// Align text to top
#define CSSTA_TOP		0x00000000
/// Align text to bottom
#define CSSTA_BOTTOM		0x00000004
/// Center text vertically (default)
#define CSSTA_VCENTER		0x00000008
/// Text wrap mask
#define CSSTA_WRAPMASK		0x00000030
/// Wrap text on word boundaries
#define CSSTA_WORDWRAP		0x00000010
/// Wrap text on character boundaries
#define CSSTA_CHARWRAP		0x00000020
/** @} */

/// Static control class messages
enum
{
  /**
   * Sent by a label to its link when a "hot key pressed/depressed"
   * event is detected
   * <pre>
   * IN:  (iEvent *)Event
   * </pre>
   */
  cscmdStaticHotKeyEvent = 0x00000700,
  /**
   * Sent by a label to its link when a mouse event is detected
   * <pre>
   * IN:  (iEvent *)Event
   * </pre>
   */
  cscmdStaticMouseEvent,
  /**
   * Set static control bitmap (if control style is csscsBitmap)<p>
   * <b>NOTE</b>: We don't delete the old bitmap, because we assume
   * that if you're changing bitmaps, you're going to reuse them.
   * <pre>
   * IN:  (csPixmap *)iBitmap
   * </pre>
   */
  cscmdStaticSetBitmap,
  /**
   * Get static control bitmap handle
   * <pre>
   * IN:  nothing
   * OUT: (csPixmap *)iBitmap
   * </pre>
   */
  cscmdStaticGetBitmap
};

/**
 * The Static component class represents a decorative control
 * (widget) which usually does not have functionality, but
 * serves as a decoration. Static controls can be created of
 * many different styles (see csStaticStyle definition above),
 * and they can look completely different one from another.
 * The only common 'function' of all static controls is that they
 * can act as 'dialog labels', i.e. the text assigned to a static
 * component can contain a 'hot' letter, and if you press 'Alt+letter'
 * the first selectable component following the static one will be
 * activated.
 */
class CS_CRYSTALSPACE_EXPORT csStatic : public csComponent
{
protected:
  // Character number that should be underlined (-1 == none)
  size_t underline_pos;
  // Static component style
  csStaticStyle style;
  // Component to which this label is linked
  csComponent *link;
  // The bitmap (if style == csscsBitmap)
  csPixmap *Bitmap;
  // Text alignment (for csscsText style)
  int TextAlignment;
  // Old keyboard and mouse owner
  csComponent *oldKO;
  // link is focused?
  bool linkactive;
  // link is focused?
  bool linkdisabled;

public:
  /// Create static component object (by default - a label) linked to another
  csStatic (csComponent *iParent, csComponent *iLink, const char *iText,
    csStaticStyle iStyle = csscsLabel);
  /// Create static component object without text (by default - a rectangle)
  csStatic (csComponent *iParent, csStaticStyle iStyle = csscsRectangle);
  /// Create static bitmap object
  csStatic (csComponent *iParent, csPixmap *iBitmap);

  /// Destroy the static object
  virtual ~csStatic ();

  /// Set static component text
  virtual void SetText (const char *iText);

  /// Draw the static component
  virtual void Draw ();

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

  /// Pre-handle input events
  virtual bool PostHandleEvent (iEvent &Event);

  /// Return the recommended minimal size of static object
  virtual void SuggestSize (int &w, int &h);

  /// Set text alignment (only for csscsText style)
  void SetTextAlign (int iTextAlignment)
  { TextAlignment = iTextAlignment; }

  /// Set linked component
  void SetLink (csComponent *iLink)
  { link = iLink; CheckUp (); }

protected:
  // Common part of constructors
  void Init (csStaticStyle iStyle);
  // Check if event is a hotkey event
  bool IsHotKey (iEvent &Event);
  // Check if focused status of link has changed
  void CheckUp ();
};

/** @} */

#endif // __CS_CSSTATIC_H__
