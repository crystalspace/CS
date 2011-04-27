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
#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include "csutil/custom_new_enable.h"

struct iObjectRegistry;
struct iScript;
struct iTextureHandle;

/**
 * Interface for the CS CEGUI wrapper.
 */
struct iCEGUI : public virtual iBase
{
  SCF_INTERFACE (iCEGUI, 3, 0, 1);

  /**
   * Initialize the plugin.
   * \param script iScript plugin to use as a scripting module.
   */
  virtual bool Initialize (iScript* script=0) = 0;

  virtual bool IsInitialized () = 0;

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

  /// Allow CEGUI to capture mouse events.
  virtual void EnableMouseCapture () = 0;

  /// Keep CEGUI from capturing mouse events.
  virtual void DisableMouseCapture () = 0;

  /// Allow CEGUI to capture keyboard events.
  virtual void EnableKeyboardCapture () = 0;

  /// Keep CEGUI from capturing keyboard events.
  virtual void DisableKeyboardCapture () = 0;

  /// Create a texture from a CS texturehandle.
  virtual CEGUI::Texture& CreateTexture (iTextureHandle* htxt)= 0;
  
  /**
   * Enable/disable auto rendering.
   * Auto rendering causes the GUI to be rendered every frame, during the "2D"
   * phase (see #CS_EVENTHANDLER_PHASE_2D). By default, auto rendering is
   * disables.
   */
  virtual void SetAutoRender (bool autoRender) = 0;
  /// Query auto rendering
  virtual bool GetAutoRender () = 0;
};

#endif
