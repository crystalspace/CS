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

#include "cssysdef.h"

#include "iengine/camera.h"
#include "csgeom/math.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/imagememory.h"
#include "cstool/csview.h"

#include "cstool/uberscreenshot.h"

namespace CS
{
  bool UberScreenshotMaker::DrawTile3D (uint tileLeft, uint tileTop,
      uint tileRight, uint tileBottom)
  {
    const uint tileW = tileRight - tileLeft;
    const uint tileH = tileBottom - tileTop;
    
    csRef<iCustomMatrixCamera> newCam (engine->CreateCustomMatrixCamera (originalCam));
    shotView->SetCustomMatrixCamera (newCam);
    CS::Math::Matrix4 oldProjection (newCam->GetCamera()->GetProjectionMatrix());
    float xScale = float (ubershotW)/float (screenW);
    float yScale = float (ubershotH)/float (screenH);
    // X offset appears inverted? Not sure why... well, whatever
    float xOffset =
      (xScale-1.0f) 				// Offset for leftmost tiles
      -(float (2*tileLeft)/float (screenW));	// Tile's actual X offset
    float yOffset = 
      (float (2*tileTop)/float (screenH))
      -(yScale-1.0f);
    CS::Math::Matrix4 projectionTransform (xScale, 0, 0, xOffset,
					   0, yScale, 0, yOffset,
					   0, 0, 1, 0,
					   0, 0, 0, 1);
    newCam->SetProjectionMatrix (projectionTransform * oldProjection);
    
    shotView->SetRectangle (0, screenH - tileH, tileW, tileH);
    
    if (!g3d->BeginDraw (CSDRAW_3DGRAPHICS))
      return false;
    
    shotView->Draw ();

    g3d->FinishDraw ();
    g3d->Print (0);
    return true;
  }

  csRef<iImage> UberScreenshotMaker::TakeScreenshot (uint tileLeft, 
    uint tileTop, uint tileRight, uint tileBottom)
  {
    csRef<iImage> shot = g2d->ScreenShot();

    const uint tileW = tileRight - tileLeft;
    const uint tileH = tileBottom - tileTop;
    if ((tileW < screenW) || (tileH < screenH))
      shot = csImageManipulate::Crop (shot, 0, 0, tileW, tileH);
    return shot;
  }
  
  csRef<iImage> UberScreenshotMaker::PostProcessImage (csImageMemory* img)
  {
    return img;
  }
  
  void UberScreenshotMaker::Setup (iCamera* camera, iEngine* engine, 
    iGraphics3D* g3d)
  {
    this->g3d = g3d;
    g2d = g3d->GetDriver2D();
    this->engine = engine;
    shotView.AttachNew (new csView (engine, g3d));
    
    screenW = g3d->GetWidth();
    screenH = g3d->GetHeight();
    
    originalCam = camera;
  }
  
  UberScreenshotMaker::UberScreenshotMaker (uint width, uint height, 
    iCamera* camera, iEngine* engine, iGraphics3D* g3d) : ubershotW (width), 
    ubershotH (height)
  {
    Setup (camera, engine, g3d);
  }
    
  UberScreenshotMaker::UberScreenshotMaker (uint width, uint height, 
    iView* view) : ubershotW (width), ubershotH (height)
  {
    Setup (view->GetCamera(), view->GetEngine(), view->GetContext());
  }
  
  csPtr<iImage> UberScreenshotMaker::Shoot ()
  {
    csRef<csImageMemory> finalImage;
    finalImage.AttachNew (new csImageMemory (ubershotW, ubershotH));
    
    int cMinX, cMinY, cMaxX, xMaxY;
    g2d->GetClipRect (cMinX, cMinY, cMaxX, xMaxY);
    g2d->SetClipRect (0, 0, screenW, screenH);
    
    const uint tilesX = (ubershotW + (screenW - 1)) / screenW;
    const uint tilesY = (ubershotH + (screenH - 1)) / screenH;
    for (uint ty = 0; ty < tilesY; ty++)
    {
      for (uint tx = 0; tx < tilesX; tx++)
      {
        const uint tileL = tx * screenW;
        const uint tileT = ty * screenH;
        const uint tileR = csMin (tileL+screenW, ubershotW);
        const uint tileB = csMin (tileT+screenH, ubershotH);
      
        csRef<iImage> shot = ShootTile (tileL, tileT, tileR, tileB);
        if (!shot.IsValid ())
        {
          g2d->SetClipRect (cMinX, cMinY, cMaxX, xMaxY);
          return 0;
	}
  
        finalImage->Copy (shot, tileL, tileT, 
          tileR - tileL, tileB - tileT);
      }
    }
    
    g2d->SetClipRect (cMinX, cMinY, cMaxX, xMaxY);
    return csPtr<iImage> (PostProcessImage (finalImage));
  }
}
