/*
    Copyright (C) 2001 by Norman Krämer

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

#include <stdarg.h>
#include "cssysdef.h"
#include "syntxldr.h"
#include "csutil/cscolor.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/util.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csgeom/transfrm.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "ivideo/material.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/polygon.h"
#include "imesh/object.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "imap/parser.h"
#include "imap/ldrctxt.h"

CS_IMPLEMENT_PLUGIN;

SCF_IMPLEMENT_IBASE (csTextSyntaxService)
  SCF_IMPLEMENTS_INTERFACE (iSyntaxService)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTextSyntaxService::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTextSyntaxService);

SCF_EXPORT_CLASS_TABLE (cssynldr)
  SCF_EXPORT_CLASS (csTextSyntaxService,
  	"crystalspace.syntax.loader.service.text",
	"Crystal Space loader services for textual CS syntax")
SCF_EXPORT_CLASS_TABLE_END

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (TILING)
  CS_TOKEN_DEF (NONE)
  CS_TOKEN_DEF (FLAT)
  CS_TOKEN_DEF (GOURAUD)
  CS_TOKEN_DEF (LIGHTMAP)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (PORTAL)
  CS_TOKEN_DEF (WARP)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (SHADING)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (UVA)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (COLLDET)
  CS_TOKEN_DEF (COSFACT)
  CS_TOKEN_DEF (MAXVISIT)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (LEN)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (SECOND_LEN)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (PLANE)
  CS_TOKEN_DEF (UV_SHIFT)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (MIRROR)
  CS_TOKEN_DEF (STATIC)
  CS_TOKEN_DEF (ZFILL)
  CS_TOKEN_DEF (CLIP)
CS_TOKEN_DEF_END

static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

csTextSyntaxService::csTextSyntaxService (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  reporter = NULL;
  thing_type = NULL;
}

csTextSyntaxService::~csTextSyntaxService ()
{
  SCF_DEC_REF (reporter);
  SCF_DEC_REF (thing_type);
}

bool csTextSyntaxService::Initialize (iObjectRegistry* object_reg)
{
  csTextSyntaxService::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

bool csTextSyntaxService::ParseMatrix (char *buf, csMatrix3 &m)
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
  int cmd;
  float angle;
  float scaler;
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
	{
	  ReportError (reporter, "crystalspace.syntax.matrix",
	    "Badly formed rotation: '%s'", params);
	  return false;
	}
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
	{
	  ReportError (reporter, "crystalspace.syntax.matrix",
	    "Badly formed scale: '%s'", params);
	  return false;
	}
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
    {
      ReportError (reporter, "crystalspace.syntax.matrix",
	"Badly formed matrix '%s'", buf);
      return false;
    }
  }
  return true;
}

bool csTextSyntaxService::ParseVector (char *buf, csVector3 &v)
{
  csScanStr (buf, "%F", list, &num);
  if (num == 3)
  {
    v.x = list[0];
    v.y = list[1];
    v.z = list[2];
  }
  else
  {
    ReportError (reporter, "crystalspace.syntax.vector",
      "Malformed vector parameter");
    return false;
  }
  return true;
}

bool csTextSyntaxService::ParseMixmode (char *buf, uint &mixmode)
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

  mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter, "crystalspace.syntax.mixmode",
        "Expected parameters instead of '%s'!", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_COPY: mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
        csScanStr (params, "%f", &alpha);
	mixmode |= CS_FX_SETALPHA(alpha);
	break;
      case CS_TOKEN_TRANSPARENT: mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: mixmode |= CS_FX_KEYCOLOR; break;
      case CS_TOKEN_TILING: mixmode |= CS_FX_TILING; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter, "crystalspace.syntax.mixmode",
      "Token '%s' not found while parsing the modes!",
      csGetLastOffender ());
    return false;
  }
  return true;
}

bool csTextSyntaxService::ParseShading (char *buf, int &shading)
{
  CS_TOKEN_TABLE_START (texturing_commands)
    CS_TOKEN_TABLE (NONE)
    CS_TOKEN_TABLE (FLAT)
    CS_TOKEN_TABLE (GOURAUD)
    CS_TOKEN_TABLE (LIGHTMAP)
  CS_TOKEN_TABLE_END

  char *params, *name;
  long cmd;

  shading = 0;
  while ((cmd = csGetObject (&buf, texturing_commands, &name, &params)) > 0)
    switch (cmd)
    {
    case CS_TOKEN_NONE:
      shading = POLYTXT_NONE;
      break;
    case CS_TOKEN_FLAT:
      shading = POLYTXT_FLAT;
      break;
    case CS_TOKEN_GOURAUD:
      shading = POLYTXT_GOURAUD;
      break;
    case CS_TOKEN_LIGHTMAP:
      shading = POLYTXT_LIGHTMAP;
      break;
    default:
      if (cmd == CS_PARSERR_TOKENNOTFOUND)
      {
	ReportError (reporter, "crystalspace.syntax.shading",
	  "Token '%s' not found while parsing the shading specification!",
	  csGetLastOffender ());
        return false;
      }
    };

  return true;
}

bool csTextSyntaxService::ParseTexture (
	char *buf, const csVector3* vref, uint &texspec,
	csVector3 &tx_orig, csVector3 &tx1, csVector3 &tx2, csVector3 &len,
	csMatrix3 &tx_m, csVector3 &tx_v,
	csVector2 &uv_shift,
	int &idx1, csVector2 &uv1,
	int &idx2, csVector2 &uv2,
	int &idx3, csVector2 &uv3,
	char *plane, const char *polyname)
{
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

  long cmd;
  char *name, *params;
  char str[100];

  float flist[100];
  int num;

  while ((cmd = csGetObject(&buf, tex_commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter, "crystalspace.syntax.texture",
        "Expected parameters instead of '%s'!", buf);
      return false;
    }
    switch (cmd)
    {
    case CS_TOKEN_ORIG:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V1;
      csScanStr (params, "%F", flist, &num);
      if (num == 1) tx_orig = vref[(int)flist[0]];
      if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
      break;
    case CS_TOKEN_FIRST:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V1;
      csScanStr (params, "%F", flist, &num);
      if (num == 1) tx1 = vref [(int)flist[0]];
      if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
      break;
    case CS_TOKEN_FIRST_LEN:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V1;
      csScanStr (params, "%f", &len.y);
      break;
    case CS_TOKEN_SECOND:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V2;
      csScanStr (params, "%F", flist, &num);
      if (num == 1) tx2 = vref[(int)flist[0]];
      if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
      break;
    case CS_TOKEN_SECOND_LEN:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V2;
      csScanStr (params, "%f", &len.z);
      break;
    case CS_TOKEN_LEN:
      texspec &= ~CSTEX_UV;
      csScanStr (params, "%f", &len.x);
      break;
    case CS_TOKEN_MATRIX:
      texspec &= ~CSTEX_UV;
      ParseMatrix (params, tx_m);
      len.x = 0;
      break;
    case CS_TOKEN_V:
      texspec &= ~CSTEX_UV;
      ParseVector (params, tx_v);
      len.x = 0;
      break;
    case CS_TOKEN_PLANE:
      texspec &= ~CSTEX_UV;
      csScanStr (params, "%s", str);
      strcpy (plane, str);
      len.x = 0;
      break;
    case CS_TOKEN_UV_SHIFT:
      texspec |= CSTEX_UV_SHIFT;
      csScanStr (params, "%f,%f", &uv_shift.x, &uv_shift.y);
      break;
    case CS_TOKEN_UVEC:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V1;
      ParseVector (params, tx1);
      len.y = tx1.Norm ();
      tx1 += tx_orig;
      break;
    case CS_TOKEN_VVEC:
      texspec &= ~CSTEX_UV;
      texspec |= CSTEX_V2;
      ParseVector (params, tx2);
      len.z = tx2.Norm ();
      tx2 += tx_orig;
      break;
    case CS_TOKEN_UV:
      texspec |= CSTEX_UV;
      csScanStr (params, "%d,%f,%f,%d,%f,%f,%d,%f,%f",
		 &idx1, &uv1.x, &uv1.y,
		 &idx2, &uv2.x, &uv2.y,
		 &idx3, &uv3.x, &uv3.y);
      break;
    }
  }

  if (texspec & CSTEX_V2)
  {
    if (!len.y)
    {
      ReportError (reporter, "crystalspace.syntax.texture",
        "Bad texture specification for POLYGON '%s'", polyname);
      len.y = 1;
      return false;
    }
    if (!len.z)
    {
      ReportError (reporter, "crystalspace.syntax.texture",
        "Bad texture specification for POLYGON '%s'", polyname);
      len.z = 1;
      return false;
    }
  }
  else
  {
    if (!len.y)
    {
      ReportError (reporter, "crystalspace.syntax.texture",
        "Bad texture specification for POLYGON '%s'", polyname);
      len.y = 1;
      return false;
    }
  }

  return true;
}

bool csTextSyntaxService::ParseWarp (
	char *buf, csVector &flags, bool &mirror, bool &warp,
	int& msv,
	csMatrix3 &m, csVector3 &before, csVector3 &after)
{

  CS_TOKEN_TABLE_START (portal_commands)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (W)
    CS_TOKEN_TABLE (MIRROR)
    CS_TOKEN_TABLE (STATIC)
    CS_TOKEN_TABLE (MAXVISIT)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (CLIP)
  CS_TOKEN_TABLE_END

  char *params, *name;
  long cmd;

  while ((cmd = csGetObject (&buf, portal_commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter, "crystalspace.syntax.warp",
        "Expected parameters instead of '%s'!", buf);
      return false;
    }
    switch (cmd)
    {
    case CS_TOKEN_MAXVISIT:
      csScanStr (params, "%d", &msv);
      break;
    case CS_TOKEN_MATRIX:
      ParseMatrix (params, m);
      mirror = false;
      warp = true;
      break;
    case CS_TOKEN_V:
      ParseVector (params, before);
      after = before;
      mirror = false;
      warp = true;
      break;
    case CS_TOKEN_W:
      ParseVector (params, after);
      mirror = false;
      warp = true;
      break;
    case CS_TOKEN_MIRROR:
      mirror = true;
      break;
    case CS_TOKEN_STATIC:
      flags.Push ((csSome)CS_PORTAL_STATICDEST);
      break;
    case CS_TOKEN_ZFILL:
      flags.Push ((csSome)CS_PORTAL_ZFILL);
      break;
    case CS_TOKEN_CLIP:
      flags.Push ((csSome)CS_PORTAL_CLIPDEST);
      break;
    }
  }

  return true;
}

void csTextSyntaxService::OptimizePolygon (iPolygon3D *p)
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

class MissingSectorCallback : public iPortalCallback
{
public:
  iLoaderContext* ldr_context;
  char* sectorname;

  SCF_DECLARE_IBASE;
  MissingSectorCallback (iLoaderContext* ldr_context, const char* sector)
  {
    SCF_CONSTRUCT_IBASE (NULL);
    MissingSectorCallback::ldr_context = ldr_context;
    sectorname = csStrNew (sector);
    if (ldr_context) ldr_context->IncRef ();
  }
  virtual ~MissingSectorCallback ()
  {
    delete[] sectorname;
    if (ldr_context) ldr_context->DecRef ();
  }
  
  virtual bool Traverse (iPortal* portal, iBase* /*context*/)
  {
    iSector* sector = ldr_context->FindSector (sectorname);
    if (!sector) return false;
    portal->SetSector (sector);
    // For efficiency reasons we deallocate the name here.
    delete[] sectorname;
    sectorname = NULL;
    return true;
  }
};

SCF_IMPLEMENT_IBASE (MissingSectorCallback)
  SCF_IMPLEMENTS_INTERFACE (iPortalCallback)
SCF_IMPLEMENT_IBASE_END

bool csTextSyntaxService::ParsePoly3d (
	iLoaderContext* ldr_context,
	iEngine* engine, iPolygon3D* poly3d, char* buf,
	float default_texlen,
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
    CS_TOKEN_TABLE (LEN)
    CS_TOKEN_TABLE (PLANE)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char *params;

  iMaterialWrapper* mat = NULL;

  if (!thing_type)
  {
    iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
    CS_ASSERT (plugin_mgr != NULL);
    thing_type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	  "crystalspace.mesh.object.thing", iMeshObjectType);
    if (!thing_type)
      thing_type = CS_LOAD_PLUGIN (plugin_mgr,
    	  "crystalspace.mesh.object.thing", iMeshObjectType);
    plugin_mgr->DecRef ();
  }

  CS_ASSERT (thing_type != NULL);
  iThingEnvironment* te =
    SCF_QUERY_INTERFACE (thing_type, iThingEnvironment);

  uint texspec = 0;
  int tx_uv_i1 = 0;
  int tx_uv_i2 = 0;
  int tx_uv_i3 = 0;
  csVector2 tx_uv1;
  csVector2 tx_uv2;
  csVector2 tx_uv3;

  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  csVector3 tx_len (default_texlen, default_texlen, default_texlen);

  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char plane_name[100];
  plane_name[0] = 0;
  csVector2 uv_shift (0, 0);

  bool do_mirror = false;
  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter, "crystalspace.syntax.polygon",
        "Expected parameters instead of '%s'!", buf);
      te->DecRef ();
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);
        mat = ldr_context->FindMaterial (str);
        if (mat == NULL)
        {
          ReportError (reporter, "crystalspace.syntax.polygon",
            "Couldn't find material named '%s'!", str);
          te->DecRef ();
          return false;
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
	  iSector* sector = ldr_context->FindSector (str);
	  if (sector)
	  {
            poly3d->CreatePortal (sector);
	  }
	  else
	  {
	    poly3d->CreateNullPortal ();
	    iPortal* portal = poly3d->GetPortal ();
	    MissingSectorCallback* mscb = new MissingSectorCallback (
	    	ldr_context, str);
	    portal->SetMissingSectorCallback (mscb);
	    mscb->DecRef ();
	  }
        }
        break;
      case CS_TOKEN_WARP:
        if (poly3d->GetPortal ())
        {
          csMatrix3 m_w; m_w.Identity ();
          csVector3 v_w_before (0, 0, 0);
          csVector3 v_w_after (0, 0, 0);
	  csVector flags;
	  bool do_warp = false;
	  int msv = -1;

	  if (ParseWarp (params, flags, do_mirror, do_warp, msv,
	      m_w, v_w_before, v_w_after))
	  {
	    for (int i = 0; i < flags.Length (); i++)
	      poly3d->GetPortal ()->GetFlags ().Set ((uint)flags.Get (i));

	    if (do_mirror)
	    {
	      if (!set_colldet) set_colldet = 1;
	    }
	    else if (do_warp)
	      poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);

	    if (msv != -1)
	    {
	      poly3d->GetPortal ()->SetMaximumSectorVisit (msv);
	    }
	  }
	  else
	  {
	    te->DecRef ();
	    return false;
	  }
        }
        break;
      case CS_TOKEN_PLANE:
	texspec &= ~CSTEX_UV;
        csScanStr (params, "%s", str);
        strcpy (plane_name, str);
        tx_len.x = 0;
        break;
      case CS_TOKEN_LEN:
	texspec &= ~CSTEX_UV;
	csScanStr (params, "%f", &tx_len.x);
	break;
      case CS_TOKEN_TEXTURE:
	if (!ParseTexture (params, thing_state->GetVertices (), texspec,
			   tx_orig, tx1, tx2, tx_len,
			   tx_matrix, tx_vector,
			   uv_shift,
			   tx_uv_i1, tx_uv1,
			   tx_uv_i2, tx_uv2,
			   tx_uv_i3, tx_uv3,
			   plane_name, poly3d->QueryObject ()->GetName ()))
	{
	  te->DecRef ();
	  return false;
	}
        break;
      case CS_TOKEN_V:
      case CS_TOKEN_VERTICES:
        {
	  char* p = params;
	  while (*p && *p == ' ') p++;
	  if (*p < '0' || *p > '9')
	  {
	    // We have a special vertex selection depending on
	    // a VBLOCK or VROOM command previously generated.
	    int vtidx;
	    if (*(p+1) == ',')
	    {
	      csScanStr (p+2, "%d", &vtidx);
	      vtidx += vt_offset;
	    }
	    else
	      vtidx = thing_state->GetVertexCount ()-8;
	    switch (*p)
	    {
	      case 'w':
	        poly3d->CreateVertex (vtidx+6);
	        poly3d->CreateVertex (vtidx+4);
	        poly3d->CreateVertex (vtidx+0);
	        poly3d->CreateVertex (vtidx+2);
		break;
	      case 'e':
	        poly3d->CreateVertex (vtidx+5);
	        poly3d->CreateVertex (vtidx+7);
	        poly3d->CreateVertex (vtidx+3);
	        poly3d->CreateVertex (vtidx+1);
		break;
	      case 'n':
	        poly3d->CreateVertex (vtidx+7);
	        poly3d->CreateVertex (vtidx+6);
	        poly3d->CreateVertex (vtidx+2);
	        poly3d->CreateVertex (vtidx+3);
		break;
	      case 's':
	        poly3d->CreateVertex (vtidx+4);
	        poly3d->CreateVertex (vtidx+5);
	        poly3d->CreateVertex (vtidx+1);
	        poly3d->CreateVertex (vtidx+0);
		break;
	      case 'u':
	        poly3d->CreateVertex (vtidx+6);
	        poly3d->CreateVertex (vtidx+7);
	        poly3d->CreateVertex (vtidx+5);
	        poly3d->CreateVertex (vtidx+4);
		break;
	      case 'd':
	        poly3d->CreateVertex (vtidx+0);
	        poly3d->CreateVertex (vtidx+1);
	        poly3d->CreateVertex (vtidx+3);
	        poly3d->CreateVertex (vtidx+2);
		break;
	    }
	  }
	  else
	  {
            int list[100], num;
            csScanStr (params, "%D", list, &num);
            for (int i = 0 ; i < num ; i++)
	    {
	      if (list[i] == list[(i-1+num)%num])
	        printf ("Duplicate vertex-index found! Ignored...\n");
	      else
	        poly3d->CreateVertex (list[i]+vt_offset);
	    }
	  }
        }
        break;
      case CS_TOKEN_SHADING:
	{
	  int shading;
	  if (ParseShading (params, shading))
              poly3d->SetTextureType (shading);
	  else
	  {
	    te->DecRef ();
	    return false;
	  }
	}
        break;
      case CS_TOKEN_MIXMODE:
        {
          uint mixmode;
	  if (ParseMixmode (params, mixmode))
	  {
	    iPolyTexType* ptt = poly3d->GetPolyTexType ();
	    ptt->SetMixMode (mixmode);
	    if (mixmode & CS_FX_MASK_ALPHA)
	      poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
	  }
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
          if (num > nv)
	    num = nv;
          for (int i = 0; i < num; i++)
            fs->SetUV (i, list [i * 2], list [i * 2 + 1]);
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
          if (num > nv)
	    num = nv;
          for (int i = 0; i < num; i++)
            gs->SetColor (i, csColor (list [i * 3], list [i * 3 + 1],
				      list [i * 3 + 2]));
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
          if (num > nv)
	    num = nv;
          for (int i = 0; i < num; i++)
          {
            float a = list [i * 3] * TWO_PI / 360.;
            fs->SetUV (i, cos (a) * list [i * 3 + 1] + list [i * 3 + 2],
                          sin (a) * list [i * 3 + 1] + list [i * 3 + 2]);
          }
	  fs->DecRef ();
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter, "crystalspace.syntax.polygon",
      "Token '%s' not found while parsing a polygon!",
      csGetLastOffender ());
    te->DecRef ();
    return false;
  }

  if (poly3d->GetVertexCount () < 3)
  {
    ReportError (reporter, "crystalspace.syntax.polygon",
      "Polygon in line %d contains just %d vertices!",
      csGetParserLine (), poly3d->GetVertexCount ());
    te->DecRef ();
    return false;
  }

  if (set_colldet == 1)
    poly3d->GetFlags ().Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->GetFlags ().Reset (CS_POLY_COLLDET);

  if (texspec & CSTEX_UV)
  {
    poly3d->SetTextureSpace (
			     poly3d->GetVertex (tx_uv_i1), tx_uv1,
			     poly3d->GetVertex (tx_uv_i2), tx_uv2,
			     poly3d->GetVertex (tx_uv_i3), tx_uv3);
  }
  else if (texspec & CSTEX_V1)
  {
    if (texspec & CSTEX_V2)
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        ReportError (reporter, "crystalspace.syntax.polygon",
          "Bad texture specification for PLANE '%s'", name);
	return false;
      }
      else if ((tx2-tx_orig) < SMALL_EPSILON)
      {
        ReportError (reporter, "crystalspace.syntax.polygon",
          "Bad texture specification for PLANE '%s'", name);
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.y, tx2, tx_len.z);
    }
    else
    {
      if ((tx1-tx_orig) < SMALL_EPSILON)
      {
        ReportError (reporter, "crystalspace.syntax.polygon",
          "Bad texture specification for PLANE '%s'", name);
	return false;
      }
      else poly3d->SetTextureSpace (tx_orig, tx1, tx_len.x);
    }
  }
  else if (plane_name[0])
  {
    iPolyTxtPlane* pl = te->FindPolyTxtPlane (plane_name);
    if (!pl)
    {
      ReportError (reporter, "crystalspace.syntax.polygon",
        "Can't find plane '%s' for polygon '%s'",
      	plane_name, poly3d->QueryObject ()->GetName ());
      return false;
    }
    poly3d->SetTextureSpace (pl);
  }
  else if (tx_len.x)
  {
    // If a length is given (with 'LEN') we will first see if the polygon
    // is coplanar with the X, Y, or Z plane. In that case we will use
    // a standard plane. Otherwise we will just create a plane specific
    // for this case given the first two vertices.
    bool same_x = true, same_y = true, same_z = true;
    const csVector3& v = poly3d->GetVertex (0);
    for (int i = 1 ; i < poly3d->GetVertexCount () ; i++)
    {
      const csVector3& v2 = poly3d->GetVertex (i);
      if (same_x && ABS (v.x-v2.x) >= SMALL_EPSILON) same_x = false;
      if (same_y && ABS (v.y-v2.y) >= SMALL_EPSILON) same_y = false;
      if (same_z && ABS (v.z-v2.z) >= SMALL_EPSILON) same_z = false;
    }
    if (same_x)
    {
      char buf[200];
      sprintf (buf, "__X_%g,%g__", v.x, tx_len.x);
      iPolyTxtPlane* pl = te->FindPolyTxtPlane (buf);
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (v.x, 0, 0), csVector3 (v.x, 0, 1),
			     tx_len.x, csVector3 (v.x, 1, 0), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else if (same_y)
    {
      char buf[200];
      sprintf (buf, "__Y_%g,%g__", v.y, tx_len.x);
      iPolyTxtPlane* pl = te->FindPolyTxtPlane (buf);
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (0, v.y, 0), csVector3 (1, v.y, 0),
			     tx_len.x, csVector3 (0, v.y, 1), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else if (same_z)
    {
      char buf[200];
      sprintf (buf, "__Z_%g,%g__", v.z, tx_len.x);
      iPolyTxtPlane* pl = te->FindPolyTxtPlane (buf);
      if (!pl)
      {
        pl = te->CreatePolyTxtPlane ();
        pl->QueryObject()->SetName (buf);
        pl->SetTextureSpace (csVector3 (0, 0, v.z), csVector3 (1, 0, v.z),
			     tx_len.x, csVector3 (0, 1, v.z), tx_len.x);
      }
      poly3d->SetTextureSpace (pl);
    }
    else
      poly3d->SetTextureSpace (poly3d->GetVertex (0), poly3d->GetVertex (1),
			       tx_len.x);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (texspec & CSTEX_UV_SHIFT)
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
      csVector3 shift (uv_shift.x, uv_shift.y, 0);
      tx_vector -= tx_matrix.GetInverse () * shift;
      poly3d->SetTextureSpace (tx_matrix, tx_vector);
      plm->DecRef ();
    }
  }

  if (do_mirror)
    poly3d->GetPortal ()->SetWarp (csTransform::GetReflect (
    	poly3d->GetWorldPlane () ));

  OptimizePolygon (poly3d);

  te->DecRef ();
  return true;
}

const char* csTextSyntaxService::MatrixToText (
	const csMatrix3 &m, int indent, bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  text = ind;
  text.Append ("MATRIX (\n");

  sprintf (line, "%g, %g, %g\n", m.m11, m.m12, m.m13);
  text.Append (ind);
  text.Append (line);

  sprintf (line, "%g, %g, %g\n", m.m21, m.m22, m.m23);
  text.Append (ind);
  text.Append (line);

  sprintf (line, "%g, %g, %g\n", m.m31, m.m32, m.m33);
  text.Append (ind);
  text.Append (line);

  text.Append (ind);
  if (newline)
    text.Append (")\n");
  else
    text.Append (")");

  return text;
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, float x, float y, float z,
					       int indent, bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  sprintf (line, "%s (%g, %g, %g)%c", vname, x, y, z, newline ? '\n' : ' ');
  text.Append (ind);
  text.Append (line);

  return text;
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, const csVector3 &v, int indent,
					       bool newline)
{
  return VectorToText (vname, v.x, v.y, v.z, indent, newline);
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, float x, float y, int indent,
					       bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  sprintf (line, "%s (%g, %g)%c", vname, x, y, newline ? '\n' : ' ');
  text.Append (ind);
  text.Append (line);

  return text;
}

const char* csTextSyntaxService::VectorToText (
	const char *vname, const csVector2 &v, int indent,
	bool newline)
{
  return VectorToText (vname, v.x, v.y, indent, newline, newline);
}

const char* csTextSyntaxService::BoolToText (
	const char *vname, bool b, int indent, bool newline)
{
  char line[100];
  csString ind = csString::PadLeft (' ', indent);

  sprintf (line, "%s (%s)%c", vname, b ? "yes" : "no", newline ? '\n' : ' ');
  text.Append (ind);
  text.Append (line);

  return text;
}

const char* csTextSyntaxService::MixmodeToText (
	uint mixmode, int indent, bool newline)
{
  csString ind = csString::PadLeft (' ', indent);

  text = ind;
  text.Append ("MIXMODE (\n");
  if (mixmode & CS_FX_COPY)
  {
    text.Append (ind);
    text.Append (" COPY ()\n");
  }
  if (mixmode & CS_FX_ADD)
  {
    text.Append (ind);
    text.Append (" ADD ()\n");
  }
  if(mixmode & CS_FX_MULTIPLY)
  {
    text.Append (ind);
    text.Append (" MULTIPLY ()\n");
  }
  if(mixmode & CS_FX_MULTIPLY2)
  {
    text.Append (ind);
    text.Append (" MULTIPLY2 ()\n");
  }
  if(mixmode & CS_FX_KEYCOLOR)
  {
    text.Append (ind);
    text.Append (" KEYCOLOR ()\n");
  }
  if(mixmode & CS_FX_TILING)
  {
    text.Append (ind);
    text.Append (" TILING ()\n");
  }
  if(mixmode & CS_FX_TRANSPARENT)
  {
    text.Append (ind);
    text.Append (" TRANSPARENT ()\n");
  }
  if(mixmode & CS_FX_ALPHA)
  {
    char buf[30];
    sprintf(buf, "  ALPHA (%.5g)\n", float(mixmode&CS_FX_MASK_ALPHA)/255.);;
    text.Append (ind);
    text.Append (buf);
  }
  text.Append (ind);
  if (newline)
    text.Append (")\n");
  else
    text.Append (")");

  return text;
}
