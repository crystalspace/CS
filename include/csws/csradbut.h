/*
    Crystal Space Windowing System: radio button class
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

#ifndef __CS_CSRADBUT_H__
#define __CS_CSRADBUT_H__

/**\file
 * Crystal Space Windowing System: radio button class
 */

/**
 * \addtogroup csws_comps_radbut
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"
#include "csbutton.h"

/// Default radio button style
#define CSBS_DEFAULTRADIOBUTTON	(CSBS_SELECTABLE)

/// Radio button messages
enum
{
  /**
   * Send to a radio button to set its state
   * <pre>
   * IN: (bool)newstate
   * </pre>
   */
  cscmdRadioButtonSet = 0x00000900,
  /**
   * Query the state of a radio button
   * <pre>
   * OUT: (bool)state
   * </pre>
   */
  cscmdRadioButtonQuery,
  /**
   * Notify parent that radio button has been selected
   * <pre>
   * IN: (csRadioButton *)source
   * </pre>
   */
  cscmdRadioButtonSelected
};

/**
 * The csRadioButton class is a close child of csButton class.
 * The radio button contains a fixed bitmap which changes depending on
 * its state (selected/unselected) and no text. Radio buttons are
 * usualy tied in groups of which only one can be selected, and when
 * user selects one previous selection automatically deselects.
 * The first radio button in group should have the CSS_GROUP flag set.
 */
class CS_CRYSTALSPACE_EXPORT csRadioButton : public csButton
{
  /// Radio button state
  bool RadioButtonState;
public:
  /// Create a radio button object
  csRadioButton (csComponent *iParent, int iButtonID, int iButtonStyle =
    CSBS_DEFAULTRADIOBUTTON);

  /// Handle external events
  virtual bool HandleEvent (iEvent &Event);

protected:
  /// Emulate a button press (generate command)
  virtual void Press ();
  /// Change bitmaps used for displaying normal/pressed states
  void SetButtBitmap (char *id_n, char *id_p);
  /// Set radio button state
  void SetRadioButtonState (bool iNewState);
};

/** @} */

#endif // __CS_CSRADBUT_H__
