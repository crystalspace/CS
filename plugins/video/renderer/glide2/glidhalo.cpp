/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "glidhalo.h"
#include "g3dglide.h"
#include "glidelib.h"

IMPLEMENT_IBASE (csGlideHalo)
  IMPLEMENTS_INTERFACE(iHalo)
IMPLEMENT_IBASE_END

csGlideHalo::csGlideHalo (float iR, float iG, float iB, unsigned char *iAlpha, int iWidth, int iHeight, csGraphics3DGlide2x *iG3D, csG3DHardwareHaloInfo *iHaloinfo){

  CONSTRUCT_IBASE(NULL);

  R = iR;
  G = iG;
  B = iB;
  (void)iAlpha;
  Width = iWidth;
  Height = iHeight;
  (G3D = iG3D)->IncRef();
  haloinfo = iHaloinfo;
  
}

csGlideHalo::~csGlideHalo(){
  if ( haloinfo != NULL ){
    G3D->m_pTextureCache->UnloadHalo(((csG3DHardwareHaloInfo*)haloinfo)->halo);
    delete haloinfo;
  }
  G3D->DecRef();
}

void csGlideHalo::Draw (float x, float y, float w, float h, float iIntensity, csVector2 *iVertices, int iVertCount){

    csVector3 *pCenter = new csVector3( x + Width/2.0, y + Height/2.0, y + Height/2.0 );
    
    if (pCenter->x > Width || pCenter->x < 0 || pCenter->y > Height || pCenter->y < 0  ) return;
/*
    int izz = QInt24 (1.0f / pCenter->z);
    HRESULT hRes = S_OK;

    unsigned long zb = z_buffer[(int)pCenter->x + (width * (int)pCenter->y)];

          // first, do a z-test to make sure the halo is visible
    if (izz < (int)zb)
      hRes = S_FALSE;
*/

    GrVertex vx[4];
    
    int ci = (int)(255.0f * (float)iIntensity);
    float len = ((float)Width/6.0);

    vx[0].a = ci; vx[0].r = ci; vx[0].g = ci; vx[0].b = ci;
    vx[0].x = pCenter->x - len;
    vx[0].y = pCenter->y - len;
    vx[0].z = pCenter->z;
    vx[0].oow = pCenter->z;
    vx[0].tmuvtx[0].sow = 0;
    vx[0].tmuvtx[0].tow = 0;

    vx[1].a = ci; vx[1].r = ci; vx[1].g = ci; vx[1].b = ci;
    vx[1].x = pCenter->x + len;
    vx[1].y = pCenter->y - len;
    vx[1].z = pCenter->z;
    vx[1].oow = pCenter->z;
    vx[1].tmuvtx[0].sow = 128;
    vx[1].tmuvtx[0].tow = 0;

    vx[2].a = ci; vx[2].r = ci; vx[2].g = ci; vx[2].b = ci;
    vx[2].x = pCenter->x + len;
    vx[2].y = pCenter->y + len;
    vx[2].z = pCenter->z;
    vx[2].oow = pCenter->z;
    vx[2].tmuvtx[0].sow = 128;
    vx[2].tmuvtx[0].tow = 128;

    vx[3].a = ci; vx[3].r = ci; vx[3].g = ci; vx[3].b = ci;
    vx[3].x = pCenter->x - len;
    vx[3].y = pCenter->y + len;
    vx[3].z = pCenter->z;
    vx[3].oow = pCenter->z;
    vx[3].tmuvtx[0].sow = 0;
    vx[3].tmuvtx[0].tow = 128;
    
    if(G3D->m_iMultiPass)
    {
      GlideLib_grAlphaBlendFunction( GR_BLEND_ONE_MINUS_SRC_COLOR, GR_BLEND_ZERO,
                                    GR_BLEND_DST_ALPHA, GR_BLEND_ZERO);
    }
    else // disable single pass blending
    {
      GlideLib_grTexCombine(G3D->m_TMUs[0].tmu_id,
                            GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);
    }

    GlideLib_grDepthBufferFunction(GR_CMP_ALWAYS);
    GlideLib_grDepthMask(FXFALSE);

    HighColorCacheAndManage_Data *halo=((csG3DHardwareHaloInfo*)haloinfo)->halo;
    TextureHandler *thTex = (TextureHandler *)halo->pData;
    GlideLib_grTexSource(thTex->tmu->tmu_id, thTex->loadAddress,
                         GR_MIPMAPLEVELMASK_BOTH,
                         &thTex->info);
    
    GlideLib_grDrawPlanarPolygonVertexList(4, vx);
    
    GlideLib_grDepthBufferFunction(GR_CMP_LEQUAL);
    GlideLib_grDepthMask(FXTRUE);

    if(G3D->m_iMultiPass)
    {
      GlideLib_grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                                    GR_BLEND_ONE, GR_BLEND_ZERO);
    }
    else // enable single pass blending
    {
      GlideLib_grTexCombine(G3D->m_TMUs[0].tmu_id,
                            GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                            GR_COMBINE_FUNCTION_ZERO, GR_COMBINE_FACTOR_NONE,
                            FXFALSE,FXFALSE);
    }

    delete pCenter;
}
