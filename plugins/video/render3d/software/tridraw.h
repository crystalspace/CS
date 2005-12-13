/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_SOFT3D_TRIDRAW_H__
#define __CS_SOFT3D_TRIDRAW_H__

#include "sft3dcom.h"

namespace cspluginSoft3d
{
  using namespace CrystalSpace::SoftShader;

  class TriangleDrawerCommon : public iTriangleDrawer
  {
  protected:
    csSoftwareGraphics3DCommon* g3d;

    // A, uh, few members to contain clipping intermediates and such.
    // @@@ FIXME Make all drawer share one set
    VertexBuffer clipInBuf[maxBuffers];
    size_t clipInStride[maxBuffers];
    VertexBuffer clipOutBuf[maxBuffers];
    iRenderBuffer** activebuffers;
    BuffersMask buffersMask;
    static const int outFloatsPerBuf = 16;
    float clipOut[maxBuffers * outFloatsPerBuf];
    ClipMeatZNear clipZNear;
    BuffersClipper<ClipMeatZNear> bclipperZNear;
    csVector3 outPersp[4];
    size_t floatsPerVert;
    csDirtyAccessArray<csVector3> clippedPersp;
    csDirtyAccessArray<float> out;
    VertexBuffer clipOutBuf2[maxBuffers];

    bool do_mirror;
    iScanlineRenderer::RenderInfoMesh scanRenderInfoMesh;
    iScanlineRenderer::RenderInfoTriangle scanRenderInfoTri;
    float texDimension;

    // Info to go over triangles
    csRenderMeshType meshtype;
    uint32* tri;
    const uint32* triEnd;
    uint32 old2, old1;
    int quadPart;

    /* Near clipping may produces 2 tris, needs some special handling.
     * Indicated by this flag. */
    bool nearClipTri2;

    void ProjectVertices (size_t rangeStart, size_t rangeEnd)
    {
      size_t num_vertices = rangeEnd + 1;
  
      csDirtyAccessArray<csVector3>& persp = g3d->persp;
      persp.SetLength (num_vertices);
      const int width2 = g3d->width2;
      const int height2 = g3d->height2;
      const float aspect = g3d->aspect;
      csRenderBufferLock<csVector3, iRenderBuffer*> work_verts 
	(activebuffers[VATTR_SPEC(POSITION)], CS_BUF_LOCK_READ);
      // Perspective project.
      for (size_t i = rangeStart; i <= rangeEnd; i++)
      {
	if (work_verts[i].z >= SMALL_Z)
	{
	  persp[i].z = 1.0f / work_verts[i].z;
	  float iz = aspect * persp[i].z;
	  persp[i].x = work_verts[i].x * iz + width2;
	  persp[i].y = work_verts[i].y * iz + height2;
	}
	else
	  persp[i] = work_verts[i];
      }
    }
    void BufInvZMulAndDenorm (size_t n)
    {
      csVector4* denormFact = scanRenderInfoTri.denormFactors;
      for (size_t i = 0; i < maxBuffers; i++)
      {
	if (!(scanRenderInfoMesh.desiredBuffers & (1 << i))) continue;
	if (!(buffersMask & (1 << i))) 
	{
	  if (scanRenderInfoTri.denormBuffers & (1 << i)) denormFact++;
	  continue;
	}
  
	csVector4* bufData = 
	  (csVector4*)clipOutBuf[i].data;
	if (scanRenderInfoTri.denormBuffers & (1 << i))
	{
	  for (size_t v = 0; v < n; v++)
	  {
	    // Denormalize buffers
	    const float iz = outPersp[v].z;
	    bufData->Set (bufData->x * denormFact->x * iz,
	      bufData->y * denormFact->y * iz,
	      bufData->z * denormFact->z * iz,
	      bufData->w * denormFact->w * iz);
	    bufData++;
	  }
	  denormFact++;
	}
	else
	{
	  for (size_t v = 0; v < n; v++)
	  {
	    const float iz = outPersp[v].z;
	    *bufData *= iz;
	    bufData++;
	  }
	}
      }
    }
    size_t ClipTriangle (const size_t* trivert)
    {
      //-----
      // Do backface culling. Note that this depends on the
      // mirroring of the current view.
      //-----
      const csVector2& pa = *(csVector2*)&outPersp[trivert[0]];
      const csVector2& pb = *(csVector2*)&outPersp[trivert[1]];
      const csVector2& pc = *(csVector2*)&outPersp[trivert[2]];
      float area = csMath2::Area2 (pa, pb, pc);
      if (!area) return 0;
      if (do_mirror)
      {
	if (area <= -SMALL_EPSILON) return 0;
      }
      else
      {
	if (area >= SMALL_EPSILON) return 0;
      }
  
      // Clip triangle. Note that the clipper doesn't care about the
      // orientation of the triangle vertices. It works just as well in
      // mirrored mode.
  
      /* You can only have as much clipped vertices as the sum of vertices in 
      * the original poly and those in the clipping poly... I think. */
      const size_t maxClipVertices = g3d->clipper->GetVertexCount() + 3;
      ClipMeatiClipper meat;
      meat.Init (g3d->clipper, maxClipVertices);
      out.SetSize (floatsPerVert * maxClipVertices);
      clippedPersp.SetSize (maxClipVertices);
      const size_t* compNum = scanRenderInfoMesh.bufferComps;
      float* outPos = out.GetArray();
      for (size_t i = 0; i < maxBuffers; i++)
      {
	if (!(scanRenderInfoMesh.desiredBuffers & (1 << i))) continue;
  
	const size_t c = *compNum;
	if (buffersMask & (1 << i))
	{
	  clipOutBuf2[i].data = (uint8*)outPos;
	  clipOutBuf2[i].comp = c;
	  clipInStride[i] = clipOutBuf[i].comp * sizeof(float);
	}
	else
	{
	  size_t n = maxClipVertices * c;
	  float* vtx = outPos + n - 1;
	  while (n-- > 0)
	  {
	    const float iz = outPersp[n / c].z;
	    *vtx-- = ((i == VATTR_SPEC(COLOR)) || ((i % c) == 3)) ? iz : 0.0f;
	  }
	}
	outPos += c * maxClipVertices;
	compNum++;
      }
  
      BuffersClipper<ClipMeatiClipper> clip (meat);
      clip.Init (outPersp, clippedPersp.GetArray(),
	clipOutBuf, clipInStride, clipOutBuf2, 
	buffersMask & scanRenderInfoMesh.desiredBuffers);
      csTriangle tri;
      if (do_mirror)
      {
	tri.a = (int)trivert[2];
	tri.b = (int)trivert[1];
	tri.c = (int)trivert[0];
      }
      else
      {
	tri.a = (int)trivert[0];
	tri.b = (int)trivert[1];
	tri.c = (int)trivert[2];
      }
      return clip.DoClip (tri);
    }

    int PickMipmap (size_t n)
    {
      if (!(buffersMask & CS_SOFT3D_BUFFERFLAG(TEXCOORD)))
	return 0;

      size_t closestVert = 0;
      float closestZ = outPersp[0].z;
      for (size_t i = 1; i < n; i++)
      {
	const float iz = outPersp[i].z;
	if (iz > closestZ)
	{
	  closestZ = iz;
	  closestVert = i;
	}
      }
      size_t nextVert = (closestVert == n-1) ? 0 : closestVert+1;
      size_t prevVert = (closestVert == 0) ? n-1 : closestVert-1;

      const csVector4* texcoords = 
	(csVector4*)clipOutBuf[VATTR_SPEC(TEXCOORD)].data;
      const csVector2 tc (texcoords[closestVert].x, texcoords[closestVert].y);
      const csVector2 tcPrev (texcoords[prevVert].x, texcoords[prevVert].y);
      const csVector2 tcNext (texcoords[nextVert].x, texcoords[nextVert].y);

      csVector2 screenDiff1 (outPersp[prevVert].x - outPersp[closestVert].x,
	outPersp[prevVert].y - outPersp[closestVert].y);
      csVector2 screenDiff2 (outPersp[nextVert].x - outPersp[closestVert].x,
	outPersp[nextVert].y - outPersp[closestVert].y);
      // Compute TCs for pixels at one step towards 'prev' resp. 'next'
      const csVector2 tcIz (tc * outPersp[closestVert].z);
      const float n1 = screenDiff1.Norm();
      const float p1 = n1 ? 1.0f / n1 : 0;
      const csVector2 tc1 = (Lerp (tcIz, 
	tcPrev * outPersp[prevVert].z, p1) 
	/ Lerp (outPersp[closestVert].z, outPersp[prevVert].z, p1));
      const float n2 = screenDiff2.Norm();
      const float p2 = n2 ? 1.0f / n2 : 0;
      const csVector2 tc2 = (Lerp (tcIz, 
	tcNext * outPersp[nextVert].z, p2) 
	/ Lerp (outPersp[closestVert].z, outPersp[nextVert].z, p2));
      /* Determine the number of texels a pixel covers (average the two
       * lengths) */
      float texelSide = (csVector2 (tc1 - tc).Norm() + 
	csVector2 (tc2 - tc).Norm()) * texDimension;
      if (texelSide >= 8)
	return 3;
      else if (texelSide >= 4)
	return 2;
      else if (texelSide >= 2)
	return 1;

      return 0;
    }

    bool NextTriangle (const csVector3*& clippedPersp, size_t& num,
      uint a, uint b, uint c)
    {
      csTriangle tri;
      tri.a = a;
      tri.b = b;
      tri.c = c;
      /* Small Z clipping. Also projects unprojected vertices (skipped in
       * ProjectVertices() due a Z coord too small) and will invert the Z
       * of the pespective verts. */
      size_t n = bclipperZNear.DoClip (tri);
      if (n == 0) return false;
      CS_ASSERT((n >= 3) && (n <= 4));
  
      // Do scanline per-tri setup
      const int mipmap = PickMipmap (n);
      SoftwareTexture* textures[activeTextureCount];
      csSoftwareTextureHandle** activeTex = g3d->activeSoftTex;
      for (size_t i = 0; i < activeTextureCount; i++)
      {
	if (activeTex[i])
	{
	  if (activeTex[i]->GetFlags() & CS_TEXTURE_NOMIPMAPS)
	    textures[i] = activeTex[i]->GetTexture (0);
	  else
	    textures[i] = activeTex[i]->GetTexture (mipmap);
	}
	else
	  textures[i] = 0;
      }
      if (!g3d->scanlineRenderer->SetupTriangle (textures, scanRenderInfoMesh, 
	scanRenderInfoTri))
	return false;

      BufInvZMulAndDenorm (n);
  
      static const size_t trivert1[3] = { 0, 1, 2 };
      num = ClipTriangle (trivert1);
      clippedPersp = this->clippedPersp.GetArray();
      if (n == 4)
      {
	nearClipTri2 = true;
      }
      return num != 0;
    }
  public:
    void BeginTriangulate (csRenderMeshType meshtype, 
      uint32* tri, const uint32* triEnd)
    { 
      this->meshtype = meshtype;
      this->tri = tri;
      this->triEnd = triEnd;
      nearClipTri2 = false;
      quadPart = 0;

      switch (meshtype)
      {
      case CS_MESHTYPE_TRIANGLESTRIP:
      case CS_MESHTYPE_TRIANGLEFAN:
	{
	  old2 = *tri++;
	  old1 = *tri++;
	  break;
	}
      default:
	;
      }
    }

    bool HasNextTri() const
    {
      return (tri < triEnd) || nearClipTri2;
    }
    void NextTri (const csVector3*& clippedPersp, size_t& num)
    {
      if (nearClipTri2)
      {
	nearClipTri2 = false;
	static const size_t trivert2[3] = { 0, 2, 3 };
	num = ClipTriangle (trivert2);
	clippedPersp = this->clippedPersp.GetArray();
	if (num != 0) return;
      }

      bool hasTri = false;
      while (!hasTri && !nearClipTri2 && (tri < triEnd))
      {
	switch (meshtype)
	{
	case CS_MESHTYPE_TRIANGLES:
	  {
	    hasTri = NextTriangle (clippedPersp, num, tri[0], tri[1], tri[2]);
	    tri += 3;
	  }
	  break;
	case CS_MESHTYPE_TRIANGLESTRIP:
	  {
	    const uint32 cur = *tri++;
	    hasTri = NextTriangle (clippedPersp, num, old2, old1, cur);
	    old2 = old1;
	    old1 = cur;
	    break;
	  }
	case CS_MESHTYPE_TRIANGLEFAN:
	  {
	    const uint32 cur = *tri++;
	    hasTri = NextTriangle (clippedPersp, num, old2, old1, cur);
	    old1 = cur;
	    break;
	  }
	case CS_MESHTYPE_QUADS:
	  {
	    if (quadPart == 0)
	      hasTri = NextTriangle (clippedPersp, num, tri[0], tri[1], tri[2]);
	    else
	    {
	      hasTri = NextTriangle (clippedPersp, num, tri[0], tri[2], tri[3]);
	      tri += 4;
	    }
	    quadPart ^= 1;
	    break;
	  }
	default:
	  ;
	}
      }
      if (!hasTri) num = 0;
    }
    void SetupDrawMesh (iRenderBuffer* activebuffers[], size_t rangeStart, 
      size_t rangeEnd, const csCoreRenderMesh* mesh,
      const iScanlineRenderer::RenderInfoMesh& scanRenderInfoMesh,
      const csRenderMeshType meshtype, uint32* tri, const uint32* triEnd)
    {
      this->activebuffers = activebuffers;
      do_mirror = mesh->do_mirror;
      this->scanRenderInfoMesh = scanRenderInfoMesh;
  
      csSoftwareTextureHandle* tex = g3d->activeSoftTex[0];
      if (tex)
      {
	int tw, th;
	tex->GetRendererDimensions (tw, th);
	texDimension = float (csMin (tw, th));
      }
      else
	texDimension = 0;

      const size_t bufNum = csMin (activeBufferCount, maxBuffers);
  
      floatsPerVert = 0;
  
      buffersMask = 0;
      const size_t* compNum = scanRenderInfoMesh.bufferComps;
      for (size_t b = 0; b < bufNum; b++)
      {
	if (scanRenderInfoMesh.desiredBuffers & (1 << b))
	{
	  floatsPerVert += *compNum;
	  compNum++;
	}
	if (activebuffers[b] == 0) continue;
	buffersMask |= 1 << b;
	if ((b != CS_SOFT3D_VA_BUFINDEX(POSITION)) 
	  && !(scanRenderInfoMesh.desiredBuffers & (1 << b))) continue;
  
	iRenderBuffer* buf = activebuffers[b];
	clipInBuf[b].data = (uint8*)buf->Lock (CS_BUF_LOCK_READ);
	clipInBuf[b].comp = buf->GetComponentCount();
	clipInStride[b] = buf->GetElementDistance();
	clipOutBuf[b].data = (uint8*)&clipOut[b * outFloatsPerBuf];
	clipOutBuf[b].comp = 4;
      }
  
      ProjectVertices (rangeStart, rangeEnd);
  
      clipZNear.Init (g3d->width2, g3d->height2, g3d->aspect);
      bclipperZNear.Init (g3d->persp.GetArray(), outPersp,
	clipInBuf, clipInStride, clipOutBuf, 
	(buffersMask & scanRenderInfoMesh.desiredBuffers) 
	  | (CS_SOFT3D_BUFFERFLAG(POSITION)));

      BeginTriangulate (meshtype, tri, triEnd);
    }
    void FinishDrawMesh ()
    {
      const size_t bufNum = csMin (activeBufferCount, maxBuffers);
      for (size_t b = 0; b < bufNum; b++)
      {
	if (activebuffers[b] != 0) activebuffers[b]->Release();
      }
    }
  public:
    TriangleDrawerCommon (csSoftwareGraphics3DCommon* g3d) : g3d(g3d),
      bclipperZNear(clipZNear)
    {}
  };

  template<typename Pix, typename SrcBlend, typename DstBlend>
  class TriangleDrawer : public TriangleDrawerCommon
  {
    Pix pix;
    PolygonRasterizer<SLLogic_ScanlineRenderer<Pix, SrcBlend, DstBlend> > 
      polyrast;
  
  public:
    TriangleDrawer (csSoftwareGraphics3DCommon* g3d) : 
      TriangleDrawerCommon (g3d), pix(g3d->pfmt)
    {
    }
  
    ~TriangleDrawer()
    {
    }
  
    void DrawMesh (iRenderBuffer* activebuffers[], size_t rangeStart, 
      size_t rangeEnd, const csCoreRenderMesh* mesh,
      const iScanlineRenderer::RenderInfoMesh& scanRenderInfoMesh,
      const csRenderMeshType meshtype, uint32* tri, const uint32* triEnd)
    {
      polyrast.Init (g3d->pfmt, g3d->width, g3d->height, g3d->z_buffer,
	g3d->line_table);
      SetupDrawMesh (activebuffers, rangeStart, rangeEnd, mesh, 
	scanRenderInfoMesh, meshtype, tri, triEnd);

      while (HasNextTri ())
      {
	const csVector3* clippedPersp;
	size_t num;

	NextTri (clippedPersp, num);

	if (num == 0) continue;

	SLLogic_ScanlineRenderer<Pix, SrcBlend, DstBlend> sll (
	  pix, scanRenderInfoMesh, scanRenderInfoTri,
	  clipOutBuf2, buffersMask & scanRenderInfoMesh.desiredBuffers,
	  floatsPerVert);
	polyrast.DrawPolygon (num, clippedPersp, sll);
      }
  
      FinishDrawMesh ();
    }
  };
    
  template<typename Pix>
  struct TriDrawMatrixFiller
  {
    template<typename SrcBlend>
    static iTriangleDrawer* NewTriangleDrawerM (csSoftwareGraphics3DCommon* g3d, 
						uint dstF)
    {
      switch (dstF)
      {
	default:
	case CS_MIXMODE_FACT_ZERO:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_Zero<FactorColorDst, 0> > (g3d);
	case CS_MIXMODE_FACT_ONE:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_Zero<FactorColorDst, 1> > (g3d);

	case CS_MIXMODE_FACT_SRCCOLOR:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_Src<FactorColorDst, 0> > (g3d);
	case CS_MIXMODE_FACT_SRCCOLOR_INV:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_Src<FactorColorDst, 1> > (g3d);

	case CS_MIXMODE_FACT_SRCALPHA:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_SrcAlpha<FactorColorDst, 0> > (g3d);
	case CS_MIXMODE_FACT_SRCALPHA_INV:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_SrcAlpha<FactorColorDst, 1> > (g3d);

	case CS_MIXMODE_FACT_DSTCOLOR:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_Dst<FactorColorDst, 0> > (g3d);
	case CS_MIXMODE_FACT_DSTCOLOR_INV:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_Dst<FactorColorDst, 1> > (g3d);

	case CS_MIXMODE_FACT_DSTALPHA:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_DstAlpha<FactorColorDst, 0> > (g3d);
	case CS_MIXMODE_FACT_DSTALPHA_INV:
	  return new TriangleDrawer<Pix, SrcBlend, Factor_DstAlpha<FactorColorDst, 1> > (g3d);
      }
    }
    static iTriangleDrawer* NewTriangleDrawer (csSoftwareGraphics3DCommon* g3d, 
					       uint srcF, uint dstF)
    {
      switch (srcF)
      {
	default:
	case CS_MIXMODE_FACT_ZERO:
	  return NewTriangleDrawerM<Factor_Zero<FactorColorSrc, 0> > (g3d, dstF);
	case CS_MIXMODE_FACT_ONE:
	  return NewTriangleDrawerM<Factor_Zero<FactorColorSrc, 1> > (g3d, dstF);

	case CS_MIXMODE_FACT_SRCCOLOR:
	  return NewTriangleDrawerM<Factor_Src<FactorColorSrc, 0> > (g3d, dstF);
	case CS_MIXMODE_FACT_SRCCOLOR_INV:
	  return NewTriangleDrawerM<Factor_Src<FactorColorSrc, 1> > (g3d, dstF);

	case CS_MIXMODE_FACT_SRCALPHA:
	  return NewTriangleDrawerM<Factor_SrcAlpha<FactorColorSrc, 0> > (g3d, dstF);
	case CS_MIXMODE_FACT_SRCALPHA_INV:
	  return NewTriangleDrawerM<Factor_SrcAlpha<FactorColorSrc, 1> > (g3d, dstF);

	case CS_MIXMODE_FACT_DSTCOLOR:
	  return NewTriangleDrawerM<Factor_Dst<FactorColorSrc, 0> > (g3d, dstF);
	case CS_MIXMODE_FACT_DSTCOLOR_INV:
	  return NewTriangleDrawerM<Factor_Dst<FactorColorSrc, 1> > (g3d, dstF);

	case CS_MIXMODE_FACT_DSTALPHA:
	  return NewTriangleDrawerM<Factor_DstAlpha<FactorColorSrc, 0> > (g3d, dstF);
	case CS_MIXMODE_FACT_DSTALPHA_INV:
	  return NewTriangleDrawerM<Factor_DstAlpha<FactorColorSrc, 1> > (g3d, dstF);
      }
    }
  public:
    static void Fill (csSoftwareGraphics3DCommon* g3d, 
		      iTriangleDrawer** matrix)
    {
      for (uint srcF = 0; srcF < CS_MIXMODE_FACT_COUNT; srcF++)
      {
	for (uint dstF = 0; dstF < CS_MIXMODE_FACT_COUNT; dstF++)
	{
	  uint index = srcF*CS_MIXMODE_FACT_COUNT + dstF;
	  matrix[index] = NewTriangleDrawer (g3d, srcF, dstF);
	}
      }
    }
  };
} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_TRIDRAW_H__
