/*
    Copyright (C) 2002 by Jorrit Tyberghein and Ryan Surkamp

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
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/vbufmgr.h"
#include "iengine/material.h"
#include "iengine/rview.h"
#include "ivideo/txtmgr.h"
#include "igraphic/image.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "imap/ldrctxt.h"
#include "csgfx/rgbpixel.h"
#include "bcterr.h"
#include "quadtree.h"
#include "qint.h"
#include "qsqrt.h"
#include "ivaria/reporter.h"
#include "quadtree.h"

static bool Inside (csBox3 bbox, csVector3 *point)
{
  if ( ( point->x <= bbox.MaxX () ) && ( point->x >= bbox.MinX () ) 
    && ( point->z <= bbox.MaxZ () ) && ( point->z >= bbox.MinZ () )
    && ( point->y <= bbox.MaxY () ) )
    return true;
  else
    return false;
}

static bool InsideExt (csBox3 bbox, csVector3 *point)
{
  if ( ( point->x <= bbox.MaxX () ) && ( point->x >= bbox.MinX () ) 
    && ( point->z <= bbox.MaxZ () ) && ( point->z >= bbox.MinZ () ) )
    return true;
  else
    return false;
}


void csColQuad::HeightTest (csVector3  *point, 
		int &hits)
{
  if ( Inside (bbox, point) )
  {
    int i;
    for (i = 0; i < num_blocks; i++)
    {
      if ( Inside (blocks[i]->bbox, point) )
      {
        if ( point->y < (blocks[i]->bbox.MaxY () + 0.2f) )
        {
          point->y = blocks[i]->bbox.MaxY () + 0.2f;
          hits += 1;
        }
      }			
    }
    if (children[0])
    {
      for (i = 0; i < 4; i++)
        children[i]->HeightTest (point, hits);
    }
  }	
}



void csColQuad::HeightTestExact (csVector3  *point, 
    int &hits)
{
  if ( Inside (bbox, point) )
  {
    int i;
    for (i = 0; i < num_blocks; i++)
    {
      if ( Inside (blocks[i]->bbox, point) )
      {
        if (point->y <= blocks[i]->bbox.MaxY ())
        {
          csVector3 newpoint;
          float u, v;
          int width;
          csVector3 temp[4];
          width = blocks[i]->owner->hor_length;          
  u = ( point->x - blocks[i]->bbox.MinX () )
    / (blocks[i]->bbox.MaxX () - blocks[i]->bbox.MinX ());
  v = (blocks[i]->bbox.MaxZ () - point->z)
    / (blocks[i]->bbox.MaxZ () - blocks[i]->bbox.MinZ ());
          if (u < 0.0f) u = -u;
          if (v < 0.0f) v = -v;
          if (u > 1.0f) u = 1.0f;
          if (v > 1.0f) v = 1.0f;
          temp[0] = BezierControlCompute (v, blocks[i]->controlpoint, width);
          temp[1] = BezierControlCompute (v, &blocks[i]->controlpoint[1], width);
          temp[2] = BezierControlCompute (v, &blocks[i]->controlpoint[2], width);
          temp[3] = BezierControlCompute (v, &blocks[i]->controlpoint[3], width);
          newpoint =  BezierCompute (u, temp);
          if ( (newpoint.y > point->y) || 
               (point->y < (newpoint.y + 2.0f)))
          {
            point->y = newpoint.y + 2.0f;
            hits += 1;
          }
          //point->y = newpoint.y + 10.0f;
        }
      }			
    }
    if (children[0])
    {
      for (i = 0; i < 4; i++)
        children[i]->HeightTestExact (point, hits);
    }
  }
}

void csColQuad::HeightTestExt (csVector3  *point, 
    int &hits)
{
  if ( InsideExt (bbox, point) )
  {
    int i;
    for (i = 0; i < num_blocks; i++)
    {
      if ( InsideExt (blocks[i]->bbox, point) )
      {
          csVector3 newpoint;
          float u, v;
          csVector3 temp[4];
          int width;
          width = blocks[i]->owner->hor_length;
  u = ( point->x - blocks[i]->bbox.MinX () )
    / (blocks[i]->bbox.MaxX () - blocks[i]->bbox.MinX ());
  v = (blocks[i]->bbox.MaxZ () - point->z)
    / (blocks[i]->bbox.MaxZ () - blocks[i]->bbox.MinZ ());
          if (u < 0.0f) u = -u;
          if (v < 0.0f) v = -v;
          if (u > 1.0f) u = 1.0f;
          if (v > 1.0f) v = 1.0f;
          temp[0] = BezierControlCompute (v, blocks[i]->controlpoint, width);
          temp[1] = BezierControlCompute (v, &blocks[i]->controlpoint[1], width);
          temp[2] = BezierControlCompute (v, &blocks[i]->controlpoint[2], width);
          temp[3] = BezierControlCompute (v, &blocks[i]->controlpoint[3], width);
          newpoint =  BezierCompute (u, temp);
          newpoint.y += 2.0f;
          if ( newpoint.y > point->y)
          {
            point->y = newpoint.y;
            hits += 1;
          }
          /*v = u - 0.1f;
          if ( v >= 0.0)
          {
            newpoint = BezierCompute (v, temp);
            newpoint.y += 2.0f;
            if (newpoint.y > point->y)
              point->y = newpoint.y;
          }
          v = u + 0.1f;
          if (v <= 1.0)
          {
            newpoint = BezierCompute (v, temp);
            newpoint.y += 2.0f;
            if (newpoint.y > point->y)
              point->y = newpoint.y;
          }*/
          //point->y = newpoint.y + 10.0f;        
      }			
    }
    if (children[0])
    {
      for (i = 0; i < 4; i++)
        children[i]->HeightTestExact (point, hits);
    }
  }
}


void csColQuad::SetupChildren (float shortest, iObjectRegistry* object_reg)
{
  csBox3 nbbox;
  csVector3 pt;
  float half_z, half_x;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCColQuad", "SetupChildren");
  half_z = (bbox.MaxZ () - bbox.MinZ ()) / 2.0f;
  half_x = (bbox.MaxX () - bbox.MinX ()) / 2.0f;
  // left
  nbbox.StartBoundingBox ();
  pt.x = bbox.MinX ();
  pt.y = bbox.MaxY ();
  pt.z = bbox.MaxZ ();
  nbbox.AddBoundingVertex (pt);
  pt.x = bbox.MinX () + half_x;
  pt.y = bbox.MinY ();
  pt.z = bbox.MinZ () + half_z;
  nbbox.AddBoundingVertex (pt);
  children[0] = new csColQuad (shortest, nbbox, object_reg);
  // right
  nbbox.StartBoundingBox ();
  nbbox.AddBoundingVertex ( bbox.Max ());
  pt.x = bbox.MinX () + half_x;
  pt.y = bbox.MinY ();
  pt.z = bbox.MinZ () + half_z;
  nbbox.AddBoundingVertex (pt);
  children[1] = new csColQuad (shortest, nbbox, object_reg);
  // down left
  nbbox.StartBoundingBox ();
  pt.x = bbox.MinX ();
  pt.y = bbox.MaxY ();
  pt.z = bbox.MinZ ();
  nbbox.AddBoundingVertex (pt);
  pt.x = bbox.MinX () + half_x;
  pt.y = bbox.MinY ();
  pt.z = bbox.MinZ () + half_z;
  nbbox.AddBoundingVertex (pt);
  children[2] = new csColQuad (shortest, nbbox, object_reg);
  // down right
  nbbox.StartBoundingBox ();
  pt.x = bbox.MaxX ();
  pt.y = bbox.MaxY ();
  pt.z = bbox.MinZ ();
  nbbox.AddBoundingVertex (pt);
  pt.x = bbox.MinX () + half_x;
  pt.y = bbox.MinY ();
  pt.z = bbox.MinZ () + half_z;
  nbbox.AddBoundingVertex (pt);
  children[3] = new csColQuad (shortest, nbbox, object_reg);
}

csColQuad::csColQuad (csVector3 *cntrl_pt, int x_blocks, int z_blocks,
					  float shortest, iObjectRegistry* object_reg)
{
  int x, z, size, i;
  float half_x, half_z;
  num_blocks = 0;
  blocks = NULL;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCColQuad", "Big Create");
  x = (x_blocks * 3) + 1;
  z = (z_blocks * 3) + 1;
  size = x * z;
  bbox.StartBoundingBox ();
  for (i = 0; i < size; i++)
  {
    bbox.AddBoundingVertex (cntrl_pt[i]);
  }
  half_z = (bbox.MaxZ () - bbox.MinZ ()) / 2.0f;
  half_x = (bbox.MaxX () - bbox.MinX ()) / 2.0f;
  for (i = 0; i < 4; i++)
  {
    children[i] = NULL;
  }
  if ( (half_z < shortest) && (half_x < shortest) )
    return;	
  if ( shortest < 0.5f) return;
  SetupChildren (shortest, object_reg);
}

csColQuad::csColQuad (float shortest, csBox3 nbbox, iObjectRegistry* object_reg)
{
  float half_x, half_z;
  int i;
  num_blocks = 0;
  blocks = NULL;
  bbox = nbbox;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCColQuad", "Small Create");
  half_z = (bbox.MaxZ () - bbox.MinZ ()) / 2.0f;
  half_x = (bbox.MaxX () - bbox.MinX ()) / 2.0f;
  for (i = 0; i < 4; i++)
  {
    children[i] = NULL;
  }
  if ( (half_z < shortest) && (half_x < shortest) )
    return;
  SetupChildren (shortest, object_reg);
}

void csColQuad::AddBlock (csBCTerrBlock *block)
{
  bool left, right, bleft, bright, multi, done;
  if (children[0])
  {
    multi = false;
    done = false;
    left = CheckBox (children[0]->bbox, block);
    right = CheckBox (children[1]->bbox, block);
    if (right && left)
      multi = true;

    bleft = CheckBox (children[2]->bbox, block);			
    if ( (bleft && right) || (bleft && left) )
      multi = true;

    bright = CheckBox (children[3]->bbox, block);			
    if ( (bright && bleft) || (bright && right) ||
       (bright && left) )
       multi = true;

    if (multi)
    {
      AddBlockToList (block);
      done = true;
    }
    else
    {
      if (left)
      {
        children[0]->AddBlock (block);
        done = true;
      }
      if (right)
      {
        children[1]->AddBlock (block);
        done = true;
      }
      if (bleft)
      {
        children[2]->AddBlock (block);
        done = true;
      }
      if (bright)
      {
        children[3]->AddBlock (block);
        done = true;
      }
      if (done == false)
        AddBlockToList (block);
    }
  } else
    AddBlockToList (block);
}

void csColQuad::AddBlockToList (csBCTerrBlock *block)
{
  int i;
  if (num_blocks > 0)
  {
    csBCTerrBlock **new_blocks = new csBCTerrBlock*[num_blocks + 1];
    for (i = 0; i < num_blocks; i++)
    {
      new_blocks[i] = blocks[i];
      blocks[i] = NULL;
    }
    delete [] blocks;
    new_blocks[num_blocks] = block;
    blocks = new_blocks;
    num_blocks++;
  } else 
  {
    num_blocks++;
    blocks = new csBCTerrBlock*[num_blocks];
    blocks[0] = block;
  }
}

void csColQuad::RebuildBoundingBoxes ()
{
  int i;
  bbox.StartBoundingBox ();
  if (children[0])
  {
    for (i = 0; i < 4; i++)
    {
      children[i]->RebuildBoundingBoxes ();
      bbox += children[i]->bbox;
    }
  }
  if (num_blocks > 0)
  {
    for (i = 0; i < num_blocks; i++)
    {
      bbox += blocks[i]->bbox;
    }
  }
}

bool csColQuad::CheckBox (csBox3 check, csBCTerrBlock *block)
{
  if ( check.In  (block->bbox.GetCenter ()))
  {
    return true;
  } else
  {
    if ( check.In (block->bbox.Max ()) ) return true;
    if ( check.In (block->bbox.Min ()) ) return true;
  }
  return false;
}

csColQuad::~csColQuad ()
{
  int i;
  if (children[0])
  {
    for (i = 0; i < 4; i++)
    {
      delete children[i];
    }
  }
  if ((num_blocks > 0) && (blocks))
  {
    for (i = 0; i < num_blocks; i++)
    {
      blocks[i] = NULL;
    }
    delete [] blocks;
  }
}

csBCCollisionQuad::csBCCollisionQuad ()
{
  root_quad = NULL;
  object_reg = NULL;
}

csBCCollisionQuad::~csBCCollisionQuad ()
{
  if (root_quad) delete root_quad;
}

csBCCollisionQuad::csBCCollisionQuad (csVector3 *cntrl_pt,
									  int x_blocks, int z_blocks,
									  float shortest, iObjectRegistry* nobject_reg)
{
  root_quad = NULL;
  object_reg = nobject_reg;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Building Quads");
  root_quad = new csColQuad (cntrl_pt, x_blocks, z_blocks, shortest, object_reg);
}

void csBCCollisionQuad::AddBlock (csBCTerrBlock *block)
{
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Adding blocks");
  root_quad->AddBlock (block);
}

void csBCCollisionQuad::RebuildBoundingBoxes ()
{
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Rebuilding blocks");
  root_quad->RebuildBoundingBoxes ();
}

int csBCCollisionQuad::HeightTest (csVector3 *point)
{
  int hits;
  hits = 0;
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Height Test Called");
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Point %f %f %f", point->x, point->y, point->z);
  //point->y = root_quad->bbox.MaxY ();
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","new point %f %f %f", point->x, point->y, point->z);
  //csVector3 max = root_quad->bbox.Max ();
  //csVector3 min = root_quad->bbox.Min (); 
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Max Point %f %f %f", max.x, max.y, max.z);
  //csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,"csBCCollisionQuad","Max Point %f %f %f", min.x, min.y, min.z);
  root_quad->HeightTest (point, hits);
  return hits;
}


int csBCCollisionQuad::HeightTestExt (csVector3 *point)
{
  int hits;
  hits = 0;
  root_quad->HeightTestExt (point, hits);
  return hits;
}

int csBCCollisionQuad::HeightTestExact (csVector3 *point)
{
  int hits;
  hits = 0;
  root_quad->HeightTestExact (point, hits);
  return hits;
}
