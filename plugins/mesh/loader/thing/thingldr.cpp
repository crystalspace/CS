/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "thingldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "iengine/thing.h"//@@@ (should be from imesh)
#include "iengine/polygon.h"//@@@ (should be from imesh)
#include "iengine/portal.h"	//@@@ (imesh)
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iobject/object.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iengine/polytmap.h"
#include "iengine/ptextype.h"
#include "iengine/curve.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (BEZIER)
  CS_TOKEN_DEF (CAMERA)
  CS_TOKEN_DEF (CIRCLE)
  CS_TOKEN_DEF (CLIP)
  CS_TOKEN_DEF (CLONE)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (COLLDET)
  CS_TOKEN_DEF (CONVEX)
  CS_TOKEN_DEF (COSFACT)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (CURVE)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (CURVESCALE)
  CS_TOKEN_DEF (DETAIL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FLAT)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (GOURAUD)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (LEN)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (LIGHTMAP)
  CS_TOKEN_DEF (MAT_SET_SELECT)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MIRROR)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (MOVEABLE)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (NAME)
  CS_TOKEN_DEF (NONE)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (PART)
  CS_TOKEN_DEF (PLANE)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (PORTAL)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SECOND_LEN)
  CS_TOKEN_DEF (SKYDOME)
  CS_TOKEN_DEF (SHADING)
  CS_TOKEN_DEF (STATIC)
  CS_TOKEN_DEF (TEMPLATE)
  CS_TOKEN_DEF (TEXLEN)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (UVA)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (UV_SHIFT)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VISTREE)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (WARP)
  CS_TOKEN_DEF (ZFILL)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csThingLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csThingSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csPlaneLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csPlaneSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csBezierLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csBezierSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csThingLoader)
IMPLEMENT_FACTORY (csThingSaver)
IMPLEMENT_FACTORY (csPlaneLoader)
IMPLEMENT_FACTORY (csPlaneSaver)
IMPLEMENT_FACTORY (csBezierLoader)
IMPLEMENT_FACTORY (csBezierSaver)

EXPORT_CLASS_TABLE (thingldr)
  EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.factory.thing",
    "Crystal Space Thing Mesh Factory Loader")
  EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.factory.thing",
    "Crystal Space Thing Mesh Factory Saver")
  EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.thing",
    "Crystal Space Thing Mesh Loader")
  EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.thing",
    "Crystal Space Thing Mesh Saver")
  EXPORT_CLASS (csPlaneLoader, "crystalspace.mesh.loader.thing.plane",
    "Crystal Space Thing Plane Loader")
  EXPORT_CLASS (csPlaneSaver, "crystalspace.mesh.saver.thing.plane",
    "Crystal Space Thing Plane Saver")
  EXPORT_CLASS (csBezierLoader, "crystalspace.mesh.loader.thing.bezier",
    "Crystal Space Thing Bezier Loader")
  EXPORT_CLASS (csBezierSaver, "crystalspace.mesh.saver.thing.bezier",
    "Crystal Space Thing Bezier Saver")
EXPORT_CLASS_TABLE_END

#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csThingLoader::~csThingLoader ()
{
}

bool csThingLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

class ThingLoadInfo
{
public:
  iMaterialWrapper* default_material;
  float default_texlen;
  bool use_mat_set;
  char* mat_set_name;
  
  ThingLoadInfo () : default_material (NULL),
    default_texlen (1),
    use_mat_set (false), mat_set_name (NULL)
    {}

  void SetTextureSet (const char* name)
  {
    delete [] mat_set_name;
    mat_set_name = new char [strlen (name) + 1];
    strcpy (mat_set_name, name);
  }   
};

static bool load_matrix (char* buf, csMatrix3 &m)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (IDENTITY)
    CS_TOKEN_TABLE (ROT_X)
    CS_TOKEN_TABLE (ROT_Y)
    CS_TOKEN_TABLE (ROT_Z)
    CS_TOKEN_TABLE (ROT)
    CS_TOKEN_TABLE (SCALE_X)
    CS_TOKEN_TABLE (SCALE_Y)
    CS_TOKEN_TABLE (SCALE_Z)
    CS_TOKEN_TABLE (SCALE)
  CS_TOKEN_TABLE_END

  char* params;
  int cmd, num;
  float angle;
  float scaler;
  float list[30];
  const csMatrix3 identity;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_IDENTITY:
        m = identity;
        break;
      case CS_TOKEN_ROT_X:
        ScanStr (params, "%f", &angle);
        m *= csXRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Y:
        ScanStr (params, "%f", &angle);
        m *= csYRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Z:
        ScanStr (params, "%f", &angle);
        m *= csZRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT:
        ScanStr (params, "%F", list, &num);
        if (num == 3)
        {
          m *= csXRotMatrix3 (list[0]);
          m *= csZRotMatrix3 (list[2]);
          m *= csYRotMatrix3 (list[1]);
        }
        else
	  printf ("Badly formed rotation: '%s'\n", params);
        break;
      case CS_TOKEN_SCALE_X:
        ScanStr (params, "%f", &scaler);
        m *= csXScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Y:
        ScanStr (params, "%f", &scaler);
        m *= csYScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Z:
        ScanStr (params, "%f", &scaler);
        m *= csZScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE:
        ScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	  m *= list[0];
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	  m *= csMatrix3 (list[0],0,0,0,list[1],0,0,0,list[2]);
        else
	  printf ("Badly formed scale: '%s'\n", params);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    // Neither SCALE, ROT, nor IDENTITY, so matrix may contain a single scaler
    // or the nine values of a 3x3 matrix.
    ScanStr (buf, "%F", list, &num);
    if (num == 1)
      m = csMatrix3 () * list[0];
    else if (num == 9)
      m = csMatrix3 (
        list[0], list[1], list[2],
        list[3], list[4], list[5],
        list[6], list[7], list[8]);
    else
      printf ("Badly formed matrix '%s'\n", buf);
  }
  return true;
}

static bool load_vector (char* buf, csVector3 &v)
{
  ScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

static void OptimizePolygon (iPolygon3D *p)
{
  if (!p->GetPortal () || p->GetAlpha ())
    return;

  iMaterialWrapper *mat = p->GetMaterial ();
  if (mat)
  {
    iMaterial *m = mat->GetMaterial ();
    iTextureHandle *th = m ? m->GetTexture () : NULL;
    if (th && th->GetKeyColor ())
      return;
  }

  p->SetTextureType (POLYTXT_NONE);
}

static UInt ParseMixmode (char* buf)
{
  CS_TOKEN_TABLE_START (modes)
    CS_TOKEN_TABLE (COPY)
    CS_TOKEN_TABLE (MULTIPLY2)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (ADD)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (KEYCOLOR)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  UInt Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
      case CS_TOKEN_COPY: Mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: Mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: Mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: Mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	Mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
	int ialpha;
        ScanStr (params, "%f", &alpha);
	ialpha = QInt (alpha * 255.99);
	Mixmode |= CS_FX_SETALPHA(ialpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing the modes!\n",
    	csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

static bool skydome_process (iThingState* thing_state, char* name, char* buf,
        iMaterialWrapper* material, int vt_offset)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (LIGHTING)
  CS_TOKEN_TABLE_END

  long cmd;
  char* params;
  float radius = 0.0f;
  int i, j;
  int num = 0;
  iPolyTexType* ptt;
  iPolyTexGouraud* gs;
  iPolyTexFlat* fs;

  // Previous vertices.
  int prev_vertices[60];        // @@@ HARDCODED!
  float prev_u[60];
  float prev_v[60];

  char poly_name[30], * end_poly_name;
  strcpy (poly_name, name);
  end_poly_name = strchr (poly_name, 0);
  int lighting_flags = CS_POLY_LIGHTING;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_RADIUS:
        ScanStr (params, "%f", &radius);
        break;
      case CS_TOKEN_VERTICES:
        ScanStr (params, "%D", prev_vertices, &num);
	for (i = 0 ; i < num ; i++)
	  prev_vertices[i] += vt_offset;
        break;
      case CS_TOKEN_LIGHTING:
        {
	  int do_lighting;
          ScanStr (params, "%b", &do_lighting);
	  if (do_lighting) lighting_flags = CS_POLY_LIGHTING;
	  else lighting_flags = 0;
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a skydome!\n",
    	csGetLastOffender ());
    return false;
  }

  csMatrix3 t_m;
  csVector3 t_v (0, 0, 0);

  // If radius is negative we have an up-side-down skydome.
  float vert_radius = radius;
  if (radius < 0) radius = -radius;

  // Number of degrees between layers.
  float radius_step = 180. / num;

  // Calculate u,v for the first series of vertices (the outer circle).
  for (j = 0 ; j < num ; j++)
  {
    float angle = 2.*radius_step*j * 2.*M_PI/360.;
    if (vert_radius < 0) angle = 2.*M_PI-angle;
    prev_u[j] = cos (angle) * .5 + .5;
    prev_v[j] = sin (angle) * .5 + .5;
  }

  // Array with new vertex indices.
  int new_vertices[60];         // @@@ HARDCODED == BAD == EASY!
  float new_u[60];
  float new_v[60];

  // First create the layered triangle strips.
  for (i = 1 ; i < num/2 ; i++)
  {
    //-----
    // First create a new series of vertices.
    //-----
    // Angle from the center to the new circle of vertices.
    float new_angle = i*radius_step * 2.*M_PI/360.;
    // Radius of the new circle of vertices.
    float new_radius = radius * cos (new_angle);
    // Height of the new circle of vertices.
    float new_height = vert_radius * sin (new_angle);
    // UV radius.
    float uv_radius = (1. - 2.*(float)i/(float)num) * .5;
    for (j = 0 ; j < num ; j++)
    {
      float angle = j*2.*radius_step * 2.*M_PI/360.;
      if (vert_radius < 0) angle = 2.*M_PI-angle;
      new_vertices[j] = thing_state->CreateVertex (
      	csVector3 (
                         new_radius * cos (angle),
                         new_height,
                         new_radius * sin (angle)));
      new_u[j] = uv_radius * cos (angle) + .5;
      new_v[j] = uv_radius * sin (angle) + .5;
    }

    //-----
    // Now make the triangle strips.
    //-----
    for (j = 0 ; j < num ; j++)
    {
      sprintf (end_poly_name, "%d_%d_A", i, j);
      iPolygon3D* p = thing_state->CreatePolygon (poly_name);
      p->SetMaterial (material);
      p->GetFlags ().Set (CS_POLY_LIGHTING, lighting_flags);
      p->SetCosinusFactor (1);
      p->CreateVertex (prev_vertices[j]);
      p->CreateVertex (new_vertices[(j+1)%num]);
      p->CreateVertex (new_vertices[j]);
      p->SetTextureType (POLYTXT_GOURAUD);
      ptt = p->GetPolyTexType ();
      gs = QUERY_INTERFACE (ptt, iPolyTexGouraud);
      fs = QUERY_INTERFACE (ptt, iPolyTexFlat);
      gs->Setup (p);
      fs->SetUV (0, prev_u[j], prev_v[j]);
      fs->SetUV (1, new_u[(j+1)%num], new_v[(j+1)%num]);
      fs->SetUV (2, new_u[j], new_v[j]);
      fs->DecRef ();
      gs->DecRef ();

      p->SetTextureSpace (t_m, t_v);

      sprintf (end_poly_name, "%d_%d_B", i, j);
      p = thing_state->CreatePolygon (poly_name);
      p->SetMaterial (material);
      p->GetFlags ().Set (CS_POLY_LIGHTING, lighting_flags);
      p->SetCosinusFactor (1);
      p->CreateVertex (prev_vertices[j]);
      p->CreateVertex (prev_vertices[(j+1)%num]);
      p->CreateVertex (new_vertices[(j+1)%num]);
      p->SetTextureType (POLYTXT_GOURAUD);
      ptt = p->GetPolyTexType ();
      gs = QUERY_INTERFACE (ptt, iPolyTexGouraud);
      fs = QUERY_INTERFACE (ptt, iPolyTexFlat);
      gs->Setup (p);
      fs->SetUV (0, prev_u[j], prev_v[j]);
      fs->SetUV (1, prev_u[(j+1)%num], prev_v[(j+1)%num]);
      fs->SetUV (2, new_u[(j+1)%num], new_v[(j+1)%num]);
      fs->DecRef ();
      gs->DecRef ();
      p->SetTextureSpace (t_m, t_v);
    }

    //-----
    // Copy the new vertex array to prev_vertices.
    //-----
    for (j = 0 ; j < num ; j++)
    {
      prev_vertices[j] = new_vertices[j];
      prev_u[j] = new_u[j];
      prev_v[j] = new_v[j];
    }
  }

  // Create the top vertex.
  int top_vertex = thing_state->CreateVertex (csVector3 (0, vert_radius, 0));
  float top_u = .5;
  float top_v = .5;

  //-----
  // Make the top triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    sprintf (end_poly_name, "%d_%d", num/2, j);
    iPolygon3D* p = thing_state->CreatePolygon (poly_name);
    p->SetMaterial (material);
    p->GetFlags ().Set (CS_POLY_LIGHTING, lighting_flags);
    p->SetCosinusFactor (1);
    p->CreateVertex (top_vertex);
    p->CreateVertex (prev_vertices[j]);
    p->CreateVertex (prev_vertices[(j+1)%num]);
    p->SetTextureType (POLYTXT_GOURAUD);
    ptt = p->GetPolyTexType ();
    gs = QUERY_INTERFACE (ptt, iPolyTexGouraud);
    fs = QUERY_INTERFACE (ptt, iPolyTexFlat);
    gs->Setup (p);
    fs->SetUV (0, top_u, top_v);
    fs->SetUV (1, prev_u[j], prev_v[j]);
    fs->SetUV (2, prev_u[(j+1)%num], prev_v[(j+1)%num]);
    fs->DecRef ();
    gs->DecRef ();
    p->SetTextureSpace (t_m, t_v);
  }
  return true;
}

static iPolygon3D* load_poly3d (iEngine* engine, char* polyname, char* buf,
  iMaterialWrapper* default_material, float default_texlen,
  iThingState* thing_state, int vt_offset)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (PORTAL)
    CS_TOKEN_TABLE (WARP)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (SHADING)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (UVA)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (COLLDET)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (COSFACT)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tex_commands)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (FIRST_LEN)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (SECOND_LEN)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (LEN)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (PLANE)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (UV_SHIFT)
    CS_TOKEN_TABLE (UV)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (portal_commands)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (W)
    CS_TOKEN_TABLE (MIRROR)
    CS_TOKEN_TABLE (STATIC)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (CLIP)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (texturing_commands)
    CS_TOKEN_TABLE (NONE)
    CS_TOKEN_TABLE (FLAT)
    CS_TOKEN_TABLE (GOURAUD)
    CS_TOKEN_TABLE (LIGHTMAP)
  CS_TOKEN_TABLE_END

  char* name;
  int i;
  long cmd;
  char* params, * params2;

  iPolygon3D* poly3d = thing_state->CreatePolygon (polyname);
  poly3d->SetMaterial (default_material);

  iMaterialWrapper* mat = NULL;

  bool tx_uv_given = false;
  int tx_uv_i1 = 0;
  int tx_uv_i2 = 0;
  int tx_uv_i3 = 0;
  csVector2 tx_uv1;
  csVector2 tx_uv2;
  csVector2 tx_uv3;

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;

  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char plane_name[30];
  plane_name[0] = 0;
  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  bool do_mirror = false;
  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = engine->FindMaterial (str/*@@@, onlyRegion*/); //@@@ REGION SUPPORT?
        if (mat == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return NULL;
        }
        poly3d->SetMaterial (mat);
        break;
      case CS_TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          poly3d->GetFlags ().Set (CS_POLY_LIGHTING,
	  	do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case CS_TOKEN_COSFACT:
        {
          float cosfact;
          ScanStr (params, "%f", &cosfact);
          poly3d->SetCosinusFactor (cosfact);
        }
        break;
      case CS_TOKEN_ALPHA:
        {
          int alpha;
          ScanStr (params, "%d", &alpha);
          poly3d->SetAlpha (alpha * 655 / 256);
        }
        break;
      case CS_TOKEN_COLLDET:
        {
          int do_colldet;
          ScanStr (params, "%b", &do_colldet);
	  if (do_colldet) set_colldet = 1;
	  else set_colldet = -1;
        }
        break;
      case CS_TOKEN_PORTAL:
        {
          ScanStr (params, "%s", str);
          iSector* s = engine->CreateSector (str, false);
          poly3d->CreatePortal (s);
        }
        break;
      case CS_TOKEN_WARP:
        if (poly3d->GetPortal ())
        {
          csMatrix3 m_w; m_w.Identity ();
          csVector3 v_w_before (0, 0, 0);
          csVector3 v_w_after (0, 0, 0);
          while ((cmd = csGetObject (&params, portal_commands,
	  	&name, &params2)) > 0)
          {
    	    if (!params2)
    	    {
      	      printf ("Expected parameters instead of '%s'!\n", params);
      	      return NULL;
    	    }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
                load_matrix (params2, m_w);
                do_mirror = false;
                break;
              case CS_TOKEN_V:
                load_vector (params2, v_w_before);
                v_w_after = v_w_before;
                do_mirror = false;
                break;
              case CS_TOKEN_W:
                load_vector (params2, v_w_after);
                do_mirror = false;
                break;
              case CS_TOKEN_MIRROR:
                do_mirror = true;
		if (!set_colldet) set_colldet = 1;
                break;
              case CS_TOKEN_STATIC:
                poly3d->GetPortal ()->GetFlags ().Set (CS_PORTAL_STATICDEST);
                break;
      	      case CS_TOKEN_ZFILL:
		poly3d->GetPortal ()->GetFlags ().Set (CS_PORTAL_ZFILL);
        	break;
      	      case CS_TOKEN_CLIP:
		poly3d->GetPortal ()->GetFlags ().Set (CS_PORTAL_CLIPDEST);
        	break;
            }
          }
          if (!do_mirror)
            poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);
        }
        break;
      case CS_TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
    	  if (!params2)
    	  {
      	    printf ("Expected parameters instead of '%s'!\n", params);
      	    return NULL;
	  }
          switch (cmd)
          {
            case CS_TOKEN_ORIG:
	      tx_uv_given = false;
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx_orig = thing_state->GetVertex ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST:
	      tx_uv_given = false;
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = thing_state->GetVertex ((int)flist[0]);
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST_LEN:
	      tx_uv_given = false;
              ScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case CS_TOKEN_SECOND:
	      tx_uv_given = false;
              tx2_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = thing_state->GetVertex ((int)flist[0]);
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_SECOND_LEN:
	      tx_uv_given = false;
              ScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case CS_TOKEN_LEN:
	      tx_uv_given = false;
              ScanStr (params2, "%f", &tx_len);
              break;
            case CS_TOKEN_MATRIX:
	      tx_uv_given = false;
              load_matrix (params2, tx_matrix);
              tx_len = 0;
              break;
            case CS_TOKEN_V:
	      tx_uv_given = false;
              load_vector (params2, tx_vector);
              tx_len = 0;
              break;
            case CS_TOKEN_PLANE:
	      tx_uv_given = false;
              ScanStr (params2, "%s", str);
              strcpy (plane_name, str);
              tx_len = 0;
              break;
            case CS_TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
            case CS_TOKEN_UVEC:
              tx1_given = true;
	      tx_uv_given = false;
              load_vector (params2, tx1);
              tx1_len = tx1.Norm ();
              tx1 += tx_orig;
              break;
            case CS_TOKEN_VVEC:
              tx2_given = true;
	      tx_uv_given = false;
              load_vector (params2, tx2);
              tx2_len = tx2.Norm ();
              tx2 += tx_orig;
              break;
	    case CS_TOKEN_UV:
	      {
		float u1, v1, u2, v2, u3, v3;
		tx_uv_given = true;
                ScanStr (params2, "%d,%f,%f,%d,%f,%f,%d,%f,%f",
			&tx_uv_i1, &u1, &v1,
			&tx_uv_i2, &u2, &v2,
			&tx_uv_i3, &u3, &v3);
		tx_uv1.Set (u1, v1);
		tx_uv2.Set (u2, v2);
		tx_uv3.Set (u3, v3);
              }
	      break;
	  }
        }
        break;
      case CS_TOKEN_VERTICES:
        {
          int list[100], num;
          ScanStr (params, "%D", list, &num);
          for (i = 0 ; i < num ; i++)
	  {
	    if (list[i] == list[(i-1+num)%num])
	      printf ("Duplicate vertex-index found in polygon! Ignored...\n");
	    else
	      poly3d->CreateVertex (list[i]+vt_offset);
	  }
        }
        break;
      case CS_TOKEN_SHADING:
        while ((cmd = csGetObject (&params, texturing_commands,
		&name, &params2)) > 0)
          switch (cmd)
          {
            case CS_TOKEN_NONE:
              poly3d->SetTextureType (POLYTXT_NONE);
              break;
            case CS_TOKEN_FLAT:
              poly3d->SetTextureType (POLYTXT_FLAT);
              break;
            case CS_TOKEN_GOURAUD:
              poly3d->SetTextureType (POLYTXT_GOURAUD);
              break;
            case CS_TOKEN_LIGHTMAP:
              poly3d->SetTextureType (POLYTXT_LIGHTMAP);
              break;
          }
        break;
      case CS_TOKEN_MIXMODE:
        {
          UInt mixmode = ParseMixmode (params);
	  iPolyTexType* ptt = poly3d->GetPolyTexType ();
          iPolyTexNone* notex = QUERY_INTERFACE (ptt, iPolyTexNone);
	  if (notex)
	  {
	    notex->SetMixmode (mixmode);
	    notex->DecRef ();
	  }
          if (mixmode & CS_FX_MASK_ALPHA)
            poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
          break;
	}
      case CS_TOKEN_UV:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  iPolyTexType* ptt = poly3d->GetPolyTexType ();
	  iPolyTexFlat* fs = QUERY_INTERFACE (ptt, iPolyTexFlat);
          int num, nv = poly3d->GetVertexCount ();
	  fs->Setup (poly3d);
          float list [2 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
            fs->SetUV (j, list [j * 2], list [j * 2 + 1]);
	  fs->DecRef ();
        }
        break;
      case CS_TOKEN_COLORS:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  iPolyTexType* ptt = poly3d->GetPolyTexType ();
	  iPolyTexGouraud* gs = QUERY_INTERFACE (ptt, iPolyTexGouraud);
          int num, nv = poly3d->GetVertexCount ();
	  gs->Setup (poly3d);
          float list [3 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
            gs->SetColor (j, csColor (list [j * 3], list [j * 3 + 1],
	    	list [j * 3 + 2]));
	  gs->DecRef ();
        }
        break;
      case CS_TOKEN_UVA:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  iPolyTexType* ptt = poly3d->GetPolyTexType ();
	  iPolyTexFlat* fs = QUERY_INTERFACE (ptt, iPolyTexFlat);
          int num, nv = poly3d->GetVertexCount ();
	  fs->Setup (poly3d);
          float list [3 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
          {
            float a = list [j * 3] * 2 * M_PI / 360.;
            fs->SetUV (j, cos (a) * list [j * 3 + 1] + list [j * 3 + 2],
                          sin (a) * list [j * 3 + 1] + list [j * 3 + 2]);
          }
	  fs->DecRef ();
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a polygon!\n",
      csGetLastOffender ());
    return NULL;
  }

  if (poly3d->GetVertexCount () < 3)
  {
    printf ("Polygon in line %d contains just %d vertices!\n",
      csGetParserLine (), poly3d->GetVertexCount ());
    return NULL;
  }

  if (set_colldet == 1)
    poly3d->GetFlags ().Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->GetFlags ().Reset (CS_POLY_COLLDET);

  if (tx_uv_given)
  {
    poly3d->SetTextureSpace (
    	poly3d->GetVertex (tx_uv_i1), tx_uv1,
	poly3d->GetVertex (tx_uv_i2), tx_uv2,
	poly3d->GetVertex (tx_uv_i3), tx_uv3);
  }
  else if (tx1_given)
    if (tx2_given)
    {
      if (!tx1_len)
      {
        printf ("Bad texture specification for POLYGON '%s'\n", polyname);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
        printf ("Bad texture specification for POLYGON '%s'\n", polyname);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else poly3d->SetTextureSpace (tx_orig, tx1, tx1_len, tx2, tx2_len);
    }
  else
  {
    if (!tx1_len)
    {
      printf ("Bad texture specification for POLYGON '%s'\n", polyname);
      tx1_len = 1;
    }
    if ((tx1-tx_orig) < SMALL_EPSILON)
      printf ("Bad texture specification for PLANE '%s'\n", name);
    else poly3d->SetTextureSpace (tx_orig, tx1, tx1_len);
  }
  else if (plane_name[0])
    poly3d->SetTextureSpace (engine->FindPolyTxtPlane (plane_name));
  else if (tx_len)
  {
    // If a length is given (with 'LEN') we will take the first two vertices
    // and calculate the texture orientation from them (with the given
    // length).
    poly3d->SetTextureSpace (poly3d->GetVertex (0), poly3d->GetVertex (1),
    	tx_len);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  iPolyTexType* ptt = poly3d->GetPolyTexType ();
  iPolyTexLightMap* plm = QUERY_INTERFACE (ptt, iPolyTexLightMap);
  if (uv_shift_given && plm)
  {
    plm->GetPolyTxtPlane ()->GetTextureSpace (tx_matrix, tx_vector);
    // T = Mot * (O - Vot)
    // T = Mot * (O - Vot) + Vuv      ; Add shift Vuv to final texture map
    // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
    // T = Mot * (O - Vot + Mot-1 * Vuv)
    csVector3 shift (u_shift, v_shift, 0);
    tx_vector -= tx_matrix.GetInverse () * shift;
    poly3d->SetTextureSpace (tx_matrix, tx_vector);
    plm->DecRef ();
  }

  if (do_mirror)
    poly3d->GetPortal ()->SetWarp (csTransform::GetReflect (
    	poly3d->GetWorldPlane () ));

  OptimizePolygon (poly3d);

  return poly3d;
}

static bool load_thing_part (ThingLoadInfo& info, iMeshWrapper* imeshwrap,
	iEngine* engine,
	iThingState* thing_state, char* buf, int vt_offset, bool isParent)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (CIRCLE)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVESCALE)
    CS_TOKEN_TABLE (CURVECONTROL)
    CS_TOKEN_TABLE (CURVE)
    CS_TOKEN_TABLE (MAT_SET_SELECT)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (TEXLEN)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (CONVEX)
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (CLONE)
    CS_TOKEN_TABLE (CAMERA)
    CS_TOKEN_TABLE (VISTREE)
    CS_TOKEN_TABLE (SKYDOME)
    CS_TOKEN_TABLE (PART)
    CS_TOKEN_TABLE (FACTORY)
  CS_TOKEN_TABLE_END

  char* name = NULL;
  char* xname;
  long cmd;
  char* params;
  char str[255];

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_VISTREE:
        if (!isParent)
	{
	  printf ("VISTREE flag only for top-level thing!\n");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_VISTREE);
        break;
      case CS_TOKEN_MOVEABLE:
        if (!isParent)
	{
	  printf ("MOVEABLE flag only for top-level thing!\n");
	  return false;
	}
        else thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
        break;
      case CS_TOKEN_TEMPLATE:
      case CS_TOKEN_FACTORY:
        if (!isParent)
	{
	  printf ("FACTORY or TEMPLATE statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
          if (!fact)
          {
            printf ("Couldn't find thing factory '%s'!\n", str);
            return false;
          }
	  iThingState* tmpl_thing_state = QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iThingState);
	  if (cmd == CS_TOKEN_FACTORY && imeshwrap)
	    imeshwrap->SetFactory (fact);
	  if (!tmpl_thing_state)
	  {
            printf ("Object '%s' is not a thing!\n", str);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  tmpl_thing_state->DecRef ();
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case CS_TOKEN_CLONE:
        if (!isParent)
	{
	  printf ("CLONE statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          ScanStr (params, "%s", str);
	  iMeshWrapper* wrap = engine->FindMeshObject (str);
          if (!wrap)
          {
            printf ("Couldn't find thing '%s'!\n", str);
            return false;
          }

	  iThingState* tmpl_thing_state = QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iThingState);
	  if (!tmpl_thing_state)
	  {
            printf ("Object '%s' is not a thing!\n", str);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  tmpl_thing_state->DecRef ();
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case CS_TOKEN_PART:
	if (!load_thing_part (info, imeshwrap, engine,
		thing_state, params, thing_state->GetVertexCount (), false))
	  return false;
        break;
      case CS_TOKEN_SKYDOME:
        skydome_process (thing_state, name, params, info.default_material,
	    vt_offset);
        break;
      case CS_TOKEN_VERTEX:
        {
	  csVector3 v;
          ScanStr (params, "%f,%f,%f", &v.x, &v.y, &v.z);
          thing_state->CreateVertex (v);
        }
        break;
      case CS_TOKEN_CIRCLE:
        {
          float x, y, z, rx, ry, rz;
          int num, dir;
          ScanStr (params, "%f,%f,%f:%f,%f,%f,%d",
	  	&x, &y, &z, &rx, &ry, &rz, &num);
          if (num < 0) { num = -num; dir = -1; }
          else dir = 1;
          for (int i = 0 ; i < num ; i++)
          {
            float rad;
            if (dir == 1) rad = 2.*M_PI*(num-i-1)/(float)num;
            else rad = 2.*M_PI*i/(float)num;

            float cx = 0, cy = 0, cz = 0;
            float cc = cos (rad);
            float ss = sin (rad);
            if      (ABS (rx) < SMALL_EPSILON)
	    { cx = x; cy = y+cc*ry; cz = z+ss*rz; }
            else if (ABS (ry) < SMALL_EPSILON)
	    { cy = y; cx = x+cc*rx; cz = z+ss*rz; }
            else if (ABS (rz) < SMALL_EPSILON)
	    { cz = z; cx = x+cc*rx; cy = y+ss*ry; }
            thing_state->CreateVertex (csVector3 (cx, cy, cz));
          }
        }
        break;
      case CS_TOKEN_FOG:
      	printf ("FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.\n");
#if 0
//@@@
        if (!isParent)
	{
	  printf ("FOG statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          csFog& f = thing->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f",
	  	&f.red, &f.green, &f.blue, &f.density);
        }
#endif
        break;
      case CS_TOKEN_POLYGON:
        {
	  iPolygon3D* poly3d = load_poly3d (engine, xname, params,
            info.default_material, info.default_texlen,
	    thing_state, vt_offset);
	  if (!poly3d) return false;
        }
        break;

      case CS_TOKEN_CURVE:
        {
	  char cname[100];
	  ScanStr (params, "%s", cname);
	  iCurveTemplate* ct = engine->FindCurveTemplate (cname/* @@@ Onlyregion?*/);
	  iCurve* p = thing_state->CreateCurve (ct);
	  p->SetName (name);
          if (!ct->GetMaterial ())
	    p->SetMaterial (info.default_material);
        }
        break;

      case CS_TOKEN_CURVECENTER:
        {
          csVector3 c;
          ScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
          thing_state->SetCurvesCenter (c);
        }
        break;
      case CS_TOKEN_CURVESCALE:
        {
	  float f;
          ScanStr (params, "%f", &f);
	  thing_state->SetCurvesScale (f);
          break;
        }
      case CS_TOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
          ScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
          thing_state->AddCurveVertex (v, t);
        }
        break;

      case CS_TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        info.default_material = engine->FindMaterial (str
		/*@@@ REGIONS?, onlyRegion*/);
        if (info.default_material == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return false;
        }
        break;
      case CS_TOKEN_TEXLEN:
        ScanStr (params, "%f", &info.default_texlen);
        break;
      case CS_TOKEN_MAT_SET_SELECT:
        ScanStr (params, "%s", str);
        info.SetTextureSet (str);
        info.use_mat_set = true;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a thing!\n",
    	csGetLastOffender ());
    return false;
  }
  return true;
}

iBase* csThingLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  iMeshObjectFactory* fact = NULL;
  iThingState* thing_state = NULL;

  iMeshWrapper* imeshwrap = QUERY_INTERFACE (context, iMeshWrapper);
  if (imeshwrap) imeshwrap->DecRef ();
  iMeshFactoryWrapper* ifactmeshwrap = QUERY_INTERFACE (context,
  	iMeshFactoryWrapper);
  if (ifactmeshwrap) ifactmeshwrap->DecRef ();

  iMeshObjectType* type = engine->GetThingType (); // @@@ LOAD_PLUGIN LATER!
  // We always do NewFactory() even for mesh objects.
  // That's because csThing implements both so a factory is a mesh object.
  fact = type->NewFactory ();
  thing_state = QUERY_INTERFACE (fact, iThingState);

  char* buf = (char*)string;
  ThingLoadInfo info;
  if (load_thing_part (info, imeshwrap, engine, thing_state,
  	buf, 0, true))
    return fact;
  else
  {
    fact->DecRef ();
    return NULL;
  }
}

//---------------------------------------------------------------------------

csThingSaver::csThingSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csThingSaver::~csThingSaver ()
{
}

bool csThingSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csThingSaver::WriteDown (iBase* /*obj*/, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace (name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf (buf, "FACTORY ('%s')\n", name);
  str->Push (strnew (buf));
  fact->DecRef ();
}

//---------------------------------------------------------------------------

csPlaneLoader::csPlaneLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csPlaneLoader::~csPlaneLoader ()
{
}

bool csPlaneLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csPlaneLoader::Parse (const char* string, iEngine* engine,
	iBase* /*context*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (FIRST_LEN)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (SECOND_LEN)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (NAME)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;
  iPolyTxtPlane* ppl = engine->CreatePolyTxtPlane ();

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = 0, tx2_len = 0;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char name[255]; name[0] = 0;
  char* buf = (char*)string;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_NAME:
        ScanStr (params, "%s", name);
	ppl->SetName (name);
        break;
      case CS_TOKEN_ORIG:
        tx1_given = true;
        load_vector (params, tx_orig);
        break;
      case CS_TOKEN_FIRST:
        tx1_given = true;
        load_vector (params, tx1);
        break;
      case CS_TOKEN_FIRST_LEN:
        ScanStr (params, "%f", &tx1_len);
        tx1_given = true;
        break;
      case CS_TOKEN_SECOND:
        tx2_given = true;
        load_vector (params, tx2);
        break;
      case CS_TOKEN_SECOND_LEN:
        ScanStr (params, "%f", &tx2_len);
        tx2_given = true;
        break;
      case CS_TOKEN_MATRIX:
        load_matrix (params, tx_matrix);
        break;
      case CS_TOKEN_V:
        load_vector (params, tx_vector);
        break;
      case CS_TOKEN_UVEC:
        tx1_given = true;
        load_vector (params, tx1);
        tx1_len = tx1.Norm ();
        tx1 += tx_orig;
        break;
      case CS_TOKEN_VVEC:
        tx2_given = true;
        load_vector (params, tx2);
        tx2_len = tx2.Norm ();
        tx2 += tx_orig;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a plane!\n",
    	csGetLastOffender ());
    return NULL;
  }

  if (tx1_given)
    if (tx2_given)
    {
      if (!tx1_len)
      {
        printf ("Bad texture specification for PLANE '%s'\n", name);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
        printf ("Bad texture specification for PLANE '%s'\n", name);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else ppl->SetTextureSpace (tx_orig, tx1, tx1_len, tx2, tx2_len);
    }
    else
    {
      printf ("Not supported!\n");
      return NULL;
    }
  else
    ppl->SetTextureSpace (tx_matrix, tx_vector);

  return ppl;
}

//---------------------------------------------------------------------------

csPlaneSaver::csPlaneSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csPlaneSaver::~csPlaneSaver ()
{
}

bool csPlaneSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csPlaneSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/,
  iEngine* /*engine*/)
{
}

//---------------------------------------------------------------------------

csBezierLoader::csBezierLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csBezierLoader::~csBezierLoader ()
{
}

bool csBezierLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csBezierLoader::Parse (const char* string, iEngine* engine,
	iBase* /*context*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (NAME)
    CS_TOKEN_TABLE (VERTICES)
  CS_TOKEN_TABLE_END

  char *xname;
  long cmd;
  int i;
  char *params;
  char name[100];

  iCurveTemplate* tmpl = engine->CreateBezierTemplate ();

  iMaterialWrapper* mat = NULL;
  char str[255];

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_NAME:
        ScanStr (params, "%s", name);
	tmpl->SetName (name);
        break;
      case CS_TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = engine->FindMaterial (str/*@@@, onlyRegion*/); //@@@ REGION SUPPORT?
        if (mat == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return NULL;
        }
        tmpl->SetMaterial (mat);
        break;
      case CS_TOKEN_VERTICES:
        {
          int list[100], num;
          ScanStr (params, "%D", list, &num);
          if (num != 9)
          {
            printf ("Wrong number of vertices to bezier!\n");
            return NULL;
          }
          for (i = 0 ; i < num ; i++) tmpl->SetVertex (i, list[i]);
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a bezier template!\n",
    	csGetLastOffender ());
    return NULL;
  }
  return tmpl;
}

//---------------------------------------------------------------------------

csBezierSaver::csBezierSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csBezierSaver::~csBezierSaver ()
{
}

bool csBezierSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csBezierSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/,
  iEngine* /*engine*/)
{
}


//---------------------------------------------------------------------------
