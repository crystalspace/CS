/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Ivan Avramovic <ivan@avramovic.com>
  
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
#include "csparser/csloader.h"
#include "csengine/sysitf.h"
#include "csengine/basic/cscoll.h"
#include "csengine/basic/triangle.h"
#include "csengine/sector.h"
#include "csengine/objects/thing.h"
#include "csengine/objects/thingtpl.h"
#include "csengine/objects/cssprite.h"
#include "csengine/polygon/polygon.h"
#include "csengine/light/dynlight.h"
#include "csengine/library.h"
#include "csengine/textrans.h"
#include "csengine/world.h"
#include "csengine/light/light.h"
#include "csengine/texture.h"
#include "csengine/curve.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/token.h"
#include "csutil/archive.h"
#include "csutil/inifile.h"
#include "cssndldr/sndload.h"
#include "csparser/sndbufo.h"
#include "csgfxldr/csimage.h"
#include "csobject/nameobj.h"
#include "csengine/scripts/objtrig.h"


//---------------------------------------------------------------------------

int CSLoader::LoadStat::polygons_loaded = 0;
int CSLoader::LoadStat::portals_loaded = 0;
int CSLoader::LoadStat::sectors_loaded = 0;
int CSLoader::LoadStat::things_loaded = 0;
int CSLoader::LoadStat::lights_loaded = 0;
int CSLoader::LoadStat::curves_loaded = 0;
int CSLoader::LoadStat::sprites_loaded = 0;

//---------------------------------------------------------------------------

csMatrix3 CSLoader::load_matrix (char* buf)
{
  if (!strcmp (buf, "IDENTITY")) return csMatrix3(); /* return the identity */

  if (!strncmp (buf, "ROT", 3))
  {
    static tokenDesc tok_cmd[] = {{1, "ROT_X"}, {2, "ROT_Y"}, 
                                  {3, "ROT_Z"}, {0,0}};
    char* params;
    int cmd;
    csMatrix3 M,N;
    float angle;
    while ((cmd = csGetCommand (&buf, tok_cmd, &params)) > 0)
    {
      switch (cmd)
      {
        case 1:
	  ScanStr (params, "%f", &angle);
  	  N.m11 = 1; N.m12 = 0;           N.m13 = 0;
  	  N.m21 = 0; N.m22 = cos (angle); N.m23 = -sin(angle);
  	  N.m31 = 0; N.m32 = sin (angle); N.m33 =  cos(angle);
	  M *= N;
	  break;
        case 2:
	  ScanStr (params, "%f", &angle);
  	  N.m11 = cos (angle); N.m12 = 0; N.m13 = -sin(angle);
  	  N.m21 = 0;           N.m22 = 1; N.m23 = 0;
  	  N.m31 = sin (angle); N.m32 = 0; N.m33 =  cos(angle);
	  M *= N;
	  break;
        case 3:
	  ScanStr (params, "%f", &angle);
  	  N.m11 = cos (angle); N.m12 = -sin(angle); N.m13 = 0;
  	  N.m21 = sin (angle); N.m22 =  cos(angle); N.m23 = 0;
  	  N.m31 = 0;           N.m32 = 0;           N.m33 = 1;
	  M *= N;
	  break;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a matrix!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
    return M;
  }

  float list[30];
  int num;
  ScanStr (buf, "%F", list, &num);
  if (num == 1) return csMatrix3() * list[0];
  else if (num == 9)
    return csMatrix3(list[0], list[1], list[2],
                   list[3], list[4], list[5],
                   list[6], list[7], list[8]);
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "Badly formed matrix!\n");
    fatal_exit (0, false);
  }
  return csMatrix3();
}


csVector3 CSLoader::load_vector (char* buf)
{ 
  float x,y,z;
  ScanStr (buf, "%f,%f,%f", &x, &y, &z); 
  return csVector3(x,y,z);
}

//---------------------------------------------------------------------------

enum { kTokenPlaneOrig = 1, kTokenPlaneFirst, kTokenPlaneFirstLen,
       kTokenPlaneSecond, kTokenPlaneSecondLen, kTokenPlaneMatrix,
       kTokenPlaneVector };

csPolyPlane* CSLoader::load_polyplane (char* buf, char* name)
{
  static tokenDesc commands[] = {
	{kTokenPlaneOrig, "ORIG"},
	{kTokenPlaneFirstLen, "FIRST_LEN"},
	{kTokenPlaneFirst, "FIRST"},
	{kTokenPlaneSecondLen, "SECOND_LEN"},
	{kTokenPlaneSecond, "SECOND"},
	{kTokenPlaneMatrix, "MATRIX"},
	{kTokenPlaneVector, "V"},
	{0,0}};
  char* xname;
  long cmd;
  char* params;
  CHK( csPolyPlane* ppl = new csPolyPlane() );
  csNameObject::AddName(*ppl, name); 

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig, tx1, tx2;
  float tx1_len = 0, tx2_len = 0;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenPlaneOrig:
        tx1_given = true;
        tx1_orig = CSLoader::load_vector (params);
	break;
      case kTokenPlaneFirst:
        tx1_given = true;
        tx1 = CSLoader::load_vector (params);
	break;
      case kTokenPlaneFirstLen:
	ScanStr (params, "%f", &tx1_len);
        tx1_given = true;
	break;
      case kTokenPlaneSecond:
        tx2_given = true;
        tx2 = CSLoader::load_vector (params);
	break;
      case kTokenPlaneSecondLen:
	ScanStr (params, "%f", &tx2_len);
        tx2_given = true;
	break;
      case kTokenPlaneMatrix:
        tx_matrix = CSLoader::load_matrix (params);
	break;
      case kTokenPlaneVector:
        tx_vector = CSLoader::load_vector (params);
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a plane!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (tx1_given)
    if (tx2_given) ppl->SetTextureSpace (tx1_orig.x, tx1_orig.y, tx1_orig.z,
		    		          tx1.x, tx1.y, tx1.z, tx1_len,
				          tx2.x, tx2.y, tx2.z, tx2_len);
    else { CsPrintf (MSG_FATAL_ERROR, "Not supported!\n"); fatal_exit (0, false); }
  else ppl->SetTextureSpace (tx_matrix, tx_vector);

  return ppl;
}

//---------------------------------------------------------------------------

enum { kTokenNone, kTokenActive, kTokenStateless, kTokenPrimActive,
       kTokenSecActive, kTokenBecActive, kTokenPrimInactive,
       kTokenSecInactive, kTokenBecInactive };

void CSLoader::load_light (char* name, char* buf)
{
  static tokenDesc commands[] = {{kTokenActive, "ACTIVE"},
                                 {kTokenStateless, "STATELESS"},
                                 {kTokenPrimActive, "PRIMARY_ACTIVE"},
                                 {kTokenSecActive, "SECONDARY_ACTIVE"},
                                 {kTokenBecActive, "BECOMING_ACTIVE"},
                                 {kTokenPrimInactive, "PRIMARY_INACTIVE"},
                                 {kTokenSecInactive, "SECONDARY_INACTIVE"},
                                 {kTokenBecInactive, "BECOMING_INACTIVE"},
                                 {0,0}};
  CHK (CLights *theLite = new CLights());
  csNameObject::AddName(*theLite,name);

  long    cmd;
  char    *params;
  int             state, theType, thePeriod, dp, intensity, di;
  while ((cmd = csGetCommand(&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenActive:
        sscanf(params, "%d", &state);
        theLite->SetInitallyActive(state);
        break;
      case kTokenStateless:
        sscanf(params, "%d", &state);
        theLite->SetStateType(state);
        break;
      case kTokenPrimActive:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStatePrimaryActive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case kTokenSecActive:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateSecondaryActive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case kTokenBecActive:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateBecomingActive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case kTokenPrimInactive:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStatePrimaryInactive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case kTokenSecInactive:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateSecondaryInactive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case kTokenBecInactive:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                        &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateBecomingInactive, theType,
                                 thePeriod, dp, intensity, di);
        break;
    }
  }
  // start the light
  theLite->Start();
}

//---------------------------------------------------------------------------

enum { kTokenColThing = 1, kTokenColCol, kTokenColLight,
       kTokenColTrigger, kTokenColSector };

csCollection* CSLoader::load_collection (char* name, csWorld* w, char* buf)
{
  static tokenDesc commands[] = {
  	{kTokenColThing, "THING"},
  	{kTokenColCol, "COLLECTION"},
  	{kTokenColLight, "LIGHT"},
  	{kTokenColTrigger, "TRIGGER"},
  	{kTokenColSector, "SECTOR"},
	{0,0}};
  char* xname;
  long cmd;
  char* params;

  CHK( csCollection* collection = new csCollection() ); 
  csNameObject::AddName(*collection, name);

  char str[255];
  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenColThing:
	{
	  ScanStr (params, "%s", str);
          csThing* th = w->GetThing (str);
          if (!th)
          {
	    CsPrintf (MSG_FATAL_ERROR, "Thing '%s' not found!\n", str);
	    fatal_exit (0, false);
          }
          collection->AddObject ((csObject*)th);
	}
	break;
      case kTokenColLight:
        {
	  int nr;
	  ScanStr (params, "%s,%d", str, &nr);
          csSector* s = (csSector*)w->sectors.FindByName (str);
          if (!s)
          {
	    CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found!\n", str);
	    fatal_exit (0, false);
          }
          csStatLight* l = (csStatLight*)s->lights[nr];
          collection->AddObject ((csObject*)l);
        }
	break;
      case kTokenColSector:
        {
	  int nr;
	  ScanStr (params, "%s,%d", str, &nr);
          csSector* s = (csSector*)w->sectors.FindByName (str);
          if (!s)
          {
	    CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found!\n", str);
	    fatal_exit (0, false);
          }
          collection->AddObject ((csObject*)s);
        }
	break;
      case kTokenColCol:
        {
	  ScanStr (params, "%s", str);
          csCollection* th = (csCollection*)w->collections.FindByName (str);
          if (!th)
          {
	    CsPrintf (MSG_FATAL_ERROR, "Collection '%s' not found!\n", str);
	    fatal_exit (0, false);
          }
          collection->AddObject (th);
	}
      case kTokenColTrigger:
        {
  	  char str2[255];
  	  char str3[255];
	  ScanStr (params, "%s,%s->%s", str, str2, str3);
          csObject* cs = collection->FindObject (str);
          if (!cs)
          {
	    CsPrintf (MSG_FATAL_ERROR, "Object '%s' not found!\n", str);
	    fatal_exit (0, false);
          }

          if (!strcmp (str2, "activate"))
          {
	    csScript* s = (csScript*)w->scripts.FindByName (str3);
	    if (!s)
	    {
	      CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str3);
	      fatal_exit (0, false);
	    }
            csObjectTrigger *objtrig = csObjectTrigger::GetTrigger(*cs);
            if (!objtrig)
            {
              CHK(objtrig = new csObjectTrigger());
              cs->ObjAdd(objtrig);
            }
            objtrig->NewActivateTrigger(s,collection);
          }
          else
          {
	    CsPrintf (MSG_FATAL_ERROR, 
                      "Trigger '%s' not supported or known for object '%s'!\n",
                      str2, xname);
	    fatal_exit (0, false);
          }
        }
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a collection!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return collection;
}

//---------------------------------------------------------------------------

enum { kTokenLightCenter = 1, kTokenLightRadius, kTokenLightDynamic,
       kTokenLightColor, kTokenLightHalo };

csStatLight* CSLoader::load_statlight (char* buf)
{
  static tokenDesc commands[] = {
        {kTokenLightCenter, "CENTER"},
        {kTokenLightRadius, "RADIUS"},
        {kTokenLightDynamic, "DYNAMIC"},
        {kTokenLightColor, "COLOR"},
        {kTokenLightHalo, "HALO"},
        {0,0}};
  long cmd;
  char* params;

  LoadStat::lights_loaded++;
  float x, y, z, dist, r, g, b;
  int dyn;
  bool halo = false;

  if (strchr (buf, ':'))
  {
    // Still support old format for backwards compatibility.
    ScanStr (buf, "%f,%f,%f:%f,%f,%f,%f,%d",
   	  &x, &y, &z, &dist, &r, &g, &b, &dyn);
    halo = false;
  }
  else
  {
    // New format.
    x = y = z = 0;
    dist = 1;
    r = g = b = 1;
    dyn = 0;
    while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
    {
      switch (cmd)
      {
        case kTokenLightRadius:
          ScanStr (params, "%f", &dist);
          break;
        case kTokenLightCenter:
          ScanStr (params, "%f,%f,%f", &x, &y, &z);
	  break;
        case kTokenLightColor:
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
	  break;
        case kTokenLightDynamic:
	  dyn = 1;
	  break;
        case kTokenLightHalo:
	  halo = true;
	  break;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a light!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  CHK (csStatLight* l = new csStatLight (x, y, z, dist, r, g, b, dyn));
  if (halo) l->EnableHalo ();
  return l;
}

//---------------------------------------------------------------------------

csPolygonSet& CSLoader::ps_process (csPolygonSet& ps, PSLoadInfo& info, int cmd, 
                                  char* name, char* params)
{
  char str[255], str2[255];
  switch (cmd)
  {
    case kTokenPSetVertex:
      {
        float x, y, z;
        ScanStr (params, "%f,%f,%f", &x, &y, &z);
        ps.AddVertex (x, y, z);
      }
      break;
    case kTokenPSetCircle:
      {
        float x, y, z, rx, ry, rz;
	int num, dir;
        ScanStr (params, "%f,%f,%f:%f,%f,%f,%d", &x, &y, &z, &rx, &ry, &rz, &num);
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
	  if      (ABS (rx) < SMALL_EPSILON) { cx = x; cy = y+cc*ry; cz = z+ss*rz; }
	  else if (ABS (ry) < SMALL_EPSILON) { cy = y; cx = x+cc*rx; cz = z+ss*rz; }
	  else if (ABS (rz) < SMALL_EPSILON) { cz = z; cx = x+cc*rx; cy = y+ss*ry; }
	  ps.AddVertex (cx, cy, cz);
	}
      }
      break;
    case kTokenPSetFog:
      {
	csFog& f = ps.GetFog ();
	f.enabled = true;
        ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
      }
      break;
    case kTokenPSetPolygon:
      ps.AddPolygon ( CSLoader::load_poly3d(name, info.w, params,
                      info.textures, info.default_texture, info.default_texlen,
                      info.default_lightx, ps.GetSector (), &ps) );
      LoadStat::polygons_loaded++;
      break;

    case kTokenPSetBezier:
      //CsPrintf(MSG_WARNING,"Encountered curve!\n");
      ps.AddCurve ( CSLoader::load_bezier(name, info.w, params,
                      info.textures, info.default_texture, info.default_texlen,
                      info.default_lightx, ps.GetSector (), &ps) );
      LoadStat::curves_loaded++;
      break;

    case kTokenPSetTexNr:
      ScanStr (params, "%s", str);
      info.default_texture = info.textures->GetTextureMM (str);
      if (info.default_texture == NULL)
      {
        CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", str);
	fatal_exit (0, false);
      }
      break;
    case kTokenPSetTexlen:
      ScanStr (params, "%f", &info.default_texlen);
      break;
    case kTokenPSetLightX:
      ScanStr (params, "%s", str);
      info.default_lightx = CLights::FindByName (str);
      break;
    case kTokenPSetActivate:
      ScanStr (params, "%s", str);
      {
        csScript* s = (csScript*)info.w->scripts.FindByName (str);
        if (!s)
        {
	  CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str);
	  fatal_exit (0, false);
        }
        csObjectTrigger* objtrig = csObjectTrigger::GetTrigger(ps);
        if (!objtrig)
        {
          CHK(objtrig = new csObjectTrigger());
          ps.ObjAdd(objtrig);
        }
        objtrig->NewActivateTrigger (s);
	objtrig->DoActivateTriggers ();
      }
      break;
    case kTokenPSetTrigger:
      ScanStr (params, "%s,%s", str, str2);
      if (!strcmp (str, "activate"))
      {
        csScript* s = (csScript*)info.w->scripts.FindByName (str2);
        if (!s)
        {
	  CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str2);
	  fatal_exit (0, false);
        }
        csObjectTrigger *objtrig = csObjectTrigger::GetTrigger(ps);
        if (!objtrig) 
        {
          CHK(objtrig = new csObjectTrigger());
          ps.ObjAdd(objtrig);
        }
        objtrig->NewActivateTrigger(s);
      }
      else
      {
	CsPrintf (MSG_FATAL_ERROR, 
                  "Trigger '%s' not supported or known for object '%s'!\n", 
                  str, csNameObject::GetName(ps));
	fatal_exit (0, false);
      }
      break;
    case kTokenPSetBsp:
      info.use_bsp = true;
      break;
  }
  return ps;
}

//---------------------------------------------------------------------------

enum { kTokenSixMove = 1, kTokenSixTexture, kTokenSixTexCeil,
       kTokenSixTexFloor, kTokenSixTexScale, kTokenSixDim,
       kTokenSixFlHeight, kTokenSixHeight, kTokenSixFloor,
       kTokenSixFloorCeil, kTokenSixCeil, kTokenSixTrigger,
       kTokenSixActivate, kTokenSixMoveable, kTokenSixFog,
       kTokenSixConvex };

typedef char ObName[30];

csThing* CSLoader::load_sixface (char* name, csWorld* w, char* buf,
                               csTextureList* textures, csSector* sec)
{
  static tokenDesc commands[] = {
  	{kTokenSixMoveable, "MOVEABLE"},
  	{kTokenSixMove, "MOVE"},
	{kTokenSixTexScale, "TEXTURE_SCALE"}, 
	{kTokenSixTexture, "TEXTURE"}, 
	{kTokenSixTexCeil, "CEIL_TEXTURE"}, 
	{kTokenSixDim, "DIM"}, 
	{kTokenSixHeight, "HEIGHT"}, 
	{kTokenSixFlHeight, "FLOOR_HEIGHT"}, 
	{kTokenSixFloorCeil, "FLOOR_CEIL"}, 
	{kTokenSixTexFloor, "FLOOR_TEXTURE"}, 
	{kTokenSixFloor, "FLOOR"}, 
	{kTokenSixCeil, "CEILING"}, 
	{kTokenSixTrigger, "TRIGGER"}, 
	{kTokenSixActivate, "ACTIVATE"},
	{kTokenSixFog, "FOG"},
	{kTokenSixConvex, "CONVEX"},
	{0,0}};
  static tokenDesc tok_matvec[] = {{1, "MATRIX"}, {2, "V"}, {0,0}};
  char* xname;

  CHK( csThing* thing = new csThing() );
  csNameObject::AddName(*thing,name);

  LoadStat::things_loaded++;

  thing->SetSector (sec);
  csReversibleTransform obj;
  csTextureHandle* texture = NULL;
  bool is_convex = false;
  float tscale = 1;
  int i;

  csVector3 v[8];
  for (i = 0;  i < 8;  i++)
   v[i] = csVector3( (i&1 ? 1 : -1), (i&2 ? -1 : 1), (i&4 ? -1 : 1) );
  float r;

  char str[255];
  char str2[255];
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenSixConvex:
        is_convex = true;
	break;
      case kTokenSixFog:
        {
	  csFog& f = thing->GetFog ();
	  f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
	break;
      case kTokenSixMoveable:
        thing->SetMoveable (true);
	break;
      case kTokenSixMove:
        {
	  char* params2;
          obj = csReversibleTransform(); // identity transform
	  while ((cmd = csGetObject (&params, tok_matvec, &xname, &params2)) > 0)
	  {
	    switch (cmd)
	    {
	      case 1: 
                obj.SetT2O ( CSLoader::load_matrix(params2) );  break;
	      case 2:
                obj.SetOrigin ( CSLoader::load_vector(params2) );  break;
	    }
	  }
	}
	break;
      case kTokenSixTexture:
	ScanStr (params, "%s", str);
        texture = textures->GetTextureMM (str);
        if (texture == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
	break;
      case kTokenSixTexScale:
	ScanStr (params, "%f", &tscale);
	break;
      case kTokenSixDim:
	{
	  float rx, ry, rz;
	  ScanStr (params, "%f,%f,%f", &rx, &ry, &rz);
	  rx /= 2; ry /= 2; rz /= 2;
          for (i = 0;  i < 8;  i++)
           v[i] = csVector3((i&1 ? rx : -rx),(i&2 ? -ry : ry),(i&4 ? -rz : rz));
	}
	break;
      case kTokenSixFlHeight:
	ScanStr (params, "%f", &r);
        v[0].y = r+v[0].y-v[2].y;
        v[1].y = r+v[1].y-v[3].y;
        v[4].y = r+v[4].y-v[6].y;
        v[5].y = r+v[5].y-v[7].y;
        v[2].y = r;
        v[3].y = r;
        v[6].y = r;
        v[7].y = r;
	break;
      case kTokenSixHeight:
	ScanStr (params, "%f", &r);
        v[0].y = r+v[2].y;
        v[1].y = r+v[3].y;
        v[4].y = r+v[6].y;
        v[5].y = r+v[7].y;
	break;
      case kTokenSixFloorCeil:
	ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
	  	 &v[2].x, &v[2].z, &v[3].x, &v[3].z, 
                 &v[7].x, &v[7].z, &v[6].x, &v[6].z);
        v[0] = v[2];
	v[1] = v[3];
	v[5] = v[7];
	v[4] = v[6];
	break;
      case kTokenSixFloor:
	ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
	  	 &v[2].x, &v[2].y, &v[2].z, &v[3].x, &v[3].y, &v[3].z, 
                 &v[7].x, &v[7].y, &v[7].z, &v[6].x, &v[6].y, &v[6].z);
	break;
      case kTokenSixCeil:
	ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
	  	 &v[0].x, &v[0].y, &v[0].z, &v[1].x, &v[1].y, &v[1].z, 
                 &v[5].x, &v[5].y, &v[5].z, &v[4].x, &v[4].y, &v[4].z);
	break;
      case kTokenSixActivate:
	ScanStr (params, "%s", str);
	{
	  csScript* s = (csScript*)w->scripts.FindByName (str);
	  if (!s)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str);
	    fatal_exit (0, false);
	  }
          csObjectTrigger *objtrig = csObjectTrigger::GetTrigger(*thing);
          if (!objtrig)
          {
            CHK(objtrig = new csObjectTrigger());
            thing->ObjAdd(objtrig);
          }
	  objtrig->NewActivateTrigger (s);
	  objtrig->DoActivateTriggers ();
	}
	break;
      case kTokenSixTrigger:
	ScanStr (params, "%s,%s", str, str2);
	if (!strcmp (str, "activate"))
	{
	  csScript* s = (csScript*)w->scripts.FindByName (str2);
	  if (!s)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str2);
	    fatal_exit (0, false);
	  }
          csObjectTrigger *objtrig = csObjectTrigger::GetTrigger(*thing);
	  if (!objtrig) 
          {
            CHK(objtrig = new csObjectTrigger());
            thing->ObjAdd(objtrig);
          }
          objtrig->NewActivateTrigger (s);
	}
        else
        {
	  CsPrintf (MSG_FATAL_ERROR, "Trigger '%s' not supported or known for object!\n", str2);
	  fatal_exit (0, false);
        }
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a sixface!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  for (i = 0;  i < 8;  i++) thing->AddVertex(v[i]);

  struct Todo
  {
    ObName poly;
    int v1, v2, v3, v4;
    int tv1, tv2;
    csTextureHandle* texture;
  };
  Todo todo[100];
  int done = 0;
  int todo_end = 0;

  strcpy (todo[todo_end].poly, "north");
  todo[todo_end].v1 = 0;
  todo[todo_end].v2 = 1;
  todo[todo_end].v3 = 3;
  todo[todo_end].v4 = 2;
  todo[todo_end].tv1 = 0;
  todo[todo_end].tv2 = 1;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "east");
  todo[todo_end].v1 = 1;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 3;
  todo[todo_end].tv1 = 1;
  todo[todo_end].tv2 = 5;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "south");
  todo[todo_end].v1 = 5;
  todo[todo_end].v2 = 4;
  todo[todo_end].v3 = 6;
  todo[todo_end].v4 = 7;
  todo[todo_end].tv1 = 5;
  todo[todo_end].tv2 = 4;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "west");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 0;
  todo[todo_end].v3 = 2;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 0;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "up");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 1;
  todo[todo_end].v4 = 0;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 5;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "down");
  todo[todo_end].v1 = 2;
  todo[todo_end].v2 = 3;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 2;
  todo[todo_end].tv2 = 3;
  todo[todo_end].texture = texture;
  todo_end++;

  while (done < todo_end)
  {
    csPolygon3D *p = thing->NewPolygon (todo[done].texture);
    csNameObject::AddName(*p,todo[done].poly);
    p->AddVertex (todo[done].v4);
    p->AddVertex (todo[done].v3);
    p->AddVertex (todo[done].v2);
    p->AddVertex (todo[done].v1);
    p->SetTextureSpace (thing->Vobj (todo[done].tv2), 
                          thing->Vobj (todo[done].tv1), tscale);
    done++;
  }

  if (is_convex || thing->GetFog ().enabled) thing->SetConvex (true);
  thing->SetTransform (obj);
  thing->Transform ();

  return thing;
}

enum { kTokenThingMove = kTokenPSetLast, kTokenThingTemplate, kTokenThingMoveable,
       kTokenThingConvex };

csThing* CSLoader::load_thing (char* name, csWorld* w, char* buf, 
                             csTextureList* textures, csSector* sec)
{
  static tokenDesc commands[] = {
  	{kTokenPSetVertex, "VERTEX"},
  	{kTokenPSetCircle, "CIRCLE"},
	{kTokenPSetPolygon, "POLYGON"},
	{kTokenPSetBezier, "BEZIER"},
	{kTokenPSetTexNr, "TEXNR"},
	{kTokenPSetTexlen, "TEXLEN"},
	{kTokenPSetTrigger, "TRIGGER"},
	{kTokenPSetActivate, "ACTIVATE"},
	{kTokenPSetLightX, "LIGHTX"},
	{kTokenPSetBsp, "BSP"},
	{kTokenPSetFog, "FOG"},
	{kTokenThingMoveable, "MOVEABLE"},
	{kTokenThingConvex, "CONVEX"},
	{kTokenThingMove, "MOVE"},
	{kTokenThingTemplate, "TEMPLATE"},
	{0,0}};
  static tokenDesc tok_matvec[] = {{1, "MATRIX"}, {2, "V"}, {0,0}};
  char* xname;

  CHK( csThing* thing = new csThing() );
  csNameObject::AddName(*thing,name); 

  LoadStat::things_loaded++;
  PSLoadInfo info(w, textures);
  thing->SetSector (sec);

  csReversibleTransform obj;
  long cmd;
  char* params;
  char str[255];
  bool is_convex = false;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenThingMoveable:
        thing->SetMoveable (true);
	break;
      case kTokenThingConvex:
        is_convex = true;
	break;
      case kTokenThingMove:
	{
	  char* params2;
	  while ((cmd=csGetObject(&params, tok_matvec, &xname, &params2)) > 0)
	  {
	    switch (cmd)
	    {
	      case 1: 
                obj.SetT2O ( CSLoader::load_matrix(params2) );  break;
	      case 2:
                obj.SetOrigin ( CSLoader::load_vector(params2) );  break;
	    }
	  }
	}
	break;
      case kTokenThingTemplate:
        {
	  ScanStr (params, "%s", str);
	  csThingTemplate* t = w->GetThingTemplate (str, true);
	  if (!t)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Can't find template '%s'!\n", str);
	    fatal_exit (0, false);
	  }
	  thing->MergeTemplate (t, info.default_texture, info.default_texlen,
	  	                 info.default_lightx);
  	  LoadStat::polygons_loaded += t->GetNumPolygon ();
	}
        break;
      default:
	CSLoader::ps_process(*thing, info, cmd, xname, params);
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a thing!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  thing->SetTransform(obj);
  thing->Transform ();
  if (is_convex || thing->GetFog ().enabled) thing->SetConvex (true);
  if (info.use_bsp) thing->UseBSP ();
 
  return thing;
}



//---------------------------------------------------------------------------

enum { kTokenPolTexNr = 1, kTokenPolLighting, kTokenPolMipmap,
       kTokenPolPortal, kTokenPolWarp, kTokenPolLightX,
       kTokenPolTexture, kTokenPolVertices, kTokenPolAlpha,
       kTokenPolFog, kTokenPolUV, kTokenPolUVA, kTokenPolColors,
       kTokenPolFlatCol, kTokenPolCosFact, kTokenPolGouraud };

enum { kTokenPTexOrig = 1, kTokenPTexFirst, kTokenPTexFirstLen,
       kTokenPTexSecond, kTokenPTexSecondLen, kTokenPTexLen,
       kTokenPTexMatrix, kTokenPTexVector, kTokenPTexVectorAfter,
       kTokenPTexPlane, kTokenPTexMirror, kTokenPTexStatic,
       kTokenPTexUVShift };

csPolygon3D* CSLoader::load_poly3d (char* polyname, csWorld* w, char* buf, 
	csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
	CLights* default_lightx, csSector* sec, csPolygonSet* parent)
{
  static tokenDesc commands[] = {
  	{kTokenPolTexNr, "TEXNR"},
	{kTokenPolLighting, "LIGHTING"},
	{kTokenPolMipmap, "MIPMAP"},
	{kTokenPolPortal, "PORTAL"},
	{kTokenPolWarp, "WARP"},
	{kTokenPolLightX, "LIGHTX"},
	{kTokenPolTexture, "TEXTURE"},
	{kTokenPolVertices, "VERTICES"},
	{kTokenPolUVA, "UVA"},
	{kTokenPolUV, "UV"},
	{kTokenPolColors, "COLORS"},
	{kTokenPolFlatCol, "FLATCOL"},
	{kTokenPolAlpha, "ALPHA"},
	{kTokenPolFog, "FOG"},
	{kTokenPolCosFact, "COSFACT"},
	{kTokenPolGouraud, "GOURAUD"},
	{0,0}};
  static tokenDesc tCommands[] = {
	{kTokenPTexOrig, "ORIG"},
	{kTokenPTexFirstLen, "FIRST_LEN"},
	{kTokenPTexFirst, "FIRST"},
	{kTokenPTexSecondLen, "SECOND_LEN"},
	{kTokenPTexSecond, "SECOND"},
	{kTokenPTexLen, "LEN"},
	{kTokenPTexMatrix, "MATRIX"},
	{kTokenPTexPlane, "PLANE"},
	{kTokenPTexVector, "V"},
	{kTokenPTexUVShift, "UV_SHIFT"},
	{0,0}};
  static tokenDesc pCommands[] = {
	{kTokenPTexMatrix, "MATRIX"},
	{kTokenPTexVector, "V"},
	{kTokenPTexVectorAfter, "W"},
	{kTokenPTexMirror, "MIRROR"},
	{kTokenPTexStatic, "STATIC"},
	{0,0}};
  char* name;
  int i;
  long cmd;
  char* params, * params2;

  CHK(csPolygon3D *poly3d = new csPolygon3D(default_texture) );
  csNameObject::AddName(*poly3d,polyname);

  csTextureHandle* tex = NULL;
  poly3d->SetSector (sec);
  poly3d->SetParent (parent);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig, tx1, tx2;
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;
  char plane_name[30];
  plane_name[0] = 0;
  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  bool do_mirror = false;
  poly3d->theDynLight = default_lightx;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenPolTexNr:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
        poly3d->SetTexture (tex);
	break;
      case kTokenPolLighting:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          poly3d->SetLighting (do_lighting);
        }
	break;
      case kTokenPolMipmap:
        {
          int do_mipmap;
          ScanStr (params, "%b", &do_mipmap);
          poly3d->SetMipmapping (do_mipmap);
        }
	break;
      case kTokenPolCosFact:
        {
          float cosfact;
          ScanStr (params, "%f", &cosfact);
	  poly3d->SetCosinusFactor (cosfact);
	}
	break;
      case kTokenPolAlpha:
        {
          int alpha;
          ScanStr (params, "%d", &alpha);
	  poly3d->SetAlpha (alpha);
	}
	break;
      case kTokenPolFog:
        {
	  //@@@ OBSOLETE
	}
	break;
      case kTokenPolPortal:
        {
          ScanStr (params, "%s", str);
          CHK(csSector *s = new csSector());
          csNameObject::AddName(*s,str);
          poly3d->SetCSPortal (s);
	  LoadStat::portals_loaded++;
        }
	break;
      case kTokenPolWarp:
        if (poly3d->GetPortal ())
	{
	  csMatrix3 m_w; m_w.Identity ();
	  csVector3 v_w_before;
	  csVector3 v_w_after;
	  bool do_static = false;
          while ((cmd = csGetObject (&params, pCommands, &name, &params2)) > 0)
          {
            switch (cmd)
	    {
	      case kTokenPTexMatrix:
	        m_w = CSLoader::load_matrix (params2);
		do_mirror = false;
	        break;
	      case kTokenPTexVector:
	        v_w_before = CSLoader::load_vector (params2);
		v_w_after = v_w_before;
		do_mirror = false;
	        break;
	      case kTokenPTexVectorAfter:
	        v_w_after = CSLoader::load_vector (params2);
		do_mirror = false;
	        break;
	      case kTokenPTexMirror:
	        do_mirror = true;
	        break;
	      case kTokenPTexStatic:
	        do_static = true;
	        break;
	    }
	  }
	  if (!do_mirror)
	    poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);
	  poly3d->GetPortal ()->SetStaticDest (do_static);
	}
	break;
      case kTokenPolLightX:
        ScanStr (params, "%s", str);
        poly3d->theDynLight = CLights::FindByName (str);
	break;
      case kTokenPolTexture:
        while ((cmd = csGetObject (&params, tCommands, &name, &params2)) > 0)
        {
          switch (cmd)
	  {
	    case kTokenPTexOrig:
	      tx1_given = true;
	      tx1_orig = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTexFirst:
	      tx1_given = true;
	      tx1 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTexFirstLen:
	      ScanStr (params2, "%f", &tx1_len);
	      tx1_given = true;
	      break;
	    case kTokenPTexSecond:
	      tx2_given = true;
	      tx2 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTexSecondLen:
	      ScanStr (params2, "%f", &tx2_len);
	      tx2_given = true;
	      break;
	    case kTokenPTexLen:
	      ScanStr (params2, "%f", &tx_len);
	      break;
	    case kTokenPTexMatrix:
	      tx_matrix = CSLoader::load_matrix (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTexVector:
	      tx_vector = CSLoader::load_vector (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTexPlane:
	      ScanStr (params2, "%s", str);
	      strcpy (plane_name, str);
	      tx_len = 0;
	      break;
	    case kTokenPTexUVShift:
	      uv_shift_given = true;
	      ScanStr (params2, "%f,%f", &u_shift, &v_shift);
	      break;
	  }
	}
	break;
      case kTokenPolVertices:
        {
	  int list[100], num;
          ScanStr (params, "%D", list, &num);
	  for (i = 0 ; i < num ; i++) poly3d->AddVertex (list[i]);
	}
	break;
      case kTokenPolFlatCol:
        {
	  float r, g, b;
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
	  poly3d->SetFlatColor (r, g, b);
	}
	break;
      case kTokenPolGouraud:
	poly3d->SetColor (0, 0, 0, 0);
	break;
      case kTokenPolUV:
        {
	  float list[6];
	  int num;
          ScanStr (params, "%F", list, &num);
	  poly3d->SetUV (0, list[0], list[1]);
	  poly3d->SetUV (1, list[2], list[3]);
	  poly3d->SetUV (2, list[4], list[5]);
	}
	break;
      case kTokenPolColors:
        {
	  float list[9];
	  int num;
          ScanStr (params, "%F", list, &num);
	  poly3d->SetColor (0, list[0], list[1], list[2]);
	  poly3d->SetColor (1, list[3], list[4], list[5]);
	  poly3d->SetColor (2, list[6], list[7], list[8]);
	}
	break;
      case kTokenPolUVA:
        {
	  float list[9];
	  int num;
          ScanStr (params, "%F", list, &num);
	  float angle;
	  angle = list[0]*2*M_PI/360.;
	  poly3d->SetUV (0, cos (angle)*list[1]+list[2], sin (angle)*list[1]+list[2]);
	  angle = list[3]*2*M_PI/360.;
	  poly3d->SetUV (1, cos (angle)*list[4]+list[5], sin (angle)*list[4]+list[5]);
	  angle = list[6]*2*M_PI/360.;
	  poly3d->SetUV (2, cos (angle)*list[7]+list[8], sin (angle)*list[7]+list[8]);
	}
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a polygon!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (tx1_given)
    if (tx2_given) 
         poly3d->SetTextureSpace (tx1_orig.x, tx1_orig.y, tx1_orig.z,
				      tx1.x, tx1.y, tx1.z, tx1_len,
				      tx2.x, tx2.y, tx2.z, tx2_len);
    else poly3d->SetTextureSpace (tx1_orig.x, tx1_orig.y, tx1_orig.z,
			    tx1.x, tx1.y, tx1.z, tx1_len);
  else if (plane_name[0]) 
           poly3d->SetTextureSpace ((csPolyPlane*)w->planes.FindByName (plane_name));
  else if (tx_len)
  {
    // If a length is given (with 'LEN') we will take the first two vertices
    // and calculate the texture orientation from them (with the given
    // length).
    poly3d->SetTextureSpace (poly3d->Vobj(0), poly3d->Vobj(1), tx_len);
  }
  else poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (uv_shift_given)
  {
    poly3d->GetPlane ()->GetTextureSpace (tx_matrix, tx_vector);
    // T = Mot * (O - Vot)
    // T = Mot * (O - Vot) + Vuv		; Add shift Vuv to final texture map
    // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
    // T = Mot * (O - Vot + Mot-1 * Vuv)
    csVector3 shift; shift.x = u_shift; shift.y = v_shift; shift.z = 0;
    tx_vector -= tx_matrix.GetInverse () * shift;
    poly3d->SetTextureSpace (tx_matrix, tx_vector);
  }

  if (do_mirror)
    poly3d->GetPortal ()->SetWarp (csTransform::GetReflect ( *(poly3d->GetPolyPlane ()) ));

  return poly3d;
}


csCurve* CSLoader::load_bezier (char* polyname, csWorld* w, char* buf, 
	csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
	CLights* default_lightx, csSector* sec, csPolygonSet* parent)
{
  static tokenDesc commands[] = {
  	{kTokenPolTexNr, "TEXNR"},
	{kTokenPolTexture, "TEXTURE"},
	{kTokenPolVertices, "VERTICES"},
	{0,0}};

  static tokenDesc tCommands[] = {
	{kTokenPTexOrig, "ORIG"},
	{kTokenPTexFirstLen, "FIRST_LEN"},
	{kTokenPTexFirst, "FIRST"},
	{kTokenPTexSecondLen, "SECOND_LEN"},
	{kTokenPTexSecond, "SECOND"},
	{kTokenPTexLen, "LEN"},
	{kTokenPTexMatrix, "MATRIX"},
	{kTokenPTexPlane, "PLANE"},
	{kTokenPTexVector, "V"},
	{kTokenPTexUVShift, "UV_SHIFT"},
	{0,0}};
  char* name;
  long cmd;
  char* params, * params2;

  (void)w; (void)default_lightx; (void)sec; (void)parent;

  CHK(csBezier *poly3d = new csBezier() );
  csNameObject::AddName(*poly3d,polyname);
  poly3d->SetTextureHandle (default_texture);
  csTextureHandle* tex = NULL;
//TODO??  poly3d->SetSector(sec);
//TODO??  poly3d->SetParent (parent);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig, tx1, tx2;
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;
  char plane_name[30];
  plane_name[0] = 0;
  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenPolTexNr:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
        poly3d->SetTextureHandle (tex);
	break;
      case kTokenPolTexture:
        while ((cmd = csGetObject (&params, tCommands, &name, &params2)) > 0)
        {
          switch (cmd)
	  {
	    case kTokenPTexOrig:
	      tx1_given = true;
	      tx1_orig = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTexFirst:
	      tx1_given = true;
	      tx1 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTexFirstLen:
	      ScanStr (params2, "%f", &tx1_len);
	      tx1_given = true;
	      break;
	    case kTokenPTexSecond:
	      tx2_given = true;
	      tx2 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTexSecondLen:
	      ScanStr (params2, "%f", &tx2_len);
	      tx2_given = true;
	      break;
	    case kTokenPTexLen:
	      ScanStr (params2, "%f", &tx_len);
	      break;
	    case kTokenPTexMatrix:
	      tx_matrix = CSLoader::load_matrix (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTexVector:
	      tx_vector = CSLoader::load_vector (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTexPlane:
	      ScanStr (params2, "%s", str);
	      strcpy (plane_name, str);
	      tx_len = 0;
	      break;
	    case kTokenPTexUVShift:
	      uv_shift_given = true;
	      ScanStr (params2, "%f,%f", &u_shift, &v_shift);
	      break;
	  }
	}
	break;
      case kTokenPolVertices:
        {
	  int list[100], num;
          ScanStr (params, "%D", list, &num);

	  if (num != 9)
	    {
	      CsPrintf (MSG_FATAL_ERROR, "Wrong number of vertices to bezier!\n");
	      fatal_exit (0, false);
	    }

	  //TODO	  for (i = 0 ; i < num ; i++) poly3d->set_vertex (i, list[i]);
	}
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a bezier!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return poly3d;
}



//---------------------------------------------------------------------------

ImageFile* CSLoader::load_image(char* name, csWorld* w, Archive* ar)
{
  Archive* world_file = NULL;
  if (w) world_file = w->GetWorldFile ();
  ImageFile* ifile = NULL;

  size_t size;
  if (ar && ar->file_exists (name, &size))
    world_file = ar;

  if (world_file && world_file->file_exists (name, &size))
  {
    char* buf;
    buf = world_file->read (name);
    if (!buf)
    {
      CsPrintf (MSG_FATAL_ERROR,
        "Can't load file '%s' from archive although it should be there!\n"
        "Maybe the archive is corrupt?\n", name);
      return NULL;
    }
    ifile = ImageLoader::load ((UByte*)buf, size);
    if (ifile)
    {
     if (ifile->get_status() & IFE_Corrupt)
     {
       CsPrintf(MSG_FATAL_ERROR, "'%s': %s!\n",name,ifile->get_status_mesg());
       CHK (delete ifile);  ifile = NULL;
     }
    } else CsPrintf (MSG_INTERNAL_ERROR, 
               "ENGINE FAILURE! Couldn't open '%s' from archive!\n", name);
    CHK (delete [] buf); 
  }
  else
  {
    FILE* fp;
    w->isys->FOpen (name, "rb", &fp);
    if (!fp)
    {
      CsPrintf (MSG_FATAL_ERROR, 
        "Can't find file '%s' in data directory or in archive!\n", name);
      return NULL;
    }
    ifile = ImageLoader::load (fp);
    if (ifile)
    {
     if (ifile->get_status() & IFE_Corrupt)
     {
       CsPrintf(MSG_FATAL_ERROR, "'%s': %s!\n",name,ifile->get_status_mesg());
       CHK (delete ifile);  ifile = NULL;
     }
    } else CsPrintf (MSG_INTERNAL_ERROR, 
               "ENGINE FAILURE! Couldn't open '%s' from disk!\n", name);
    w->isys->FClose (fp);
  }
  return ifile;
}

enum { kTokenTexTrans = 1, kTokenTexFilter };

void CSLoader::txt_process (csTextureHandle* txt_handle, char* buf)
{
  static tokenDesc commands[] = {
        {kTokenTexTrans, "TRANSPARENT"},
        {kTokenTexFilter, "FILTER"},
        {0,0}};
  long cmd;
  char* params;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenTexTrans:
        {
          float r, g, b;
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
          txt_handle->SetTransparent ((int)(r*255.), (int)(g*255.), (int)(b*255.));
        }
        break;
      case kTokenTexFilter:
        CsPrintf (MSG_WARNING, "Warning! TEXTURE/FILTER statement is obsolete"
                               " and does not do anything!\n");
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a texture specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
}

//---------------------------------------------------------------------------

enum { kTokenPolTTexNr = 1, kTokenPolTLighting, kTokenPolTMipmap,
       kTokenPolTTexture, kTokenPolTVertices, kTokenPolTFlatCol, kTokenPolTGouraud };

enum { kTokenPTTexOrig = 1, kTokenPTTexFirst, kTokenPTTexFirstLen,
       kTokenPTTexSecond, kTokenPTTexSecondLen, kTokenPTTexLen,
       kTokenPTTexMatrix, kTokenPTTexVector, kTokenPTTexUVShift };

csPolygonTemplate* CSLoader::load_ptemplate (char* ptname, char* buf, 
	csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
        csThingTemplate* parent)
{
  static tokenDesc commands[] = {
  	{kTokenPolTTexNr, "TEXNR"},
	{kTokenPolTLighting, "LIGHTING"},
	{kTokenPolTMipmap, "MIPMAP"},
	{kTokenPolTTexture, "TEXTURE"},
	{kTokenPolTVertices, "VERTICES"},
	{kTokenPolTFlatCol, "FLATCOL"},
	{kTokenPolTGouraud, "GOURAUD"},
	{0,0}};
  static tokenDesc tCommands[] = {
	{kTokenPTTexOrig, "ORIG"},
	{kTokenPTTexFirstLen, "FIRST_LEN"},
	{kTokenPTTexFirst, "FIRST"},
	{kTokenPTTexSecondLen, "SECOND_LEN"},
	{kTokenPTTexSecond, "SECOND"},
	{kTokenPTTexLen, "LEN"},
	{kTokenPTTexMatrix, "MATRIX"},
	{kTokenPTTexVector, "V"},
	{kTokenPTTexUVShift, "UV_SHIFT"},
	{0,0}};
  char* name;
  int i;
  long cmd;
  char* params, * params2;

  CHK(csPolygonTemplate *ptemplate = 
              new csPolygonTemplate(parent, ptname, default_texture));
  csTextureHandle* tex;
  if (default_texture == NULL) tex = NULL;
  else ptemplate->SetTexture (default_texture);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig, tx1, tx2;
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenPolTTexNr:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, 
                    "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
        ptemplate->SetTexture (tex);
	break;
      case kTokenPolTGouraud:
        ptemplate->SetGouraud ();
	break;
      case kTokenPolTFlatCol:
        {
	  float r, g, b;
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
	  ptemplate->SetFlatColor (r, g, b);
	}
	break;
      case kTokenPolTLighting:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting); 
          ptemplate->SetLighting (do_lighting);
        }
	break;
      case kTokenPolTMipmap:
        {
          int do_mipmap;
          ScanStr (params, "%b", &do_mipmap); 
          ptemplate->SetMipmapping (do_mipmap);
        }
	break;
      case kTokenPolTTexture:
        while ((cmd = csGetObject (&params, tCommands, &name, &params2)) > 0)
        {
          switch (cmd)
	  {
	    case kTokenPTTexOrig:
	      tx1_given = true;
	      tx1_orig = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTTexFirst:
	      tx1_given = true;
	      tx1 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTTexFirstLen:
	      ScanStr (params2, "%f", &tx1_len);
	      tx1_given = true;
	      break;
	    case kTokenPTTexSecond:
	      tx2_given = true;
	      tx2 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTTexSecondLen:
	      ScanStr (params2, "%f", &tx2_len);
	      tx2_given = true;
	      break;
	    case kTokenPTTexLen:
	      ScanStr (params2, "%f", &tx_len);
	      break;
	    case kTokenPTTexMatrix:
	      tx_matrix = CSLoader::load_matrix (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTTexVector:
	      tx_vector = CSLoader::load_vector (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTTexUVShift:
	      uv_shift_given = true;
	      ScanStr (params2, "%f,%f", &u_shift, &v_shift);
	      break;
	  }
	}
	break;
      case kTokenPolTVertices:
        {
	  int list[100], num;
          ScanStr (params, "%D", list, &num);
	  for (i = 0 ; i < num ; i++) ptemplate->AddVertex (list[i]);
	}
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a polygon template!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (tx1_given)
    if (tx2_given)
      TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	tx1_orig.x, tx1_orig.y, tx1_orig.z,
	tx1.x, tx1.y, tx1.z, tx1_len,
	tx2.x, tx2.y, tx2.z, tx2_len);
    else
    {
      float A, B, C;
      ptemplate->PlaneNormal (&A, &B, &C);
      TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	tx1_orig.x, tx1_orig.y, tx1_orig.z,
	tx1.x, tx1.y, tx1.z, tx1_len,
	A, B, C);
    }
  else if (tx_len)
  {
    // If a length is given (with 'LEN') we will take the first two vertices
    // and calculate the texture orientation from them (with the given
    // length).
    float A, B, C;
    ptemplate->PlaneNormal (&A, &B, &C);
    TextureTrans::compute_texture_space (tx_matrix, tx_vector,
    	parent->Vtex (ptemplate->GetVerticesIdx ()[0]),
    	parent->Vtex (ptemplate->GetVerticesIdx ()[1]), tx_len,
	A, B, C);
  }
  if (uv_shift_given)
  {
    // T = Mot * (O - Vot)
    // T = Mot * (O - Vot) + Vuv		; Add shift Vuv to final texture map
    // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
    // T = Mot * (O - Vot + Mot-1 * Vuv)
    csVector3 shift; shift.x = u_shift; shift.y = v_shift; shift.z = 0;
    tx_vector -= tx_matrix.GetInverse () * shift;
  }
  ptemplate->SetTextureSpace (tx_matrix, tx_vector);

  return ptemplate;
}

csCurveTemplate* CSLoader::load_beziertemplate (char* ptname, char* buf, 
	csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
        csThingTemplate* parent)
{
  static tokenDesc commands[] = {
  	{kTokenPolTTexNr, "TEXNR"},
	{kTokenPolTTexture, "TEXTURE"},
	{kTokenPolTVertices, "VERTICES"},
	{0,0}};
  static tokenDesc tCommands[] = {
	{kTokenPTTexOrig, "ORIG"},
	{kTokenPTTexFirstLen, "FIRST_LEN"},
	{kTokenPTTexFirst, "FIRST"},
	{kTokenPTTexSecondLen, "SECOND_LEN"},
	{kTokenPTTexSecond, "SECOND"},
	{kTokenPTTexLen, "LEN"},
	{kTokenPTTexMatrix, "MATRIX"},
	{kTokenPTTexVector, "V"},
	{kTokenPTTexUVShift, "UV_SHIFT"},
	{0,0}};
  char* name;
  long cmd;
  int i;
  char* params, * params2;

  CHK(csBezierTemplate *ptemplate = 
              new csBezierTemplate());
  csNameObject::AddName(*ptemplate,ptname);
  
  ptemplate->SetParent  (parent);

  csTextureHandle* tex;
  if (default_texture == NULL) tex = NULL;
  else ptemplate->SetTextureHandle (default_texture);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig, tx1, tx2;
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenPolTTexNr:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, 
                    "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
        ptemplate->SetTextureHandle (tex);
	break;
      case kTokenPolTTexture:
        while ((cmd = csGetObject (&params, tCommands, &name, &params2)) > 0)
        {
          switch (cmd)
	  {
	    case kTokenPTTexOrig:
	      tx1_given = true;
	      tx1_orig = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTTexFirst:
	      tx1_given = true;
	      tx1 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTTexFirstLen:
	      ScanStr (params2, "%f", &tx1_len);
	      tx1_given = true;
	      break;
	    case kTokenPTTexSecond:
	      tx2_given = true;
	      tx2 = CSLoader::load_vector (params2);
	      break;
	    case kTokenPTTexSecondLen:
	      ScanStr (params2, "%f", &tx2_len);
	      tx2_given = true;
	      break;
	    case kTokenPTTexLen:
	      ScanStr (params2, "%f", &tx_len);
	      break;
	    case kTokenPTTexMatrix:
	      tx_matrix = CSLoader::load_matrix (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTTexVector:
	      tx_vector = CSLoader::load_vector (params2);
	      tx_len = 0;
	      break;
	    case kTokenPTTexUVShift:
	      uv_shift_given = true;
	      ScanStr (params2, "%f,%f", &u_shift, &v_shift);
	      break;
	  }
	}
	break;
      case kTokenPolTVertices:
        {
	  int list[100], num;
          ScanStr (params, "%D", list, &num);
	  if (num != 9)
	    {
	      CsPrintf (MSG_FATAL_ERROR, "Wrong number of vertices to bezier!\n");
	      fatal_exit (0, false);
	    }
	  for (i = 0 ; i < num ; i++) ptemplate->SetVertex (i,list[i]);
	}
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a bezier template!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
  return ptemplate;
}



//---------------------------------------------------------------------------

enum { 
  kTokenThingTMove = kTokenPSetLast,
  kTokenThingCurveCenter, 
  kTokenThingCurveScale,
  kTokenTSetCurveVertex
 };

csThingTemplate* CSLoader::load_thingtpl (char* tname, char* buf, 
                                        csTextureList* textures)
{
  static tokenDesc commands[] = {
  	{kTokenPSetVertex, "VERTEX"},
  	{kTokenPSetCircle, "CIRCLE"},
	{kTokenPSetPolygon, "POLYGON"},
	{kTokenPSetBezier, "BEZIER"},
	{kTokenPSetTexNr, "TEXNR"},
	{kTokenPSetTexlen, "TEXLEN"},
	{kTokenPSetFog, "FOG"},
	{kTokenThingTMove, "MOVE"},

	{kTokenThingCurveCenter, "CURVECENTER"},
	{kTokenThingCurveScale, "CURVESCALE"},
	{kTokenTSetCurveVertex, "CURVECONTROL"},

	{0,0}};
  static tokenDesc tok_matrix[] = {{1, "MATRIX"}, {0,0}};
  static tokenDesc tok_vector[] = {{1, "V"}, {0,0}};
  char* name;
  char str[255];
  int i;

  CHK( csThingTemplate *tmpl = new csThingTemplate() );
  csNameObject::AddName(*tmpl,tname);
  long cmd;
  char* params;
  csTextureHandle* default_texture = NULL;
  float default_texlen = 1.;

  csMatrix3 m_move;
  csVector3 v_move;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenPSetVertex:
        {
	  float x, y, z;
	  ScanStr (params, "%f,%f,%f", &x, &y, &z);
          tmpl->AddVertex (x, y, z);
	}
        break;
      case kTokenPSetCircle:
        {
          float x, y, z, rx, ry, rz;
	  int num, dir;
          ScanStr (params, "%f,%f,%f:%f,%f,%f,%d", &x, &y, &z, &rx, &ry, &rz, &num);
	  if (num < 0) { num = -num; dir = -1; }
	  else dir = 1;
	  for (i = 0 ; i < num ; i++)
	  {
	    float rad;
	    if (dir == 1) rad = 2.*M_PI*(num-i-1)/(float)num;
	    else rad = 2.*M_PI*i/(float)num;

	    float cx = 0, cy = 0, cz = 0;
	    float cc = cos (rad);
	    float ss = sin (rad);
	    if      (ABS (rx) < SMALL_EPSILON) { cx = x; cy = y+cc*ry; cz = z+ss*rz; }
	    else if (ABS (ry) < SMALL_EPSILON) { cy = y; cx = x+cc*rx; cz = z+ss*rz; }
	    else if (ABS (rz) < SMALL_EPSILON) { cz = z; cx = x+cc*rx; cy = y+ss*ry; }
	    tmpl->AddVertex (cx, cy, cz);
	  }
        }
        break;
      case kTokenPSetFog:
        {
	  csFog& f = tmpl->GetFog ();
	  f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
	}
        break;
      case kTokenPSetPolygon:
	tmpl->AddPolygon ( CSLoader::load_ptemplate(name, params, textures,
                            default_texture, default_texlen, tmpl) );
        break;
      case kTokenPSetBezier:
        //CsPrintf(MSG_WARNING,"Encountered template curve!\n");
        tmpl->AddCurve ( 
	      CSLoader::load_beziertemplate(name, params, textures,
                        default_texture, default_texlen, tmpl)  );
        break;

    case kTokenThingCurveCenter:
      {
	csVector3 c;
	ScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
	tmpl->curves_center = c;
      }
      break;
    case kTokenThingCurveScale:
      ScanStr (params, "%f", &tmpl->curves_scale);
      break;

    case kTokenTSetCurveVertex:
      {
	csVector3 v;
	csVector2 t;
	ScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
	tmpl->AddCurveVertex (v,t);
      }
      break;


      case kTokenPSetTexNr:
        ScanStr (params, "%s", str);
        default_texture = textures->GetTextureMM (str);
        if (default_texture == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, 
                    "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
        break;
      case kTokenPSetTexlen:
        ScanStr (params, "%f", &default_texlen);
        break;
      case kTokenThingTMove:
	{
	  char* params2;
	  csGetObject (&params, tok_matrix, &name, &params2);
          m_move = CSLoader::load_matrix(params2);
	  csGetObject (&params, tok_vector, &name, &params2);
          v_move = CSLoader::load_vector(params2);
	}
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a thing template!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  for (i = 0 ; i < tmpl->GetNumVertices () ; i++)
    tmpl->Vtex(i) = v_move + m_move * tmpl->Vtex(i);
  return tmpl;
}

//---------------------------------------------------------------------------

enum { kTokenTSixMove = 1, kTokenTSixTexture, kTokenTSixTexCeil,
       kTokenTSixTexFloor, kTokenTSixTexScale, kTokenTSixDim,
       kTokenTSixFlHeight, kTokenTSixHeight, kTokenTSixFloor,
       kTokenTSixFloorCeil, kTokenTSixCeil, kTokenTSixTrigger,
       kTokenTSixFog };

csThingTemplate* CSLoader::load_sixtpl(char* tname,char* buf,csTextureList* textures)
{
  static tokenDesc commands[] = {
  	{kTokenTSixMove, "MOVE"},
	{kTokenTSixTexScale, "TEXTURE_SCALE"}, 
	{kTokenTSixTexture, "TEXTURE"}, 
	{kTokenTSixTexCeil, "CEIL_TEXTURE"}, 
	{kTokenTSixDim, "DIM"}, 
	{kTokenTSixHeight, "HEIGHT"}, 
	{kTokenTSixFlHeight, "FLOOR_HEIGHT"}, 
	{kTokenTSixFloorCeil, "FLOOR_CEIL"}, 
	{kTokenTSixTexFloor, "FLOOR_TEXTURE"}, 
	{kTokenTSixFloor, "FLOOR"}, 
	{kTokenTSixCeil, "CEILING"}, 
	{kTokenTSixFog, "FOG"}, 
	{0,0}};
  static tokenDesc tok_matrix[] = {{1, "MATRIX"}, {0,0}};
  static tokenDesc tok_vector[] = {{1, "V"}, {0,0}};
  char* name;
  int i;

  CHK( csThingTemplate* tmpl = new csThingTemplate() );
  csNameObject::AddName(*tmpl,tname);

  csTextureHandle* texture = NULL;
  float tscale = 1;

  csVector3 v0, v1, v2, v3, v4, v5, v6, v7;
  v0.x = -1; v0.y =  1; v0.z =  1;
  v1.x =  1; v1.y =  1; v1.z =  1;
  v2.x = -1; v2.y = -1; v2.z =  1;
  v3.x =  1; v3.y = -1; v3.z =  1;
  v4.x = -1; v4.y =  1; v4.z = -1;
  v5.x =  1; v5.y =  1; v5.z = -1;
  v6.x = -1; v6.y = -1; v6.z = -1;
  v7.x =  1; v7.y = -1; v7.z = -1;
  float r;

  char str[255];
  long cmd;
  char* params;

  csMatrix3 m_move;
  csVector3 v_move;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenTSixFog:
        {
	  csFog& f = tmpl->GetFog ();
	  f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
	break;
      case kTokenTSixMove:
        {
	  char* params2;
	  csGetObject (&params, tok_matrix, &name, &params2);
	  m_move = CSLoader::load_matrix (params2);
	  csGetObject (&params, tok_vector, &name, &params2);
	  v_move = CSLoader::load_vector (params2);
	}
	break;
      case kTokenTSixTexture:
	ScanStr (params, "%s", str);
        texture = textures->GetTextureMM (str);
        if (texture == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, 
                    "Couldn't find texture named '%s'!\n", str);
	  fatal_exit (0, false);
        }
	break;
      case kTokenTSixTexScale:
	ScanStr (params, "%f", &tscale);
	break;
      case kTokenTSixDim:
	{
	  float rx, ry, rz;
	  ScanStr (params, "%f,%f,%f", &rx, &ry, &rz);
	  rx /= 2; ry /= 2; rz /= 2;
          v0.x = -rx; v0.y =  ry; v0.z =  rz;
          v1.x =  rx; v1.y =  ry; v1.z =  rz;
          v2.x = -rx; v2.y = -ry; v2.z =  rz;
          v3.x =  rx; v3.y = -ry; v3.z =  rz;
          v4.x = -rx; v4.y =  ry; v4.z = -rz;
          v5.x =  rx; v5.y =  ry; v5.z = -rz;
          v6.x = -rx; v6.y = -ry; v6.z = -rz;
          v7.x =  rx; v7.y = -ry; v7.z = -rz;
	}
	break;
      case kTokenTSixFlHeight:
	ScanStr (params, "%f", &r);
        v0.y = r+v0.y-v2.y;
        v1.y = r+v1.y-v3.y;
        v4.y = r+v4.y-v6.y;
        v5.y = r+v5.y-v7.y;
        v2.y = r;
        v3.y = r;
        v6.y = r;
        v7.y = r;
	break;
      case kTokenTSixHeight:
	ScanStr (params, "%f", &r);
        v0.y = r+v2.y;
        v1.y = r+v3.y;
        v4.y = r+v6.y;
        v5.y = r+v7.y;
	break;
      case kTokenTSixFloorCeil:
	ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
	  	&v2.x, &v2.z, &v3.x, &v3.z, &v7.x, &v7.z, &v6.x, &v6.z);
        v0 = v2;
	v1 = v3;
	v5 = v7;
	v4 = v6;
	break;
      case kTokenTSixFloor:
	ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
	  	 &v2.x, &v2.y, &v2.z, &v3.x, &v3.y, &v3.z, 
                 &v7.x, &v7.y, &v7.z, &v6.x, &v6.y, &v6.z);
	break;
      case kTokenTSixCeil:
	ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
	  	 &v0.x, &v0.y, &v0.z, &v1.x, &v1.y, &v1.z, 
                 &v5.x, &v5.y, &v5.z, &v4.x, &v4.y, &v4.z);
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a sixface template!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  tmpl->AddVertex (v0);
  tmpl->AddVertex (v1);
  tmpl->AddVertex (v2);
  tmpl->AddVertex (v3);
  tmpl->AddVertex (v4);
  tmpl->AddVertex (v5);
  tmpl->AddVertex (v6);
  tmpl->AddVertex (v7);

  csPolygonTemplate* p;

  struct Todo
  {
    ObName poly;
    int v1, v2, v3, v4;
    int tv1, tv2;
    csTextureHandle* texture;
  };
  Todo todo[100];
  int done = 0;
  int todo_end = 0;

  strcpy (todo[todo_end].poly, "north");
  todo[todo_end].v1 = 0;
  todo[todo_end].v2 = 1;
  todo[todo_end].v3 = 3;
  todo[todo_end].v4 = 2;
  todo[todo_end].tv1 = 0;
  todo[todo_end].tv2 = 1;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "east");
  todo[todo_end].v1 = 1;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 3;
  todo[todo_end].tv1 = 1;
  todo[todo_end].tv2 = 5;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "south");
  todo[todo_end].v1 = 5;
  todo[todo_end].v2 = 4;
  todo[todo_end].v3 = 6;
  todo[todo_end].v4 = 7;
  todo[todo_end].tv1 = 5;
  todo[todo_end].tv2 = 4;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "west");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 0;
  todo[todo_end].v3 = 2;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 0;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "up");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 1;
  todo[todo_end].v4 = 0;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 5;
  todo[todo_end].texture = texture;
  todo_end++;

  strcpy (todo[todo_end].poly, "down");
  todo[todo_end].v1 = 2;
  todo[todo_end].v2 = 3;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 2;
  todo[todo_end].tv2 = 3;
  todo[todo_end].texture = texture;
  todo_end++;

  while (done < todo_end)
  {
    CHK (p = new csPolygonTemplate (tmpl, todo[done].poly, todo[done].texture));
    tmpl->AddPolygon (p);
    p->AddVertex (todo[done].v4);
    p->AddVertex (todo[done].v3);
    p->AddVertex (todo[done].v2);
    p->AddVertex (todo[done].v1);
    csMatrix3 m_tx;
    csVector3 v_tx;
    float A, B, C;
    p->PlaneNormal (&A, &B, &C);
    TextureTrans::compute_texture_space (m_tx, v_tx,
    	tmpl->Vtex(todo[done].tv2), tmpl->Vtex(todo[done].tv1),
	tscale, A, B, C);
    p->SetTextureSpace (m_tx, v_tx);
    done++;
  }

  for (i = 0 ; i < tmpl->GetNumVertices () ; i++)
    tmpl->Vtex(i) = v_move + m_move * tmpl->Vtex(i);

  for (i = 0 ; i < tmpl->GetNumPolygon () ; i++)
    tmpl->GetPolygon (i)->Transform (m_move, v_move);

  return tmpl;
}

//---------------------------------------------------------------------------


#define MAX_ROOM_PORTALS 30
#define MAX_ROOM_SPLIT 60
#define MAX_ROOM_COLORS 100
#define MAX_ROOM_LIGHT 50

struct RPortal
{
  ObName poly;
  ObName sector;
  bool is_warp;
  bool do_mirror;
  bool do_static;
  csMatrix3 m_warp;
  csVector3 v_warp_before;
  csVector3 v_warp_after;
  int alpha;
};

struct Split
{
  ObName poly;
  float widA[20];
  int dir;
  int cnt;
};

struct Color
{
  Color () { len = 0; }
  ObName poly;
  ObName plane;
  csTextureHandle* texture;
  float len;
};

struct DLight
{
  ObName poly;
  ObName light;
};

struct Todo
{
  ObName poly;
  int v1, v2, v3, v4;
  int tv1, tv2;
  csTextureHandle* texture;
  int col_idx;		// Idx in colors table if there was an override.
  CLights* light;	// A dynamic light for this polygon
};

void add_to_todo (Todo* todo, int& todo_end, char* poly,
	int v1, int v2, int v3, int v4, int tv1, int tv2,
	csTextureHandle* texture, int col_idx,
	CLights* light,
	Color* colors, int num_colors,
	DLight* dlights, int num_light)
{
  int i;
  strcpy (todo[todo_end].poly, poly);
  todo[todo_end].v1 = v1;
  todo[todo_end].v2 = v2;
  todo[todo_end].v3 = v3;
  todo[todo_end].v4 = v4;
  todo[todo_end].tv1 = tv1;
  todo[todo_end].tv2 = tv2;
  todo[todo_end].texture = texture;
  todo[todo_end].col_idx = col_idx;
  todo[todo_end].light = light;
  for (i = 0 ; i < num_colors ; i++)
    if (!strcmp (poly, colors[i].poly))
    {
      todo[todo_end].col_idx = i;
      break;
    }
  for (i = 0 ; i < num_light ; i++)
    if (!strcmp (poly, dlights[i].poly))
    {
      todo[todo_end].light = CLights::FindByName (dlights[i].light);
      break;
    }
  todo_end++;
}

enum { kTokenRTexTexture = 1, kTokenRTexPlane, kTokenRTexLen };

void load_tex (char** buf, Color* colors, int num_colors, csTextureList* textures,
               char* name)
{
  static tokenDesc commands[] = {
	{kTokenRTexTexture, "TEXTURE"},
	{kTokenRTexPlane, "PLANE"},
	{kTokenRTexLen, "LEN"},
	{0,0}};
  long cmd;
  char* params;
  char str[255];

  strcpy (colors[num_colors].poly, name);
  colors[num_colors].plane[0] = 0;
  colors[num_colors].texture = NULL;
  colors[num_colors].len = 0;

  while ((cmd = csGetCommand (buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenRTexTexture:
        ScanStr (params, "%s", str);
	colors[num_colors].texture = textures->GetTextureMM (str);
	if (colors[num_colors].texture == NULL)
	{
	  CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n"
                    "Make sure that the TEXTURES statement has an entry for this texture.\n", str);
	  fatal_exit (0, false);
	}
	break;
      case kTokenRTexPlane:
        ScanStr (params, "%s", str);
	strcpy (colors[num_colors].plane, str);
	break;
      case kTokenRTexLen:
        ScanStr (params, "%f", &colors[num_colors].len);
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a texture specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
}

enum { kTokenRoomMove = 1, kTokenRoomTexture, kTokenRoomTexLight,
       kTokenRoomTexMM, kTokenRoomTexCeil, kTokenRoomTexFloor,
       kTokenRoomLightX, kTokenRoomTex, kTokenRoomTexScale,
       kTokenRoomDim, kTokenRoomFlHeight, kTokenRoomHeight,
       kTokenRoomFloor, kTokenRoomFloorCeil, kTokenRoomCeil,
       kTokenRoomLight, kTokenRoomSixface, kTokenRoomThing,
       kTokenRoomOcclus, kTokenRoomPortal, kTokenRoomSplit,
       kTokenRoomTrigger, kTokenRoomActivate, kTokenRoomStatBsp,
       kTokenRoomBsp, kTokenRoomSprite, kTokenRoomFog };

enum { kTokenPortPoly = 1, kTokenPortSector, kTokenPortAlpha,
       kTokenPortWarp };

enum { kTokenWarpMatrix = 1, kTokenWarpVector, kTokenWarpVectorAfter, 
       kTokenWarpMirror, kTokenWarpStatic };

csSector* CSLoader::load_room (char* secname, csWorld* w, char* buf, 
                             csTextureList* textures)
{
  static tokenDesc commands[] = {
  	{kTokenRoomMove, "MOVE"},
	{kTokenRoomTexLight, "TEXTURE_LIGHTING"}, 
	{kTokenRoomTexMM, "TEXTURE_MIPMAP"}, 
	{kTokenRoomTexScale, "TEXTURE_SCALE"}, 
	{kTokenRoomTexture, "TEXTURE"}, 
	{kTokenRoomTex, "TEX"}, 
	{kTokenRoomTexCeil, "CEIL_TEXTURE"}, 
	{kTokenRoomLightX, "LIGHTX"}, 
	{kTokenRoomLight, "LIGHT"}, 
	{kTokenRoomDim, "DIM"}, 
	{kTokenRoomHeight, "HEIGHT"}, 
	{kTokenRoomFlHeight, "FLOOR_HEIGHT"}, 
	{kTokenRoomFloorCeil, "FLOOR_CEIL"}, 
	{kTokenRoomTexFloor, "FLOOR_TEXTURE"}, 
	{kTokenRoomFloor, "FLOOR"}, 
	{kTokenRoomCeil, "CEILING"}, 
	{kTokenRoomSixface, "SIXFACE"}, 
	{kTokenRoomThing, "THING"}, 
	{kTokenRoomPortal, "PORTAL"}, 
	{kTokenRoomSplit, "SPLIT"}, 
	{kTokenRoomTrigger, "TRIGGER"}, 
	{kTokenRoomActivate, "ACTIVATE"}, 
	{kTokenRoomBsp, "BSP"},
	{kTokenRoomStatBsp, "STATBSP"},
	{kTokenRoomSprite, "SPRITE"},
	{kTokenRoomFog, "FOG"},
	{0,0}};
  static tokenDesc pCommands[] = {
  	{kTokenPortPoly, "POLYGON"},
	{kTokenPortSector, "SECTOR"},
	{kTokenPortAlpha, "ALPHA"},
	{kTokenPortWarp, "WARP"},
	{0,0}};
  static tokenDesc mCommands[] = {
	{kTokenWarpMatrix, "MATRIX"},
	{kTokenWarpVector, "V"},
	{kTokenWarpVectorAfter, "W"},
	{kTokenWarpMirror, "MIRROR"},
	{kTokenWarpStatic, "MIRROR"},
	{0,0}};
  static tokenDesc tok_matrix[] = {{1, "MATRIX"}, {0,0}};
  static tokenDesc tok_vector[] = {{1, "V"}, {0,0}};
  char* name;
  long cmd;
  char* params, * params2;
  int i, l;
  int i1, i2, i3, i4;
  bool do_bsp = false;
  bool do_stat_bsp = false;

  CHK(csSector* sector = new csSector());
  csNameObject::AddName(*sector,secname);

  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  LoadStat::sectors_loaded++;

  csMatrix3 mm;
  csVector3 vm (0, 0, 0);
  csTextureHandle* texture = NULL;
  float tscale = 1;
  int no_mipmap = false, no_lighting = false;

  int num_portals = 0;
  RPortal portals[MAX_ROOM_PORTALS];

  int num_splits = 0;
  Split to_split[MAX_ROOM_SPLIT];

  int num_colors = 0;
  Color colors[MAX_ROOM_COLORS];

  int num_light = 0;
  DLight dlights[MAX_ROOM_LIGHT];

  csVector3 v0, v1, v2, v3, v4, v5, v6, v7;
  v0.x = -1; v0.y =  1; v0.z =  1;
  v1.x =  1; v1.y =  1; v1.z =  1;
  v2.x = -1; v2.y = -1; v2.z =  1;
  v3.x =  1; v3.y = -1; v3.z =  1;
  v4.x = -1; v4.y =  1; v4.z = -1;
  v5.x =  1; v5.y =  1; v5.z = -1;
  v6.x = -1; v6.y = -1; v6.z = -1;
  v7.x =  1; v7.y = -1; v7.z = -1;
  float r;

  char str[255];
  char str2[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenRoomBsp:
        do_bsp = true;
	do_stat_bsp = false;
	break;
      case kTokenRoomStatBsp:
        do_stat_bsp = true;
	do_bsp = false;
	break;
      case kTokenRoomMove:
	{
	  char* params2;
	  csGetObject (&params, tok_matrix, &name, &params2);
	  mm = CSLoader::load_matrix (params2);
	  csGetObject (&params, tok_vector, &name, &params2);
	  vm = CSLoader::load_vector (params2);
	}
	break;
      case kTokenRoomTexture:
	ScanStr (params, "%s", str);
        texture = textures->GetTextureMM (str);
        if (texture == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n"
                    "Make sure that the TEXTURES statement has an entry for"
                    " this texture.\n", str);
	  fatal_exit (0, false);
        }
	break;
      case kTokenRoomTexLight:
	ScanStr (params, "%b", &no_lighting); no_lighting = !no_lighting;
	break;
      case kTokenRoomTexMM:
	ScanStr (params, "%b", &no_mipmap); no_mipmap = !no_mipmap;
	break;
      case kTokenRoomTexCeil:
      case kTokenRoomTexFloor:
	ScanStr (params, "%s", str);
        colors[num_colors].texture = textures->GetTextureMM (str);
        if (colors[num_colors].texture == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find texture named '%s'!\n"
                    "Make sure that the TEXTURES statement has an entry "
                    "for this texture.\n", str);
	  fatal_exit (0, false);
        }
        strcpy (colors[num_colors].poly, 
                cmd == kTokenRoomTexCeil ? "up" : "down");
        colors[num_colors].plane[0] = 0;
        if (num_colors >= MAX_ROOM_COLORS)
        {
          CsPrintf (MSG_FATAL_ERROR, "OVERFLOW number of colors in room!\n");
	  fatal_exit (0, false);
        }
        num_colors++;
	break;
      case kTokenRoomLightX:
	ScanStr (params, "%s,%s", str, str2);
        strcpy (dlights[num_light].poly, str);
        strcpy (dlights[num_light].light, str2);
        num_light++;
	break;
      case kTokenRoomTex:
	load_tex (&params, colors, num_colors, textures, name);
	num_colors++;
	break;
      case kTokenRoomTexScale:
	ScanStr (params, "%f", &tscale);
	break;
      case kTokenRoomDim:
	{
	  float rx, ry, rz;
	  ScanStr (params, "%f,%f,%f", &rx, &ry, &rz);
	  rx /= 2; ry /= 2; rz /= 2;
          v0.x = -rx; v0.y =  ry; v0.z =  rz;
          v1.x =  rx; v1.y =  ry; v1.z =  rz;
          v2.x = -rx; v2.y = -ry; v2.z =  rz;
          v3.x =  rx; v3.y = -ry; v3.z =  rz;
          v4.x = -rx; v4.y =  ry; v4.z = -rz;
          v5.x =  rx; v5.y =  ry; v5.z = -rz;
          v6.x = -rx; v6.y = -ry; v6.z = -rz;
          v7.x =  rx; v7.y = -ry; v7.z = -rz;
	}
	break;
      case kTokenRoomFlHeight:
	ScanStr (params, "%f", &r);
        v0.y = r+v0.y-v2.y;
        v1.y = r+v1.y-v3.y;
        v4.y = r+v4.y-v6.y;
        v5.y = r+v5.y-v7.y;
        v2.y = r;
        v3.y = r;
        v6.y = r;
        v7.y = r;
	break;
      case kTokenRoomHeight:
	ScanStr (params, "%f", &r);
        v0.y = r+v2.y;
        v1.y = r+v3.y;
        v4.y = r+v6.y;
        v5.y = r+v7.y;
	break;
      case kTokenRoomFloorCeil:
	ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
	   	 &v2.x, &v2.z, &v3.x, &v3.z, &v7.x, &v7.z, &v6.x, &v6.z);
        v0 = v2;
	v1 = v3;
	v5 = v7;
	v4 = v6;
	break;
      case kTokenRoomFloor:
	ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
	  	 &v2.x, &v2.y, &v2.z, &v3.x, &v3.y, &v3.z, 
                 &v7.x, &v7.y, &v7.z, &v6.x, &v6.y, &v6.z);
	break;
      case kTokenRoomCeil:
	ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
	  	 &v0.x, &v0.y, &v0.z, &v1.x, &v1.y, &v1.z, 
                 &v5.x, &v5.y, &v5.z, &v4.x, &v4.y, &v4.z);
	break;
      case kTokenRoomLight:
        sector->AddLight ( load_statlight(params) );
	break;
      case kTokenRoomSixface:
        sector->AddThing ( load_sixface(name,w,params,textures,sector) );
	break;
      case kTokenRoomFog:
	{
	  csFog& f = sector->GetFog ();
	  f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
	}
        break;
      case kTokenRoomSprite:
	{
          CHK (csSprite3D* sp = new csSprite3D ());
          csNameObject::AddName(*sp,name);
          CSLoader::LoadSprite (sp, w, params, textures);
	  w->sprites.Push (sp);
	  sp->MoveToSector (sector);
	}
	break;
      case kTokenRoomThing:
        sector->AddThing ( load_thing(name,w,params,textures,sector) );
	break;
      case kTokenRoomPortal:
        {  
          if (num_portals >= MAX_ROOM_PORTALS)
          {
	    CsPrintf (MSG_FATAL_ERROR, 
                      "OVERFLOW with number of portals in room!\n");
	    fatal_exit (0, false);
          }
	  portals[num_portals].is_warp = false;
	  portals[num_portals].alpha = 0;
          while ((cmd = csGetObject (&params, pCommands, &name, &params2)) > 0)
          {
            switch (cmd)
	    {
	      case kTokenPortPoly:
	        ScanStr (params2, "%s", portals[num_portals].poly);
		break;
	      case kTokenPortSector:
	        ScanStr (params2, "%s", portals[num_portals].sector);
		break;
	      case kTokenPortAlpha:
	        ScanStr (params2, "%d", &portals[num_portals].alpha);
		break;
	      case kTokenPortWarp:
	  	{
		  portals[num_portals].do_static = false;
		  char* params3;
          	  while ((cmd = csGetObject (&params2, mCommands, &name, &params3)) > 0)
          	  {
            	    switch (cmd)
	    	    {
	      	      case kTokenWarpMatrix:
	        	portals[num_portals].m_warp 
                                          = CSLoader::load_matrix (params3);
	        	portals[num_portals].do_mirror = false;
	        	break;
	      	      case kTokenWarpVector:
	        	portals[num_portals].v_warp_before 
                                          = CSLoader::load_vector (params3);
			portals[num_portals].v_warp_after 
                                          = portals[num_portals].v_warp_before;
	        	portals[num_portals].do_mirror = false;
	        	break;
	      	      case kTokenWarpVectorAfter:
	        	portals[num_portals].v_warp_after 
                                          = CSLoader::load_vector (params3);
	        	portals[num_portals].do_mirror = false;
	        	break;
	      	      case kTokenWarpMirror:
	        	portals[num_portals].do_mirror = true;
	        	break;
	      	      case kTokenWarpStatic:
	        	portals[num_portals].do_static = true;
	        	break;
	    	    }
		  }
        	  portals[num_portals].is_warp = true;
		}
		break;
	    }
	  }
	}
        num_portals++;
	break;
      case kTokenRoomSplit:
	{
	  ScanStr (params, "%s,%s(%F)", 
                   to_split[num_splits].poly, str, to_split[num_splits].widA,
	  	   &to_split[num_splits].cnt);
          if (!strcmp (str, "VER")) to_split[num_splits].dir = 0;
          else if (!strcmp (str, "HOR")) to_split[num_splits].dir = 1;
          else
          {
            CsPrintf (MSG_FATAL_ERROR, 
                      "Expected 'VER' or 'HOR' in SPLIT statement!\n");
	    fatal_exit (0, false);
          }
          if (to_split[num_splits].cnt >= MAX_ROOM_SPLIT)
          {
            CsPrintf (MSG_FATAL_ERROR, "OVERFLOW number of splits in room!\n");
	    fatal_exit (0, false);
          }
          num_splits++;
	}
	break;
      case kTokenRoomActivate:
	ScanStr (params, "%s", str);
	{
	  csScript* s = (csScript*)w->scripts.FindByName (str);
	  if (!s)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str);
	    fatal_exit (0, false);
	  }
          csObjectTrigger* objtrig = csObjectTrigger::GetTrigger(*sector);
          if (!objtrig)
          {
            CHK(objtrig = new csObjectTrigger());
            sector->ObjAdd(objtrig);
          }
	  objtrig->NewActivateTrigger (s);
	  objtrig->DoActivateTriggers ();
	}
	break;
      case kTokenRoomTrigger:
	ScanStr (params, "%s,%s", str, str2);
	if (!strcmp (str, "activate"))
	{
	  csScript* s = (csScript*)w->scripts.FindByName (str2);
	  if (!s)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Don't know script '%s'!\n", str2);
	    fatal_exit (0, false);
	  }
          csObjectTrigger* objtrig = csObjectTrigger::GetTrigger(*sector);
	  if (!objtrig) 
          {
            CHK(objtrig = new csObjectTrigger());
            sector->ObjAdd(objtrig);
          }
          objtrig->NewActivateTrigger (s);
	}
        else
        {
	  CsPrintf (MSG_FATAL_ERROR, 
                    "Trigger '%s' not supported or known for object!\n", str2);
	  fatal_exit (0, false);
        }
	break;
      default:
	CsPrintf (MSG_FATAL_ERROR, "Unrecognized token in room '%s'!\n",
                  csNameObject::GetName(*sector));
	fatal_exit (0, false);
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a room!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  csVector3 v, vv;
  vv = vm + mm * v0; sector->AddVertex (vv);
  vv = vm + mm * v1; sector->AddVertex (vv);
  vv = vm + mm * v2; sector->AddVertex (vv);
  vv = vm + mm * v3; sector->AddVertex (vv);
  vv = vm + mm * v4; sector->AddVertex (vv);
  vv = vm + mm * v5; sector->AddVertex (vv);
  vv = vm + mm * v6; sector->AddVertex (vv);
  vv = vm + mm * v7; sector->AddVertex (vv);

  csPolygon3D* p;

  Todo todo[100];
  int done = 0;
  int todo_end = 0;

  add_to_todo (todo, todo_end, "north", 0, 1, 3, 2, 0, 1, 
               texture, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "east", 1, 5, 7, 3, 1, 5, 
               texture, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "south", 5, 4, 6, 7, 5, 4, 
               texture, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "west", 4, 0, 2, 6, 4, 0, 
               texture, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "up", 4, 5, 1, 0, 4, 5, 
               texture, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "down", 2, 3, 7, 6, 2, 3, 
               texture, -1, NULL, colors, num_colors, dlights, num_light);

  int split;
  while (done < todo_end)
  {
    split = false;
    for (i = 0 ; i < num_splits ; i++)
      if (!strcmp (todo[done].poly, to_split[i].poly))
      {
        split = true;
	break;
      }

    if (split)
    {
      char pname[255];
      if (to_split[i].dir)
      {
	// Horizontal
	i1 = todo[done].v1;
	i2 = todo[done].v2;
	i3 = todo[done].v3;
	i4 = todo[done].v4;

	for (l = 0 ; l < to_split[i].cnt ; l++)
	{
	  csMath3::Between (sector->Vwor (i1), sector->Vwor (i2), 
                            v, -1, to_split[i].widA[l]);
	  sector->AddVertex (v);
	  csMath3::Between (sector->Vwor (i4), sector->Vwor (i3), 
                            v, -1, to_split[i].widA[l]);
	  sector->AddVertex (v);

	  sprintf (pname, "%s%c", todo[done].poly, l+'A');
  	  add_to_todo(todo, todo_end, pname, i1, sector->GetNumVertices ()-2, 
                      sector->GetNumVertices ()-1, i4, todo[done].tv1, 
	  	      todo[done].tv2, todo[done].texture, todo[done].col_idx, 
                      todo[done].light, colors, num_colors, dlights,num_light);
	  i1 = sector->GetNumVertices () - 2;
	  i4 = sector->GetNumVertices () - 1;
	}

	sprintf (pname, "%s%c", todo[done].poly, l+'A');
  	add_to_todo (todo, todo_end, pname, i1, i2, i3, i4, todo[done].tv1, 
		     todo[done].tv2, todo[done].texture, todo[done].col_idx,
                     todo[done].light, colors, num_colors, dlights, num_light);
      }
      else
      {
	// Vertical
	i1 = todo[done].v1;
	i2 = todo[done].v2;
	i3 = todo[done].v3;
	i4 = todo[done].v4;

	for (l = 0 ; l < to_split[i].cnt ; l++)
	{
	  csMath3::Between (sector->Vwor (i4), sector->Vwor (i1), v, -1, 
                            to_split[i].widA[l]);
	  sector->AddVertex (v);
	  csMath3::Between (sector->Vwor (i3), sector->Vwor (i2), v, -1, 
                            to_split[i].widA[l]);
	  sector->AddVertex (v);

	  sprintf (pname, "%s%d", todo[done].poly, l+1);
  	  add_to_todo(todo, todo_end, pname, sector->GetNumVertices () - 2, 
                      sector->GetNumVertices () - 1, 
                      i3, i4, todo[done].tv1, todo[done].tv2,
	  	      todo[done].texture, todo[done].col_idx, todo[done].light,
                      colors, num_colors, dlights, num_light);
	  i3 = sector->GetNumVertices () - 1;
	  i4 = sector->GetNumVertices () - 2;
	}

	sprintf (pname, "%s%d", todo[done].poly, l+1);
  	add_to_todo (todo, todo_end, pname, i1, i2, i3, i4, todo[done].tv1, 
		     todo[done].tv2, todo[done].texture, todo[done].col_idx, 
                     todo[done].light, colors, num_colors, dlights, num_light);
      }
    }
    else
    {
      float len;
      int idx = todo[done].col_idx;
      if (idx == -1 || colors[idx].texture == NULL) 
        texture = todo[done].texture;
      else texture = colors[idx].texture;

      p = sector->NewPolygon (texture);
      csNameObject::AddName(*p,todo[done].poly);
      p->AddVertex (todo[done].v1);
      p->AddVertex (todo[done].v2);
      p->AddVertex (todo[done].v3);
      p->AddVertex (todo[done].v4);
      len = tscale;
      if (idx != -1 && colors[idx].len) len = colors[idx].len;
      if (idx == -1 || colors[idx].plane[0] == 0)
	p->SetTextureSpace (sector->Vwor (todo[done].tv1), 
                              sector->Vwor (todo[done].tv2), len);
      else
	p->SetTextureSpace ((csPolyPlane*)w->planes.FindByName (colors[idx].plane));
      p->SetMipmapping (!no_mipmap);
      p->SetLighting (!no_lighting);
      p->theDynLight = todo[done].light;
    }
    done++;
  }

  csSector* portal;

  for (i = 0 ; i < num_portals ; i++)
  {
    p = sector->GetPolygon (portals[i].poly);
    if (!p)
    {
      CsPrintf (MSG_FATAL_ERROR, "Error locating polygon '%s' in room '%s'!\n",
                portals[i].poly, name);
      fatal_exit (0, false);
    }

    // This will later be defined correctly
    CHK( portal = new csSector () );  
    csNameObject::AddName(*portal,portals[i].sector);
    p->SetCSPortal (portal);
    LoadStat::portals_loaded++;
    if (portals[i].is_warp)
    {
      if (portals[i].do_mirror)
        p->SetWarp (csTransform::GetReflect ( *(p->GetPolyPlane ()) ));
      else p->SetWarp (portals[i].m_warp, portals[i].v_warp_before, 
                        portals[i].v_warp_after);
      p->GetPortal  ()->SetStaticDest (portals[i].do_static);
    }
    p->SetAlpha (portals[i].alpha);
  }

  if (do_bsp) sector->UseBSP ();
  if (do_stat_bsp) sector->UseStaticBSP ();

  return sector;
}

enum { kTokenSectorThing = kTokenPSetLast, kTokenSectorSixface,
       kTokenSectorLight, kTokenSectorOcclus, kTokenSectorSprite,
       kTokenSectorStatBsp, kTokenSectorSkyDome };

csSector* CSLoader::load_sector (char* secname, csWorld* w, char* buf, 
                               csTextureList* textures)
{
  static tokenDesc commands[] = {
  	{kTokenPSetVertex, "VERTEX"},
  	{kTokenPSetCircle, "CIRCLE"},
	{kTokenPSetPolygon, "POLYGON"},
	{kTokenPSetTexNr, "TEXNR"},
	{kTokenPSetTexlen, "TEXLEN"},
	{kTokenPSetTrigger, "TRIGGER"},
	{kTokenPSetActivate, "ACTIVATE"},
	{kTokenPSetLightX, "LIGHTX"},
	{kTokenPSetFog, "FOG"},
	{kTokenPSetBsp, "BSP"},
	{kTokenSectorStatBsp, "STATBSP"},
	{kTokenSectorThing, "THING"},
	{kTokenSectorSixface, "SIXFACE"},
	{kTokenSectorLight, "LIGHT"},
	{kTokenSectorSprite, "SPRITE"},
	{kTokenSectorSkyDome, "SKYDOME"},
	{0,0}};
  char* name;
  long cmd;
  char* params;
  bool do_stat_bsp = false;

  CHK( csSector* sector = new csSector() );
  csNameObject::AddName(*sector,secname);

  LoadStat::sectors_loaded++;
  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  PSLoadInfo info(w,textures);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenSectorSkyDome:
        CSLoader::skydome_process (*sector, name, params, info.default_texture);
        break;
      case kTokenSectorStatBsp:
        do_stat_bsp = true;
	info.use_bsp = false;
	break;
      case kTokenSectorThing:
        sector->AddThing ( load_thing(name,w,params,textures,sector) );
	break;
      case kTokenSectorSprite:
	{
          CHK (csSprite3D* sp = new csSprite3D ());
          csNameObject::AddName(*sp,name);
          CSLoader::LoadSprite (sp, w, params, textures);
	  w->sprites.Push (sp);
	  sp->MoveToSector (sector);
	}
	break;
      case kTokenSectorSixface:
        sector->AddThing ( load_sixface(name,w,params,textures,sector) );
	break;
      case kTokenSectorLight:
        sector->AddLight ( load_statlight(params) );
	break;
      default:
	CSLoader::ps_process (*sector, info, cmd, name, params);
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a sector!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (info.use_bsp) sector->UseBSP ();
  if (do_stat_bsp) sector->UseStaticBSP ();
  return sector;
}

enum { kTokenSkyRadius = 1, kTokenSkyVertices };

void CSLoader::skydome_process (csSector& sector, char* name, char* buf,
	csTextureHandle* texture)
{
  static tokenDesc commands[] = {
        {kTokenSkyRadius, "RADIUS"},
        {kTokenSkyVertices, "VERTICES"},
        {0,0}};
  long cmd;
  char* params;
  float radius;
  int i, j;
  int num;

  // Previous vertices.
  int prev_vertices[60];	// @@@ HARDCODED!
  float prev_u[60];
  float prev_v[60];

  char poly_name[30], * end_poly_name;
  strcpy (poly_name, name);
  end_poly_name = strchr (poly_name, 0);

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenSkyRadius:
        ScanStr (params, "%f", &radius);
        break;
      case kTokenSkyVertices:
        ScanStr (params, "%D", prev_vertices, &num);
	break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a skydome!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  csMatrix3 t_m;
  csVector3 t_v;

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
  int new_vertices[60];		// @@@ HARDCODED == BAD == EASY!
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
      new_vertices[j] = sector.AddVertex (
      			 new_radius * cos (angle),
			 new_height,
      			 new_radius * sin (angle));
      new_u[j] = uv_radius * cos (angle) + .5;
      new_v[j] = uv_radius * sin (angle) + .5;
    }

    //-----
    // Now make the triangle strips.
    //-----
    for (j = 0 ; j < num ; j++)
    {
      sprintf (end_poly_name, "%d_%d_A", i, j);
      CHK (csPolygon3D* p = new csPolygon3D (texture));
      csNameObject::AddName(*p,poly_name);
      p->SetSector (&sector);
      p->SetParent (&sector);
      p->SetLighting (true);
      p->SetMipmapping (false);
      p->SetCosinusFactor (1);
      p->AddVertex (prev_vertices[j]);
      p->AddVertex (new_vertices[(j+1)%num]);
      p->AddVertex (new_vertices[j]);
      p->SetUV (0, prev_u[j], prev_v[j]);
      p->SetUV (1, new_u[(j+1)%num], new_v[(j+1)%num]);
      p->SetUV (2, new_u[j], new_v[j]);
      p->SetTextureSpace (t_m, t_v);
      sector.AddPolygon (p);
      LoadStat::polygons_loaded++;
      sprintf (end_poly_name, "%d_%d_B", i, j);
      CHK (p = new csPolygon3D (texture));
      csNameObject::AddName(*p,poly_name);
      p->SetSector (&sector);
      p->SetParent (&sector);
      p->SetLighting (false);
      p->SetMipmapping (false);
      p->SetCosinusFactor (1);
      p->AddVertex (prev_vertices[j]);
      p->AddVertex (prev_vertices[(j+1)%num]);
      p->AddVertex (new_vertices[(j+1)%num]);
      p->SetUV (0, prev_u[j], prev_v[j]);
      p->SetUV (1, prev_u[(j+1)%num], prev_v[(j+1)%num]);
      p->SetUV (2, new_u[(j+1)%num], new_v[(j+1)%num]);
      p->SetTextureSpace (t_m, t_v);
      sector.AddPolygon (p);
      LoadStat::polygons_loaded++;
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
  int top_vertex = sector.AddVertex (0, vert_radius, 0);
  float top_u = .5;
  float top_v = .5;

  //-----
  // Make the top triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    sprintf (end_poly_name, "%d_%d", num/2, j);
    CHK (csPolygon3D* p = new csPolygon3D (texture));
    csNameObject::AddName(*p,poly_name);
    p->SetSector (&sector);
    p->SetParent (&sector);
    p->SetLighting (true);
    p->SetMipmapping (false);
    p->SetCosinusFactor (1);
    p->AddVertex (top_vertex);
    p->AddVertex (prev_vertices[j]);
    p->AddVertex (prev_vertices[(j+1)%num]);
    p->SetUV (0, top_u, top_v);
    p->SetUV (1, prev_u[j], prev_v[j]);
    p->SetUV (2, prev_u[(j+1)%num], prev_v[(j+1)%num]);
    p->SetTextureSpace (t_m, t_v);
    sector.AddPolygon (p);
    LoadStat::polygons_loaded++;
  }
}

//---------------------------------------------------------------------------

bool CSLoader::load_library_def (csLibrary* library, csWorld* w, char* file, csTextureList* textures)
{
  library->Clear ();

  FILE* fp;
  fp = fopen (file, "rb");
  if (!fp)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open library file '%s'!\n", file);
    return false;
  }

  char* buf;

  Archive* ar;

  // First check if the library file is a ZIP archive.
  char hdr[5];
  fread (hdr, 4, 1, fp);
  hdr[4] = 0;
  if (!strcmp (hdr, "PK\003\004"))
  {
    fclose (fp);
    fp = NULL;
    CHK (ar = new Archive (file));
    buf = ar->read ("library");
    if (!buf)
    {
      CsPrintf (MSG_FATAL_ERROR, "Could not find 'library' file inside ZIP archive '%s'!\n\
Are you sure '%s' is a valid Crystal Space library?\n", file, file);
      CHK (delete ar);
      return false;
    }
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "'%s' is not a valid ZIP archive!\n", file);
    return false;
  }

  CsPrintf (MSG_INITIALIZATION, "Loading library '%s' from '%s'.\n", 
            csNameObject::GetName(*library), file);
  bool rc = load_library_def (library, w, ar, buf, textures);

  library->SetArchive (ar);

  CHK (delete [] buf);
  if (fp) fclose (fp);

  return rc;
}

enum { kTokenLibDefThing = 1, kTokenLibDefSprite, kTokenLibDefTexs, kTokenLibDefSounds };

bool CSLoader::load_library_def (csLibrary* library, csWorld* w, Archive* ar, char* buf, csTextureList* textures)
{
  static tokenDesc tokens[] = {{1, "LIBRARY"}, {0,0}};
  static tokenDesc commands[] = {
  	{kTokenLibDefTexs, "TEXTURES"},
  	{kTokenLibDefThing, "THING"},
  	{kTokenLibDefSprite, "SPRITE"},
  	{kTokenLibDefSounds, "SOUNDS"},
	{0,0}};
  char* name, * data;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    long cmd;
    char* params;

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      switch (cmd)
      {
        case kTokenLibDefTexs:
	  // Append textures to world.
          if (!CSLoader::LoadTextures (w->GetTextures (), params, w, ar))
	    return false;
	  break;
        case kTokenLibDefSounds:
          if (!CSLoader::LoadSounds (w, params, ar))
            return false;
	  break;
        case kTokenLibDefSprite:
	  {
	    csSpriteTemplate* t = (csSpriteTemplate*)library->sprite_templates.FindByName (name);
	    if (!t)
	    {
	      CHK (t = new csSpriteTemplate ());
              csNameObject::AddName(*t, name);
	      library->sprite_templates.Push (t);
	    }
	    CSLoader::LoadSpriteTemplate (t, params, textures);
	  }
        case kTokenLibDefThing:
          if (!library->thing_templates.FindByName (name))
            library->thing_templates.Push (CSLoader::load_thingtpl (name, params, textures));
	  break;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a library definition!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }
  return true;
}

csSoundBufferObject* CSLoader::load_sound(char* name, csWorld* w, Archive* ar)
{
  Archive* world_file = NULL;
  if (w) world_file = w->GetWorldFile ();
  csSoundBufferObject* sndobj = NULL;
  csSoundBuffer* snd = NULL;

  size_t size;
  if (ar && ar->file_exists (name, &size))
    world_file = ar;

  if (world_file && world_file->file_exists (name, &size))
  {
    char* buf;
    buf = world_file->read (name);
    if (!buf)
    {
      CsPrintf (MSG_FATAL_ERROR,
        "Can't load file '%s' from archive although it should be there!\n"
        "Maybe the archive is corrupt?\n", name);
      return NULL;
    }
    snd = csSoundBufferLoader::load ((UByte*)buf, size);
    if (!snd)
    {
      CsPrintf (MSG_INTERNAL_ERROR, 
               "ENGINE FAILURE! Couldn't open '%s' from archive!\n", name);
    }
    CHK (delete [] buf); 
  }
  else
  {
    FILE* fp;
    w->isys->FOpen (name, "rb", &fp);
    if (!fp)
    {
      CsPrintf (MSG_FATAL_ERROR, 
        "Can't find file '%s' in data directory or in archive!\n", name);
      return NULL;
    }
    snd = csSoundBufferLoader::load (fp);
    if (!snd)
    {
      CsPrintf (MSG_INTERNAL_ERROR, 
               "ENGINE FAILURE! Couldn't open '%s' from disk!\n", name);
    }
    w->isys->FClose (fp);
  }
  CHK(sndobj = new csSoundBufferObject(snd));
  csNameObject::AddName(*sndobj,name);
  return sndobj;
}

//---------------------------------------------------------------------------

enum { kTokenWorldSector = 1, kTokenWorldPlane, kTokenWorldCol,
       kTokenWorldScript, kTokenWorldTexs, kTokenWorldLightX,
       kTokenWorldRoom, kTokenWorldThing, kTokenWorldSixface,
       kTokenWorldSpriteTmp, kTokenWorldLibrary, kTokenWorldStart,
       kTokenWorldSounds };

bool CSLoader::LoadWorld (csWorld* world, LanguageLayer* layer, char* buf)
{
  static tokenDesc tokens[] = {{1, "WORLD"}, {0,0}};
  static tokenDesc commands[] = {
  	{kTokenWorldSector, "SECTOR"},
  	{kTokenWorldRoom, "ROOM"},
  	{kTokenWorldPlane, "PLANE"},
  	{kTokenWorldCol, "COLLECTION"},
  	{kTokenWorldScript, "SCRIPT"},
  	{kTokenWorldTexs, "TEXTURES"},
  	{kTokenWorldLightX, "LIGHTX"},
  	{kTokenWorldThing, "THING"},
  	{kTokenWorldSixface, "SIXFACE"},
  	{kTokenWorldSpriteTmp, "SPRITE"},
  	{kTokenWorldLibrary, "LIBRARY"},
  	{kTokenWorldStart, "START"},
  	{kTokenWorldSounds, "SOUNDS"},
	{0,0}};
  char* name, * data;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    long cmd;
    char* params;

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      switch (cmd)
      {
  	case kTokenWorldSpriteTmp:
	  {
	    csSpriteTemplate* t = world->GetSpriteTemplate (name);
	    if (!t)
	    {
	      CHK (t = new csSpriteTemplate ());
              csNameObject::AddName(*t,name);
	      world->sprite_templates.Push (t);
	    }
	    CSLoader::LoadSpriteTemplate (t, params, world->GetTextures ());
	  }
	  break;
        case kTokenWorldThing:
          if (!world->GetThingTemplate (name))
            world->thing_templates.Push ( CSLoader::load_thingtpl(name, params, world->GetTextures ()) );
	  break;
        case kTokenWorldSixface:
          if (!world->GetThingTemplate (name))
            world->thing_templates.Push ( CSLoader::load_sixtpl(name, params, world->GetTextures ()) );
	  break;
        case kTokenWorldSector:
          if (!world->sectors.FindByName (name))
            world->sectors.Push( CSLoader::load_sector(name, world, params, world->GetTextures ()) );
	  break;
        case kTokenWorldPlane:
          world->planes.Push ( CSLoader::load_polyplane (params, name) );
	  break;
        case kTokenWorldCol:
          world->collections.Push ( CSLoader::load_collection(name,world,params) );
	  break;
        case kTokenWorldScript:
          world->NewScript (layer, name, params);
	  break;
        case kTokenWorldTexs:
	  {
	    world->GetTextures ()->Clear ();
            if (!CSLoader::LoadTextures (world->GetTextures (), params, world)) return false;
	  }
	  break;
	case kTokenWorldSounds:
	  if (!CSLoader::LoadSounds (world, params)) return false;
	  break;
	case kTokenWorldRoom:
          // Not an object but it is translated to a special sector.
          if (!world->sectors.FindByName (name))
            world->sectors.Push ( load_room(name, world, params, world->GetTextures ()) );
	  break;
        case kTokenWorldLightX:
          load_light (name, params);
	  break;
	case kTokenWorldLibrary:
          {
  	    char str[255];
	    ScanStr (params, "%s", str);
	    csLibrary* lib = (csLibrary*)(world->libraries).FindByName (name);
	    if (!lib)
	    {
	      // Library was not already loaded.
	      CHK (lib = new csLibrary ());
              csNameObject::AddName(*lib,name);
	      world->libraries.Push (lib);
	      load_library_def (lib, world, str, world->GetTextures ());
	    }
	  }
	  break;
	case kTokenWorldStart:
	  {
	    char str[255];
	    ScanStr (params, "%s,%f,%f,%f", str, &world->start_vec.x, &world->start_vec.y, &world->start_vec.z);
	    CHK (delete world->start_sector);
	    CHK (world->start_sector = new char [strlen (str)+1]);
	    strcpy (world->start_sector, str);
	  }
	  break;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a world!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  int sn = world->sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)(world->sectors)[sn];
    for (csPolygonSet *ps = s;  ps;  
         (ps==s) ? (ps = s->GetFirstThing ()) : (ps = ps->GetNext ()) )
      for (int i=0;  i < ps->GetNumPolygons ();  i++)
      {
        csPolygon3D* p = (csPolygon3D*)(ps->GetPolygon (i));
        if (p && p->GetPortal ())
        {
          csPortalCS* portal = (csPortalCS*)(p->GetPortal ());
          csSector *stmp = portal->GetSector ();
          csSector *snew = (csSector*)(world->sectors).FindByName(
                                       csNameObject::GetName(*stmp) );
	  if (!snew)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found for portal in"
                      " polygon '%s/%s'!\n", csNameObject::GetName(*stmp),
	    	      csNameObject::GetName( *((csObject*)(p->GetParent())) ),
                      csNameObject::GetName(*p));
	    fatal_exit (0, false);
	  }
          portal->SetSector (snew);
          CHK( delete stmp );
        }
      }
  }
  
  return true;
}

bool CSLoader::LoadWorldFile (csWorld* world, LanguageLayer* layer, char* file)
{
  world->StartWorld ();
  LoadStat::Init ();

  FILE* fp;
  fp = fopen (file, "rb");
  if (!fp)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open world file '%s'!\n\
The world file should either be a .ZIP level file or else an ASCII\n\
file describing how the world looks.\n", file);
    return false;
  }

  char* buf;

  // First check if the world file is a ZIP archive.
  char hdr[5];
  fread (hdr, 4, 1, fp);
  hdr[4] = 0;
  Archive* wf = NULL;
  if (!strcmp (hdr, "PK\003\004"))
  {
    fclose (fp);
    fp = NULL;
    wf = world->OpenWorldFile (file);
    buf = wf->read ("world");
    if (!buf)
    {
      CsPrintf (MSG_FATAL_ERROR, "Could not find 'world' file inside ZIP archive '%s'!\n\
Are you sure '%s' is a valid Crystal Space level archive?\n", file, file);
      world->CloseWorldFile ();
      return false;
    }
  }
  else
  {
    wf = world->OpenWorldFile ("precalc.zip");
    fseek (fp, 0, SEEK_END);
    long off = ftell (fp);
    fseek (fp, 0, SEEK_SET);
    CHK (buf = new char [off+1]);
    fread (buf, 1, off, fp);
    buf[off] = 0;
  }

  csIniFile* cfg = NULL;
  if (wf) CHKB (cfg = new csIniFile (wf, "cryst.cfg"));
  if (cfg)
  {
    csPolygon3D::def_mipmap_size = cfg->GetInt ("TextureMapper", "LIGHTMAP_SIZE", csPolygon3D::def_mipmap_size);
    CHK (delete cfg);
  }
  CsPrintf (MSG_INITIALIZATION, "Lightmap grid size = %dx%d.\n", csPolygon3D::def_mipmap_size,
      csPolygon3D::def_mipmap_size);

  if (!LoadWorld (world, layer, buf)) return false;

  if (LoadStat::polygons_loaded)
  {
    CsPrintf (MSG_INITIALIZATION, "Loaded world file:\n");
    CsPrintf (MSG_INITIALIZATION, "  %d polygons (%d portals),\n", LoadStat::polygons_loaded,
      LoadStat::portals_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d sectors, %d things, %d sprites, \n", LoadStat::sectors_loaded,
      LoadStat::things_loaded, LoadStat::sprites_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d curves and %d lights.\n", LoadStat::curves_loaded,
      LoadStat::lights_loaded);
  } /* endif */

  CHK (delete [] buf);

  if (fp) fclose (fp);
  if (world->GetWorldFile ()) world->GetWorldFile ()->write_archive ();

  return true;
}

//---------------------------------------------------------------------------

enum { kTokenTexsMaxT = 1, kTokenTexsTex };

bool CSLoader::LoadTextures (csTextureList* textures, char* buf, csWorld* world, Archive* ar)
{
  static tokenDesc commands[] = {
        {kTokenTexsMaxT, "MAX_TEXTURES"},
        {kTokenTexsTex, "TEXTURE"},
        {0,0}};
  char* name;
  long cmd;
  char* params;
  csTextureHandle* tex;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenTexsMaxT:
        // ignored for backward compatibility
        break;
      case kTokenTexsTex:
        {
          ImageFile *image = CSLoader::load_image (name, world, ar);
          tex = textures->NewTexture (image);
          csNameObject::AddName(*tex,name);
          //if (!tex->loaded_correctly ())
            //return false;
          CSLoader::txt_process (tex, params);
        }
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a matrix!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

csLibrary* CSLoader::LoadLibrary (csWorld* world, char* name, char* fname)
{
  CHK (csLibrary* lib = new csLibrary ());
  csNameObject::AddName(*lib,name);
  world->libraries.Push (lib);
  CSLoader::load_library_def (lib, world, fname, world->GetTextures ());
  return lib;
}

csTextureHandle* CSLoader::LoadTexture (csWorld* world, char* name, char* fname, Archive* ar)
{
  if (!ar && world->libraries.Length () >= 1)
    ar = ((csLibrary*)(world->libraries)[0])->GetArchive ();
  ImageFile* image = CSLoader::load_image (fname, world, ar);
  csTextureHandle* tm = world->GetTextures ()->NewTexture (image);
  csNameObject::AddName(*tm,name);
  return tm;
}

//---------------------------------------------------------------------------

bool CSLoader::LoadSounds (csWorld* world, char* buf, Archive* ar)
{
  enum { kTokenSoundsSound = 1 };

  static tokenDesc commands[] = {
        {kTokenSoundsSound, "SOUND"},
        {0,0}};
  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenSoundsSound:
        {
          csSoundBuffer *snd = csSoundBufferObject::GetSound(*world,name);
          if (!snd)
          {
            csSoundBufferObject *s = CSLoader::load_sound (name, world, ar);
            if (s) world->ObjAdd(s);
          }
        }
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the list of sounds!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

enum { kTokenSTplTexNr = 1, kTokenSTplFrame,
       kTokenSTplTriangle, kTokenSTplAction };

bool CSLoader::LoadSpriteTemplate (csSpriteTemplate* stemp, char* buf, csTextureList* textures)
{
  static tokenDesc commands[] = {
    {kTokenSTplTexNr, "TEXNR"},
    {kTokenSTplFrame, "FRAME"},
    {kTokenSTplAction, "ACTION"},
    {kTokenSTplTriangle, "TRIANGLE"},
    {0,0}};
  static tokenDesc tok_frame[] = {{2, "V"}, {0,0}};
  static tokenDesc tok_frameset[] = {{2, "F"}, {0,0}};
  char* name;
    
  long cmd;
  char* params;
  char* params2;
  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenSTplTexNr:
        {
          ScanStr (params, "%s", str);
          stemp->SetTexture (textures, str);
        }
        break;

      case kTokenSTplAction:
        {
          csSpriteAction* act = stemp->AddAction ();
          act->SetName (name);
          int d;
          char fn[64];
          while ((cmd = csGetObject (&params, tok_frameset, &name, &params2)) > 0)
          {
            switch (cmd)
            {
              case 2:
                ScanStr (params2, "%s,%d", fn, &d);
                csFrame * ff = stemp->FindFrame (fn);
                if(!ff)
                {
                  CsPrintf (MSG_FATAL_ERROR, "Error! Trying to add a unknown frame '%s' in %s action !\n",
		  	fn, act->GetName ());
                  fatal_exit (0, false);
                }
                act->AddFrame (ff, d);
                break;
            }
          }
        }
        break;

      case kTokenSTplFrame:
        {
          csFrame* fr = stemp->AddFrame ();
          fr->SetName (name);
          int i = 0;
          float x, y, z, u, v;
          while ((cmd = csGetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            switch (cmd)
            {
            case 2:
              ScanStr (params2, "%f,%f,%f:%f,%f", &x, &y, &z, &u, &v);
              // check if it's the first frame
              if (stemp->GetNumFrames () == 1)
              {
                // add vertice/texel in current frame
                if (stemp->GetNumVertices () >= fr->GetMaxVertices ())
                {
                  int more = 1;
                  stemp->SetNumVertices (stemp->GetNumVertices ()+more);
                  fr->AddVertex (more);
                }
              }
              else if (i >= stemp->GetNumVertices ())
              {
                CsPrintf (MSG_FATAL_ERROR, "Error! Trying to add too many vertices in frame '%s'!\n",
			fr->GetName ());
                fatal_exit (0, false);
              }
              fr->SetVertex (i, x, y, z);
              fr->SetTexel (i, u, v);
              i++;
              break;
            }
          }
  	  if (cmd == PARSERR_TOKENNOTFOUND)
  	  {
    	    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing frame '%s'!\n",
	    	fr->GetName (), csGetLastOffender ());
    	    fatal_exit (0, false);
  	  }
          if (i < stemp->GetNumVertices ())
          {
            CsPrintf (MSG_FATAL_ERROR, "Error! Too few vertices in frame '%s'! (%d %d)\n",
	    	fr->GetName (), i, stemp->GetNumVertices ());
            fatal_exit (0, false);
          }
        }
        break;

      case kTokenSTplTriangle:
        {
          int a, b, c;
          ScanStr (params, "%d,%d,%d", &a, &b, &c);
          stemp->GetBaseMesh ()->AddTriangle (a, b, c);
        }
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite template!\n",
    	csGetLastOffender ());
    fatal_exit (0, false);
  }

  stemp->GenerateLOD ();

  return true;
}

//---------------------------------------------------------------------------

enum { kTokenSpriteTemplate = 1, kTokenSpriteMove, kTokenSpriteTexnr};

bool CSLoader::LoadSprite (csSprite3D* spr, csWorld* w, char* buf, csTextureList* textures)
{
  static tokenDesc commands[] = {
  	{kTokenSpriteTemplate, "TEMPLATE"},
  {kTokenSpriteTexnr, "TEXNR"},
	{kTokenSpriteMove, "MOVE"},
	{0,0}};
  static tokenDesc tok_matvec[] = {{1, "MATRIX"}, {2, "V"}, {0,0}};
  char* name;

  LoadStat::sprites_loaded++;

  long cmd;
  char* params;
  char str[255], str2[255];
  csSpriteTemplate* tpl;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case kTokenSpriteMove:
        {
          char* params2;
	  spr->SetTransform (csMatrix3 ());	// Identity matrix.
	  spr->SetMove (0, 0, 0);
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            switch (cmd)
            {
             case 1: spr->SetTransform (CSLoader::load_matrix (params2));  break;
             case 2: spr->SetMove (CSLoader::load_vector (params2));  break;
            }
          }
        }
        break;

      case kTokenSpriteTemplate:
        memset (str, 0, 255);
        memset (str2, 0, 255);
        ScanStr (params, "%s,%s", str, str2);
        tpl = w->GetSpriteTemplate (str, true);
        if (tpl == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find template named '%s'!\n", str);
          fatal_exit (0, false);
        }
        if (tpl->FindAction (str2) == NULL)
        {
          CsPrintf (MSG_FATAL_ERROR, "Couldn't find action named '%s' in %s template !\n", str2, str);
          fatal_exit (0, false);
        }
	spr->SetTemplate (tpl);
	spr->SetAction (str2);
        break;

      case kTokenSpriteTexnr:
        memset (str, 0, 255);
        ScanStr (params, "%s", str);
        spr->SetTexture (str, textures);
	// unset_texture ();
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  spr->InitSprite ();
  return true;
}

//---------------------------------------------------------------------------
