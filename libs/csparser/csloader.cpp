/*
    Copyright (C) 1998,2000 by Ivan Avramovic <ivan@avramovic.com>

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
#include "csengine/cssprite.h"
#include "csengine/meshobj.h"
#include "csengine/csspr2d.h"
#include "csengine/skeleton.h"
#include "csengine/polygon.h"
#include "csengine/polytmap.h"
#include "csengine/textrans.h"
#include "csengine/engine.h"
#include "csengine/light.h"
#include "csengine/texture.h"
#include "csengine/curve.h"
#include "csengine/terrain.h"
#include "csengine/terrddg.h"
#include "csengine/terrlod.h"
#include "csengine/dumper.h"
#include "csengine/keyval.h"
#include "csengine/particle.h"
#include "csengine/region.h"
#include "csengine/halo.h"
#include "csfx/partexp.h"
#include "csfx/partfire.h"
#include "csfx/partspir.h"
#include "csfx/partsnow.h"
#include "csfx/partfoun.h"
#include "csfx/partrain.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/token.h"
#include "csutil/inifile.h"
#include "csutil/util.h"
#include "cssys/system.h"
#include "csgfxldr/csimage.h"
#include "ivfs.h"
#include "idatabuf.h"
#include "isndldr.h"
#include "isnddata.h"
#include "isndrdr.h"
#include "itxtmgr.h"
#include "imotion.h"
#include "ildrplug.h"

typedef char ObName[30];
/// The engine we are currently processing
static csEngine* Engine;
/// Loader flags
static int flags = 0;
/// If true the we only load in the current region
bool onlyRegion = false;

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
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ACCEL)
  CS_TOKEN_DEF (ACTION)
  CS_TOKEN_DEF (ACTIVATE)
  CS_TOKEN_DEF (ACTIVE)
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (AFFECTOR)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (AMBIENT)
  CS_TOKEN_DEF (ANIM)
  CS_TOKEN_DEF (ATTENUATION)
  CS_TOKEN_DEF (AZIMUTH)
  CS_TOKEN_DEF (BECOMING_ACTIVE)
  CS_TOKEN_DEF (BECOMING_INACTIVE)
  CS_TOKEN_DEF (BEZIER)
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (BSP)
  CS_TOKEN_DEF (CAMERA)
  CS_TOKEN_DEF (CEILING)
  CS_TOKEN_DEF (CEIL_TEXTURE)
  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (CIRCLE)
  CS_TOKEN_DEF (CLIP)
  CS_TOKEN_DEF (COLLDET)
  CS_TOKEN_DEF (COLLECTION)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (COLORSCALE)
  CS_TOKEN_DEF (CONVEX)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (COSFACT)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (CURVESCALE)
  CS_TOKEN_DEF (DETAIL)
  CS_TOKEN_DEF (DIFFUSE)
  CS_TOKEN_DEF (DIM)
  CS_TOKEN_DEF (DITHER)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (DYNAMIC)
  CS_TOKEN_DEF (ELEVATION)
  CS_TOKEN_DEF (EULER)
  CS_TOKEN_DEF (F)
  CS_TOKEN_DEF (FALLTIME)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FILTER)
  CS_TOKEN_DEF (FIRE)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FLAT)
  CS_TOKEN_DEF (FLATCOL)
  CS_TOKEN_DEF (FLOOR)
  CS_TOKEN_DEF (FLOOR_CEIL)
  CS_TOKEN_DEF (FLOOR_HEIGHT)
  CS_TOKEN_DEF (FLOOR_TEXTURE)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (FOR_2D)
  CS_TOKEN_DEF (FOR_3D)
  CS_TOKEN_DEF (FOUNTAIN)
  CS_TOKEN_DEF (FRAME)
  CS_TOKEN_DEF (GOURAUD)
  CS_TOKEN_DEF (HALO)
  CS_TOKEN_DEF (HARDMOVE)
  CS_TOKEN_DEF (HEIGHT)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (INCLUDESPRITE)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (LEN)
  CS_TOKEN_DEF (LIBRARY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (LIGHTMAP)
  CS_TOKEN_DEF (LIGHTX)
  CS_TOKEN_DEF (LIMB)
  CS_TOKEN_DEF (LINK)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATERIALS)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MAX_TEXTURES)
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (MIPMAP)
  CS_TOKEN_DEF (MIRROR)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (MOVE)
  CS_TOKEN_DEF (MOVEABLE)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (NODE)
  CS_TOKEN_DEF (NONE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (OPENING)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (PERSISTENT)
  CS_TOKEN_DEF (PLANE)
  CS_TOKEN_DEF (PLUGIN)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (PORTAL)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PRIMARY_ACTIVE)
  CS_TOKEN_DEF (PRIMARY_INACTIVE)
  CS_TOKEN_DEF (PROCEDURAL)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (RAIN)
  CS_TOKEN_DEF (REFLECTION)
  CS_TOKEN_DEF (REGION)
  CS_TOKEN_DEF (ROOM)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SCRIPT)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SECONDARY_ACTIVE)
  CS_TOKEN_DEF (SECONDARY_INACTIVE)
  CS_TOKEN_DEF (SECOND_LEN)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (SHADING)
  CS_TOKEN_DEF (SIXFACE)
  CS_TOKEN_DEF (SKELETON)
  CS_TOKEN_DEF (SKY)
  CS_TOKEN_DEF (SKYDOME)
  CS_TOKEN_DEF (SMOOTH)
  CS_TOKEN_DEF (SNOW)
  CS_TOKEN_DEF (SOUND)
  CS_TOKEN_DEF (SOUNDS)
  CS_TOKEN_DEF (SPEED)
  CS_TOKEN_DEF (SPLIT)
  CS_TOKEN_DEF (SPRITE)
  CS_TOKEN_DEF (SPRITE2D)
  CS_TOKEN_DEF (START)
  CS_TOKEN_DEF (STATBSP)
  CS_TOKEN_DEF (STATELESS)
  CS_TOKEN_DEF (STATIC)
  CS_TOKEN_DEF (SWIRL)
  CS_TOKEN_DEF (CLONE)
  CS_TOKEN_DEF (TEMPLATE)
  CS_TOKEN_DEF (TERRAIN)
  CS_TOKEN_DEF (TEX)
  CS_TOKEN_DEF (TEXLEN)
  CS_TOKEN_DEF (TEXNR)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TEXTURES)
  CS_TOKEN_DEF (TEXTURE_LIGHTING)
  CS_TOKEN_DEF (TEXTURE_MIPMAP)
  CS_TOKEN_DEF (TEXTURE_SCALE)
  CS_TOKEN_DEF (MAT_SET)
  CS_TOKEN_DEF (MAT_SET_SELECT)
  CS_TOKEN_DEF (MOTION)
  CS_TOKEN_DEF (Q)
  CS_TOKEN_DEF (THING)
  CS_TOKEN_DEF (TOTALTIME)
  CS_TOKEN_DEF (TRANSFORM)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (TRIANGLE)
  CS_TOKEN_DEF (TRIGGER)
  CS_TOKEN_DEF (TWEEN)
  CS_TOKEN_DEF (TYPE)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (UVA)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (UV_SHIFT)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (WARP)
  CS_TOKEN_DEF (WORLD)
  CS_TOKEN_DEF (ZFILL)
CS_TOKEN_DEF_END

//---------------------------------------------------------------------------

void csLoader::SetMode (int iFlags)
{
  flags = iFlags;
}

bool csLoader::load_matrix (char* buf, csMatrix3 &m)
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
	  CsPrintf (MSG_WARNING, "Badly formed rotation: '%s'\n", params);
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
	  CsPrintf (MSG_WARNING, "Badly formed scale: '%s'\n", params);
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
      CsPrintf (MSG_WARNING, "Badly formed matrix '%s'\n", buf);
  }
  return true;
}

bool csLoader::load_vector (char* buf, csVector3 &v)
{
  ScanStr (buf, "%f,%f,%f", &v.x, &v.y, &v.z);
  return true;
}

bool csLoader::load_quaternion (char* buf, csQuaternion &q)
{
  ScanStr (buf, "%f,%f,%f,%f", &q.x, &q.y, &q.z, &q.r);
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

csMaterialWrapper *csLoader::FindMaterial (const char *iName, bool onlyRegion)
{
  csMaterialWrapper *mat = Engine->FindCsMaterial (iName, onlyRegion);
  if (mat)
    return mat;

  csTextureWrapper *tex = Engine->FindCsTexture (iName, onlyRegion);
  if (tex)
  {
    // Add a default material with the same name as the texture
    csMaterial *material = new csMaterial ();
    material->SetTextureWrapper (tex);
    csMaterialWrapper *mat = Engine->GetMaterials ()->NewMaterial (material);
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
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (FIRST_LEN)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (SECOND_LEN)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

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
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SPRITE)
    CS_TOKEN_TABLE (COLLECTION)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (TRIGGER)
    CS_TOKEN_TABLE (SECTOR)
  CS_TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;

  csCollection* collection = new csCollection (Engine);
  collection->SetName (name);

  char str[255];
  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    str[0] = 0;
    switch (cmd)
    {
      case CS_TOKEN_THING:
        {
          ScanStr (params, "%s", str);
	  iThing* th = Engine->FindThing (str, onlyRegion);
          if (!th)
          {
            CsPrintf (MSG_FATAL_ERROR, "Thing '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          collection->AddObject ((csObject*)(th->GetPrivateObject ()));
        }
        break;
      case CS_TOKEN_SPRITE:
        {
          ScanStr (params, "%s", str);
	  iSprite* spr = Engine->FindSprite (str, onlyRegion);
          if (!spr)
          {
            CsPrintf (MSG_FATAL_ERROR, "Sprite '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          collection->AddObject ((csObject*)(spr->GetPrivateObject ()));
        }
        break;
      case CS_TOKEN_LIGHT:
        {
          ScanStr (params, "%s", str);
	  csStatLight* l = Engine->FindLight (str);
          if (!l)
            CsPrintf (MSG_WARNING, "Light '%s' not found!\n", str);
	  else
	    collection->AddObject ((csObject*)l);
        }
        break;
      case CS_TOKEN_SECTOR:
        {
          ScanStr (params, "%s", str);
	  iSector* s = Engine->FindSector (str, onlyRegion);
          if (!s)
          {
            CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          collection->AddObject ((csObject*)(s->GetPrivateObject ()));
        }
        break;
      case CS_TOKEN_COLLECTION:
        {
          ScanStr (params, "%s", str);
	  //@@@$$$ TODO: Collection in regions.
          csCollection* th = (csCollection*)Engine->collections.FindByName (str);
          if (!th)
          {
            CsPrintf (MSG_FATAL_ERROR, "Collection '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          collection->AddObject (th);
        }
        break;
      case CS_TOKEN_TRIGGER:
        CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a collection!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return collection;
}

//---------------------------------------------------------------------------

UInt ParseMixmode (char* buf)
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
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
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
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
  return Mixmode;
}

//---------------------------------------------------------------------------

csParticleSystem* csLoader::load_fountain (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (ACCEL)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (FALLTIME)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (OPENING)
    CS_TOKEN_TABLE (AZIMUTH)
    CS_TOKEN_TABLE (ELEVATION)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case CS_TOKEN_TEXTURE: //@@@MAT
        ScanStr (params, "%s", str);
        material = FindMaterial (str, onlyRegion);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case CS_TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case CS_TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case CS_TOKEN_ORIGIN:
        ScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
        break;
      case CS_TOKEN_ACCEL:
        ScanStr (params, "%f,%f,%f", &accel.x, &accel.y, &accel.z);
        break;
      case CS_TOKEN_ELEVATION:
        ScanStr (params, "%f", &elevation);
        break;
      case CS_TOKEN_AZIMUTH:
        ScanStr (params, "%f", &azimuth);
        break;
      case CS_TOKEN_OPENING:
        ScanStr (params, "%f", &opening);
        break;
      case CS_TOKEN_SPEED:
        ScanStr (params, "%f", &speed);
        break;
      case CS_TOKEN_FALLTIME:
        ScanStr (params, "%f", &fall_time);
        break;
      case CS_TOKEN_COLOR:
        ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
        break;
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
  	Engine, number, material, mixmode, lighted_particles, drop_width,
	drop_height, origin, accel, fall_time, speed, opening,
	azimuth, elevation);
  partsys->SetName (name);
  partsys->SetColor (color);
  return partsys;
}

csParticleSystem* csLoader::load_fire (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (COLORSCALE)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (SWIRL)
    CS_TOKEN_TABLE (TOTALTIME)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case CS_TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str, onlyRegion);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case CS_TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case CS_TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case CS_TOKEN_ORIGIN:
        ScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
        break;
      case CS_TOKEN_SWIRL:
        ScanStr (params, "%f", &swirl);
        break;
      case CS_TOKEN_TOTALTIME:
        ScanStr (params, "%f", &total_time);
        break;
      case CS_TOKEN_COLORSCALE:
        ScanStr (params, "%f", &colorscale);
        break;
      case CS_TOKEN_SPEED:
        ScanStr (params, "%f,%f,%f", &speed.x, &speed.y, &speed.z);
        break;
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
  	Engine, number, material, mixmode, lighted_particles, drop_width,
	drop_height, total_time, speed, origin, swirl, colorscale);
  partsys->SetName (name);
  return partsys;
}

csParticleSystem* csLoader::load_rain (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case CS_TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str, onlyRegion);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case CS_TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case CS_TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case CS_TOKEN_BOX:
      {
        float x1, y1, z1, x2, y2, z2;
        ScanStr (params, "%f,%f,%f,%f,%f,%f", &x1, &y1, &z1, &x2, &y2, &z2);
	box.Set (x1, y1, z1, x2, y2, z2);
        break;
      }
      case CS_TOKEN_SPEED:
        ScanStr (params, "%f,%f,%f", &speed.x, &speed.y, &speed.z);
        break;
      case CS_TOKEN_COLOR:
        ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
        break;
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
  	Engine, number, material, mixmode, lighted_particles, drop_width,
	drop_height, box.Min (), box.Max (), speed);
  partsys->SetName (name);
  partsys->SetColor (color);
  return partsys;
}

csParticleSystem* csLoader::load_snow (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (SWIRL)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_NUMBER:
        ScanStr (params, "%d", &number);
        break;
      case CS_TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str, onlyRegion);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_MIXMODE:
        mixmode = ParseMixmode (params);
        break;
      case CS_TOKEN_LIGHTING:
        ScanStr (params, "%b", &lighted_particles);
        break;
      case CS_TOKEN_DROPSIZE:
        ScanStr (params, "%f,%f", &drop_width, &drop_height);
	break;
      case CS_TOKEN_SWIRL:
        ScanStr (params, "%f", &swirl);
	break;
      case CS_TOKEN_BOX:
      {
        float x1, y1, z1, x2, y2, z2;
        ScanStr (params, "%f,%f,%f,%f,%f,%f", &x1, &y1, &z1, &x2, &y2, &z2);
	box.Set (x1, y1, z1, x2, y2, z2);
        break;
      }
      case CS_TOKEN_SPEED:
        ScanStr (params, "%f,%f,%f", &speed.x, &speed.y, &speed.z);
        break;
      case CS_TOKEN_COLOR:
        ScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
        break;
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
  	Engine, number, material, mixmode, lighted_particles, drop_width,
	drop_height, box.Min (), box.Max (), speed, swirl);
  partsys->SetName (name);
  partsys->SetColor (color);
  return partsys;
}

csStatLight* csLoader::load_statlight (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (ATTENUATION)
    CS_TOKEN_TABLE (CENTER)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (DYNAMIC)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (HALO)
  CS_TOKEN_TABLE_END

  long cmd;
  char* params;

  csLoaderStat::lights_loaded++;
  float x, y, z, dist = 0, r, g, b;
  int cnt;
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
        case CS_TOKEN_RADIUS:
          ScanStr (params, "%f", &dist);
          break;
        case CS_TOKEN_CENTER:
          ScanStr (params, "%f,%f,%f", &x, &y, &z);
          break;
        case CS_TOKEN_COLOR:
          ScanStr (params, "%f,%f,%f", &r, &g, &b);
          break;
        case CS_TOKEN_DYNAMIC:
          dyn = 1;
          break;
        case CS_TOKEN_HALO:
	  str[0] = 0;
          cnt = ScanStr (params, "%s", str);
          if (cnt == 0 || !strncmp (str, "CROSS", 5))
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
        case CS_TOKEN_ATTENUATION:
          ScanStr (params, "%s", str);
          if (strcmp (str, "none")      == 0) attenuation = CS_ATTN_NONE;
          if (strcmp (str, "linear")    == 0) attenuation = CS_ATTN_LINEAR;
          if (strcmp (str, "inverse")   == 0) attenuation = CS_ATTN_INVERSE;
          if (strcmp (str, "realistic") == 0) attenuation = CS_ATTN_REALISTIC;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
    	csGetParserLine());
    fatal_exit (0, false);
    return NULL;
  }
}

csMapNode* csLoader::load_node (char* name, char* buf, csSector* sec)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (POSITION)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_KEY:
        load_key(params, pNode);
        break;
      case CS_TOKEN_POSITION:
        ScanStr (params, "%f,%f,%f", &x, &y, &z);
        break;
      default:
        abort ();
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char str[255];
  char* xname;
  switch (cmd)
  {
    case CS_TOKEN_VERTEX:
      {
        float x, y, z;
        ScanStr (params, "%f,%f,%f", &x, &y, &z);
        ps.AddVertex (x, y, z);
      }
      break;
    case CS_TOKEN_CIRCLE:
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
    case CS_TOKEN_FOG:
      {
        csFog& f = ps.GetFog ();
        f.enabled = true;
        ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
      }
      break;
    case CS_TOKEN_POLYGON:
      {
	csPolygon3D* poly3d = load_poly3d (name, params,
          info.default_material, info.default_texlen, &ps);
	if (poly3d)
	{
	  ps.AddPolygon (poly3d);
	  csLoaderStat::polygons_loaded++;
	}
      }
      break;

    case CS_TOKEN_HARDMOVE:
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
            case CS_TOKEN_MATRIX:
            {
              csMatrix3 m;
              load_matrix (params2, m);
              info.hard_trans.SetT2O (m);
	      info.do_hard_trans = true;
              break;
            }
            case CS_TOKEN_V:
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

    case CS_TOKEN_BEZIER:
      {
        csCurveTemplate* ct = load_beziertemplate (name, params,
	    info.default_material, info.default_texlen,
	    ps.GetCurveVertices ());
	Engine->curve_templates.Push (ct);
	csCurve* p = ct->MakeCurve ();
	p->SetName (ct->GetName ());
	p->SetParent (&ps);
	p->SetSector (sector);
        if (!ct->GetMaterialWrapper ()) p->SetMaterialWrapper (info.default_material);
	int j;
        for (j = 0 ; j < ct->NumVertices () ; j++)
          p->SetControlPoint (j, ct->GetVertex (j));
	ps.AddCurve (p);
      }
      break;

    case CS_TOKEN_CURVECENTER:
      {
        csVector3 c;
        ScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
        ps.curves_center = c;
      }
      break;
    case CS_TOKEN_CURVESCALE:
      ScanStr (params, "%f", &ps.curves_scale);
      break;

    case CS_TOKEN_CURVECONTROL:
      {
        csVector3 v;
        csVector2 t;
        ScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
        ps.AddCurveVertex (v, t);
      }
      break;

    case CS_TOKEN_TEXNR:
      //@@OBSOLETE, retained for backward compatibility
    case CS_TOKEN_MATERIAL:
      ScanStr (params, "%s", str);
      info.default_material = FindMaterial (str, onlyRegion);
      if (info.default_material == NULL)
      {
        CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
        fatal_exit (0, true);
      }
      break;
    case CS_TOKEN_TEXLEN:
      ScanStr (params, "%f", &info.default_texlen);
      break;
    case CS_TOKEN_MAT_SET_SELECT:
      ScanStr(params, "%s", str);
      info.SetTextureSet( str );
      info.use_mat_set=true;
      break;
    case CS_TOKEN_LIGHTX:
      CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                               " and does not do anything!\n");
      break;
    case CS_TOKEN_ACTIVATE:
      CsPrintf (MSG_WARNING, "Warning! ACTIVATE statement is obsolete"
                                 " and does not do anything!\n");
      break;
    case CS_TOKEN_TRIGGER:
      CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
      break;
    case CS_TOKEN_BSP:
      CsPrintf (MSG_FATAL_ERROR,
        "BSP keyword is no longer supported. Use STATBSP instead after putting\n\
all non-convex polygons in things.\n");
      break;
  }
  return ps;
}

//---------------------------------------------------------------------------

csThing* csLoader::load_sixface (char* name, char* buf, csSector* sec,
    	bool is_template)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (TEXTURE_SCALE)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (CEIL_TEXTURE)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (DIM)
    CS_TOKEN_TABLE (HEIGHT)
    CS_TOKEN_TABLE (FLOOR_HEIGHT)
    CS_TOKEN_TABLE (FLOOR_CEIL)
    CS_TOKEN_TABLE (FLOOR_TEXTURE)
    CS_TOKEN_TABLE (FLOOR)
    CS_TOKEN_TABLE (CEILING)
    CS_TOKEN_TABLE (TRIGGER)
    CS_TOKEN_TABLE (ACTIVATE)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (CONVEX)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* xname;

  csThing* thing = new csThing (Engine, false, is_template);
  thing->SetName (name);

  csLoaderStat::things_loaded++;

  if (sec) thing->GetMovable ().SetSector (sec);
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
      case CS_TOKEN_CONVEX:
        is_convex = true;
        break;
      case CS_TOKEN_FOG:
        {
          csFog& f = thing->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case CS_TOKEN_MOVEABLE:
        thing->flags.Set (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
        break;
      case CS_TOKEN_DETAIL:
        thing->flags.Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
        break;
      case CS_TOKEN_MOVE:
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
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                load_matrix (params2, m);
                obj.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
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
      case CS_TOKEN_TEXTURE: //@@@MAT
        ScanStr (params, "%s", str);
        material = FindMaterial (str, onlyRegion);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_TEXTURE_SCALE:
        ScanStr (params, "%f", &tscale);
        break;
      case CS_TOKEN_DIM:
        {
          float rx, ry, rz;
          ScanStr (params, "%f,%f,%f", &rx, &ry, &rz);
          rx /= 2; ry /= 2; rz /= 2;
          for (i = 0;  i < 8;  i++)
           v[i] = csVector3((i&1 ? rx : -rx),(i&2 ? -ry : ry),(i&4 ? -rz : rz));
        }
        break;
      case CS_TOKEN_FLOOR_HEIGHT:
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
      case CS_TOKEN_HEIGHT:
        ScanStr (params, "%f", &r);
        v[0].y = r+v[2].y;
        v[1].y = r+v[3].y;
        v[4].y = r+v[6].y;
        v[5].y = r+v[7].y;
        break;
      case CS_TOKEN_FLOOR_CEIL:
        ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
                 &v[2].x, &v[2].z, &v[3].x, &v[3].z,
                 &v[7].x, &v[7].z, &v[6].x, &v[6].z);
        v[0] = v[2];
        v[1] = v[3];
        v[5] = v[7];
        v[4] = v[6];
        break;
      case CS_TOKEN_FLOOR:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v[2].x, &v[2].y, &v[2].z, &v[3].x, &v[3].y, &v[3].z,
                 &v[7].x, &v[7].y, &v[7].z, &v[6].x, &v[6].y, &v[6].z);
        break;
      case CS_TOKEN_CEILING:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v[0].x, &v[0].y, &v[0].z, &v[1].x, &v[1].y, &v[1].z,
                 &v[5].x, &v[5].y, &v[5].z, &v[4].x, &v[4].y, &v[4].z);
        break;
      case CS_TOKEN_KEY:
        load_key (params, thing);
        break;
      case CS_TOKEN_ACTIVATE:
        CsPrintf (MSG_WARNING, "Warning! ACTIVATE statement is obsolete"
                                 " and does not do anything!\n");
        break;
      case CS_TOKEN_TRIGGER:
        CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
    thing->GetMovable ().UpdateMove ();

  return thing;
}

csThing* csLoader::load_thing (char* name, char* buf, csSector* sec,
    bool is_sky, bool is_template)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (CIRCLE)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (BEZIER)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVESCALE)
    CS_TOKEN_TABLE (CURVECONTROL)
    CS_TOKEN_TABLE (MAT_SET_SELECT)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (TEXLEN)
    CS_TOKEN_TABLE (TRIGGER)
    CS_TOKEN_TABLE (ACTIVATE)
    CS_TOKEN_TABLE (LIGHTX)
    CS_TOKEN_TABLE (BSP)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (CONVEX)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (HARDMOVE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (CLONE)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (CAMERA)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* xname;

  csThing* thing = new csThing (Engine, is_sky, is_template);
  thing->SetName (name);

  csLoaderStat::things_loaded++;
  PSLoadInfo info;
  if (sec) thing->GetMovable ().SetSector (sec);

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
      case CS_TOKEN_MOVEABLE:
        thing->flags.Set (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
        break;
      case CS_TOKEN_DETAIL:
        thing->flags.Set (CS_ENTITY_DETAIL, CS_ENTITY_DETAIL);
        break;
      case CS_TOKEN_CONVEX:
        is_convex = true;
        break;
      case CS_TOKEN_CAMERA:
        thing->flags.Set (CS_ENTITY_CAMERA, CS_ENTITY_CAMERA);
        break;
      case CS_TOKEN_MOVE:
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
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                load_matrix (params2, m);
                obj.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
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
      case CS_TOKEN_TEMPLATE:
        {
          ScanStr (params, "%s", str);
          csThing* t = (csThing*)Engine->thing_templates.FindByName (str);
          if (!t)
          {
            CsPrintf (MSG_FATAL_ERROR, "Couldn't find thing template '%s'!\n", str);
            fatal_exit (0, false);
          }
	  if (info.use_mat_set)
          {
            thing->MergeTemplate (t, sec, Engine->GetMaterials (), info.mat_set_name,
              info.default_material, info.default_texlen, false);
            info.use_mat_set = false;
	  }
          else
            thing->MergeTemplate (t, sec, info.default_material, info.default_texlen,
		false);
          csLoaderStat::polygons_loaded += t->GetNumPolygons ();
        }
        break;
      case CS_TOKEN_CLONE:
        {
          ScanStr (params, "%s", str);
	  iThing* t = Engine->FindThing (str, onlyRegion);
          if (!t)
          {
            CsPrintf (MSG_FATAL_ERROR, "Couldn't find thing '%s'!\n", str);
            fatal_exit (0, false);
          }
	  if (info.use_mat_set)
          {
            thing->MergeTemplate (t->GetPrivateObject (), sec, Engine->GetMaterials (), info.mat_set_name,
              info.default_material, info.default_texlen, true);
            info.use_mat_set = false;
	  }
          else
            thing->MergeTemplate (t->GetPrivateObject (), sec, info.default_material, info.default_texlen,
		true);
          csLoaderStat::polygons_loaded += t->GetPrivateObject ()->GetNumPolygons ();
        }
        break;
      case CS_TOKEN_KEY:
        load_key (params, thing);
        break;
      default:
        ps_process (*thing, sec, info, cmd, xname, params);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
    thing->GetMovable ().UpdateMove ();
  if (is_convex || thing->GetFog ().enabled)
    thing->flags.Set (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX);

  return thing;
}



//---------------------------------------------------------------------------

csPolygon3D* csLoader::load_poly3d (char* polyname, char* buf,
  csMaterialWrapper* default_material, float default_texlen,
  csPolygonSet* parent)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (MIPMAP)
    CS_TOKEN_TABLE (PORTAL)
    CS_TOKEN_TABLE (WARP)
    CS_TOKEN_TABLE (LIGHTX)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (SHADING)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (UVA)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (COLLDET)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (COSFACT)
    CS_TOKEN_TABLE (GOURAUD)
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

  csPolygon3D *poly3d = new csPolygon3D (default_material);
  poly3d->SetName (polyname);

  csMaterialWrapper* mat = NULL;
  poly3d->SetParent (parent);

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
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case CS_TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = FindMaterial (str, onlyRegion);
        if (mat == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        poly3d->SetMaterial (mat);
        break;
      case CS_TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          poly3d->flags.Set (CS_POLY_LIGHTING, do_lighting ? CS_POLY_LIGHTING : 0);
        }
        break;
      case CS_TOKEN_MIPMAP:
        //@@@ OBSOLETE
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
      case CS_TOKEN_FOG:
        //@@@ OBSOLETE
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
          csSector *s = new csSector (Engine);
          s->SetName (str);
          poly3d->SetCSPortal (s);
          csLoaderStat::portals_loaded++;
        }
        break;
      case CS_TOKEN_WARP:
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
                poly3d->GetPortal ()->flags.Set (CS_PORTAL_STATICDEST);
                break;
      	      case CS_TOKEN_ZFILL:
		poly3d->GetPortal ()->flags.Set (CS_PORTAL_ZFILL);
        	break;
      	      case CS_TOKEN_CLIP:
		poly3d->GetPortal ()->flags.Set (CS_PORTAL_CLIPDEST);
        	break;
            }
          }
          if (!do_mirror)
            poly3d->GetPortal ()->SetWarp (m_w, v_w_before, v_w_after);
        }
        break;
      case CS_TOKEN_LIGHTX:
        CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                               " and does not do anything!\n");
        break;
      case CS_TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
    	  if (!params2)
    	  {
      	    CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
      	    fatal_exit (0, false);
	  }
          switch (cmd)
          {
            case CS_TOKEN_ORIG:
	      tx_uv_given = false;
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx_orig = parent->Vobj ((int)flist[0]);
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST:
	      tx_uv_given = false;
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = parent->Vobj ((int)flist[0]);
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
              if (num == 1) tx2 = parent->Vobj ((int)flist[0]);
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
	      CsPrintf (MSG_WARNING, "Duplicate vertex-index found in polygon! Ignored...\n");
	    else
	      poly3d->AddVertex (list[i]);
	  }
        }
        break;
      case CS_TOKEN_SHADING:
        while ((cmd = csGetObject (&params, texturing_commands, &name, &params2)) > 0)
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
      case CS_TOKEN_GOURAUD:
        //@@OBSOLETE, see above
        break;
      case CS_TOKEN_MIXMODE:
        {
          UInt mixmode = ParseMixmode (params);
          csPolyTexNone *notex = poly3d->GetNoTexInfo ();
	  if (notex) notex->SetMixmode (mixmode);
          if (mixmode & CS_FX_MASK_ALPHA)
            poly3d->SetAlpha (mixmode & CS_FX_MASK_ALPHA);
          break;
	}
      case CS_TOKEN_UV:
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
      case CS_TOKEN_COLORS:
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
      case CS_TOKEN_UVA:
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
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a polygon!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (poly3d->GetNumVertices () < 3)
  {
    CsPrintf (MSG_WARNING, "Polygon in line %d contains just %d vertices!\n",
      csGetParserLine(), poly3d->GetNumVertices());
    return NULL;
  }

  if (set_colldet == 1)
    poly3d->flags.Set (CS_POLY_COLLDET);
  else if (set_colldet == -1)
    poly3d->flags.Reset (CS_POLY_COLLDET);

  if (tx_uv_given)
  {
    poly3d->SetTextureSpace (
    	poly3d->Vobj (tx_uv_i1), tx_uv1,
	poly3d->Vobj (tx_uv_i2), tx_uv2,
	poly3d->Vobj (tx_uv_i3), tx_uv3);
  }
  else if (tx1_given)
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
    poly3d->SetTextureSpace ((csPolyTxtPlane*)Engine->planes.FindByName (plane_name));
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


//---------------------------------------------------------------------------

iImage* csLoader::load_image (const char* name)
{
  iImage *ifile = NULL;
  iDataBuffer *buf = System->VFS->ReadFile (name);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    CsPrintf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", name);
    return NULL;
  }

  ifile = csImageLoader::Load (buf->GetUint8 (), buf->GetSize (), Engine->GetTextureFormat ());
  buf->DecRef ();

  if (!ifile)
  {
    CsPrintf (MSG_WARNING, "'%s': Cannot load image. Unknown format or wrong extension!\n",name);
    return NULL;
  }

  iDataBuffer *xname = System->VFS->ExpandPath (name);
  ifile->SetName (**xname);
  xname->DecRef ();

  return ifile;
}

void csLoader::txt_process (char *name, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (FILTER)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (MIPMAP)
    CS_TOKEN_TABLE (DITHER)
    CS_TOKEN_TABLE (PROCEDURAL)
    CS_TOKEN_TABLE (PERSISTENT)
    CS_TOKEN_TABLE (FOR_2D)
    CS_TOKEN_TABLE (FOR_3D)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_FOR_2D:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_2D;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_2D;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid FOR_2D() value, 'yes' or 'no' expected\n");
        break;
      case CS_TOKEN_FOR_3D:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_3D;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_3D;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid FOR_3D() value, 'yes' or 'no' expected\n");
        break;
      case CS_TOKEN_PERSISTENT:
        flags |= CS_TEXTURE_PROC_PERSISTENT;
        break;
      case CS_TOKEN_PROCEDURAL:
        flags |= CS_TEXTURE_PROC;
        break;
      case CS_TOKEN_TRANSPARENT:
        do_transp = true;
        ScanStr (params, "%f,%f,%f", &transp.red, &transp.green, &transp.blue);
        break;
      case CS_TOKEN_FILTER:
        CsPrintf (MSG_WARNING, "Warning! TEXTURE/FILTER statement is obsolete"
                               " and does not do anything!\n");
        break;
      case CS_TOKEN_FILE:
        filename = params;
        break;
      case CS_TOKEN_MIPMAP:
        if (strcasecmp (params, "yes") == 0)
          flags &= ~CS_TEXTURE_NOMIPMAPS;
        else if (strcasecmp (params, "no") == 0)
          flags |= CS_TEXTURE_NOMIPMAPS;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid MIPMAP() value, 'yes' or 'no' expected\n");
        break;
      case CS_TOKEN_DITHER:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_DITHER;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_DITHER;
        else
          CsPrintf (MSG_WARNING, "Warning! Invalid MIPMAP() value, 'yes' or 'no' expected\n");
        break;
    }
  }

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
//csMaterialWrapper *mat = Engine->GetMaterials ()->NewMaterial (material);

  csTextureWrapper *tex = Engine->GetTextures ()->NewTexture (image);
//material->SetTextureWrapper (tex);
  tex->flags = flags;
  tex->SetName (name);
  // dereference image pointer since tex already incremented it
  image->DecRef ();
//material->DecRef ();

  if (do_transp)
    tex->SetKeyColor (QInt (transp.red * 255.99),
      QInt (transp.green * 255.99), QInt (transp.blue * 255.99));
}

void csLoader::mat_process (char *name, char* buf, const char *prefix)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (DIFFUSE)
    CS_TOKEN_TABLE (AMBIENT)
    CS_TOKEN_TABLE (REFLECTION)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  char str [255];
  float tmp;

  csMaterial* material = new csMaterial ();

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_TEXTURE:
      {
        ScanStr (params, "%s", str);
        csTextureWrapper *texh = Engine->FindCsTexture (str, onlyRegion);
        if (texh)
          material->SetTextureWrapper (texh);
        else
        {
          CsPrintf (MSG_FATAL_ERROR, "Cannot find texture `%s' for material `%s'\n", str, name);
          fatal_exit (0, false);
        }
        break;
      }
      case CS_TOKEN_COLOR:
        load_color (params, material->GetFlatColor ());
        break;
      case CS_TOKEN_DIFFUSE:
        ScanStr (params, "%f", &tmp);
        material->SetDiffuse (tmp);
        break;
      case CS_TOKEN_AMBIENT:
        ScanStr (params, "%f", &tmp);
        material->SetAmbient (tmp);
        break;
      case CS_TOKEN_REFLECTION:
        ScanStr (params, "%f", &tmp);
        material->SetReflection (tmp);
        break;
    }
  }

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a material specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  csMaterialWrapper *mat = Engine->GetMaterials ()->NewMaterial (material);
  if (prefix)
  {
    char *prefixedname = new char [strlen (name) + strlen (prefix) + 2];
    strcpy (prefixedname, prefix);
    strcat (prefixedname, "_");
    strcat (prefixedname, name);
    mat->SetName (prefixedname);
    delete [] prefixedname;
  }
  else
    mat->SetName (name);
  // dereference material since mat already incremented it
  material->DecRef ();
}

csCurveTemplate* csLoader::load_beziertemplate (char* ptname, char* buf,
  csMaterialWrapper* default_material, float default_texlen,
  csVector3* curve_vertices)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (VERTICES)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tex_commands)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (FIRST_LEN)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (SECOND_LEN)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (LEN)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (UV_SHIFT)
  CS_TOKEN_TABLE_END

  char *name;
  long cmd;
  int i;
  char *params, *params2;

  csBezierTemplate *ptemplate = new csBezierTemplate();
  ptemplate->SetName (ptname);

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
      case CS_TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case CS_TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        mat = FindMaterial (str, onlyRegion);
        if (mat == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        ptemplate->SetMaterialWrapper (mat);
        break;
      case CS_TOKEN_TEXTURE:
        while ((cmd = csGetObject (&params, tex_commands, &name, &params2)) > 0)
        {
          if (!params2)
          {
            CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
            fatal_exit (0, false);
          }
          switch (cmd)
          {
            case CS_TOKEN_ORIG:
              tx1_given = true;
              int num;
              float flist[100];
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx_orig = curve_vertices[(int)flist[0]];
              if (num == 3) tx_orig = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST:
              tx1_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx1 = curve_vertices[(int)flist[0]];
              if (num == 3) tx1 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_FIRST_LEN:
              ScanStr (params2, "%f", &tx1_len);
              tx1_given = true;
              break;
            case CS_TOKEN_SECOND:
              tx2_given = true;
              ScanStr (params2, "%F", flist, &num);
              if (num == 1) tx2 = curve_vertices[(int)flist[0]];
              if (num == 3) tx2 = csVector3(flist[0],flist[1],flist[2]);
              break;
            case CS_TOKEN_SECOND_LEN:
              ScanStr (params2, "%f", &tx2_len);
              tx2_given = true;
              break;
            case CS_TOKEN_LEN:
              ScanStr (params2, "%f", &tx_len);
              break;
            case CS_TOKEN_MATRIX:
              load_matrix (params2, tx_matrix);
              tx_len = 0;
              break;
            case CS_TOKEN_V:
              load_vector (params2, tx_vector);
              tx_len = 0;
              break;
            case CS_TOKEN_UV_SHIFT:
              uv_shift_given = true;
              ScanStr (params2, "%f,%f", &u_shift, &v_shift);
              break;
            case CS_TOKEN_UVEC:
              tx1_given = true;
              load_vector (params2, tx1);
              tx1_len = tx1.Norm ();
              tx1 += tx_orig;
              break;
            case CS_TOKEN_VVEC:
              tx2_given = true;
              load_vector (params2, tx2);
              tx2_len = tx2.Norm ();
              tx2 += tx_orig;
              break;
          }
        }
        break;
      case CS_TOKEN_VERTICES:
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
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a bezier template!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
  return ptemplate;
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

struct Todo
{
  ObName poly;
  int v1, v2, v3, v4;
  int tv1, tv2;
  csMaterialWrapper* material;
  int col_idx;          // Idx in colors table if there was an override.
};

void add_to_todo (Todo* todo, int& todo_end, char* poly,
        int v1, int v2, int v3, int v4, int tv1, int tv2,
        csMaterialWrapper* material, int col_idx,
        Color* colors, int num_colors)
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
  for (i = 0 ; i < num_colors ; i++)
    if (!strcmp (poly, colors[i].poly))
    {
      todo[todo_end].col_idx = i;
      break;
    }
  todo_end++;
}

void csLoader::load_tex (char** buf, Color* colors, int num_colors, char* name)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (PLANE)
    CS_TOKEN_TABLE (LEN)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_TEXTURE://@@@MAT
        ScanStr (params, "%s", str);
        colors[num_colors].material = FindMaterial (str, onlyRegion);
        if (colors[num_colors].material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_PLANE:
        ScanStr (params, "%s", str);
        strcpy (colors[num_colors].plane, str);
        break;
      case CS_TOKEN_LEN:
        ScanStr (params, "%f", &colors[num_colors].len);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a texture specification!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
}

csSector* csLoader::load_room (char* secname, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (TEXTURE_LIGHTING)
    CS_TOKEN_TABLE (TEXTURE_MIPMAP)
    CS_TOKEN_TABLE (TEXTURE_SCALE)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (TEX)
    CS_TOKEN_TABLE (CEIL_TEXTURE)
    CS_TOKEN_TABLE (FLOOR_TEXTURE)
    CS_TOKEN_TABLE (LIGHTX)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (DIM)
    CS_TOKEN_TABLE (HEIGHT)
    CS_TOKEN_TABLE (FLOOR_HEIGHT)
    CS_TOKEN_TABLE (FLOOR_CEIL)
    CS_TOKEN_TABLE (FLOOR)
    CS_TOKEN_TABLE (CEILING)
    CS_TOKEN_TABLE (SIXFACE)
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SKY)
    CS_TOKEN_TABLE (PORTAL)
    CS_TOKEN_TABLE (SPLIT)
    CS_TOKEN_TABLE (TRIGGER)
    CS_TOKEN_TABLE (ACTIVATE)
    CS_TOKEN_TABLE (BSP)
    CS_TOKEN_TABLE (STATBSP)
    CS_TOKEN_TABLE (SPRITE2D)
    CS_TOKEN_TABLE (SPRITE)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (FOUNTAIN)
    CS_TOKEN_TABLE (RAIN)
    CS_TOKEN_TABLE (SNOW)
    CS_TOKEN_TABLE (FIRE)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (portal_commands)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (SECTOR)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (WARP)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (mCommands)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (W)
    CS_TOKEN_TABLE (MIRROR)
    CS_TOKEN_TABLE (STATIC)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matrix)
    CS_TOKEN_TABLE (MATRIX)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_vector)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params, * params2;
  int i, l;
  int i1, i2, i3, i4;
  bool do_stat_bsp = false;

  csSector* sector = new csSector (Engine);
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
      case CS_TOKEN_BSP:
        CsPrintf (MSG_FATAL_ERROR,
          "BSP keyword is no longer supported. Use STATBSP instead after putting\n"
          "all non-convex polygons in things.\n");
        break;
      case CS_TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case CS_TOKEN_MOVE:
        {
          char* params2;
          csGetObject (&params, tok_matrix, &name, &params2);
          load_matrix (params2, mm);
          csGetObject (&params, tok_vector, &name, &params2);
          load_vector (params2, vm);
        }
        break;
      case CS_TOKEN_TEXTURE:
        ScanStr (params, "%s", str);
        material = FindMaterial (str, onlyRegion);
        if (material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        break;
      case CS_TOKEN_TEXTURE_LIGHTING:
        ScanStr (params, "%b", &no_lighting); no_lighting = !no_lighting;
        break;
      case CS_TOKEN_TEXTURE_MIPMAP:
        //@@@ OBSOLETE
        break;
      case CS_TOKEN_CEIL_TEXTURE:
      case CS_TOKEN_FLOOR_TEXTURE:
        ScanStr (params, "%s", str);
        colors[num_colors].material = FindMaterial (str, onlyRegion);
        if (colors[num_colors].material == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find material named '%s'!\n", str);
          fatal_exit (0, true);
        }
        strcpy (colors[num_colors].poly,
                cmd == CS_TOKEN_CEIL_TEXTURE ? "up" : "down");
        colors[num_colors].plane[0] = 0;
        if (num_colors >= MAX_ROOM_COLORS)
        {
          CsPrintf (MSG_FATAL_ERROR, "OVERFLOW number of colors in room!\n");
          fatal_exit (0, false);
        }
        num_colors++;
        break;
      case CS_TOKEN_LIGHTX:
        CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                               " and does not do anything!\n");
        break;
      case CS_TOKEN_TEX:
        load_tex (&params, colors, num_colors, name);
        num_colors++;
        break;
      case CS_TOKEN_TEXTURE_SCALE:
        ScanStr (params, "%f", &tscale);
        break;
      case CS_TOKEN_DIM:
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
      case CS_TOKEN_FLOOR_HEIGHT:
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
      case CS_TOKEN_HEIGHT:
        ScanStr (params, "%f", &r);
        v0.y = r+v2.y;
        v1.y = r+v3.y;
        v4.y = r+v6.y;
        v5.y = r+v7.y;
        break;
      case CS_TOKEN_FLOOR_CEIL:
        ScanStr (params, "(%f,%f) (%f,%f) (%f,%f) (%f,%f)",
                 &v2.x, &v2.z, &v3.x, &v3.z, &v7.x, &v7.z, &v6.x, &v6.z);
        v0 = v2;
        v1 = v3;
        v5 = v7;
        v4 = v6;
        break;
      case CS_TOKEN_FLOOR:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v2.x, &v2.y, &v2.z, &v3.x, &v3.y, &v3.z,
                 &v7.x, &v7.y, &v7.z, &v6.x, &v6.y, &v6.z);
        break;
      case CS_TOKEN_CEILING:
        ScanStr (params, "(%f,%f,%f) (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
                 &v0.x, &v0.y, &v0.z, &v1.x, &v1.y, &v1.z,
                 &v5.x, &v5.y, &v5.z, &v4.x, &v4.y, &v4.z);
        break;
      case CS_TOKEN_LIGHT:
        sector->AddLight ( load_statlight(name, params) );
        break;
      case CS_TOKEN_FOG:
        {
          csFog& f = sector->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case CS_TOKEN_KEY:
        load_key (params, sector);
        break;
      case CS_TOKEN_FIRE:
        partsys = load_fire (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_FOUNTAIN:
        partsys = load_fountain (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_RAIN:
        partsys = load_rain (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_SNOW:
        partsys = load_snow (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_MESHOBJ:
        {
          csMeshWrapper* sp = new csMeshWrapper (Engine);
          sp->SetName (name);
          LoadMeshObject (sp, params);
          Engine->sprites.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_SPRITE:
        {
          csSprite3D* sp = new csSprite3D (Engine);
          sp->SetName (name);
          LoadSprite (sp, params);
          Engine->sprites.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_SPRITE2D:
        {
          csSprite2D* sp = new csSprite2D (Engine);
          sp->SetName (name);
          LoadSprite (sp, params);
          Engine->sprites.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_SKY:
        Engine->skies.Push (load_thing (name, params, sector, true, false));
        break;
      case CS_TOKEN_THING:
        Engine->things.Push (load_thing (name, params, sector, false, false));
        break;
      case CS_TOKEN_SIXFACE:
        Engine->things.Push (load_sixface (name, params, sector, false));
        break;
      case CS_TOKEN_PORTAL:
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
              case CS_TOKEN_POLYGON:
                ScanStr (params2, "%s", portals[num_portals].poly);
                break;
              case CS_TOKEN_SECTOR:
                ScanStr (params2, "%s", portals[num_portals].sector);
                break;
              case CS_TOKEN_ALPHA:
                ScanStr (params2, "%d", &portals[num_portals].alpha);
                portals[num_portals].alpha = portals[num_portals].alpha * 655 / 256;
                break;
              case CS_TOKEN_WARP:
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
                      case CS_TOKEN_MATRIX:
                        load_matrix (params3, portals[num_portals].m_warp);
                        portals[num_portals].do_mirror = false;
                        break;
                      case CS_TOKEN_V:
                        load_vector (params3, portals[num_portals].v_warp_before);
                        portals[num_portals].v_warp_after =
                          portals[num_portals].v_warp_before;
                        portals[num_portals].do_mirror = false;
                        break;
                      case CS_TOKEN_W:
                        load_vector (params3, portals[num_portals].v_warp_after);
                        portals[num_portals].do_mirror = false;
                        break;
                      case CS_TOKEN_MIRROR:
                        portals[num_portals].do_mirror = true;
                        break;
                      case CS_TOKEN_STATIC:
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
      case CS_TOKEN_SPLIT:
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
      case CS_TOKEN_ACTIVATE:
        CsPrintf (MSG_WARNING, "Warning! ACTIVATE statement is obsolete"
                                 " and does not do anything!\n");
        break;
      case CS_TOKEN_TRIGGER:
        CsPrintf (MSG_WARNING, "Warning! TRIGGER statement is obsolete"
                                 " and does not do anything!\n");
        break;
      default:
        CsPrintf (MSG_FATAL_ERROR, "Unrecognized token in room '%s'!\n",
                  sector->GetName ());
        fatal_exit (0, false);
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
               material, -1, colors, num_colors);
  add_to_todo (todo, todo_end, "east", 1, 5, 7, 3, 1, 5,
               material, -1, colors, num_colors);
  add_to_todo (todo, todo_end, "south", 5, 4, 6, 7, 5, 4,
               material, -1, colors, num_colors);
  add_to_todo (todo, todo_end, "west", 4, 0, 2, 6, 4, 0,
               material, -1, colors, num_colors);
  add_to_todo (todo, todo_end, "up", 4, 5, 1, 0, 4, 5,
               material, -1, colors, num_colors);
  add_to_todo (todo, todo_end, "down", 2, 3, 7, 6, 2, 3,
               material, -1, colors, num_colors);

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
                      colors, num_colors);
          i1 = sector->GetNumVertices () - 2;
          i4 = sector->GetNumVertices () - 1;
        }

        sprintf (pname, "%s%c", todo[done].poly, l+'A');
        add_to_todo (todo, todo_end, pname, i1, i2, i3, i4, todo[done].tv1,
                     todo[done].tv2, todo[done].material, todo[done].col_idx,
                     colors, num_colors);
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
                      todo[done].material, todo[done].col_idx,
                      colors, num_colors);
          i3 = sector->GetNumVertices () - 1;
          i4 = sector->GetNumVertices () - 2;
        }

        sprintf (pname, "%s%d", todo[done].poly, l+1);
        add_to_todo (todo, todo_end, pname, i1, i2, i3, i4, todo[done].tv1,
                     todo[done].tv2, todo[done].material, todo[done].col_idx,
                     colors, num_colors);
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
        p->SetTextureSpace ((csPolyTxtPlane*)Engine->planes.FindByName (colors[idx].plane));
      p->flags.Set (CS_POLY_LIGHTING, (no_lighting ? 0 : CS_POLY_LIGHTING));
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
    portal = new csSector (Engine) ;
    portal->SetName (portals[i].sector);
    p->SetCSPortal (portal);
    csLoaderStat::portals_loaded++;
    if (portals[i].is_warp)
    {
      if (portals[i].do_mirror)
      {
        p->SetWarp (csTransform::GetReflect ( *(p->GetPolyPlane ()) ));
	p->flags.Set (CS_POLY_COLLDET);
      }
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
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (CIRCLE)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (TEXLEN)
    CS_TOKEN_TABLE (TRIGGER)
    CS_TOKEN_TABLE (ACTIVATE)
    CS_TOKEN_TABLE (LIGHTX)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (BSP)
    CS_TOKEN_TABLE (STATBSP)
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SIXFACE)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (SPRITE2D)
    CS_TOKEN_TABLE (SPRITE)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (SKYDOME)
    CS_TOKEN_TABLE (SKY)
    CS_TOKEN_TABLE (TERRAIN)
    CS_TOKEN_TABLE (NODE)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (FOUNTAIN)
    CS_TOKEN_TABLE (RAIN)
    CS_TOKEN_TABLE (SNOW)
    CS_TOKEN_TABLE (FIRE)
    CS_TOKEN_TABLE (HARDMOVE)
    CS_TOKEN_TABLE (BEZIER)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVESCALE)
    CS_TOKEN_TABLE (CURVECONTROL)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  bool do_stat_bsp = false;

  csSector* sector = new csSector (Engine) ;
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
      case CS_TOKEN_SKYDOME:
        skydome_process (*sector, name, params, info.default_material);
        break;
      case CS_TOKEN_TERRAIN:
        terrain_process (*sector, name, params);
        break;
      case CS_TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case CS_TOKEN_FIRE:
        partsys = load_fire (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_FOUNTAIN:
        partsys = load_fountain (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_RAIN:
        partsys = load_rain (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_SNOW:
        partsys = load_snow (name, params);
	partsys->GetMovable ().SetSector (sector);
	partsys->GetMovable ().UpdateMove ();
        break;
      case CS_TOKEN_SKY:
        Engine->skies.Push (load_thing (name, params, sector, true, false));
        break;
      case CS_TOKEN_THING:
        Engine->things.Push (load_thing (name, params, sector, false, false));
        break;
      case CS_TOKEN_SIXFACE:
        Engine->things.Push (load_sixface (name, params, sector, false));
        break;
      case CS_TOKEN_MESHOBJ:
        {
          csMeshWrapper* sp = new csMeshWrapper (Engine);
          sp->SetName (name);
          LoadMeshObject (sp, params);
          Engine->sprites.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_SPRITE:
        {
          csSprite3D* sp = new csSprite3D (Engine);
          sp->SetName (name);
          LoadSprite (sp, params);
          Engine->sprites.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_SPRITE2D:
        {
          csSprite2D* sp = new csSprite2D (Engine);
          sp->SetName (name);
          LoadSprite (sp, params);
          Engine->sprites.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_LIGHT:
        sector->AddLight ( load_statlight(name, params) );
        break;
      case CS_TOKEN_NODE:
        sector->ObjAdd ( load_node(name, params, sector) );
        break;
      case CS_TOKEN_KEY:
      {
        load_key(params, sector);
        break;
      }
      default:
        ps_process (*sector, sector, info, cmd, name, params);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
      case CS_TOKEN_RADIUS:
        ScanStr (params, "%f", &radius);
        break;
      case CS_TOKEN_VERTICES:
        ScanStr (params, "%D", prev_vertices, &num);
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
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TYPE)
    CS_TOKEN_TABLE (HEIGHTMAP)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (TEXTURE)
  CS_TOKEN_TABLE_END

  long cmd;
  char* params;
  char type[256];		// @@@ Hardcoded.
  bool lod;
  char heightmapname[256];	// @@@ Hardcoded.
  char texturebasename[256];	// @@@ Hardcoded.
  unsigned int detail = 3000;

  lod = false;
  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_TYPE:
        ScanStr (params, "%s", type);
	if (!strcasecmp (type, "lod")) lod = true;
        break;
      case CS_TOKEN_HEIGHTMAP:
        ScanStr (params, "%s", heightmapname);
        break;
      case CS_TOKEN_TEXTURE:
        ScanStr (params, "%s", texturebasename);
        break;
      case CS_TOKEN_DETAIL:
        ScanStr (params, "%d", &detail);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a terrain!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (!System->VFS->Exists(heightmapname))
  {
    CsPrintf (MSG_FATAL_ERROR, "Error locating height field: %s\n", heightmapname);
    fatal_exit (0, false);
  }

  iDataBuffer *heightmap = System->VFS->ReadFile (heightmapname);
  if (!heightmap)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error loading height field: %s\n", heightmapname);
    fatal_exit (0, false);
  }

  csTerrain* terr;
  if (lod)
    terr = (csTerrain*)new csLODTerrain ();
  else
    terr = (csTerrain*)new csDDGTerrain ();
  terr->SetName (name);

  // Otherwise read file, if that fails generate a random map.
  if (!terr->Initialize (**heightmap, heightmap->GetSize ()))
  {
    heightmap->DecRef ();
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
    csMaterialWrapper* mat = FindMaterial (matname, onlyRegion);
    if (mat == NULL) mat = first_mat;
    first_mat = mat;
    terr->SetMaterial (i, mat);
  }
  terr->SetDetail (detail);

  heightmap->DecRef ();
  sector.terrains.Push (terr);
}

//---------------------------------------------------------------------------

iSoundData* csLoader::LoadSoundData(const char* filename) {
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
  iDataBuffer *buf = System->VFS->ReadFile (filename);
  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    CsPrintf (MSG_WARNING,
      "Cannot open sound file \"%s\" from VFS\n", filename);
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
  iSoundData *Sound = SoundLoader->LoadSound(buf->GetUint8 (), buf->GetSize (), Format);
  buf->DecRef ();
  /* ### */SoundLoader->DecRef();

  /* check for valid sound data */
  if (!Sound) {
    CsPrintf (MSG_WARNING, "Cannot create sound data from file \"%s\"!\n", filename);
    return NULL;
  }

  return Sound;
}


csSoundDataObject *csLoader::LoadSoundObject (csEngine* engine,
  char* name, const char* fname) {

  Engine=engine;
  /* load the sound data */
  iSoundData *Sound = LoadSoundData(fname);

  /* build wrapper object */
  csSoundDataObject* sndobj = new csSoundDataObject (Sound);
  sndobj->SetName (name);

  /* add it to the engine */
// @@@ engine->GetSounds()->Add(sndobj);

  return sndobj;
}

//---------------------------------------------------------------------------

bool csLoader::LoadMap (char* buf, bool onlyRegion)
{
  ::onlyRegion = onlyRegion;

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (WORLD)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (SECTOR)
    CS_TOKEN_TABLE (ROOM)
    CS_TOKEN_TABLE (PLANE)
    CS_TOKEN_TABLE (COLLECTION)
    CS_TOKEN_TABLE (SCRIPT)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (MAT_SET)
    CS_TOKEN_TABLE (LIGHTX)
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SIXFACE)
    CS_TOKEN_TABLE (SPRITE)
    CS_TOKEN_TABLE (LIBRARY)
    CS_TOKEN_TABLE (START)
    CS_TOKEN_TABLE (SOUNDS)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MOTION)
    CS_TOKEN_TABLE (INCLUDESPRITE)
    CS_TOKEN_TABLE (REGION)
  CS_TOKEN_TABLE_END

  csResetParserLine();
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

    Engine->SelectLibrary (name); //@@@? Don't do this for regions!

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", data);
        fatal_exit (0, false);
      }
      switch (cmd)
      {
        case CS_TOKEN_MOTION:
	  {
	    iMotionManager* motionmanager = System->MotionMan;
	    if (!motionmanager)
	    {
	      CsPrintf (MSG_FATAL_ERROR, "No motion manager loaded!\n");
	      fatal_exit (0, false);
	    }
	    else
	    {
	      iMotion* m = motionmanager->FindByName (name);
	      if (!m)
	      {
		m = motionmanager->AddMotion (name);
		LoadMotion (m, params);
	      }
	    }
	  }
	  break;
        case CS_TOKEN_MESHOBJ:
          {
            csMeshFactoryWrapper* t = (csMeshFactoryWrapper*)Engine->meshobj_factories.FindByName (name);
            if (!t)
            {
              t = new csMeshFactoryWrapper ();
              t->SetName (name);
              Engine->meshobj_factories.Push (t);
            }
            LoadMeshObjectFactory (t, params);
          }
	  break;
        case CS_TOKEN_REGION:
	  {
	    char str[255];
	    ScanStr (params, "%s", str);
	    if (*str)
	      Engine->SelectRegion (str);
	    else
	      Engine->SelectRegion (NULL);
	  }
	  break;
        case CS_TOKEN_INCLUDESPRITE:
	  {
	    char str[255];
	    ScanStr (params, "%s", str);
	    CsPrintf (MSG_WARNING, "Loading sprite '%s'\n", str);
	    LoadSpriteTemplate(Engine, str);
	  }
	  break;
        case CS_TOKEN_SPRITE:
          {
            csSpriteTemplate* t = (csSpriteTemplate*)Engine->sprite_templates.FindByName (name);
            if (!t)
            {
              t = new csSpriteTemplate ();
              t->SetName (name);
              Engine->sprite_templates.Push (t);
            }
            LoadSpriteTemplate (t, params);
          }
          break;
        case CS_TOKEN_THING:
          if (!Engine->thing_templates.FindByName (name))
            Engine->thing_templates.Push (load_thing (name, params, NULL, false, true));
          break;
        case CS_TOKEN_SIXFACE:
          if (!Engine->thing_templates.FindByName (name))
            Engine->thing_templates.Push (load_sixface (name, params, NULL, true));
          break;
        case CS_TOKEN_SECTOR:
          if (!Engine->FindSector (name, onlyRegion))
            Engine->sectors.Push (load_sector (name, params));
          break;
        case CS_TOKEN_PLANE:
          Engine->planes.Push (load_polyplane (params, name));
          break;
        case CS_TOKEN_COLLECTION:
          Engine->collections.Push (load_collection (name, params));
          break;
        case CS_TOKEN_SCRIPT:
          CsPrintf (MSG_WARNING, "Warning! SCRIPT statement is obsolete"
                                 " and does not do anything!\n");
          break;
	case CS_TOKEN_MAT_SET:
          if (!LoadMaterials (params, name))
            return false;
          break;
        case CS_TOKEN_TEXTURES:
          {
            //Engine->GetTextures ()->DeleteAll ();
            if (!LoadTextures (params))
              return false;
          }
          break;
        case CS_TOKEN_MATERIALS:
          {
            //Engine->GetMaterials ()->DeleteAll ();
            if (!LoadMaterials (params))
              return false;
          }
          break;
        case CS_TOKEN_SOUNDS:
          if (!LoadSounds (params))
            return false;
          break;
        case CS_TOKEN_ROOM:
          // Not an object but it is translated to a special sector.
          if (!Engine->FindSector (name, onlyRegion))
            Engine->sectors.Push (load_room (name, params));
          break;
        case CS_TOKEN_LIGHTX:
          CsPrintf (MSG_WARNING, "Warning! LIGHTX statement is obsolete"
                                 " and does not do anything!\n");
          break;
        case CS_TOKEN_LIBRARY:
          LoadLibraryFile (Engine, name);
          break;
        case CS_TOKEN_START:
        {
          char start_sector [100];
          csVector3 pos (0, 0, 0);
          ScanStr (params, "%s,%f,%f,%f", &start_sector, &pos.x, &pos.y, &pos.z);
          Engine->camera_positions.Push (new csCameraPosition ("Start",
            start_sector, pos, csVector3 (0, 0, 1), csVector3 (0, 1, 0)));
          break;
        }
        case CS_TOKEN_KEY:
          load_key (params, Engine);
          break;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a map!\n", csGetLastOffender ());
      fatal_exit (0, false);
    }
  }

  int sn = Engine->sectors.Length ();
  csRegion* cur_region = NULL;
  if (onlyRegion) cur_region = Engine->GetCsCurrentRegion ();
  while (sn > 0)
  {
    sn--;
    csSector* s = (csSector*)(Engine->sectors)[sn];
    if (cur_region && !cur_region->IsInRegion (s))
      continue;
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
	  iSector *snew = Engine->FindSector (stmp->GetName (), onlyRegion);
          if (!snew)
          {
            CsPrintf (MSG_FATAL_ERROR, "Sector '%s' not found for portal in"
                      " polygon '%s/%s'!\n", stmp->GetName (),
                      ((csObject*)p->GetParent ())->GetName (),
                      p->GetName ());
            fatal_exit (0, false);
          }
          portal->SetSector (snew->GetPrivateObject ());
          delete stmp;
        }
      }
    }
  }

  return true;
}

bool csLoader::LoadMapFile (csEngine* engine, const char* file)
{
  engine->StartEngine ();
  return AppendMapFile (engine, file);
}

bool csLoader::AppendMapFile (csEngine* engine, const char* file,
	bool onlyRegion)
{
  Engine = engine;

  csLoaderStat::Init ();

  iDataBuffer *buf = System->VFS->ReadFile (file);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open map file \"%s\" on VFS!\n", file);
    return false;
  }

  csIniFile* cfg = new csIniFile ("map.cfg", System->VFS);
  if (cfg)
  {
    csLightMap::SetLightCellSize (cfg->GetInt ("Lighting", "LIGHTMAP_SIZE",
    	csLightMap::lightcell_size));
    delete cfg;
  }
  CsPrintf (MSG_INITIALIZATION, "Lightmap grid size = %dx%d.\n",
      csLightMap::lightcell_size, csLightMap::lightcell_size);

  if (!LoadMap (**buf, onlyRegion))
    return false;

  if (csLoaderStat::polygons_loaded)
  {
    CsPrintf (MSG_INITIALIZATION, "Loaded map file:\n");
    CsPrintf (MSG_INITIALIZATION, "  %d polygons (%d portals),\n", csLoaderStat::polygons_loaded,
      csLoaderStat::portals_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d sectors, %d things, %d sprites, \n", csLoaderStat::sectors_loaded,
      csLoaderStat::things_loaded, csLoaderStat::sprites_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d curves, %d lights, %d sounds.\n", csLoaderStat::curves_loaded,
      csLoaderStat::lights_loaded, csLoaderStat::sounds_loaded);
  } /* endif */

  buf->DecRef ();

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadTextures (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MAX_TEXTURES)
    CS_TOKEN_TABLE (TEXTURE)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_MAX_TEXTURES:
        // ignored for backward compatibility
        break;
      case CS_TOKEN_TEXTURE:
        txt_process (name, params);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a matrix!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadMaterials (char* buf, const char* prefix)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_MATERIAL:
        mat_process (name, params, prefix);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing material!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadLibrary (char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (LIBRARY)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SPRITE)
    CS_TOKEN_TABLE (SOUNDS)
    CS_TOKEN_TABLE (PLANE)
  CS_TOKEN_TABLE_END

  char *name, *data;
  if (csGetObject (&buf, tokens, &name, &data))
  {
    // Empty LIBRARY () directive?
    if (!data)
      return false;

    long cmd;
    char* params;

    Engine->SelectLibrary (name);

    while ((cmd = csGetObject (&data, commands, &name, &params)) > 0)
    {
      if (!params)
      {
        CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", data);
        return false;
      }

      switch (cmd)
      {
        case CS_TOKEN_PLANE:
          Engine->planes.Push ( load_polyplane (params, name) );
          break;
        case CS_TOKEN_TEXTURES:
          // Append textures to engine.
          if (!LoadTextures (params))
            return false;
          break;
        case CS_TOKEN_MATERIALS:
          if (!LoadMaterials (params))
            return false;
          break;
        case CS_TOKEN_SOUNDS:
          if (!LoadSounds (params))
            return false;
          break;
        case CS_TOKEN_SPRITE:
          {
            csSpriteTemplate* t = (csSpriteTemplate*)Engine->sprite_templates.FindByName (name);
            if (!t)
            {
              t = new csSpriteTemplate ();
              t->SetName (name);
              Engine->sprite_templates.Push (t);
            }
            LoadSpriteTemplate (t, params);
          }
          break;
        case CS_TOKEN_THING:
          if (!Engine->thing_templates.FindByName (name))
            Engine->thing_templates.Push (load_thing (name, params, NULL, false, true));
          break;
      }
    }
    if (cmd == CS_PARSERR_TOKENNOTFOUND)
    {
      CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a library file!\n", csGetLastOffender ());
      return false;
    }
  }
  return true;
}

bool csLoader::LoadLibraryFile (csEngine* engine, const char* fname)
{
  iDataBuffer *buf = System->VFS->ReadFile (fname);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open library file \"%s\" on VFS!\n", fname);
    return false;
  }

  Engine = engine;
  bool retcode = LoadLibrary (**buf);

  buf->DecRef ();

  return retcode;
}

csTextureWrapper* csLoader::LoadTexture (csEngine* engine, const char* name, const char* fname)
{
  Engine = engine;
  iImage *image = load_image (fname);
  if (!image)
    return NULL;
  csTextureWrapper *th = engine->GetTextures ()->NewTexture (image);
  th->SetName (name);
  // dereference image pointer since th already incremented it
  image->DecRef ();

  csMaterial* material = new csMaterial ();
  csMaterialWrapper* mat = Engine->GetMaterials ()->NewMaterial (material);
  mat->SetName (name);
  material->SetTextureWrapper (th);
  material->DecRef ();

  return th;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSounds (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (SOUND)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (options)
    CS_TOKEN_TABLE (FILE)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_SOUND:
      {
        const char* filename = name;
        char* maybename;
        cmd = csGetCommand (&params, options, &maybename);
        if (cmd == CS_TOKEN_FILE)
          filename = maybename;
        else if (cmd == CS_PARSERR_TOKENNOTFOUND)
        {
          CsPrintf (MSG_FATAL_ERROR, "Unknown token '%s' found while parsing SOUND directive.\n", csGetLastOffender());
          fatal_exit (0, false);
        }
        iSoundData *snd = csSoundDataObject::GetSound (*Engine, name);
        if (!snd)
        {
          csSoundDataObject *s = LoadSoundObject(Engine, name, filename);
          if (s)
          {
            Engine->ObjAdd(s);
            csLoaderStat::sounds_loaded++;
          }
        }
      }
      break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the list of sounds!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadSkeleton (csSkeletonLimb* limb, char* buf, bool is_connection)
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

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_LIMB:
        {
          csSkeletonConnection* con = new csSkeletonConnection ();
	  if (name) con->SetName (name);
	  if (!LoadSkeleton (con, params, true)) return false;
	  limb->AddChild (con);
	}
        break;
      case CS_TOKEN_TRANSFORM:
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
              case CS_TOKEN_MATRIX:
                load_matrix (params2, m);
		break;
              case CS_TOKEN_V:
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
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite skeleton!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

// @@@ MEMORY LEAK!!! We should unload all the plugins we load here.
csVector csLoader::loaded_plugins;

iPlugIn* csLoader::FindPlugIn (const char* name)
{
  int i;
  for (i = 0 ; i < loaded_plugins.Length () ; i++)
  {
    LoadedPlugin* lp = (LoadedPlugin*)loaded_plugins[i];
    if (!strcmp (lp->name, name)) return lp->plugin;
  }
  return NULL;
}

void csLoader::NewPlugIn (const char* name, iPlugIn* plugin)
{
  LoadedPlugin* lp = new LoadedPlugin ();
  lp->name = name;
  lp->plugin = plugin;
}

bool csLoader::LoadMeshObjectFactory (csMeshFactoryWrapper* stemp, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  iLoaderPlugIn* plug = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_PARAMS:
	if (!plug)
	{
          CsPrintf (MSG_FATAL_ERROR, "Could not load plugin!\n");
          fatal_exit (0, false);
	}
	else
	{
	  iBase* mof = plug->Parse (params,
	      (iEngine*)QUERY_INTERFACE (csEngine::current_engine, iEngine));
	  iMeshObjectFactory* mof2 = QUERY_INTERFACE (mof, iMeshObjectFactory);
	  stemp->SetMeshObjectFactory (mof2);
	  mof2->DecRef ();
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)FindPlugIn (str);
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "MeshLdr", iLoaderPlugIn);
	    if (plug) NewPlugIn (str, plug);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a mesh factory!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadMeshObject (csMeshWrapper* mesh, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (MOVE)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  csLoaderStat::sprites_loaded++;
  iLoaderPlugIn* plug = NULL;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_MOVE:
        {
          char* params2;
          mesh->GetMovable ().SetTransform (csMatrix3 ());     // Identity matrix.
          mesh->GetMovable ().SetPosition (csVector3 (0, 0, 0));
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", params);
              fatal_exit (0, false);
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                load_matrix (params2, m);
                mesh->GetMovable ().SetTransform (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                load_vector (params2, v);
                mesh->GetMovable ().SetPosition (v);
                break;
              }
            }
          }
	  mesh->GetMovable ().UpdateMove ();
        }
        break;

      case CS_TOKEN_PARAMS:
	if (!plug)
	{
          CsPrintf (MSG_FATAL_ERROR, "Could not load plugin!\n");
          fatal_exit (0, false);
	}
	else
	{
	  iBase* mo = plug->Parse (params,
	      (iEngine*)QUERY_INTERFACE (csEngine::current_engine, iEngine));
	  iMeshObject* mo2 = QUERY_INTERFACE (mo, iMeshObject);
	  mesh->SetMeshObject (mo2);
	  mo2->DecRef ();
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)FindPlugIn (str);
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "MeshLdr", iLoaderPlugIn);
	    if (plug) NewPlugIn (str, plug);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the mesh object!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

csSpriteTemplate* csLoader::LoadSpriteTemplate (csEngine* engine,
	const char* fname)
{
  Engine = engine;

  iDataBuffer *databuff = System->VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open sprite template file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (SPRITE)
  CS_TOKEN_TABLE_END

  char *name, *data;
  char *buf = **databuff;

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
      Engine->sprite_templates.Push (tmpl);
      databuff->DecRef ();
      return tmpl;
    }
    else
    {
      delete tmpl;
      databuff->DecRef ();
      return NULL;
    }
  }
  databuff->DecRef ();
  return NULL;
}

bool csLoader::LoadSpriteTemplate (csSpriteTemplate* stemp, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FRAME)
    CS_TOKEN_TABLE (ACTION)
    CS_TOKEN_TABLE (SMOOTH)
    CS_TOKEN_TABLE (TRIANGLE)
    CS_TOKEN_TABLE (SKELETON)
    CS_TOKEN_TABLE (FILE)
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

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case CS_TOKEN_MATERIAL:
        {
          ScanStr (params, "%s", str);
          csMaterialWrapper *mat = FindMaterial (str, onlyRegion);
          if (mat)
            stemp->SetMaterial (mat);
          else
          {
            CsPrintf (MSG_FATAL_ERROR, "Material `%s' not found!\n", str);
            fatal_exit (0, true);
          }
        }
        break;

      case CS_TOKEN_SKELETON:
	{
          csSkeleton* skeleton = new csSkeleton ();
	  if(name) skeleton->SetName (name);
	  if (!LoadSkeleton (skeleton, params, false)) return false;
	  stemp->SetSkeleton (skeleton);
	}
        break;

      case CS_TOKEN_ACTION:
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
              case CS_TOKEN_F:
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

      case CS_TOKEN_FRAME:
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
              case CS_TOKEN_V:
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
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
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

      case CS_TOKEN_TRIANGLE:
        {
          int a, b, c;
          ScanStr (params, "%d,%d,%d", &a, &b, &c);
          stemp->AddTriangle (a, b, c);
        }
        break;

      case CS_TOKEN_FILE:
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

      case CS_TOKEN_SMOOTH:
        {
          int num, list[30];
          ScanStr (params, "%D", list, &num);
          switch (num)
          {
            case 0  :  stemp->MergeNormals ();                  break;
            case 1  :  stemp->MergeNormals (list[0]);           break;
            case 2  :  stemp->MergeNormals (list[0], list[1]);  break;
            default :  CsPrintf (MSG_WARNING, "Confused by SMOOTH options: '%s'\n", params);
                       CsPrintf (MSG_WARNING, "no smoothing performed\n");
          }
        }
        break;

      case CS_TOKEN_TWEEN:
	{
	  bool do_tween;
          ScanStr (params, "%b", &do_tween);
          stemp->EnableTweening (do_tween);
	}
	break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (LIGHTING)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_MIXMODE:
        spr->SetMixmode (ParseMixmode (params));
        break;
      case CS_TOKEN_VERTICES:
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
      case CS_TOKEN_UV:
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
      case CS_TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          spr->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_COLORS:
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
      case CS_TOKEN_MOVE:
        {
	  float x, y, z;
	  ScanStr (params, "%f,%f,%f", &x, &y, &z);
          spr->SetPosition (csVector3 (x, y, z));
	}
        break;

      case CS_TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case CS_TOKEN_MATERIAL:
        {
          ScanStr (params, "%s", str);
          csMaterialWrapper* mat = FindMaterial (str, onlyRegion);
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
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the sprite!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadSprite (csSprite3D* spr, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (TWEEN)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_MIXMODE:
        spr->SetMixmode (ParseMixmode (params));
        break;
      case CS_TOKEN_MOVE:
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
              case CS_TOKEN_MATRIX:
              {
                csMatrix3 m;
                load_matrix (params2, m);
                spr->GetMovable ().SetTransform (m);
                break;
              }
              case CS_TOKEN_V:
              {
                csVector3 v;
                load_vector (params2, v);
                spr->GetMovable ().SetPosition (v);
                break;
              }
            }
          }
	  spr->GetMovable ().UpdateMove ();
        }
        break;

      case CS_TOKEN_TEMPLATE:
        memset (str, 0, 255);
        memset (str2, 0, 255);
        ScanStr (params, "%s,%s", str, str2);
        tpl = (csSpriteTemplate*)Engine->sprite_templates.FindByName (str);
        if (tpl == NULL)
        {
          CsPrintf (MSG_WARNING, "Couldn't find sprite template '%s'!\n", str);
          fatal_exit (0, true);
        }
        spr->SetTemplate (tpl);
        if (tpl->FindAction (str2) != NULL)
          spr->SetAction (str2);
        break;

      case CS_TOKEN_TWEEN:
	{
	  bool do_tween;
          ScanStr (params, "%b", &do_tween);
          spr->EnableTweening (do_tween);
	}
	break;

      case CS_TOKEN_TEXNR:
        //@@OBSOLETE, retained for backward compatibility
      case CS_TOKEN_MATERIAL:
        ScanStr (params, "%s", str);
        csMaterialWrapper *mat = FindMaterial (str, onlyRegion);
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
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the sprite!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  spr->InitSprite ();
  return true;
}

csFrame* csLoader::LoadFrame (csSpriteTemplate* stemp, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ACTION)
    CS_TOKEN_TABLE (FRAME)
  CS_TOKEN_TABLE_END

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
      case CS_TOKEN_ACTION:
        ScanStr (params, "%s", action);
        action_specified = true;
        break;
      case CS_TOKEN_FRAME:
        ScanStr (params, "%d", &frame);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
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

iMotion* csLoader::LoadMotion (csEngine* engine, const char* fname)
{
  Engine = engine;

  iDataBuffer *databuff = System->VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open motion file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
  	CS_TOKEN_TABLE (MOTION)
  CS_TOKEN_TABLE_END

  char *name, *data;
  char *buf = **databuff;
	long cmd;

  if ((cmd=csGetObject (&buf, tokens, &name, &data)) > 0)
  {
    if (!data)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }

    iMotionManager* motionmanager=System->MotionMan;
    if (!motionmanager)
      CsPrintf (MSG_FATAL_ERROR, "No motion manager loaded!\n");
    else
    {
      iMotion* m = motionmanager->FindByName (name);
      if (!m)
      {
	m=motionmanager->AddMotion (name);
	if (LoadMotion (m, data))
	{
	  databuff->DecRef ();
	  return m;
	}
	else
	{
	  m->DecRef ();
	  databuff->DecRef ();
	  return NULL;
	}
      }
    }
  }
  databuff->DecRef ();
  return NULL;
}

bool csLoader::LoadMotion (iMotion* mot, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ANIM)
    CS_TOKEN_TABLE (FRAME)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_anim)
    CS_TOKEN_TABLE (EULER)
    CS_TOKEN_TABLE (Q)
    CS_TOKEN_TABLE (MATRIX)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_frame)
    CS_TOKEN_TABLE (LINK)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char* params2;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_ANIM:
        {
	  cmd = csGetObject (&params, tok_anim, &name, &params2);
	  switch (cmd) 
	  {
	    case CS_TOKEN_EULER:
	      {
	        csVector3 e;
	        csQuaternion quat;
	        load_vector(params2, e);
	        quat.SetWithEuler(e);
	        mot->AddAnim(quat);
	      }
	      break;
	    case CS_TOKEN_Q:
	      {
	        csQuaternion quat;
	        load_quaternion(params2, quat);
	        mot->AddAnim(quat);
	      }
	      break;
	    case CS_TOKEN_MATRIX:
	      {
	        csMatrix3 mat;
	        load_matrix(params2, mat);
	        mot->AddAnim(mat);
	      }
	      break;
	    default:
	      CsPrintf (MSG_FATAL_ERROR, "Expected MATRIX or Q instead of '%s'!\n", buf);
	      fatal_exit (0, false);
	  }     
        }
        break;
      case CS_TOKEN_FRAME:
	{
	  int framenumber;
	  ScanStr(name, "%d", &framenumber);
	  int index=mot->AddFrame(framenumber);
	  while((cmd = csGetObject (&params, tok_frame, &name, &params2))>0)
	  {
	    if(cmd!=CS_TOKEN_LINK)
	    {
	      CsPrintf (MSG_FATAL_ERROR, "Expected LINK instead of '%s'!\n", buf);
	      fatal_exit (0, false);
	    }
	    int link;
	    ScanStr(params2, "%d", &link);
	    mot->AddFrameLink(index, name, link);
	  }
        }
	break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a sprite template!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }
  return true;
}
