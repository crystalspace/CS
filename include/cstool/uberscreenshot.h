/*
  Copyright (C) 2006 by Frank Richter

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

#ifndef __CSTOOL_UBERSCREENSHOT_H__
#define __CSTOOL_UBERSCREENSHOT_H__

#include "iengine/engine.h"
#include "igraphic/image.h"
#include "ivaria/view.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

class csImageMemory;

namespace CS
{
  /**
   * Helper to create "&uuml;berscreenshots", screenshots with a resolution
   * larger than the current framebuffer resolution.
   *
   * It works by splitting up the ubershot into tiles, creating multiple 
   * screenshots (one for each tile) and stitching those tiles together into
   * a larger image.
   *
   * \remarks Since the ubershot maker bypasses the normal event loop to 
   *   render the tiles, e.g. GUI elements can't appear on the screenshot.
   *   However, you can derive a custom uberscreenshot maker from this
   *   class which implements a custom ShootTile() method to handle GUI 
   *   drawing 
   */
  class CS_CRYSTALSPACE_EXPORT UberScreenshotMaker
  {
  protected:
    /// Width of the uberscreenshot
    uint ubershotW;
    /// Height of the uberscreenshot
    uint ubershotH;
    /// Width of the framebuffer
    uint screenW;
    /// Height of the framebuffer
    uint screenH;
    
    csRef<iGraphics3D> g3d;
    csRef<iGraphics2D> g2d;
    csRef<iEngine> engine;
    /// View used to render the tiles
    csRef<iView> shotView;
    /// Original camera
    csRef<iCamera> originalCam;
    
    /**
     * Draw the view, set up to cover the current tile.
     */
    virtual bool DrawTile3D (uint tileLeft, uint tileTop,
      uint tileRight, uint tileBottom);
    /**
     * Take and crop the actual screenshot.
     */
    virtual csRef<iImage> TakeScreenshot (uint tileLeft, uint tileTop,
      uint tileRight, uint tileBottom);
    /**
     * Shoot the image for a single tile. The area of the tile on the
     * ubershot is given by the parameters.
     *
     * Any custom implementations should call DrawTile3D(), do any custom
     * drawing, and finally call TakeScreenshot().
     */
    virtual csRef<iImage> ShootTile (uint tileLeft, uint tileTop,
      uint tileRight, uint tileBottom)
    {
      if (!DrawTile3D (tileLeft, tileTop, tileRight, tileBottom))
        return 0;
      return TakeScreenshot (tileLeft, tileTop, tileRight, tileBottom);
    }
  
    /**
     * Post-process the final image (e.g. add watermark). By default does
     * nothing.
     */
    virtual csRef<iImage> PostProcessImage (csImageMemory* img);
    
    /// Setup the shotView member.
    void Setup (iCamera* camera, iEngine* engine, iGraphics3D* g3d);
  public:
    /// Initialize for dimensions \p width and \p height.
    UberScreenshotMaker (uint width, uint height, iCamera* camera, 
      iEngine* engine, iGraphics3D* g3d);
    /// Initialize, with camera, engine and g3d taken from \p view.
    UberScreenshotMaker (uint width, uint height, iView* view);
    /// Destroy.
    virtual ~UberScreenshotMaker() {}
    
    /// Create an uberscreenshot.
    csPtr<iImage> Shoot ();
  };
}

#endif // __CSTOOL_UBERSCREENSHOT_H__
