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
#include "csparser/crossbld.h"
#include "csengine/sysitf.h"
#include "csengine/cscoll.h"
#include "csengine/triangle.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/thingtpl.h"
#include "csengine/cssprite.h"
#include "csengine/skeleton.h"
#include "csengine/polygon.h"
#include "csengine/polytmap.h"
#include "csengine/dynlight.h"
#include "csengine/textrans.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csengine/texture.h"
#include "csengine/curve.h"
#include "csengine/terrain.h"
#include "csengine/dumper.h"
#include "csengine/keyval.h"
#include "csterr/ddgtmesh.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/token.h"
#include "csutil/vfs.h"
#include "csutil/inifile.h"
#include "csutil/util.h"
#include "cssys/system.h"
#include "cssfxldr/sndload.h"
#include "csparser/snddatao.h"
#include "csgfxldr/csimage.h"
#include "csscript/objtrig.h"
#include "csscript/scripts.h"

typedef char ObName[30];

//---------------------------------------------------------------------------

class csLoaderStat
{
public:
  static int polygons_loaded;
  static int portals_loaded;
  static int sectors_loaded;
  static int things_loaded;
  static int lights_loaded;
  static int curves_loaded;
  static int sprites_loaded;
  static void Init()
  {
    polygons_loaded = 0;
    portals_loaded  = 0;
    sectors_loaded  = 0;
    things_loaded   = 0;
    lights_loaded   = 0;
    curves_loaded   = 0;
    sprites_loaded  = 0;
  }
};

int csLoaderStat::polygons_loaded = 0;
int csLoaderStat::portals_loaded  = 0;
int csLoaderStat::sectors_loaded  = 0;
int csLoaderStat::things_loaded   = 0;
int csLoaderStat::lights_loaded   = 0;
int csLoaderStat::curves_loaded   = 0;
int csLoaderStat::sprites_loaded  = 0;

// Define all tokens used through this file
TOKEN_DEF_START
  TOKEN_DEF (ACTION)
  TOKEN_DEF (ACTIVATE)
  TOKEN_DEF (ACTIVE)
  TOKEN_DEF (ALPHA)
  TOKEN_DEF (ATTENUATION)
  TOKEN_DEF (BECOMING_ACTIVE)
  TOKEN_DEF (BECOMING_INACTIVE)
  TOKEN_DEF (BEZIER)
  TOKEN_DEF (BSP)
  TOKEN_DEF (CEILING)
  TOKEN_DEF (CEIL_TEXTURE)
  TOKEN_DEF (CENTER)
  TOKEN_DEF (CIRCLE)
  TOKEN_DEF (CLIP)
  TOKEN_DEF (COLLECTION)
  TOKEN_DEF (COLOR)
  TOKEN_DEF (COLORS)
  TOKEN_DEF (CONVEX)
  TOKEN_DEF (COSFACT)
  TOKEN_DEF (CURVECENTER)
  TOKEN_DEF (CURVECONTROL)
  TOKEN_DEF (CURVESCALE)
  TOKEN_DEF (DIM)
  TOKEN_DEF (DYNAMIC)
  TOKEN_DEF (F)
  TOKEN_DEF (FILE)
  TOKEN_DEF (FILTER)
  TOKEN_DEF (FIRST)
  TOKEN_DEF (FIRST_LEN)
  TOKEN_DEF (FLATCOL)
  TOKEN_DEF (FLOOR)
  TOKEN_DEF (FLOOR_CEIL)
  TOKEN_DEF (FLOOR_HEIGHT)
  TOKEN_DEF (FLOOR_TEXTURE)
  TOKEN_DEF (FOG)
  TOKEN_DEF (FRAME)
  TOKEN_DEF (GOURAUD)
  TOKEN_DEF (HALO)
  TOKEN_DEF (HEIGHT)
  TOKEN_DEF (IDENTITY)
  TOKEN_DEF (KEY)
  TOKEN_DEF (LEN)
  TOKEN_DEF (LIBRARY)
  TOKEN_DEF (LIGHT)
  TOKEN_DEF (LIGHTING)
  TOKEN_DEF (LIGHTX)
  TOKEN_DEF (LIMB)
  TOKEN_DEF (MATRIX)
  TOKEN_DEF (MAX_TEXTURES)
  TOKEN_DEF (MIPMAP)
  TOKEN_DEF (MIRROR)
  TOKEN_DEF (MOVE)
  TOKEN_DEF (MOVEABLE)
  TOKEN_DEF (NODE)
  TOKEN_DEF (ORIG)
  TOKEN_DEF (PLANE)
  TOKEN_DEF (POLYGON)
  TOKEN_DEF (PORTAL)
  TOKEN_DEF (POSITION)
  TOKEN_DEF (PRIMARY_ACTIVE)
  TOKEN_DEF (PRIMARY_INACTIVE)
  TOKEN_DEF (RADIUS)
  TOKEN_DEF (ROOM)
  TOKEN_DEF (ROT)
  TOKEN_DEF (ROT_X)
  TOKEN_DEF (ROT_Y)
  TOKEN_DEF (ROT_Z)
  TOKEN_DEF (SCALE)
  TOKEN_DEF (SCALE_X)
  TOKEN_DEF (SCALE_Y)
  TOKEN_DEF (SCALE_Z)
  TOKEN_DEF (SCRIPT)
  TOKEN_DEF (SECOND)
  TOKEN_DEF (SECONDARY_ACTIVE)
  TOKEN_DEF (SECONDARY_INACTIVE)
  TOKEN_DEF (SECOND_LEN)
  TOKEN_DEF (SECTOR)
  TOKEN_DEF (SIXFACE)
  TOKEN_DEF (SKELETON)
  TOKEN_DEF (SKYDOME)
  TOKEN_DEF (SOUND)
  TOKEN_DEF (SOUNDS)
  TOKEN_DEF (SPLIT)
  TOKEN_DEF (SPRITE)
  TOKEN_DEF (START)
  TOKEN_DEF (STATBSP)
  TOKEN_DEF (STATELESS)
  TOKEN_DEF (STATIC)
  TOKEN_DEF (TEMPLATE)
  TOKEN_DEF (TERRAIN)
  TOKEN_DEF (HEIGHTMAP)
  TOKEN_DEF (DETAIL)
  TOKEN_DEF (TEX)
  TOKEN_DEF (TEXLEN)
  TOKEN_DEF (TEXNR)
  TOKEN_DEF (TEXTURE)
  TOKEN_DEF (TEXTURES)
  TOKEN_DEF (TEXTURE_LIGHTING)
  TOKEN_DEF (TEXTURE_MIPMAP)
  TOKEN_DEF (TEXTURE_SCALE)
  TOKEN_DEF (TEX_SET)
  TOKEN_DEF (TEX_SET_SELECT)
  TOKEN_DEF (THING)
  TOKEN_DEF (TRANSFORM)
  TOKEN_DEF (TRANSPARENT)
  TOKEN_DEF (TRIANGLE)
  TOKEN_DEF (TRIGGER)
  TOKEN_DEF (UV)
  TOKEN_DEF (UVA)
  TOKEN_DEF (UV_SHIFT)
  TOKEN_DEF (V)
  TOKEN_DEF (VERTEX)
  TOKEN_DEF (VERTICES)
  TOKEN_DEF (W)
  TOKEN_DEF (WARP)
  TOKEN_DEF (WORLD)
TOKEN_DEF_END

//---------------------------------------------------------------------------

csMatrix3 csLoader::load_matrix (char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (IDENTITY)
    TOKEN_TABLE (ROT_X)
    TOKEN_TABLE (ROT_Y)
    TOKEN_TABLE (ROT_Z)
    TOKEN_TABLE (ROT)
    TOKEN_TABLE (SCALE_X)
    TOKEN_TABLE (SCALE_Y)
    TOKEN_TABLE (SCALE_Z)
    TOKEN_TABLE (SCALE)
  TOKEN_TABLE_END

  char* params;
  int cmd, num;
  float angle;
  float scaler;
  float list[30];
  const csMatrix3 identity;
  csMatrix3 M;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_IDENTITY:
        M = identity;
        break;
      case TOKEN_ROT_X:
        ScanStr (params, "%f", &angle);
        M *= csXRotMatrix3 (angle);
        break;
      case TOKEN_ROT_Y:
        ScanStr (params, "%f", &angle);
        M *= csYRotMatrix3 (angle);
        break;
      case TOKEN_ROT_Z:
        ScanStr (params, "%f", &angle);
        M *= csZRotMatrix3 (angle);
        break;
      case TOKEN_ROT:
        ScanStr (params, "%F", list, &num);
        if (num == 3)
        {
          M *= csXRotMatrix3 (list[0]);
          M *= csZRotMatrix3 (list[2]);
          M *= csYRotMatrix3 (list[1]);
        }
        else
	  CsPrintf (MSG_WARNING, "Badly formed rotation: '%s'\n", params);
        break;
      case TOKEN_SCALE_X:
        ScanStr (params, "%f", &scaler);
        M *= csXScaleMatrix3(scaler);
        break;
      case TOKEN_SCALE_Y:
        ScanStr (params, "%f", &scaler);
        M *= csYScaleMatrix3(scaler);
        break;
      case TOKEN_SCALE_Z:
        ScanStr (params, "%f", &scaler);
        M *= csZScaleMatrix3(scaler);
        break;
      case TOKEN_SCALE:
        ScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	  M *= list[0];
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	  M *= csMatrix3 (list[0],0,0,0,list[1],0,0,0,list[2]);
        else
	  CsPrintf (MSG_WARNING, "Badly formed scale: '%s'\n", params);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    // Neither SCALE, ROT, nor IDENTITY, so matrix may contain a single scaler
    // or the nine values of a 3x3 matrix.
    ScanStr (buf, "%F", list, &num);
    if (num == 1)
      M = csMatrix3 () * list[0];
    else if (num == 9)
      M = csMatrix3 (
        list[0], list[1], list[2],
        list[3], list[4], list[5],
        list[6], list[7], list[8]);
    else
      CsPrintf (MSG_WARNING, "Badly formed matrix '%s'\n", buf);
  }
  return M;
}

csVector3 csLoader::load_vector (char* buf)
{
  float x,y,z;
  ScanStr (buf, "%f,%f,%f", &x, &y, &z);
  return csVector3(x,y,z);
}

//---------------------------------------------------------------------------

csPolyTxtPlane* csLoader::load_polyplane (char* buf, char* name)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;
  CHK( csPolyTxtPlane* ppl = new csPolyTxtPlane() );
  ppl->SetName (name);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = 0, tx2_len = 0;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_ORIG:
        tx1_given = true;
        tx1_orig = load_vector (params);
        break;
      case TOKEN_FIRST:
        tx1_given = true;
        tx1 = load_vector (params);
        break;
      case TOKEN_FIRST_LEN:
        ScanStr (params, "%f", &tx1_len);
        tx1_given = true;
        break;
      case TOKEN_SECOND:
        tx2_given = true;
        tx2 = load_vector (params);
        break;
      case TOKEN_SECOND_LEN:
        ScanStr (params, "%f", &tx2_len);
        tx2_given = true;
        break;
      case TOKEN_MATRIX:
        tx_matrix = load_matrix (params);
        break;
      case TOKEN_V:
        tx_vector = load_vector (params);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a plane!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (tx1_given)
    if (tx2_given)
      ppl->SetTextureSpace (tx1_orig, tx1, tx1_len, tx2, tx2_len);
    else
    {
      CsPrintf (MSG_FATAL_ERROR, "Not supported!\n");
      fatal_exit (0, true);
    }
  else
    ppl->SetTextureSpace (tx_matrix, tx_vector);

  return ppl;
}

//---------------------------------------------------------------------------

void csLoader::load_light (char* name, char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (ACTIVE)
    TOKEN_TABLE (STATELESS)
    TOKEN_TABLE (PRIMARY_ACTIVE)
    TOKEN_TABLE (SECONDARY_ACTIVE)
    TOKEN_TABLE (BECOMING_ACTIVE)
    TOKEN_TABLE (PRIMARY_INACTIVE)
    TOKEN_TABLE (SECONDARY_INACTIVE)
    TOKEN_TABLE (BECOMING_INACTIVE)
  TOKEN_TABLE_END

  CHK (CLights *theLite = new CLights());
  theLite->SetName (name);

  long cmd;
  char *params;
  int state, theType, thePeriod, dp, intensity, di;
  while ((cmd = csGetCommand(&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_ACTIVE:
        sscanf(params, "%d", &state);
        theLite->SetInitallyActive(state);
        break;
      case TOKEN_STATELESS:
        sscanf(params, "%d", &state);
        theLite->SetStateType(state);
        break;
      case TOKEN_PRIMARY_ACTIVE:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStatePrimaryActive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case TOKEN_SECONDARY_ACTIVE:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateSecondaryActive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case TOKEN_BECOMING_ACTIVE:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateBecomingActive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case TOKEN_PRIMARY_INACTIVE:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStatePrimaryInactive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case TOKEN_SECONDARY_INACTIVE:
        sscanf(params, "%d,%d,%d,%d,%d", &theType, &thePeriod,
                                         &dp, &intensity, &di);
        theLite->SetFunctionData(CLights::kStateSecondaryInactive, theType,
                                 thePeriod, dp, intensity, di);
        break;
      case TOKEN_BECOMING_INACTIVE:
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

csCollection* csLoader::load_collection (char* name, csWorld* w, char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (THING)
    TOKEN_TABLE (COLLECTION)
    TOKEN_TABLE (LIGHT)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (SECTOR)
  TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;

  CHK( csCollection* collection = new csCollection() );
  collection->SetName (name);

  char str[255];
  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_THING:
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
      case TOKEN_LIGHT:
        {
          int nr;
          ScanStr (params, "%s,%d", str, &nr);
          csSector* s = (csSector*)w->sectors.FindByName (str);
          if (!s)
          {
            CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          csStatLight *l = (csStatLight*)s->lights[nr];
          collection->AddObject ((csObject*)l);
        }
        break;
      case TOKEN_SECTOR:
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
      case TOKEN_COLLECTION:
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
        break;
      case TOKEN_TRIGGER:
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
            csScript* s = csScriptList::GetScript(str3);
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

csStatLight* csLoader::load_statlight (char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (ATTENUATION)
    TOKEN_TABLE (CENTER)
    TOKEN_TABLE (RADIUS)
    TOKEN_TABLE (DYNAMIC)
    TOKEN_TABLE (COLOR)
    TOKEN_TABLE (HALO)
  TOKEN_TABLE_END

  long cmd;
  char* params;

  csLoaderStat::lights_loaded++;
  float x, y, z, dist = 0, r, g, b;
  int dyn, attenuation = CS_ATTN_LINEAR;
  bool halo = false;
  float haloIntensity = 0.0;
  float haloCross     = 0.0;

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
        case TOKEN_RADIUS:
          ScanStr (params, "%f", &dist);
          break;
        case TOKEN_CENTER:
          ScanStr (params, "%f,%f,%f", &x, &y, &z);
          break;
        case TOKEN_COLOR:
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
          break;
        case TOKEN_DYNAMIC:
          dyn = 1;
          break;
        case TOKEN_HALO:
          halo = true;
          haloIntensity = 0.5; haloCross = 2.0;
          ScanStr (params, "%f,%f", &haloIntensity, &haloCross);
          break;
        case TOKEN_ATTENUATION:
          char str [100];
          ScanStr (params, "%s", str);
          if (strcmp (str, "none")      == 0) attenuation = CS_ATTN_NONE;
          if (strcmp (str, "linear")    == 0) attenuation = CS_ATTN_LINEAR;
          if (strcmp (str, "inverse")   == 0) attenuation = CS_ATTN_INVERSE;
          if (strcmp (str, "realistic") == 0) attenuation = CS_ATTN_REALISTIC;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a light!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  // implicit radius
  if (dist == 0)
  {
    if (r > g && r > b) dist = r;
    else if (g > b) dist = g;
    else dist = b;
    switch (attenuation)
    {
      case CS_ATTN_NONE      : dist = 100000000; break;
      case CS_ATTN_LINEAR    : break;
      case CS_ATTN_INVERSE   : dist = 16 * sqrt (dist); break;
      case CS_ATTN_REALISTIC : dist = 256 * dist; break;
    }
  }

  CHK (csStatLight* l = new csStatLight (x, y, z, dist, r, g, b, dyn));
  if (halo)
  {
    l->SetFlags (CS_LIGHT_HALO, CS_LIGHT_HALO);
    l->SetHaloType (haloIntensity, haloCross);
  }
  l -> SetAttenuation (attenuation);
  return l;
}

csKeyValuePair* csLoader::load_key (char* buf, csObject* pParent)
{
  char Key  [256];
  char Value[10000]; //Value can potentially grow _very_ large.
  if (ScanStr(buf, "%S,%S", Key, Value) == 2)
  {
    CHK (csKeyValuePair* kvp = new csKeyValuePair(Key, Value));
    if (pParent)
    {
      pParent->ObjAdd(kvp);
    }
    return kvp;
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "Illegal Syntax for KEY() command in line %d", parser_line);
    fatal_exit (0, false);
    return NULL;
  }
}

csMapNode* csLoader::load_node (char* name, char* buf, csSector* sec)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (KEY)
    TOKEN_TABLE (POSITION)
  TOKEN_TABLE_END

  CHK( csMapNode* pNode = new csMapNode(name));
  pNode->SetSector(sec);

  long  cmd;
  char* xname;
  char* params;

  float x     = 0;
  float y     = 0;
  float z     = 0;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_KEY:
        load_key(params, pNode);
        break;
      case TOKEN_POSITION:
        ScanStr (params, "%f,%f,%f", &x, &y, &z);
        break;
      default:
        abort ();
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a thing!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  pNode->SetPosition(csVector3(x,y,z));

  return pNode;
}

//---------------------------------------------------------------------------

csPolygonSet& csLoader::ps_process (csPolygonSet& ps, PSLoadInfo& info, int cmd,
                                  char* name, char* params)
{
  char str[255], str2[255];
  switch (cmd)
  {
    case TOKEN_VERTEX:
      {
        float x, y, z;
        ScanStr (params, "%f,%f,%f", &x, &y, &z);
        ps.AddVertex (x, y, z);
      }
      break;
    case TOKEN_CIRCLE:
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
    case TOKEN_FOG:
      {
        csFog& f = ps.GetFog ();
        f.enabled = true;
        ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
      }
      break;
    case TOKEN_POLYGON:
      {
	csPolygon3D* poly3d = load_poly3d(name, info.w, params,
			info.textures, info.default_texture, info.default_texlen,
			info.default_lightx, ps.GetSector (), &ps);
	if (poly3d)
	{
	  ps.AddPolygon (poly3d);
	  csLoaderStat::polygons_loaded++;
	}
      }
      break;

    case TOKEN_BEZIER:
      //CsPrintf(MSG_WARNING,"Encountered curve!\n");
      ps.AddCurve ( load_bezier(name, info.w, params,
                      info.textures, info.default_texture, info.default_texlen,
                      info.default_lightx, ps.GetSector (), &ps) );
      csLoaderStat::curves_loaded++;
      break;

    case TOKEN_TEXNR:
      ScanStr (params, "%s", str);
      info.default_texture = info.textures->GetTextureMM (str);
      if (info.default_texture == NULL)
      {
        CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
        fatal_exit (0, true);
      }
      break;
    case TOKEN_TEXLEN:
      ScanStr (params, "%f", &info.default_texlen);
      break;
    case TOKEN_TEX_SET_SELECT:
      ScanStr(params, "%s", str);
      info.SetTextureSet( str );
      info.use_tex_set=true;
      break;
    case TOKEN_LIGHTX:
      ScanStr (params, "%s", str);
      info.default_lightx = CLights::FindByName (str);
      break;
    case TOKEN_ACTIVATE:
      ScanStr (params, "%s", str);
      {
        csScript* s = csScriptList::GetScript(str);
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
    case TOKEN_TRIGGER:
      ScanStr (params, "%s,%s", str, str2);
      if (!strcmp (str, "activate"))
      {
        csScript* s = csScriptList::GetScript(str2);
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
                  str, ps.GetName ());
        fatal_exit (0, false);
      }
      break;
    case TOKEN_BSP:
      CsPrintf (MSG_FATAL_ERROR,
        "BSP keyword is no longer supported. Use STATBSP instead after putting\n\
all non-convex polygons in things.\n");
      break;
  }
  return ps;
}

//---------------------------------------------------------------------------

csThing* csLoader::load_sixface (char* name, csWorld* /*w*/, char* buf,
  csTextureList* textures, csSector* sec)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MOVEABLE)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (TEXTURE_SCALE)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (CEIL_TEXTURE)
    TOKEN_TABLE (DIM)
    TOKEN_TABLE (HEIGHT)
    TOKEN_TABLE (FLOOR_HEIGHT)
    TOKEN_TABLE (FLOOR_CEIL)
    TOKEN_TABLE (FLOOR_TEXTURE)
    TOKEN_TABLE (FLOOR)
    TOKEN_TABLE (CEILING)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (ACTIVATE)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (CONVEX)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* xname;

  CHK (csThing* thing = new csThing ());
  thing->SetName (name);

  csLoaderStat::things_loaded++;

  thing->SetSector (sec);
  csReversibleTransform obj;
  csTextureHandle* texture = NULL;
  bool is_convex = false;
  float tscale = 1;
  int i;

  csVector3 v[8];
  for (i = 0;  i < 8;  i++)
    v [i] = csVector3 ((i & 1 ? 1 : -1), (i & 2 ? -1 : 1), (i & 4 ? -1 : 1));
  float r;

  char str[255];
  char str2[255];
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }

    switch (cmd)
    {
      case TOKEN_CONVEX:
        is_convex = true;
        break;
      case TOKEN_FOG:
        {
          csFog& f = thing->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case TOKEN_MOVEABLE:
        thing->SetFlags (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
        break;
      case TOKEN_MOVE:
        {
          char* params2;
          obj = csReversibleTransform(); // identity transform
          while ((cmd = csGetObject (&params, tok_matvec, &xname, &params2)) > 0)
          {
    	    if (!params2)
    	    {
      	      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	      fatal_exit (0, false);
    	    }
            switch (cmd)
            {
              case TOKEN_MATRIX:
                obj.SetT2O (load_matrix (params2));
                break;
              case TOKEN_V:
                obj.SetOrigin (load_vector (params2));
                break;
            }
          }
        }
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        texture = textures->GetTextureMM (str);
        if (texture == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_TEXTURE_SCALE:
        ScanStr (params, "%f", &tscale);
        break;
      case TOKEN_DIM:
        {
          float rx, ry, rz;
          ScanStr (params, "%f,%f,%f", &rx, &ry, &rz);
          rx /= 2; ry /= 2; rz /= 2;
          for (i = 0;  i < 8;  i++)
           v[i] = csVector3((i&1 ? rx : -rx),(i&2 ? -ry : ry),(i&4 ? -rz : rz));
        }
        break;
      case TOKEN_FLOOR_HEIGHT:
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
      case TOKEN_HEIGHT:
        ScanStr (params, "%f", &r);
        v[0].y = r+v[2].y;
        v[1].y = r+v[3].y;
        v[4].y = r+v[6].y;
        v[5].y = r+v[7].y;
        break;
      case TOKEN_FLOOR_CEIL:
        ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
                 &v[2].x, &v[2].z, &v[3].x, &v[3].z,
                 &v[7].x, &v[7].z, &v[6].x, &v[6].z);
        v[0] = v[2];
        v[1] = v[3];
        v[5] = v[7];
        v[4] = v[6];
        break;
      case TOKEN_FLOOR:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v[2].x, &v[2].y, &v[2].z, &v[3].x, &v[3].y, &v[3].z,
                 &v[7].x, &v[7].y, &v[7].z, &v[6].x, &v[6].y, &v[6].z);
        break;
      case TOKEN_CEILING:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v[0].x, &v[0].y, &v[0].z, &v[1].x, &v[1].y, &v[1].z,
                 &v[5].x, &v[5].y, &v[5].z, &v[4].x, &v[4].y, &v[4].z);
        break;
      case TOKEN_ACTIVATE:
        ScanStr (params, "%s", str);
        {
          csScript* s = csScriptList::GetScript(str);
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
      case TOKEN_TRIGGER:
        ScanStr (params, "%s,%s", str, str2);
        if (!strcmp (str, "activate"))
        {
          csScript* s = csScriptList::GetScript(str2);
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
    p->SetName (todo[done].poly);
    p->AddVertex (todo[done].v4);
    p->AddVertex (todo[done].v3);
    p->AddVertex (todo[done].v2);
    p->AddVertex (todo[done].v1);
    p->SetTextureSpace (thing->Vobj (todo[done].tv2),
                          thing->Vobj (todo[done].tv1), tscale);
    done++;
  }

  if (is_convex || thing->GetFog ().enabled)
    thing->SetFlags (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX);
  thing->SetTransform (obj);
  thing->Transform ();
  thing->CompressVertices ();

  return thing;
}

csThing* csLoader::load_thing (char* name, csWorld* w, char* buf,
  csTextureList* textures, csSector* sec)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTEX)
    TOKEN_TABLE (CIRCLE)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (BEZIER)
    TOKEN_TABLE (TEX_SET_SELECT)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (TEXLEN)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (ACTIVATE)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (BSP)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (MOVEABLE)
    TOKEN_TABLE (CONVEX)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (TEMPLATE)
    TOKEN_TABLE (KEY)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* xname;

  CHK( csThing* thing = new csThing() );
  thing->SetName (name);

  csLoaderStat::things_loaded++;
  PSLoadInfo info(w, textures);
  thing->SetSector (sec);

  csReversibleTransform obj;
  long cmd;
  char* params;
  char str[255];
  bool is_convex = false;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_MOVEABLE:
        thing->SetFlags (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
        break;
      case TOKEN_CONVEX:
        is_convex = true;
        break;
      case TOKEN_MOVE:
        {
          char* params2;
          while ((cmd=csGetObject(&params, tok_matvec, &xname, &params2)) > 0)
          {
    	    if (!params2)
    	    {
      	      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	      fatal_exit (0, false);
    	    }
            switch (cmd)
            {
              case TOKEN_MATRIX:
                obj.SetT2O (load_matrix (params2));
                break;
              case TOKEN_V:
                obj.SetOrigin (load_vector (params2));
                break;
            }
          }
        }
        break;
      case TOKEN_TEMPLATE:
        {
          ScanStr (params, "%s", str);
          csThingTemplate* t = w->GetThingTemplate (str);
          if (!t)
          {
            CsPrintf (MSG_FATAL_ERROR, "Couldn't find thing template '%s'!\n", str);
            fatal_exit (0, false);
          }
	  if ( info.use_tex_set ){
              thing->MergeTemplate (t, w->GetTextures(), info.tex_set_name, info.default_texture, info.default_texlen, info.default_lightx);
	      info.use_tex_set = false;
	  }else
             thing->MergeTemplate (t, info.default_texture, info.default_texlen,  info.default_lightx);
          csLoaderStat::polygons_loaded += t->GetNumPolygon ();
        }
        break;
      case TOKEN_KEY:
        load_key(params, thing);
        break;
      default:
        ps_process (*thing, info, cmd, xname, params);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a thing!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  thing->SetTransform (obj);
  thing->Transform ();
  thing->CompressVertices ();
  if (is_convex || thing->GetFog ().enabled) thing->SetFlags (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX);

  return thing;
}



//---------------------------------------------------------------------------

csPolygon3D* csLoader::load_poly3d (char* polyname, csWorld* w, char* buf,
  csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
  CLights* default_lightx, csSector* sec, csPolygonSet* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (MIPMAP)
    TOKEN_TABLE (PORTAL)
    TOKEN_TABLE (WARP)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (UVA)
    TOKEN_TABLE (UV)
    TOKEN_TABLE (COLORS)
    TOKEN_TABLE (FLATCOL)
    TOKEN_TABLE (ALPHA)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (COSFACT)
    TOKEN_TABLE (GOURAUD)
    TOKEN_TABLE (CLIP)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tex_commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (LEN)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (PLANE)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (portal_commands)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
    TOKEN_TABLE (W)
    TOKEN_TABLE (MIRROR)
    TOKEN_TABLE (STATIC)
  TOKEN_TABLE_END

  char* name;
  int i;
  long cmd;
  char* params, * params2;

  CHK(csPolygon3D *poly3d = new csPolygon3D (default_texture));
  poly3d->SetName (polyname);

  csTextureHandle* tex = NULL;
  poly3d->SetSector (sec);
  poly3d->SetParent (parent);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char plane_name[30];
  plane_name[0] = 0;
  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  bool do_mirror = false;
  csLightMapped* pol_lm = poly3d->GetLightMapInfo ();
  if (pol_lm) pol_lm->SetUniformDynLight (default_lightx);

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_TEXNR:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        poly3d->SetTexture (tex);
        break;
      case TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          poly3d->SetFlags (CS_POLY_LIGHTING, do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case TOKEN_MIPMAP:
        {
          int do_mipmap;
          ScanStr (params, "%b", &do_mipmap);
          poly3d->SetFlags (CS_POLY_MIPMAP, do_mipmap ? CS_POLY_MIPMAP : 0);
        }
        break;
      case TOKEN_COSFACT:
        {
          float cosfact;
          ScanStr (params, "%f", &cosfact);
          poly3d->SetCosinusFactor (cosfact);
        }
        break;
      case TOKEN_ALPHA:
        {
          int alpha;
          ScanStr (params, "%d", &alpha);
          poly3d->SetAlpha (alpha);
        }
        break;
      case TOKEN_FOG:
        {
          //@@@ OBSOLETE
        }
        break;
      case TOKEN_PORTAL:
        {
          ScanStr (params, "%s", str);
          CHK(csSector *s = new csSector());
          s->SetName (str);
          poly3d->SetCSPortal (s);
          csLoaderStat::portals_loaded++;
        }
        break;
      case TOKEN_CLIP:
	if (poly3d->GetPortal ()) poly3d->GetPortal ()->SetClippingPortal (true);
        break;
      case TOKEN_WARP:
        if (poly3d->GetPortal ())
        {
          csMatrix3 m_w; m_w.Identity ();
          csVector3 v_w_before (0, 0, 0);
          csVector3 v_w_after (0, 0, 0);
          bool do_static = false;
          while ((cmd = csGetObject (&params, portal_commands, &name, &params2)) > 0)
          {
    	    if (!params2)
    	    {
      	      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	      fatal_exit (0, false);
    	    }
            switch (cmd)
            {
              case TOKEN_MATRIX:
                m_w = load_matrix (params2);
                do_mirror = false;
                break;
              case TOKEN_V:
                v_w_before = load_vector (params2);
                v_w_after = v_w_before;
                do_mirror = false;
                break;
              case TOKEN_W:
                v_w_after = load_vector (params2);
                do_mirror = false;
                break;
              case TOKEN_MIRROR:
                do_mirror = true;
                break;
              case TOKEN_STATIC:
                do_static = true;
                break;
            }
          }
          if (!do_mirror)
            poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);
          poly3d->GetPortal ()->SetStaticDest (do_static);
        }
        break;
      case TOKEN_LIGHTX:
        ScanStr (params, "%s", str);
  	pol_lm = poly3d->GetLightMapInfo ();
        if (pol_lm) pol_lm->SetUniformDynLight (CLights::FindByName (str));
        break;
      case TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
    	  if (!params2)
    	  {
      	    CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	    fatal_exit (0, false);
	  }
          switch (cmd)
          {
            case TOKEN_ORIG:
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1_orig = parent->Vobj ((int)flist[0]);
              if (num == 3) tx1_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST:
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = parent->Vobj ((int)flist[0]);
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST_LEN:
              ScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case TOKEN_SECOND:
              tx2_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = parent->Vobj ((int)flist[0]);
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_SECOND_LEN:
              ScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case TOKEN_LEN:
              ScanStr (params2, "%f", &tx_len);
              break;
            case TOKEN_MATRIX:
              tx_matrix = load_matrix (params2);
              tx_len = 0;
              break;
            case TOKEN_V:
              tx_vector = load_vector (params2);
              tx_len = 0;
              break;
            case TOKEN_PLANE:
              ScanStr (params2, "%s", str);
              strcpy (plane_name, str);
              tx_len = 0;
              break;
            case TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
          }
        }
        break;
      case TOKEN_VERTICES:
        {
          int list[100], num;
          ScanStr (params, "%D", list, &num);
          for (i = 0 ; i < num ; i++)
	  {
	    if (list[i] == list[(i-1+num)%num])
	      CsPrintf (MSG_WARNING, "Duplicate vertex-index found in polygon! Ignored...\n");
	    else
	      poly3d->AddVertex (list[i]);
	  }
        }
        break;
      case TOKEN_FLATCOL:
        {
          float r, g, b;
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
          poly3d->SetFlatColor (r, g, b);
        }
        break;
      case TOKEN_GOURAUD:
        poly3d->SetTextureType (POLYTXT_GOURAUD);
	poly3d->GetGouraudInfo ()->Setup (poly3d->GetVertices ().GetNumVertices ());
	poly3d->GetGouraudInfo ()->EnableGouraud (true);
        break;
      case TOKEN_UV:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  csGouraudShaded* gs = poly3d->GetGouraudInfo ();
          int num, nv = poly3d->GetVertices ().GetNumVertices ();
	  gs->Setup (nv);
          float list [2 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
            gs->SetUV (j, list [j * 2], list [j * 2 + 1]);
        }
        break;
      case TOKEN_COLORS:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  csGouraudShaded* gs = poly3d->GetGouraudInfo ();
          int num, nv = poly3d->GetVertices ().GetNumVertices ();
	  gs->Setup (nv);
          float list [3 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
            gs->SetColor (j, list [j * 3], list [j * 3 + 1], list [j * 3 + 2]);
        }
        break;
      case TOKEN_UVA:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  csGouraudShaded* gs = poly3d->GetGouraudInfo ();
          int num, nv = poly3d->GetVertices ().GetNumVertices ();
	  gs->Setup (nv);
          float list [3 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
          {
            float a = list [j * 3] * 2 * M_PI / 360.;
            gs->SetUV (j, cos (a) * list [j * 3 + 1] + list [j * 3 + 2],
                          sin (a) * list [j * 3 + 1] + list [j * 3 + 2]);
          }
        }
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a polygon!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (poly3d->GetNumVertices() == 0)
  {
    CsPrintf (MSG_WARNING, "Polygon in line %d contains no vertices!\n", parser_line);
    return NULL;
  }

  if (tx1_given)
    if (tx2_given)
      poly3d->SetTextureSpace (tx1_orig.x, tx1_orig.y, tx1_orig.z,
                               tx1.x, tx1.y, tx1.z, tx1_len,
                               tx2.x, tx2.y, tx2.z, tx2_len);
  else
    poly3d->SetTextureSpace (tx1_orig.x, tx1_orig.y, tx1_orig.z,
                             tx1.x, tx1.y, tx1.z, tx1_len);
  else if (plane_name[0])
    poly3d->SetTextureSpace ((csPolyTxtPlane*)w->planes.FindByName (plane_name));
  else if (tx_len)
  {
    // If a length is given (with 'LEN') we will take the first two vertices
    // and calculate the texture orientation from them (with the given
    // length).
    poly3d->SetTextureSpace (poly3d->Vobj(0), poly3d->Vobj(1), tx_len);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (uv_shift_given)
  {
    poly3d->GetLightMapInfo ()->GetTxtPlane ()->
    	GetTextureSpace (tx_matrix, tx_vector);
    // T = Mot * (O - Vot)
    // T = Mot * (O - Vot) + Vuv      ; Add shift Vuv to final texture map
    // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
    // T = Mot * (O - Vot + Mot-1 * Vuv)
    csVector3 shift (u_shift, v_shift, 0);
    tx_vector -= tx_matrix.GetInverse () * shift;
    poly3d->SetTextureSpace (tx_matrix, tx_vector);
  }

  if (do_mirror)
    poly3d->GetPortal ()->SetWarp (csTransform::GetReflect (
    	*(poly3d->GetPolyPlane ()) ));

  return poly3d;
}


csCurve* csLoader::load_bezier (char* polyname, csWorld* w, char* buf,
  csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
  CLights* default_lightx, csSector* sec, csPolygonSet* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (VERTICES)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tex_commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (LEN)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (PLANE)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params, * params2;

  (void)w; (void)default_lightx; (void)sec; (void)parent;

  CHK (csBezier *poly3d = new csBezier (NULL));
  poly3d->SetName (polyname);
  poly3d->SetTextureHandle (default_texture);
  csTextureHandle* tex = NULL;
//TODO??  poly3d->SetSector(sec);
//TODO??  poly3d->SetParent (parent);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char plane_name[30];
  plane_name[0] = 0;
  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_TEXNR:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        poly3d->SetTextureHandle (tex);
        break;
      case TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
          if (!params2)
          {
            CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
            fatal_exit (0, false);
          }
          switch (cmd)
          {
            case TOKEN_ORIG:
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1_orig = parent->Vobj ((int)flist[0]);
              if (num == 3) tx1_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST:
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = parent->Vobj ((int)flist[0]);
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST_LEN:
              ScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case TOKEN_SECOND:
              tx2_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = parent->Vobj ((int)flist[0]);
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_SECOND_LEN:
              ScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case TOKEN_LEN:
              ScanStr (params2, "%f", &tx_len);
              break;
            case TOKEN_MATRIX:
              tx_matrix = load_matrix (params2);
              tx_len = 0;
              break;
            case TOKEN_V:
              tx_vector = load_vector (params2);
              tx_len = 0;
              break;
            case TOKEN_PLANE:
              ScanStr (params2, "%s", str);
              strcpy (plane_name, str);
              tx_len = 0;
              break;
            case TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
          }
        }
        break;
      case TOKEN_VERTICES:
        {
          int list[100], num;
          ScanStr (params, "%D", list, &num);

          if (num != 9)
            {
              CsPrintf (MSG_FATAL_ERROR, "Wrong number of vertices to bezier!\n");
              fatal_exit (0, false);
            }

          //TODO          for (i = 0 ; i < num ; i++) poly3d->set_vertex (i, list[i]);
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

csImageFile* csLoader::load_image (const char* name)
{
  size_t size;
  csImageFile *ifile = NULL;
  char *buf = System->VFS->ReadFile (name, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", name);
    return NULL;
  }

  ifile = csImageLoader::load ((UByte *)buf, size);
  CHK (delete [] buf);

  if (!ifile)
  {
    CsPrintf (MSG_WARNING, "'%s': Cannot load image. Unknown format or wrong extension!\n",name);
    return NULL;
  }

  if (ifile->get_status () & IFE_Corrupt)
  {
    CsPrintf (MSG_WARNING, "'%s': %s!\n",name,ifile->get_status_mesg());
    CHK (delete ifile);
    return NULL;
  }

  char *xname = System->VFS->ExpandPath (name);
  ifile->SetName (xname);
  delete [] xname;

  return ifile;
}

void csLoader::txt_process (char *name, char* buf, csTextureList* textures, csWorld* /*world*/, const char* prefix)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TRANSPARENT)
    TOKEN_TABLE (FILTER)
    TOKEN_TABLE (FILE)
  TOKEN_TABLE_END

  long cmd;
  const char *filename = name;
  char *params;
  csColor transp;
  bool do_transp = false;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_TRANSPARENT:
        do_transp = true;
        ScanStr (params, "%f,%f,%f", &transp.red, &transp.green, &transp.blue);
        break;
      case TOKEN_FILTER:
        CsPrintf (MSG_WARNING, "Warning! TEXTURE/FILTER statement is obsolete"
                               " and does not do anything!\n");
        break;
      case TOKEN_FILE:
        filename = params;
        break;
    }
  }

  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a texture specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  csImageFile *image = load_image (filename);
  if (!image)
    return;

  // The size of image should be checked before registering it with
  // the 3D or 2D driver... if the texture is used for 2D only, it can
  // not have power-of-two dimensions...

  csTextureHandle *tex = textures->NewTexture (image);
  if ( prefix ){
    char *prefixedname = new char[ strlen( name ) + strlen( prefix ) + 2 ];
    strcpy( prefixedname, prefix );
    strcat( prefixedname, "_" );
    strcat( prefixedname, name );
    tex->SetName( prefixedname );
  }else
    tex->SetName (name);
  // dereference image pointer since tex already incremented it
  image->DecRef ();

  if (do_transp)
    tex->SetTransparent (QInt (transp.red * 255.),
      QInt (transp.green * 255.), QInt (transp.blue * 255.));
}

//---------------------------------------------------------------------------

csPolygonTemplate* csLoader::load_ptemplate (char* ptname, char* buf,
  csTextureList* textures, csTextureHandle* default_texture,
  float default_texlen, csThingTemplate* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (MIPMAP)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (FLATCOL)
    TOKEN_TABLE (GOURAUD)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tex_commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (LEN)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

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
  csVector3 tx1_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);

  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_TEXNR:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        ptemplate->SetTexture (tex);
        break;
      case TOKEN_GOURAUD:
        ptemplate->SetGouraud ();
        break;
      case TOKEN_FLATCOL:
        {
          float r, g, b;
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
          ptemplate->SetFlatColor (r, g, b);
        }
        break;
      case TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          ptemplate->SetLighting (do_lighting);
        }
        break;
      case TOKEN_MIPMAP:
        {
          int do_mipmap;
          ScanStr (params, "%b", &do_mipmap);
          ptemplate->SetMipmapping (do_mipmap);
        }
        break;
      case TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
          if (!params2)
          {
            CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
            fatal_exit (0, false);
          }
          switch (cmd)
          {
            case TOKEN_ORIG:
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1_orig = parent->Vtex ((int)flist[0]);
              if (num == 3) tx1_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST:
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = parent->Vtex ((int)flist[0]);
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST_LEN:
              ScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case TOKEN_SECOND:
              tx2_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = parent->Vtex ((int)flist[0]);
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_SECOND_LEN:
              ScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case TOKEN_LEN:
              ScanStr (params2, "%f", &tx_len);
              break;
            case TOKEN_MATRIX:
              tx_matrix = load_matrix (params2);
              tx_len = 0;
              break;
            case TOKEN_V:
              tx_vector = load_vector (params2);
              tx_len = 0;
              break;
            case TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
          }
        }
        break;
      case TOKEN_VERTICES:
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
        tx1_orig, tx1, tx1_len, tx2, tx2_len);
    else
    {
      float A, B, C;
      ptemplate->PlaneNormal (&A, &B, &C);
      TextureTrans::compute_texture_space (tx_matrix, tx_vector,
        tx1_orig, tx1, tx1_len, A, B, C);
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
    // T = Mot * (O - Vot) + Vuv                ; Add shift Vuv to final texture map
    // T = Mot * (O - Vot) + Mot * Mot-1 * Vuv
    // T = Mot * (O - Vot + Mot-1 * Vuv)
    csVector3 shift (u_shift, v_shift, 0);
    tx_vector -= tx_matrix.GetInverse () * shift;
  }
  ptemplate->SetTextureSpace (tx_matrix, tx_vector);

  return ptemplate;
}

csCurveTemplate* csLoader::load_beziertemplate (char* ptname, char* buf,
        csTextureList* textures, csTextureHandle* default_texture, float default_texlen,
        csThingTemplate* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (VERTICES)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tex_commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (LEN)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

  char *name;
  long cmd;
  int i;
  char *params, *params2;

  CHK(csBezierTemplate *ptemplate = new csBezierTemplate());
  ptemplate->SetName (ptname);

  ptemplate->SetParent (parent);

  csTextureHandle* tex;
  if (default_texture == NULL) tex = NULL;
  else ptemplate->SetTextureHandle (default_texture);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx1_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = default_texlen, tx2_len = default_texlen;
  float tx_len = default_texlen;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);

  bool uv_shift_given = false;
  float u_shift = 0, v_shift = 0;

  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_TEXNR:
        ScanStr (params, "%s", str);
        tex = textures->GetTextureMM (str);
        if (tex == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        ptemplate->SetTextureHandle (tex);
        break;
      case TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
          if (!params2)
          {
            CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
            fatal_exit (0, false);
          }
          switch (cmd)
          {
            case TOKEN_ORIG:
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1_orig = parent->CurveVertex ((int)flist[0]);
              if (num == 3) tx1_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST:
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = parent->CurveVertex ((int)flist[0]);
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_FIRST_LEN:
              ScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case TOKEN_SECOND:
              tx2_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = parent->CurveVertex ((int)flist[0]);
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case TOKEN_SECOND_LEN:
              ScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case TOKEN_LEN:
              ScanStr (params2, "%f", &tx_len);
              break;
            case TOKEN_MATRIX:
              tx_matrix = load_matrix (params2);
              tx_len = 0;
              break;
            case TOKEN_V:
              tx_vector = load_vector (params2);
              tx_len = 0;
              break;
            case TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
          }
        }
        break;
      case TOKEN_VERTICES:
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

csThingTemplate* csLoader::load_thingtpl (char* tname, char* buf,
                                        csTextureList* textures)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTEX)
    TOKEN_TABLE (CIRCLE)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (BEZIER)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (TEXLEN)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (FILE)
    TOKEN_TABLE (CURVECENTER)
    TOKEN_TABLE (CURVESCALE)
    TOKEN_TABLE (CURVECONTROL)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matrix)
    TOKEN_TABLE (MATRIX)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_vector)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* name;
  char str[255];
  int i;

  CHK( csThingTemplate *tmpl = new csThingTemplate() );
  tmpl->SetName (tname);
  long cmd;
  char* params;
  csTextureHandle* default_texture = NULL;
  float default_texlen = 1.;

  csMatrix3 m_move;
  csVector3 v_move (0, 0, 0);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_VERTEX:
        {
          float x, y, z;
          ScanStr (params, "%f,%f,%f", &x, &y, &z);
          tmpl->AddVertex (x, y, z);
        }
        break;
      case TOKEN_CIRCLE:
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
      case TOKEN_FOG:
        {
          csFog& f = tmpl->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case TOKEN_POLYGON:
        tmpl->AddPolygon ( load_ptemplate(name, params, textures,
                            default_texture, default_texlen, tmpl) );
        break;
      case TOKEN_BEZIER:
        //CsPrintf(MSG_WARNING,"Encountered template curve!\n");
        tmpl->AddCurve (
              load_beziertemplate(name, params, textures,
                        default_texture, default_texlen, tmpl)  );
        break;

      case TOKEN_CURVECENTER:
        {
          csVector3 c;
          ScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
          tmpl->curves_center = c;
        }
        break;
      case TOKEN_CURVESCALE:
        ScanStr (params, "%f", &tmpl->curves_scale);
        break;

      case TOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
          ScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
          tmpl->AddCurveVertex (v,t);
        }
        break;

      case TOKEN_TEXNR:
        ScanStr (params, "%s", str);
        default_texture = textures->GetTextureMM (str);
        if (default_texture == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;

      case TOKEN_TEXLEN:
        ScanStr (params, "%f", &default_texlen);
        break;

      case TOKEN_MOVE:
        {
          char* params2;
          csGetObject (&params, tok_matrix, &name, &params2);
          m_move = load_matrix(params2);
          csGetObject (&params, tok_vector, &name, &params2);
          v_move = load_vector(params2);
        }
        break;

      case TOKEN_FILE:
        {
          ScanStr (params, "%s", str);
	  CHK (converter* filedata = new converter);
	  if (filedata->ivcon (str, true, false, NULL, System->VFS) == ERROR)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Error loading file model '%s'!\n", str);
	    CHK (delete filedata);
	    fatal_exit (0, false);
	  }
	  csCrossBuild_ThingTemplateFactory builder;
	  builder.CrossBuild (tmpl, *filedata);
	  CHK (delete filedata);
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

csThingTemplate* csLoader::load_sixtpl(char* tname,char* buf,csTextureList* textures)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (TEXTURE_SCALE)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (CEIL_TEXTURE)
    TOKEN_TABLE (DIM)
    TOKEN_TABLE (HEIGHT)
    TOKEN_TABLE (FLOOR_HEIGHT)
    TOKEN_TABLE (FLOOR_CEIL)
    TOKEN_TABLE (FLOOR_TEXTURE)
    TOKEN_TABLE (FLOOR)
    TOKEN_TABLE (CEILING)
    TOKEN_TABLE (FOG)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matrix)
    TOKEN_TABLE (MATRIX)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_vector)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* name;
  int i;

  CHK (csThingTemplate* tmpl = new csThingTemplate ());
  tmpl->SetName (tname);

  csTextureHandle* texture = NULL;
  float tscale = 1;

  csVector3 v0 (-1,  1,  1);
  csVector3 v1 ( 1,  1,  1);
  csVector3 v2 (-1, -1,  1);
  csVector3 v3 ( 1, -1,  1);
  csVector3 v4 (-1,  1, -1);
  csVector3 v5 ( 1,  1, -1);
  csVector3 v6 (-1, -1, -1);
  csVector3 v7 ( 1, -1, -1);
  float r;

  char str[255];
  long cmd;
  char* params;

  csMatrix3 m_move;
  csVector3 v_move (0, 0, 0);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_FOG:
        {
          csFog& f = tmpl->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case TOKEN_MOVE:
        {
          char* params2;
          csGetObject (&params, tok_matrix, &name, &params2);
          m_move = load_matrix (params2);
          csGetObject (&params, tok_vector, &name, &params2);
          v_move = load_vector (params2);
        }
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        texture = textures->GetTextureMM (str);
        if (texture == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_TEXTURE_SCALE:
        ScanStr (params, "%f", &tscale);
        break;
      case TOKEN_DIM:
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
      case TOKEN_FLOOR_HEIGHT:
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
      case TOKEN_HEIGHT:
        ScanStr (params, "%f", &r);
        v0.y = r+v2.y;
        v1.y = r+v3.y;
        v4.y = r+v6.y;
        v5.y = r+v7.y;
        break;
      case TOKEN_FLOOR_CEIL:
        ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
                &v2.x, &v2.z, &v3.x, &v3.z, &v7.x, &v7.z, &v6.x, &v6.z);
        v0 = v2;
        v1 = v3;
        v5 = v7;
        v4 = v6;
        break;
      case TOKEN_FLOOR:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v2.x, &v2.y, &v2.z, &v3.x, &v3.y, &v3.z,
                 &v7.x, &v7.y, &v7.z, &v6.x, &v6.y, &v6.z);
        break;
      case TOKEN_CEILING:
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
    csVector3 v_tx (0, 0, 0);
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
  int col_idx;          // Idx in colors table if there was an override.
  CLights* light;       // A dynamic light for this polygon
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

void load_tex (char** buf, Color* colors, int num_colors, csTextureList* textures,
               char* name)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (PLANE)
    TOKEN_TABLE (LEN)
  TOKEN_TABLE_END

  long cmd;
  char *params;
  char str [255];

  strcpy (colors [num_colors].poly, name);
  colors[num_colors].plane[0] = 0;
  colors[num_colors].texture = NULL;
  colors[num_colors].len = 0;

  while ((cmd = csGetCommand (buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        colors[num_colors].texture = textures->GetTextureMM (str);
        if (colors[num_colors].texture == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_PLANE:
        ScanStr (params, "%s", str);
        strcpy (colors[num_colors].plane, str);
        break;
      case TOKEN_LEN:
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

csSector* csLoader::load_room (char* secname, csWorld* w, char* buf,
  csTextureList* textures)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (TEXTURE_LIGHTING)
    TOKEN_TABLE (TEXTURE_MIPMAP)
    TOKEN_TABLE (TEXTURE_SCALE)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (TEX)
    TOKEN_TABLE (CEIL_TEXTURE)
    TOKEN_TABLE (FLOOR_TEXTURE)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (LIGHT)
    TOKEN_TABLE (DIM)
    TOKEN_TABLE (HEIGHT)
    TOKEN_TABLE (FLOOR_HEIGHT)
    TOKEN_TABLE (FLOOR_CEIL)
    TOKEN_TABLE (FLOOR)
    TOKEN_TABLE (CEILING)
    TOKEN_TABLE (SIXFACE)
    TOKEN_TABLE (THING)
    TOKEN_TABLE (PORTAL)
    TOKEN_TABLE (SPLIT)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (ACTIVATE)
    TOKEN_TABLE (BSP)
    TOKEN_TABLE (STATBSP)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (FOG)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (portal_commands)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (SECTOR)
    TOKEN_TABLE (ALPHA)
    TOKEN_TABLE (WARP)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (mCommands)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
    TOKEN_TABLE (W)
    TOKEN_TABLE (MIRROR)
    TOKEN_TABLE (STATIC)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matrix)
    TOKEN_TABLE (MATRIX)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_vector)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params, * params2;
  int i, l;
  int i1, i2, i3, i4;
  bool do_stat_bsp = false;

  CHK(csSector* sector = new csSector());
  sector->SetName (secname);

  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  csLoaderStat::sectors_loaded++;

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

  csVector3 v0 (-1,  1,  1);
  csVector3 v1 ( 1,  1,  1);
  csVector3 v2 (-1, -1,  1);
  csVector3 v3 ( 1, -1,  1);
  csVector3 v4 (-1,  1, -1);
  csVector3 v5 ( 1,  1, -1);
  csVector3 v6 (-1, -1, -1);
  csVector3 v7 ( 1, -1, -1);
  float r;

  char str[255];
  char str2[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_BSP:
        CsPrintf (MSG_FATAL_ERROR,
          "BSP keyword is no longer supported. Use STATBSP instead after putting\n\
all non-convex polygons in things.\n");
        break;
      case TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case TOKEN_MOVE:
        {
          char* params2;
          csGetObject (&params, tok_matrix, &name, &params2);
          mm = load_matrix (params2);
          csGetObject (&params, tok_vector, &name, &params2);
          vm = load_vector (params2);
        }
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        texture = textures->GetTextureMM (str);
        if (texture == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_TEXTURE_LIGHTING:
        ScanStr (params, "%b", &no_lighting); no_lighting = !no_lighting;
        break;
      case TOKEN_TEXTURE_MIPMAP:
        ScanStr (params, "%b", &no_mipmap); no_mipmap = !no_mipmap;
        break;
      case TOKEN_CEIL_TEXTURE:
      case TOKEN_FLOOR_TEXTURE:
        ScanStr (params, "%s", str);
        colors[num_colors].texture = textures->GetTextureMM (str);
        if (colors[num_colors].texture == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find texture named '%s'!\n", str);
          fatal_exit (0, true);
        }
        strcpy (colors[num_colors].poly,
                cmd == TOKEN_CEIL_TEXTURE ? "up" : "down");
        colors[num_colors].plane[0] = 0;
        if (num_colors >= MAX_ROOM_COLORS)
        {
          CsPrintf (MSG_FATAL_ERROR, "OVERFLOW number of colors in room!\n");
          fatal_exit (0, false);
        }
        num_colors++;
        break;
      case TOKEN_LIGHTX:
        ScanStr (params, "%s,%s", str, str2);
        strcpy (dlights[num_light].poly, str);
        strcpy (dlights[num_light].light, str2);
        num_light++;
        break;
      case TOKEN_TEX:
        load_tex (&params, colors, num_colors, textures, name);
        num_colors++;
        break;
      case TOKEN_TEXTURE_SCALE:
        ScanStr (params, "%f", &tscale);
        break;
      case TOKEN_DIM:
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
      case TOKEN_FLOOR_HEIGHT:
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
      case TOKEN_HEIGHT:
        ScanStr (params, "%f", &r);
        v0.y = r+v2.y;
        v1.y = r+v3.y;
        v4.y = r+v6.y;
        v5.y = r+v7.y;
        break;
      case TOKEN_FLOOR_CEIL:
        ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
                 &v2.x, &v2.z, &v3.x, &v3.z, &v7.x, &v7.z, &v6.x, &v6.z);
        v0 = v2;
        v1 = v3;
        v5 = v7;
        v4 = v6;
        break;
      case TOKEN_FLOOR:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v2.x, &v2.y, &v2.z, &v3.x, &v3.y, &v3.z,
                 &v7.x, &v7.y, &v7.z, &v6.x, &v6.y, &v6.z);
        break;
      case TOKEN_CEILING:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v0.x, &v0.y, &v0.z, &v1.x, &v1.y, &v1.z,
                 &v5.x, &v5.y, &v5.z, &v4.x, &v4.y, &v4.z);
        break;
      case TOKEN_LIGHT:
        sector->AddLight ( load_statlight(params) );
        break;
      case TOKEN_SIXFACE:
        sector->AddThing ( load_sixface(name,w,params,textures,sector) );
        break;
      case TOKEN_FOG:
        {
          csFog& f = sector->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case TOKEN_SPRITE:
        {
          CHK (csSprite3D* sp = new csSprite3D ());
          sp->SetName (name);
          LoadSprite (sp, w, params, textures);
          w->sprites.Push (sp);
          sp->MoveToSector (sector);
        }
        break;
      case TOKEN_THING:
        sector->AddThing ( load_thing(name,w,params,textures,sector) );
        break;
      case TOKEN_PORTAL:
        {
          if (num_portals >= MAX_ROOM_PORTALS)
          {
            CsPrintf (MSG_FATAL_ERROR,
                      "OVERFLOW with number of portals in room!\n");
            fatal_exit (0, false);
          }
          portals[num_portals].is_warp = false;
          portals[num_portals].alpha = 0;
          while ((cmd = csGetObject (&params, portal_commands, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
              fatal_exit (0, false);
            }
            switch (cmd)
            {
              case TOKEN_POLYGON:
                ScanStr (params2, "%s", portals[num_portals].poly);
                break;
              case TOKEN_SECTOR:
                ScanStr (params2, "%s", portals[num_portals].sector);
                break;
              case TOKEN_ALPHA:
                ScanStr (params2, "%d", &portals[num_portals].alpha);
                break;
              case TOKEN_WARP:
                {
                  portals[num_portals].do_static = false;
                  char* params3;
                  while ((cmd = csGetObject (&params2, mCommands, &name, &params3)) > 0)
                  {
                    if (!params3)
                    {
                      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params2);
                      fatal_exit (0, false);
                    }
                    switch (cmd)
                    {
                      case TOKEN_MATRIX:
                        portals[num_portals].m_warp
                                          = load_matrix (params3);
                        portals[num_portals].do_mirror = false;
                        break;
                      case TOKEN_V:
                        portals[num_portals].v_warp_before
                                          = load_vector (params3);
                        portals[num_portals].v_warp_after
                                          = portals[num_portals].v_warp_before;
                        portals[num_portals].do_mirror = false;
                        break;
                      case TOKEN_W:
                        portals[num_portals].v_warp_after
                                          = load_vector (params3);
                        portals[num_portals].do_mirror = false;
                        break;
                      case TOKEN_MIRROR:
                        portals[num_portals].do_mirror = true;
                        break;
                      case TOKEN_STATIC:
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
      case TOKEN_SPLIT:
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
      case TOKEN_ACTIVATE:
        ScanStr (params, "%s", str);
        {
          csScript* s = csScriptList::GetScript(str);
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
      case TOKEN_TRIGGER:
        ScanStr (params, "%s,%s", str, str2);
        if (!strcmp (str, "activate"))
        {
          csScript* s = csScriptList::GetScript(str2);
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
                  sector->GetName ());
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
      p->SetName (todo[done].poly);
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
        p->SetTextureSpace ((csPolyTxtPlane*)w->planes.FindByName (colors[idx].plane));
      p->SetFlags (CS_POLY_MIPMAP|CS_POLY_LIGHTING,
        (no_mipmap ? 0 : CS_POLY_MIPMAP) | (no_lighting ? 0 : CS_POLY_LIGHTING));
      csLightMapped* pol_lm = p->GetLightMapInfo ();
      if (pol_lm) pol_lm->SetUniformDynLight (todo[done].light);
    }
    done++;
  }

  csSector* portal;

  for (i = 0 ; i < num_portals ; i++)
  {
    p = sector->GetPolygon3D (portals[i].poly);
    if (!p)
    {
      CsPrintf (MSG_FATAL_ERROR, "Error locating polygon '%s' in room '%s'!\n",
                portals[i].poly, name);
      fatal_exit (0, false);
    }

    // This will later be defined correctly
    CHK( portal = new csSector () );
    portal->SetName (portals[i].sector);
    p->SetCSPortal (portal);
    csLoaderStat::portals_loaded++;
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

  sector->CompressVertices ();
  if (do_stat_bsp) sector->UseStaticTree ();

  return sector;
}

csSector* csLoader::load_sector (char* secname, csWorld* w, char* buf,
  csTextureList* textures)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTEX)
    TOKEN_TABLE (CIRCLE)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (TEXLEN)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (ACTIVATE)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (BSP)
    TOKEN_TABLE (STATBSP)
    TOKEN_TABLE (THING)
    TOKEN_TABLE (SIXFACE)
    TOKEN_TABLE (LIGHT)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (SKYDOME)
    TOKEN_TABLE (TERRAIN)
    TOKEN_TABLE (NODE)
    TOKEN_TABLE (KEY)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  bool do_stat_bsp = false;

  CHK( csSector* sector = new csSector() );
  sector->SetName (secname);

  csLoaderStat::sectors_loaded++;
  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  PSLoadInfo info(w,textures);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_SKYDOME:
        skydome_process (*sector, name, params, info.default_texture);
        break;
      case TOKEN_TERRAIN:
        terrain_process (*sector, name, params, info.default_texture);
        break;
      case TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case TOKEN_THING:
        sector->AddThing ( load_thing(name,w,params,textures,sector) );
        break;
      case TOKEN_SPRITE:
        {
          CHK (csSprite3D* sp = new csSprite3D ());
          sp->SetName (name);
          LoadSprite (sp, w, params, textures);
          w->sprites.Push (sp);
          sp->MoveToSector (sector);
        }
        break;
      case TOKEN_SIXFACE:
        sector->AddThing ( load_sixface(name,w,params,textures,sector) );
        break;
      case TOKEN_LIGHT:
        sector->AddLight ( load_statlight(params) );
        break;
      case TOKEN_NODE:
        sector->ObjAdd ( load_node(name, params, sector) ); 
        break;
      case TOKEN_KEY:
      {
        load_key(params, sector);
        break;
      }
      default:
        ps_process (*sector, info, cmd, name, params);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a sector!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  sector->CompressVertices ();
  if (do_stat_bsp) sector->UseStaticTree ();
  return sector;
}

void csLoader::skydome_process (csSector& sector, char* name, char* buf,
        csTextureHandle* texture)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (RADIUS)
    TOKEN_TABLE (VERTICES)
  TOKEN_TABLE_END

  long cmd;
  char* params;
  float radius = 0.0f;
  int i, j;
  int num = 0;
  csGouraudShaded* gs;

  // Previous vertices.
  int prev_vertices[60];        // @@@ HARDCODED!
  float prev_u[60];
  float prev_v[60];

  char poly_name[30], * end_poly_name;
  strcpy (poly_name, name);
  end_poly_name = strchr (poly_name, 0);

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_RADIUS:
        ScanStr (params, "%f", &radius);
        break;
      case TOKEN_VERTICES:
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
      p->SetName (poly_name);
      p->SetSector (&sector);
      p->SetParent (&sector);
      p->SetFlags (CS_POLY_MIPMAP|CS_POLY_LIGHTING, CS_POLY_LIGHTING);
      p->SetCosinusFactor (1);
      p->AddVertex (prev_vertices[j]);
      p->AddVertex (new_vertices[(j+1)%num]);
      p->AddVertex (new_vertices[j]);
      p->SetTextureType (POLYTXT_GOURAUD);
      gs = p->GetGouraudInfo ();
      gs->Setup (p->GetVertices ().GetNumVertices ());
      gs->EnableGouraud (true);
      gs->SetUV (0, prev_u[j], prev_v[j]);
      gs->SetUV (1, new_u[(j+1)%num], new_v[(j+1)%num]);
      gs->SetUV (2, new_u[j], new_v[j]);

      p->SetTextureSpace (t_m, t_v);
      sector.AddPolygon (p);
      csLoaderStat::polygons_loaded++;
      sprintf (end_poly_name, "%d_%d_B", i, j);
      CHK (p = new csPolygon3D (texture));
      p->SetName (poly_name);
      p->SetSector (&sector);
      p->SetParent (&sector);
      p->SetFlags (CS_POLY_MIPMAP|CS_POLY_LIGHTING, CS_POLY_LIGHTING);
      p->SetCosinusFactor (1);
      p->AddVertex (prev_vertices[j]);
      p->AddVertex (prev_vertices[(j+1)%num]);
      p->AddVertex (new_vertices[(j+1)%num]);
      p->SetTextureType (POLYTXT_GOURAUD);
      gs = p->GetGouraudInfo ();
      gs->Setup (p->GetVertices ().GetNumVertices ());
      gs->EnableGouraud (true);
      gs->SetUV (0, prev_u[j], prev_v[j]);
      gs->SetUV (1, prev_u[(j+1)%num], prev_v[(j+1)%num]);
      gs->SetUV (2, new_u[(j+1)%num], new_v[(j+1)%num]);
      p->SetTextureSpace (t_m, t_v);
      sector.AddPolygon (p);
      csLoaderStat::polygons_loaded++;
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
    p->SetName (poly_name);
    p->SetSector (&sector);
    p->SetParent (&sector);
    p->SetFlags (CS_POLY_MIPMAP|CS_POLY_LIGHTING, CS_POLY_LIGHTING);
    p->SetCosinusFactor (1);
    p->AddVertex (top_vertex);
    p->AddVertex (prev_vertices[j]);
    p->AddVertex (prev_vertices[(j+1)%num]);
    p->SetTextureType (POLYTXT_GOURAUD);
    gs = p->GetGouraudInfo ();
    gs->Setup (p->GetVertices ().GetNumVertices ());
    gs->EnableGouraud (true);
    gs->SetUV (0, top_u, top_v);
    gs->SetUV (1, prev_u[j], prev_v[j]);
    gs->SetUV (2, prev_u[(j+1)%num], prev_v[(j+1)%num]);
    p->SetTextureSpace (t_m, t_v);
    sector.AddPolygon (p);
    csLoaderStat::polygons_loaded++;
  }
}

//---------------------------------------------------------------------------

void csLoader::terrain_process (csSector& sector, char* name, char* buf,
        csTextureHandle* texture)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (HEIGHTMAP)
    TOKEN_TABLE (DETAIL)
  TOKEN_TABLE_END

  long cmd;
  char* params;
  char heightmap[256];	// @@@ Hardcoded.
  int detail;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_HEIGHTMAP:
        ScanStr (params, "%s", heightmap);
        break;
      case TOKEN_DETAIL:
        ScanStr (params, "%d", &detail);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a terrain!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  CHK (csTerrain* terr = new csTerrain ());
  terr->SetName (name);
  terr->SetTexture (texture);

  // Otherwise read file, if that fails generate a random map.
  if (!terr->Initialize (heightmap))
  {
    CsPrintf (MSG_FATAL_ERROR, "Error creating height field\n");
    fatal_exit (0, false);
  }

  sector.terrains.Push (terr);
}

//---------------------------------------------------------------------------

csSoundDataObject* csLoader::load_sound(char* name, const char* filename, csWorld* w)
{
  (void) w;

  csSoundDataObject* sndobj = NULL;
  csSoundData* snd = NULL;

  size_t size;
  char* buf = System->VFS->ReadFile (filename, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_FATAL_ERROR,
      "Cannot read sound file \"%s\" from VFS\n", filename);
    return NULL;
  }

  snd = csSoundLoader::load ((UByte*)buf, size);
  CHK (delete [] buf);
  if (!snd)
  {
    CsPrintf (MSG_FATAL_ERROR, "The sound file \"%s\" is corrupt!\n", filename);
    return NULL;
  }

  CHK (sndobj = new csSoundDataObject (snd));
  sndobj->SetName (name);

  return sndobj;
}

//---------------------------------------------------------------------------

bool csLoader::LoadWorld (csWorld* world, LanguageLayer* layer, char* buf)
{
  TOKEN_TABLE_START (tokens)
    TOKEN_TABLE (WORLD)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (SECTOR)
    TOKEN_TABLE (ROOM)
    TOKEN_TABLE (PLANE)
    TOKEN_TABLE (COLLECTION)
    TOKEN_TABLE (SCRIPT)
    TOKEN_TABLE (TEXTURES)
    TOKEN_TABLE (TEX_SET)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (THING)
    TOKEN_TABLE (SIXFACE)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (LIBRARY)
    TOKEN_TABLE (START)
    TOKEN_TABLE (SOUNDS)
    TOKEN_TABLE (KEY)
  TOKEN_TABLE_END

  parser_line = 1;
  char *name, *data;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    if (!data)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    long cmd;
    char* params;

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", data);
        fatal_exit (0, false);
      }
      switch (cmd)
      {
        case TOKEN_SPRITE:
          {
            csSpriteTemplate* t = world->GetSpriteTemplate (name);
            if (!t)
            {
              CHK (t = new csSpriteTemplate ());
              t->SetName (name);
              world->sprite_templates.Push (t);
            }
            LoadSpriteTemplate (t, params, world->GetTextures ());
          }
          break;
        case TOKEN_THING:
          if (!world->GetThingTemplate (name))
            world->thing_templates.Push ( load_thingtpl(name, params, world->GetTextures ()) );
          break;
        case TOKEN_SIXFACE:
          if (!world->GetThingTemplate (name))
            world->thing_templates.Push ( load_sixtpl(name, params, world->GetTextures ()) );
          break;
        case TOKEN_SECTOR:
          if (!world->sectors.FindByName (name))
            world->sectors.Push( load_sector(name, world, params, world->GetTextures ()) );
          break;
        case TOKEN_PLANE:
          world->planes.Push ( load_polyplane (params, name) );
          break;
        case TOKEN_COLLECTION:
          world->collections.Push ( load_collection(name,world,params) );
          break;
        case TOKEN_SCRIPT:
          csScriptList::NewScript (layer, name, params);
          break;
	case TOKEN_TEX_SET:
          if (!LoadTextures ( world->GetTextures (), params, world, name)) return false;
          break;
        case TOKEN_TEXTURES:
          {
            world->GetTextures ()->Clear ();
            if (!LoadTextures (world->GetTextures (), params, world)) return false;
          }
          break;
        case TOKEN_SOUNDS:
          if (!LoadSounds (world, params)) return false;
          break;
        case TOKEN_ROOM:
          // Not an object but it is translated to a special sector.
          if (!world->sectors.FindByName (name))
            world->sectors.Push (load_room (name, world, params, world->GetTextures ()));
          break;
        case TOKEN_LIGHTX:
          load_light (name, params);
          break;
        case TOKEN_LIBRARY:
          LoadLibraryFile (world, name);
          break;
        case TOKEN_START:
          {
            char str[255];
            ScanStr (params, "%s,%f,%f,%f", str, &world->start_vec.x, &world->start_vec.y, &world->start_vec.z);
            CHK (delete world->start_sector);
            CHK (world->start_sector = new char [strlen (str)+1]);
            strcpy (world->start_sector, str);
          }
          break;
      case TOKEN_KEY:
        load_key(params, world);
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
        csPolygon3D* p = ps->GetPolygon3D (i);
        if (p && p->GetPortal ())
        {
          csPortal* portal = p->GetPortal ();
          csSector *stmp = portal->GetSector ();
          csSector *snew = (csSector*)(world->sectors).FindByName(stmp->GetName ());
          if (!snew)
          {
            CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found for portal in"
                      " polygon '%s/%s'!\n", stmp->GetName (),
                      ((csObject*)p->GetParent ())->GetName (),
                      p->GetName ());
            fatal_exit (0, false);
          }
          portal->SetSector (snew);
          CHK( delete stmp );
        }
      }
  }

  return true;
}

bool csLoader::LoadWorldFile (csWorld* world, LanguageLayer* layer, const char* file)
{
  world->StartWorld ();
  csLoaderStat::Init ();

  size_t size;
  char *buf = System->VFS->ReadFile (file, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open world file \"%s\" on VFS!\n", file);
    return false;
  }

  CHK (csIniFile* cfg = new csIniFile ("world.cfg"));
  if (cfg)
  {
    csPolygon3D::def_mipmap_size = cfg->GetInt ("Lighting", "LIGHTMAP_SIZE",
    	csPolygon3D::def_mipmap_size);
    csPolygon3D::do_lightmap_highqual = cfg->GetInt ("Lighting", "LIGHTMAP_HIGHQUAL",
    	csPolygon3D::do_lightmap_highqual);
    CHK (delete cfg);
  }
  CsPrintf (MSG_INITIALIZATION, "Lightmap grid size = %dx%d%s.\n", csPolygon3D::def_mipmap_size,
      csPolygon3D::def_mipmap_size,
      csPolygon3D::do_lightmap_highqual ? " (high quality)" : " (normal quality)");

  if (!LoadWorld (world, layer, buf)) return false;

  if (csLoaderStat::polygons_loaded)
  {
    CsPrintf (MSG_INITIALIZATION, "Loaded world file:\n");
    CsPrintf (MSG_INITIALIZATION, "  %d polygons (%d portals),\n", csLoaderStat::polygons_loaded,
      csLoaderStat::portals_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d sectors, %d things, %d sprites, \n", csLoaderStat::sectors_loaded,
      csLoaderStat::things_loaded, csLoaderStat::sprites_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d curves and %d lights.\n", csLoaderStat::curves_loaded,
      csLoaderStat::lights_loaded);
  } /* endif */

  CHK (delete [] buf);

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadTextures (csTextureList* textures, char* buf, csWorld* world, const char* prefix)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MAX_TEXTURES)
    TOKEN_TABLE (TEXTURE)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_MAX_TEXTURES:
        // ignored for backward compatibility
        break;
      case TOKEN_TEXTURE:
        txt_process (name, params, textures, world, prefix);
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

bool csLoader::LoadLibrary (csWorld* world, char* buf)
{
  TOKEN_TABLE_START (tokens)
    TOKEN_TABLE (LIBRARY)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXTURES)
    TOKEN_TABLE (THING)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (SOUNDS)
  TOKEN_TABLE_END

  char *name, *data;
  if (csGetObject (&buf, tokens, &name, &data))
  {
    // Empty LIBRARY () directive?
    if (!data)
      return false;

    long cmd;
    char* params;

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", data);
        return false;
      }

      switch (cmd)
      {
        case TOKEN_TEXTURES:
          // Append textures to world.
          if (!LoadTextures (world->GetTextures (), params, world))
            return false;
          break;
        case TOKEN_SOUNDS:
          if (!LoadSounds (world, params))
            return false;
          break;
        case TOKEN_SPRITE:
          {
            csSpriteTemplate* t = world->GetSpriteTemplate (name);
            if (!t)
            {
              CHK (t = new csSpriteTemplate ());
              t->SetName (name);
              world->sprite_templates.Push (t);
            }
            LoadSpriteTemplate (t, params, world->GetTextures ());
          }
          break;
        case TOKEN_THING:
          if (!world->GetThingTemplate (name))
            world->thing_templates.Push (load_thingtpl (name, params, world->GetTextures ()));
          break;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a library file!\n", csGetLastOffender ());
      return false;
    }
  }
  return true;
}

bool csLoader::LoadLibraryFile (csWorld* world, const char* fname)
{
  size_t size;
  char *buf = System->VFS->ReadFile (fname, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open library file \"%s\" on VFS!\n", fname);
    return false;
  }

  bool retcode = LoadLibrary (world, buf);

  CHK (delete [] buf);
  return retcode;
}

csTextureHandle* csLoader::LoadTexture (csWorld* world, const char* name, const char* fname)
{
  csImageFile *image = load_image (fname);
  csTextureHandle *tm = world->GetTextures ()->NewTexture (image);
  tm->SetName (name);
  // dereference image pointer since tm already incremented it
  image->DecRef ();
  return tm;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSounds (csWorld* world, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (SOUND)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (options)
    TOKEN_TABLE (FILE)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_SOUND:
        {
          const char* filename = name;
	  char* maybename;
          cmd = csGetCommand (&params, options, &maybename);
	  if (cmd == TOKEN_FILE)
            filename = maybename;
          else if (cmd == PARSERR_TOKENNOTFOUND)
	  {
            CsPrintf (MSG_FATAL_ERROR, "Unknown token '%s' found while parsing SOUND directive.\n", csGetLastOffender());
            fatal_exit (0, false);
	  }
          csSoundData *snd = csSoundDataObject::GetSound(*world,name);
          if (!snd)
          {
            csSoundDataObject *s = load_sound (name, filename, world);
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

bool csLoader::LoadSkeleton (csSkeletonLimb* limb, char* buf, bool is_connection)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (LIMB)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (TRANSFORM)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* name;
  char* xname;

  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_LIMB:
        {
          CHK (csSkeletonConnection* con = new csSkeletonConnection ());
	  if (!LoadSkeleton (con, params, true)) return false;
	  limb->AddChild (con);
	}
        break;
      case TOKEN_TRANSFORM:
        if (is_connection)
        {
          char* params2;
	  csMatrix3 m;
	  csVector3 v (0, 0, 0);
          while ((cmd = csGetObject (&params, tok_matvec, &xname, &params2)) > 0)
          {
    	    if (!params2)
    	    {
      	      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	      fatal_exit (0, false);
    	    }
            switch (cmd)
            {
              case TOKEN_MATRIX:
                m = load_matrix (params2);
		break;
              case TOKEN_V:
                v = load_vector (params2);
		break;
            }
          }
	  csTransform tr (m, -m.GetInverse () * v);
	  ((csSkeletonConnection*)limb)->SetTransformation (tr);
        }
	else
	{
	  CsPrintf (MSG_FATAL_ERROR, "TRANSFORM not valid for this type of skeleton limb!\n");
	  fatal_exit (0, false);
	}
	break;
      case TOKEN_VERTICES:
        {
          int list[1000], num;	//@@@ HARDCODED!!!
          ScanStr (params, "%D", list, &num);
          for (int i = 0 ; i < num ; i++) limb->AddVertex (list[i]);
        }
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite skeleton!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSpriteTemplate (csSpriteTemplate* stemp, char* buf, csTextureList* textures)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (FRAME)
    TOKEN_TABLE (ACTION)
    TOKEN_TABLE (TRIANGLE)
    TOKEN_TABLE (SKELETON)
    TOKEN_TABLE (FILE)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_frame)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_frameset)
    TOKEN_TABLE (F)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char* params2;
  char str[255];

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_TEXNR:
        {
          ScanStr (params, "%s", str);
          stemp->SetTexture (textures, str);
        }
        break;

      case TOKEN_SKELETON:
	{
          CHK (csSkeleton* skeleton = new csSkeleton ());
	  if (!LoadSkeleton (skeleton, params, false)) return false;
	  stemp->SetSkeleton (skeleton);
	}
        break;

      case TOKEN_ACTION:
        {
          csSpriteAction* act = stemp->AddAction ();
          act->SetName (name);
          int d;
          char fn[64];
          while ((cmd = csGetObject (&params, tok_frameset, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
              fatal_exit (0, false);
            }
            switch (cmd)
            {
              case TOKEN_F:
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

      case TOKEN_FRAME:
        {
          csFrame* fr = stemp->AddFrame ();
          fr->SetName (name);
          int i = 0;
          float x, y, z, u, v;
          while ((cmd = csGetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
              fatal_exit (0, false);
            }
            switch (cmd)
            {
              case TOKEN_V:
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

      case TOKEN_TRIANGLE:
        {
          int a, b, c;
          ScanStr (params, "%d,%d,%d", &a, &b, &c);
          stemp->GetBaseMesh ()->AddTriangle (a, b, c);
        }
        break;

      case TOKEN_FILE:
        {
          ScanStr (params, "%s", str);
	  CHK (converter* filedata = new converter);
	  if (filedata->ivcon (str, true, false, NULL, System->VFS) == ERROR)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Error loading file model '%s'!\n", str);
	    CHK (delete filedata);
	    fatal_exit (0, false);
	  }
	  csCrossBuild_SpriteTemplateFactory builder;
	  builder.CrossBuild (stemp, *filedata);
	  CHK (delete filedata);
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
  stemp->ComputeBoundingBox ();

  int same_count = stemp->MergeTexels ();
  CsPrintf (MSG_INITIALIZATION, "Deleted %d redundant texel maps.\n", same_count);

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSprite (csSprite3D* spr, csWorld* w, char* buf, csTextureList* textures)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEMPLATE)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MOVE)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255], str2[255];
  csSpriteTemplate* tpl;

  csLoaderStat::sprites_loaded++;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_MOVE:
        {
          char* params2;
          spr->SetTransform (csMatrix3 ());     // Identity matrix.
          spr->SetMove (0, 0, 0);
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
              fatal_exit (0, false);
            }
            switch (cmd)
            {
              case TOKEN_MATRIX:
                spr->SetTransform (load_matrix (params2));
                break;
              case TOKEN_V:
                spr->SetMove (load_vector (params2));
                break;
            }
          }
        }
        break;

      case TOKEN_TEMPLATE:
        memset (str, 0, 255);
        memset (str2, 0, 255);
        ScanStr (params, "%s,%s", str, str2);
        tpl = w->GetSpriteTemplate (str);
        if (tpl == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find sprite template '%s'!\n", str);
          fatal_exit (0, true);
        }
        spr->SetTemplate (tpl);
        if (tpl->FindAction (str2) != NULL)
          spr->SetAction (str2);
        break;

      case TOKEN_TEXNR:
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
