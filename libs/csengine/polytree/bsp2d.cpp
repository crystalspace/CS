/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
  
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
#include "csengine/bsp2d.h"

//---------------------------------------------------------------------------

csSegmentArray::~csSegmentArray ()
{
  DeleteAll ();
}

//---------------------------------------------------------------------------

csBspNode2D::csBspNode2D () : segments (10, 10)
{
  front = back = NULL;
}

csBspNode2D::~csBspNode2D ()
{
  delete front;
  delete back;
}

void csBspNode2D::AddSegment (csSegment2* seg)
{
  segments.Push (seg);
}

//---------------------------------------------------------------------------

csBspTree2D::csBspTree2D ()
{
  root = NULL;
}

csBspTree2D::~csBspTree2D ()
{
  delete root;
}

void csBspTree2D::Add (csBspNode2D* node, csSegment2* segment)
{
  float c1 = node->splitter.Classify (segment->Start ());
  float c2 = node->splitter.Classify (segment->End ());
  if (ABS (c1) < EPSILON) c1 = 0;
  if (ABS (c2) < EPSILON) c2 = 0;
  if (c1 == 0 && c2 == 0)
  {
    // Same plane.
    // Check if it is really the same plane or if the
    // direction is reversed.
    csPlane2 pl (*segment);
    if (csMath2::PlanesClose (pl, node->splitter))
      node->AddSegment (segment);
    else
    {
      // Not really the same plane. Send to the front node.
      if (!node->front)
      {
        node->front = new csBspNode2D ();
        node->front->splitter.Set (*segment);
        node->front->AddSegment (segment);
      }
      else
        Add (node->front, segment);
    }
  }
  else if (c1 >= 0 && c2 >= 0)
  {
    // Front.
    if (!node->front)
    {
      node->front = new csBspNode2D ();
      node->front->splitter.Set (*segment);
      node->front->AddSegment (segment);
    }
    else
      Add (node->front, segment);
  }
  else if (c1 <= 0 && c2 <= 0)
  {
    // Back.
    if (!node->back)
    {
      node->back = new csBspNode2D ();
      node->back->splitter.Set (*segment);
      node->back->AddSegment (segment);
    }
    else
      Add (node->back, segment);
  }
  else
  {
    // Split needed.
    csVector2 isect;
    float dist;
    csIntersect2::Plane (*segment, node->splitter, isect, dist);
    csSegment2* segf = new csSegment2 ();
    csSegment2* segb = new csSegment2 ();
    if (c1 < 0)
    {
      segb->Set (segment->Start (), isect);
      segf->Set (isect, segment->End ());
    }
    else
    {
      segf->Set (segment->Start (), isect);
      segb->Set (isect, segment->End ());
    }

    if (!node->front)
    {
      node->front = new csBspNode2D ();
      node->front->splitter.Set (*segf);
      node->front->AddSegment (segf);
    }
    else
      Add (node->front, segf);

    if (!node->back)
    {
      node->back = new csBspNode2D ();
      node->back->splitter.Set (*segb);
      node->back->AddSegment (segb);
    }
    else
      Add (node->back, segb);

    delete segment;
  }
}

void csBspTree2D::Add (csSegment2* segment)
{
  if (root)
    Add (root, segment);
  else
  {
    root = new csBspNode2D ();
    root->splitter.Set (*segment);
    root->AddSegment (segment);
  }
}

void* csBspTree2D::Back2Front (const csVector2& pos, csTree2DVisitFunc* func,
	void* data)
{
  return Back2Front (root, pos, func, data);
}

void* csBspTree2D::Front2Back (const csVector2& pos, csTree2DVisitFunc* func,
	void* data)
{
  return Front2Back (root, pos, func, data);
}

void* csBspTree2D::Back2Front (csBspNode2D* node, const csVector2& pos,
	csTree2DVisitFunc* func, void* data)
{
  if (!node) return NULL;
  void* rc;

//@@@@@@ THIS VISIBILITY TEST IS REVERSED (also in Front2Back).
//Check what the reason is for this.

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (!csMath2::Visible (pos, node->splitter))
  {
    // Front.
    rc = Back2Front (node->back, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Back2Front (node->front, pos, func, data);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Back2Front (node->front, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Back2Front (node->back, pos, func, data);
    if (rc) return rc;
  }
  return NULL;
}

void* csBspTree2D::Front2Back (csBspNode2D* node, const csVector2& pos,
	csTree2DVisitFunc* func, void* data)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (!csMath2::Visible (pos, node->splitter))
  {
    // Front.
    rc = Front2Back (node->front, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Front2Back (node->back, pos, func, data);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Front2Back (node->back, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Front2Back (node->front, pos, func, data);
    if (rc) return rc;
  }
  return NULL;
}

//---------------------------------------------------------------------------
