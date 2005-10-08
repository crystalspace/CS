/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

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

#ifndef __CS_IVARIA_CEGUI_H__
#define __CS_IVARIA_CEGUI_H__

/**\file
 * CEGUI wrapper interface
 */

#include "csutil/scf.h"
#include "CEGUI.h"

SCF_VERSION (iCEGUI, 0, 0, 1);

struct iObjectRegistry;
struct iScript;

/**
 * Interface for the CS CEGUI wrapper.
 */
struct iCEGUI : public virtual iBase
{
  /**
   * Initialize the plugin.
   * \param script iScript plugin to use as a scripting module.
   */
  virtual bool Initialize (iScript* script=0) = 0;

  /// Render the GUI.
  virtual void Render () const = 0;

  /// Get a pointer to the CEGUI::System singleton.
  virtual CEGUI::System* GetSystemPtr () const = 0;

  /// Get a pointer to the CEGUI::FontManager singleton.
  virtual CEGUI::FontManager* GetFontManagerPtr () const = 0;

  /// Get a pointer to the CEGUI::GlobalEventSet singleton.
  virtual CEGUI::GlobalEventSet* GetGlobalEventSetPtr () const = 0;

  /// Get a pointer to the CEGUI::ImagesetManager singleton.
  virtual CEGUI::ImagesetManager* GetImagesetManagerPtr () const = 0;

  /// Get a pointer to the CEGUI::Logger singleton.
  virtual CEGUI::Logger* GetLoggerPtr () const = 0;

  /// Get a pointer to the CEGUI::MouseCursor singleton.
  virtual CEGUI::MouseCursor* GetMouseCursorPtr () const = 0;

  /// Get a pointer to the CEGUI::SchemeManager singleton.
  virtual CEGUI::SchemeManager* GetSchemeManagerPtr () const = 0;

  /// Get a pointer to the CEGUI::WindowFactoryManager singleton.
  virtual CEGUI::WindowFactoryManager* GetWindowFactoryManagerPtr () const = 0;

  /// Get a pointer to the CEGUI::WindowManager singleton.
  virtual CEGUI::WindowManager* GetWindowManagerPtr () const = 0;
};

#endif
