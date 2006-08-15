/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein
    Rewritten during Sommer of Code 2006 by Christoph "Fossi" Mewes

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

#include "csutil/ref.h"
#include "csgeom/vector3.h"
#include "iengine/engine.h"
#include "iengine/rview.h"
#include "cstool/procmesh.h"

class csEngine;
class csImposterMesh;

class csImposterProcTex : public scfImplementation0<csImposterProcTex>
{
private:
  //parent billboard
  csImposterMesh *mesh;

  //r2t texture
  iTextureWrapper *tex;

  //flag if this imposter was rendered to texture and can be used
  bool imposter_ready;

  //imposter texture size
  int w, h;

  //cached csStringID values
  csStringID stringid_standard;
  csStringID stringid_light_ambient;

  //convenience shortcuts
  csEngine* engine;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;

public:
  csImposterProcTex (csEngine* engine, csImposterMesh *parent);
  ~csImposterProcTex ();

  bool GetImposterReady () { return imposter_ready; }
  void SetImposterReady (bool r) { imposter_ready = r; }
  void Animate (iRenderView *rview, iSector *s);
  csImposterMesh *GetParent () { return mesh; }
  iTextureWrapper *GetTexture () { return tex; }
};

#endif // __CS_IMPPRCTX_H__

