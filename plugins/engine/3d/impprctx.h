/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein

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

#ifndef __CS_IMPPRCTX_H__
#define __CS_IMPPRCTX_H__

#include <stdarg.h>
#include "csutil/ref.h"
#include "cstool/proctex.h"
#include "csgeom/vector3.h"
#include "iengine/engine.h"
#include "iengine/rview.h"

struct iLoader;
struct iGraphics3D;
struct iKeyboardDriver;
struct iSector;
struct iVFS;
struct iEvent;
struct iTextureHandle;
struct iObjectRegistry;
struct iVirtualClock;
struct iThingState;
struct iMaterialWrapper;
struct iMeshWrapper;
struct iGeneralFactoryState;
class csEngineProcTex;
class csImposterMesh;

class csImposterProcTex : public csProcTexture
{
private:
  iEngine* Engine;
  iRenderView* View;
  csImposterMesh *mesh;
  bool imposter_ready;

public:
  csImposterProcTex (csImposterMesh *parent);
  ~csImposterProcTex ();

  bool GetImposterReady () { return imposter_ready; }
  void SetImposterReady (bool r) { imposter_ready = r; }
  virtual bool PrepareAnim ();
  virtual void Animate (csTicks current_time);
};

#endif // __CS_IMPPRCTX_H__

