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
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/comp.h"
#include "ivaria/cegui.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

#include "ceguievthandler.h"
#include "ceguiresourceprovider.h"
#include "CEGUI.h"

struct iObjectRegistry;

class csCEGUITexture;

/**
* The actual implementation of the CEGUI wrapper for CS.
*/
class csCEGUIRenderer : public CEGUI::Renderer, public iCEGUI, public iComponent
{
public:

  SCF_DECLARE_IBASE;

  /// Constructor.
  csCEGUIRenderer (iBase *parent);

  /// Destructor.
  virtual ~csCEGUIRenderer ();

  /**
   * Initialize CEGUI plugin, optionally specifying the width and height
   * of the CEGUI display size.
   */
  virtual bool Initialize (int width, int height);

  /// Initialize with an iObjectRegistry pointer (called by plugin loader).
  virtual bool Initialize (iObjectRegistry *reg) 
  {
    obj_reg = reg; 
    return true;
  }

  /// Render the GUI.
  virtual void Render () const
  {
    ceguisystem->renderGUI();
  }

  /// Get a reference to the CEGUI system.
  virtual CEGUI::System& GetSystem () const
  {return *ceguisystem;}

  /// Get a pointer to the CEGUI system.
  virtual CEGUI::System* GetSystemPtr () const
  {return ceguisystem;}

  /// Get a reference to the CEGUI font manager.
  virtual CEGUI::FontManager& GetFontManager () const
  {return CEGUI::FontManager::getSingleton();}

  /// Get a pointer to the CEGUI font manager.
  virtual CEGUI::FontManager* GetFontManagerPtr () const
  {return CEGUI::FontManager::getSingletonPtr();}

  /// Get a reference to the CEGUI global event set.
  virtual CEGUI::GlobalEventSet& GetGlobalEventSet () const
  {return CEGUI::GlobalEventSet::getSingleton();}

  /// Get a pointer to the CEGUI global event set.
  virtual CEGUI::GlobalEventSet* GetGlobalEventSetPtr () const
  {return CEGUI::GlobalEventSet::getSingletonPtr();}

  /// Get a reference to the CEGUI imageset manager.
  virtual CEGUI::ImagesetManager& GetImagesetManager () const
  {return CEGUI::ImagesetManager::getSingleton();}

  /// Get a pointer to the CEGUI imageset manager.
  virtual CEGUI::ImagesetManager* GetImagesetManagerPtr () const
  {return CEGUI::ImagesetManager::getSingletonPtr();}

  /// Get a reference to the CEGUI logger.
  virtual CEGUI::Logger& GetLogger () const
  {return CEGUI::Logger::getSingleton();}

  /// Get a pointer to the CEGUI logger.
  virtual CEGUI::Logger* GetLoggerPtr () const
  {return CEGUI::Logger::getSingletonPtr();}

  /// Get a reference to the CEGUI mouse cursor.
  virtual CEGUI::MouseCursor& GetMouseCursor () const
  {return CEGUI::MouseCursor::getSingleton();}

  /// Get a pointer to the CEGUI mouse cursor.
  virtual CEGUI::MouseCursor* GetMouseCursorPtr () const
  {return CEGUI::MouseCursor::getSingletonPtr();}

  /// Get a reference to the CEGUI scheme manager.
  virtual CEGUI::SchemeManager& GetSchemeManager () const
  {return CEGUI::SchemeManager::getSingleton();}

  /// Get a pointer to the CEGUI scheme manager.
  virtual CEGUI::SchemeManager* GetSchemeManagerPtr () const
  {return CEGUI::SchemeManager::getSingletonPtr();}

  /// Get a reference to the CEGUI window factory manager.
  virtual CEGUI::WindowFactoryManager& GetWindowFactoryManager () const
  {return CEGUI::WindowFactoryManager::getSingleton();}

  /// Get a pointer to the CEGUI window factory manager.
  virtual CEGUI::WindowFactoryManager* GetWindowFactoryManagerPtr () const
  {return CEGUI::WindowFactoryManager::getSingletonPtr();}

  /// Get a reference to the CEGUI window manager.
  virtual CEGUI::WindowManager& GetWindowManager () const
  {return CEGUI::WindowManager::getSingleton();}

  /// Get a pointer to the CEGUI window manager.
  virtual CEGUI::WindowManager* GetWindowManagerPtr () const
  {return CEGUI::WindowManager::getSingletonPtr();}

  /// Set the display size of the CEGUI render area.
  void setDisplaySize (const CEGUI::Size& sz);

  /// Destroy all textures that are still active.
  virtual void destroyAllTextures ();

private:
  iObjectRegistry* obj_reg;
  csCEGUIEventHandler* events;
  csCEGUIResourceProvider* resourceProvider;
  CEGUI::System* ceguisystem;

  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;

  /// Add a quad to the renderer queue.
  virtual void addQuad (const CEGUI::Rect& dest_rect, float z, 
    const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect, 
    const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode);

  /// Render the GUI.
  virtual void doRender ();

  /// Remove all meshes in the render list.
  virtual void clearRenderList ();

  /**
   * Set if queueing for quads should be enabled.
   * If enabled, queueing gives a large speed boost when quads are rendered
   * several times, although it is better to disable this
   * for the mouse cursor and similar things. This method is
   * called from CEGUI.
   */
  virtual void setQueueingEnabled (bool setting)
  {
    queueing = setting;
  }

  struct RenderQuad
  {
    csVector2 tex[4];
    csVector4 color[4];
    csVector3 vertex[4];
    uint indices[6];
  };

  struct QuadInfo
  {
    csCEGUITexture* texid;
    CEGUI::Rect	position;
    float z;
    CEGUI::Rect	texPosition;

    csVector4 topLeftColor;
    csVector4 topRightColor;
    csVector4 bottomLeftColor;
    csVector4 bottomRightColor;

    CEGUI::QuadSplitMode splitMode;
  };

  /// Create an empty texture.
  virtual CEGUI::Texture* createTexture ();

  /// Create a texture based on a filename, belonging to a special resource group.
  virtual CEGUI::Texture* createTexture (const CEGUI::String& filename, 
    const CEGUI::String& resourceGroup);

  /// Create an empty texture, but specify its size (square, and power of 2).
  virtual CEGUI::Texture* createTexture (float size);

  /// Destroy a texture, given the pointer to it.
  virtual void destroyTexture (CEGUI::Texture* texture);

  /// Check if queuing is enabled or not.
  virtual bool isQueueingEnabled () const
  {
    return queueing;
  }

  /// Get the display area width.
  virtual float getWidth () const
  {
    return m_displayArea.getWidth();
  }

  /// Get the display area height.
  virtual float getHeight () const
  {
    return m_displayArea.getHeight();
  }

  /// Get the display area size.
  virtual CEGUI::Size getSize () const
  {
    return m_displayArea.getSize();
  }

  /// Get the display area.
  virtual CEGUI::Rect getRect () const
  {
    return m_displayArea;
  }

  /// Get the maximum possible texture size.
  virtual uint getMaxTextureSize () const
  {
    return m_maxTextureSize;
  }

  /// Get the horizontal dots per inch.
  virtual uint getHorzScreenDPI () const
  {
    return 96;
  }

  /// Get the vertical dots per inch.
  virtual uint getVertScreenDPI () const
  {
    return 96;
  }

  /// Create a resource provider, which enables the use of VFS.
  virtual CEGUI::ResourceProvider* createResourceProvider ();

  /**
   * Adds all current vertices in buffer to a mesh, 
   * to enable rendering.
   */
  void UpdateMeshList();

  /**
   * Prepare a quad for rendering. This fills a RenderQuad structure
   * with vertex, index, color, and texcoord information.
   */
  void PrepareQuad (const QuadInfo quad, RenderQuad& rquad) const;

  /// Render a quad directly instead of queueing it.
  void RenderQuadDirect (const CEGUI::Rect& dest_rect, float z, 
    const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect, 
    const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode);

  /// Convert a CEGUI::colour to a CS csVector4 color
  csVector4 ColorToCS (const CEGUI::colour &color) const;

  csArray<QuadInfo> quadList;

  bool newQuadAdded;

  CEGUI::Rect m_displayArea;

  RenderQuad myBuff[2048];

  bool queueing;
  int m_bufferPos;
  csCEGUITexture* texture;

  //!< List used to track textures.
  csPDelArray<csCEGUITexture> textureList;

  csPDelArray<csSimpleRenderMesh> meshList;

  bool meshIsValid;

  /// Maximum supported texture size (in pixels).
  uint m_maxTextureSize;
};

#endif
