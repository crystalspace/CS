/*
    Crystal Space Windowing System: spin box class
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

#ifndef __CS_CSSPINBX_H__
#define __CS_CSSPINBX_H__

/**\file
 * Crystal Space Windowing System: spin box class
 */

/**
 * \addtogroup csws_comps_spinbox
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"
#include "cstimer.h"
#include "csiline.h"
#include "csutil/stringarray.h"

/// Spin box item structure
struct csSpinBoxItem
{
  /// Item value
  char *Value;
  /// Item position (ordinal of element before which item will be inserted)
  int Position;
};

/**
 * \name Spin box item placement values
 * @{ */
/// Insert item before all other items
#define CSSB_ITEM_BEFOREALL	0
/// Insert item after all other items
#define CSSB_ITEM_AFTERALL	999999
/** @} */

/// Spin box upper/lower limits.
struct csSpinBoxLimits
{
  int MinValue,MaxValue;
  char *ValueFormat;
};

/// Spin box messages
enum
{
  /**
   * Query value of spin box
   * <pre>
   * OUT: (int)Value
   * </pre>
   */
  cscmdSpinBoxQueryValue = 0x00000A00,
  /**
   * Set spin box value
   * <pre>
   * IN: (int)Value
   * </pre>
   */
  cscmdSpinBoxSetValue,
  /**
   * Insert a string into list of spin box's values
   * <pre>
   * IN: (csSpinBoxItem *)Item
   * OUT: (int)Ordinal
   * </pre>
   */
  cscmdSpinBoxInsertItem,
  /**
   * Set spinbox type to numerical and set its limits
   * <pre>
   * IN: (csSpinBoxLimits *)Limits
   * </pre>
   */
  cscmdSpinBoxSetLimits,
  /**
   * Passed to parent control as spinbox value changes
   * <pre>
   * IN: (csSpinBox *)Source
   * </pre>
   */
  cscmdSpinBoxValueChanged
};

/**
 * The spinbox class is a combination of an input line and a bi-directional
 * arrow button which can be used to switch input line contents back and
 * forth between a set of predefined values.
 */
class CS_CRYSTALSPACE_EXPORT csSpinBox : public csInputLine
{
  /// Current spin box value
  int Value;
  /// The list of allowed values; if list is empty, spin box is numerical
  csStringArray Values;
  /// Alternative value limits for numerical content
  csSpinBoxLimits NumLimits;
  /// Spin box state
  int SpinState;
  /// Spin timer
  csTimer *SpinTimer;
  /// spin box size in pixels
  int SpinBoxSize;
  /// Number of autorepeats since spin beginning
  int AutoRepeats;

public:
  /// Create spin box object
  csSpinBox (csComponent *iParent,
    csInputLineFrameStyle iFrameStyle = csifsThickRect);

  /// Destructor
  virtual ~csSpinBox ();

  /// Draw the spin box
  virtual void Draw ();

  /// Handle external events
  virtual bool HandleEvent (iEvent &Event);

  /// Set spin box type to numerical and set spin limits
  void SetLimits (int iMin, int iMax, char *iFormat = "%d");

  /**
   * Set spin box content.<p>
   * If spin box type is numerical, the iValue parameter is the number
   * that will be set in entry field (after it will be clipped to Min..Max),
   * if spin box type is enumerated, iValue is the ordinal of value to be set.
   */
  void SetValue (int iValue);

  /// Insert a item into spinbox (same as csSpinBoxInsertItem message)
  int InsertItem (char *iValue, int iPosition);

  /// Set spinbox text
  virtual void SetText (const char *iText);

private:
  /// Spin to other value
  void Spin (int iDelta);
  /// Spin value depending on spin box state
  void Spin ();
};

/** @} */

#endif // __CS_CSSPINBX_H__
