/*
    Crystal Space Windowing System: check box button class
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

#ifndef __CS_CSCHKBOX_H__
#define __CS_CSCHKBOX_H__

/**\file
 * Crystal Space Windowing System: check box button class
 */

/**
 * \addtogroup csws_comps_chkbx
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"
#include "csbutton.h"

/**
 * \name Check box style flags
 * @{ */
/// Checkbox type mask
#define CSBS_CBTYPEMASK		0x00010000
/// The checkbox is a 2-state checkbox (on/off)
#define CSBS_CB2STATE		0x00000000
/// The checkbox is a 3-state checkbox (on/off/indefinite)
#define CSBS_CB3STATE		0x00010000
/// The checkbox automatically switches between states (on->off{->indefinite})
#define CSBS_CBAUTO		0x00020000
/// Default checkbox button style
#define CSBS_DEFAULTCHECKBOX    \
	(CSBS_SELECTABLE | CSBS_CB2STATE | CSBS_CBAUTO)
/** @} */

/// Check box messages
enum
{
  /**
   * Send to a checkbox to set its state
   * <pre>
   * IN: (csCheckBoxState)newstate
   * </pre>
   */
  cscmdCheckBoxSet = 0x00000800,
  /**
   * Query the state of a check box
   * <pre>
   * OUT: (csCheckBoxState)state
   * </pre>
   */
  cscmdCheckBoxQuery,
  /**
   * Notify parent that check box state has changed
   * <pre>
   * IN: (csCheckBox *)source
   * </pre>
   */
  cscmdCheckBoxSwitched
};

/// Possible check box states
enum csCheckBoxState
{
  /// The check box has no "checked" mark inside
  cscbsNonChecked,
  /// The check box contains a "check" mark
  cscbsChecked,
  /// The checkbox state is indefinite (hashed state)
  cscbsIndefinite
};

/**
 * The csCheckBox class is a close child of csButton class.
 * The check boxes contains a fixed bitmap which changes depending on
 * checkbox state and no text.
 */
class CS_CRYSTALSPACE_EXPORT csCheckBox : public csButton
{
  /// Checkbox state
  csCheckBoxState CheckBoxState;
public:
  /// Create a checkbox object
  csCheckBox (csComponent *iParent, int iButtonID, int iButtonStyle =
    CSBS_DEFAULTCHECKBOX);

  /// Handle external events
  virtual bool HandleEvent (iEvent &Event);

protected:
  /// Emulate a button press (generate command)
  virtual void Press ();
  /// Change bitmaps used for displaying normal/pressed states
  void SetButtBitmap (char *id_n, char *id_p);
  /// Set checkbox state
  void SetCheckBoxState (csCheckBoxState iNewState);
};

/** @} */

#endif // __CS_CSCHKBOX_H__
