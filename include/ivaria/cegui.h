/*
  Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __CS_IVARIA_CEGUI_H__
#define __CS_IVARIA_CEGUI_H__

#include "csutil/scf.h"
#include "CEGUI.h"

SCF_VERSION (iCEGUI, 0, 0, 1);

struct iObjectRegistry;

/**
 * Interface for the CS CEGUI wrapper.
 */
struct iCEGUI : public virtual iBase
{
  /// Initialize the plugin.
  virtual bool Initialize (int width=-1, int height=-1) = 0;

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
