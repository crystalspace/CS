/*
    Copyright (C) 1998-2002 by Jorrit Tyberghein

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
#include "cssys/csendian.h"
#include "qint.h"
#include "csengine/thing.h"
#include "csengine/polygon.h"
#include "csengine/polytmap.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/lppool.h"
#include "csgeom/polypool.h"
#include "iengine/light.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "csengine/curve.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "csgeom/sphere.h"
#include "csgeom/math3d.h"
#include "csutil/csstring.h"
#include "csutil/memfile.h"
#include "csutil/hashmap.h"
#include "csutil/debug.h"
#include "csutil/csmd5.h"
#include "ivideo/txtmgr.h"
#include "ivideo/vbufmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iengine/shadcast.h"
#include "csutil/hashmap.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/cache.h"
#include "iengine/rview.h"
#include "iengine/fview.h"
#include "qint.h"
#include "qsqrt.h"
#include "ivideo/graph3d.h"
#include "ivaria/reporter.h"


//---------------------------------------------------------------------------

static void Warn (iObjectRegistry* object_reg, const char *description, ...)
{
  va_list arg;
  va_start (arg, description);

  csRef<iReporter> Reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  if (Reporter)
  {
    Reporter->ReportV (
        CS_REPORTER_SEVERITY_WARNING,
        "crystalspace.engine.warning",
        description,
        arg);
  }
  else
  {
    csPrintfV (description, arg);
    csPrintf ("\n");
  }

  va_end (arg);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csThing)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iThingState)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iLightingInfo)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iObjectModel)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPolygonMesh)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iShadowCaster)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iShadowReceiver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMeshObject)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iMeshObjectFactory)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ThingState)
  SCF_IMPLEMENTS_INTERFACE(iThingState)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::LightingInfo)
  SCF_IMPLEMENTS_INTERFACE(iLightingInfo)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ObjectModel)
  SCF_IMPLEMENTS_INTERFACE(iObjectModel)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::PolyMesh)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ShadowCaster)
  SCF_IMPLEMENTS_INTERFACE(iShadowCaster)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::ShadowReceiver)
  SCF_IMPLEMENTS_INTERFACE(iShadowReceiver)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::MeshObject)
  SCF_IMPLEMENTS_INTERFACE(iMeshObject)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThing::MeshObjectFactory)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectFactory)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

int csThing:: last_thing_id = 0;

csThing::csThing (iBase *parent, csThingObjectType* thing_type) :
  csObject(parent),
  polygons(64, 64),
  curves(16, 16)
{
  csThing::thing_type = thing_type;

  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingState);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightingInfo);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiObjectModel);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygonMesh);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowCaster);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiShadowReceiver);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshObject);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiMeshObjectFactory);
  DG_TYPE (this, "csThing");

  scfiPolygonMesh.SetThing (this);
  scfiPolygonMeshLOD.SetThing (this);

  last_thing_id++;
  thing_id = last_thing_id;
  last_polygon_id = 0;
  logparent = NULL;

  curves_center.x = curves_center.y = curves_center.z = 0;
  curves_scale = 40;
  curve_vertices = NULL;
  curve_texels = NULL;
  num_curve_vertices = max_curve_vertices = 0;

  max_vertices = num_vertices = 0;
  wor_verts = NULL;
  obj_verts = NULL;
  cam_verts = NULL;
  num_cam_verts = 0;

  draw_busy = 0;
#ifndef CS_USE_NEW_RENDERER
  fog.enabled = false;
#endif // CS_USE_NEW_RENDERER
  bbox = NULL;
  obj_bbox_valid = false;

  dynamic_ambient.Set (0,0,0);
  ambient_version = 0;

  center_idx = -1;
  ParentTemplate = NULL;

  cameranr = -1;
  movablenr = -1;
  shapenr = -1;
  wor_bbox_movablenr = -1;
  cached_movable = NULL;

  cfg_moving = CS_THING_MOVE_NEVER;

  prepared = false;

  current_lod = 1;
  current_features = 0;
  thing_edges_valid = false;

  curves_transf_ok = false;

#ifndef CS_USE_NEW_RENDERER
  polybuf = NULL;
#endif // CS_USE_NEW_RENDERER
  polybuf_materials = NULL;

  obj_normals = NULL;
  smoothed = false;
  current_visnr = 1;
}

csThing::~csThing ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf) polybuf->DecRef ();
#endif // CS_USE_NEW_RENDERER
  delete[] polybuf_materials;

  if (wor_verts == obj_verts)
    delete[] obj_verts;
  else
  {
    delete[] wor_verts;
    delete[] obj_verts;
  }

  delete[] cam_verts;
  delete[] curve_vertices;
  delete[] curve_texels;
  delete bbox;

  polygons.DeleteAll ();          // delete prior to portal_poly array !
  if (portal_polygons.Length ()) portal_polygons.DeleteAll ();
  CleanupThingEdgeTable ();

  delete [] obj_normals;
}

char* csThing::GenerateCacheName ()
{
  csBox3 b;
  GetBoundingBox (b);

  csMemFile mf;
  long l;
  l = convert_endian ((long)num_vertices);
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)polygons.Length ());
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)num_curve_vertices);
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)curves.Length ());
  mf.Write ((char*)&l, 4);

  if (logparent)
  {
    csRef<iMeshWrapper> mw (SCF_QUERY_INTERFACE (logparent, iMeshWrapper));
    if (mw)
    {
      if (mw->QueryObject ()->GetName ())
        mf.Write (mw->QueryObject ()->GetName (),
		strlen (mw->QueryObject ()->GetName ()));
      iSector* sect = mw->GetMovable ()->GetSectors ()->Get (0);
      if (sect && sect->QueryObject ()->GetName ())
        mf.Write (sect->QueryObject ()->GetName (),
		strlen (sect->QueryObject ()->GetName ()));
    }
  }

  l = convert_endian ((long)QInt ((b.MinX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MinY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MinZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MaxX () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MaxY () * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((long)QInt ((b.MaxZ () * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());

  char* cachename = new char[33];
  sprintf (cachename,
  	"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
  	digest.data[0], digest.data[1], digest.data[2], digest.data[3],
  	digest.data[4], digest.data[5], digest.data[6], digest.data[7],
  	digest.data[8], digest.data[9], digest.data[10], digest.data[11],
  	digest.data[12], digest.data[13], digest.data[14], digest.data[15]);
  return cachename;
}

void csThing::MarkLightmapsDirty ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf)
    polybuf->MarkLightmapsDirty ();
#endif // CS_USE_NEW_RENDERER
}

void csThing::CleanupThingEdgeTable ()
{
  int i;
  for (i = 0; i < thing_edges.Length (); i++)
  {
    csThingEdge &te = thing_edges[i];
    delete[] te.polygon_indices;
  }

  thing_edges.SetLength (0);
  thing_edges_valid = false;
}

void csThing::ComputeThingEdgeTable ()
{
  if (thing_edges_valid) return ;
  CleanupThingEdgeTable ();

  //@@@
  thing_edges_valid = true;
}

void csThing::SetMovingOption (int opt)
{
  cfg_moving = opt;
  curves_transf_ok = false;
  switch (cfg_moving)
  {
    case CS_THING_MOVE_NEVER:
      if (wor_verts != obj_verts) delete[] wor_verts;
      wor_verts = obj_verts;
      break;

    case CS_THING_MOVE_OCCASIONAL:
      if ((wor_verts == NULL || wor_verts == obj_verts) && max_vertices)
      {
        wor_verts = new csVector3[max_vertices];
        memcpy (wor_verts, obj_verts, max_vertices * sizeof (csVector3));
      }

      cached_movable = NULL;
      break;

    case CS_THING_MOVE_OFTEN:
      if (wor_verts != obj_verts) delete[] wor_verts;
      wor_verts = obj_verts;
      break;
  }

  movablenr = -1;                 // @@@ Is this good?
}

void csThing::WorUpdate ()
{
  int i;
  switch (cfg_moving)
  {
    case CS_THING_MOVE_NEVER:
      UpdateCurveTransform ();
      return ;

    case CS_THING_MOVE_OCCASIONAL:
      if (cached_movable && cached_movable->GetUpdateNumber () != movablenr)
      {
        movablenr = cached_movable->GetUpdateNumber ();

	if (cached_movable->IsFullTransformIdentity ())
	{
	  memcpy (wor_verts, obj_verts, num_vertices * (sizeof (csVector3)));
	  csReversibleTransform movtrans;	// Identity.
	  // @@@ It is possible to optimize the below too. Don't know
	  // if it is worth it though.
          for (i = 0; i < polygons.Length (); i++)
          {
            csPolygon3D *p = GetPolygon3D (i);
            p->ObjectToWorld (movtrans, p->Vwor (0));
          }

          UpdateCurveTransform (movtrans);
	}
	else
	{
          csReversibleTransform movtrans = cached_movable->GetFullTransform ();
          for (i = 0; i < num_vertices; i++)
            wor_verts[i] = movtrans.This2Other (obj_verts[i]);
          for (i = 0; i < polygons.Length (); i++)
          {
            csPolygon3D *p = GetPolygon3D (i);
            p->ObjectToWorld (movtrans, p->Vwor (0));
          }

          UpdateCurveTransform (movtrans);
	}

        // If the movable changed we invalidate the camera number as well
        // to make sure the camera vertices are recalculated as well.
        cameranr--;
      }
      break;

    case CS_THING_MOVE_OFTEN:
      //@@@ Not implemented yet!
      return ;
  }
}

void csThing::UpdateTransformation (const csTransform &c, long cam_cameranr)
{
  if (!cam_verts || num_vertices != num_cam_verts)
  {
    delete[] cam_verts;
    cam_verts = new csVector3[num_vertices];
    num_cam_verts = num_vertices;
    cameranr = cam_cameranr - 1;  // To make sure we will transform.
  }

  if (cameranr != cam_cameranr)
  {
    cameranr = cam_cameranr;

    int i;
    for (i = 0; i < num_vertices; i++)
      cam_verts[i] = c.Other2This (wor_verts[i]);
  }
}


void csThing::SetSmoothingFlag(bool smooth)
{
  smoothed = smooth;
}

void csThing::CalculateNormals()
{
  int polyCount = polygons.Length();
  int i, j, k;
  int* vertIndices;

  delete[] obj_normals;
  obj_normals = new csVector3[num_vertices];
	csVector3** normals = new csVector3*[num_vertices];

  for(i = 0; i < num_vertices; i++)
  {
    normals[i] = new csVector3[polyCount];
    obj_normals[i].x = obj_normals[i].y = obj_normals[i].z = 0.0;
    for (j = 0; j< polyCount; j++)
      normals[i][j] = obj_normals[i];
  }


  // Get all the normals of a vertex affected by all the polygons
  for(j = 0; j < polyCount; j++)
  {
    csPolygon3D* p = polygons.Get(j);
    vertIndices = p->GetVertexIndices();
    csVector3 normal = p->GetPlane()->GetWorldPlane().Normal();

    // Add the normal to all the vertexs of the polygon
    int vertCount = p->GetVertexCount();
    for(k = 0; k < vertCount; k++)
    {
      normals[vertIndices[k]][j] = normal;
    }
  }

  // Remove the repeated normals
  float absolut;
  for (i = 0; i < num_vertices; i++)
  {
    for(j = 0; j < polyCount-1; j++)
    {
      csVector3 normalAct = normals[i][j];
      // Search in the rest of the normals
      absolut = fabs(normalAct.x) + fabs(normalAct.y) + fabs(normalAct.z);
      if ( absolut > 0.5  )
      {
	for (k=j+1; k< polyCount; k++)
	{
	  // If the normals are the same, remove the new one
	  csVector3 normalZero = normalAct - normals[i][k];
	  absolut = fabs(normalZero.x) + fabs(normalZero.y)
	  	+ fabs(normalZero.z);
	  if (absolut < 0.1)
	  {
	    normals[i][k].x = normals[i][k].y = normals[i][k].z = 0.0;
	  }
	}
      }
    }
  }


  for(i = 0; i < num_vertices; i++)
  {
    for(j = 0; j < polyCount; j++)
    {
      obj_normals[i] += normals[i][j];
    }
    obj_normals[i].Normalize();

    csVector3 v = Vwor(i);
    csVector3 normalPerfecta (-v.x, -v.y+20.0, -v.z);
    normalPerfecta.Normalize ();
  }

  for(i = 0; i < num_vertices; i++)
    delete [] normals[i];
  delete [] normals;
}

void csThing::Prepare ()
{
  if (prepared) return ;
  prepared = true;
  shapenr++;
  scfiPolygonMeshLOD.Cleanup ();
  scfiPolygonMesh.Cleanup ();
  if (!flags.Check (CS_THING_NOCOMPRESS))
  {
    CompressVertices ();
    RemoveUnusedVertices ();
  }

  if (smoothed)
    CalculateNormals();

  int i;
  csPolygon3D *p;
  for (i = 0; i < polygons.Length (); i++)
  {
    p = polygons.Get (i);
    p->Finish ();
  }
  FireListeners ();


  {
static int total_size = 0;
static int total_cnt = 0;
    int size = sizeof (csThing);
    int vtsize = sizeof (csVector3);
    size += vtsize * max_vertices;	// obj_verts;
    size += vtsize * num_cam_verts;	// cam_verts;
    if (obj_verts != wor_verts)
      size += vtsize * max_vertices;
    if (obj_normals)
      size += vtsize * num_vertices;
    size += sizeof (csThingBBox);
    size += 4 * polygons.Limit ();
    size += GetName () ? strlen (GetName ())+3 : 0;
    int polsize = sizeof (csPolygon3D) + sizeof (csPolyTexture)
    	+ sizeof (csPolyTexLightMap) + sizeof (csPolyTxtPlane)
	+ sizeof (csPolyPlane) + sizeof (csLightMap);
    polsize *= polygons.Length ();
    int i;
    for (i = 0 ; i < polygons.Length () ; i++)
    {
      csPolygon3D *p = polygons.Get (i);
      polsize += 4*4;	// Average indexes.
      csPolyTexLightMap *lmi = p->GetLightMapInfo ();
      if (lmi)
      {
        csLightMap* lm = lmi->GetPolyTex ()->GetCSLightMap ();
	if (lm)
	  polsize += lm->GetSize () * sizeof (csRGBpixel) * 2;
      }
    }

    total_size += size+polsize;
    total_cnt++;
    //printf ("thing size=%d polysize=%d total=%d total_size=%d average_size=%d count=%d\n", size, polsize, size + polsize, total_size, total_size / total_cnt, total_cnt);
  }

}

int csThing::AddCurveVertex (const csVector3 &v, const csVector2 &t)
{
  if (!curve_vertices)
  {
    max_curve_vertices = 10;
    curve_vertices = new csVector3[max_curve_vertices];
    curve_texels = new csVector2[max_curve_vertices];
  }

  while (num_curve_vertices >= max_curve_vertices)
  {
    max_curve_vertices += 10;

    csVector3 *new_vertices = new csVector3[max_curve_vertices];
    csVector2 *new_texels = new csVector2[max_curve_vertices];
    memcpy (
      new_vertices,
      curve_vertices,
      sizeof (csVector3) * num_curve_vertices);
    memcpy (
      new_texels,
      curve_texels,
      sizeof (csVector2) * num_curve_vertices);
    delete[] curve_vertices;
    delete[] curve_texels;
    curve_vertices = new_vertices;
    curve_texels = new_texels;
  }

  curve_vertices[num_curve_vertices] = v;
  curve_texels[num_curve_vertices] = t;
  num_curve_vertices++;
  return num_curve_vertices - 1;
}

void csThing::SetCurveVertex (int idx, const csVector3 &vt)
{
  CS_ASSERT (idx >= 0 && idx < num_curve_vertices);
  curve_vertices[idx] = vt;
  obj_bbox_valid = false;
  delete bbox;
  bbox = NULL;
  curves_transf_ok = false;
}

void csThing::SetCurveTexel (int idx, const csVector2 &vt)
{
  CS_ASSERT (idx >= 0 && idx < num_curve_vertices);
  curve_texels[idx] = vt;
}

void csThing::ClearCurveVertices ()
{
  delete[] curve_vertices;
  curve_vertices = NULL;
  delete[] curve_texels;
  curve_texels = NULL;
  obj_bbox_valid = false;
  delete bbox;
  bbox = NULL;
  curves_transf_ok = false;
}

int csThing::AddVertex (float x, float y, float z)
{
  if (!obj_verts)
  {
    max_vertices = 10;
    obj_verts = new csVector3[max_vertices];

    // Only if we occasionally move do we use the world vertex cache.
    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
      wor_verts = new csVector3[max_vertices];
    else
      wor_verts = obj_verts;
  }

  while (num_vertices >= max_vertices)
  {
    if (max_vertices < 10000)
      max_vertices *= 2;
    else
      max_vertices += 10000;

    csVector3 *new_obj_verts = new csVector3[max_vertices];
    memcpy (new_obj_verts, obj_verts, sizeof (csVector3) * num_vertices);
    delete[] obj_verts;
    obj_verts = new_obj_verts;

    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
    {
      csVector3 *new_wor_verts = new csVector3[max_vertices];
      memcpy (new_wor_verts, wor_verts, sizeof (csVector3) * num_vertices);
      delete[] wor_verts;
      wor_verts = new_wor_verts;
    }
    else
      wor_verts = obj_verts;
  }

  // By default all vertices are set with the same object space and world space.
  obj_verts[num_vertices].Set (x, y, z);
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
    wor_verts[num_vertices].Set (x, y, z);
  num_vertices++;
  return num_vertices - 1;
}

int csThing::AddVertexSmart (float x, float y, float z)
{
  if (!obj_verts)
  {
    AddVertex (x, y, z);
    return 0;
  }

  int i;
  for (i = 0; i < num_vertices; i++)
  {
    if (
      ABS (x - obj_verts[i].x) < SMALL_EPSILON &&
      ABS (y - obj_verts[i].y) < SMALL_EPSILON &&
      ABS (z - obj_verts[i].z) < SMALL_EPSILON)
    {
      return i;
    }
  }

  AddVertex (x, y, z);
  return num_vertices - 1;
}

void csThing::SetVertex (int idx, const csVector3 &vt)
{
  CS_ASSERT (idx >= 0 && idx < num_vertices);
  obj_verts[idx] = vt;
  if (wor_verts && wor_verts != obj_verts) wor_verts[idx] = vt;
}

void csThing::DeleteVertex (int idx)
{
  CS_ASSERT (idx >= 0 && idx < num_vertices);

  int copysize = sizeof (csVector3) * (num_vertices - idx - 1);
  memmove (obj_verts + idx, obj_verts + idx + 1, copysize);
  if (wor_verts && wor_verts != obj_verts)
    memmove (wor_verts + idx, wor_verts + idx + 1, copysize);
  if (cam_verts) memmove (cam_verts + idx, cam_verts + idx + 1, copysize);
  num_vertices--;
}

void csThing::DeleteVertices (int from, int to)
{
  if (from <= 0 && to >= num_vertices - 1)
  {
    // Delete everything.
    if (wor_verts == obj_verts)
      delete[] obj_verts;
    else
    {
      delete[] wor_verts;
      delete[] obj_verts;
    }

    delete[] cam_verts;
    max_vertices = num_vertices = 0;
    wor_verts = NULL;
    obj_verts = NULL;
    cam_verts = NULL;
    num_cam_verts = 0;
  }
  else
  {
    if (from < 0) from = 0;
    if (to >= num_vertices) to = num_vertices - 1;

    int rangelen = to - from + 1;
    int copysize = sizeof (csVector3) * (num_vertices - from - rangelen);
    memmove (obj_verts + from, obj_verts + from + rangelen, copysize);
    if (wor_verts && wor_verts != obj_verts)
      memmove (wor_verts + from, wor_verts + from + rangelen, copysize);
    if (cam_verts)
      memmove (cam_verts + from, cam_verts + from + rangelen, copysize);
    num_vertices -= rangelen;
  }
}

struct CompressVertex
{
  int orig_idx;
  float x, y, z;
  int new_idx;
  bool used;
};

static int compare_vt (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->x < sp2->x)
    return -1;
  else if (sp1->x > sp2->x)
    return 1;
  if (sp1->y < sp2->y)
    return -1;
  else if (sp1->y > sp2->y)
    return 1;
  if (sp1->z < sp2->z)
    return -1;
  else if (sp1->z > sp2->z)
    return 1;
  return 0;
}

static int compare_vt_orig (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->orig_idx < sp2->orig_idx)
    return -1;
  else if (sp1->orig_idx > sp2->orig_idx)
    return 1;
  return 0;
}

void csThing::CompressVertices ()
{
  if (num_vertices <= 0) return ;

  // Copy all the vertices.
  CompressVertex *vt = new CompressVertex[num_vertices];
  int i, j;
  for (i = 0; i < num_vertices; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = (float)ceil (obj_verts[i].x * 1000000);
    vt[i].y = (float)ceil (obj_verts[i].y * 1000000);
    vt[i].z = (float)ceil (obj_verts[i].z * 1000000);
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, num_vertices, sizeof (CompressVertex), compare_vt);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  int count_unique = 1;
  int last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1; i < num_vertices; i++)
  {
    if (
      vt[i].x != vt[last_unique].x ||
      vt[i].y != vt[last_unique].y ||
      vt[i].z != vt[last_unique].z)
    {
      last_unique = i;
      count_unique++;
    }

    vt[i].new_idx = last_unique;
  }

  // If count_unique == num_vertices then there is nothing to do.
  if (count_unique == num_vertices)
  {
    delete[] vt;
    return ;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  csVector3 *new_obj = new csVector3[count_unique];
  new_obj[0] = obj_verts[vt[0].orig_idx];

  csVector3 *new_wor = 0;
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
  {
    new_wor = new csVector3[count_unique];
    new_wor[0] = wor_verts[vt[0].orig_idx];
  }

  vt[0].new_idx = 0;
  j = 1;
  for (i = 1; i < num_vertices; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_obj[j] = obj_verts[vt[i].orig_idx];
      if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
        new_wor[j] = wor_verts[vt[i].orig_idx];
      vt[i].new_idx = j;
      j++;
    }
    else
      vt[i].new_idx = j - 1;
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, num_vertices, sizeof (CompressVertex), compare_vt_orig);

  // Replace the old vertex tables.
  delete[] obj_verts;
  obj_verts = new_obj;
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
  {
    delete[] wor_verts;
    wor_verts = new_wor;
  }
  else
    wor_verts = obj_verts;
  num_vertices = max_vertices = count_unique;

  // Now we can remap the vertices in all polygons.
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    csPolyIndexed &pi = p->GetVertices ();
    int *idx = pi.GetVertexIndices ();
    for (j = 0; j < pi.GetVertexCount (); j++) idx[j] = vt[idx[j]].new_idx;
  }

  delete[] vt;

  // If there is a bounding box we recreate it.
  if (bbox) CreateBoundingBox ();
}

void csThing::RemoveUnusedVertices ()
{
  if (num_vertices <= 0) return ;

  // Copy all the vertices that are actually used by polygons.
  bool *used = new bool[num_vertices];
  int i, j;
  for (i = 0; i < num_vertices; i++) used[i] = false;

  // Mark all vertices that are used as used.
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    csPolyIndexed &pi = p->GetVertices ();
    int *idx = pi.GetVertexIndices ();
    for (j = 0; j < pi.GetVertexCount (); j++) used[idx[j]] = true;
  }

  // Count relevant values.
  int count_relevant = 0;
  for (i = 0; i < num_vertices; i++)
  {
    if (used[i]) count_relevant++;
  }

  // If all vertices are relevant then there is nothing to do.
  if (count_relevant == num_vertices)
  {
    delete[] used;
    return ;
  }

  // Now allocate and fill new vertex tables.
  // Also fill the 'relocate' table.
  csVector3 *new_obj = new csVector3[count_relevant];
  csVector3 *new_wor = 0;
  int *relocate = new int[num_vertices];
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
    new_wor = new csVector3[count_relevant];
  j = 0;
  for (i = 0; i < num_vertices; i++)
  {
    if (used[i])
    {
      new_obj[j] = obj_verts[i];
      if (cfg_moving == CS_THING_MOVE_OCCASIONAL) new_wor[j] = wor_verts[i];
      relocate[i] = j;
      j++;
    }
    else
      relocate[i] = -1;
  }

  // Replace the old vertex tables.
  delete[] obj_verts;
  obj_verts = new_obj;
  if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
  {
    delete[] wor_verts;
    wor_verts = new_wor;
  }
  else
    wor_verts = obj_verts;
  num_vertices = max_vertices = count_relevant;

  // Now we can remap the vertices in all polygons.
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    csPolyIndexed &pi = p->GetVertices ();
    int *idx = pi.GetVertexIndices ();
    for (j = 0; j < pi.GetVertexCount (); j++) idx[j] = relocate[idx[j]];
  }

  delete[] relocate;
  delete[] used;

  // If there is a bounding box we recreate it.
  if (bbox) CreateBoundingBox ();
}

csPolygon3D *csThing::GetPolygon3D (const char *name)
{
  int idx = polygons.FindKey (name);
  return idx >= 0 ? polygons.Get (idx) : NULL;
}

int csThing::FindPolygonIndex (iPolygon3D *polygon) const
{
  csPolygon3D *p = polygon->GetPrivateObject ();
  return polygons.Find (p);
}

csPolygon3D *csThing::NewPolygon (csMaterialWrapper *material)
{
  csPolygon3D *p = new csPolygon3D (material);
  AddPolygon (p);
  return p;
}

void csThing::InvalidateThing ()
{
#ifndef CS_USE_NEW_RENDERER
  if (polybuf)
  {
    polybuf->DecRef ();
    polybuf = NULL;
  }
#endif // CS_USE_NEW_RENDERER

  delete[] polybuf_materials;
  polybuf_materials = NULL;
  thing_edges_valid = false;
  prepared = false;
  obj_bbox_valid = false;
  delete bbox;
  bbox = NULL;
  CleanupThingEdgeTable ();

  shapenr++;
  scfiPolygonMeshLOD.Cleanup ();
  scfiPolygonMesh.Cleanup ();
  delete [] obj_normals; obj_normals = NULL;
  FireListeners ();
}

void csThing::RemovePolygon (int idx)
{
  InvalidateThing ();

  csPolygon3D *poly3d = GetPolygon3D (idx);
  if (poly3d->GetPortal ())
  {
    RemovePortalPolygon (poly3d);
  }

  polygons.Delete (idx);
}

iPolygonMesh* csThing::GetWriteObject ()
{
  return &scfiPolygonMeshLOD;
}

void csThing::RemovePolygons ()
{
  InvalidateThing ();
  polygons.DeleteAll ();          // delete prior to portal_poly array !
  if (portal_polygons.Length ()) portal_polygons.DeleteAll ();
}

void csThing::AddPolygon (csPolygon3D *poly)
{
  InvalidateThing ();

  poly->SetParent (this);
  polygons.Push (poly);
  if (poly->GetPortal ()) AddPortalPolygon (poly);
}

csCurve *csThing::GetCurve (char *name) const
{
  int idx = curves.FindKey (name);
  return idx >= 0 ? curves.Get (idx) : NULL;
}

void csThing::AddCurve (csCurve *curve)
{
  curve->SetParentThing (this);
  curves.Push (curve);
  curves_transf_ok = false;
  obj_bbox_valid = false;
  delete bbox;
  bbox = NULL;
}

iCurve *csThing::CreateCurve (iCurveTemplate *tmpl)
{
  iCurve *curve = tmpl->MakeCurve ();
  csCurve *c = curve->GetOriginalObject ();
  c->SetParentThing (this);

  int i;
  for (i = 0; i < tmpl->GetVertexCount (); i++)
    curve->SetControlPoint (i, tmpl->GetVertex (i));
  AddCurve (c);
  return curve;
}

int csThing::FindCurveIndex (iCurve *curve) const
{
  return curves.Find (curve->GetOriginalObject ());
}

void csThing::RemoveCurve (int idx)
{
  curves.Delete (idx);
  curves_transf_ok = false;
  obj_bbox_valid = false;
  delete bbox;
  bbox = NULL;
}

void csThing::RemoveCurves ()
{
  curves.DeleteAll ();
  curves_transf_ok = false;
  obj_bbox_valid = false;
  delete bbox;
  bbox = NULL;
}

void csThing::HardTransform (const csReversibleTransform &t)
{
  int i, j;

  for (i = 0; i < num_vertices; i++)
  {
    obj_verts[i] = t.This2Other (obj_verts[i]);
    if (cfg_moving == CS_THING_MOVE_OCCASIONAL) wor_verts[i] = obj_verts[i];
  }

  curves_center = t.This2Other (curves_center);
  if (curve_vertices)
    for (i = 0; i < num_curve_vertices; i++)
      curve_vertices[i] = t.This2Other (curve_vertices[i]);

  //-------
  // First we collect all planes from the set of polygons
  // and transform each plane one by one. We actually create
  // new planes and ditch the others (DecRef()) to avoid
  // sharing planes with polygons that were not affected by
  // this HardTransform().
  csHashSet planes (8087);
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    csPolyTexLightMap *lmi = p->GetLightMapInfo ();
    if (lmi)
    {
      iPolyTxtPlane *pl = lmi->GetPolyTxtPlane ();
      planes.Add (pl);
    }
  }

  // As a special optimization we don't copy planes that have no
  // name. This under the assumption that planes with no name
  // cannot be shared (let's make this a rule!).
  csHashIterator *hashit = new csHashIterator (planes.GetHashMap ());
  while (hashit->HasNext ())
  {
    iPolyTxtPlane *pl = (iPolyTxtPlane *)hashit->Next ();
    if (pl->QueryObject ()->GetName () == NULL)
    {
      // This is a non-shared plane.
      pl->GetPrivateObject ()->HardTransform (t);
    }
    else
    {
      // This plane is potentially shared. We have to make a duplicate
      // and modify all polygons to use this one. Note that this will
      // mean that potentially two planes exist with the same name.
      csRef<iPolyTxtPlane> new_pl (thing_type->
        CreatePolyTxtPlane (pl->QueryObject ()->GetName ()));
      csMatrix3 m;
      csVector3 v;
      pl->GetTextureSpace (m, v);
      new_pl->SetTextureSpace (m, v);
      new_pl->GetPrivateObject ()->HardTransform (t);
      for (j = 0; j < polygons.Length (); j++)
      {
        csPolygon3D *p = GetPolygon3D (j);
        csPolyTexLightMap *lmi = p->GetLightMapInfo ();
        if (lmi && lmi->GetPolyTxtPlane () == pl)
        {
          lmi->SetTxtPlane (new_pl->GetPrivateObject ());
          lmi->Setup (p, p->GetMaterialWrapper ());
        }
      }
    }
  }

  delete hashit;

  //-------
  //-------
  // Now do a similar thing for the normal planes.
  planes.DeleteAll ();
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    csPolyPlane *pl = p->GetPlane ();
    if (!planes.In (pl))
    {
      planes.Add (pl);

      csPlane3 new_plane;
      t.This2Other (pl->GetObjectPlane (), p->Vobj (0), new_plane);
      pl->GetObjectPlane () = new_plane;
      pl->GetWorldPlane () = new_plane;
    }
  }

  //-------
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    p->HardTransform (t);
  }

  curves_transf_ok = false;
  for (i = 0; i < curves.Length (); i++)
  {
    csCurve *c = GetCurve (i);
    c->HardTransform (t);
  }
}

csPolygon3D *csThing::IntersectSegmentFull (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr,
  csMeshWrapper **p_mesh)
{
  if (p_mesh) *p_mesh = NULL;

  int i;
  float r, best_r = 2000000000.;
  csVector3 cur_isect;
  csPolygon3D *best_p = NULL;

  // @@@ This routine is not very optimal. Especially for things
  // with large number of polygons.
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    if (p->IntersectSegment (start, end, cur_isect, &r))
    {
      if (r < best_r)
      {
        best_r = r;
        best_p = p;
        isect = cur_isect;
      }
    }
  }

  if (pr) *pr = best_r;
  if (p_mesh) *p_mesh = NULL;
  return best_p;
}

csPolygon3D *csThing::IntersectSegment (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr,
  bool only_portals)
{
  if (only_portals)
  {
    int i;
    float r, best_r = 2000000000.;
    csVector3 cur_isect;
    csPolygon3D *best_p = NULL;
    for (i = 0; i < portal_polygons.Length (); i++)
    {
      csPolygon3D *p = (csPolygon3D *)portal_polygons[i];
      if (p->IntersectSegment (start, end, cur_isect, &r))
      {
        if (r < best_r)
        {
          best_r = r;
          best_p = p;
          isect = cur_isect;
        }
      }
    }

    if (pr) *pr = best_r;
    return best_p;
  }

  int i;
  float r, best_r = 2000000000.;
  csVector3 cur_isect;
  csPolygon3D *best_p = NULL;

  // @@@ This routine is not very optimal. Especially for things
  // with large number of polygons.
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    if (p->IntersectSegment (start, end, cur_isect, &r))
    {
      if (r < best_r)
      {
        best_r = r;
        best_p = p;
        isect = cur_isect;
      }
    }
  }

  if (pr) *pr = best_r;
  return best_p;
}

bool csThing::HitBeamOutline (const csVector3& start,
  const csVector3& end, csVector3& isect, float* pr)
{
  int i;
  float r;

  // @@@ This routine is not very optimal. Especially for things
  // with large number of polygons.
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *p = polygons.Get (i);
    if (p->IntersectSegment (start, end, isect, &r))
    {
      if (pr) *pr = r;
      return true;
    }
  }

  return false;
}

bool csThing::HitBeamObject (const csVector3& start,
  const csVector3& end, csVector3& isect, float *pr)
{
  csPolygon3D* poly = IntersectSegment (start, end, isect, pr, false);
  if (poly) return true;
  return false;
}

#ifndef CS_USE_NEW_RENDERER
void csThing::DrawOnePolygon (
  csPolygon3D *p,
  csPolygon2D *poly,
  iRenderView *d,
  csZBufMode zMode)
{
  iCamera *icam = d->GetCamera ();

  if (d->AddedFogInfo ())
  {
    // If fog info was added then we are dealing with vertex fog and
    // the current sector has fog. This means we have to complete the
    // fog_info structure with the plane of the current polygon.
    d->GetFirstFogInfo ()->outgoing_plane = p->GetPlane ()->GetCameraPlane ();
  }

  csPortal *po = p->GetPortal ();
  if (csSector::do_portals && po)
  {
    bool filtered = false;

    // is_this_fog is true if this sector is fogged.
    bool is_this_fog = d->GetThisSector ()->HasFog ();

    // If there is filtering (alpha mapping or something like that) we need
    // to keep the texture plane so that it can be drawn after the sector has
    // been drawn. The texture plane needs to be kept because this polygon
    // may be rendered again (through mirrors) possibly overwriting the plane.
    csPolyPlane *keep_plane = NULL;
    if (
      d->GetGraphics3D ()->GetRenderState (
          G3DRENDERSTATE_TRANSPARENCYENABLE))
      filtered = p->IsTransparent ();
    if (filtered || is_this_fog || (po && po->flags.Check (CS_PORTAL_ZFILL)))
    {
      keep_plane = new csPolyPlane (*(p->GetPlane ()));
    }

    // Draw through the portal. If this fails we draw the original polygon
    // instead. Drawing through a portal can fail because we have reached
    // the maximum number that a sector is drawn (for mirrors).
    if (po->Draw (poly, &(p->scfiPolygon3D), d))
    {
      if (filtered) poly->DrawFilled (d, p, keep_plane, zMode);
      if (is_this_fog)
      {
        poly->AddFogPolygon (
            d->GetGraphics3D (),
            p,
            keep_plane,
            icam->IsMirrored (),
            d->GetThisSector ()->QueryObject ()->GetID (),
            CS_FOG_BACK);
      }

      // Here we z-fill the portal contents to make sure that sprites
      // that are drawn outside of this portal cannot accidently cross
      // into the others sector space (we cannot trust the Z-buffer here).
      if (po->flags.Check (CS_PORTAL_ZFILL))
        poly->FillZBuf (d, p, keep_plane);
    }
    else
      poly->DrawFilled (d, p, p->GetPlane (), zMode);

    // Cleanup.
    if (keep_plane) keep_plane->DecRef ();
  }
  else
    poly->DrawFilled (d, p, p->GetPlane (), zMode);
}

void csThing::DrawPolygonArray (
  csPolygon3D **polygon,
  int num,
  iRenderView *d,
  csZBufMode zMode)
{
  csPolygon3D *p;
  csVector3 *verts;
  int num_verts;
  int i;
  csPoly2DPool *render_pool = thing_type->render_pol2d_pool;
  csPolygon2D *clip;
  iCamera *icam = d->GetCamera ();
  const csReversibleTransform &camtrans = icam->GetTransform ();

  // Setup clip and far plane.
  csPlane3 clip_plane, *pclip_plane;
  bool do_clip_plane = d->GetClipPlane (clip_plane);
  if (do_clip_plane)
    pclip_plane = &clip_plane;
  else
    pclip_plane = NULL;

  csPlane3 *plclip = icam->GetFarPlane ();

  for (i = 0; i < num; i++)
  {
    p = polygon[i];
    if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
    p->UpdateTransformation (camtrans, icam->GetCameraNumber ());
    if (p->ClipToPlane (pclip_plane, camtrans.GetOrigin (), verts, num_verts))  //@@@Pool for verts?
    {
      // The far plane is defined negative. So if the polygon is entirely
      // in front of the far plane it is not visible. Otherwise we will render
      // it.
      if (
        !plclip ||
        csPoly3D::Classify (*plclip, verts, num_verts) != CS_POL_FRONT)
      {
        clip = (csPolygon2D *) (render_pool->Alloc ());
        if (
          p->DoPerspective (
              camtrans,
              verts,
              num_verts,
              clip,
              NULL,
              icam->IsMirrored ()) &&
          clip->ClipAgainst (d->GetClipper ()))
        {
          p->GetPlane ()->WorldToCamera (camtrans, verts[0]);
          DrawOnePolygon (p, clip, d, zMode);
        }

        render_pool->Free (clip);
      }
    }
  }
}

struct MatPol
{
  iMaterialWrapper *mat;
  int mat_index;
  csPolygon3D *poly;
};

static int compare_material (const void *p1, const void *p2)
{
  MatPol *sp1 = (MatPol *)p1;
  MatPol *sp2 = (MatPol *)p2;
  if (sp1->mat < sp2->mat)
    return -1;
  else if (sp1->mat > sp2->mat)
    return 1;
  return 0;
}

void csThing::PreparePolygonBuffer ()
{
  if (polybuf) return ;

  iVertexBufferManager *vbufmgr = thing_type->G3D->
    GetVertexBufferManager ();
  polybuf = vbufmgr->CreatePolygonBuffer ();
  polybuf->SetVertexArray (obj_verts, num_vertices);

  int i;

  //-----
  // First collect all material wrappers and polygons.
  //-----
  MatPol *matpol = new MatPol[polygons.Length ()];
  for (i = 0; i < polygons.Length (); i++)
  {
    matpol[i].poly = GetPolygon3D (i);
    matpol[i].mat = &(matpol[i].poly->GetMaterialWrapper ()
    	->scfiMaterialWrapper);
  }

  //-----
  // Sort on material.
  //-----
  qsort (matpol, polygons.Length (), sizeof (MatPol), compare_material);

  //-----
  // Now count all different materials we have and add them to the polygon
  // buffer. Also update the matpol structure with the index in the
  // material table.
  //-----
  polybuf->AddMaterial (matpol[0].mat->GetMaterialHandle ());
  matpol[0].mat_index = 0;
  polybuf_material_count = 1;
  for (i = 1; i < polygons.Length (); i++)
  {
    if (matpol[i].mat != matpol[i - 1].mat)
    {
      polybuf->AddMaterial (matpol[i].mat->GetMaterialHandle ());
      matpol[i].mat_index = polybuf_material_count;
      polybuf_material_count++;
    }
    else
    {
      matpol[i].mat_index = matpol[i - 1].mat_index;
    }
  }

  //-----
  // Update our local material wrapper table.
  //-----
  polybuf_materials = new iMaterialWrapper *[polybuf_material_count];
  polybuf_materials[0] = matpol[0].mat;
  polybuf_material_count = 1;
  for (i = 1; i < polygons.Length (); i++)
  {
    if (matpol[i].mat != matpol[i - 1].mat)
    {
      polybuf_materials[polybuf_material_count] = matpol[i].mat;
      polybuf_material_count++;
    }
  }

  //-----
  // Now add the polygons to the polygon buffer sorted by material.
  //-----
  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D *poly = matpol[i].poly;
    csPolyTexLightMap *lmi = poly->GetLightMapInfo ();

    // @@@ what if lmi == NULL?
    //CS_ASSERT (lmi != NULL);
    if (lmi)
    {
      csPolyTxtPlane *txt_plane = lmi->GetTxtPlane ();
      csMatrix3 *m_obj2tex;
      csVector3 *v_obj2tex;
      txt_plane->GetObjectToTexture (m_obj2tex, v_obj2tex);
      polybuf->AddPolygon (
          poly->GetVertexIndices (),
          poly->GetVertexCount (),
          poly->GetPlane ()->GetObjectPlane (),
          matpol[i].mat_index,
          *m_obj2tex,
          *v_obj2tex,
          lmi->GetPolyTex ());
    }
    else
    {
      csMatrix3 m_obj2tex;	// @@@
      csVector3 v_obj2tex;
      polybuf->AddPolygon (
          poly->GetVertexIndices (),
          poly->GetVertexCount (),
          poly->GetPlane ()->GetObjectPlane (),
          matpol[i].mat_index,
          m_obj2tex,
          v_obj2tex,
          NULL);
    }
  }

  delete[] matpol;
}
#endif // CS_USE_NEW_RENDERER

void csThing::GetTransformedBoundingBox (
  const csReversibleTransform &trans,
  csBox3 &cbox)
{
  //@@@@@@@@@@@@@@

  // @@@ Shouldn't we try to cache this depending on camera/movable number?

  // Similar to what happens in csSprite3D.
  csBox3 box;
  GetBoundingBox (box);
  cbox.StartBoundingBox (trans * box.GetCorner (0));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (1));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (2));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (3));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (4));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (5));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (6));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (7));
}

static void Perspective (
  const csVector3 &v,
  csVector2 &p,
  float fov,
  float sx,
  float sy)
{
  float iz = fov / v.z;
  p.x = v.x * iz + sx;
  p.y = v.y * iz + sy;
}

float csThing::GetScreenBoundingBox (
  float fov,
  float sx,
  float sy,
  const csReversibleTransform &trans,
  csBox2 &sbox,
  csBox3 &cbox)
{
  csVector2 oneCorner;

  GetTransformedBoundingBox (trans, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Sprite is very close to camera.

    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    Perspective (cbox.Max (), oneCorner, fov, sx, sy);
    sbox.StartBoundingBox (oneCorner);

    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    Perspective (cbox.Min (), oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    Perspective (v, oneCorner, fov, sx, sy);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

#ifndef CS_USE_NEW_RENDERER
void csThing::DrawPolygonArrayDPM (
  csPolygon3D ** /*polygon*/,
  int /*num*/,
  iRenderView *rview,
  iMovable *movable,
  csZBufMode zMode)
{
  PreparePolygonBuffer ();

  iCamera *icam = rview->GetCamera ();
  csReversibleTransform tr_o2c = icam->GetTransform ();
  if (!movable->IsFullTransformIdentity ())
    tr_o2c /= movable->GetFullTransform ();

  G3DPolygonMesh mesh;
  csBox2 bbox;
  csBox3 bbox3;
  if (
    GetScreenBoundingBox (
        icam->GetFOV (),
        icam->GetShiftX (),
        icam->GetShiftY (),
        tr_o2c,
        bbox,
        bbox3) < 0)
    return ;  // Not visible.
  if (
    !rview->ClipBBox (
        bbox,
        bbox3,
        mesh.clip_portal,
        mesh.clip_plane,
        mesh.clip_z_plane))
    return ;  // Not visible.
  int i;

  rview->GetGraphics3D ()->SetObjectToCamera (&tr_o2c);
  rview->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zMode);

  mesh.polybuf = polybuf;
  for (i = 0; i < polybuf_material_count; i++)
  {
    polybuf_materials[i]->Visit ();
    polybuf->SetMaterial (i, polybuf_materials[i]->GetMaterialHandle ());
  }

  mesh.do_fog = false;
  mesh.do_mirror = icam->IsMirrored ();
  mesh.vertex_mode = G3DPolygonMesh::VM_WORLDSPACE;
  mesh.vertex_fog = NULL;
  mesh.mixmode = CS_FX_COPY;

  rview->CalculateFogMesh(tr_o2c,mesh);
  rview->GetGraphics3D ()->DrawPolygonMesh (mesh);
}
#endif // CS_USE_NEW_RENDERER

#ifdef CS_DEBUG
bool viscnt_enabled;
int viscnt_vis_poly;
int viscnt_invis_poly;
int viscnt_vis_node;
int viscnt_invis_node;
int viscnt_vis_obj;
int viscnt_invis_obj;
float viscnt_vis_node_vol;
float viscnt_invis_node_vol;
#endif

// @@@ We need a more clever algorithm here. We should try
// to recognize convex sub-parts of a polygonset and return
// convex shadow frustums for those. This will significantly
// reduce the number of shadow frustums. There are basicly
// two ways to do this:
//	- Split object into convex sub-parts in 3D.
//	- Split object into convex sub-parts in 2D.
// The first way is probably best because it is more efficient
// at runtime (important if we plan to use dynamic shadows for things)
// and also more correct in that a convex 3D object has no internal
// shadowing while a convex outline may have no correspondance to internal
// shadows.
void csThing::AppendShadows (
  iMovable* movable,
  iShadowBlockList *shadows,
  const csVector3 &origin)
{
  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  iShadowBlock *list = shadows->NewShadowBlock (
      polygons.Length ());
  csFrustum *frust;
  int i, j;
  csPolygon3D *p;
  bool cw = true;                   //@@@ Use mirroring parameter here!
  for (i = 0; i < polygons.Length (); i++)
  {
    p = polygons.Get (i);
    if (p->GetPortal ()) continue;  // No portals

    //if (p->GetPlane ()->VisibleFromPoint (origin) != cw) continue;
    float clas = p->GetPlane ()->GetWorldPlane ().Classify (origin);
    if (ABS (clas) < EPSILON) continue;
    if ((clas <= 0) != cw) continue;

    csPlane3 pl = p->GetPlane ()->GetWorldPlane ();
    pl.DD += origin * pl.norm;
    pl.Invert ();
    frust = list->AddShadow (
        origin,
        (void *)p,
        p->GetVertices ().GetVertexCount (),
        pl);
    for (j = 0; j < p->GetVertices ().GetVertexCount (); j++)
      frust->GetVertex (j).Set (p->Vwor (j) - origin);
  }
}

void csThing::CreateBoundingBox ()
{
  float minx, miny, minz, maxx, maxy, maxz;
  delete bbox;
  bbox = NULL;
  if (num_vertices <= 0 && num_curve_vertices <= 0) return ;
  bbox = new csThingBBox ();
  int i;
  if (num_vertices > 0)
  {
    minx = maxx = obj_verts[0].x;
    miny = maxy = obj_verts[0].y;
    minz = maxz = obj_verts[0].z;

    for (i = 1; i < num_vertices; i++)
    {
      if (obj_verts[i].x < minx)
        minx = obj_verts[i].x;
      else if (obj_verts[i].x > maxx)
        maxx = obj_verts[i].x;
      if (obj_verts[i].y < miny)
        miny = obj_verts[i].y;
      else if (obj_verts[i].y > maxy)
        maxy = obj_verts[i].y;
      if (obj_verts[i].z < minz)
        minz = obj_verts[i].z;
      else if (obj_verts[i].z > maxz)
        maxz = obj_verts[i].z;
    }
  }
  else if (num_curve_vertices == 0)
  {
    minx = 10000000.;
    miny = 10000000.;
    minz = 10000000.;
    maxx = -10000000.;
    maxy = -10000000.;
    maxz = -10000000.;
  }

  if (num_curve_vertices > 0)
  {
    int stidx = 0;
    if (num_vertices == 0)
    {
      csVector3 &cv = curve_vertices[0];
      minx = maxx = cv.x;
      miny = maxy = cv.y;
      minz = maxz = cv.z;
      stidx = 1;
    }

    for (i = stidx ; i < num_curve_vertices ; i++)
    {
      csVector3 &cv = curve_vertices[i];
      if (cv.x < minx)
        minx = cv.x;
      else if (cv.x > maxx)
        maxx = cv.x;
      if (cv.y < miny)
        miny = cv.y;
      else if (cv.y > maxy)
        maxy = cv.y;
      if (cv.z < minz)
        minz = cv.z;
      else if (cv.z > maxz)
        maxz = cv.z;
    }
  }

  bbox->i7 = AddVertex (minx, miny, minz);
  bbox->i3 = AddVertex (minx, miny, maxz);
  bbox->i5 = AddVertex (minx, maxy, minz);
  bbox->i1 = AddVertex (minx, maxy, maxz);
  bbox->i8 = AddVertex (maxx, miny, minz);
  bbox->i4 = AddVertex (maxx, miny, maxz);
  bbox->i6 = AddVertex (maxx, maxy, minz);
  bbox->i2 = AddVertex (maxx, maxy, maxz);
}

void csThing::GetRadius (csVector3 &rad, csVector3 &cent)
{
  csBox3 b;
  GetBoundingBox (b);
  rad = obj_radius;
  cent = b.GetCenter ();
}

void csThing::GetBoundingBox (csBox3 &box)
{
  if (obj_bbox_valid)
  {
    box = obj_bbox;
    return ;
  }

  obj_bbox_valid = true;

  if (!bbox) CreateBoundingBox ();

  if (!obj_verts)
  {
    obj_bbox.Set (0, 0, 0, 0, 0, 0);
    box = obj_bbox;
    return ;
  }

  csVector3 min_bbox, max_bbox;
  min_bbox = max_bbox = Vobj (bbox->i1);
  if (Vobj (bbox->i2).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i2).x;
  else if (Vobj (bbox->i2).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i2).x;
  if (Vobj (bbox->i2).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i2).y;
  else if (Vobj (bbox->i2).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i2).y;
  if (Vobj (bbox->i2).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i2).z;
  else if (Vobj (bbox->i2).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i2).z;
  if (Vobj (bbox->i3).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i3).x;
  else if (Vobj (bbox->i3).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i3).x;
  if (Vobj (bbox->i3).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i3).y;
  else if (Vobj (bbox->i3).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i3).y;
  if (Vobj (bbox->i3).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i3).z;
  else if (Vobj (bbox->i3).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i3).z;
  if (Vobj (bbox->i4).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i4).x;
  else if (Vobj (bbox->i4).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i4).x;
  if (Vobj (bbox->i4).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i4).y;
  else if (Vobj (bbox->i4).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i4).y;
  if (Vobj (bbox->i4).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i4).z;
  else if (Vobj (bbox->i4).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i4).z;
  if (Vobj (bbox->i5).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i5).x;
  else if (Vobj (bbox->i5).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i5).x;
  if (Vobj (bbox->i5).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i5).y;
  else if (Vobj (bbox->i5).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i5).y;
  if (Vobj (bbox->i5).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i5).z;
  else if (Vobj (bbox->i5).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i5).z;
  if (Vobj (bbox->i6).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i6).x;
  else if (Vobj (bbox->i6).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i6).x;
  if (Vobj (bbox->i6).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i6).y;
  else if (Vobj (bbox->i6).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i6).y;
  if (Vobj (bbox->i6).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i6).z;
  else if (Vobj (bbox->i6).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i6).z;
  if (Vobj (bbox->i7).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i7).x;
  else if (Vobj (bbox->i7).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i7).x;
  if (Vobj (bbox->i7).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i7).y;
  else if (Vobj (bbox->i7).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i7).y;
  if (Vobj (bbox->i7).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i7).z;
  else if (Vobj (bbox->i7).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i7).z;
  if (Vobj (bbox->i8).x < min_bbox.x)
    min_bbox.x = Vobj (bbox->i8).x;
  else if (Vobj (bbox->i8).x > max_bbox.x)
    max_bbox.x = Vobj (bbox->i8).x;
  if (Vobj (bbox->i8).y < min_bbox.y)
    min_bbox.y = Vobj (bbox->i8).y;
  else if (Vobj (bbox->i8).y > max_bbox.y)
    max_bbox.y = Vobj (bbox->i8).y;
  if (Vobj (bbox->i8).z < min_bbox.z)
    min_bbox.z = Vobj (bbox->i8).z;
  else if (Vobj (bbox->i8).z > max_bbox.z)
    max_bbox.z = Vobj (bbox->i8).z;
  obj_bbox.Set (min_bbox, max_bbox);
  box = obj_bbox;
  obj_radius = (max_bbox - min_bbox) * 0.5f;
  max_obj_radius = qsqrt (csSquaredDist::PointPoint (max_bbox, min_bbox)) * 0.5f;
}

void csThing::GetBoundingBox (iMovable *movable, csBox3 &box)
{
  if (wor_bbox_movablenr != movable->GetUpdateNumber ())
  {
    // First make sure obj_bbox is valid.
    GetBoundingBox (box);
    wor_bbox_movablenr = movable->GetUpdateNumber ();

    // @@@ Maybe it would be better to really calculate the bounding box
    // here instead of just transforming the object space bounding box?
    if (movable->IsFullTransformIdentity ())
    {
      wor_bbox = obj_bbox;
    }
    else
    {
      csReversibleTransform mt = movable->GetFullTransform ();
      wor_bbox.StartBoundingBox (mt.This2Other (obj_bbox.GetCorner (0)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (1)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (2)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (3)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (4)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (5)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (6)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (7)));
    }
  }

  box = wor_bbox;
}

//-------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csThing::PolyMeshLOD)
  SCF_IMPLEMENTS_INTERFACE(iPolygonMesh)
SCF_IMPLEMENT_IBASE_END

csThing::PolyMeshLOD::PolyMeshLOD () : PolyMeshHelper (CS_POLY_VISCULL)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

//-------------------------------------------------------------------------

void PolyMeshHelper::Setup ()
{
  if (polygons || alloc_vertices)
  {
    // Already set up. First we check if the object vertex array
    // is still valid (if it is not a copy).
    if (alloc_vertices) return ;
    if (vertices == thing->obj_verts) return ;
  }

  vertices = NULL;

  // Count the number of needed polygons and vertices.
  num_verts = thing->GetVertexCount ();
  num_poly = 0;

  int i, j;
  const csPolygonArray &pol = thing->polygons;
  for (i = 0; i < thing->GetPolygonCount (); i++)
  {
    csPolygon3D *p = pol.Get (i);
    if (p->flags.Check (poly_flag)) num_poly++;
  }

  // Check curves.
  for (i = 0; i < thing->GetCurveCount (); i++)
  {
    csCurve *c = thing->curves.Get (i);
    csCurveTesselated *tess = c->Tesselate (1000);    // @@@ High quality?
    num_poly += tess->GetTriangleCount ();
    num_verts += tess->GetVertexCount ();
  }

  // Allocate the arrays and the copy the data.
  if (num_verts)
  {
    // If there are no curves we don't need to copy vertex data.
    if (thing->GetCurveCount () == 0)
    {
      vertices = thing->obj_verts;
    }
    else
    {
      alloc_vertices = new csVector3[num_verts];
      vertices = alloc_vertices;

      // Copy the polygon vertices.
      // Set num_verts to the number of vertices in polygon set so
      // that we can continue copying vertices from curves.
      num_verts = thing->GetVertexCount ();
      if (num_verts)
      {
        memcpy (
          vertices,
          thing->obj_verts,
          sizeof (csVector3) * num_verts);
      }
    }
  }

  if (num_poly)
  {
    polygons = new csMeshedPolygon[num_poly];
    num_poly = 0;
    for (i = 0; i < thing->GetPolygonCount (); i++)
    {
      csPolygon3D *p = pol.Get (i);
      if (p->flags.Check (poly_flag))
      {
        polygons[num_poly].num_vertices = p->GetVertexCount ();
        polygons[num_poly].vertices = p->GetVertexIndices ();
        num_poly++;
      }
    }

    // Indicate that polygons after and including this index need to
    // have their 'vertices' array cleaned up. These polygons were generated
    // from curves.
    curve_poly_start = num_poly;
#ifndef CS_USE_NEW_RENDERER
    for (i = 0; i < thing->GetCurveCount (); i++)
    {
      csCurve *c = thing->curves.Get (i);
      csCurveTesselated *tess = c->Tesselate (1000);  // @@@ High quality?
      csTriangle *tris = tess->GetTriangles ();
      int tri_count = tess->GetTriangleCount ();
      for (j = 0; j < tri_count; j++)
      {
        polygons[num_poly].num_vertices = 3;
        polygons[num_poly].vertices = new int[3];

        // Adjust indices to skip the original polygon set vertices and

        // preceeding curves.
        polygons[num_poly].vertices[0] = tris[j].a + num_verts;
        polygons[num_poly].vertices[1] = tris[j].b + num_verts;
        polygons[num_poly].vertices[2] = tris[j].c + num_verts;
        num_poly++;
      }

      csVector3 *vts = tess->GetVertices ();
      int num_vt = tess->GetVertexCount ();
      memcpy (vertices + num_verts, vts, sizeof (csVector3) * num_vt);
      num_verts += num_vt;
    }
#endif // CS_USE_NEW_RENDERER
  }
}

void PolyMeshHelper::Cleanup ()
{
  int i;

  // Delete all polygons which were generated from curved surfaces.
  // The other polygons just have a reference to the original polygons
  // from the parent.
  if (polygons)
  {
    for (i = curve_poly_start; i < num_poly; i++)
    {
      delete[] polygons[i].vertices;
    }

    delete[] polygons;
    polygons = NULL;
  }

  delete[] alloc_vertices;
  alloc_vertices = NULL;
  vertices = NULL;
}

//-------------------------------------------------------------------------

void csThing::SetConvex (bool c)
{
  flags.Set (CS_ENTITY_CONVEX, c ? CS_ENTITY_CONVEX : 0);
  if (c)
  {
    if (center_idx == -1) center_idx = AddVertex (0, 0, 0);

    int i;
    float minx = 1000000000., miny = 1000000000., minz = 1000000000.;
    float maxx = -1000000000., maxy = -1000000000., maxz = -1000000000.;
    for (i = 0; i < num_vertices; i++)
    {
      if (i != center_idx)
      {
        if (obj_verts[i].x < minx) minx = obj_verts[i].x;
        if (obj_verts[i].x > maxx) maxx = obj_verts[i].x;
        if (obj_verts[i].y < miny) miny = obj_verts[i].y;
        if (obj_verts[i].y > maxy) maxy = obj_verts[i].y;
        if (obj_verts[i].z < minz) minz = obj_verts[i].z;
        if (obj_verts[i].z > maxz) maxz = obj_verts[i].z;
      }
    }

    obj_verts[center_idx].Set (
        (minx + maxx) / 2,
        (miny + maxy) / 2,
        (minz + maxz) / 2);
    if (cfg_moving == CS_THING_MOVE_OCCASIONAL)
      wor_verts[center_idx].Set (
          (minx + maxx) / 2,
          (miny + maxy) / 2,
          (minz + maxz) / 2);
  }
}

void csThing::UpdateCurveTransform (const csReversibleTransform &movtrans)
{
  if (GetCurveCount () == 0) return ;

  // since obj has changed (possibly) we need to tell all of our curves
  csReversibleTransform o2w = movtrans.GetInverse ();
  int i;
  for (i = 0; i < GetCurveCount (); i++)
  {
    csCurve *c = curves.Get (i);
    c->SetObject2World (&o2w);
  }
}

void csThing::UpdateCurveTransform ()
{
  if (curves_transf_ok) return ;
  curves_transf_ok = true;
  if (GetCurveCount () == 0) return ;

  csReversibleTransform o2w;                    // Identity transform.
  int i;
  for (i = 0; i < GetCurveCount (); i++)
  {
    csCurve *c = curves.Get (i);
    c->SetObject2World (&o2w);
  }
}

csPolygon3D *csThing::IntersectSphere (
  csVector3 &center,
  float radius,
  float *pr)
{
  float d, min_d = radius;
  int i;
  csPolygon3D *p, *min_p = NULL;
  csVector3 hit;

  for (i = 0; i < polygons.Length (); i++)
  {
    p = GetPolygon3D (i);

    const csPlane3 &wpl = p->GetPlane ()->GetObjectPlane ();
    d = wpl.Distance (center);
    if (d < min_d && csMath3::Visible (center, wpl))
    {
      hit = -center;
      hit -= wpl.Normal ();
      hit *= d;
      hit += center;
      if (p->IntersectRay (center, hit))
      {
        min_d = d;
        min_p = p;
      }
    }
  }

  if (pr) *pr = min_d;
  return min_p;
}

/// The list of fog vertices
#ifndef CS_USE_NEW_RENDERER
CS_TYPEDEF_GROWING_ARRAY (engine3d_StaticFogVerts, G3DFogInfo);
CS_IMPLEMENT_STATIC_VAR (GetStaticFogVerts, engine3d_StaticFogVerts,())

bool csThing::DrawCurves (
  iRenderView *rview,
  iMovable *movable,
  csZBufMode zMode)
{
  static engine3d_StaticFogVerts &fog_verts = *GetStaticFogVerts ();
  if (GetCurveCount () <= 0) return false;

  iCamera *icam = rview->GetCamera ();
  const csReversibleTransform &camtrans = icam->GetTransform ();

  csReversibleTransform movtrans;

  // Only get the transformation if this thing can move.
  bool can_move = false;
  if (movable && cfg_moving != CS_THING_MOVE_NEVER)
  {
    movtrans = movable->GetFullTransform ();
    can_move = true;
  }

  int i;
  int res = 1;

  // Calculate tesselation resolution
  csVector3 wv = curves_center;
  csVector3 world_coord;
  if (can_move)
    world_coord = movtrans.This2Other (wv);
  else
    world_coord = wv;

  csVector3 camera_coord = camtrans.Other2This (world_coord);

  if (camera_coord.z >= SMALL_Z)
  {
    res = (int)(curves_scale / camera_coord.z);
  }
  else
    res = 1000;                                 // some big tesselation value...

  // Create the combined transform of object to camera by
  // combining object to world and world to camera.
  csReversibleTransform obj_cam = camtrans;
  if (can_move) obj_cam /= movtrans;
  rview->GetGraphics3D ()->SetObjectToCamera (&obj_cam);
  rview->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zMode);

  // Base of the mesh.
  G3DTriangleMesh mesh;
  mesh.morph_factor = 0;
  mesh.num_vertices_pool = 1;
  mesh.do_mirror = icam->IsMirrored ();
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;

  iVertexBufferManager *vbufmgr = rview->GetGraphics3D ()
    ->GetVertexBufferManager ();

  // Loop over all curves
  csCurve *c;
  for (i = 0; i < GetCurveCount (); i++)
  {
    c = curves.Get (i);

    // First get a bounding box in camera space.
    csBox3 cbox;
    csBox2 sbox;
    if (c->GetScreenBoundingBox (obj_cam, icam, cbox, sbox) < 0)
      continue;                                 // Not visible.
    int clip_portal, clip_plane, clip_z_plane;
    if (!rview->ClipBBox (sbox, cbox, clip_portal, clip_plane, clip_z_plane))
      continue;                                 // Not visible.

    // If we have a dirty lightmap recombine the curves and the shadow maps.
    bool updated_lm = c->RecalculateDynamicLights ();

    // Create a new tesselation reuse an old one.
    csCurveTesselated *tess = c->Tesselate (res);

    // If the lightmap was updated or the new tesselation doesn't yet
    // have a valid colors table we need to update colors here.
    if (updated_lm || !tess->AreColorsValid ())
      tess->UpdateColors (c->LightMap);

    // Setup the structure for DrawTriangleMesh.
    if (tess->GetVertexCount () > fog_verts.Limit ())
    {
      fog_verts.SetLimit (tess->GetVertexCount ());
    }

    c->GetMaterial ()->Visit ();

    iVertexBuffer *vbuf = c->GetVertexBuffer ();

    mesh.mat_handle = c->GetMaterialHandle ();
    mesh.buffers[0] = vbuf;
    mesh.num_triangles = tess->GetTriangleCount ();
    mesh.triangles = tess->GetTriangles ();
    mesh.clip_portal = clip_portal;
    mesh.clip_plane = clip_plane;
    mesh.clip_z_plane = clip_z_plane;
    mesh.vertex_fog = fog_verts.GetArray ();

    bool gouraud = !!c->LightMap;
    mesh.mixmode = CS_FX_COPY | (gouraud ? CS_FX_GOURAUD : 0);
    mesh.use_vertex_color = gouraud;
    if (mesh.mat_handle == NULL)
    {
      // @@@ Use other Warn!!!
      Warn (thing_type->object_reg, "Warning! Curve without material!");
      continue;
    }

    CS_ASSERT (!vbuf->IsLocked ());
    vbufmgr->LockBuffer (
        vbuf,
        tess->GetVertices (),
        tess->GetTxtCoords (),
        tess->GetColors (),
        tess->GetVertexCount (),
        0);
    rview->CalculateFogMesh (obj_cam, mesh);
    rview->GetGraphics3D ()->DrawTriangleMesh (mesh);
    vbufmgr->UnlockBuffer (vbuf);
  }

  return true;                                  //@@@ RETURN correct vis info
}
#endif // CS_USE_NEW_RENDERER

void csThing::FireListeners ()
{
  int i;
  for (i = 0 ; i < listeners.Length () ; i++)
    listeners[i]->ObjectModelChanged (&scfiObjectModel);
}

void csThing::AddListener (iObjectModelListener *listener)
{
  RemoveListener (listener);
  listeners.Push (listener);
}

void csThing::RemoveListener (iObjectModelListener *listener)
{
  int idx = listeners.Find (listener);
  if (idx == -1) return ;
  listeners.Delete (idx);
}

bool csThing::DrawTest (iRenderView *rview, iMovable *movable)
{
  Prepare ();

  iCamera *icam = rview->GetCamera ();
  const csReversibleTransform &camtrans = icam->GetTransform ();

  // Only get the transformation if this thing can move.
  bool can_move = false;
  if (movable && cfg_moving != CS_THING_MOVE_NEVER)
  {
    can_move = true;
  }

  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

#if 1
  csBox3 b;
  GetBoundingBox (b);

  csSphere sphere;
  sphere.SetCenter (b.GetCenter ());
  sphere.SetRadius (max_obj_radius);
  if (can_move)
  {
    csReversibleTransform tr_o2c = camtrans;
    if (!movable->IsFullTransformIdentity ())
      tr_o2c /= movable->GetFullTransform ();
    bool rc = rview->TestBSphere (tr_o2c, sphere);
    return rc;
  }
  else
  {
    bool rc = rview->TestBSphere (camtrans, sphere);
    return rc;
  }

#else
  csBox3 b;
  GetBoundingBox (b);

  csVector3 center = b.GetCenter ();
  float maxradius = max_obj_radius;
  if (can_move) center = movable->GetFullTransform ().This2Other (center);
  center = camtrans.Other2This (center);
  if (center.z + maxradius < 0) return false;

  //-----
  // If the camera location (origin in camera space) is inside the
  // bounding sphere then the object is certainly visible.
  //-----
  if (
    ABS (center.x) <= maxradius &&
    ABS (center.y) <= maxradius &&
    ABS (center.z) <= maxradius)
    return true;

  //-----
  // Also do frustum checking.
  //-----
  float lx, rx, ty, by;
  rview->GetFrustum (lx, rx, ty, by);
  lx *= center.z;
  if (center.x + maxradius < lx) return false;
  rx *= center.z;
  if (center.x - maxradius > rx) return false;
  ty *= center.z;
  if (center.y + maxradius < ty) return false;
  by *= center.z;
  if (center.y - maxradius > by) return false;

  return true;
#endif
}

bool csThing::Draw (iRenderView *rview, iMovable *movable, csZBufMode zMode)
{
#ifndef CS_USE_NEW_RENDERER
  iCamera *icam = rview->GetCamera ();
  const csReversibleTransform &camtrans = icam->GetTransform ();

  draw_busy++;

  DrawCurves (rview, movable, zMode);

  if (flags.Check (CS_THING_FASTMESH))
    DrawPolygonArrayDPM (
      polygons.GetArray (),
      polygons.Length (),
      rview,
      movable,
      zMode);
  else
  {
    UpdateTransformation (camtrans, icam->GetCameraNumber ());
    DrawPolygonArray (
      polygons.GetArray (),
      polygons.Length (),
      rview,
      zMode);
  }

  draw_busy--;
#endif // CS_USE_NEW_RENDERER
  return true;                                  // @@@@ RETURN correct vis info
}

#ifndef CS_USE_NEW_RENDERER
bool csThing::DrawFoggy (iRenderView *d, iMovable *)
{
  draw_busy++;

  iCamera *icam = d->GetCamera ();
  const csReversibleTransform &camtrans = icam->GetTransform ();
  UpdateTransformation (camtrans, icam->GetCameraNumber ());

  csPolygon3D *p;
  csVector3 *verts;
  int num_verts;
  int i;
  csPoly2DPool *render_pool = thing_type->render_pol2d_pool;
  csPolygon2D *clip;

  // @@@ Wouldn't it be nice if we checked all vertices against the Z plane?
  {
    csVector2 orig_triangle[3];
    d->GetGraphics3D ()->OpenFogObject (GetID (), &GetFog ());

    icam->SetMirrored (!icam->IsMirrored ());
    for (i = 0; i < polygons.Length (); i++)
    {
      p = GetPolygon3D (i);
      if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
      clip = (csPolygon2D *) (render_pool->Alloc ());

      const csPlane3 &wplane = p->GetPlane ()->GetWorldPlane ();
      bool front = csMath3::Visible (camtrans.GetOrigin (), wplane);

      csPlane3 clip_plane, *pclip_plane;
      bool do_clip_plane = d->GetClipPlane (clip_plane);
      if (do_clip_plane)
        pclip_plane = &clip_plane;
      else
        pclip_plane = NULL;
      if (
        !front &&
        p->ClipToPlane (
            pclip_plane,
            camtrans.GetOrigin (),
            verts,
            num_verts,
            false) &&
        p->DoPerspective (
            camtrans,
            verts,
            num_verts,
            clip,
            orig_triangle,
            icam->IsMirrored ()) &&
        clip->ClipAgainst (d->GetClipper ()))
      {
        p->GetPlane ()->WorldToCamera (camtrans, verts[0]);

        clip->AddFogPolygon (
            d->GetGraphics3D (),
            p,
            p->GetPlane (),
            icam->IsMirrored (),
            GetID (),
            CS_FOG_BACK);
      }

      render_pool->Free (clip);
    }

    icam->SetMirrored (!icam->IsMirrored ());
    for (i = 0; i < polygons.Length (); i++)
    {
      p = GetPolygon3D (i);
      if (p->flags.Check (CS_POLY_NO_DRAW)) continue;
      clip = (csPolygon2D *) (render_pool->Alloc ());

      const csPlane3 &wplane = p->GetPlane ()->GetWorldPlane ();
      bool front = csMath3::Visible (camtrans.GetOrigin (), wplane);

      csPlane3 clip_plane, *pclip_plane;
      bool do_clip_plane = d->GetClipPlane (clip_plane);
      if (do_clip_plane)
        pclip_plane = &clip_plane;
      else
        pclip_plane = NULL;
      if (
        front &&
        p->ClipToPlane (
            pclip_plane,
            camtrans.GetOrigin (),
            verts,
            num_verts,
            true) &&
        p->DoPerspective (
            camtrans,
            verts,
            num_verts,
            clip,
            orig_triangle,
            icam->IsMirrored ()) &&
        clip->ClipAgainst (d->GetClipper ()))
      {
        p->GetPlane ()->WorldToCamera (camtrans, verts[0]);

        clip->AddFogPolygon (
            d->GetGraphics3D (),
            p,
            p->GetPlane (),
            icam->IsMirrored (),
            GetID (),
            CS_FOG_FRONT);
      }

      render_pool->Free (clip);
    }

    d->GetGraphics3D ()->CloseFogObject (GetID ());
  }

  draw_busy--;
  return true;                                  // @@@@ RETURN correct vis info
}
#endif // CS_USE_NEW_RENDERER

//----------------------------------------------------------------------

void csThing::CastShadows (iFrustumView *lview, iMovable *movable)
{
  //@@@ Ok?
  cached_movable = movable;
  WorUpdate ();

  int i;

  draw_busy++;

  for (i = 0; i < polygons.Length (); i++)
  {
    csPolygon3D* poly = GetPolygon3D (i);
    csLightingPolyTexQueue *lptq = (csLightingPolyTexQueue *)
      (lview->GetUserdata ());
    if (lptq->IsDynamic ())
      poly->CalculateLightingDynamic ((csFrustumView*)lview);
    else
      poly->CalculateLightingStatic ((csFrustumView*)lview, true);
  }

  for (i = 0; i < GetCurveCount (); i++)
  {
    csCurve* curve = curves.Get (i);
    csLightingPolyTexQueue *lptq = (csLightingPolyTexQueue *)
      (lview->GetUserdata ());
    if (lptq->IsDynamic ())
      curve->CalculateLightingDynamic ((csFrustumView*)lview);
    else
      curve->CalculateLightingStatic ((csFrustumView*)lview, true);
  }

  draw_busy--;
}

void csThing::InitializeDefault ()
{
  Prepare ();

  int i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i)->InitializeDefault ();
  for (i = 0; i < GetCurveCount (); i++)
    curves.Get (i)->InitializeDefaultLighting ();
}

bool csThing::ReadFromCache (iCacheManager* cache_mgr)
{
  Prepare ();
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  bool rc = false;
  csRef<iDataBuffer> db = cache_mgr->ReadCache ("thing_lm", NULL, ~0);
  if (db)
  {
    csMemFile mf ((const char*)(db->GetData ()), db->GetSize ());
    int i;
    for (i = 0; i < polygons.Length (); i++)
      if (!polygons.Get (i)->ReadFromCache (&mf)) goto stop;
    for (i = 0; i < GetCurveCount (); i++)
      if (!curves.Get (i)->ReadFromCache (&mf)) goto stop;
    rc = true;
  }

stop:
  cache_mgr->SetCurrentScope (NULL);
  return rc;
}

bool csThing::WriteToCache (iCacheManager* cache_mgr)
{
  char* cachename = GenerateCacheName ();
  cache_mgr->SetCurrentScope (cachename);
  delete[] cachename;

  int i;
  bool rc = false;
  csMemFile mf;
  for (i = 0; i < polygons.Length (); i++)
    if (!polygons.Get (i)->WriteToCache (&mf)) goto stop;
  for (i = 0; i < GetCurveCount (); i++)
    if (!curves.Get (i)->WriteToCache (&mf)) goto stop;
  if (!cache_mgr->CacheData ((void*)(mf.GetData ()), mf.GetSize (),
    	"thing_lm", NULL, ~0))
    goto stop;

  rc = true;

stop:
  cache_mgr->SetCurrentScope (NULL);
  return rc;
}

void csThing::PrepareLighting ()
{
  int i;
  for (i = 0; i < polygons.Length (); i++)
    polygons.Get (i)->PrepareLighting ();
  for (i = 0; i < GetCurveCount (); i++) curves.Get (i)->PrepareLighting ();
}

void csThing::Merge (csThing *other)
{
  int i, j;
  int *merge_vertices = new int[other->GetVertexCount () + 1];
  for (i = 0; i < other->GetVertexCount (); i++)
    merge_vertices[i] = AddVertex (other->Vwor (i));

  for (i = 0; i < other->polygons.Length (); i++)
  {
    csPolygon3D *p = other->GetPolygon3D (i);
    int *idx = p->GetVertices ().GetVertexIndices ();
    for (j = 0; j < p->GetVertices ().GetVertexCount (); j++)
      idx[j] = merge_vertices[idx[j]];
    AddPolygon (p);
    other->polygons[i] = NULL;
  }

  for (i = 0; i < other->GetCurveVertexCount (); i++)
    AddCurveVertex (other->GetCurveVertex (i), other->GetCurveTexel (i));

  for (i = 0; i < other->curves.Length (); i++)
  {
    csCurve *c = other->GetCurve (i);
    AddCurve (c);
    other->curves[i] = NULL;
  }

  delete[] merge_vertices;
}

void csThing::MergeTemplate (
  iThingState *tpl,
  iMaterialWrapper *default_material,
  csVector3 *shift,
  csMatrix3 *transform)
{
  int i, j;
  int *merge_vertices;

  flags.SetAll (tpl->GetFlags ().Get ());

  //@@@ OBSOLETE: SetZBufMode (tpl->GetZBufMode ());
  SetMovingOption (tpl->GetMovingOption ());

  //TODO should merge? take averages or something?
  curves_center = tpl->GetCurvesCenter ();
  curves_scale = tpl->GetCurvesScale ();

  //@@@ TEMPORARY
  csRef<iThingState> ith (SCF_QUERY_INTERFACE (tpl, iThingState));
  ParentTemplate = (csThing *) (ith->GetPrivateObject ());

  merge_vertices = new int[tpl->GetVertexCount () + 1];
  for (i = 0; i < tpl->GetVertexCount (); i++)
  {
    csVector3 v;
    v = tpl->GetVertex (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    merge_vertices[i] = AddVertex (v);
  }

  for (i = 0; i < tpl->GetPolygonCount (); i++)
  {
    iPolygon3D *pt = tpl->GetPolygon (i);
    csPolygon3D *p;
    iMaterialWrapper *mat = pt->GetMaterial ();
    p = NewPolygon (mat->GetPrivateObject ());  //@@@
    p->SetName (pt->QueryObject ()->GetName ());

    iMaterialWrapper *wrap = pt->GetMaterial ();
    if (!wrap && default_material)
      p->SetMaterial (default_material->GetPrivateObject ());

    int *idx = pt->GetVertexIndices ();
    for (j = 0; j < pt->GetVertexCount (); j++)
      p->AddVertex (merge_vertices[idx[j]]);

    p->flags.SetAll (pt->GetFlags ().Get ());
    p->CopyTextureType (pt);
  }

  for (i = 0; i < tpl->GetCurveVertexCount (); i++)
  {
    csVector3 v = tpl->GetCurveVertex (i);
    if (transform) v = *transform * v;
    if (shift) v += *shift;
    AddCurveVertex (v, tpl->GetCurveTexel (i));
  }

  for (i = 0; i < tpl->GetCurveCount (); i++)
  {
    iCurve *orig_curve = tpl->GetCurve (i);
    iCurve *p = CreateCurve (orig_curve->GetParentTemplate ());
    p->QueryObject ()->SetName (orig_curve->QueryObject ()->GetName ());
    if (orig_curve->GetMaterial ())
      p->SetMaterial (orig_curve->GetMaterial ());
    else
      p->SetMaterial (default_material);
  }

  delete[] merge_vertices;
}

void csThing::ReplaceMaterials (iMaterialList *matList, const char *prefix)
{
  int i;
  for (i = 0; i < GetPolygonCount (); i++)
  {
    csPolygon3D *p = GetPolygon3D (i);
    const char *txtname = p->GetMaterialWrapper ()->GetName ();
    char *newname = new char[strlen (prefix) + strlen (txtname) + 2];
    sprintf (newname, "%s_%s", prefix, txtname);

    iMaterialWrapper *th = matList->FindByName (newname);
    if (th != NULL) p->SetMaterial (th->GetPrivateObject ());
    delete[] newname;
  }
}

void csThing::AddPortalPolygon (csPolygon3D *poly)
{
  int idx = portal_polygons.Find (poly);

  //@@@???CS_ASSERT (idx == -1);
  if (idx != -1) return ;

  CS_ASSERT (poly->GetPortal () != NULL);
  CS_ASSERT (poly->GetParent () == this);
  portal_polygons.Push (poly);
}

void csThing::RemovePortalPolygon (csPolygon3D *poly)
{
  int idx = portal_polygons.Find (poly);

  //@@@???CS_ASSERT (idx != -1);
  if (idx == -1) return ;

  CS_ASSERT (poly->GetPortal () == NULL || poly->GetParent () != this);
  portal_polygons.Delete (idx);
}

//---------------------------------------------------------------------------
iCurve *csThing::ThingState::GetCurve (int idx) const
{
  csCurve *c = scfParent->GetCurve (idx);
  return &(c->scfiCurve);
}

iPolygon3D *csThing::ThingState::GetPolygon (int idx)
{
  csPolygon3D *p = scfParent->GetPolygon3D (idx);
  if (!p) return NULL;
  return &(p->scfiPolygon3D);
}

iPolygon3D *csThing::ThingState::GetPolygon (const char *name)
{
  csPolygon3D *p = scfParent->GetPolygon3D (name);
  if (!p) return NULL;
  return &(p->scfiPolygon3D);
}

iPolygon3D *csThing::ThingState::CreatePolygon (const char *iName)
{
  csPolygon3D *p = new csPolygon3D ((csMaterialWrapper *)NULL);
  if (iName) p->SetName (iName);
  scfParent->AddPolygon (p);

  csRef<iPolygon3D> ip (SCF_QUERY_INTERFACE (p, iPolygon3D));
  return ip;	// DecRef is ok here.
}

int csThing::ThingState::GetPortalCount () const
{
  return scfParent->portal_polygons.Length ();
}

iPortal *csThing::ThingState::GetPortal (int idx) const
{
  csPolygon3D *p = (csPolygon3D *) (scfParent->portal_polygons)[idx];
  return &(p->GetPortal ()->scfiPortal);
}

iPolygon3D *csThing::ThingState::GetPortalPolygon (int idx) const
{
  csPolygon3D *p = (csPolygon3D *) (scfParent->portal_polygons)[idx];
  return &(p->scfiPolygon3D);
}

iPolygon3D* csThing::ThingState::IntersectSegment (const csVector3& start,
	const csVector3& end, csVector3& isect,
	float* pr, bool only_portals)
{
  csPolygon3D* p = scfParent->IntersectSegment (start, end, isect, pr,
  	only_portals);
  return p ? &(p->scfiPolygon3D) : NULL;
}

//---------------------------------------------------------------------------
iMeshObjectFactory *csThing::MeshObject::GetFactory () const
{
  if (!scfParent->ParentTemplate) return NULL;
  return &scfParent->ParentTemplate->scfiMeshObjectFactory;
}

//---------------------------------------------------------------------------
csPtr<iMeshObject> csThing::MeshObjectFactory::NewInstance ()
{
  csThing *thing = new csThing (scfParent, scfParent->thing_type);
  thing->MergeTemplate (&(scfParent->scfiThingState), NULL);
  return csPtr<iMeshObject> (&thing->scfiMeshObject);
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE(csThingObjectType)
  SCF_IMPLEMENTS_INTERFACE(iMeshObjectType)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iThingEnvironment)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingObjectType::eiThingEnvironment)
  SCF_IMPLEMENTS_INTERFACE(iThingEnvironment)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingObjectType)

csThingObjectType::csThingObjectType (
  iBase *pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiThingEnvironment);
  lightpatch_pool = NULL;
  render_pol2d_pool = NULL;
}

csThingObjectType::~csThingObjectType ()
{
  ClearPolyTxtPlanes ();
  ClearCurveTemplates ();
  delete render_pol2d_pool;
  delete lightpatch_pool;
}

bool csThingObjectType::Initialize (iObjectRegistry *object_reg)
{
  csThingObjectType::object_reg = object_reg;
  csRef<iEngine> e = CS_QUERY_REGISTRY (object_reg, iEngine);
  engine = e;	// We don't want a real ref here to avoid circular refs.
  csRef<iGraphics3D> g = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  G3D = g;

  lightpatch_pool = new csLightPatchPool ();
  render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory ());
  return true;
}

void csThingObjectType::Clear ()
{
  ClearPolyTxtPlanes ();
  ClearCurveTemplates ();
  delete lightpatch_pool;
  delete render_pol2d_pool;
  lightpatch_pool = new csLightPatchPool ();
  render_pol2d_pool = new csPoly2DPool (csPolygon2DFactory::SharedFactory ());
}

csPtr<iMeshObjectFactory> csThingObjectType::NewFactory ()
{
  csThing *cm = new csThing (this, this);
  csRef<iMeshObjectFactory> ifact (SCF_QUERY_INTERFACE (
      cm, iMeshObjectFactory));
  cm->DecRef ();
  return csPtr<iMeshObjectFactory> (ifact);
}

csPtr<iPolyTxtPlane> csThingObjectType::CreatePolyTxtPlane (const char *name)
{
  csPolyTxtPlane *pl = new csPolyTxtPlane ();
  planes.Push (pl);
  if (name) pl->SetName (name);
  pl->IncRef ();
  return &(pl->scfiPolyTxtPlane);
}

iPolyTxtPlane *csThingObjectType::FindPolyTxtPlane (const char *iName)
{
  csPolyTxtPlane *pl;
  pl = (csPolyTxtPlane *)planes.FindByName (iName);
  if (!pl) return NULL;
  return &(pl->scfiPolyTxtPlane);
}

csPtr<iCurveTemplate> csThingObjectType::CreateBezierTemplate (const char *name)
{
  csBezierTemplate *ptemplate = new csBezierTemplate (this);
  if (name) ptemplate->SetName (name);
  curve_templates.Push (ptemplate);
  return SCF_QUERY_INTERFACE (ptemplate, iCurveTemplate);
}

iCurveTemplate *csThingObjectType::FindCurveTemplate (const char *iName)
{
  csCurveTemplate *pl;
  pl = (csCurveTemplate *)curve_templates.FindByName (iName);
  if (!pl) return NULL;

  csRef<iCurveTemplate> itmpl (SCF_QUERY_INTERFACE (pl, iCurveTemplate));
  return itmpl;	// DecRef is ok here.
}

void csThingObjectType::RemovePolyTxtPlane (iPolyTxtPlane *pl)
{
  int i;
  for (i = 0; i < planes.Length (); i++)
  {
    csPolyTxtPlane *pli = (csPolyTxtPlane *)planes[i];
    if (pl == &(pli->scfiPolyTxtPlane))
    {
      planes.Delete (i);
      return ;
    }
  }
}

void csThingObjectType::RemoveCurveTemplate (iCurveTemplate *ct)
{
  int i;
  for (i = 0; i < curve_templates.Length (); i++)
  {
    csCurveTemplate *cti = (csCurveTemplate *)curve_templates[i];
    csRef<iCurveTemplate> i_cti (SCF_QUERY_INTERFACE (cti, iCurveTemplate));
    if (ct == i_cti)
    {
      curve_templates.Delete (i);
      return ;
    }
  }
}

void csThingObjectType::ClearPolyTxtPlanes ()
{
  int i;
  for (i = 0; i < planes.Length (); i++)
  {
    csPolyTxtPlane *p = (csPolyTxtPlane *)planes[i];
    planes[i] = NULL;
    p->DecRef ();
  }

  planes.DeleteAll ();
}

void csThingObjectType::ClearCurveTemplates ()
{
  curve_templates.DeleteAll ();
}

//---------------------------------------------------------------------------
