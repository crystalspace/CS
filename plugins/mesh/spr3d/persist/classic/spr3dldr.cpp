/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "cssys/sysfunc.h"
#include "csgeom/math3d.h"
#include "csgeom/matrix3.h"
#include "csgeom/quaterni.h"
#include "csgeom/transfrm.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/sprite3d.h"
#include "imesh/skeleton.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "iengine/motion.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "spr3dldr.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (TILING)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (ACTION)
  CS_TOKEN_DEF (APPLY_MOTION)
  CS_TOKEN_DEF (BASECOLOR)
  CS_TOKEN_DEF (F)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FRAME)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (LIMB)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (Q)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SKELETON)
  CS_TOKEN_DEF (SMOOTH)
  CS_TOKEN_DEF (TRANSFORM)
  CS_TOKEN_DEF (TRIANGLE)
  CS_TOKEN_DEF (TWEEN)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VERTICES)
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

SCF_IMPLEMENT_IBASE (csSprite3DFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSprite3DSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSprite3DSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSprite3DFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSprite3DFactorySaver)
SCF_IMPLEMENT_FACTORY (csSprite3DLoader)
SCF_IMPLEMENT_FACTORY (csSprite3DSaver)

SCF_EXPORT_CLASS_TABLE (spr3dldr)
  SCF_EXPORT_CLASS (csSprite3DFactoryLoader,
  	"crystalspace.mesh.loader.factory.sprite.3d",
	"Crystal Space Sprite3D Mesh Factory Loader")
  SCF_EXPORT_CLASS (csSprite3DFactorySaver,
  	"crystalspace.mesh.saver.factory.sprite.3d",
	"Crystal Space Sprite3D Mesh Factory Saver")
  SCF_EXPORT_CLASS (csSprite3DLoader, "crystalspace.mesh.loader.sprite.3d",
    "Crystal Space Sprite3D Mesh Loader")
  SCF_EXPORT_CLASS (csSprite3DSaver, "crystalspace.mesh.saver.sprite.3d",
    "Crystal Space Sprite3D Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csSprite3DFactoryLoader::csSprite3DFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csSprite3DFactoryLoader::~csSprite3DFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  if (reporter) reporter->DecRef ();
}

bool csSprite3DFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

static bool load_quaternion (char* buf, csQuaternion &q)
{
  csScanStr (buf, "%f,%f,%f,%f", &q.x, &q.y, &q.z, &q.r);
  return true;
}

static bool load_matrix (iReporter* reporter, char* buf, csMatrix3 &m)
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
    CS_TOKEN_TABLE (Q)
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
	{
          ReportError (reporter,
		"crystalspace.sprite3dloader.parse.matrix.badrotation",
		"Badly formed rotation '%s'!", params);
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
          ReportError (reporter,
		"crystalspace.sprite3dloader.parse.matrix.badscale",
		"Badly formed scale '%s'!", params);
	  return false;
	}
        break;
      case CS_TOKEN_Q:
        csQuaternion quat;
        load_quaternion(params, quat);
        csMatrix3 quatmat(quat);
        m*=quatmat;
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
      ReportError (reporter,
		"crystalspace.sprite3dloader.parse.matrix.badmatrix",
		"Badly formed matrix '%s'!", buf);
      return false;
    }
  }
  return true;
}

static bool load_vector (char* buf, csVector3 &v)
{
  csScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

static uint ParseMixmode (iReporter* reporter, char* buf)
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

  uint Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.sprite3dloader.parse.mixmode.badformat",
		"Bad format while parsing mixmode!");
      return ~0;
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
    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.mixmode.badtoken",
		"Token '%s' not found while parsing mixmodes!",
		csGetLastOffender ());
    return ~0;
  }
  return Mixmode;
}

bool csSprite3DFactoryLoader::LoadSkeleton (iReporter* reporter,
	iSkeletonLimb* limb, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (LIMB)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (TRANSFORM)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (Q)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  char* xname;

  long cmd;
  char* params;

  iSkeletonConnection* con = SCF_QUERY_INTERFACE (limb, iSkeletonConnection);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.skeleton.badformat",
		"Bad format while parsing skeleton!");
      if (con) con->DecRef ();
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_LIMB:
        {
          iSkeletonConnection* newcon = limb->CreateConnection ();
	  iSkeletonLimb* newlimb = SCF_QUERY_INTERFACE (newcon, iSkeletonLimb);
	  if (name) newlimb->SetName (name);
	  if (!LoadSkeleton (reporter, newlimb, params))
	  {
	    if (con) con->DecRef ();
	    return false;
	  }
	}
        break;
      case CS_TOKEN_TRANSFORM:
        if (con)
        {
          char* params2;
	  csMatrix3 m;
	  csVector3 v (0, 0, 0);
          while ((cmd = csGetObject(&params,tok_matvec,&xname,&params2)) > 0)
          {
    	    if (!params2)
    	    {
	      ReportError (reporter,
	      "crystalspace.sprite3dfactoryloader.parse.skeleton.badtransform",
		"Bad format while parsing skeleton transform!");
	      con->DecRef ();
	      return false;
    	    }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
                if (!load_matrix (reporter, params2, m))
		{
		  con->DecRef ();
		  return false;
		}
		break;
   	      case CS_TOKEN_Q: {
		csQuaternion q;
	        if (!load_quaternion (params2, q))
	        {
		  con->DecRef ();
		  return false;
	        }
		m.Set(q);
	        break;
	      }
              case CS_TOKEN_V:
                load_vector (params2, v);
		break;
            }
          }
	  csTransform tr (m, -m.GetInverse () * v);
	  con->SetTransformation (tr);
        }
	else
	{
	  ReportError (reporter,
	    "crystalspace.sprite3dfactoryloader.parse.skeleton.badtransform",
	    "TRANSFORM not valid for this type of skeleton limb!");
	  if (con) con->DecRef ();
	  return false;
	}
	break;
      case CS_TOKEN_VERTICES:
        {
          int vert, next, tot=0;
	  while (csScanStr (&params[tot], "%d%n", &vert, &next) > 0)
	  {
            limb->AddVertex (vert);
	    tot+=next;
	    next=0;
	    csScanStr(&params[tot], ",%n", &next);
	    tot+=next;
	  }
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.skeleton.badtoken",
		"Token '%s' not found while parsing skeleton!",
		csGetLastOffender ());
    if (con) con->DecRef ();
    return false;
  }

  if (con) con->DecRef ();
  return true;
}

iBase* csSprite3DFactoryLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase* context)
{
  // @@@ Implement MIXMODE
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FRAME)
    CS_TOKEN_TABLE (ACTION)
    CS_TOKEN_TABLE (SMOOTH)
    CS_TOKEN_TABLE (TRIANGLE)
    CS_TOKEN_TABLE (SKELETON)
    CS_TOKEN_TABLE (TWEEN)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_frame)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_frameset)
    CS_TOKEN_TABLE (F)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char* params2;
  char str[255];

  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.sprite.3d", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.sprite.3d",
    	iMeshObjectType);
  }
  if (!type)
  {
    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.setup.objecttype",
		"Could not load the sprite.3d mesh object plugin!");
    return NULL;
  }

  // @@@ Temporary fix to allow to set actions for objects loaded
  // with impexp. Once those loaders move to another plugin this code
  // below should be removed.
  iMeshObjectFactory* fact =
    context
      ? SCF_QUERY_INTERFACE (context, iMeshObjectFactory)
      : NULL;
  // DecRef of fact will be handled later.

  // If there was no factory we create a new one.
  if (!fact)
    fact = type->NewFactory ();

  type->DecRef ();
  iSprite3DFactoryState* spr3dLook = SCF_QUERY_INTERFACE (fact,
  	iSprite3DFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.badformat",
		"Bad format while parsing sprite3d factory!");
      fact->DecRef ();
      spr3dLook->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.unknownmaterial",
		"Couldn't find material named '%s'", str);
	    spr3dLook->DecRef ();
            fact->DecRef ();
            return NULL;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_SKELETON:
	{
	  spr3dLook->EnableSkeletalAnimation ();
	  iSkeleton* skeleton = spr3dLook->GetSkeleton ();
	  iSkeletonLimb* skellimb = SCF_QUERY_INTERFACE (skeleton,
	  	iSkeletonLimb);
	  if (name) skellimb->SetName (name);
	  if (!LoadSkeleton (reporter, skellimb, params))
	  {
	    spr3dLook->DecRef ();
	    fact->DecRef ();
	    return NULL;
	  }
	}
        break;

      case CS_TOKEN_ACTION:
        {
          iSpriteAction* act = spr3dLook->AddAction ();
          act->SetName (name);
          int d;
          char fn[64];
          while ((cmd = csGetObject (&params, tok_frameset, &name,
	  	&params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.action.badformat",
		"Bad format while parsing ACTION!");
	      spr3dLook->DecRef ();
	      fact->DecRef ();
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_F:
                csScanStr (params2, "%s,%d", fn, &d);
                iSpriteFrame* ff = spr3dLook->FindFrame (fn);
                if (!ff)
                {
	          ReportError (reporter,
		    "crystalspace.sprite3dfactoryloader.parse.action.badframe",
		    "Trying to add unknown frame '%s' to action '%s'!",
		    fn, act->GetName ());
		  spr3dLook->DecRef ();
		  fact->DecRef ();
                  return NULL;
                }
                act->AddFrame (ff, d);
                break;
            }
          }
        }
        break;

      case CS_TOKEN_FRAME:
        {
          iSpriteFrame* fr = spr3dLook->AddFrame ();
          fr->SetName (name);
          int anm_idx = fr->GetAnmIndex ();
          int tex_idx = fr->GetTexIndex ();
          int i = 0;
          float x, y, z, u, v;
          while ((cmd = csGetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.badformat",
		"Bad format while parsing FRAME!");
	      spr3dLook->DecRef ();
	      fact->DecRef ();
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_V:
                csScanStr (params2, "%f,%f,%f:%f,%f", &x, &y, &z, &u, &v);
                // check if it's the first frame
                if (spr3dLook->GetFrameCount () == 1)
                {
                  spr3dLook->AddVertices (1);
                }
                else if (i >= spr3dLook->GetVertexCount ())
                {
	          ReportError (reporter,
		    "crystalspace.sprite3dfactoryloader.parse.frame.vertices",
		    "Trying to add too many vertices to frame '%s'!",
		    fr->GetName ());
		  spr3dLook->DecRef ();
		  fact->DecRef ();
		  return NULL;
                }
                spr3dLook->SetVertex (anm_idx, i, csVector3 (x, y, z));
                spr3dLook->SetTexel  (tex_idx, i, csVector2 (u, v));
                i++;
                break;
            }
          }
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
          {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.badtoken",
		"Token '%s' not found while parsing frame '%s'!",
		csGetLastOffender (), fr->GetName ());
	    spr3dLook->DecRef ();
	    fact->DecRef ();
	    return NULL;
          }
          if (i < spr3dLook->GetVertexCount ())
          {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.vertices",
		"Too few vertices in frame '%s'!",
		fr->GetName ());
	    spr3dLook->DecRef ();
	    fact->DecRef ();
	    return NULL;
          }
        }
        break;

      case CS_TOKEN_TRIANGLE:
        {
          int a, b, c;
          csScanStr (params, "%d,%d,%d", &a, &b, &c);
          spr3dLook->AddTriangle (a, b, c);
        }
        break;

      case CS_TOKEN_SMOOTH:
        {
          int num, list[30];
          csScanStr (params, "%D", list, &num);
          switch (num)
          {
            case 0  :  spr3dLook->MergeNormals ();                  break;
            case 1  :  spr3dLook->MergeNormals (list[0]);           break;
            case 2  :  spr3dLook->MergeNormals (list[0], list[1]);  break;
	    default:
	      ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.badsmooth",
		"Bad smooth option '%s', use 0, 1, or 2 parameters!", params);
	      spr3dLook->DecRef ();
	      fact->DecRef ();
	      return NULL;
          }
        }
        break;

      case CS_TOKEN_TWEEN:
	{
	  bool do_tween;
          csScanStr (params, "%b", &do_tween);
          spr3dLook->EnableTweening (do_tween);
	}
	break;
    }
  }
  spr3dLook->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csSprite3DFactorySaver::csSprite3DFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csSprite3DFactorySaver::~csSprite3DFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
  if (reporter) reporter->DecRef ();
}

bool csSprite3DFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

static void WriteMixmode(csString& str, uint mixmode)
{
  str.Append("  MIXMODE (");
  if(mixmode&CS_FX_COPY) str.Append(" COPY ()");
  if(mixmode&CS_FX_ADD) str.Append(" ADD ()");
  if(mixmode&CS_FX_MULTIPLY) str.Append(" MULTIPLY ()");
  if(mixmode&CS_FX_MULTIPLY2) str.Append(" MULTIPLY2 ()");
  if(mixmode&CS_FX_KEYCOLOR) str.Append(" KEYCOLOR ()");
  if(mixmode&CS_FX_TILING) str.Append(" TILING ()");
  if(mixmode&CS_FX_TRANSPARENT) str.Append(" TRANSPARENT ()");
  if(mixmode&CS_FX_ALPHA) {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str.Append(buf);
  }
  str.Append(")");
}


/// helper function to write a matrix
static void WriteMatrix(const csMatrix3& m, csString& str)
{
  char buf[MAXLINE];
  sprintf(buf, "MATRIX (%g,%g,%g,%g,%g,%g,%g,%g,%g)", m.m11, m.m12, m.m13,
    m.m21, m.m22, m.m23, m.m31, m.m32, m.m33);
  str.Append(buf);
}


void csSprite3DFactorySaver::SaveSkeleton (iSkeletonLimb* limb,
  csString& str)
{
  iSkeletonConnection* con = SCF_QUERY_INTERFACE (limb, iSkeletonConnection);
  int i;
  char buf[MAXLINE];
  str.Append("VERTICES (");
  for(i=0; i<limb->GetVertexCount(); i++)
  {
    sprintf(buf, "%d%s", limb->GetVertices()[i],
      (i==limb->GetVertexCount()-1)?"":",");
    str.Append(buf);
  }
  str.Append(")\n");

  str.Append("TRANSFORM (");
  WriteMatrix(con->GetTransformation().GetO2T(), str);
  sprintf(buf, " V(%g,%g,%g))", con->GetTransformation().GetO2TTranslation().x,
    con->GetTransformation().GetO2TTranslation().y,
    con->GetTransformation().GetO2TTranslation().z);
  str.Append(buf);

  iSkeletonLimb *ch = limb->GetChildren();
  while(ch)
  {
    sprintf(buf, "LIMB '%s' (", ch->GetName());
    str.Append(buf);
    SaveSkeleton(ch, str);
    str.Append(")\n");
    ch = ch->GetNextSibling();
  }

  con->DecRef();
}

void csSprite3DFactorySaver::WriteDown (iBase* obj, iFile * file)
{
  csString str;
  iSprite3DFactoryState *state =
    SCF_QUERY_INTERFACE (obj, iSprite3DFactoryState);
  char buf[MAXLINE];

  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str.Append(buf);

  int i,j;
  for(i=0; i<state->GetFrameCount(); i++)
  {
    iSpriteFrame* frame = state->GetFrame(i);
    sprintf(buf, "FRAME '%s' (\n", frame->GetName());
    str.Append(buf);
    int anm_idx = frame->GetAnmIndex ();
    int tex_idx = frame->GetTexIndex ();
    for(j=0; j<state->GetVertexCount(); j++)
    {
      sprintf(buf, "  V(%g,%g,%g:%g,%g)\n", state->GetVertex(anm_idx, j).x,
        state->GetVertex(anm_idx, j).y, state->GetVertex(anm_idx, j).z,
	state->GetTexel(tex_idx, j).x, state->GetTexel(tex_idx, j).y);
      str.Append(buf);
    }
    str.Append(")\n");
  }

  for(i=0; i<state->GetActionCount(); i++)
  {
    iSpriteAction* action = state->GetAction(i);
    sprintf(buf, "ACTION '%s' (", action->GetName());
    str.Append(buf);
    for(j=0; j<action->GetFrameCount(); j++)
    {
      sprintf(buf, " F(%s,%d)", action->GetFrame(j)->GetName(),
        action->GetFrameDelay(j));
      str.Append(buf);
    }
    str.Append(")\n");
  }

  for(i=0; i<state->GetTriangleCount(); i++)
  {
    sprintf(buf, "TRIANGLE (%d,%d,%d)\n", state->GetTriangle(i).a,
      state->GetTriangle(i).b, state->GetTriangle(i).c);
    str.Append(buf);
  }

  iSkeleton *skeleton = state->GetSkeleton();
  if(skeleton)
  {
    iSkeletonLimb* skellimb = SCF_QUERY_INTERFACE (skeleton, iSkeletonLimb);
    iSkeletonLimb* skelp = skellimb;
    while(skelp)
    {
      sprintf(buf, "SKELETON '%s' (\n", skelp->GetName());
      str.Append(buf);
      SaveSkeleton(skelp, str);
      str.Append(")\n");
      skelp = skelp->GetNextSibling();
    }
    skellimb->DecRef();
  }

  /// @@@ Cannot retrieve smoothing information.
  /// SMOOTH()
  /// SMOOTH(baseframenr)
  /// or a list of SMOOTH(basenr, framenr);

  sprintf(buf, "TWEEN (%s)\n", state->IsTweeningEnabled()?"true":"false");
  str.Append(buf);

  state->DecRef();
  file->Write ((const char*)str, str.Length ());
}

//---------------------------------------------------------------------------
csSprite3DLoader::csSprite3DLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csSprite3DLoader::~csSprite3DLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  if (reporter) reporter->DecRef ();
}

bool csSprite3DLoader::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

iBase* csSprite3DLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ACTION)
    CS_TOKEN_TABLE (APPLY_MOTION)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (BASECOLOR)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (TWEEN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iSprite3DState* spr3dLook = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.sprite3dloader.parse.badformat",
		"Bad format while parsing sprite3d!");
      if (spr3dLook) spr3dLook->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.unknownfactory",
		"Couldn't find factory '%s'!", str);
	    if (spr3dLook) spr3dLook->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          spr3dLook = SCF_QUERY_INTERFACE (mesh, iSprite3DState);
	}
	break;
      case CS_TOKEN_ACTION:
        csScanStr (params, "%s", str);
	spr3dLook->SetAction (str);
        break;
      case CS_TOKEN_BASECOLOR:
	{
	  csColor col;
          csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  spr3dLook->SetBaseColor (col);
	}
        break;
      case CS_TOKEN_LIGHTING:
	{
	  bool do_lighting;
          csScanStr (params, "%b", &do_lighting);
	  spr3dLook->SetLighting (do_lighting);
	}
        break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.unknownmaterial",
		"Couldn't find material '%s'!", str);
            mesh->DecRef ();
	    if (spr3dLook) spr3dLook->DecRef ();
            return NULL;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        {
	  uint mm = ParseMixmode (reporter, params);
	  if (mm == (uint)~0)
	  {
	    if (spr3dLook) spr3dLook->DecRef ();
	    mesh->DecRef ();
	    return NULL;
	  }
          spr3dLook->SetMixMode (mm);
	}
	break;
      case CS_TOKEN_APPLY_MOTION:
	{
	  csScanStr (params, "%s", str);
	  iMotionManager *motman = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
		"crystalspace.motion.manager.default",
		iMotionManager);
	  if (!motman)
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.setup.motion.motionmanager",
		"Could not find motion manager!");
	    return NULL;
	  }
	  motman->DecRef();
	  if (!spr3dLook)
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.motion.missingfactory",
		"No Factory! Please define 'FACTORY' before 'APPLY_MOTION'!");
	    return NULL;
	  }
	  iSkeletonState *skel_state = spr3dLook->GetSkeletonState();
	  iSkeletonLimbState *limb = SCF_QUERY_INTERFACE (skel_state,
	  	iSkeletonLimbState );
	  limb->DecRef();
	  if (!(limb = limb->GetChildren()))
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.motion.nochildren",
		"Skeleton has no libs. Cannot apply motion!");
	    return NULL;
	  }
	  iSkeletonConnectionState *con = SCF_QUERY_INTERFACE (limb,
	  	iSkeletonConnectionState );
	  iSkeletonBone *bone = SCF_QUERY_INTERFACE (con, iSkeletonBone);
	  if (!bone)
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.motion.nobones",
		"The skeleton has no bones!");
	    return NULL;
	  }
          iMotionTemplate* motion=motman->FindMotionByName(str);
	  if (!motion)
	  {
      	    ReportError (reporter,
		"crystalspace.sprite3dloader.parse.motion.nomotion",
		"The motion '%s' does not exist!", str);
	    return NULL;
	  }
          iMotionController* mc=motman->AddController(bone);
          mc->SetMotion(motion);
	}
	break;
      case CS_TOKEN_TWEEN:
	{
	  bool do_tween;
          csScanStr (params, "%b", &do_tween);
          spr3dLook->EnableTweening (do_tween);
	}
	break;

    }
  }

  if (spr3dLook) spr3dLook->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csSprite3DSaver::csSprite3DSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csSprite3DSaver::~csSprite3DSaver ()
{
  SCF_DEC_REF (plugin_mgr);
  if (reporter) reporter->DecRef ();
}

bool csSprite3DSaver::Initialize (iObjectRegistry* object_reg)
{
  csSprite3DSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

void csSprite3DSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iSprite3DState *state = SCF_QUERY_INTERFACE (obj, iSprite3DState);

  iMeshObject *meshobj= SCF_QUERY_INTERFACE (obj, iMeshObject);
  iSprite3DFactoryState *factstate = SCF_QUERY_INTERFACE (
    meshobj->GetFactory(), iSprite3DFactoryState);
  meshobj->DecRef();
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);

  if(state->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, state->GetMixMode());
  }
  if(state->GetMaterialWrapper() != factstate->GetMaterialWrapper())
  {
    sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
      QueryObject ()->GetName());
    str.Append(buf);
  }
  sprintf(buf, "LIGHTING (%s)\n", state->IsLighting ()?"true":"false");
  str.Append(buf);
  sprintf(buf, "TWEEN (%s)\n", state->IsTweeningEnabled()?"true":"false");
  str.Append(buf);
  if(state->GetCurAction())
  {
    sprintf(buf, "ACTION (%s)\n", state->GetCurAction()->GetName());
    str.Append(buf);
  }
  fact->DecRef();
  factstate->DecRef();
  state->DecRef();
  file->Write ((const char*)str, str.Length ());
}
