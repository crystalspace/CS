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

#include "sysdef.h"
#include "qint.h"
#include "csengine/tranman.h"

//---------------------------------------------------------------------------

csVertexArray::csVertexArray ()
{
  max_vertices = 10;
  CHK (vertices = new csVector3 [max_vertices]);
  num_vertices = 0;
  cookie = 0;
}

csVertexArray::~csVertexArray ()
{
  CHK (delete [] vertices);
}

void csVertexArray::SetSize (int s)
{
  num_vertices = s;
  if (num_vertices > max_vertices)
  {
    max_vertices = num_vertices;
    CHK (delete [] vertices);
    CHK (vertices = new csVector3 [max_vertices]);
  }
}

//---------------------------------------------------------------------------

csTransformationManager::csTransformationManager ()
{
  freed = NULL;
  alloced = NULL;
  last_alloced = NULL;
  cookie = 0;
  max_cookie = 0;
  frame_cookie = 0;
}

csTransformationManager::~csTransformationManager ()
{
  while (freed)
  {
    csVertexArray* n = freed->next;
    CHK (delete freed);
    freed = n;
  }
  while (alloced)
  {
    csVertexArray* n = alloced->next;
    CHK (delete alloced);
    alloced = n;
  }
}

void csTransformationManager::NewFrame ()
{
  // Append the free list to the alloced list and put
  // both in freed.
  if (last_alloced)
  {
    last_alloced->next = freed;
    freed = last_alloced;
    alloced = last_alloced = NULL;
  }

  if (max_cookie > 4000000000U)
  {
    // If we are this high we reset the cookie counter to 0.
    // We will also traverse all allocated and freed vertex arrays and
    // reset their cookies to 0. That is to make sure that none of
    // those arrays are considered valid.
    // Cookie number 0 is never a valid cookie number.
    max_cookie = 0;
    csVertexArray* ar = alloced;
    while (ar)
    {
      ar->cookie = 0;
      ar = ar->next;
    }
    ar = freed;
    while (ar)
    {
      ar->cookie = 0;
      ar = ar->next;
    }
  }

  max_cookie++;
  cookie = max_cookie;
  frame_cookie = cookie;
}

csTranCookie csTransformationManager::NewCameraFrame ()
{
  csTranCookie old_cookie = cookie;
  max_cookie++;
  cookie = max_cookie;
  return old_cookie;
}

void csTransformationManager::RestoreCameraFrame (csTranCookie prev_cookie)
{
  cookie = prev_cookie;
}

//---------------------------------------------------------------------------

csTransformedSet::csTransformedSet ()
{
  tr_manager = NULL;
  vertex_arrays = NULL;
  current_array = NULL;
  last_cookie = 0;
}

csTransformedSet::csTransformedSet (csTransformationManager* manager)
{
  tr_manager = manager;
  vertex_arrays = NULL;
  current_array = NULL;
  last_cookie = 0;
}

csTransformedSet::~csTransformedSet ()
{
}

void csTransformedSet::Update ()
{
  if (tr_manager->HasFrameChanged (last_cookie))
  {
    // The frame has changed. Our current array list is invalid.
    vertex_arrays = NULL;
  }
  // Look for a vertex array which has the current cookie.
  last_cookie = tr_manager->GetCookie ();
  csVertexArray* ar = vertex_arrays;
  while (ar)
  {
    if (ar->cookie == last_cookie) break;
    ar = ar->next_set;
  }
  if (!ar)
  {
    // We didn't find one so we have to ask a new array to the transformation
    // manager.
    ar = tr_manager->Alloc ();
    ar->next_set = vertex_arrays;
    ar->cookie = last_cookie;
    vertex_arrays = ar;
  }
  current_array = ar;
}


void csTransformedSet::Transform (csVector3* wor_verts, int num_vertices,
	const csTransform& w2c)
{
  Update ();
  int i;
  current_array->SetSize (num_vertices);
  csVector3* cam_verts = current_array->GetVertices ();

#if 1
  for (i = 0 ; i < num_vertices ; i++)
    cam_verts[i] = w2c.Other2This (wor_verts[i]);
#else
  // This loop has been made explicit because this makes better
  // usage of the cache.
  // @@@ It's an experiment. We need to do more testing here.
  int i;
 
  float dx, dy, dz;
  float cx, cy, cz;
 
  const csMatrix3& m_o2t = w2c.GetO2T ();
  const csVector3& v_o2t = w2c.GetO2TTranslation ();

  float m11 = m_o2t.m11;
  float m21 = m_o2t.m21;
  float m31 = m_o2t.m31;
 
  float m12 = m_o2t.m12;
  float m22 = m_o2t.m22;
  float m32 = m_o2t.m32;
  
  float m13 = m_o2t.m13;
  float m23 = m_o2t.m23;
  float m33 = m_o2t.m33;
 
  cx = v_o2t.x;
  cy = v_o2t.y;
  cz = v_o2t.z;
 
  csVector3* world_verts = wor_verts;
  csVector3* camra_verts = cam_verts;
  for (i = 0 ; i < num_vertices ; i++)
  {        
    dx = world_verts->x - cx;
    dy = world_verts->y - cy;
    dz = world_verts->z - cz; 
 
    camra_verts->x  = m11 * dx + m12 * dy + m13 *dz;
    camra_verts->y  = m21 * dx + m22 * dy + m23 *dz;
    camra_verts->z  = m31 * dx + m32 * dy + m33 *dz;

    world_verts++;
    camra_verts++;
  }
#endif
}

void csTransformedSet::Translate (csVector3* wor_verts, int num_vertices,
	const csVector3& trans)
{
  Update ();
  int i;
  current_array->SetSize (num_vertices);
  csVector3* cam_verts = current_array->GetVertices ();

  for (i = 0 ; i < num_vertices ; i++)
    cam_verts[i] = wor_verts[i]-trans;
}
 
//---------------------------------------------------------------------------

