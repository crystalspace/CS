/*
    Copyright (C) 2010 by Jelle Hellemans

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

#ifndef __CS_IUTIL_CFGNOTIFIER_H__
#define __CS_IUTIL_CFGNOTIFIER_H__

/**\file
 * Configuration notifier interface.
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"
#include "csutil/ref.h"

/**
 * Configuration listener interface.
 */
struct iConfigListener : public virtual iBase
{
  SCF_INTERFACE(iConfigListener, 1,0,0);

  /// Called when a null-terminated string value has been set.
  virtual void Set (const char* key, const char* value) = 0;
  /// Called when an integer value has been set.
  virtual void Set (const char* key, int value) = 0;
  /// Called when a floating-point value has been set.
  virtual void Set (const char* key, float value) = 0;
  /// Called when a boolean value has been set.
  virtual void Set (const char* key, bool value) = 0;
  /// Called when a tuple value has been set.
  virtual void Set (const char* key, iStringArray* value) = 0;
};

/**
 * Configuration notifier interface.
 */
struct iConfigNotifier : public virtual iBase
{
  SCF_INTERFACE(iConfigNotifier, 1,0,0);

  /**
   * Adds the given iConfigListener to the notifier's list.
   */
  virtual void AddListener (iConfigListener*) = 0;

  /**
   * Removes the given iConfigListener from the notifier's list.
   */
  virtual void RemoveListener (iConfigListener*) = 0;
};
/** @} */

#endif // __CS_IUTIL_CFGFILE_H__
