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
#include "spr3dldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "imspr3d.h"
#include "iskel.h"
#include "igraph3d.h"
#include "qint.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (ACTION)
  CS_TOKEN_DEF (F)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FRAME)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (LIMB)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MIXMODE)
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

IMPLEMENT_IBASE (csSprite3DFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSprite3DLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSprite3DFactoryLoader)
IMPLEMENT_FACTORY (csSprite3DLoader)

EXPORT_CLASS_TABLE (spr3dldr)
  EXPORT_CLASS (csSprite3DFactoryLoader, "crystalspace.mesh.loader.factory.sprite.3d",
    "Crystal Space Sprite3D Mesh Factory Loader")
  EXPORT_CLASS (csSprite3DLoader, "crystalspace.mesh.loader.sprite.3d",
    "Crystal Space Sprite3D Mesh Loader")
EXPORT_CLASS_TABLE_END

csSprite3DFactoryLoader::csSprite3DFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite3DFactoryLoader::~csSprite3DFactoryLoader ()
{
}

bool csSprite3DFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

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
        //else
	  //@@@ Error handling!: CsPrintf (MSG_WARNING, "Badly formed rotation: '%s'\n", params);
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
        //else
	  //@@@ Error handling!: CsPrintf (MSG_WARNING, "Badly formed scale: '%s'\n", params);
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
    //else
      //@@@ Error handling!: CsPrintf (MSG_WARNING, "Badly formed matrix '%s'\n", buf);
  }
  return true;
}

static bool load_vector (char* buf, csVector3 &v)
{
  ScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
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
    printf ("Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

bool csSprite3DFactoryLoader::LoadSkeleton (iSkeletonLimb* limb, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (LIMB)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (TRANSFORM)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  char* xname;

  long cmd;
  char* params;

  iSkeletonConnection* con = QUERY_INTERFACE (limb, iSkeletonConnection);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      //@@@ Error handling!
      //CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      con->DecRef ();
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_LIMB:
        {
          iSkeletonConnection* newcon = limb->CreateConnection ();
	  iSkeletonLimb* newlimb = QUERY_INTERFACE (newcon, iSkeletonLimb);
	  if (name) newlimb->SetName (name);
	  if (!LoadSkeleton (newlimb, params))
	  {
	    //@@@
	    con->DecRef ();
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
          while ((cmd = csGetObject (&params, tok_matvec, &xname, &params2)) > 0)
          {
    	    if (!params2)
    	    {
	      //@@@ Error handling!
      	      //CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	      //fatal_exit (0, false);
	      con->DecRef ();
	      return false;
    	    }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
                load_matrix (params2, m);
		break;
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
	  //@@@ Error handling!
	  //CsPrintf (MSG_FATAL_ERROR, "TRANSFORM not valid for this type of skeleton limb!\n");
	  //fatal_exit (0, false);
	  con->DecRef ();
	  return false;
	}
	break;
      case CS_TOKEN_VERTICES:
        {
          int list[1000], num;	//@@@ HARDCODED!!!
          ScanStr (params, "%D", list, &num);
          for (int i = 0 ; i < num ; i++) limb->AddVertex (list[i]);
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    //@@@ Error handling!
    //CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite skeleton!\n",
        //csGetLastOffender ());
    //fatal_exit (0, false);
    con->DecRef ();
    return false;
  }

  con->DecRef ();
  return true;
}

iBase* csSprite3DFactoryLoader::Parse (const char* string, iEngine* engine)
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

  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.sprite.3d\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  iSprite3DFactoryState* spr3dLook = QUERY_INTERFACE (fact, iSprite3DFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      printf ("No params!\n");
      fact->DecRef ();
      spr3dLook->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
	    printf ("Can't find material!\n");
            fact->DecRef ();
	    spr3dLook->DecRef ();
            return NULL;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_SKELETON:
	{
	  spr3dLook->EnableSkeletalAnimation ();
	  iSkeleton* skeleton = spr3dLook->GetSkeleton ();
	  iSkeletonLimb* skellimb = QUERY_INTERFACE (skeleton, iSkeletonLimb);
	  if (name) skellimb->SetName (name);
	  if (!LoadSkeleton (skellimb, params))
	  {
	    // @@@ Error handling!
	    printf ("Bad skeleton!\n");
	    fact->DecRef ();
	    spr3dLook->DecRef ();
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
          while ((cmd = csGetObject (&params, tok_frameset, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      //@@@ Error handling!
              //CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
	      printf ("Bad action!\n");
	      fact->DecRef ();
	      spr3dLook->DecRef ();
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_F:
                ScanStr (params2, "%s,%d", fn, &d);
                iSpriteFrame* ff = spr3dLook->FindFrame (fn);
                if (!ff)
                {
	          //@@@ Error handling!
                  //CsPrintf (MSG_FATAL_ERROR, "Error! Trying to add a unknown frame '%s' in %s action !\n",
                        //fn, act->GetName ());
		  printf ("Bad frame!\n");
		  fact->DecRef ();
		  spr3dLook->DecRef ();
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
	      //@@@ Error handling!
              //CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
	      printf ("Error\n");
	      fact->DecRef ();
	      spr3dLook->DecRef ();
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_V:
                ScanStr (params2, "%f,%f,%f:%f,%f", &x, &y, &z, &u, &v);
                // check if it's the first frame
                if (spr3dLook->GetNumFrames () == 1)
                {
                  spr3dLook->AddVertices (1);
                }
                else if (i >= spr3dLook->GetNumTexels ())
                {
		  //@@@ Error handling!
                  //CsPrintf (MSG_FATAL_ERROR, "Error! Trying to add too many vertices in frame '%s'!\n",
                    //fr->GetName ());
	      	  printf ("Too many vertices!\n");
		  fact->DecRef ();
		  spr3dLook->DecRef ();
		  return NULL;
                }
                spr3dLook->GetVertex (anm_idx, i) = csVector3 (x, y, z);
                spr3dLook->GetTexel  (tex_idx, i) = csVector2 (u, v);
                i++;
                break;
            }
          }
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
          {
	    //@@@ Error handling!
            //CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing frame '%s'!\n",
                //fr->GetName (), csGetLastOffender ());
	    printf ("Token not found!\n");
	    fact->DecRef ();
	    spr3dLook->DecRef ();
	    return NULL;
          }
          if (i < spr3dLook->GetNumTexels ())
          {
	    //@@@ Error handling!
            //CsPrintf (MSG_FATAL_ERROR, "Error! Too few vertices in frame '%s'! (%d %d)\n",
                //fr->GetName (), i, state->GetNumTexels ());
	    printf ("Too few vertices!\n");
	    fact->DecRef ();
	    spr3dLook->DecRef ();
	    return NULL;
          }
        }
        break;

      case CS_TOKEN_TRIANGLE:
        {
          int a, b, c;
          ScanStr (params, "%d,%d,%d", &a, &b, &c);
          spr3dLook->AddTriangle (a, b, c);
        }
        break;

      case CS_TOKEN_SMOOTH:
        {
          int num, list[30];
          ScanStr (params, "%D", list, &num);
          switch (num)
          {
            case 0  :  spr3dLook->MergeNormals ();                  break;
            case 1  :  spr3dLook->MergeNormals (list[0]);           break;
            case 2  :  spr3dLook->MergeNormals (list[0], list[1]);  break;
            //default :  CsPrintf (MSG_WARNING, "Confused by SMOOTH options: '%s'\n", params);
                       //CsPrintf (MSG_WARNING, "no smoothing performed\n");
          }
        }
        break;

      case CS_TOKEN_TWEEN:
	{
	  bool do_tween;
          ScanStr (params, "%b", &do_tween);
          spr3dLook->EnableTweening (do_tween);
	}
	break;
    }
  }
  spr3dLook->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csSprite3DLoader::csSprite3DLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite3DLoader::~csSprite3DLoader ()
{
}

bool csSprite3DLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csSprite3DLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ACTION)
    CS_TOKEN_TABLE (FACTORY)
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
      // @@@ Error handling!
      if (spr3dLook) spr3dLook->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
	{
          ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    printf ("Can't find factory!\n");
	    if (spr3dLook) spr3dLook->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          spr3dLook = QUERY_INTERFACE (mesh, iSprite3DState);
	}
	break;
      case CS_TOKEN_ACTION:
        ScanStr (params, "%s", str);
	spr3dLook->SetAction (str);
        break;
      case CS_TOKEN_MATERIAL:
	{
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
	    printf ("Can't find material!\n");
            mesh->DecRef ();
	    if (spr3dLook) spr3dLook->DecRef ();
            return NULL;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        spr3dLook->SetMixMode (ParseMixmode (params));
	break;
      case CS_TOKEN_TWEEN:
	{
	  bool do_tween;
          ScanStr (params, "%b", &do_tween);
          spr3dLook->EnableTweening (do_tween);
	}
	break;

    }
  }

  if (spr3dLook) spr3dLook->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

