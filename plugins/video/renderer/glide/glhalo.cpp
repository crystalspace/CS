/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written for Glide by Norman Kraemer
    
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
#include "glhalo.h"
#include "gllib.h"
#include <glide.h>
#include <unistd.h>

#define SNAP ((float)(3L<<18))
SCF_IMPLEMENT_IBASE (csGlideHalo)
  SCF_IMPLEMENTS_INTERFACE(iHalo)
SCF_IMPLEMENT_IBASE_END

csGlideHalo::csGlideHalo (float iR, float iG, float iB, int iWidth, int iHeight, csGraphics3DGlide *iG3D, csGlideAlphaMap *am){

  SCF_CONSTRUCT_IBASE(NULL);

  R = int( iR * 255.);
  G = int( iG * 255.);
  B = int( iB * 255.);
  Width = iWidth;
  Height = iHeight;
  (G3D = iG3D)->IncRef();
  this->am = am;
  vert = new csVector2[ 4 ];
  vx = new MyGrVertex[4];
  oldVerts=4;
}

csGlideHalo::~csGlideHalo(){
  if ( am != NULL )
    G3D->m_pAlphamapCache->Remove (am);
  G3D->DecRef();
  delete [] vert;
  delete am;
  if ( vx ) delete [] vx;
}

void csGlideHalo::Draw (float x, float y, float w, float h, float iIntensity, 
                         csVector2 *iVertices, int iVertCount)
{
    csVector2 *v;
    int i = 0;
    if (w<0) w = Width;
    if (h<0) h = Height;
    int p2width, p2height;
    am->GetMipMapDimensions ( 0, p2width, p2height );

    G3D->m_pAlphamapCache->Add( am, true );
    TextureHandler &texhnd = ((csGlideCacheData*)am->GetCacheData())->texhnd;
    
    int screenheight = G3D->GetHeight();
    if ( !iVertices )
    {
      v = vert;
      v[0].x = MAX( 0, x );
      v[1].x = MAX( 0, x );
      v[2].x = MIN( x + w, G3D->GetWidth() );
      v[3].x = MIN( x + w, G3D->GetWidth() );
      v[0].y = MIN( y + w, screenheight );
      v[1].y = MAX( 0, y );
      v[2].y = MAX( 0, y );
      v[3].y = MIN( y + w, screenheight );
      iVertCount = 4;
    }
    else
      v = iVertices;

    if ( iVertCount > oldVerts )
    {
      vx = new MyGrVertex[ i ];
      oldVerts = iVertCount;
    }

    float u_scale = p2width;
    u_scale /= Width;
    float v_scale = p2height;
    v_scale /= Height;
    // Build the vertex structure
    for ( i=0; i < iVertCount; i++ )
    {
      vx[i].x = v[i].x ;
      vx[i].y = screenheight - v[i].y ;
      vx[i].oow = 1.f;
      vx[i].tmuvtx[0].sow = ((v[i].x - x)/Width) * texhnd.width/u_scale;
      vx[i].tmuvtx[0].tow = ((v[i].y - y)/Height) * texhnd.height/v_scale;
    }

    int ci = (int)(255.0f * (float)iIntensity);
    GlideLib_grConstantColorValue( ( ci << 24 ) | ( R << 16 ) | ( G << 8 ) | B );
    
    TextureHandler *haloinfo = &((csGlideCacheData*)am->GetCacheData())->texhnd;
    
    if ( G3D->m_pAlphamapCache == G3D->m_pTextureCache )
      GlideLib_grTexClampMode (haloinfo->tmu->tmu_id, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP );
    GlideLib_grTexCombine(haloinfo->tmu->tmu_id,
                          GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                          GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                          FXFALSE, FXFALSE);

    GlideLib_grColorCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
                              GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_CONSTANT, FXFALSE );
    GlideLib_grAlphaCombine ( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_LOCAL,
                              GR_COMBINE_LOCAL_CONSTANT, GR_COMBINE_OTHER_TEXTURE, FXFALSE );

    GlideLib_grAlphaBlendFunction ( GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA,
                                    GR_BLEND_ZERO, GR_BLEND_ZERO);
    GlideLib_grTexSource(haloinfo->tmu->tmu_id, haloinfo->loadAddress,
                         GR_MIPMAPLEVELMASK_BOTH,
                         &haloinfo->info);

    GlideLib_grDepthMask (FXFALSE);
    
    GlideLib_grDrawPlanarPolygonVertexList (iVertCount, vx);
    GlideLib_grDepthMask (FXTRUE);

    if ( G3D->m_pAlphamapCache == G3D->m_pTextureCache )
      GlideLib_grTexClampMode (haloinfo->tmu->tmu_id, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP );
}
