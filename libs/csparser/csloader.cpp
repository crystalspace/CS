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

#include "cssysdef.h"
#include "qint.h"
#include "csparser/csloader.h"
#include "csparser/crossbld.h"
#include "csparser/snddatao.h"
#include "csengine/cscoll.h"
#include "csengine/campos.h"
#include "csengine/triangle.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/thingtpl.h"
#include "csengine/cssprite.h"
#include "csengine/csspr2d.h"
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
#include "csengine/particle.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/token.h"
#include "csutil/vfs.h"
#include "csutil/inifile.h"
#include "csutil/util.h"
#include "cssys/system.h"
#include "isndldr.h"
#include "isnddata.h"
#include "isndrdr.h"
#include "csgfxldr/csimage.h"
#include "itxtmgr.h"

typedef char ObName[30];
/// The world we are currently processing
static csWorld* World;
/// Loader flags
static int flags = 0;

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
  static int sounds_loaded;
  static void Init()
  {
    polygons_loaded = 0;
    portals_loaded  = 0;
    sectors_loaded  = 0;
    things_loaded   = 0;
    lights_loaded   = 0;
    curves_loaded   = 0;
    sprites_loaded  = 0;
    sounds_loaded   = 0;
  }
};

int csLoaderStat::polygons_loaded = 0;
int csLoaderStat::portals_loaded  = 0;
int csLoaderStat::sectors_loaded  = 0;
int csLoaderStat::things_loaded   = 0;
int csLoaderStat::lights_loaded   = 0;
int csLoaderStat::curves_loaded   = 0;
int csLoaderStat::sprites_loaded  = 0;
int csLoaderStat::sounds_loaded   = 0;

// Define all tokens used through this file
TOKEN_DEF_START
  TOKEN_DEF (ACCEL)
  TOKEN_DEF (ACTION)
  TOKEN_DEF (ACTIVATE)
  TOKEN_DEF (ACTIVE)
  TOKEN_DEF (ADD)
  TOKEN_DEF (ALPHA)
  TOKEN_DEF (AMBIENT)
  TOKEN_DEF (ATTENUATION)
  TOKEN_DEF (AZIMUTH)
  TOKEN_DEF (BECOMING_ACTIVE)
  TOKEN_DEF (BECOMING_INACTIVE)
  TOKEN_DEF (BEZIER)
  TOKEN_DEF (BOX)
  TOKEN_DEF (BSP)
  TOKEN_DEF (CAMERA)
  TOKEN_DEF (CEILING)
  TOKEN_DEF (CEIL_TEXTURE)
  TOKEN_DEF (CENTER)
  TOKEN_DEF (CIRCLE)
  TOKEN_DEF (CLIP)
  TOKEN_DEF (COLLDET)
  TOKEN_DEF (COLLECTION)
  TOKEN_DEF (COLOR)
  TOKEN_DEF (COLORS)
  TOKEN_DEF (COLORSCALE)
  TOKEN_DEF (CONVEX)
  TOKEN_DEF (COPY)
  TOKEN_DEF (COSFACT)
  TOKEN_DEF (CURVECENTER)
  TOKEN_DEF (CURVECONTROL)
  TOKEN_DEF (CURVESCALE)
  TOKEN_DEF (DETAIL)
  TOKEN_DEF (DIFFUSE)
  TOKEN_DEF (DIM)
  TOKEN_DEF (DITHER)
  TOKEN_DEF (DROPSIZE)
  TOKEN_DEF (DYNAMIC)
  TOKEN_DEF (ELEVATION)
  TOKEN_DEF (F)
  TOKEN_DEF (FALLTIME)
  TOKEN_DEF (FILE)
  TOKEN_DEF (FILTER)
  TOKEN_DEF (FIRE)
  TOKEN_DEF (FIRST)
  TOKEN_DEF (FIRST_LEN)
  TOKEN_DEF (FLAT)
  TOKEN_DEF (FLATCOL)
  TOKEN_DEF (FLOOR)
  TOKEN_DEF (FLOOR_CEIL)
  TOKEN_DEF (FLOOR_HEIGHT)
  TOKEN_DEF (FLOOR_TEXTURE)
  TOKEN_DEF (FOG)
  TOKEN_DEF (FOR_2D)
  TOKEN_DEF (FOR_3D)
  TOKEN_DEF (FOUNTAIN)
  TOKEN_DEF (FRAME)
  TOKEN_DEF (GOURAUD)
  TOKEN_DEF (HALO)
  TOKEN_DEF (HARDMOVE)
  TOKEN_DEF (HEIGHT)
  TOKEN_DEF (HEIGHTMAP)
  TOKEN_DEF (IDENTITY)
  TOKEN_DEF (KEY)
  TOKEN_DEF (KEYCOLOR)
  TOKEN_DEF (LEN)
  TOKEN_DEF (LIBRARY)
  TOKEN_DEF (LIGHT)
  TOKEN_DEF (LIGHTING)
  TOKEN_DEF (LIGHTMAP)
  TOKEN_DEF (LIGHTX)
  TOKEN_DEF (LIMB)
  TOKEN_DEF (MATERIAL)
  TOKEN_DEF (MATERIALS)
  TOKEN_DEF (MATRIX)
  TOKEN_DEF (MAX_TEXTURES)
  TOKEN_DEF (MERGE_NORMALS)
  TOKEN_DEF (MERGE_TEXELS)
  TOKEN_DEF (MERGE_VERTICES)
  TOKEN_DEF (MIPMAP)
  TOKEN_DEF (MIRROR)
  TOKEN_DEF (MIXMODE)
  TOKEN_DEF (MOVE)
  TOKEN_DEF (MOVEABLE)
  TOKEN_DEF (MULTIPLY)
  TOKEN_DEF (MULTIPLY2)
  TOKEN_DEF (NODE)
  TOKEN_DEF (NONE)
  TOKEN_DEF (NUMBER)
  TOKEN_DEF (OPENING)
  TOKEN_DEF (ORIG)
  TOKEN_DEF (ORIGIN)
  TOKEN_DEF (PERSISTENT)
  TOKEN_DEF (PLANE)
  TOKEN_DEF (POLYGON)
  TOKEN_DEF (PORTAL)
  TOKEN_DEF (POSITION)
  TOKEN_DEF (PRIMARY_ACTIVE)
  TOKEN_DEF (PRIMARY_INACTIVE)
  TOKEN_DEF (PROCEDURAL)
  TOKEN_DEF (RADIUS)
  TOKEN_DEF (RAIN)
  TOKEN_DEF (REFLECTION)
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
  TOKEN_DEF (SHADING)
  TOKEN_DEF (SIXFACE)
  TOKEN_DEF (SKELETON)
  TOKEN_DEF (SKY)
  TOKEN_DEF (SKYDOME)
  TOKEN_DEF (SNOW)
  TOKEN_DEF (SOUND)
  TOKEN_DEF (SOUNDS)
  TOKEN_DEF (SPEED)
  TOKEN_DEF (SPLIT)
  TOKEN_DEF (SPRITE)
  TOKEN_DEF (SPRITE2D)
  TOKEN_DEF (START)
  TOKEN_DEF (STATBSP)
  TOKEN_DEF (STATELESS)
  TOKEN_DEF (STATIC)
  TOKEN_DEF (SWIRL)
  TOKEN_DEF (TEMPLATE)
  TOKEN_DEF (TERRAIN)
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
  TOKEN_DEF (TOTALTIME)
  TOKEN_DEF (TRANSFORM)
  TOKEN_DEF (TRANSPARENT)
  TOKEN_DEF (TRIANGLE)
  TOKEN_DEF (TRIGGER)
  TOKEN_DEF (TWEEN)
  TOKEN_DEF (UV)
  TOKEN_DEF (UVA)
  TOKEN_DEF (UVEC)
  TOKEN_DEF (UV_SHIFT)
  TOKEN_DEF (V)
  TOKEN_DEF (VERTEX)
  TOKEN_DEF (VERTICES)
  TOKEN_DEF (VVEC)
  TOKEN_DEF (W)
  TOKEN_DEF (WARP)
  TOKEN_DEF (WORLD)
  TOKEN_DEF (ZFILL)
TOKEN_DEF_END

//---------------------------------------------------------------------------

void csLoader::SetMode (int iFlags)
{
  flags = iFlags;
}

bool csLoader::load_matrix (char* buf, csMatrix3 &m)
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

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_IDENTITY:
        m = identity;
        break;
      case TOKEN_ROT_X:
        ScanStr (params, "%f", &angle);
        m *= csXRotMatrix3 (angle);
        break;
      case TOKEN_ROT_Y:
        ScanStr (params, "%f", &angle);
        m *= csYRotMatrix3 (angle);
        break;
      case TOKEN_ROT_Z:
        ScanStr (params, "%f", &angle);
        m *= csZRotMatrix3 (angle);
        break;
      case TOKEN_ROT:
        ScanStr (params, "%F", list, &num);
        if (num == 3)
        {
          m *= csXRotMatrix3 (list[0]);
          m *= csZRotMatrix3 (list[2]);
          m *= csYRotMatrix3 (list[1]);
        }
        else
	  CsPrintf (MSG_WARNING, "Badly formed rotation: '%s'\n", params);
        break;
      case TOKEN_SCALE_X:
        ScanStr (params, "%f", &scaler);
        m *= csXScaleMatrix3(scaler);
        break;
      case TOKEN_SCALE_Y:
        ScanStr (params, "%f", &scaler);
        m *= csYScaleMatrix3(scaler);
        break;
      case TOKEN_SCALE_Z:
        ScanStr (params, "%f", &scaler);
        m *= csZScaleMatrix3(scaler);
        break;
      case TOKEN_SCALE:
        ScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	  m *= list[0];
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	  m *= csMatrix3 (list[0],0,0,0,list[1],0,0,0,list[2]);
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
      m = csMatrix3 () * list[0];
    else if (num == 9)
      m = csMatrix3 (
        list[0], list[1], list[2],
        list[3], list[4], list[5],
        list[6], list[7], list[8]);
    else
      CsPrintf (MSG_WARNING, "Badly formed matrix '%s'\n", buf);
  }
  return true;
}

bool csLoader::load_vector (char* buf, csVector3 &v)
{
  ScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

bool csLoader::load_color (char *buf, csRGBcolor &c)
{
  float r, g, b;
  ScanStr (buf, "%f,%f,%f", &r, &g, &b);
  c.red   = QInt (r * 255.99);
  c.green = QInt (g * 255.99);
  c.blue  = QInt (b * 255.99);
  return true;
}

csMaterialWrapper *csLoader::FindMaterial (const char *iName)
{
  csMaterialWrapper *mat = World->GetMaterials ()->FindByName (iName);
  if (mat)
    return mat;

  csTextureWrapper *tex = World->GetTextures ()->FindByName (iName);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csMaterial *material = new csMaterial ();
    csMaterialWrapper *mat = World->GetMaterials ()->NewMaterial (material);
    material->SetTextureWrapper (tex);
    mat->SetName (iName);
    material->DecRef ();
    return mat;
  } /* endif */

  CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", iName);
  fatal_exit (0, true);
  return NULL;
}

void csLoader::OptimizePolygon (csPolygon3D *p)
{
  if (!p->GetPortal () || p->GetAlpha ())
    return;

  csMaterialWrapper *mat = p->GetMaterialWrapper ();
  if (mat)
  {
    iMaterial *m = mat->GetMaterial ();
    iTextureHandle *th = m ? m->GetTexture () : NULL;
    if (th && th->GetKeyColor ())
      return;
  }

  p->SetTextureType (POLYTXT_NONE);
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
    TOKEN_TABLE (UVEC)
    TOKEN_TABLE (VVEC)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;
  csPolyTxtPlane* ppl = new csPolyTxtPlane ();
  ppl->SetName (name);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
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
        load_vector (params, tx_orig);
        break;
      case TOKEN_FIRST:
        tx1_given = true;
        load_vector (params, tx1);
        break;
      case TOKEN_FIRST_LEN:
        ScanStr (params, "%f", &tx1_len);
        tx1_given = true;
        break;
      case TOKEN_SECOND:
        tx2_given = true;
        load_vector (params, tx2);
        break;
      case TOKEN_SECOND_LEN:
        ScanStr (params, "%f", &tx2_len);
        tx2_given = true;
        break;
      case TOKEN_MATRIX:
        load_matrix (params, tx_matrix);
        break;
      case TOKEN_V:
        load_vector (params, tx_vector);
        break;
      case TOKEN_UVEC:
        tx1_given = true;
        load_vector (params, tx1);
        tx1_len = tx1.Norm ();
        tx1 += tx_orig;
        break;
      case TOKEN_VVEC:
        tx2_given = true;
        load_vector (params, tx2);
        tx2_len = tx2.Norm ();
        tx2 += tx_orig;
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
    {
      if (!tx1_len)
      {
        CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
        CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
      else ppl->SetTextureSpace (tx_orig, tx1, tx1_len, tx2, tx2_len);
    }
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

csCollection* csLoader::load_collection (char* name, char* buf)
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

  csCollection* collection = new csCollection ();
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
          csThing* th = World->GetThing (str);
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
          csSector* s = (csSector*)World->sectors.FindByName (str);
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
          csSector* s = (csSector*)World->sectors.FindByName (str);
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
          csCollection* th = (csCollection*)World->collections.FindByName (str);
          if (!th)
          {
            CsPrintf (MSG_FATAL_ERROR, "Collection '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          collection->AddObject (th);
        }
        break;
      case TOKEN_TRIGGER:
        CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
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

UInt ParseMixmode (char* buf)
{
  TOKEN_TABLE_START (modes)
    TOKEN_TABLE (COPY)
    TOKEN_TABLE (MULTIPLY2)
    TOKEN_TABLE (MULTIPLY)
    TOKEN_TABLE (ADD)
    TOKEN_TABLE (ALPHA)
    TOKEN_TABLE (TRANSPARENT)
    TOKEN_TABLE (KEYCOLOR)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  UInt Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_COPY: Mixmode |= CS_FX_COPY; break;
      case TOKEN_MULTIPLY: Mixmode |= CS_FX_MULTIPLY; break;
      case TOKEN_MULTIPLY2: Mixmode |= CS_FX_MULTIPLY2; break;
      case TOKEN_ADD: Mixmode |= CS_FX_ADD; break;
      case TOKEN_ALPHA:
	Mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
	int ialpha;
        ScanStr (params, "%f", &alpha);
	ialpha = QInt (alpha * 255.99);
	Mixmode |= CS_FX_SETALPHA(ialpha);
	break;
      case TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
  return Mixmode;
}

//---------------------------------------------------------------------------

csParticleSystem* csLoader::load_fountain (char* name, char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (ORIGIN)
    TOKEN_TABLE (ACCEL)
    TOKEN_TABLE (SPEED)
    TOKEN_TABLE (FALLTIME)
    TOKEN_TABLE (COLOR)
    TOKEN_TABLE (OPENING)
    TOKEN_TABLE (AZIMUTH)
    TOKEN_TABLE (ELEVATION)
    TOKEN_TABLE (DROPSIZE)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (NUMBER)
    TOKEN_TABLE (MIXMODE)
  TOKEN_TABLE_END

  long cmd;
  char* params;

  char str[255];
  int number = 50;
  csMaterialWrapper* material = NULL;
  UInt mixmode = CS_FX_ADD;
  int lighted_particles = 0;
  float drop_width = .1;
  float drop_height = .1;
  csVector3 origin (0);
  csVector3 accel (0, -1.0, 0);
  float fall_time = 5.0;
  float speed = 3.0;
  float opening = 0.2;
  float azimuth = 0.0;
  float elevation = 3.1415926535/2.;
  csColor color (.25, .25, .25);

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case TOKEN_TEXTURE: //@@@MAT
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case TOKEN_ORIGIN:
        ScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
        break;
      case TOKEN_ACCEL:
        ScanStr (params, "%f,%f,%f", &accel.x, &accel.y, &accel.z);
        break;
      case TOKEN_ELEVATION:
        ScanStr (params, "%f", &elevation);
        break;
      case TOKEN_AZIMUTH:
        ScanStr (params, "%f", &azimuth);
        break;
      case TOKEN_OPENING:
        ScanStr (params, "%f", &opening);
        break;
      case TOKEN_SPEED:
        ScanStr (params, "%f", &speed);
        break;
      case TOKEN_FALLTIME:
        ScanStr (params, "%f", &fall_time);
        break;
      case TOKEN_COLOR:
        ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
        break;
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a fountain!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  if (material == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "A fountain requires a material!\n");
    fatal_exit (0, false);
  }

  csFountainParticleSystem* partsys = new csFountainParticleSystem (
  	World, number, material, mixmode, lighted_particles, drop_width,
	drop_height, origin, accel, fall_time, speed, opening,
	azimuth, elevation);
  partsys->SetName (name);
  partsys->SetColor (color);
  return partsys;
}

csParticleSystem* csLoader::load_fire (char* name, char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (ORIGIN)
    TOKEN_TABLE (SPEED)
    TOKEN_TABLE (COLORSCALE)
    TOKEN_TABLE (DROPSIZE)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (NUMBER)
    TOKEN_TABLE (MIXMODE)
    TOKEN_TABLE (SWIRL)
    TOKEN_TABLE (TOTALTIME)
  TOKEN_TABLE_END

  long cmd;
  char* params;

  char str[255];
  int number = 50;
  csMaterialWrapper* material = NULL;
  UInt mixmode = CS_FX_ADD;
  int lighted_particles = 0;
  float drop_width = .1;
  float drop_height = .1;
  csVector3 origin (0);
  float total_time = 3.0;
  csVector3 speed (0, 0.5, 0);
  float colorscale = .35;
  float swirl = 0.3;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case TOKEN_ORIGIN:
        ScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
        break;
      case TOKEN_SWIRL:
        ScanStr (params, "%f", &swirl);
        break;
      case TOKEN_TOTALTIME:
        ScanStr (params, "%f", &total_time);
        break;
      case TOKEN_COLORSCALE:
        ScanStr (params, "%f", &colorscale);
        break;
      case TOKEN_SPEED:
        ScanStr (params, "%f,%f,%f", &speed.x, &speed.y, &speed.z);
        break;
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a fountain!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  if (material == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "A fire requires a material!\n");
    fatal_exit (0, false);
  }

  csFireParticleSystem* partsys = new csFireParticleSystem (
  	World, number, material, mixmode, lighted_particles, drop_width,
	drop_height, total_time, speed, origin, swirl, colorscale);
  partsys->SetName (name);
  return partsys;
}

csParticleSystem* csLoader::load_rain (char* name, char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (BOX)
    TOKEN_TABLE (SPEED)
    TOKEN_TABLE (COLOR)
    TOKEN_TABLE (DROPSIZE)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (NUMBER)
    TOKEN_TABLE (MIXMODE)
  TOKEN_TABLE_END

  long cmd;
  char* params;

  char str[255];
  int number = 400;
  csMaterialWrapper* material = NULL;
  UInt mixmode = CS_FX_ADD;
  int lighted_particles = 0;
  float drop_width = .3/50.;
  float drop_height = .3;
  csVector3 speed (0, -2, 0);
  csBox3 box (-1, -1, -1, 1, 1, 1);
  csColor color (.25, .25, .25);

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case TOKEN_BOX:
      {
        float x1, y1, z1, x2, y2, z2;
        ScanStr (params, "%f,%f,%f,%f,%f,%f", &x1, &y1, &z1, &x2, &y2, &z2);
	box.Set (x1, y1, z1, x2, y2, z2);
        break;
      }
      case TOKEN_SPEED:
        ScanStr (params, "%f,%f,%f", &speed.x, &speed.y, &speed.z);
        break;
      case TOKEN_COLOR:
        ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
        break;
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a fountain!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  if (material == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Rain requires a material!\n");
    fatal_exit (0, false);
  }

  csRainParticleSystem* partsys = new csRainParticleSystem (
  	World, number, material, mixmode, lighted_particles, drop_width,
	drop_height, box.Min (), box.Max (), speed);
  partsys->SetName (name);
  partsys->SetColor (color);
  return partsys;
}

csParticleSystem* csLoader::load_snow (char* name, char* buf)
{
  TOKEN_TABLE_START(commands)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (BOX)
    TOKEN_TABLE (SPEED)
    TOKEN_TABLE (COLOR)
    TOKEN_TABLE (DROPSIZE)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (NUMBER)
    TOKEN_TABLE (MIXMODE)
    TOKEN_TABLE (SWIRL)
  TOKEN_TABLE_END

  long cmd;
  char* params;

  char str[255];
  int number = 400;
  csMaterialWrapper* material = NULL;
  UInt mixmode = CS_FX_ADD;
  int lighted_particles = 0;
  float drop_width = .1;
  float drop_height = .1;
  csVector3 speed (0, -.3, 0);
  float swirl = .2;
  csBox3 box (-1, -1, -1, 1, 1, 1);
  csColor color (.25, .25, .25);

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case TOKEN_SWIRL:
        ScanStr (params, "%f", &swirl);
	break;
      case TOKEN_BOX:
      {
        float x1, y1, z1, x2, y2, z2;
        ScanStr (params, "%f,%f,%f,%f,%f,%f", &x1, &y1, &z1, &x2, &y2, &z2);
	box.Set (x1, y1, z1, x2, y2, z2);
        break;
      }
      case TOKEN_SPEED:
        ScanStr (params, "%f,%f,%f", &speed.x, &speed.y, &speed.z);
        break;
      case TOKEN_COLOR:
        ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
        break;
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a fountain!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  if (material == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Snow requires a material!\n");
    fatal_exit (0, false);
  }

  csSnowParticleSystem* partsys = new csSnowParticleSystem (
  	World, number, material, mixmode, lighted_particles, drop_width,
	drop_height, box.Min (), box.Max (), speed, swirl);
  partsys->SetName (name);
  partsys->SetColor (color);
  return partsys;
}

csStatLight* csLoader::load_statlight (char* name, char* buf)
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
  char str [100];
  struct csHaloDef
  {
    int type;
    union
    {
      struct
      {
        float Intensity;
        float Cross;
      } cross;
      struct
      {
        int Seed;
        int NumSpokes;
        float Roundness;
      } nova;
    };
  } halo;

  memset (&halo, 0, sizeof (halo));

  if (strchr (buf, ':'))
  {
    // Still support old format for backwards compatibility.
    ScanStr (buf, "%f,%f,%f:%f,%f,%f,%f,%d",
          &x, &y, &z, &dist, &r, &g, &b, &dyn);
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
          ScanStr (params, "%s", str);
          if (!strncmp (str, "CROSS", 5))
          {
            params = strchr (str, ',');
            if (params) params++;
defaulthalo:
            halo.type = 1;
            halo.cross.Intensity = 2.0; halo.cross.Cross = 0.45;
            if (params)
              ScanStr (params, "%f,%f", &halo.cross.Intensity, &halo.cross.Cross);
          }
          else if (!strncmp (str, "NOVA", 5))
          {
            params = strchr (str, ',');
            if (params) params++;
            halo.type = 2;
            halo.nova.Seed = 0; halo.nova.NumSpokes = 100; halo.nova.Roundness = 0.5;
            if (params)
              ScanStr (params, "%d,%d,%f", &halo.nova.Seed, &halo.nova.NumSpokes, &halo.nova.Roundness);
          }
          else
            goto defaulthalo;
          break;
        case TOKEN_ATTENUATION:
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

  csStatLight* l = new csStatLight (x, y, z, dist, r, g, b, dyn);
  l->SetName (name);
  switch (halo.type)
  {
    case 1:
      l->SetHalo (new csCrossHalo (halo.cross.Intensity, halo.cross.Cross));
      break;
    case 2:
      l->SetHalo (new csNovaHalo (halo.nova.Seed, halo.nova.NumSpokes, halo.nova.Roundness));
      break;
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
    csKeyValuePair* kvp = new csKeyValuePair(Key, Value);
    if (pParent)
      pParent->ObjAdd (kvp);
    return kvp;
  }
  else
  {
    CsPrintf (MSG_FATAL_ERROR, "Illegal Syntax for KEY() command in line %d\n",
    	parser_line);
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

   csMapNode* pNode = new csMapNode(name);
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

csPolygonSet& csLoader::ps_process (csPolygonSet& ps, csSector* sector,
    PSLoadInfo& info, int cmd, char* name, char* params)
{
  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char str[255];
  char* xname;
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
	csPolygon3D* poly3d = load_poly3d (name, params,
          info.default_material, info.default_texlen,
          info.default_lightx, &ps);
	if (poly3d)
	{
	  ps.AddPolygon (poly3d);
	  csLoaderStat::polygons_loaded++;
	}
      }
      break;

    case TOKEN_HARDMOVE:
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
            {
              csMatrix3 m;
              load_matrix (params2, m);
              info.hard_trans.SetT2O (m);
	      info.do_hard_trans = true;
              break;
            }
            case TOKEN_V:
            {
              csVector3 v;
              load_vector (params2, v);
              info.hard_trans.SetOrigin (v);
	      info.do_hard_trans = true;
              break;
            }
          }
        }
      }
      break;

    case TOKEN_BEZIER:
      //CsPrintf(MSG_WARNING,"Encountered curve!\n");
      ps.AddCurve (load_bezier (name, params,
        info.default_material, info.default_texlen,
        info.default_lightx, sector, &ps) );
      csLoaderStat::curves_loaded++;
      break;

    case TOKEN_TEXNR:
      //@@OBSOLETE, retained for backward compatibility
    case TOKEN_MATERIAL:
      ScanStr (params, "%s", str);
      info.default_material = FindMaterial (str);
      if (info.default_material == NULL)
      {
        CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
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
      CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                               " and does not do anything!\n");
      break;
    case TOKEN_ACTIVATE:
      CsPrintf (MSG_WARNING, "Warning! ACTIVATE statement is obsolete"
                                 " and does not do anything!\n");
      break;
    case TOKEN_TRIGGER:
      CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
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

csThing* csLoader::load_sixface (char* name, char* buf, csSector* sec)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MOVEABLE)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (TEXTURE_SCALE)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (CEIL_TEXTURE)
    TOKEN_TABLE (DETAIL)
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
    TOKEN_TABLE (KEY)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* xname;

  csThing* thing = new csThing (World);
  thing->SetName (name);

  csLoaderStat::things_loaded++;

  thing->GetMovable ().SetSector (sec);
  csReversibleTransform obj;
  csMaterialWrapper* material = NULL;
  bool is_convex = false;
  float tscale = 1;
  int i;

  csVector3 v[8];
  for (i = 0;  i < 8;  i++)
    v [i] = csVector3 ((i & 1 ? 1 : -1), (i & 2 ? -1 : 1), (i & 4 ? -1 : 1));
  float r;

  char str[255];
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
        thing->flags.Set (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
        break;
      case TOKEN_DETAIL:
        thing->flags.Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
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
              {
                csMatrix3 m;
                load_matrix (params2, m);
                obj.SetT2O (m);
                break;
              }
              case TOKEN_V:
              {
                csVector3 v;
                load_vector (params2, v);
                obj.SetOrigin (v);
                break;
              }
            }
          }
        }
        break;
      case TOKEN_TEXTURE: //@@@MAT
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
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
      case TOKEN_KEY:
        load_key (params, thing);
        break;
      case TOKEN_ACTIVATE:
        CsPrintf (MSG_WARNING, "Warning! ACTIVATE statement is obsolete"
                                 " and does not do anything!\n");
        break;
      case TOKEN_TRIGGER:
        CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
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
    csMaterialWrapper* material;
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
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "east");
  todo[todo_end].v1 = 1;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 3;
  todo[todo_end].tv1 = 1;
  todo[todo_end].tv2 = 5;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "south");
  todo[todo_end].v1 = 5;
  todo[todo_end].v2 = 4;
  todo[todo_end].v3 = 6;
  todo[todo_end].v4 = 7;
  todo[todo_end].tv1 = 5;
  todo[todo_end].tv2 = 4;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "west");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 0;
  todo[todo_end].v3 = 2;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 0;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "up");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 1;
  todo[todo_end].v4 = 0;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 5;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "down");
  todo[todo_end].v1 = 2;
  todo[todo_end].v2 = 3;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 2;
  todo[todo_end].tv2 = 3;
  todo[todo_end].material = material;
  todo_end++;

  while (done < todo_end)
  {
    csPolygon3D *p = thing->NewPolygon (todo[done].material);
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
    thing->flags.Set (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX);
  thing->GetMovable ().SetTransform (obj);

  if (!(flags & CS_LOADER_NOCOMPRESS))
    thing->CompressVertices ();
  if (!(flags & CS_LOADER_NOTRANSFORM))
    thing->UpdateMove ();

  return thing;
}

csThing* csLoader::load_thing (char* name, char* buf, csSector* sec)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTEX)
    TOKEN_TABLE (CIRCLE)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (BEZIER)
    TOKEN_TABLE (TEX_SET_SELECT)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
    TOKEN_TABLE (TEXLEN)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (ACTIVATE)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (BSP)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (MOVEABLE)
    TOKEN_TABLE (DETAIL)
    TOKEN_TABLE (CONVEX)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (HARDMOVE)
    TOKEN_TABLE (TEMPLATE)
    TOKEN_TABLE (KEY)
    TOKEN_TABLE (CAMERA)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tok_matvec)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (V)
  TOKEN_TABLE_END

  char* xname;

  csThing* thing = new csThing (World) ;
  thing->SetName (name);

  csLoaderStat::things_loaded++;
  PSLoadInfo info;
  thing->GetMovable ().SetSector (sec);

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
        thing->flags.Set (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
        break;
      case TOKEN_DETAIL:
        thing->flags.Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
        break;
      case TOKEN_CONVEX:
        is_convex = true;
        break;
      case TOKEN_CAMERA:
        thing->flags.Set (CS_ENTITY_CAMERA, CS_ENTITY_CAMERA);
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
              {
                csMatrix3 m;
                load_matrix (params2, m);
                obj.SetT2O (m);
                break;
              }
              case TOKEN_V:
              {
                csVector3 v;
                load_vector (params2, v);
                obj.SetOrigin (v);
                break;
              }
            }
          }
        }
        break;
      case TOKEN_TEMPLATE:
        {
          ScanStr (params, "%s", str);
          csThingTemplate* t = World->GetThingTemplate (str);
          if (!t)
          {
            CsPrintf (MSG_FATAL_ERROR, "Couldn't find thing template '%s'!\n", str);
            fatal_exit (0, false);
          }
	  if (info.use_tex_set)
          {
            thing->MergeTemplate (t, World->GetMaterials (), info.tex_set_name,
              info.default_material, info.default_texlen, info.default_lightx);
            info.use_tex_set = false;
	  }
          else
            thing->MergeTemplate (t, info.default_material, info.default_texlen,
              info.default_lightx);
          csLoaderStat::polygons_loaded += t->GetNumPolygon ();
        }
        break;
      case TOKEN_KEY:
        load_key (params, thing);
        break;
      default:
        ps_process (*thing, sec, info, cmd, xname, params);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a thing!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (info.do_hard_trans)
    thing->HardTransform (info.hard_trans);

  thing->GetMovable ().SetTransform (obj);
  if (!(flags & CS_LOADER_NOCOMPRESS))
    thing->CompressVertices ();
  if (!(flags & CS_LOADER_NOTRANSFORM))
    thing->UpdateMove ();
  if (is_convex || thing->GetFog ().enabled)
    thing->flags.Set (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX);

  return thing;
}



//---------------------------------------------------------------------------

csPolygon3D* csLoader::load_poly3d (char* polyname, char* buf,
  csMaterialWrapper* default_material, float default_texlen,
  CLights* default_lightx, csPolygonSet* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (MIPMAP)
    TOKEN_TABLE (PORTAL)
    TOKEN_TABLE (WARP)
    TOKEN_TABLE (LIGHTX)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (SHADING)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (UVA)
    TOKEN_TABLE (UV)
    TOKEN_TABLE (COLORS)
    TOKEN_TABLE (COLLDET)
    TOKEN_TABLE (ALPHA)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (COSFACT)
    TOKEN_TABLE (GOURAUD)
    TOKEN_TABLE (MIXMODE)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tex_commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (UVEC)
    TOKEN_TABLE (VVEC)
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
    TOKEN_TABLE (ZFILL)
    TOKEN_TABLE (CLIP)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (texturing_commands)
    TOKEN_TABLE (NONE)
    TOKEN_TABLE (FLAT)
    TOKEN_TABLE (GOURAUD)
    TOKEN_TABLE (LIGHTMAP)
  TOKEN_TABLE_END

  char* name;
  int i;
  long cmd;
  char* params, * params2;

  csPolygon3D *poly3d = new csPolygon3D (default_material);
  poly3d->SetName (polyname);

  csMaterialWrapper* mat = NULL;
  poly3d->SetParent (parent);

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
  csPolyTexLightMap* pol_lm = poly3d->GetLightMapInfo ();
  if (pol_lm) pol_lm->SetUniformDynLight (default_lightx);
  int set_colldet = 0; // If 1 then set, if -1 then reset, else default.

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
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = FindMaterial (str);
        if (mat == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        poly3d->SetMaterial (mat);
        break;
      case TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          poly3d->flags.Set (CS_POLY_LIGHTING, do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case TOKEN_MIPMAP:
        //@@@ OBSOLETE
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
          poly3d->SetAlpha (alpha * 655 / 256);
        }
        break;
      case TOKEN_FOG:
        //@@@ OBSOLETE
        break;
      case TOKEN_COLLDET:
        {
          int do_colldet;
          ScanStr (params, "%b", &do_colldet);
	  if (do_colldet) set_colldet = 1;
	  else set_colldet = -1;
        }
        break;
      case TOKEN_PORTAL:
        {
          ScanStr (params, "%s", str);
          csSector *s = new csSector (World);
          s->SetName (str);
          poly3d->SetCSPortal (s);
          csLoaderStat::portals_loaded++;
        }
        break;
      case TOKEN_WARP:
        if (poly3d->GetPortal ())
        {
          csMatrix3 m_w; m_w.Identity ();
          csVector3 v_w_before (0, 0, 0);
          csVector3 v_w_after (0, 0, 0);
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
                load_matrix (params2, m_w);
                do_mirror = false;
                break;
              case TOKEN_V:
                load_vector (params2, v_w_before);
                v_w_after = v_w_before;
                do_mirror = false;
                break;
              case TOKEN_W:
                load_vector (params2, v_w_after);
                do_mirror = false;
                break;
              case TOKEN_MIRROR:
                do_mirror = true;
		if (!set_colldet) set_colldet = 1;
                break;
              case TOKEN_STATIC:
                poly3d->GetPortal ()->flags.Set (CS_PORTAL_STATICDEST);
                break;
      	      case TOKEN_ZFILL:
		poly3d->GetPortal ()->flags.Set (CS_PORTAL_ZFILL);
        	break;
      	      case TOKEN_CLIP:
		poly3d->GetPortal ()->flags.Set (CS_PORTAL_CLIPDEST);
        	break;
            }
          }
          if (!do_mirror)
            poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);
        }
        break;
      case TOKEN_LIGHTX:
        CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                               " and does not do anything!\n");
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
              if (num == 1) tx_orig = parent->Vobj ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
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
              load_matrix (params2, tx_matrix);
              tx_len = 0;
              break;
            case TOKEN_V:
              load_vector (params2, tx_vector);
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
            case TOKEN_UVEC:
              tx1_given = true;
              load_vector (params2, tx1);
              tx1_len = tx1.Norm ();
              tx1 += tx_orig;
              break;
            case TOKEN_VVEC:
              tx2_given = true;
              load_vector (params2, tx2);
              tx2_len = tx2.Norm ();
              tx2 += tx_orig;
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
      case TOKEN_SHADING:
        while ((cmd = csGetObject (&params, texturing_commands, &name, &params2)) > 0)
          switch (cmd)
          {
            case TOKEN_NONE:
              poly3d->SetTextureType (POLYTXT_NONE);
              break;
            case TOKEN_FLAT:
              poly3d->SetTextureType (POLYTXT_FLAT);
              break;
            case TOKEN_GOURAUD:
              poly3d->SetTextureType (POLYTXT_GOURAUD);
              break;
            case TOKEN_LIGHTMAP:
              poly3d->SetTextureType (POLYTXT_LIGHTMAP);
              break;
          }
        break;
      case TOKEN_GOURAUD:
        //@@OBSOLETE, see above
        break;
      case TOKEN_MIXMODE:
        {
          UInt mixmode = ParseMixmode (params);
          csPolyTexNone *notex = poly3d->GetNoTexInfo ();
	  if (notex) notex->SetMixmode (mixmode);
          if (mixmode & CS_FX_MASK_ALPHA)
            poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
          break;
	}
      case TOKEN_UV:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  csPolyTexFlat* fs = poly3d->GetFlatInfo ();
          int num, nv = poly3d->GetVertices ().GetNumVertices ();
	  fs->Setup (poly3d);
          float list [2 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
	  int j;
          for (j = 0; j < num; j++)
            fs->SetUV (j, list [j * 2], list [j * 2 + 1]);
        }
        break;
      case TOKEN_COLORS:
        {
          poly3d->SetTextureType (POLYTXT_GOURAUD);
	  csPolyTexGouraud* gs = poly3d->GetGouraudInfo ();
          int num, nv = poly3d->GetVertices ().GetNumVertices ();
	  gs->Setup (poly3d);
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
	  csPolyTexFlat* fs = poly3d->GetFlatInfo ();
          int num, nv = poly3d->GetVertices ().GetNumVertices ();
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
        }
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a polygon!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (poly3d->GetNumVertices () < 3)
  {
    CsPrintf (MSG_WARNING, "Polygon in line %d contains just %d vertices!\n", parser_line, poly3d->GetNumVertices());
    return NULL;
  }

  if (set_colldet == 1)
    poly3d->flags.Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->flags.Reset (CS_POLY_COLLDET);

  if (tx1_given)
    if (tx2_given)
    {
      if (!tx1_len)
      {
        CsPrintf (MSG_WARNING, "Bad texture specification for POLYGON '%s'\n", polyname);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
        CsPrintf (MSG_WARNING, "Bad texture specification for POLYGON '%s'\n", polyname);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
      else poly3d->SetTextureSpace (tx_orig.x, tx_orig.y, tx_orig.z,
                               tx1.x, tx1.y, tx1.z, tx1_len,
                               tx2.x, tx2.y, tx2.z, tx2_len);
    }
  else
  {
    if (!tx1_len)
    {
      CsPrintf (MSG_WARNING, "Bad texture specification for POLYGON '%s'\n", polyname);
      tx1_len = 1;
    }
    if ((tx1-tx_orig) < SMALL_EPSILON)
      CsPrintf (MSG_WARNING, "Bad texture specification for PLANE '%s'\n", name);
    else poly3d->SetTextureSpace (tx_orig.x, tx_orig.y, tx_orig.z,
                             tx1.x, tx1.y, tx1.z, tx1_len);
  }
  else if (plane_name[0])
    poly3d->SetTextureSpace ((csPolyTxtPlane*)World->planes.FindByName (plane_name));
  else if (tx_len)
  {
    // If a length is given (with 'LEN') we will take the first two vertices
    // and calculate the texture orientation from them (with the given
    // length).
    poly3d->SetTextureSpace (poly3d->Vobj(0), poly3d->Vobj(1), tx_len);
  }
  else
    poly3d->SetTextureSpace (tx_matrix, tx_vector);

  if (uv_shift_given && poly3d->GetLightMapInfo ())
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

  OptimizePolygon (poly3d);

  return poly3d;
}


csCurve* csLoader::load_bezier (char* polyname, char* buf,
  csMaterialWrapper* default_material, float default_texlen,
  CLights* default_lightx, csSector* sec, csPolygonSet* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
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
    TOKEN_TABLE (UVEC)
    TOKEN_TABLE (VVEC)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params, * params2;

  (void)default_lightx; (void)sec; (void)parent;

  csBezierCurve *curve = new csBezierCurve (NULL);
  curve->SetName (polyname);
  curve->SetMaterialWrapper (default_material);
  curve->SetSector(sec);
  csMaterialWrapper* mat = NULL;
//TODO??  poly3d->SetSector(sec);
//TODO??  poly3d->SetParent (parent);

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
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = FindMaterial (str);
        if (mat == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        curve->SetMaterialWrapper (mat);
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
              if (num == 1) tx_orig = parent->Vobj ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
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
              load_matrix (params2, tx_matrix);
              tx_len = 0;
              break;
            case TOKEN_V:
              load_vector (params2, tx_vector);
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
            case TOKEN_UVEC:
              tx1_given = true;
              load_vector (params2, tx1);
              tx1_len = tx1.Norm ();
              tx1 += tx_orig;
              break;
            case TOKEN_VVEC:
              tx2_given = true;
              load_vector (params2, tx2);
              tx2_len = tx2.Norm ();
              tx2 += tx_orig;
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

  return curve;
}



//---------------------------------------------------------------------------

iImage* csLoader::load_image (const char* name)
{
  size_t size;
  iImage *ifile = NULL;
  char *buf = System->VFS->ReadFile (name, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", name);
    return NULL;
  }

  ifile = csImageLoader::Load ((UByte *)buf, size, World->GetTextureFormat ());
  delete [] buf;

  if (!ifile)
  {
    CsPrintf (MSG_WARNING, "'%s': Cannot load image. Unknown format or wrong extension!\n",name);
    return NULL;
  }

  char *xname = System->VFS->ExpandPath (name);
  ifile->SetName (xname);
  delete [] xname;

  return ifile;
}

void csLoader::txt_process (char *name, char* buf, const char* prefix)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TRANSPARENT)
    TOKEN_TABLE (FILTER)
    TOKEN_TABLE (FILE)
    TOKEN_TABLE (MIPMAP)
    TOKEN_TABLE (DITHER)
    TOKEN_TABLE (PROCEDURAL)
    TOKEN_TABLE (PERSISTENT)
    TOKEN_TABLE (FOR_2D)
    TOKEN_TABLE (FOR_3D)
  TOKEN_TABLE_END

  long cmd;
  const char *filename = name;
  char *params;
  csColor transp (0, 0, 0);
  bool do_transp = false;
  int flags = CS_TEXTURE_3D;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_FOR_2D:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_2D;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_2D;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid FOR_2D() value, 'yes' or 'no' expected\n");
        break;
      case TOKEN_FOR_3D:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_3D;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_3D;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid FOR_3D() value, 'yes' or 'no' expected\n");
        break;
      case TOKEN_PERSISTENT:
        flags |= CS_TEXTURE_PROC_PERSISTENT;
        break;
      case TOKEN_PROCEDURAL:
        flags |= CS_TEXTURE_PROC;
        break;
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
      case TOKEN_MIPMAP:
        if (strcasecmp (params, "yes") == 0)
          flags &= ~CS_TEXTURE_NOMIPMAPS;
        else if (strcasecmp (params, "no") == 0)
          flags |= CS_TEXTURE_NOMIPMAPS;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid MIPMAP() value, 'yes' or 'no' expected\n");
        break;
      case TOKEN_DITHER:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_DITHER;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_DITHER;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid MIPMAP() value, 'yes' or 'no' expected\n");
        break;
    }
  }

  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a texture specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  iImage *image = load_image (filename);
  if (!image)
    return;

  // The size of image should be checked before registering it with
  // the 3D or 2D driver... if the texture is used for 2D only, it can
  // not have power-of-two dimensions...

  // Add a default material with the same name as the texture.
//csMaterial *material = new csMaterial ();
//csMaterialWrapper *mat = World->GetMaterials ()->NewMaterial (material);

  csTextureWrapper *tex = World->GetTextures ()->NewTexture (image);
//material->SetTextureWrapper (tex);
  tex->flags = flags;
  if (prefix)
  {
    char *prefixedname = new char [strlen (name) + strlen (prefix) + 2];
    strcpy (prefixedname, prefix);
    strcat (prefixedname, "_");
    strcat (prefixedname, name);
    tex->SetName (prefixedname);
//  mat->SetName (prefixedname);
    delete [] prefixedname;
  }
  else
  {
    tex->SetName (name);
//  mat->SetName (name);
  }
  // dereference image pointer since tex already incremented it
  image->DecRef ();
//material->DecRef ();

  if (do_transp)
    tex->SetKeyColor (QInt (transp.red * 255.99),
      QInt (transp.green * 255.99), QInt (transp.blue * 255.99));
}

void csLoader::mat_process (char *name, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (COLOR)
    TOKEN_TABLE (DIFFUSE)
    TOKEN_TABLE (AMBIENT)
    TOKEN_TABLE (REFLECTION)
  TOKEN_TABLE_END

  long cmd;
  char *params;
  char str [255];
  float tmp;

  csMaterial* material = new csMaterial ();

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_TEXTURE:
      {
        ScanStr (params, "%s", str);
        csTextureWrapper *texh = World->GetTextures ()->FindByName (str);
        if (texh)
          material->SetTextureWrapper (texh);
        else
        {
          CsPrintf (MSG_FATAL_ERROR, "Cannot find texture `%s' for material `%s'\n", str, name);
          fatal_exit (0, false);
        }
        break;
      }
      case TOKEN_COLOR:
        load_color (params, material->GetFlatColor ());
        break;
      case TOKEN_DIFFUSE:
        ScanStr (params, "%f", &tmp);
        material->SetDiffuse (tmp);
        break;
      case TOKEN_AMBIENT:
        ScanStr (params, "%f", &tmp);
        material->SetAmbient (tmp);
        break;
      case TOKEN_REFLECTION:
        ScanStr (params, "%f", &tmp);
        material->SetReflection (tmp);
        break;
    }
  }

  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a material specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  csMaterialWrapper *mat = World->GetMaterials ()->NewMaterial (material);
  mat->SetName (name);
  // dereference material since mat already incremented it
  material->DecRef ();
}

//---------------------------------------------------------------------------

csPolygonTemplate* csLoader::load_ptemplate (char* ptname, char* buf,
  csMaterialWrapper* default_material, float default_texlen,
  csThingTemplate* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
    TOKEN_TABLE (LIGHTING)
    TOKEN_TABLE (MIPMAP)
    TOKEN_TABLE (TEXTURE)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (FLATCOL)
    TOKEN_TABLE (GOURAUD)
    TOKEN_TABLE (COLLDET)
    TOKEN_TABLE (SHADING)
    TOKEN_TABLE (COLORS)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (tex_commands)
    TOKEN_TABLE (ORIG)
    TOKEN_TABLE (FIRST_LEN)
    TOKEN_TABLE (FIRST)
    TOKEN_TABLE (SECOND_LEN)
    TOKEN_TABLE (SECOND)
    TOKEN_TABLE (LEN)
    TOKEN_TABLE (MATRIX)
    TOKEN_TABLE (UVEC)
    TOKEN_TABLE (VVEC)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (texturing_commands)
    TOKEN_TABLE (NONE)
    TOKEN_TABLE (FLAT)
    TOKEN_TABLE (GOURAUD)
    TOKEN_TABLE (LIGHTMAP)
  TOKEN_TABLE_END

  char* name;
  int i;
  long cmd;
  char* params, * params2;

  csPolygonTemplate *ptemplate =
              new csPolygonTemplate(parent, ptname, default_material);
  csMaterialWrapper* mat;
  if (default_material == NULL) mat = NULL;
  else ptemplate->SetMaterial (default_material);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
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
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = FindMaterial (str);
        if (mat == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        ptemplate->SetMaterial (mat);
        break;
      case TOKEN_SHADING:
        while ((cmd = csGetObject (&params, texturing_commands, &name, &params2)) > 0)
          switch (cmd)
          {
            case TOKEN_NONE:
              ptemplate->flags.Set (CS_POLYTPL_TEXMODE, CS_POLYTPL_TEXMODE_NONE);
              break;
            case TOKEN_FLAT:
              ptemplate->flags.Set (CS_POLYTPL_TEXMODE, CS_POLYTPL_TEXMODE_FLAT);
              break;
            case TOKEN_GOURAUD:
              ptemplate->flags.Set (CS_POLYTPL_TEXMODE, CS_POLYTPL_TEXMODE_GOURAUD);
              break;
            case TOKEN_LIGHTMAP:
              ptemplate->flags.Set (CS_POLYTPL_TEXMODE, CS_POLYTPL_TEXMODE_LIGHTMAP);
              break;
          }
        break;
      case TOKEN_GOURAUD:
        //@@OBSOLETE, see above
        break;
      case TOKEN_FLATCOL:
        //@@OBSOLETE, flat color belongs to material not to polygon
        break;
      case TOKEN_MIPMAP:
        //@@@ OBSOLETE
        break;
      case TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          ptemplate->flags.SetBool (CS_POLY_LIGHTING, do_lighting);
        }
        break;
      case TOKEN_COLLDET:
        {
          int do_colldet;
          ScanStr (params, "%b", &do_colldet);
          ptemplate->flags.SetBool (CS_POLYTPL_COLLDET,
            do_colldet ? CS_POLYTPL_COLLDET_ENABLE : CS_POLYTPL_COLLDET_DISABLE);
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
              if (num == 1) tx_orig = parent->Vtex ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
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
              load_matrix (params2, tx_matrix);
              tx_len = 0;
              break;
            case TOKEN_V:
              load_vector (params2, tx_vector);
              tx_len = 0;
              break;
            case TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
            case TOKEN_UVEC:
              tx1_given = true;
              load_vector (params2, tx1);
              tx1_len = tx1.Norm ();
              tx1 += tx_orig;
              break;
            case TOKEN_VVEC:
              tx2_given = true;
              load_vector (params2, tx2);
              tx2_len = tx2.Norm ();
              tx2 += tx_orig;
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
      case TOKEN_COLORS:
        {
          ptemplate->flags.Set (CS_POLYTPL_TEXMODE, CS_POLYTPL_TEXMODE_GOURAUD);
	  int num, nv = ptemplate->GetNumVertices ();
	  float list [3 * 100];
          ScanStr (params, "%F", list, &num);
          if (num > nv) num = nv;
          for (int j = 0; j < num; j++)
            ptemplate->SetColor (j, list [j * 3], list [j * 3 + 1], list [j * 3 + 2]);
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
        tx_orig, tx1, tx1_len, tx2, tx2_len);
    else
    {
      float A, B, C;
      ptemplate->PlaneNormal (&A, &B, &C);
      TextureTrans::compute_texture_space (tx_matrix, tx_vector,
        tx_orig, tx1, tx1_len, A, B, C);
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
  csMaterialWrapper* default_material, float default_texlen,
  csThingTemplate* parent)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
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
    TOKEN_TABLE (UVEC)
    TOKEN_TABLE (VVEC)
    TOKEN_TABLE (V)
    TOKEN_TABLE (UV_SHIFT)
  TOKEN_TABLE_END

  char *name;
  long cmd;
  int i;
  char *params, *params2;

  csBezierTemplate *ptemplate = new csBezierTemplate();
  ptemplate->SetName (ptname);

  ptemplate->SetParent (parent);

  csMaterialWrapper* mat;
  if (default_material == NULL) mat = NULL;
  else ptemplate->SetMaterialWrapper (default_material);

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
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
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = FindMaterial (str);
        if (mat == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        ptemplate->SetMaterialWrapper (mat);
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
              if (num == 1) tx_orig = parent->CurveVertex ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
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
              load_matrix (params2, tx_matrix);
              tx_len = 0;
              break;
            case TOKEN_V:
              load_vector (params2, tx_vector);
              tx_len = 0;
              break;
            case TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
            case TOKEN_UVEC:
              tx1_given = true;
              load_vector (params2, tx1);
              tx1_len = tx1.Norm ();
              tx1 += tx_orig;
              break;
            case TOKEN_VVEC:
              tx2_given = true;
              load_vector (params2, tx2);
              tx2_len = tx2.Norm ();
              tx2 += tx_orig;
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

csThingTemplate* csLoader::load_thingtpl (char* tname, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTEX)
    TOKEN_TABLE (CIRCLE)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (BEZIER)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
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

  csThingTemplate *tmpl = new csThingTemplate() ;
  tmpl->SetName (tname);
  long cmd;
  char* params;
  csMaterialWrapper* default_material = NULL;
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
        tmpl->AddPolygon (load_ptemplate (name, params, default_material,
          default_texlen, tmpl));
        break;
      case TOKEN_BEZIER:
        //CsPrintf(MSG_WARNING,"Encountered template curve!\n");
        tmpl->AddCurve (load_beziertemplate(name, params, default_material,
          default_texlen, tmpl));
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
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        default_material = FindMaterial (str);
        if (default_material == NULL)
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
          load_matrix(params2, m_move);
          csGetObject (&params, tok_vector, &name, &params2);
          load_vector(params2, v_move);
        }
        break;

      case TOKEN_FILE:
        {
          ScanStr (params, "%s", str);
	  converter* filedata = new converter;
	  if (filedata->ivcon (str, true, false, NULL, System->VFS) == ERROR)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Error loading file model '%s'!\n", str);
	    delete filedata;
	    fatal_exit (0, false);
	  }
	  csCrossBuild_ThingTemplateFactory builder;
	  builder.CrossBuild (tmpl, *filedata);
	  delete filedata;
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

csThingTemplate* csLoader::load_sixtpl (char* tname, char* buf)
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

  csThingTemplate* tmpl = new csThingTemplate ();
  tmpl->SetName (tname);

  csMaterialWrapper* material = NULL;
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
          load_matrix (params2, m_move);
          csGetObject (&params, tok_vector, &name, &params2);
          load_vector (params2, v_move);
        }
        break;
      case TOKEN_TEXTURE: //@@@MAT
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
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
    csMaterialWrapper* material;
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
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "east");
  todo[todo_end].v1 = 1;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 3;
  todo[todo_end].tv1 = 1;
  todo[todo_end].tv2 = 5;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "south");
  todo[todo_end].v1 = 5;
  todo[todo_end].v2 = 4;
  todo[todo_end].v3 = 6;
  todo[todo_end].v4 = 7;
  todo[todo_end].tv1 = 5;
  todo[todo_end].tv2 = 4;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "west");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 0;
  todo[todo_end].v3 = 2;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 0;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "up");
  todo[todo_end].v1 = 4;
  todo[todo_end].v2 = 5;
  todo[todo_end].v3 = 1;
  todo[todo_end].v4 = 0;
  todo[todo_end].tv1 = 4;
  todo[todo_end].tv2 = 5;
  todo[todo_end].material = material;
  todo_end++;

  strcpy (todo[todo_end].poly, "down");
  todo[todo_end].v1 = 2;
  todo[todo_end].v2 = 3;
  todo[todo_end].v3 = 7;
  todo[todo_end].v4 = 6;
  todo[todo_end].tv1 = 2;
  todo[todo_end].tv2 = 3;
  todo[todo_end].material = material;
  todo_end++;

  while (done < todo_end)
  {
    p = new csPolygonTemplate (tmpl, todo[done].poly, todo[done].material);
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
  csMaterialWrapper* material;
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
  csMaterialWrapper* material;
  int col_idx;          // Idx in colors table if there was an override.
  CLights* light;       // A dynamic light for this polygon
};

void add_to_todo (Todo* todo, int& todo_end, char* poly,
        int v1, int v2, int v3, int v4, int tv1, int tv2,
        csMaterialWrapper* material, int col_idx,
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
  todo[todo_end].material = material;
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

void csLoader::load_tex (char** buf, Color* colors, int num_colors, char* name)
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
  colors[num_colors].material = NULL;
  colors[num_colors].len = 0;

  while ((cmd = csGetCommand (buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_TEXTURE://@@@MAT
        ScanStr (params, "%s", str);
        colors[num_colors].material = FindMaterial (str);
        if (colors[num_colors].material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
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

csSector* csLoader::load_room (char* secname, char* buf)
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
    TOKEN_TABLE (SKY)
    TOKEN_TABLE (PORTAL)
    TOKEN_TABLE (SPLIT)
    TOKEN_TABLE (TRIGGER)
    TOKEN_TABLE (ACTIVATE)
    TOKEN_TABLE (BSP)
    TOKEN_TABLE (STATBSP)
    TOKEN_TABLE (SPRITE2D)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (FOG)
    TOKEN_TABLE (FOUNTAIN)
    TOKEN_TABLE (RAIN)
    TOKEN_TABLE (SNOW)
    TOKEN_TABLE (FIRE)
    TOKEN_TABLE (KEY)
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

  csSector* sector = new csSector (World);
  sector->SetName (secname);

  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  csLoaderStat::sectors_loaded++;

  csMatrix3 mm;
  csVector3 vm (0, 0, 0);
  csMaterialWrapper* material = NULL;
  float tscale = 1;
  int no_lighting = false;

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
  csParticleSystem* partsys;

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
          "BSP keyword is no longer supported. Use STATBSP instead after putting\n"
          "all non-convex polygons in things.\n");
        break;
      case TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case TOKEN_MOVE:
        {
          char* params2;
          csGetObject (&params, tok_matrix, &name, &params2);
          load_matrix (params2, mm);
          csGetObject (&params, tok_vector, &name, &params2);
          load_vector (params2, vm);
        }
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case TOKEN_TEXTURE_LIGHTING:
        ScanStr (params, "%b", &no_lighting); no_lighting = !no_lighting;
        break;
      case TOKEN_TEXTURE_MIPMAP:
        //@@@ OBSOLETE
        break;
      case TOKEN_CEIL_TEXTURE:
      case TOKEN_FLOOR_TEXTURE:
        ScanStr (params, "%s", str);
        colors[num_colors].material = FindMaterial (str);
        if (colors[num_colors].material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
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
        CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                               " and does not do anything!\n");
        break;
      case TOKEN_TEX:
        load_tex (&params, colors, num_colors, name);
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
        sector->AddLight ( load_statlight(name, params) );
        break;
      case TOKEN_SIXFACE:
        sector->things.Push (load_sixface (name,params,sector));
        break;
      case TOKEN_FOG:
        {
          csFog& f = sector->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case TOKEN_KEY:
        load_key (params, sector);
        break;
      case TOKEN_FIRE:
        partsys = load_fire (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_FOUNTAIN:
        partsys = load_fountain (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_RAIN:
        partsys = load_rain (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_SNOW:
        partsys = load_snow (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_SPRITE:
        {
          csSprite3D* sp = new csSprite3D (World);
          sp->SetName (name);
          LoadSprite (sp, params);
          World->sprites.Push (sp);
          sp->MoveToSector (sector);
        }
        break;
      case TOKEN_SPRITE2D:
        {
          csSprite2D* sp = new csSprite2D (World);
          sp->SetName (name);
          LoadSprite (sp, params);
          World->sprites.Push (sp);
          sp->MoveToSector (sector);
        }
        break;
      case TOKEN_SKY:
        sector->skies.Push (load_thing (name,params,sector));
        break;
      case TOKEN_THING:
        sector->things.Push (load_thing (name,params,sector));
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
                portals[num_portals].alpha = portals[num_portals].alpha * 655 / 256;
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
                        load_matrix (params3, portals[num_portals].m_warp);
                        portals[num_portals].do_mirror = false;
                        break;
                      case TOKEN_V:
                        load_vector (params3, portals[num_portals].v_warp_before);
                        portals[num_portals].v_warp_after =
                          portals[num_portals].v_warp_before;
                        portals[num_portals].do_mirror = false;
                        break;
                      case TOKEN_W:
                        load_vector (params3, portals[num_portals].v_warp_after);
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
        CsPrintf (MSG_WARNING, "Warning! ACTIVATE statement is obsolete"
                                 " and does not do anything!\n");
        break;
      case TOKEN_TRIGGER:
        CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
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
               material, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "east", 1, 5, 7, 3, 1, 5,
               material, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "south", 5, 4, 6, 7, 5, 4,
               material, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "west", 4, 0, 2, 6, 4, 0,
               material, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "up", 4, 5, 1, 0, 4, 5,
               material, -1, NULL, colors, num_colors, dlights, num_light);
  add_to_todo (todo, todo_end, "down", 2, 3, 7, 6, 2, 3,
               material, -1, NULL, colors, num_colors, dlights, num_light);

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
                      todo[done].tv2, todo[done].material, todo[done].col_idx,
                      todo[done].light, colors, num_colors, dlights,num_light);
          i1 = sector->GetNumVertices () - 2;
          i4 = sector->GetNumVertices () - 1;
        }

        sprintf (pname, "%s%c", todo[done].poly, l+'A');
        add_to_todo (todo, todo_end, pname, i1, i2, i3, i4, todo[done].tv1,
                     todo[done].tv2, todo[done].material, todo[done].col_idx,
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
                      todo[done].material, todo[done].col_idx, todo[done].light,
                      colors, num_colors, dlights, num_light);
          i3 = sector->GetNumVertices () - 1;
          i4 = sector->GetNumVertices () - 2;
        }

        sprintf (pname, "%s%d", todo[done].poly, l+1);
        add_to_todo (todo, todo_end, pname, i1, i2, i3, i4, todo[done].tv1,
                     todo[done].tv2, todo[done].material, todo[done].col_idx,
                     todo[done].light, colors, num_colors, dlights, num_light);
      }
    }
    else
    {
      float len;
      int idx = todo[done].col_idx;
      if (idx == -1 || colors[idx].material == NULL)
        material = todo[done].material;
      else material = colors[idx].material;

      p = sector->NewPolygon (material);
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
        p->SetTextureSpace ((csPolyTxtPlane*)World->planes.FindByName (colors[idx].plane));
      p->flags.Set (CS_POLY_LIGHTING, (no_lighting ? 0 : CS_POLY_LIGHTING));
      csPolyTexLightMap* pol_lm = p->GetLightMapInfo ();
      if (pol_lm) pol_lm->SetUniformDynLight (todo[done].light);
      OptimizePolygon (p);
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
    portal = new csSector (World) ;
    portal->SetName (portals[i].sector);
    p->SetCSPortal (portal);
    csLoaderStat::portals_loaded++;
    if (portals[i].is_warp)
    {
      if (portals[i].do_mirror)
        p->SetWarp (csTransform::GetReflect ( *(p->GetPolyPlane ()) ));
      else p->SetWarp (portals[i].m_warp, portals[i].v_warp_before,
                        portals[i].v_warp_after);
      p->GetPortal ()->flags.SetBool (CS_PORTAL_STATICDEST, portals[i].do_static);
    }
    p->SetAlpha (portals [i].alpha);
  }

  if (!(flags & CS_LOADER_NOCOMPRESS))
    sector->CompressVertices ();
  if (!(flags & CS_LOADER_NOBSP))
    if (do_stat_bsp) sector->UseStaticTree ();

  return sector;
}

csSector* csLoader::load_sector (char* secname, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTEX)
    TOKEN_TABLE (CIRCLE)
    TOKEN_TABLE (POLYGON)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
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
    TOKEN_TABLE (SPRITE2D)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (SKYDOME)
    TOKEN_TABLE (SKY)
    TOKEN_TABLE (TERRAIN)
    TOKEN_TABLE (NODE)
    TOKEN_TABLE (KEY)
    TOKEN_TABLE (FOUNTAIN)
    TOKEN_TABLE (RAIN)
    TOKEN_TABLE (SNOW)
    TOKEN_TABLE (FIRE)
    TOKEN_TABLE (HARDMOVE)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  bool do_stat_bsp = false;

  csSector* sector = new csSector (World) ;
  sector->SetName (secname);

  csLoaderStat::sectors_loaded++;
  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  PSLoadInfo info;
  csParticleSystem* partsys;

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
        skydome_process (*sector, name, params, info.default_material);
        break;
      case TOKEN_TERRAIN:
        terrain_process (*sector, name, params);
        break;
      case TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case TOKEN_FIRE:
        partsys = load_fire (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_FOUNTAIN:
        partsys = load_fountain (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_RAIN:
        partsys = load_rain (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_SNOW:
        partsys = load_snow (name, params);
	partsys->MoveToSector (sector);
        break;
      case TOKEN_SKY:
        sector->skies.Push (load_thing (name,params,sector));
        break;
      case TOKEN_THING:
        sector->things.Push (load_thing (name,params,sector));
        break;
      case TOKEN_SPRITE:
        {
          csSprite3D* sp = new csSprite3D (World);
          sp->SetName (name);
          LoadSprite (sp, params);
          World->sprites.Push (sp);
          sp->MoveToSector (sector);
        }
        break;
      case TOKEN_SPRITE2D:
        {
          csSprite2D* sp = new csSprite2D (World);
          sp->SetName (name);
          LoadSprite (sp, params);
          World->sprites.Push (sp);
          sp->MoveToSector (sector);
        }
        break;
      case TOKEN_SIXFACE:
        sector->things.Push ( load_sixface(name,params,sector) );
        break;
      case TOKEN_LIGHT:
        sector->AddLight ( load_statlight(name, params) );
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
        ps_process (*sector, sector, info, cmd, name, params);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a sector!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (info.do_hard_trans)
    sector->HardTransform (info.hard_trans);

  if (!(flags & CS_LOADER_NOCOMPRESS))
    sector->CompressVertices ();
  if (!(flags & CS_LOADER_NOBSP))
    if (do_stat_bsp) sector->UseStaticTree ();
  return sector;
}

void csLoader::skydome_process (csSector& sector, char* name, char* buf,
        csMaterialWrapper* material)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (RADIUS)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (LIGHTING)
  TOKEN_TABLE_END

  long cmd;
  char* params;
  float radius = 0.0f;
  int i, j;
  int num = 0;
  csPolyTexGouraud* gs;

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
      case TOKEN_RADIUS:
        ScanStr (params, "%f", &radius);
        break;
      case TOKEN_VERTICES:
        ScanStr (params, "%D", prev_vertices, &num);
        break;
      case TOKEN_LIGHTING:
        {
	  int do_lighting;
          ScanStr (params, "%b", &do_lighting);
	  if (do_lighting) lighting_flags = CS_POLY_LIGHTING;
	  else lighting_flags = 0;
        }
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
      csPolygon3D* p = new csPolygon3D (material);
      p->SetName (poly_name);
      p->SetParent (&sector);
      p->flags.Set (CS_POLY_LIGHTING, lighting_flags);
      p->SetCosinusFactor (1);
      p->AddVertex (prev_vertices[j]);
      p->AddVertex (new_vertices[(j+1)%num]);
      p->AddVertex (new_vertices[j]);
      p->SetTextureType (POLYTXT_GOURAUD);
      gs = p->GetGouraudInfo ();
      gs->Setup (p);
      gs->SetUV (0, prev_u[j], prev_v[j]);
      gs->SetUV (1, new_u[(j+1)%num], new_v[(j+1)%num]);
      gs->SetUV (2, new_u[j], new_v[j]);

      p->SetTextureSpace (t_m, t_v);
      sector.AddPolygon (p);
      csLoaderStat::polygons_loaded++;
      sprintf (end_poly_name, "%d_%d_B", i, j);
      p = new csPolygon3D (material);
      p->SetName (poly_name);
      p->SetParent (&sector);
      p->flags.Set (CS_POLY_LIGHTING, lighting_flags);
      p->SetCosinusFactor (1);
      p->AddVertex (prev_vertices[j]);
      p->AddVertex (prev_vertices[(j+1)%num]);
      p->AddVertex (new_vertices[(j+1)%num]);
      p->SetTextureType (POLYTXT_GOURAUD);
      gs = p->GetGouraudInfo ();
      gs->Setup (p);
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
    csPolygon3D* p = new csPolygon3D (material);
    p->SetName (poly_name);
    p->SetParent (&sector);
    p->flags.Set (CS_POLY_LIGHTING, lighting_flags);
    p->SetCosinusFactor (1);
    p->AddVertex (top_vertex);
    p->AddVertex (prev_vertices[j]);
    p->AddVertex (prev_vertices[(j+1)%num]);
    p->SetTextureType (POLYTXT_GOURAUD);
    gs = p->GetGouraudInfo ();
    gs->Setup (p);
    gs->SetUV (0, top_u, top_v);
    gs->SetUV (1, prev_u[j], prev_v[j]);
    gs->SetUV (2, prev_u[(j+1)%num], prev_v[(j+1)%num]);
    p->SetTextureSpace (t_m, t_v);
    sector.AddPolygon (p);
    csLoaderStat::polygons_loaded++;
  }
}

//---------------------------------------------------------------------------

void csLoader::terrain_process (csSector& sector, char* name, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (HEIGHTMAP)
    TOKEN_TABLE (DETAIL)
    TOKEN_TABLE (TEXTURE)
  TOKEN_TABLE_END

  long cmd;
  char* params;
  char heightmapname[256];	// @@@ Hardcoded.
  char texturebasename[256];	// @@@ Hardcoded.
  unsigned int detail = 3000;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case TOKEN_HEIGHTMAP:
        ScanStr (params, "%s", heightmapname);
        break;
      case TOKEN_TEXTURE:
        ScanStr (params, "%s", texturebasename);
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

  if (!System->VFS->Exists(heightmapname))
  {
    CsPrintf (MSG_FATAL_ERROR, "Error locating height field: %s\n", heightmapname);
    fatal_exit (0, false);
  }

  size_t heightmapsize = 257;
  char* heightmap = System->VFS->ReadFile (heightmapname, heightmapsize);
  if (heightmap == NULL)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error loading height field: %s\n", heightmapname);
    fatal_exit (0, false);
  }

  csTerrain* terr = new csTerrain ();
  terr->SetName (name);

  // Otherwise read file, if that fails generate a random map.
  if (!terr->Initialize (heightmap, heightmapsize))
  {
    delete[] heightmap;
    CsPrintf (MSG_FATAL_ERROR, "Error creating height field from: %s\n", heightmapname);
    fatal_exit (0, false);
  }

  // Initialize all textures. The first texture is reused for all
  // texture entries that fail (cannot be found).
  int num_mat = terr->GetNumMaterials ();
  int i;
  char matname[256];
  csMaterialWrapper* first_mat = NULL;
  for (i = 0 ; i < num_mat ; i++)
  {
    sprintf (matname, texturebasename, i);
    csMaterialWrapper* mat = FindMaterial (matname);
    if (mat == NULL) mat = first_mat;
    first_mat = mat;
    terr->SetMaterial (i, mat);
  }

  terr->SetDetail(detail);
  delete[] heightmap;
  sector.terrains.Push (terr);
}

//---------------------------------------------------------------------------

csSoundDataObject* csLoader::load_sound(char* name, const char* filename)
{
  /* @@@ get the needed plugin interfaces:
   * when moving the loader to a plug-in, this should be done
   * at initialization, and pointers shouldn't be DecRef'ed here.
   * The 'no sound loader' warning should also be printed at
   * initialization.
   * I marked all cases with '###'.
   */

  /* get format descriptor */
  /* ### */iSoundRender *SoundRender = QUERY_PLUGIN(System, iSoundRender);
  /* ### */if (!SoundRender) return NULL;
  const csSoundFormat *Format = SoundRender->GetLoadFormat();
  /* ### */SoundRender->DecRef();

  /* read the file data */
  size_t size;
  char* buf = System->VFS->ReadFile (filename, size);
  if (!buf || !size) {
    CsPrintf (MSG_WARNING,
      "Cannot read sound file \"%s\" from VFS\n", filename);
    return NULL;
  }

  /* ### get sound loader plugin */
  static bool TriedToLoadSound = false;
  iSoundLoader *SoundLoader = QUERY_PLUGIN(System, iSoundLoader);
  if (!SoundLoader) {
    if (!TriedToLoadSound) {
      TriedToLoadSound = true;
      CsPrintf(MSG_WARNING,
        "Trying to load sound without sound loader.\n");
    }
    return NULL;
  }

  /* load the sound */
  iSoundData *Sound = SoundLoader->LoadSound((UByte*)buf, size, Format);
  delete [] buf;
  /* ### */SoundLoader->DecRef();

  /* check for valid sound data */
  if (!Sound) {
    CsPrintf (MSG_WARNING, "The sound file \"%s\" is corrupt!\n", filename);
    return NULL;
  }

  /* build wrapper object */
  csSoundDataObject* sndobj = new csSoundDataObject (Sound);
  sndobj->SetName (name);

  return sndobj;
}

//---------------------------------------------------------------------------

bool csLoader::LoadWorld (char* buf)
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
    TOKEN_TABLE (MATERIALS)
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

    World->SelectLibrary (name);

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
            csSpriteTemplate* t = World->GetSpriteTemplate (name);
            if (!t)
            {
              t = new csSpriteTemplate ();
              t->SetName (name);
              World->sprite_templates.Push (t);
            }
            LoadSpriteTemplate (t, params);
          }
          break;
        case TOKEN_THING:
          if (!World->GetThingTemplate (name))
            World->thing_templates.Push (load_thingtpl (name, params));
          break;
        case TOKEN_SIXFACE:
          if (!World->GetThingTemplate (name))
            World->thing_templates.Push (load_sixtpl (name, params));
          break;
        case TOKEN_SECTOR:
          if (!World->sectors.FindByName (name))
            World->sectors.Push (load_sector (name, params));
          break;
        case TOKEN_PLANE:
          World->planes.Push (load_polyplane (params, name));
          break;
        case TOKEN_COLLECTION:
          World->collections.Push (load_collection (name, params));
          break;
        case TOKEN_SCRIPT:
          CsPrintf (MSG_WARNING, "Warning! SCRIPT statement is obsolete"
                                 " and does not do anything!\n");
          break;
	case TOKEN_TEX_SET:
          if (!LoadTextures (params, name))
            return false;
          break;
        case TOKEN_TEXTURES:
          {
            World->GetTextures ()->DeleteAll ();
            if (!LoadTextures (params))
              return false;
          }
          break;
        case TOKEN_MATERIALS:
          {
            World->GetMaterials ()->DeleteAll ();
            if (!LoadMaterials (params))
              return false;
          }
          break;
        case TOKEN_SOUNDS:
          if (!LoadSounds (params))
            return false;
          break;
        case TOKEN_ROOM:
          // Not an object but it is translated to a special sector.
          if (!World->sectors.FindByName (name))
            World->sectors.Push (load_room (name, params));
          break;
        case TOKEN_LIGHTX:
          CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                                 " and does not do anything!\n");
          break;
        case TOKEN_LIBRARY:
          LoadLibraryFile (World, name);
          break;
        case TOKEN_START:
        {
          char start_sector [100];
          csVector3 pos (0, 0, 0);
          ScanStr (params, "%s,%f,%f,%f", &start_sector, &pos.x, &pos.y, &pos.z);
          World->camera_positions.Push (new csCameraPosition ("Start",
            start_sector, pos, csVector3 (0, 0, 1), csVector3 (0, 1, 0)));
          break;
        }
        case TOKEN_KEY:
          load_key (params, World);
          break;
      }
    }
    if (cmd == PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a world!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  int sn = World->sectors.Length ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)(World->sectors)[sn];
    int st = s->things.Length ();
    int j = -1;
    while (j < st)
    {
      csPolygonSet* ps;
      if (j == -1) ps = s;
      else ps = (csPolygonSet*)(s->things[j]);
      j++;
      for (int i=0;  i < ps->GetNumPolygons ();  i++)
      {
        csPolygon3D* p = ps->GetPolygon3D (i);
        if (p && p->GetPortal ())
        {
          csPortal *portal = p->GetPortal ();
          csSector *stmp = portal->GetSector ();
          csSector *snew = (csSector*)(World->sectors).FindByName(stmp->GetName ());
          if (!snew)
          {
            CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found for portal in"
                      " polygon '%s/%s'!\n", stmp->GetName (),
                      ((csObject*)p->GetParent ())->GetName (),
                      p->GetName ());
            fatal_exit (0, false);
          }
          portal->SetSector (snew);
          delete stmp;
        }
      }
    }
  }

  return true;
}

bool csLoader::LoadWorldFile (csWorld* world, const char* file)
{
  World = world;

  world->StartWorld ();
  csLoaderStat::Init ();

  size_t size;
  char *buf = System->VFS->ReadFile (file, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open world file \"%s\" on VFS!\n", file);
    return false;
  }

  csIniFile* cfg = new csIniFile (System->VFS, "world.cfg");
  if (cfg)
  {
    csLightMap::SetLightCellSize (cfg->GetInt ("Lighting", "LIGHTMAP_SIZE",
    	csLightMap::lightcell_size));
    delete cfg;
  }
  CsPrintf (MSG_INITIALIZATION, "Lightmap grid size = %dx%d.\n",
      csLightMap::lightcell_size, csLightMap::lightcell_size);

  if (!LoadWorld (buf))
    return false;

  if (csLoaderStat::polygons_loaded)
  {
    CsPrintf (MSG_INITIALIZATION, "Loaded world file:\n");
    CsPrintf (MSG_INITIALIZATION, "  %d polygons (%d portals),\n", csLoaderStat::polygons_loaded,
      csLoaderStat::portals_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d sectors, %d things, %d sprites, \n", csLoaderStat::sectors_loaded,
      csLoaderStat::things_loaded, csLoaderStat::sprites_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d curves, %d lights, %d sounds.\n", csLoaderStat::curves_loaded,
      csLoaderStat::lights_loaded, csLoaderStat::sounds_loaded);
  } /* endif */

  delete [] buf;

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadTextures (char* buf, const char* prefix)
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
        txt_process (name, params, prefix);
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

bool csLoader::LoadMaterials (char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MATERIAL)
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
      case TOKEN_MATERIAL:
        mat_process (name, params);
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

bool csLoader::LoadLibrary (char* buf)
{
  TOKEN_TABLE_START (tokens)
    TOKEN_TABLE (LIBRARY)
  TOKEN_TABLE_END

  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXTURES)
    TOKEN_TABLE (MATERIALS)
    TOKEN_TABLE (THING)
    TOKEN_TABLE (SPRITE)
    TOKEN_TABLE (SOUNDS)
    TOKEN_TABLE (PLANE)
  TOKEN_TABLE_END

  char *name, *data;
  if (csGetObject (&buf, tokens, &name, &data))
  {
    // Empty LIBRARY () directive?
    if (!data)
      return false;

    long cmd;
    char* params;

    World->SelectLibrary (name);

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", data);
        return false;
      }

      switch (cmd)
      {
        case TOKEN_PLANE:
          World->planes.Push ( load_polyplane (params, name) );
          break;
        case TOKEN_TEXTURES:
          // Append textures to world.
          if (!LoadTextures (params))
            return false;
          break;
        case TOKEN_MATERIALS:
          if (!LoadMaterials (params))
            return false;
          break;
        case TOKEN_SOUNDS:
          if (!LoadSounds (params))
            return false;
          break;
        case TOKEN_SPRITE:
          {
            csSpriteTemplate* t = World->GetSpriteTemplate (name);
            if (!t)
            {
              t = new csSpriteTemplate ();
              t->SetName (name);
              World->sprite_templates.Push (t);
            }
            LoadSpriteTemplate (t, params);
          }
          break;
        case TOKEN_THING:
          if (!World->GetThingTemplate (name))
            World->thing_templates.Push (load_thingtpl (name, params));
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

  World = world;
  bool retcode = LoadLibrary (buf);

  delete [] buf;

  return retcode;
}

csTextureWrapper* csLoader::LoadTexture (csWorld* world, const char* name, const char* fname)
{
  World = world;
  iImage *image = load_image (fname);
  if (!image)
    return NULL;
  csTextureWrapper *th = world->GetTextures ()->NewTexture (image);
  th->SetName (name);
  // dereference image pointer since th already incremented it
  image->DecRef ();

  csMaterial* material = new csMaterial ();
  csMaterialWrapper* mat = World->GetMaterials ()->NewMaterial (material);
  mat->SetName (name);
  material->SetTextureWrapper (th);
  material->DecRef ();

  return th;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSounds (char* buf)
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
        iSoundData *snd = csSoundDataObject::GetSound (*World, name);
        if (!snd)
        {
          csSoundDataObject *s = load_sound (name, filename);
          if (s)
          {
            World->ObjAdd(s);
            csLoaderStat::sounds_loaded++;
          }
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
          csSkeletonConnection* con = new csSkeletonConnection ();
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
                load_matrix (params2, m);
		break;
              case TOKEN_V:
                load_vector (params2, v);
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

csSpriteTemplate* csLoader::LoadSpriteTemplate (csWorld* world,
	const char* fname)
{
  World = world;

  size_t size;
  char *buf = System->VFS->ReadFile (fname, size);

  if (!buf || !size)
  {
    CsPrintf (MSG_FATAL_ERROR, "Could not open sprite template file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  TOKEN_TABLE_START (tokens)
    TOKEN_TABLE (SPRITE)
  TOKEN_TABLE_END

  char *name, *data;

  if (csGetObject (&buf, tokens, &name, &data))
  {
    if (!data)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }

    csSpriteTemplate* tmpl = new csSpriteTemplate ();
    tmpl->SetName (name);
    if (LoadSpriteTemplate (tmpl, data))
    {
      World->sprite_templates.Push (tmpl);
      return tmpl;
    }
    else
    {
      delete tmpl;
      return NULL;
    }
  }
  return NULL;
}

bool csLoader::LoadSpriteTemplate (csSpriteTemplate* stemp, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
    TOKEN_TABLE (FRAME)
    TOKEN_TABLE (ACTION)
    TOKEN_TABLE (MERGE_NORMALS)
    TOKEN_TABLE (MERGE_TEXELS)
    TOKEN_TABLE (MERGE_VERTICES)
    TOKEN_TABLE (TRIANGLE)
    TOKEN_TABLE (SKELETON)
    TOKEN_TABLE (FILE)
    TOKEN_TABLE (TWEEN)
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
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        {
          ScanStr (params, "%s", str);
          csMaterialWrapper *mat = FindMaterial (str);
          if (mat)
            stemp->SetMaterial (mat);
          else
          {
            CsPrintf (MSG_FATAL_ERROR, "Material `%s' not found!\n", str);
            fatal_exit (0, true);
          }
        }
        break;

      case TOKEN_SKELETON:
	{
          csSkeleton* skeleton = new csSkeleton ();
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
          int anm_idx = fr->GetAnmIndex ();
          int tex_idx = fr->GetTexIndex ();
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
                  stemp->AddVertex();
                }
                else if (i >= stemp->GetNumTexels ())
                {
                  CsPrintf (MSG_FATAL_ERROR, "Error! Trying to add too many vertices in frame '%s'!\n",
                    fr->GetName ());
                  fatal_exit (0, false);
                }
                stemp->GetVertex (anm_idx, i) = csVector3 (x, y, z);
                stemp->GetTexel  (tex_idx, i) = csVector2 (u, v);
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
          if (i < stemp->GetNumTexels ())
          {
            CsPrintf (MSG_FATAL_ERROR, "Error! Too few vertices in frame '%s'! (%d %d)\n",
                fr->GetName (), i, stemp->GetNumTexels ());
            fatal_exit (0, false);
          }
        }
        break;

      case TOKEN_TRIANGLE:
        {
          int a, b, c;
          ScanStr (params, "%d,%d,%d", &a, &b, &c);
          stemp->AddTriangle (a, b, c);
        }
        break;

      case TOKEN_FILE:
        {
          ScanStr (params, "%s", str);
	  converter* filedata = new converter;
	  if (filedata->ivcon (str, true, false, NULL, System->VFS) == ERROR)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Error loading file model '%s'!\n", str);
	    delete filedata;
	    fatal_exit (0, false);
	  }
	  csCrossBuild_SpriteTemplateFactory builder;
	  builder.CrossBuild (stemp, *filedata);
	  delete filedata;
        }
        break;

      case TOKEN_MERGE_NORMALS:
        {
	  CsPrintf (MSG_WARNING, "MERGE_NORMALS is obsolete.\n");
        }
        break;

      case TOKEN_MERGE_TEXELS:
        {
	  CsPrintf (MSG_WARNING, "MERGE_TEXELS is obsolete.\n");
        }
        break;

      case TOKEN_MERGE_VERTICES:
        {
	  CsPrintf (MSG_WARNING, "MERGE_VERTICES is obsolete.\n");
        }
        break;

      case TOKEN_TWEEN:
	{
	  bool do_tween;
          ScanStr (params, "%b", &do_tween);
          stemp->EnableTweening (do_tween);
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

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSprite (csSprite2D* spr, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (VERTICES)
    TOKEN_TABLE (UV)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
    TOKEN_TABLE (MIXMODE)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (COLORS)
    TOKEN_TABLE (LIGHTING)
  TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  int i;

  csLoaderStat::sprites_loaded++;

  csColoredVertices& verts = spr->GetVertices ();
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
      case TOKEN_MIXMODE:
        spr->SetMixmode (ParseMixmode (params));
        break;
      case TOKEN_VERTICES:
        {
          float list[100];
	  int num;
          ScanStr (params, "%F", list, &num);
	  num /= 2;
	  verts.SetLength (num);
          for (i = 0 ; i < num ; i++)
	  {
	    verts[i].pos.x = list[i*2+0];
	    verts[i].pos.y = list[i*2+1];
	    verts[i].color_init.Set (0, 0, 0);
	    verts[i].color.Set (0, 0, 0);
	  }
        }
        break;
      case TOKEN_UV:
        {
          float list[100];
	  int num;
          ScanStr (params, "%F", list, &num);
	  num /= 2;
	  verts.SetLength (num);
          for (i = 0 ; i < num ; i++)
	  {
	    verts[i].u = list[i*2+0];
	    verts[i].v = list[i*2+1];
	  }
        }
        break;
      case TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          spr->SetLighting (do_lighting);
        }
        break;
      case TOKEN_COLORS:
        {
          float list[100];
	  int num;
          ScanStr (params, "%F", list, &num);
	  num /= 3;
	  verts.SetLength (num);
          for (i = 0 ; i < num ; i++)
	  {
	    verts[i].color_init.red = list[i*3+0];
	    verts[i].color_init.green = list[i*3+1];
	    verts[i].color_init.blue = list[i*3+2];
	  }
        }
        break;
      case TOKEN_MOVE:
        {
	  float x, y, z;
	  ScanStr (params, "%f,%f,%f", &x, &y, &z);
          spr->SetPosition (csVector3 (x, y, z));
	}
        break;

      case TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        {
          ScanStr (params, "%s", str);
          csMaterialWrapper* mat = FindMaterial (str);
          if (mat == NULL)
          {
            CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", params);
            fatal_exit (0, true);
          }
          spr->SetMaterial (mat);
	}
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the sprite!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadSprite (csSprite3D* spr, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (MIXMODE)
    TOKEN_TABLE (TEMPLATE)
    TOKEN_TABLE (TEXNR)
    TOKEN_TABLE (MATERIAL)
    TOKEN_TABLE (MOVE)
    TOKEN_TABLE (TWEEN)
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
      case TOKEN_MIXMODE:
        spr->SetMixmode (ParseMixmode (params));
        break;
      case TOKEN_MOVE:
        {
          char* params2;
          spr->GetMovable ().SetTransform (csMatrix3 ());     // Identity matrix.
          spr->GetMovable ().SetPosition (csVector3 (0, 0, 0));
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
              {
                csMatrix3 m;
                load_matrix (params2, m);
                spr->GetMovable ().SetTransform (m);
                break;
              }
              case TOKEN_V:
              {
                csVector3 v;
                load_vector (params2, v);
                spr->GetMovable ().SetPosition (v);
                break;
              }
            }
          }
	  spr->UpdateMove ();
        }
        break;

      case TOKEN_TEMPLATE:
        memset (str, 0, 255);
        memset (str2, 0, 255);
        ScanStr (params, "%s,%s", str, str2);
        tpl = World->GetSpriteTemplate (str);
        if (tpl == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find sprite template '%s'!\n", str);
          fatal_exit (0, true);
        }
        spr->SetTemplate (tpl);
        if (tpl->FindAction (str2) != NULL)
          spr->SetAction (str2);
        break;

      case TOKEN_TWEEN:
	{
	  bool do_tween;
          ScanStr (params, "%b", &do_tween);
          spr->EnableTweening (do_tween);
	}
	break;

      case TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        FindMaterial (str);
        csMaterialWrapper *mat = FindMaterial (str);
        if (!mat)
        {
          CsPrintf (MSG_FATAL_ERROR, "No material named `%s' found!\n", str);
          fatal_exit (0, true);
        }
        else
          spr->SetMaterial (mat);
        // unset_texture ();
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the sprite!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  spr->InitSprite ();
  return true;
}

csFrame* csLoader::LoadFrame (csSpriteTemplate* stemp, char* buf)
{
  TOKEN_TABLE_START (commands)
    TOKEN_TABLE (ACTION)
    TOKEN_TABLE (FRAME)
  TOKEN_TABLE_END

  long cmd;
  char* name;
  char* params;
  char action[255];
  bool action_specified = false;
  int frame = 0;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case TOKEN_ACTION:
        ScanStr (params, "%s", action);
        action_specified = true;
        break;
      case TOKEN_FRAME:
        ScanStr (params, "%d", &frame);
        break;
    }
  }
  if (cmd == PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while getting a frame!\n",
      csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (action_specified)
    return (stemp->FindAction(action))->GetFrame(frame);
  else
    return stemp->GetFrame(frame);
}

//---------------------------------------------------------------------------
