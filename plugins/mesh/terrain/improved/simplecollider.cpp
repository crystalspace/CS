/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#include "simplecollider.h"

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"

#include "iterrain/terraincell.h"

namespace
{
  int sign (float val)
  {
    return val > 0 ? 1 : val < 0 ? -1 : 0;
  }
};

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainSimpleCollider)

csTerrainSimpleCellCollisionProperties::csTerrainSimpleCellCollisionProperties
 (iBase* parent)
  : scfImplementationType (this, parent)
{
  collideable = true;
}

csTerrainSimpleCellCollisionProperties::~csTerrainSimpleCellCollisionProperties
()
{
}

bool csTerrainSimpleCellCollisionProperties::GetCollideable () const
{
  return collideable;
}

void csTerrainSimpleCellCollisionProperties::SetCollideable (bool value)
{
  collideable = value;
}

csTerrainSimpleCollider::csTerrainSimpleCollider (iBase* parent)
  : scfImplementationType (this, parent)
{
}


csTerrainSimpleCollider::~csTerrainSimpleCollider ()
{
}

csPtr<iTerrainCellCollisionProperties> csTerrainSimpleCollider::
CreateProperties ()
{
  return new csTerrainSimpleCellCollisionProperties(NULL);
}

bool csTerrainSimpleCollider::CollideSegment (iTerrainCell* cell,
const csVector3& start, const csVector3& end, bool oneHit,
iTerrainVector3Array& points)
{
  const csVector2& pos = cell->GetPosition ();
  const csVector3& size = cell->GetSize ();
  
  if (fabsf (start.x - end.x) < EPSILON && fabsf (start.z - end.z) < EPSILON)
  {
    float height = cell->GetHeight(csVector2(start.x, start.z) - cell->GetPosition());
    
    float min_height, max_height;
    
    if (start.y < end.y)
    {
      min_height = start.y;
      max_height = end.y;
    }
    else
    {
      max_height = start.y;
      min_height = end.y;
    }
    
    if (height >= min_height && height <= max_height)
    {
      points.Push (csVector3 (start.x, height, start.z));
      return true;
    }
  }
  else
  {
    float scale_u = size.x / (cell->GetGridWidth() - 1);
    float scale_v = size.y / (cell->GetGridHeight() - 1);
    
    // U, V and height of segment start in cell space
    float u0 = (start.x - pos.x) / scale_u;
    float v0 = (start.z - pos.y) / scale_v;
    float h0 = start.y;
    
    // U, V and height of segment end in cell space
    float u1 = (end.x - pos.x) / scale_u;
    float v1 = (end.z - pos.y) / scale_v;
    float h1 = end.y;
    
    // Constants
    const float rootOf2 = 1.414213f;
    const float halfRoot2 = rootOf2 / 2;
    
    // Compute differences for ray (lengths along axes) and their inverse
    float du = u1 - u0;
    float dv = v1 - v0;
    float dh = h1 - h0;
    
    float oneOverdu = 1 / du; // +INF if du = 0, it's ok (?)
    float oneOverdv = 1 / dv; // +INF if dv = 0, it's ok (?)
    
    // Distance to intersection with u/v axes
    float eu = u0 - floor(u0);
    float ev = v0 - floor(v0);
    
    // Differences and distance to intersection with diagonal
    float dp = (du + dv) / rootOf2;
    float oneOverdp = 1 / dp;
    float ep = abs(dp) * (1 - eu - ev) / (dv + du); // line-line intersection
    
    // Fixup for positive directions
    if (du > 0) eu = 1 - eu;
    if (dv > 0) ev = 1 - ev;
    if (ep < 0) ep += halfRoot2;
    
    // Stepping variables
    float t = 0;
    float h = h0;
    float cell_height = cell->GetHeight (csVector2 (start.x - pos.x,
                                                    start.z - pos.y));
    
    bool firsttime = true;
    
    static int hack = 2;
    
    if (points.IsEmpty ())
    {
      hack = (hack + 1) % 3;
      points.Push (csVector3(FLT_MAX, FLT_MAX, FLT_MAX));
    }
    
    // Trace along the line
    while (t < 1 - EPSILON)
    {
      float r_h0 = h;
      float tstep = 0;
      
      if (hack != 2)
      {
        csVector2 uv = csVector2 ((u0 + du * t) * scale_u,
                                  (v0 + dv * t) * scale_v); 
        float height = cell->GetHeight (uv);
        if (hack) height = h0 + dh * t;
      
        points.Push (csVector3 (pos.x + uv.x, height, pos.y + uv.y));
      }
      
      if (!firsttime)
      {
        float tToU = eu * abs(oneOverdu); // Time to reach U intersection
        float tToV = ev * abs(oneOverdv); // Time to reach V intersection
        float tToP = ep * abs(oneOverdp); // Time to reach P intersection
      
        if (tToU <= tToV && tToU <= tToP)
        {
          // U intersection first
          tstep = tToU;
          t += tToU;
          if (t > 1) t = 1 - EPSILON;
         
          // Update distances
          eu = 0;
          ev -= abs(dv) * tToU;
          ep -= abs(dp) * tToU;
         
          // Update height
          h += dh * tToU;
        }
        else if (tToV <= tToU && tToV <= tToP)
        {
          // V intersection first
          tstep = tToV;
          t += tToV;
          if (t > 1) t = 1 - EPSILON;
  
          // Update distances
          eu -= abs(du) * tToV;
          ev = 0;
          ep -= abs(dp) * tToV;
        
          // Update height
          h += dh * tToV;
        }
        else
        {
          // P intersection first
          tstep = tToP;
          t += tToP;
          if (t > 1) t = 1 - EPSILON;
          
          // Update distances
          eu -= abs(du) * tToP;
          ev -= abs(dv) * tToP;
          ep = 0;
        
          // Update height
          h += dh * tToP;
        }
        
        // Wrap around
        if (eu <= 0) eu = 1;
        if (ev <= 0) ev = 1;
        if (ep <= 0) ep = halfRoot2;
      }
      
      firsttime = false;
      
      // Check for intersection
      float r_h1 = h;
      float h_h0 = cell_height;
      
      csVector2 uv = csVector2 ((u0 + du * t) * scale_u,
                                (v0 + dv * t) * scale_v);
      float h_h1 = cell->GetHeight (uv);
      
      // Remember height value
      cell_height = h_h1;
      
      // Check intersection
      int cmp_h0 = sign (r_h0 - h_h0);
      int cmp_h1 = sign (r_h1 - h_h1);
      
      if (cmp_h0 * cmp_h1 == -1 || abs (r_h1 - h_h1) < EPSILON)
      {
        float correct_t = t;
        
        if (cmp_h0 != cmp_h1)
        {
          //    A                 B                   C
          // (vertex) ------(intersection)------- (vertex)
          // AC = tstep (in terms of t), we have to subtract
          // BC from correct_t. AB / BC equals the height ratio
          float coeff = abs(r_h0 - h_h0) / abs(r_h1 - h_h1);
          
          // AB / BC = coeff
          // AB + BC = tstep
          // AB = coeff * BC
          // BC * (coeff + 1) = tstep
          
          correct_t -= tstep / (coeff + 1);
        }
        
        if (hack == 2)
        {
          csVector2 uv = csVector2 ((u0 + du * correct_t) * scale_u,
                                    (v0 + dv * correct_t) * scale_v); 
          float height = h0 + dh * correct_t;
      
          points.Push (csVector3 (pos.x + uv.x, height, pos.y + uv.y));
        }
      }
    }

    return true;
  }
  
  return false;
}

bool csTerrainSimpleCollider::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}

void csTerrainSimpleCollider::OnHeightUpdate (iTerrainCell* cell, const csRect&
  rectangle, const float* data, unsigned int pitch)
{
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
