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
  CS_TOKEN_DEF (ADDON)
  CS_TOKEN_DEF (AMBIENT)
  CS_TOKEN_DEF (ANIM)
  CS_TOKEN_DEF (ATTENUATION)
  CS_TOKEN_DEF (BACK2FRONT)
  CS_TOKEN_DEF (CAMERA)
  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (COLLECTION)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (CONVEX)
  CS_TOKEN_DEF (CULLER)
  CS_TOKEN_DEF (DETAIL)
  CS_TOKEN_DEF (DIFFUSE)
  CS_TOKEN_DEF (DITHER)
  CS_TOKEN_DEF (DYNAMIC)
  CS_TOKEN_DEF (EULER)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FILTER)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (FOR_2D)
  CS_TOKEN_DEF (FOR_3D)
  CS_TOKEN_DEF (FRAME)
  CS_TOKEN_DEF (HALO)
  CS_TOKEN_DEF (HARDMOVE)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (INVISIBLE)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (LIBRARY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (LINK)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATERIALS)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MESHFACT)
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (MIPMAP)
  CS_TOKEN_DEF (MOVE)
  CS_TOKEN_DEF (NODE)
  CS_TOKEN_DEF (NOLIGHTING)
  CS_TOKEN_DEF (NOSHADOWS)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (PERSISTENT)
  CS_TOKEN_DEF (PLUGIN)
  CS_TOKEN_DEF (PLUGINS)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (PROCEDURAL)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (REFLECTION)
  CS_TOKEN_DEF (REGION)
  CS_TOKEN_DEF (RENDERPRIORITIES)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (SOUND)
  CS_TOKEN_DEF (SOUNDS)
  CS_TOKEN_DEF (START)
  CS_TOKEN_DEF (TERRAINFACTORY)
  CS_TOKEN_DEF (TERRAINOBJ)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TEXTURES)
  CS_TOKEN_DEF (MAT_SET)
  CS_TOKEN_DEF (MOTION)
  CS_TOKEN_DEF (Q)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (V)
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

//---------------------------------------------------------------------------

csCollection* csLoader::load_collection (char* name, char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (ADDON)
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
      case CS_TOKEN_ADDON:
        CsPrintf (MSG_WARNING, "ADDON not yet supported in collection!\n");
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
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (POSITION)
  CS_TOKEN_TABLE_END

  csMapNode* pNode = new csMapNode (name);
  pNode->SetSector (sec);

  long  cmd;
  char* xname;
  char* params;

  float x = 0;
  float y = 0;
  float z = 0;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
        CsPrintf (MSG_WARNING, "ADDON not yet supported in node!\n");
      	break;
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

void csLoader::txt_process (char *name, char* buf)
{
  if (!GlobalLoader) return;

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

  iImage *image = GlobalLoader->LoadImage (filename);
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

//---------------------------------------------------------------------------

csSector* csLoader::load_sector (char* secname, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (CULLER)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (TERRAINOBJ)
    CS_TOKEN_TABLE (NODE)
    CS_TOKEN_TABLE (KEY)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  bool do_culler = false;
  char bspname[100];

  csSector* sector = new csSector (Engine) ;
  sector->SetName (secname);

  csLoaderStat::sectors_loaded++;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
	LoadAddOn (params, &(sector->scfiSector));
      	break;
      case CS_TOKEN_CULLER:
        do_culler = true;
	if (!ScanStr (params, "%s", bspname))
	{
          CsPrintf (MSG_FATAL_ERROR,
	  	"CULLER expects the name of a mesh object!\n");
          fatal_exit (0, false);
	}
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
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing a sector!\n", csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (!(flags & CS_LOADER_NOBSP))
    if (do_culler) sector->UseCuller (bspname);
  return sector;
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

static void ResolvePortalSectors (csEngine* Engine, bool onlyRegion,
	csThing* ps)
{
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

bool csLoader::LoadMap (char* buf, bool onlyRegion)
{
  ::onlyRegion = onlyRegion;

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (WORLD)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (SECTOR)
    CS_TOKEN_TABLE (COLLECTION)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (MAT_SET)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (LIBRARY)
    CS_TOKEN_TABLE (START)
    CS_TOKEN_TABLE (SOUNDS)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MOTION)
    CS_TOKEN_TABLE (REGION)
    CS_TOKEN_TABLE (TERRAINFACTORY)
    CS_TOKEN_TABLE (RENDERPRIORITIES)
    CS_TOKEN_TABLE (PLUGINS)
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
        CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n",
		data);
        fatal_exit (0, false);
      }
      switch (cmd)
      {
        case CS_TOKEN_RENDERPRIORITIES:
	  Engine->ClearRenderPriorities ();
	  LoadRenderPriorities (params);
	  break;
        case CS_TOKEN_ADDON:
	  LoadAddOn (params, (iEngine*)Engine);
      	  break;
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
        case CS_TOKEN_MESHFACT:
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
            csTerrainFactoryWrapper* pWrapper =(csTerrainFactoryWrapper*)
	  	Engine->terrain_factories.FindByName( name );
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
        case CS_TOKEN_SECTOR:
          if (!Engine->FindSector (name, onlyRegion))
            Engine->sectors.Push (load_sector (name, params));
          break;
        case CS_TOKEN_COLLECTION:
          Engine->collections.Push (load_collection (name, params));
          break;
	case CS_TOKEN_MAT_SET:
          if (!LoadMaterials (params, name))
            return false;
          break;
	case CS_TOKEN_PLUGINS:
	  if (!LoadPlugins (params))
	    return false;
	  break;
        case CS_TOKEN_TEXTURES:
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
    int st = s->GetNumberMeshes ();
    int j = 0;
    while (j < st)
    {
      csMeshWrapper* ps = s->GetMesh (j);
      j++;
      // @@@ UGLY!!!! NEED A MORE GENERAL SOLUTION!
      iThing* ith = QUERY_INTERFACE (ps->GetMeshObject (), iThing);
      if (ith)
      {
        ResolvePortalSectors (Engine, onlyRegion, ith->GetPrivateObject ());
	ith->DecRef ();
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

bool csLoader::LoadPlugins (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
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
      case CS_TOKEN_PLUGIN:
	ScanStr (params, "%s", str);
	loaded_plugins.NewPlugIn (name, str, NULL);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing plugin descriptors!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }

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
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing textures!\n", csGetLastOffender ());
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
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (TEXTURES)
    CS_TOKEN_TABLE (MATERIALS)
    CS_TOKEN_TABLE (MESHFACT)
    CS_TOKEN_TABLE (SOUNDS)
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
        case CS_TOKEN_ADDON:
	  LoadAddOn (params, (iEngine*)Engine);
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
        case CS_TOKEN_MESHFACT:
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
  if (!GlobalLoader) return NULL;

  Engine = engine;
  iImage *image = GlobalLoader->LoadImage (fname);
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

iPlugIn* csLoader::csLoadedPluginVector::FindPlugIn (const char* name,
	const char* classID)
{
  LoadedPlugin* pl = FindPlugInPrivate (name);
  if (!pl) return NULL;
  if (!pl->plugin)
  {
    // First load the plugin.
    pl->plugin = LOAD_PLUGIN (System, pl->name, classID, iLoaderPlugIn);
  }
  return pl->plugin;
}

csLoader::LoadedPlugin::LoadedPlugin (const char* shortName,
	const char *theName, iPlugIn *thePlugin)
{ 
  if (shortName) short_name = strnew (shortName);
  else short_name = NULL;
  name = strnew (theName);
  plugin = thePlugin; 
}

csLoader::LoadedPlugin::~LoadedPlugin ()
{ 
  delete [] short_name; 
  delete [] name; 
  if (plugin) plugin->DecRef (); 
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
    CsPrintf (MSG_FATAL_ERROR,
    	"Could not open mesh object file \"%s\" on VFS!\n", fname);
    return NULL;
  }

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (MESHFACT)
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
    CS_TOKEN_TABLE (ADDON)
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
      case CS_TOKEN_ADDON:
	LoadAddOn (params, &(stemp->scfiMeshFactoryWrapper));
      	break;
      case CS_TOKEN_PARAMS:
	if (!plug)
	{
          CsPrintf (MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", str);
          fatal_exit (0, false);
	}
	else
	{
	  iBase* mof = plug->Parse (params, csEngine::current_iengine,
	  	&(stemp->scfiMeshFactoryWrapper));
	  if (!mof)
	  {
	    CsPrintf (MSG_FATAL_ERROR,
	    	"Plugin '%s' did not return a factory (mesh fact '%s')!\n",
		str, stemp->GetName ());
	    fatal_exit (0, false);
	  }
	  else
	  {
	    stemp->SetMeshObjectFactory ((iMeshObjectFactory *)mof);
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
  	  iMeshObjectType* type = QUERY_PLUGIN_CLASS (System,
	  	"crystalspace.mesh.object.sprite.3d", "MeshObj",
		iMeshObjectType);
  	  if (!type)
  	  {
    	    type = LOAD_PLUGIN (System, "crystalspace.mesh.object.sprite.3d",
	    	"MeshObj", iMeshObjectType);
    	    printf ("Load TYPE plugin crystalspace.mesh.object.sprite.3d\n");
  	  }
	  iMeshObjectFactory* fact = type->NewFactory ();
	  stemp->SetMeshObjectFactory (fact);
	  fact->DecRef ();
	  csCrossBuild_SpriteTemplateFactory builder;
	  builder.CrossBuild (fact, *filedata);
	  delete filedata;
        }
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (str, "MeshLdr");
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "MeshLdr", iLoaderPlugIn);
	    if (plug) loaded_plugins.NewPlugIn (NULL, str, plug);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing the a mesh factory!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadMeshObject (csMeshWrapper* mesh, char* buf, csSector* sector)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (MESHOBJ)
    CS_TOKEN_TABLE (MOVE)
    CS_TOKEN_TABLE (HARDMOVE)
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
    CS_TOKEN_TABLE (PRIORITY)
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
  char priority[255]; priority[0] = 0;

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
      case CS_TOKEN_PRIORITY:
	ScanStr (params, "%s", priority);
	break;
      case CS_TOKEN_ADDON:
	LoadAddOn (params, &(mesh->scfiMeshWrapper));
      	break;
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
        if (!priority[0]) strcpy (priority, "wall");
        mesh->SetZBufMode (CS_ZBUF_FILL);
        break;
      case CS_TOKEN_ZUSE:
        if (!priority[0]) strcpy (priority, "object");
        mesh->SetZBufMode (CS_ZBUF_USE);
        break;
      case CS_TOKEN_ZNONE:
        if (!priority[0]) strcpy (priority, "sky");
        mesh->SetZBufMode (CS_ZBUF_NONE);
        break;
      case CS_TOKEN_ZTEST:
        if (!priority[0]) strcpy (priority, "alpha");
        mesh->SetZBufMode (CS_ZBUF_TEST);
        break;
      case CS_TOKEN_CAMERA:
        if (!priority[0]) strcpy (priority, "sky");
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
          if (sector) sp->GetMovable ().SetSector (sector);
	  sp->GetMovable ().UpdateMove ();
        }
        break;
      case CS_TOKEN_HARDMOVE:
        {
          char* params2;
	  csReversibleTransform tr;
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR,
	      	"Expected parameters instead of '%s'!\n", params);
              fatal_exit (0, false);
            }
            switch (cmd)
            {
              case CS_TOKEN_MATRIX:
              {
		csMatrix3 m;
                load_matrix (params2, m);
                tr.SetT2O (m);
                break;
              }
              case CS_TOKEN_V:
              {
		csVector3 v;
                load_vector (params2, v);
		tr.SetOrigin (v);
                break;
              }
            }
          }
	  mesh->HardTransform (tr);
        }
        break;
      case CS_TOKEN_MOVE:
        {
          char* params2;
          mesh->GetMovable ().SetTransform (csMatrix3 ());     // Identity
          mesh->GetMovable ().SetPosition (csVector3 (0));
          while ((cmd = csGetObject (&params, tok_matvec, &name, &params2)) > 0)
          {
            if (!params2)
            {
              CsPrintf (MSG_FATAL_ERROR,
	      	"Expected parameters instead of '%s'!\n", params);
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
	  iBase* mo = plug->Parse (params, csEngine::current_iengine,
	  	&(mesh->scfiMeshWrapper));
	  iMeshObject* mo2 = QUERY_INTERFACE (mo, iMeshObject);
	  mesh->SetMeshObject (mo2);
	  mo2->DecRef ();
	  mo->DecRef ();
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (str, "MeshLdr");
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "MeshLdr", iLoaderPlugIn);
	    if (plug) loaded_plugins.NewPlugIn (NULL, str, plug);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing the mesh object!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }

  if (!priority[0]) strcpy (priority, "object");
  mesh->SetRenderPriority (Engine->GetRenderPriority (priority));

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadTerrainObjectFactory (csTerrainFactoryWrapper* pWrapper,
	char *pBuf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
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
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n",
      	pBuf);
      fatal_exit( 0, false );
    }
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
	LoadAddOn (params, &(pWrapper->scfiTerrainFactoryWrapper));
      	break;
      case CS_TOKEN_PARAMS:
	if (!iPlugIn)
	{
          CsPrintf( MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", pStr );
          fatal_exit (0, false);
	}
	else
	{
          // Use the plugin to parse through the parameters the engine is
	  // passed also a pointer to the factory is returned.
	  iBase *pBaseFactory = iPlugIn->Parse (params,
	  	csEngine::current_iengine,
		&(pWrapper->scfiTerrainFactoryWrapper));
          // If we couldn't get the factory leave.
	  if (!pBaseFactory)
	  {
	    CsPrintf (MSG_FATAL_ERROR,
	    	"Plugin '%s' did not return a factory!\n", pStr);
	    fatal_exit (0, false);
	  }
	  else
	  {
            // Convert the iBase to iTerrainObjectFactory.
	    iTerrainObjectFactory *pFactory = 
              QUERY_INTERFACE (pBaseFactory, iTerrainObjectFactory);
            // Set the factory into the csTerrainFactoryWrapper.
	    pWrapper->SetTerrainObjectFactory (pFactory);
	    pFactory->DecRef ();
	  }
	}
        break;
      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", pStr);
	  iPlugIn = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (pStr,
	  	"TerrainLdr");
	  if (!iPlugIn)
	  {
	    printf ("Plugin '%s' loaded!\n", pStr);
	    iPlugIn = LOAD_PLUGIN (System, pStr, "TerrainLdr", iLoaderPlugIn);
	    if (iPlugIn)
              loaded_plugins.NewPlugIn (NULL, pStr, iPlugIn);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing the a terrain factory!\n",
        csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

bool csLoader::LoadTerrainObject (csTerrainWrapper *pWrapper,
	char *pBuf, csSector* sector)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ADDON)
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
      CsPrintf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n",
      	pBuf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_ADDON:
	LoadAddOn (params, &(pWrapper->scfiTerrainWrapper));
      	break;
      case CS_TOKEN_PARAMS:
        if (!iPlugIn)
        {
          CsPrintf( MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", pStr );
          fatal_exit (0, false);
        }
        else
        {
          // Use the plugin to parse through the parameters the engine
	  // is passed also a pointer to the factory is returned.
	  iBase *iBaseObject = iPlugIn->Parse (params,
	  	csEngine::current_iengine, &(pWrapper->scfiTerrainWrapper));
          // if we couldn't get the factory leave
	  if (!iBaseObject)
	  {
	    CsPrintf (MSG_FATAL_ERROR,
	    	"Plugin '%s' did not return a factory!\n", pStr);
	    fatal_exit (0, false);
	  }
	  else
	  {
            // convert the iBase to iTerrainObject
            iTerrainObject *iTerrObj = QUERY_INTERFACE (iBaseObject,
	    	iTerrainObject);
            pWrapper->SetTerrainObject (iTerrObj);

            iTerrObj->DecRef ();
            iBaseObject->DecRef ();
	  }
        }
        break;
      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", pStr);
	  iPlugIn = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (pStr,
	  	"TerrainLdr");
	  if (!iPlugIn)
	  {
	    printf ("Plugin '%s' loaded!\n", pStr);
	    iPlugIn = LOAD_PLUGIN (System, pStr, "TerrainLdr", iLoaderPlugIn);
	    if (iPlugIn)
              loaded_plugins.NewPlugIn (NULL, pStr, iPlugIn);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing the mesh object!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }

  // add new terrain to sector
  sector->AddTerrain (pWrapper);

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadAddOn (char* buf, iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PLUGIN)
    CS_TOKEN_TABLE (PARAMS)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];
  str[0] = 0;

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
          CsPrintf (MSG_FATAL_ERROR, "Could not load plugin '%s'!\n", str);
          fatal_exit (0, false);
	}
	else
	{
	  plug->Parse (params, csEngine::current_iengine, context);
	}
        break;

      case CS_TOKEN_PLUGIN:
	{
	  ScanStr (params, "%s", str);
	  plug = (iLoaderPlugIn*)loaded_plugins.FindPlugIn (str, "Loader");
	  if (!plug)
	  {
	    printf ("Plugin '%s' loaded!\n", str);
	    plug = LOAD_PLUGIN (System, str, "Loader", iLoaderPlugIn);
	    if (plug) loaded_plugins.NewPlugIn (NULL, str, plug);
	  }
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing the plugin!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

bool csLoader::LoadRenderPriorities (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PRIORITY)
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
      case CS_TOKEN_PRIORITY:
      {
        long pri;
	char sorting[100];
	ScanStr (params, "%d,%s", &pri, sorting);
	if (!strcmp (sorting, "BACK2FRONT"))
	{
	}
	else if (!strcmp (sorting, "FRONT2BACK"))
	{
	}
	else if (!strcmp (sorting, "NONE"))
	{
	}
	else
	{
	  CsPrintf (MSG_FATAL_ERROR,
	  	"Unknown sorting attribute '%s' for the render priority!\n\
Use BACK2FRONT, FRONT2BACK, or NONE\n", sorting);
	  fatal_exit (0, false);
	}
	Engine->RegisterRenderPriority (name, pri);
        break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    CsPrintf (MSG_FATAL_ERROR,
    	"Token '%s' not found while parsing the render priorities!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }

  return true;
}

//---------------------------------------------------------------------------

csMeshWrapper * csLoader::LoadMeshObject (csEngine* engine, const char* fname)
{
  Engine = engine;

  iDataBuffer *databuff = System->VFS->ReadFile (fname);

  if (!databuff || !databuff->GetSize ())
  {
    if (databuff) databuff->DecRef ();
    CsPrintf (MSG_FATAL_ERROR,
    	"Could not open mesh object file file \"%s\" on VFS!\n", fname);
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

    csMeshWrapper* sp = new csMeshWrapper (Engine);
    sp->SetName (name);
    LoadMeshObject (sp, buf, NULL);
    return sp;
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


/************ iLoaderNew implementation **************/

iLoaderNew *csLoader::GlobalLoader = NULL;

IMPLEMENT_IBASE(csLoader);
  IMPLEMENTS_INTERFACE(iLoaderNew);
  IMPLEMENTS_INTERFACE(iPlugIn);
IMPLEMENT_IBASE_END;

IMPLEMENT_FACTORY(csLoader);

EXPORT_CLASS_TABLE (lvlload)
  EXPORT_CLASS_DEP (csLoader, "crystalspace.level.loader",
    "Level and library file loader", "crystalspace.kernel., "
    "crystalspace.sound.loader., crystalspace.image.loader, "
    "crystalspace.mesh.loader., crystalspace.terrain.loader., "
    "crystalspace.engine.core")
EXPORT_CLASS_TABLE_END

csLoader::csLoader(iBase *p)
{
  CONSTRUCT_IBASE(p);

  tmpWrap.VFS = NULL;
  tmpWrap.ImageLoader = NULL;
  tmpWrap.SoundLoader = NULL;
  tmpWrap.Engine = NULL;

  GlobalLoader = this;
}

csLoader::~csLoader()
{
  DEC_REF(tmpWrap.VFS);
  DEC_REF(tmpWrap.ImageLoader);
  DEC_REF(tmpWrap.SoundLoader);
  DEC_REF(tmpWrap.Engine);

  GlobalLoader = NULL;
}

#define GET_PLUGIN(var, func, intf, msgname)		\
  var = QUERY_PLUGIN_ID(System, func, intf);		\
  if (!var)						\
    System->Printf(MSG_INITIALIZATION,			\
      "  Failed to query "msgname" plug-in.\n");	\


bool csLoader::Initialize(iSystem *System)
{
  System->Printf(MSG_INITIALIZATION, "Initializing loader plug-in...\n");

  // get the virtual file system plugin
  GET_PLUGIN(tmpWrap.VFS, CS_FUNCID_VFS, iVFS, "VFS");
  if (!tmpWrap.VFS) return false;

  // get all optional plugins
  GET_PLUGIN(tmpWrap.ImageLoader, CS_FUNCID_IMGLOADER, iImageLoader, "image loader");
  GET_PLUGIN(tmpWrap.SoundLoader, CS_FUNCID_SNDLOADER, iSoundLoader, "sound loader");
  GET_PLUGIN(tmpWrap.Engine, CS_FUNCID_ENGINE, iEngine, "engine");

  return true;
}

iImage* csLoader::LoadImage (const char* name)
{
  if (!tmpWrap.Engine || !tmpWrap.ImageLoader)
     return NULL;

  iImage *ifile = NULL;
  iDataBuffer *buf = tmpWrap.VFS->ReadFile (name);

  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    System->Printf (MSG_WARNING, "Cannot read image file \"%s\" from VFS\n", name);
    return NULL;
  }

  ifile = tmpWrap.ImageLoader->Load (buf->GetUint8 (), buf->GetSize (),
    tmpWrap.Engine->GetTextureFormat ());
  buf->DecRef ();

  if (!ifile)
  {
    CsPrintf (MSG_WARNING, "'%s': Cannot load image. Unknown format or wrong extension!\n",name);
    return NULL;
  }

  iDataBuffer *xname = tmpWrap.VFS->ExpandPath (name);
  ifile->SetName (**xname);
  xname->DecRef ();

  return ifile;
}
