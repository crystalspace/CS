/*
  Copyright (C) 2010-11 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_IVARIA_HUDMANAGER_H__
#define __CS_IVARIA_HUDMANAGER_H__

/**\file 
 * Head-Up Display tool for the display of information for the user
 */

/**
 * \addtogroup appframe
 * @{ */

#include "cssysdef.h"
#include "csutil/scf.h"

struct iStringArray;

namespace CS {
namespace Utility {

/**
 * A generic tool managing the display of a minimal text-based HUD, eg for applications
 * implementing CS::Utility::DemoApplication or providing a user interface through the keyboard.
 *
 * The HUD consists of the Crystal Space logo, the list of available keyboard and mouse actions
 * that can be used to interact with the demo, and a list of strings
 * describing the current state of the application.
 *
 * You need to setup or update the list of keys and current states (keyDescriptions and
 * stateDescriptions) whenever they change. The description of the state is augmented with
 * informations such as the current Frames Per Second.
 */
struct iHUDManager : public virtual iBase
{
  SCF_INTERFACE (CS::Utility::iHUDManager, 1, 0, 0);

  /**
   * Switch to the next page describing the list of available keyboard keys. This is useful
   * when the list of available keyboard keys is too big and needs to be split in several
   * different pages.
   */
  virtual void SwitchKeysPage () = 0;

  /**
   * Display a 2D text with a shadow. Additional parameters can be defined,
   * they will be formated into the text string by using the cs_snprintf()-style
   * formatting directives.
   */
  virtual void WriteShadow (int x, int y, int color, const char *str,...) const = 0;
  /**
   * Display a 2D text. Additional parameters can be defined,
   * they will be formated into the text string by using the cs_snprintf()-style
   * formatting directives.
   */
  virtual void Write (int x, int y, int fg, int color, const char *str,...) const = 0;

  /**
   * Set whether or not the HUD will be displayed. If not enabled, then this manager
   * will not be displayed nor active at all.
   */
  virtual void SetEnabled (bool enabled) = 0;

  /**
   * Get whether or not the HUD is displayed. If not enabled, then this manager
   * will not be displayed nor active at all.
   */
  virtual bool GetEnabled () const = 0;

  /**
   * Return the array of string describing the available user keys (eg <tt>'d: toggle debug mode'</tt>).
   * You can manipulate this array in order to define the list of keys to be displayed.
   * By default the array is filled with the keys available to move the camera. You
   * can call iStringArray::Empty() if you don't want these keys to be displayed.
   */
  virtual iStringArray* GetKeyDescriptions () = 0;

  /**
   * Return the array of string describing the state of the application (eg <tt>'Debug mode enabled'</tt>).
   * You can manipulate this array in order to define the list of state descriptions to be displayed.
   */
  virtual iStringArray* GetStateDescriptions () = 0;
};

} //namespace Utility
} //namespace CS

/** @} */

#endif // __CS_IVARIA_HUDMANAGER_H__
