/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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
#include "csengine/campos.h"
#include "csparser/crossbld.h"
#include "csengine/cscoll.h"
#include "csparser/csloader.h"
#include "csengine/curve.h"
#include "csengine/dumper.h"
#include "csengine/engine.h"
#include "csengine/halo.h"
#include "csengine/keyval.h"
#include "csengine/light.h"
#include "csengine/meshobj.h"
#include "csengine/polygon.h"
#include "csengine/polytmap.h"
#include "csengine/region.h"
#include "csengine/sector.h"
#include "csparser/snddatao.h"
#include "csengine/terrobj.h"
#include "csengine/textrans.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "cssys/system.h"
#include "csutil/cfgfile.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/token.h"
#include "csutil/util.h"
#include "iutil/databuff.h"
#include "imap/reader.h"
#include "iengine/motion.h"
#include "imesh/sprite3d.h"
#include "imesh/skeleton.h"
#include "iengine/skelbone.h"
#include "isound/data.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "iterrain/object.h"
#include "iengine/terrain.h"
#include "ivideo/txtmgr.h"
#include "isys/vfs.h"
#include "igraphic/image.h"
#include "igraphic/loader.h"
#include "csgfx/csimage.h"

#include "iterrain/ddg.h"

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
  static int meshes_loaded;
  static int terrains_loaded;
  static int sounds_loaded;
  static void Init()
  {
    polygons_loaded = 0;
    portals_loaded  = 0;
    sectors_loaded  = 0;
    things_loaded   = 0;
    lights_loaded   = 0;
    curves_loaded   = 0;
    meshes_loaded  = 0;
    terrains_loaded  = 0;
    sounds_loaded   = 0;
  }
};

int csLoaderStat::polygons_loaded = 0;
int csLoaderStat::portals_loaded  = 0;
int csLoaderStat::sectors_loaded  = 0;
int csLoaderStat::things_loaded   = 0;
int csLoaderStat::lights_loaded   = 0;
int csLoaderStat::curves_loaded   = 0;
int csLoaderStat::meshes_loaded  = 0;
int csLoaderStat::terrains_loaded = 0;
int csLoaderStat::sounds_loaded   = 0;

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ACTIVE)
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (AMBIENT)
  CS_TOKEN_DEF (ANIM)
  CS_TOKEN_DEF (ATTENUATION)
  CS_TOKEN_DEF (BACK2FRONT)
  CS_TOKEN_DEF (BEZIER)
  CS_TOKEN_DEF (CAMERA)
  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (CIRCLE)
  CS_TOKEN_DEF (CLIP)
  CS_TOKEN_DEF (COLLDET)
  CS_TOKEN_DEF (COLLECTION)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (COLORS)
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
  CS_TOKEN_DEF (DYNAMIC)
  CS_TOKEN_DEF (EULER)
  CS_TOKEN_DEF (F)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FILTER)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FLAT)
  CS_TOKEN_DEF (FLATCOL)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (FOR_2D)
  CS_TOKEN_DEF (FOR_3D)
  CS_TOKEN_DEF (FRAME)
  CS_TOKEN_DEF (GOURAUD)
  CS_TOKEN_DEF (HALO)
  CS_TOKEN_DEF (HARDMOVE)
  CS_TOKEN_DEF (HEIGHT)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (INVISIBLE)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (LEN)
  CS_TOKEN_DEF (LIBRARY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (LIGHTMAP)
  CS_TOKEN_DEF (LINK)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATERIALS)
  CS_TOKEN_DEF (MATRIX)
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
  CS_TOKEN_DEF (NOLIGHTING)
  CS_TOKEN_DEF (NOSHADOWS)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (PART)
  CS_TOKEN_DEF (PERSISTENT)
  CS_TOKEN_DEF (PLANE)
  CS_TOKEN_DEF (PLUGIN)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (PORTAL)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PROCEDURAL)
  CS_TOKEN_DEF (RADIUS)
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
  CS_TOKEN_DEF (SECOND_LEN)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (SHADING)
  CS_TOKEN_DEF (SIXFACE)
  CS_TOKEN_DEF (SKY)
  CS_TOKEN_DEF (SKYDOME)
  CS_TOKEN_DEF (SOUND)
  CS_TOKEN_DEF (SOUNDS)
  CS_TOKEN_DEF (START)
  CS_TOKEN_DEF (STATBSP)
  CS_TOKEN_DEF (STATIC)
  CS_TOKEN_DEF (CLONE)
  CS_TOKEN_DEF (TEMPLATE)
  CS_TOKEN_DEF (TERRAINFACTORY)
  CS_TOKEN_DEF (TERRAINOBJ)
  CS_TOKEN_DEF (TEX)
  CS_TOKEN_DEF (TEXLEN)
  CS_TOKEN_DEF (TEXNR)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TEXTURES)
  CS_TOKEN_DEF (MAT_SET)
  CS_TOKEN_DEF (MAT_SET_SELECT)
  CS_TOKEN_DEF (MOTION)
  CS_TOKEN_DEF (Q)
  CS_TOKEN_DEF (THING)
  CS_TOKEN_DEF (TRANSFORM)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (TYPE)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (UVA)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (UV_SHIFT)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (VISTREE)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (WARP)
  CS_TOKEN_DEF (WORLD)
  CS_TOKEN_DEF (ZFILL)
  CS_TOKEN_DEF (ZNONE)
  CS_TOKEN_DEF (ZUSE)
  CS_TOKEN_DEF (ZTEST)
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
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (COLLECTION)
    CS_TOKEN_TABLE (LIGHT)
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
      case CS_TOKEN_MESHOBJ:
        {
# if 0
//@@@@@@
          ScanStr (params, "%s", str);
	  iMeshWrapper* spr = Engine->FindMeshObject (str, onlyRegion);
          if (!spr)
          {
            CsPrintf (MSG_FATAL_ERROR, "Mesh object '%s' not found!\n", str);
            fatal_exit (0, false);
          }
          collection->AddObject ((csObject*)(spr->GetPrivateObject ()));
# endif
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

void csLoader::load_thing_part (csThing* thing, csSector* sec, PSLoadInfo& info,
	csReversibleTransform& obj,
	char* name, char* buf, int vt_offset,
	bool isParent)
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
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (CONVEX)
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (HARDMOVE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (CLONE)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (CAMERA)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (ZNONE)
    CS_TOKEN_TABLE (ZUSE)
    CS_TOKEN_TABLE (ZTEST)
    CS_TOKEN_TABLE (INVISIBLE)
    CS_TOKEN_TABLE (BACK2FRONT)
    CS_TOKEN_TABLE (VISTREE)
    CS_TOKEN_TABLE (NOSHADOWS)
    CS_TOKEN_TABLE (NOLIGHTING)
    CS_TOKEN_TABLE (SKYDOME)
    CS_TOKEN_TABLE (PART)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* xname;

  long cmd;
  char* params;
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
      case CS_TOKEN_NOLIGHTING:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "NOLIGHTING flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_ENTITY_NOLIGHTING);
        break;
      case CS_TOKEN_VISTREE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "VISTREE flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_THING_VISTREE);
        break;
      case CS_TOKEN_NOSHADOWS:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "NOSHADOWS flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_ENTITY_NOSHADOWS);
        break;
      case CS_TOKEN_BACK2FRONT:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "BACK2FRONT flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_ENTITY_BACK2FRONT);
        break;
      case CS_TOKEN_INVISIBLE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "INVISIBLE flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_ENTITY_INVISIBLE);
        break;
      case CS_TOKEN_MOVEABLE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "MOVEABLE flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
        break;
      case CS_TOKEN_DETAIL:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "DETAIL flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_ENTITY_DETAIL);
        break;
      case CS_TOKEN_ZFILL:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "ZFILL flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->SetZBufMode (CS_ZBUF_FILL);
        break;
      case CS_TOKEN_ZUSE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "ZUSE flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->SetZBufMode (CS_ZBUF_USE);
        break;
      case CS_TOKEN_ZNONE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "ZNONE flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->SetZBufMode (CS_ZBUF_NONE);
        break;
      case CS_TOKEN_ZTEST:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "ZTEST flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->SetZBufMode (CS_ZBUF_TEST);
        break;
      case CS_TOKEN_CAMERA:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "CAMERA flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else thing->flags.Set (CS_ENTITY_CAMERA);
        break;
      case CS_TOKEN_CONVEX:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "CONVEX flag only for top-level thing!\n");
	  fatal_exit (0, false);
	}
        else info.is_convex = true;
        break;
      case CS_TOKEN_MOVE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "MOVE statement only for top-level thing!\n");
	  fatal_exit (0, false);
	}
	else
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
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "TEMPLATE statement only for top-level thing!\n");
	  fatal_exit (0, false);
	}
	else
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
            thing->MergeTemplate (t, sec, Engine->GetMaterials (),
	    	info.mat_set_name, info.default_material, info.default_texlen);
            info.use_mat_set = false;
	  }
          else
            thing->MergeTemplate (t, sec, info.default_material,
	    	info.default_texlen);
          csLoaderStat::polygons_loaded += t->GetNumPolygons ();
        }
        break;
      case CS_TOKEN_CLONE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "CLONE statement only for top-level thing!\n");
	  fatal_exit (0, false);
	}
	else
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
            thing->MergeTemplate (t->GetPrivateObject (), sec,
	    	Engine->GetMaterials (), info.mat_set_name,
		info.default_material, info.default_texlen);
            info.use_mat_set = false;
	  }
          else
            thing->MergeTemplate (t->GetPrivateObject (), sec,
	    	info.default_material, info.default_texlen);
          csLoaderStat::polygons_loaded += t->GetPrivateObject ()->GetNumPolygons ();
        }
        break;
      case CS_TOKEN_KEY:
        load_key (params, thing);
        break;
      case CS_TOKEN_PART:
	load_thing_part (thing, sec, info, obj, name, params,
		thing->GetNumVertices (), false);
        break;
      case CS_TOKEN_SKYDOME:
        skydome_process (*thing, name, params, info.default_material,
	    vt_offset);
        break;
      case CS_TOKEN_VERTEX:
        {
          float x, y, z;
          ScanStr (params, "%f,%f,%f", &x, &y, &z);
          thing->AddVertex (x, y, z);
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
            thing->AddVertex (cx, cy, cz);
          }
        }
        break;
      case CS_TOKEN_FOG:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "FOG statement only for top-level thing!\n");
	  fatal_exit (0, false);
	}
	else
        {
          csFog& f = thing->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case CS_TOKEN_POLYGON:
        {
	  csPolygon3D* poly3d = load_poly3d (name, params,
            info.default_material, info.default_texlen, thing, vt_offset);
	  if (poly3d)
	  {
	    thing->AddPolygon (poly3d);
	    csLoaderStat::polygons_loaded++;
	  }
        }
        break;

      case CS_TOKEN_HARDMOVE:
        if (!isParent)
	{
	  CsPrintf (MSG_FATAL_ERROR, "HARDMOVE statement only for top-level thing!\n");
	  fatal_exit (0, false);
	}
	else
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
	      thing->GetCurveVertices ());
	  Engine->curve_templates.Push (ct);
	  csCurve* p = ct->MakeCurve ();
	  p->SetName (ct->GetName ());
	  p->SetParent (thing);
	  p->SetSector (sec);
          if (!ct->GetMaterialWrapper ()) p->SetMaterialWrapper (info.default_material);
	  int j;
          for (j = 0 ; j < ct->NumVertices () ; j++)
            p->SetControlPoint (j, ct->GetVertex (j));
	  thing->AddCurve (p);
        }
        break;

      case CS_TOKEN_CURVECENTER:
        {
          csVector3 c;
          ScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
          thing->SetCurvesCenter (c);
        }
        break;
      case CS_TOKEN_CURVESCALE:
        {
	  float f;
          ScanStr (params, "%f", &f);
	  thing->SetCurvesScale (f);
          break;
        }
      case CS_TOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
          ScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
          thing->AddCurveVertex (v, t);
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
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a thing!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }
}

csThing* csLoader::load_thing (char* name, char* buf, csSector* sec,
    bool is_sky, bool is_template)
{
  csThing* thing = new csThing (Engine, is_sky, is_template);
  thing->SetName (name);

  csLoaderStat::things_loaded++;
  PSLoadInfo info;
  if (sec) thing->GetMovable ().SetSector (sec);

  csReversibleTransform obj;
  load_thing_part (thing, sec, info, obj, name, buf, 0, true);

  if (info.do_hard_trans)
    thing->HardTransform (info.hard_trans);

  thing->GetMovable ().SetTransform (obj);
  if (!(flags & CS_LOADER_NOCOMPRESS))
    thing->CompressVertices ();
  if (!(flags & CS_LOADER_NOTRANSFORM))
    thing->GetMovable ().UpdateMove ();
  if (info.is_convex || thing->GetFog ().enabled)
    thing->flags.Set (CS_ENTITY_CONVEX);
  return thing;
}


//---------------------------------------------------------------------------

csPolygon3D* csLoader::load_poly3d (char* polyname, char* buf,
  csMaterialWrapper* default_material, float default_texlen,
  csThing* parent, int vt_offset)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXNR)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (MIPMAP)
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
	      poly3d->AddVertex (list[i]+vt_offset);
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
  static bool TriedToLoadImage = false;
  iImageLoader *ImageLoader =
    QUERY_PLUGIN_ID (System, CS_FUNCID_IMGLOADER, iImageLoader);
  if (ImageLoader == NULL)
  {
    if (!TriedToLoadImage)
    {
      CsPrintf (MSG_WARNING, "Trying to load image \"%s\" without "
        "image loader\n", name);
      TriedToLoadImage = true;
    }
    return NULL;
  }

  iImage *ifile = NULL;
  iDataBuffer *buf = System->VFS->ReadFile (name);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    ImageLoader->DecRef ();
    CsPrintf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", name);
    return NULL;
  }

  ifile = ImageLoader->Load (buf->GetUint8 (), buf->GetSize (), Engine->GetTextureFormat ());
  buf->DecRef ();
  ImageLoader->DecRef ();

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

csSector* csLoader::load_sector (char* secname, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (STATBSP)
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SIXFACE)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (SKY)
    CS_TOKEN_TABLE (TERRAINOBJ)
    CS_TOKEN_TABLE (NODE)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  bool do_stat_bsp = false;

  csSector* sector = new csSector (Engine) ;
  sector->SetName (secname);

  csLoaderStat::sectors_loaded++;
  sector->SetAmbientColor (csLight::ambient_red, csLight::ambient_green, csLight::ambient_blue);

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_STATBSP:
        do_stat_bsp = true;
        break;
      case CS_TOKEN_SKY:
        Engine->skies.Push (load_thing (name, params, sector, true, false));
        break;
      case CS_TOKEN_THING:
        Engine->things.Push (load_thing (name, params, sector, false, false));
        break;
      case CS_TOKEN_SIXFACE:
        CsPrintf (MSG_WARNING, "Warning! SIXFACE statement is obsolete! Use THING instead!\n");
        break;
      case CS_TOKEN_MESHOBJ:
        {
          csMeshWrapper* sp = new csMeshWrapper (Engine);
          sp->SetName (name);
          LoadMeshObject (sp, params, sector);
          Engine->meshes.Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_TERRAINOBJ:
        {
          csTerrainWrapper *pWrapper = new csTerrainWrapper (Engine);
          pWrapper->SetName (name);
          Engine->terrains.Push (pWrapper);
          LoadTerrainObject (pWrapper, params, sector);
        }
        break;
      case CS_TOKEN_LIGHT:
        sector->AddLight ( load_statlight(name, params) );
        break;
      case CS_TOKEN_NODE:
        sector->ObjAdd ( load_node(name, params, sector) );
        break;
      case CS_TOKEN_FOG:
        {
          csFog& f = sector->GetFog ();
          f.enabled = true;
          ScanStr (params, "%f,%f,%f,%f", &f.red, &f.green, &f.blue, &f.density);
        }
        break;
      case CS_TOKEN_KEY:
      {
        load_key(params, sector);
        break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing a sector!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (!(flags & CS_LOADER_NOBSP))
    if (do_stat_bsp) sector->UseStaticTree ();
  return sector;
}

void csLoader::skydome_process (csThing& thing, char* name, char* buf,
        csMaterialWrapper* material, int vt_offset)
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
      new_vertices[j] = thing.AddVertex (
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
      p->SetParent (&thing);
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
      thing.AddPolygon (p);
      csLoaderStat::polygons_loaded++;
      sprintf (end_poly_name, "%d_%d_B", i, j);
      p = new csPolygon3D (material);
      p->SetName (poly_name);
      p->SetParent (&thing);
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
      thing.AddPolygon (p);
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
  int top_vertex = thing.AddVertex (0, vert_radius, 0);
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
    p->SetParent (&thing);
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
    thing.AddPolygon (p);
    csLoaderStat::polygons_loaded++;
  }
}

//---------------------------------------------------------------------------

iSoundHandle* csLoader::LoadSoundHandle(const char* filename) {
  /* @@@ get the needed plugin interfaces:
   * when moving the loader to a plug-in, this should be done
   * at initialization, and pointers shouldn't be DecRef'ed here.
   * The 'no sound loader' warning should also be printed at
   * initialization.
   * I marked all cases with '###'.
   */

  /* get format descriptor */
  /* ### */iSoundRender *SoundRender =
    QUERY_PLUGIN_ID(System, CS_FUNCID_SOUND, iSoundRender);
  /* ### */if (!SoundRender) return NULL;

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
  iSoundLoader *SoundLoader =
    QUERY_PLUGIN_ID(System, CS_FUNCID_SNDLOADER, iSoundLoader);
  if (!SoundLoader) {
    if (!TriedToLoadSound) {
      TriedToLoadSound = true;
      CsPrintf(MSG_WARNING,
        "Trying to load sound without sound loader.\n");
    }
    return NULL;
  }

  /* load the sound */
  iSoundData *Sound = SoundLoader->LoadSound(buf->GetUint8 (), buf->GetSize ());
  buf->DecRef ();
  /* ### */SoundLoader->DecRef();

  /* check for valid sound data */
  if (!Sound) {
    CsPrintf (MSG_WARNING, "Cannot create sound data from file \"%s\"!\n", filename);
    return NULL;
  }

  /* register the sound */
  iSoundHandle *hdl = SoundRender->RegisterSound(Sound);
  /* ### */SoundRender->DecRef();

  return hdl;
}


csSoundDataObject *csLoader::LoadSoundObject (csEngine* engine,
  char* name, const char* fname) {

  Engine=engine;
  /* load the sound handle */
  iSoundHandle *Sound = LoadSoundHandle(fname);

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
    CS_TOKEN_TABLE (THING)
    CS_TOKEN_TABLE (SIXFACE)
    CS_TOKEN_TABLE (LIBRARY)
    CS_TOKEN_TABLE (START)
    CS_TOKEN_TABLE (SOUNDS)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MOTION)
    CS_TOKEN_TABLE (REGION)
    CS_TOKEN_TABLE (TERRAINFACTORY)
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
            csMeshFactoryWrapper* t = (csMeshFactoryWrapper*)Engine->mesh_factories.FindByName (name);
            if (!t)
            {
              t = new csMeshFactoryWrapper ();
              t->SetName (name);
              Engine->mesh_factories.Push (t);
            }
            LoadMeshObjectFactory (t, params);
          }
	  break;
        case CS_TOKEN_TERRAINFACTORY:
        {
          csTerrainFactoryWrapper* pWrapper = (csTerrainFactoryWrapper*)Engine->terrain_factories.FindByName( name );
          if (!pWrapper)
          {
            pWrapper = new csTerrainFactoryWrapper ();
            pWrapper->SetName( name );
            Engine->terrain_factories.Push (pWrapper);
          }
          LoadTerrainObjectFactory (pWrapper, params);
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
        case CS_TOKEN_THING:
          if (!Engine->thing_templates.FindByName (name))
            Engine->thing_templates.Push (load_thing (name, params, NULL, false, true));
          break;
        case CS_TOKEN_SIXFACE:
          CsPrintf (MSG_WARNING, "Warning! SIXFACE statement is obsolete! Use THING instead!\n");
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
          CsPrintf (MSG_WARNING, "Warning! ROOM statement is obsolete! Use SECTOR instead!\n");
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
    int st = s->GetNumThings ();
    int j = 0;
    while (j < st)
    {
      csThing* ps = s->GetThing (j);
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

  iConfigFile *cfg = new csConfigFile ("map.cfg", System->VFS);
  if (cfg)
  {
    csLightMap::SetLightCellSize (cfg->GetInt ("Engine.Lighting.LightmapSize",
    	csLightMap::lightcell_size));
    cfg->DecRef();
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
    CsPrintf (MSG_INITIALIZATION, "  %d sectors, %d things, %d meshes, \n", csLoaderStat::sectors_loaded,
      csLoaderStat::things_loaded, csLoaderStat::meshes_loaded);
    CsPrintf (MSG_INITIALIZATION, "  %d curves, %d lights, %d sounds.\n", csLoaderStat::curves_loaded,
      csLoaderStat::lights_loaded, csLoaderStat::sounds_loaded);
  } /* endif */

  buf->DecRef ();
  loaded_plugins.DeleteAll ();

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadTextures (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
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
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (THING)
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
        case CS_TOKEN_MESHOBJ:
          {
            csMeshFactoryWrapper* t = (csMeshFactoryWrapper*)Engine->mesh_factories.FindByName (name);
            if (!t)
            {
              t = new csMeshFactoryWrapper ();
              t->SetName (name);
              Engine->mesh_factories.Push (t);
            }
            LoadMeshObjectFactory (t, params);
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
  
  loaded_plugins.DeleteAll ();
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
        iSoundHandle *snd = csSoundDataObject::GetSound (*Engine, name);
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

// @@@ MEMORY LEAK!!! We should unload all the plugins we load here.
csLoader::csLoadedPluginVector csLoader::loaded_plugins;

csLoader::LoadedPlugin::LoadedPlugin (const char *theName, iPlugIn *thePlugin)
{ 
  name = strnew (theName); plugin = thePlugin; 
}

csLoader::LoadedPlugin::~LoadedPlugin ()
{ 
  delete [] name; 
  plugin->DecRef (); 
}                                                                                  
//---------------------------------------------------------------------------

csMeshFactoryWrapper* csLoader::LoadMeshObjectFactory (csEngine* engine,
	const char* fname)
{
  Engine = engine;

  iDataBuffer *databuff = System->VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open mesh object file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (MESHOBJ)
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

    csMeshFactoryWrapper* tmpl = new csMeshFactoryWrapper ();
    tmpl->SetName (name);
    if (LoadMeshObjectFactory (tmpl, data))
    {
      Engine->mesh_factories.Push (tmpl);
      databuff->DecRef ();
      loaded_plugins.DeleteAll ();
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

bool csLoader::LoadMeshObjectFactory (csMeshFactoryWrapper* stemp, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  iLoaderPlugIn* plug = NULL;
  str[0] = 0;

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
          CsPrintf (MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", str);
          fatal_exit (0, false);
	}
	else
	{
	  iBase* mof = stemp->GetMeshObjectFactory();
	  mof = plug->Parse (params, csEngine::current_iengine, mof);
	  if (!mof)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Plugin '%s' did not return a factory!\n",
	    	str);
	    fatal_exit (0, false);
	  }
	  else
	  {
		// This is a glorified type cast.
//	    iMeshObjectFactory* mof2 = QUERY_INTERFACE (mof, iMeshObjectFactory);
	    stemp->SetMeshObjectFactory ((iMeshObjectFactory *)mof);
	    mof->DecRef ();
	  }
	}
        break;

      case CS_TOKEN_MATERIAL:
        {
          ScanStr (params, "%s", str);
          csMaterialWrapper *mat = FindMaterial (str, onlyRegion);
          if (mat)
	  {
	    iSprite3DFactoryState* state = QUERY_INTERFACE (
	    	stemp->GetMeshObjectFactory (),
		iSprite3DFactoryState);
            iMaterialWrapper* imat = QUERY_INTERFACE (mat, iMaterialWrapper);
            state->SetMaterialWrapper (imat);
	    imat->DecRef ();
	    state->DecRef ();
	  }
          else
          {
            CsPrintf (MSG_FATAL_ERROR, "Material `%s' not found!\n", str);
            fatal_exit (0, true);
          }
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
  	  iMeshObjectType* type = QUERY_PLUGIN_CLASS (System, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
  	  if (!type)
  	  {
    	    type = LOAD_PLUGIN (System, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
    	    printf ("Load TYPE plugin crystalspace.mesh.object.sprite.3d\n");
  	  }
	  iMeshObjectFactory* fact = type->NewFactory ();
	  stemp->SetMeshObjectFactory (fact);
	  fact->DecRef ();
	  iSprite3DFactoryState* state = QUERY_INTERFACE (fact, iSprite3DFactoryState);
	  csCrossBuild_SpriteTemplateFactory builder;
	  builder.CrossBuild (fact, *filedata);
//	  state->DecRef ();
	  delete filedata;
        }
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (str);
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "MeshLdr", iLoaderPlugIn);
	    if (plug) loaded_plugins.NewPlugIn (str, plug);
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

bool csLoader::LoadMeshObject (csMeshWrapper* mesh, char* buf, csSector* sector)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (NOLIGHTING)
    CS_TOKEN_TABLE (NOSHADOWS)
    CS_TOKEN_TABLE (BACK2FRONT)
    CS_TOKEN_TABLE (INVISIBLE)
    CS_TOKEN_TABLE (DETAIL)
    CS_TOKEN_TABLE (ZFILL)
    CS_TOKEN_TABLE (ZNONE)
    CS_TOKEN_TABLE (ZUSE)
    CS_TOKEN_TABLE (ZTEST)
    CS_TOKEN_TABLE (CAMERA)
    CS_TOKEN_TABLE (CONVEX)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_matvec)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  str[0] = 0;

  csLoaderStat::meshes_loaded++;
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
      case CS_TOKEN_NOLIGHTING:
        mesh->flags.Set (CS_ENTITY_NOLIGHTING);
        break;
      case CS_TOKEN_NOSHADOWS:
        mesh->flags.Set (CS_ENTITY_NOSHADOWS);
        break;
      case CS_TOKEN_BACK2FRONT:
        mesh->flags.Set (CS_ENTITY_BACK2FRONT);
        break;
      case CS_TOKEN_INVISIBLE:
        mesh->flags.Set (CS_ENTITY_INVISIBLE);
        break;
      case CS_TOKEN_DETAIL:
        mesh->flags.Set (CS_ENTITY_DETAIL);
        break;
      case CS_TOKEN_ZFILL:
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case CS_TOKEN_ZUSE:
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case CS_TOKEN_ZNONE:
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case CS_TOKEN_ZTEST:
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case CS_TOKEN_CAMERA:
        mesh->flags.Set (CS_ENTITY_CAMERA);
        break;
      case CS_TOKEN_CONVEX:
        mesh->flags.Set (CS_ENTITY_CONVEX);
        break;
      case CS_TOKEN_KEY:
        load_key (params, mesh);
        break;
      case CS_TOKEN_MESHOBJ:
        {
          csMeshWrapper* sp = new csMeshWrapper (mesh);
          sp->SetName (name);
          LoadMeshObject (sp, params, sector);
          mesh->GetChildren ().Push (sp);
          sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
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
          CsPrintf (MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", str);
          fatal_exit (0, false);
	}
	else
	{
	  iBase* mo = plug->Parse (params, csEngine::current_iengine, NULL);
	  iMeshObject* mo2 = QUERY_INTERFACE (mo, iMeshObject);
	  mesh->SetMeshObject (mo2);
	  mo2->DecRef ();
	  mo->DecRef ();
	  // This is a bit ugly but I don't know another way to do it.
	  // Here we find the csMeshFactoryWrapper which is holding the
	  // reference to the factory that created this object.
	  iMeshObjectFactory* fact = mo2->GetFactory ();
	  int i;
	  for (i = 0 ; i < Engine->mesh_factories.Length () ; i++)
	  {
	    csMeshFactoryWrapper* factwrap = (csMeshFactoryWrapper*)(Engine->mesh_factories[i]);
	    if (factwrap->GetMeshObjectFactory () == fact)
	    {
	      mesh->SetFactory (factwrap);
	      break;
	    }
	  }
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (str);
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "MeshLdr", iLoaderPlugIn);
	    if (plug) loaded_plugins.NewPlugIn (str, plug);
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

bool csLoader::LoadTerrainObjectFactory (csTerrainFactoryWrapper* pWrapper, char *pBuf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PARAMS)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char *params;
  char *pStr = new char[255];
  iLoaderPlugIn *iPlugIn = NULL;
  pStr[0] = 0;

  while ((cmd = csGetObject (&pBuf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", pBuf);
      fatal_exit( 0, false );
    }
    switch (cmd)
    {
      case CS_TOKEN_PARAMS:
	if (!iPlugIn)
	{
          CsPrintf( MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", pStr );
          fatal_exit (0, false);
	}
	else
	{
          // use the plugin to parse through the parameters the engine is passed also
          // a pointer to the factory is returned
	  iBase *pBaseFactory = iPlugIn->Parse (params, csEngine::current_iengine,NULL);
          // if we couldn't get the factory leave
	  if (!pBaseFactory)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Plugin '%s' did not return a factory!\n",
	    	    pStr);
	    fatal_exit (0, false);
	  }
	  else
	  {
            // convert the iBase to iTerrainObjectFactory
	    iTerrainObjectFactory *pFactory = 
            QUERY_INTERFACE (pBaseFactory, iTerrainObjectFactory);
            // set the factory into the csTerrainFactoryWrapper
	    pWrapper->SetTerrainObjectFactory (pFactory);
	    pFactory->DecRef ();
	  }
	}
        break;
      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", pStr);
	  iPlugIn = (iLoaderPlugIn*)loaded_plugins.FindPlugIn( pStr );
	  if (!iPlugIn)
	  {
	    printf ("Plugin '%s' loaded!\n", pStr);
	    iPlugIn = LOAD_PLUGIN (System, pStr, "TerrainLdr", iLoaderPlugIn);
	    if (iPlugIn)
              loaded_plugins.NewPlugIn (pStr, iPlugIn);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR, "Token '%s' not found while parsing the a terrain factory!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadTerrainObject (csTerrainWrapper *pWrapper, char *pBuf, csSector* sector)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char *pStr = new char[255];
  pStr[0] = 0;

  csLoaderStat::terrains_loaded++;
  iLoaderPlugIn* iPlugIn = NULL;

  while ((cmd = csGetObject (&pBuf, commands, &name, &params) ) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", pBuf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_PARAMS:
        if (!iPlugIn)
        {
          CsPrintf( MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", pStr );
          fatal_exit (0, false);
        }
        else
        {
          // use the plugin to parse through the parameters the engine is passed also
          // a pointer to the factory is returned
	  iBase *iBaseObject = iPlugIn->Parse (params, csEngine::current_iengine, NULL);
          // if we couldn't get the factory leave
	  if (!iBaseObject)
	  {
	    CsPrintf (MSG_FATAL_ERROR, "Plugin '%s' did not return a factory!\n",
	    	      pStr);
	    fatal_exit (0, false);
	  }
	  else
	  {
            // convert the iBase to iTerrainObject
            iTerrainObject *iTerrObj = QUERY_INTERFACE (iBaseObject, iTerrainObject);
            pWrapper->SetTerrainObject (iTerrObj);

            iTerrObj->DecRef ();
            iBaseObject->DecRef ();
	  }
        }
        break;
      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", pStr);
	  iPlugIn = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (pStr);
	  if (!iPlugIn)
	  {
	    printf ("Plugin '%s' loaded!\n", pStr);
	    iPlugIn = LOAD_PLUGIN (System, pStr, "TerrainLdr", iLoaderPlugIn);
	    if (iPlugIn)
              loaded_plugins.NewPlugIn (pStr, iPlugIn);
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

  // add new terrain to sector
  sector->AddTerrain (pWrapper);

  return true;
}

//---------------------------------------------------------------------------

csThing * csLoader::LoadThing (csEngine* engine, const char* fname)
{
  Engine = engine;

  iDataBuffer *databuff = System->VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open thing file file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (THING)
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

    csThing * thng = load_thing( name, buf, NULL, false, false );
    return thng;
  }
  databuff->DecRef ();
  return NULL;
}

//---------------------------------------------------------------------------
csThing * csLoader::LoadThingTemplate (csEngine* engine, const char* fname)
{
  Engine = engine;

  iDataBuffer *databuff = System->VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    CsPrintf (MSG_FATAL_ERROR, "Could not open thing file file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (THING)
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

    csThing * thng = load_thing( name, buf, NULL, false, true );
    return thng;
  }
  databuff->DecRef ();
  return NULL;
}

//---------------------------------------------------------------------------

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
