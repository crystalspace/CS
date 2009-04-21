/*
    Copyright (C) 2008 by Frank Richter

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

#include "csplugincommon/rendermanager/rendertree.h"

#include "ivideo/graph2d.h"

namespace CS
{
  namespace RenderManager
  {
    RenderTreeBase::DebugPersistent::DebugPersistent () : nextDebugId (0) {}
      
    uint RenderTreeBase::DebugPersistent::RegisterDebugFlag (const char* string)
    {
      uint* id = debugIdMappings.GetElementPointer (string);
      if (id != 0) return *id;
      uint newID = nextDebugId++;
      debugIdMappings.Put (string, newID);
      
      csString strToSplit (string);
      while (!strToSplit.IsEmpty())
      {
	size_t dot = strToSplit.FindLast ('.');
	if (dot == (size_t)-1) break;
	strToSplit.Truncate (dot);
	uint parentID = RegisterDebugFlag (strToSplit);
	csArray<uint>& parentChildren = debugIdChildren.GetOrCreate (parentID);
	parentChildren.Push (newID);
      }

      return newID;
    }
  
    uint RenderTreeBase::DebugPersistent::QueryDebugFlag (const char* string)
    {
      uint* id = debugIdMappings.GetElementPointer (string);
      if (id != 0) return *id;
      return (uint)-1;
    }
  
    bool RenderTreeBase::DebugPersistent::IsDebugFlagEnabled (uint flag)
    {
      if (flag >= debugFlags.GetSize()) return false;
      return debugFlags.IsBitSet (flag);
    }
    
    void RenderTreeBase::DebugPersistent::EnableDebugFlag (uint flag, bool state)
    {
      if (flag >= debugFlags.GetSize()) debugFlags.SetSize (flag+1);
      debugFlags.Set (flag, state);
      csArray<uint>* children = debugIdChildren.GetElementPointer (flag);
      if (children == 0) return;
      for (size_t i = 0; i < children->GetSize(); i++)
      {
	uint child = (*children)[i];
	if (child >= debugFlags.GetSize()) debugFlags.SetSize (child+1);
	debugFlags.Set (child, state);
      }
    }
    
    void RenderTreeBase::AddDebugTexture (iTextureHandle* tex, float aspect)
    {
      if (!tex) return;
    
      DebugTexture dt;
      dt.texh = tex;
      dt.aspect = aspect;
      debugTextures.Push (dt);
    }
    
    void RenderTreeBase::RenderDebugTextures (iGraphics3D* g3d)
    {
      if (debugTextures.GetSize() == 0) return;
    
      g3d->BeginDraw (CSDRAW_2DGRAPHICS);
      int scrWidth = g3d->GetWidth();
      int scrHeight = g3d->GetHeight();
      
      int desired_height = scrHeight / 6;
      int total_width = (int)ceilf (debugTextures[0].aspect * desired_height);
      
      for (size_t i = 1; i < debugTextures.GetSize(); i++)
      {
        total_width += (int)ceilf (debugTextures[i].aspect * desired_height) + 16;
      }
      float scale = 1.0f;
      if (total_width > scrWidth)
        scale = (float)scrWidth/(float)total_width;
      
      float left = 0;
      const float top = scrHeight - desired_height * scale;
      const float bottom = scrHeight;
      
      csVector3 coords[4];
      csVector2 tcs[4];
      tcs[0].Set (0, 0);
      tcs[1].Set (1, 0);
      tcs[2].Set (1, 1);
      tcs[3].Set (0, 1);
      
      csSimpleRenderMesh mesh;
      mesh.alphaType.alphaType = csAlphaMode::alphaNone;
      mesh.meshtype = CS_MESHTYPE_QUADS;
      mesh.vertexCount = 4;
      mesh.vertices = coords;
      mesh.texcoords = tcs;
      for (size_t i = 0; i < debugTextures.GetSize(); i++)
      {
        const float right = left + debugTextures[i].aspect * desired_height * scale;
        coords[0].Set (left, top, 0);
        coords[1].Set (right, top, 0);
        coords[2].Set (right, bottom, 0);
        coords[3].Set (left, bottom, 0);
        
        mesh.texture = debugTextures[i].texh;
        g3d->DrawSimpleMesh (mesh, csSimpleMeshScreenspace);
        
        left = right + 16;
      }
      g3d->FinishDraw ();
    }
  
    void RenderTreeBase::AddDebugLine3D (const csVector3& v1, const csVector3& v2,
                                         const csColor& color1, const csColor& color2)
    {
      debugLines.verts.Push (v1);
      debugLines.verts.Push (v2);
      debugLines.colors.Push (
        csVector4 (color1.red, color1.green, color1.blue));
      debugLines.colors.Push (
        csVector4 (color2.red, color2.green, color2.blue));
    }
    
    void RenderTreeBase::AddDebugLine3DTF (const csVector3& v1, const csVector3& v2,
                                           const csTransform& toWorldSpace,
                                           const csColor& color1, const csColor& color2)
    {
      AddDebugLine3D (toWorldSpace.Other2This (v1),
	toWorldSpace.Other2This (v2),
	color1, color2);
    }
    void RenderTreeBase::AddDebugBBox (const csBox3& box,
                                       const csTransform& toWorldSpace,
                                       const csColor& col)
    {
      static const int numEdges = 12;
      static const int edges[numEdges] =
      {
        CS_BOX_EDGE_xyz_Xyz,
        CS_BOX_EDGE_xyz_xYz,
        CS_BOX_EDGE_Xyz_XYz,
        CS_BOX_EDGE_xYz_XYz,
        
        CS_BOX_EDGE_xyZ_XyZ,
        CS_BOX_EDGE_xyZ_xYZ,
        CS_BOX_EDGE_XyZ_XYZ,
        CS_BOX_EDGE_xYZ_XYZ,
        
        CS_BOX_EDGE_xyz_xyZ,
        CS_BOX_EDGE_xYz_xYZ,
        CS_BOX_EDGE_Xyz_XyZ,
        CS_BOX_EDGE_XYz_XYZ
      };
      for (int e = 0; e < numEdges; e++)
      {
        csSegment3 edge (box.GetEdge (edges[e]));
        AddDebugLine3D (toWorldSpace.Other2This (edge.Start()),
          toWorldSpace.Other2This (edge.End()),
          col, col);
      }
    }
    void RenderTreeBase::AddDebugPlane (const csPlane3& _plane,
                                        const csTransform& toWorldSpace,
                                        const csColor& col,
                                        const csVector3& linesOrg)
    {
      csPlane3 plane (_plane); plane.Normalize();
      csVector3 actualOrg = plane.ProjectOnto (linesOrg);
      const csVector3& p_n = plane.Normal();
      csVector3 p, q;
      {
        csVector3 someOtherPointOnPlane = actualOrg;
        int c = p_n.DominantAxis();
        someOtherPointOnPlane[(c+1)%3] += 1.0f;
        someOtherPointOnPlane[(c+2)%3] += 1.0f;
        someOtherPointOnPlane = plane.ProjectOnto (someOtherPointOnPlane);
        p = (someOtherPointOnPlane-actualOrg);
        p.Normalize();
        q = p_n % p;
        q.Normalize();
      }
      
      AddDebugLine3DTF (
        actualOrg + (p + q) * 0.5f,
        actualOrg + (p + q) * -0.5f,
        toWorldSpace, col, col);
      AddDebugLine3DTF (
        actualOrg + (p - q) * 0.5f,
        actualOrg + (p - q) * -0.5f,
        toWorldSpace, col, col);
      AddDebugLine3DTF (
        actualOrg, actualOrg + p_n,
        toWorldSpace, col*2, col*2);
    }
    
    void RenderTreeBase::AddDebugClipPlanes (RenderView* view)
    {
      csRenderContext* ctxt = view->GetCsRenderContext();
      if (ctxt == 0) return;
      
      iCamera* cam = view->GetCamera();
    
      csVector3 org (0, 0, 2);
      {
        const csTransform& camToWorld = cam->GetTransform().GetInverse();
	uint32 camMask;
	const csPlane3* frust = cam->GetVisibleVolume (camMask);
	uint i;
	for (i = 0; uint (1 << i) <= camMask; i++)
	{
	  if (!(camMask & (1 << i))) continue;
	  AddDebugPlane (frust[i], camToWorld, csColor (0.2f, 0.2f, 1),
	    org);
	}
      }
      {
        csTransform identity;
	uint32 camMask = ctxt->clip_planes_mask;
	const csPlane3* frust = ctxt->clip_planes;
	uint i;
	for (i = 0; uint (1 << i) <= camMask; i++)
	{
	  if (!(camMask & (1 << i))) continue;
	  AddDebugPlane (frust[i], identity, csColor (0.2f, 1, 0.2f),
	    cam->GetTransform().GetInverse().Other2This (org));
	}
	/*csPlane3 pznear = ctxt->clip_plane;
	pznear.Invert ();
	planes[i] = tr_o2c.This2Other (pznear);
	frustum_mask |= (1 << i);*/
      }
    }
    
    void RenderTreeBase::AddDebugLineScreen (const csVector2& v1, 
                                             const csVector2& v2,
                                             csRGBcolor color)
    {
      DebugLineScreen line;
      line.v1 = v1;
      line.v2 = v2;
      line.color = color;
      debugLinesScreen.Push (line);
    }
    

    void RenderTreeBase::DrawDebugLines (iGraphics3D* g3d, RenderView* view)
    {
      if (debugLines.verts.GetSize() > 0)
      {
	g3d->SetProjectionMatrix (view->GetCamera()->GetProjectionMatrix());
	g3d->SetClipper (0, CS_CLIPPER_TOPLEVEL);
	// [res] WTF - why does that not work, but in mesh.object2world it does?
	//g3d->SetWorldToCamera (view->GetCamera()->GetTransform().GetInverse());
	
	g3d->BeginDraw (CSDRAW_3DGRAPHICS);
	
	csSimpleRenderMesh mesh;
	mesh.alphaType.alphaType = csAlphaMode::alphaNone;
	mesh.meshtype = CS_MESHTYPE_LINES;
	mesh.vertexCount = (uint)debugLines.verts.GetSize();
	mesh.vertices = debugLines.verts.GetArray();
	mesh.colors = debugLines.colors.GetArray();
	mesh.object2world = view->GetCamera()->GetTransform().GetInverse();
	g3d->DrawSimpleMesh (mesh);
	g3d->FinishDraw ();
      }
      
      if (debugLinesScreen.GetSize() > 0)
      {
        iGraphics2D* g2d = g3d->GetDriver2D();
	g3d->BeginDraw (CSDRAW_2DGRAPHICS);
	for (size_t i = 0; i < debugLinesScreen.GetSize(); i++)
	{
	  const DebugLineScreen& line = debugLinesScreen[i];
	  int color = g2d->FindRGB (line.color.red, line.color.green,
	    line.color.blue);
	  g2d->DrawLine (line.v1.x, line.v1.y, line.v2.x, line.v2.y, color);
	}
	g3d->FinishDraw ();
      }
    }
  } // namespace RenderManager
} // namespace CS
