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
#include "isys/plugin.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/curve.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (BEZIER)
  CS_TOKEN_DEF (CIRCLE)
  CS_TOKEN_DEF (CLIP)
  CS_TOKEN_DEF (CLONE)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (COLLDET)
  CS_TOKEN_DEF (COSFACT)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (CURVE)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (CURVESCALE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FASTMESH)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FLAT)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (GOURAUD)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (TILING)
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

SCF_IMPLEMENT_IBASE (csThingLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csThingSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csPlaneLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPlaneLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csPlaneSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPlaneSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBezierLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBezierLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBezierSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBezierSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csThingLoader)
SCF_IMPLEMENT_FACTORY (csThingSaver)
SCF_IMPLEMENT_FACTORY (csPlaneLoader)
SCF_IMPLEMENT_FACTORY (csPlaneSaver)
SCF_IMPLEMENT_FACTORY (csBezierLoader)
SCF_IMPLEMENT_FACTORY (csBezierSaver)

SCF_EXPORT_CLASS_TABLE (thingldr)
  SCF_EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.factory.thing",
    "Crystal Space Thing Mesh Factory Loader")
  SCF_EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.factory.thing",
    "Crystal Space Thing Mesh Factory Saver")
  SCF_EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.thing",
    "Crystal Space Thing Mesh Loader")
  SCF_EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.thing",
    "Crystal Space Thing Mesh Saver")
  SCF_EXPORT_CLASS (csPlaneLoader, "crystalspace.mesh.loader.thing.plane",
    "Crystal Space Thing Plane Loader")
  SCF_EXPORT_CLASS (csPlaneSaver, "crystalspace.mesh.saver.thing.plane",
    "Crystal Space Thing Plane Saver")
  SCF_EXPORT_CLASS (csBezierLoader, "crystalspace.mesh.loader.thing.bezier",
    "Crystal Space Thing Bezier Loader")
  SCF_EXPORT_CLASS (csBezierSaver, "crystalspace.mesh.saver.thing.bezier",
    "Crystal Space Thing Bezier Saver")
SCF_EXPORT_CLASS_TABLE_END

#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingLoader::~csThingLoader ()
{
}

bool csThingLoader::Initialize (iObjectRegistry* object_reg)
{
  csThingLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
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
        csScanStr (params, "%f", &angle);
        m *= csXRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Y:
        csScanStr (params, "%f", &angle);
        m *= csYRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Z:
        csScanStr (params, "%f", &angle);
        m *= csZRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT:
        csScanStr (params, "%F", list, &num);
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
        csScanStr (params, "%f", &scaler);
        m *= csXScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Y:
        csScanStr (params, "%f", &scaler);
        m *= csYScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Z:
        csScanStr (params, "%f", &scaler);
        m *= csZScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE:
        csScanStr (params, "%F", list, &num);
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
    csScanStr (buf, "%F", list, &num);
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
  csScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

static void OptimizePolygon (iPolygon3D *p)
{
  if (!p->GetPortal () || p->GetAlpha ()
  	|| p->GetPolyTexType ()->GetMixMode () != 0)
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
    CS_TOKEN_TABLE (TILING)
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
        csScanStr (params, "%f", &alpha);
	Mixmode |= CS_FX_SETALPHA(alpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
      case CS_TOKEN_TILING: Mixmode |= CS_FX_TILING; break;
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
  if (default_material)
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
        csScanStr (params, "%s", str);
	//@@@ REGION SUPPORT? (below)
        mat = engine->GetMaterialList ()->
		FindByName (str/*@@@, onlyRegion*/);
        if (mat == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return NULL;
        }
        poly3d->SetMaterial (mat);
        break;
      case CS_TOKEN_LIGHTING:
        {
          bool do_lighting;
          csScanStr (params, "%b", &do_lighting);
          poly3d->GetFlags ().Set (CS_POLY_LIGHTING,
	  	do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case CS_TOKEN_COSFACT:
        {
          float cosfact;
          csScanStr (params, "%f", &cosfact);
          poly3d->SetCosinusFactor (cosfact);
        }
        break;
      case CS_TOKEN_ALPHA:
        {
          int alpha;
          csScanStr (params, "%d", &alpha);
          poly3d->SetAlpha (alpha * 655 / 256);
        }
        break;
      case CS_TOKEN_COLLDET:
        {
          bool do_colldet;
          csScanStr (params, "%b", &do_colldet);
	  if (do_colldet) set_colldet = 1;
	  else set_colldet = -1;
        }
        break;
      case CS_TOKEN_PORTAL:
        {
          csScanStr (params, "%s", str);
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
        while ((cmd = csGetObject(&params, tex_commands, &name, &params2)) > 0)
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
              csScanStr (params2, "%F", flist, &num);
              if (num == 1) tx_orig = thing_state->GetVertex ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST:
	      tx_uv_given = false;
              tx1_given = true;
              csScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = thing_state->GetVertex ((int)flist[0]);
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST_LEN:
	      tx_uv_given = false;
              csScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case CS_TOKEN_SECOND:
	      tx_uv_given = false;
              tx2_given = true;
              csScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = thing_state->GetVertex ((int)flist[0]);
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_SECOND_LEN:
	      tx_uv_given = false;
              csScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case CS_TOKEN_LEN:
	      tx_uv_given = false;
              csScanStr (params2, "%f", &tx_len);
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
              csScanStr (params2, "%s", str);
              strcpy (plane_name, str);
              tx_len = 0;
              break;
            case CS_TOKEN_UV_SHIFT:
              uv_shift_given = true;
              csScanStr (params2, "%f,%f", &u_shift, &v_shift);
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
                csScanStr (params2, "%d,%f,%f,%d,%f,%f,%d,%f,%f",
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
          csScanStr (params, "%D", list, &num);
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
	  ptt->SetMixMode (mixmode);
          if (mixmode & CS_FX_MASK_ALPHA)
            poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
          break;
	}
      case CS_TOKEN_UV:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  iPolyTexType* ptt = poly3d->GetPolyTexType ();
	  iPolyTexFlat* fs = SCF_QUERY_INTERFACE (ptt, iPolyTexFlat);
          int num, nv = poly3d->GetVertexCount ();
	  fs->Setup (poly3d);
          float list [2 * 100];
          csScanStr (params, "%F", list, &num);
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
	  iPolyTexGouraud* gs = SCF_QUERY_INTERFACE (ptt, iPolyTexGouraud);
          int num, nv = poly3d->GetVertexCount ();
	  gs->Setup (poly3d);
          float list [3 * 100];
          csScanStr (params, "%F", list, &num);
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
	  iPolyTexFlat* fs = SCF_QUERY_INTERFACE (ptt, iPolyTexFlat);
          int num, nv = poly3d->GetVertexCount ();
	  fs->Setup (poly3d);
          float list [3 * 100];
          csScanStr (params, "%F", list, &num);
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

  if (uv_shift_given)
  {
    iPolyTexType* ptt = poly3d->GetPolyTexType ();
    iPolyTexLightMap* plm = SCF_QUERY_INTERFACE (ptt, iPolyTexLightMap);
    if (plm)
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
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (CLONE)
    CS_TOKEN_TABLE (VISTREE)
    CS_TOKEN_TABLE (FASTMESH)
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
      case CS_TOKEN_FASTMESH:
        if (!isParent)
	{
	  printf ("FASTMESH flag only for top-level thing!\n");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_FASTMESH);
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
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->GetMeshFactories ()
	  	->FindByName (str);
          if (!fact)
          {
            printf ("Couldn't find thing factory '%s'!\n", str);
            return false;
          }
	  iThingState* tmpl_thing_state = SCF_QUERY_INTERFACE (
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
          csScanStr (params, "%s", str);
	  iMeshWrapper* wrap = engine->GetMeshes ()->FindByName (str);
          if (!wrap)
          {
            printf ("Couldn't find thing '%s'!\n", str);
            return false;
          }

	  iThingState* tmpl_thing_state = SCF_QUERY_INTERFACE (
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
      case CS_TOKEN_VERTEX:
        {
	  csVector3 v;
          csScanStr (params, "%f,%f,%f", &v.x, &v.y, &v.z);
          thing_state->CreateVertex (v);
        }
        break;
      case CS_TOKEN_CIRCLE:
        {
          float x, y, z, rx, ry, rz;
          int num, dir;
          csScanStr (params, "%f,%f,%f:%f,%f,%f,%d",
	  	&x, &y, &z, &rx, &ry, &rz, &num);
          if (num < 0) { num = -num; dir = -1; }
          else dir = 1;
		  {
		  int i;
          for (i = 0 ; i < num ; i++)
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
          csScanStr (params, "%f,%f,%f,%f",
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
	  csScanStr (params, "%s", cname);
	  iCurveTemplate* ct =
	    engine->FindCurveTemplate (cname/* @@@ Onlyregion?*/);
	  iCurve* p = thing_state->CreateCurve (ct);
	  p->QueryObject()->SetName (name);
          if (!ct->GetMaterial ())
	    p->SetMaterial (info.default_material);
        }
        break;

      case CS_TOKEN_CURVECENTER:
        {
          csVector3 c;
          csScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
          thing_state->SetCurvesCenter (c);
        }
        break;
      case CS_TOKEN_CURVESCALE:
        {
	  float f;
          csScanStr (params, "%f", &f);
	  thing_state->SetCurvesScale (f);
          break;
        }
      case CS_TOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
          csScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
          thing_state->AddCurveVertex (v, t);
        }
        break;

      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);
        info.default_material = engine->GetMaterialList ()->
		FindByName (str
		/*@@@ REGIONS?, onlyRegion*/);
        if (info.default_material == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return false;
        }
        break;
      case CS_TOKEN_TEXLEN:
        csScanStr (params, "%f", &info.default_texlen);
        break;
      case CS_TOKEN_MAT_SET_SELECT:
        csScanStr (params, "%s", str);
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

  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  if (imeshwrap) imeshwrap->DecRef ();
  iMeshFactoryWrapper* ifactmeshwrap = SCF_QUERY_INTERFACE (context,
  	iMeshFactoryWrapper);
  if (ifactmeshwrap) ifactmeshwrap->DecRef ();

  iMeshObjectType* type = engine->GetThingType (); // @@@ CS_LOAD_PLUGIN LATER!
  // We always do NewFactory() even for mesh objects.
  // That's because csThing implements both so a factory is a mesh object.
  fact = type->NewFactory ();
  thing_state = SCF_QUERY_INTERFACE (fact, iThingState);

  char* buf = (char*)string;
  ThingLoadInfo info;
  if (!load_thing_part (info, imeshwrap, engine, thing_state,
  	buf, 0, true))
  {
    fact->DecRef ();
    fact = NULL;
  }
  thing_state->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csThingSaver::csThingSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingSaver::~csThingSaver ()
{
}

bool csThingSaver::Initialize (iObjectRegistry* object_reg)
{
  csThingSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csThingSaver::WriteDown (iBase* /*obj*/, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace (name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf (buf, "FACTORY ('%s')\n", name);
  str->Push (csStrNew (buf));
  fact->DecRef ();
}

//---------------------------------------------------------------------------

csPlaneLoader::csPlaneLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csPlaneLoader::~csPlaneLoader ()
{
}

bool csPlaneLoader::Initialize (iObjectRegistry* object_reg)
{
  csPlaneLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
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
        csScanStr (params, "%s", name);
	ppl->QueryObject()->SetName (name);
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
        csScanStr (params, "%f", &tx1_len);
        tx1_given = true;
        break;
      case CS_TOKEN_SECOND:
        tx2_given = true;
        load_vector (params, tx2);
        break;
      case CS_TOKEN_SECOND_LEN:
        csScanStr (params, "%f", &tx2_len);
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
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csPlaneSaver::~csPlaneSaver ()
{
}

bool csPlaneSaver::Initialize (iObjectRegistry* object_reg)
{
  csPlaneSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csPlaneSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/,
  iEngine* /*engine*/)
{
}

//---------------------------------------------------------------------------

csBezierLoader::csBezierLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBezierLoader::~csBezierLoader ()
{
}

bool csBezierLoader::Initialize (iObjectRegistry* object_reg)
{
  csBezierLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
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
        csScanStr (params, "%s", name);
	tmpl->QueryObject()->SetName (name);
        break;
      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);
	//@@@ REGION SUPPORT? (below)
        mat = engine->GetMaterialList ()->
		FindByName (str/*@@@, onlyRegion*/);
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
          csScanStr (params, "%D", list, &num);
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
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBezierSaver::~csBezierSaver ()
{
}

bool csBezierSaver::Initialize (iObjectRegistry* object_reg)
{
  csBezierSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csBezierSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/,
  iEngine* /*engine*/)
{
}
