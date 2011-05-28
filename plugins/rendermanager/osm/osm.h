/*
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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

#ifndef __CS_RM_OSM_H__
#define __CS_RM_OSM_H__

#include "cssysdef.h"
#include "itexture.h"
#include "csutil/scf_implementation.h"
#include "iengine/rendermanager.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMOSM)
{
  class RMOSM : public scfImplementation2<RMOSM, iRenderManager, iComponent>
  {
  public:

    /// Constructor.
    RMOSM(iBase *parent);

    //---- iComponent Interface ----
    virtual bool Initialize(iObjectRegistry *registry);

    //---- iRenderManager Interface ----
    virtual bool RenderView(iView *view);
    virtual bool PrecacheView(iView *view);

  protected:

    iObjectRegistry *objRegistry;

    csRef<iTextureHandle> accumBuffer;

  };
}
CS_PLUGIN_NAMESPACE_END(RMOSM)

#endif // __CS_RM_OSM_H__
