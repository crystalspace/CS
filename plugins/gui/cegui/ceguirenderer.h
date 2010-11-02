/*
    Copyright (C) 2005 Dan Hardfeldt, Seth Yastrov and Jelle Hellemans

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

#ifndef _CS_CEGUI_RENDERER_H
#define _CS_CEGUI_RENDERER_H

/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/vector4.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/weakref.h"
#include "iutil/comp.h"
#include "ivaria/icegui.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

#include "ceguiimports.h"
#include "ceguievthandler.h"
#include "ceguiresourceprovider.h"
#include "ceguiscriptmodule.h"

#include "windowfactory.h"
#include "config/settingslider.h"
#include "config/settingcombobox.h"

#include <vector>

namespace CEGUI
{
  class GeometryBuffer;
  class Texture;
  class ResourceProvider;
  class ImageCodec;
}

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  class GeometryBuffer;
  class Texture;
  class ResourceProvider;
  class ImageCodec;
  class RenderTarget;
  class TextureTarget;

  /**
   * The actual implementation of the CEGUI wrapper for CS.
   */
  class Renderer : public CEGUI::Renderer, 
    public scfImplementation2<Renderer, 
    iCEGUI,
    iComponent>
  {
  public:
    /// Constructor.
    Renderer (iBase *parent);

    /// Destructor.
    virtual ~Renderer ();

    /**
     * Initialize the plugin.
     * \param script iScript plugin to use as a scripting module.
     */
    virtual bool Initialize (iScript* script=0);

    virtual bool IsInitialized () { return initialized; }

    /// Initialize with an iObjectRegistry pointer (called by plugin loader).
    virtual bool Initialize (iObjectRegistry *reg) 
    {
      obj_reg = reg; 
      return true;
    }

    /// Render the GUI.
    virtual void Render () const
    {
      CEGUI::System::getSingletonPtr()->renderGUI();
    }

    /// Get a pointer to the CEGUI system.
    virtual CEGUI::System* GetSystemPtr () const
    {return CEGUI::System::getSingletonPtr();}

    /// Get a pointer to the CEGUI font manager.
    virtual CEGUI::FontManager* GetFontManagerPtr () const
    {return CEGUI::FontManager::getSingletonPtr();}

    /// Get a pointer to the CEGUI global event set.
    virtual CEGUI::GlobalEventSet* GetGlobalEventSetPtr () const
    {return CEGUI::GlobalEventSet::getSingletonPtr();}

    /// Get a pointer to the CEGUI imageset manager.
    virtual CEGUI::ImagesetManager* GetImagesetManagerPtr () const
    {return CEGUI::ImagesetManager::getSingletonPtr();}

    /// Get a pointer to the CEGUI logger.
    virtual CEGUI::Logger* GetLoggerPtr () const
    {return CEGUI::Logger::getSingletonPtr();}

    /// Get a pointer to the CEGUI mouse cursor.
    virtual CEGUI::MouseCursor* GetMouseCursorPtr () const
    {return CEGUI::MouseCursor::getSingletonPtr();}

    /// Get a pointer to the CEGUI scheme manager.
    virtual CEGUI::SchemeManager* GetSchemeManagerPtr () const
    {return CEGUI::SchemeManager::getSingletonPtr();}

    /// Get a pointer to the CEGUI window factory manager.
    virtual CEGUI::WindowFactoryManager* GetWindowFactoryManagerPtr () const
    {return CEGUI::WindowFactoryManager::getSingletonPtr();}

    /// Get a pointer to the CEGUI window manager.
    virtual CEGUI::WindowManager* GetWindowManagerPtr () const
    {return CEGUI::WindowManager::getSingletonPtr();}

    /// Allow CEGUI to capture mouse events.
    void EnableMouseCapture ();

    /// Keep CEGUI from capturing mouse events.
    void DisableMouseCapture ();

    /// Allow CEGUI to capture keyboard events.
    void EnableKeyboardCapture ();

    /// Keep CEGUI from capturing keyboard events.
    void DisableKeyboardCapture ();


    // implement CEGUI::Renderer interface
    CEGUI::RenderingRoot& getDefaultRenderingRoot();
    CEGUI::GeometryBuffer& createGeometryBuffer();
    void destroyGeometryBuffer(const CEGUI::GeometryBuffer& buffer);
    void destroyAllGeometryBuffers();
    CEGUI::TextureTarget* createTextureTarget();
    void destroyTextureTarget(CEGUI::TextureTarget* target);
    void destroyAllTextureTargets();
    CEGUI::Texture& createTexture();
    CEGUI::Texture& createTexture(const CEGUI::String& filename, const CEGUI::String& resourceGroup);
    CEGUI::Texture& createTexture(const CEGUI::Size& size);
    // iCEGUI interface
    CEGUI::Texture& CreateTexture(iTextureHandle* htxt);
    void destroyTexture(CEGUI::Texture& texture);
    void destroyAllTextures();
    void beginRendering();
    void endRendering();
    void setDisplaySize(const CEGUI::Size& sz);
    const CEGUI::Size& getDisplaySize() const;
    const CEGUI::Vector2& getDisplayDPI() const;
    uint getMaxTextureSize() const;
    const CEGUI::String& getIdentifierString() const;

    void SetAutoRender (bool autoRender);
    bool GetAutoRender ();
  protected:

    /// String holding the renderer identification text.
    static CEGUI::String d_rendererID;
    /// What the renderer considers to be the current display size.
    CEGUI::Size d_displaySize;
    /// What the renderer considers to be the current display DPI resolution.
    CEGUI::Vector2 d_displayDPI;
    /// The default rendering root object
    CEGUI::RenderingRoot* d_defaultRoot;
    /// The default RenderTarget (used by d_defaultRoot)
    RenderTarget* d_defaultTarget;
    /// container type used to hold TextureTargets we create.
    typedef std::vector<TextureTarget*> TextureTargetList;
    /// Container used to track texture targets.
    TextureTargetList d_textureTargets;
    /// container type used to hold GeometryBuffers we create.
    typedef std::vector<GeometryBuffer*> GeometryBufferList;
    /// Container used to track geometry buffers.
    GeometryBufferList d_geometryBuffers;
    /// container type used to hold Textures we create.
    typedef std::vector<Texture*> TextureList;
    /// Container used to track textures.
    TextureList d_textures;
    /// What the renderer thinks the max texture size is.
    uint d_maxTextureSize;

  private:
    bool initialized;
    iObjectRegistry* obj_reg;
    CEGUIEventHandler* events;
    CEGUIScriptModule* scriptModule;

    csRef<iGraphics3D> g3d;
    csRef<iGraphics2D> g2d;

    csWindowFactory<SettingSlider> settingsSliderFact;
    csWindowFactory<SettingComboBox> settingComboBoxFact;
  
    class AutoRenderEventHandler :
      public scfImplementation1<AutoRenderEventHandler, iEventHandler>
    {
      csWeakRef<Renderer> renderer;
    public:
      AutoRenderEventHandler (Renderer* renderer)
       : scfImplementationType (this), renderer (renderer) {}
    
      bool HandleEvent (iEvent& ev)
      {
	if (renderer) renderer->Render ();
	return true;
      }
      
      CS_EVENTHANDLER_PHASE_2D("crystalspace.cegui.autorender");
    };
    csRef<AutoRenderEventHandler> autoRenderHandler;
    
    void InstallAutoEventHandler ();
    void RemoveAutoEventHandler ();
  };


} CS_PLUGIN_NAMESPACE_END(cegui)

#endif  // end of guard _CS_CEGUI_RENDERER_H
