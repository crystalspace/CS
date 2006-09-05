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

class csEngine;
class csImposterMesh;

class csImposterProcTex : public scfImplementation0<csImposterProcTex>
{
private:
  //parent billboard
  csImposterMesh *mesh;

  //r2t texture
  iTextureWrapper *tex;

  //flag if this imposter is currently in queue to be rendered to texture
  bool updating;

  //imposter texture size
  int w, h;

  //clipper for r2t
  iClipper2D *clip;

  //cached csStringID values
  csStringID stringid_standard;
  csStringID stringid_light_ambient;

  //convenience shortcuts
  csEngine *engine;
  csRef<iGraphics3D> g3d;
  csRef<iGraphics2D> g2d;
  csRef<iShaderManager> shadermanager;

  void SetImposterReady (bool r);

public:
  csImposterProcTex (csEngine* engine, csImposterMesh *parent);
  ~csImposterProcTex ();

  bool GetImposterReady () { return !updating; }
  void RenderToTexture (iRenderView *rview, iSector *s);
  void Update ();
  csImposterMesh *GetParent () { return mesh; }
  iTextureWrapper *GetTexture () { return tex; }
};

#endif // __CS_IMPPRCTX_H__

