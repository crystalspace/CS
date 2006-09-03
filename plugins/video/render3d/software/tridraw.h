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

#include "csgeom/polyclip.h"
#include "csgeom/math2d.h"
#include "csgfx/trianglestream.h"
#include "cstool/rbuflock.h"
#include "sft3dcom.h"

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

  using namespace CS::PluginCommon::SoftShader;

  class TriangleDrawerCommon : public iTriangleDrawer
  {
  protected:
    csSoftwareGraphics3DCommon* g3d;

    // A, uh, few members to contain clipping intermediates and such.
    // @@@ FIXME Make all drawer share one set
    ClipMeatZNear clipZNear;
    BuffersClipper<ClipMeatZNear> bclipperZNear;
    csVector3 outPersp[4];
    csDirtyAccessArray<csVector3> clippedPersp;
    csDirtyAccessArray<float> out;

    const VerticesLTN* clipInZNear;
    VerticesLTN clipOutZNear;
    VerticesLTN clipInClipper;
    VerticesLTN clipOutClipper;

    bool do_mirror;
    iScanlineRenderer::RenderInfoMesh scanRenderInfoMesh;
    iScanlineRenderer::RenderInfoTriangle scanRenderInfoTri;
    float texDimension;

    // Info to go over triangles
    csTriangle* triangle;
    size_t triangleCount;

    /* Near clipping may produce 2 tris, needs some special handling.
     * Indicated by this flag. */
    bool nearClipTri2;

    void ProjectVertices (size_t rangeStart, size_t rangeEnd)
    {
      size_t num_vertices = rangeEnd + 1;
  
      csDirtyAccessArray<csVector3>& persp = g3d->persp;
      persp.SetLength (num_vertices);
      const int width2 = g3d->persp_center_x;
      const int height2 = g3d->persp_center_y;
      const float aspect = g3d->aspect;
      const float* work_verts = clipInZNear->GetData() +
        clipInZNear->GetOffset (CS_SOFT3D_VA_BUFINDEX(POSITION));
      const size_t stride = clipInZNear->GetStride();
      work_verts += rangeStart * stride;
      // Perspective project.
      for (size_t i = rangeStart; i <= rangeEnd; i++)
      {
	if (work_verts[2] >= SMALL_Z)
	{
	  persp[i].z = 1.0f / work_verts[2];
	  float iz = aspect * persp[i].z;
	  persp[i].x = work_verts[0] * iz + width2;
	  persp[i].y = work_verts[1] * iz + height2;
	}
	else
          persp[i].Set (work_verts[0], work_verts[1], work_verts[2]);
        work_verts += stride;
      }
    }

    void BufInvZMulAndDenorm (size_t n)
    {
      const size_t stride = clipInClipper.GetStride();
      CS_ALLOC_STACK_ARRAY(float, ltnCoeff, n * stride);
      memset (ltnCoeff, 0, n * stride * sizeof (float));
      csVector4* denormFact = scanRenderInfoTri.denormFactors;
      for (size_t i = 0; i < maxBuffers; i++)
      {
	if (!(scanRenderInfoMesh.desiredBuffers & (1 << i))) continue;

	if (scanRenderInfoTri.denormBuffers & (1 << i))
	{
	  for (size_t v = 0; v < n; v++)
	  {
            float* coeffPtr = ltnCoeff + v * stride + clipInClipper.GetOffset (i);
	    // Denormalize buffers
	    const float iz = outPersp[v].z;
            for (size_t c = 0; c < clipInClipper.GetCompCount (i); c++)
            {
              *coeffPtr++ = (*denormFact)[c] * iz;
            }
	  }
	  denormFact++;
	}
	else
	{
	  for (size_t v = 0; v < n; v++)
	  {
            float* coeffPtr = ltnCoeff + v * stride + clipInClipper.GetOffset (i);
	    const float iz = outPersp[v].z;
            for (size_t c = 0; c < clipInClipper.GetCompCount (i); c++)
            {
              *coeffPtr++ = iz;
            }
	  }
	}
      }
      clipInClipper.Multiply (ltnCoeff);
    }

    size_t ClipTriangle (const size_t* trivert)
    {
      /* Do backface culling. Note that this depends on the
       * mirroring of the current view. */
      const csVector2 pa (outPersp[trivert[0]][0], outPersp[trivert[0]][1]);
      const csVector2 pb (outPersp[trivert[1]][0], outPersp[trivert[1]][1]);
      const csVector2 pc (outPersp[trivert[2]][0], outPersp[trivert[2]][1]);
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
  
      /* Clip triangle. Note that the clipper doesn't care about the
       * orientation of the triangle vertices. It works just as well in
       * mirrored mode. */
  
      /* You can only have as much clipped vertices as the sum of vertices in 
       * the original poly and those in the clipping poly... I think. */
      const size_t maxClipVertices = 
	g3d->clipper ? g3d->clipper->GetVertexCount() + 3 : 7;

      clippedPersp.SetSize (maxClipVertices);
      clipOutClipper.RemoveVertices ();
  
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

      ClipMeatiClipper meat;
      CS_ASSERT (g3d->clipper);
      meat.Init (g3d->clipper, maxClipVertices);
      BuffersClipper<ClipMeatiClipper> clip (meat);
      clip.Init (outPersp, clippedPersp.GetArray(),
        &clipInClipper, &clipOutClipper);
      return clip.DoClip (tri);
    }

    int PickMipmap (size_t n)
    {
      if (!(scanRenderInfoMesh.desiredBuffers & CS_SOFT3D_BUFFERFLAG(TEXCOORD)))
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

      const float* texcoords = clipInClipper.GetData () + 
        clipInClipper.GetOffset (VATTR_SPEC(TEXCOORD));
      const size_t stride = clipInClipper.GetStride();
      const csVector2 tc (texcoords[closestVert*stride+0], 
        texcoords[closestVert*stride+1]);
      const csVector2 tcPrev (texcoords[prevVert*stride+0], 
        texcoords[prevVert*stride+1]);
      const csVector2 tcNext (texcoords[nextVert*stride+0], 
        texcoords[nextVert*stride+1]);

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

    bool NextTriangle (csVector3*& clippedPersp, size_t& num,
      uint a, uint b, uint c)
    {
      csTriangle tri;
      tri.a = a;
      tri.b = b;
      tri.c = c;
      /* Small Z clipping. Also projects unprojected vertices (skipped in
       * ProjectVertices() due a Z coord too small) and will invert the Z
       * of the pespective verts. */
      clipOutZNear.RemoveVertices ();
      size_t n = bclipperZNear.DoClip (tri);
      if (n == 0) return false;
      CS_ASSERT((n >= 3) && (n <= 4));

      clipInClipper.CopyFromMasked (clipOutZNear, 
        scanRenderInfoMesh.desiredBuffers);
  
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
    bool HasNextTri() const
    {
      return (triangleCount > 0) || nearClipTri2;
    }

    void NextTri (csVector3*& clippedPersp, size_t& num)
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
      while (!hasTri && !nearClipTri2 && (triangleCount > 0))
      {
        csTriangle tri = *triangle++;
        triangleCount--;
        hasTri = NextTriangle (clippedPersp, num, tri.a, tri.b, tri.c);
      }
      if (!hasTri) num = 0;
    }

    void SetupDrawMesh (const VerticesLTN& buffers, size_t rangeStart, 
      size_t rangeEnd, const csCoreRenderMesh* mesh,
      const iScanlineRenderer::RenderInfoMesh& scanRenderInfoMesh,
      csTriangle* triangles, size_t triangleCount)
    {
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

      clipInZNear = &buffers;
      clipOutZNear.SetupEmpty (*clipInZNear);
      clipInClipper.SetupEmpty (scanRenderInfoMesh.bufferComps, 
        scanRenderInfoMesh.desiredBuffers);
      clipOutClipper.SetupEmpty (scanRenderInfoMesh.bufferComps, 
        scanRenderInfoMesh.desiredBuffers);
  
      ProjectVertices (rangeStart, rangeEnd);
  
      bclipperZNear.Init (g3d->persp.GetArray(), outPersp,
	clipInZNear, &clipOutZNear);
      clipZNear.Init (g3d->persp_center_x, g3d->persp_center_y, g3d->aspect);

      this->triangle = triangles;
      this->triangleCount = triangleCount;
      nearClipTri2 = false;
    }

    void FinishDrawMesh ()
    {
    }
  public:
    TriangleDrawerCommon (csSoftwareGraphics3DCommon* g3d) : g3d(g3d),
      bclipperZNear (clipZNear)
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
  
    void DrawMesh (const VerticesLTN& buffers, size_t rangeStart, 
      size_t rangeEnd, const csCoreRenderMesh* mesh,
      const iScanlineRenderer::RenderInfoMesh& scanRenderInfoMesh,
      csTriangle* triangles, size_t triangleCount)
    {
      int w, h;
      if (g3d->smallerActive)
      {
	w = g3d->width/2; h = g3d->height/2;
      }
      else
      {
	w = g3d->width; h = g3d->height;
      }
      polyrast.Init (g3d->pfmt, w, h, g3d->z_buffer,
	g3d->line_table, g3d->ilaceActive ? g3d->do_interlaced : -1);
      SetupDrawMesh (buffers, rangeStart, rangeEnd, mesh, 
	scanRenderInfoMesh, triangles, triangleCount);

      while (HasNextTri ())
      {
	csVector3* clippedPersp;
	size_t num;

	NextTri (clippedPersp, num);

	if (num == 0) continue;
      
	SLLogic_ScanlineRenderer<Pix, SrcBlend, DstBlend> sll (
	  pix, scanRenderInfoMesh, scanRenderInfoTri,
          &clipOutClipper);
	if (g3d->smallerActive)
	{
	  for (size_t i = 0; i < num; i++)
	  {
	    clippedPersp[i].x *= 0.5f;
	    clippedPersp[i].y *= 0.5f;
	  }
	}
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
}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_SOFT3D_TRIDRAW_H__
