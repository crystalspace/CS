/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csver.h"

#include "csutil/hash.h"
#include "csutil/set.h"

#include "mapstd.h"
#include "map.h"
#include "entity.h"
#include "mpoly.h"
#include "cworld.h"
#include "csector.h"
#include "cthing.h"
#include "texplane.h"
#include "zipfile.h"
#include "vertbuf.h"
#include "mcurve.h"
#include "sparser.h"

#include "dochelp.h"

#define TEMPWORLD "map2cs2.$$$"

CCSWorld::CCSWorld()
{
}

CCSWorld::~CCSWorld()
{
}

void CCSWorld::FindSectors()
{
  size_t j, SectorCounter = 0;
  size_t i;

  //iterate all entities and brushes
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity((int)i);
    if (strcmp(pEntity->GetClassname(), "cs_sector")==0)
    {
      //This entity is a sector!
      for (j=0; j<pEntity->GetNumBrushes(); j++)
      {
        SectorCounter++;
        CMapBrush* pBrush  = pEntity->GetBrush(j);
        CCSSector* pSector = new CCSSector(pBrush);
        m_Sectors.Push(pSector);
      } //for brush
    }
    else
    {
      //no sector. Remember the pointer to it.
      m_Entities.Push(pEntity);
    }
  } //for entity

  if (m_Sectors.Length() == 0)
  {
    GenerateDefaultsector();
  }
}

void CCSWorld::GenerateDefaultsector()
{
  if (m_pMap->GetPlaneCount()<=0)
  {
    csPrintf("Map contains no data. Aborting!");
    exit(1);
  }

  //Find out the size of this sector
  CdVector3 Min, Max;
  m_pMap->GetMapSize(Min, Max);

  Min.x -= 10;
  Min.y -= 10;
  Min.z -= 10;

  Max.x += 10;
  Max.y += 10;
  Max.z += 10;

  CdVector3 v[8];
  v[0] = CdVector3(Min.x, Min.y, Min.z);
  v[1] = CdVector3(Max.x, Min.y, Min.z);
  v[2] = CdVector3(Max.x, Min.y, Max.z);
  v[3] = CdVector3(Min.x, Min.y, Max.z);
  v[4] = CdVector3(Min.x, Max.y, Min.z);
  v[5] = CdVector3(Max.x, Max.y, Min.z);
  v[6] = CdVector3(Max.x, Max.y, Max.z);
  v[7] = CdVector3(Min.x, Max.y, Max.z);

  static int Planes[6][4] =
  {
    {4,5,1,0},
    {5,6,2,1},
    {6,7,3,2},
    {7,4,0,3},
    {0,1,2,3},
    {7,6,5,4}
  };

  //Create the Brush for that sector;
  CMapBrush* pBrush = new CMapBrush(0);

  int i;
  for (i=0; i<6; i++)
  {
    //For every side of the default sector create a flatshaded
    //plane in black color.
    CMapTexturedPlane* pPlane = m_pMap->AddPlane(v[Planes[i][2]],
                                                 v[Planes[i][1]],
                                                 v[Planes[i][0]],
                                                 0,0,0); //black
    assert(pPlane);
    pBrush->AddPlane(pPlane);
  }

  pBrush->CreatePolygons();

  CCSSector* pSector = new CCSSector(pBrush);
  m_Sectors.Push(pSector);
}

bool CCSWorld::Write(csRef<iDocumentNode> root, CMapFile* pMap,
  const char * /*sourcename*/)
{
  if (!PrepareData(TEMPWORLD, pMap)) return false;

  AllocateSkytextures();

  time_t Time;
  time (&Time);
  struct tm *now = localtime (&Time);
  csString buf;

  buf.Format (" Created by map2cs " CS_VERSION " on "
    "%04d-%02d-%02d %02d:%02d:%02d ", now->tm_year+1900, 
    now->tm_mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  DocNode temp = root->CreateNodeBefore (CS_NODE_COMMENT);
  temp->SetValue (buf);
  DocNode world = CreateNode(root, "world");

  CMapEntity* pEntity = GetWorldspawn();
  
  {
    size_t keyNum = pEntity->GetNumberOfKeyValuePairs();
    for (size_t i = 0; i < keyNum; i++) {
      CMapKeyValuePair*	newPair = pEntity->GetKeyValuePair(i);
      if (strcmp(newPair->GetKey (), "world_scale")==0 
       || strcmp(newPair->GetKey (), "archive")==0
       || strcmp(newPair->GetKey (), "lightmapsize")==0
       || strcmp(newPair->GetKey (), "ambient")==0
       || strcmp(newPair->GetKey (), "clearscreen")==0)
	continue;

      DocNode newKey = CreateNode (world, "key");
      newKey->SetAttribute("name", newPair->GetKey());
      newKey->SetAttribute("value", newPair->GetValue());
    }
  }

  {
    DocNode scaling = CreateNode (world, "key");
    scaling->SetAttribute ("name", "map2cs_scaling");
    scaling->SetAttributeAsFloat ("value", m_ScaleFactor);
  }

  WritePlayerStart (world);
  WriteTextures (world);
  WriteLibs (world);
  WriteSettings(world);

  {
    DocNode renderpriorities = CreateNode (world, "renderpriorities");

    DocNode sky = CreateNode (renderpriorities, "priority");
    sky->SetAttribute ("name", "sky");
    CreateNode (sky, "level", "1");
    CreateNode (sky, "sort", "NONE");

    DocNode mirror = CreateNode (renderpriorities, "priority");
    mirror->SetAttribute ("name", "mirror");
    CreateNode (mirror, "level", "2");
    CreateNode (mirror, "sort", "FRONT2BACK");

    DocNode wall = CreateNode (renderpriorities, "priority");
    wall->SetAttribute ("name", "wall");
    CreateNode (wall, "level", "3");
    CreateNode (wall, "sort", "NONE");

    DocNode object = CreateNode (renderpriorities, "priority");
    object->SetAttribute ("name", "object");
    CreateNode (object, "level", "4");
    CreateNode (object, "sort", "NONE");

    DocNode alpha = CreateNode (renderpriorities, "priority");
    alpha->SetAttribute ("name", "alpha");
    CreateNode (alpha, "level", "5");
    CreateNode (alpha, "sort", "BACK2FRONT");
  }

  WritePlugins (world);
  WriteSounds (world);
  WriteSpritesTemplate (world);
  WriteScriptsTemplate (world);

  WriteCurvetemplates (world);

  WriteSkysector (world);
  WriteSectors (world);
  
  WriteLibs (world, "end");

  return true;
}

CMapEntity* CCSWorld::GetWorldspawn()
{
  size_t i, NumEntities = GetNumEntities();
  for (i=0; i<NumEntities; i++)
  {
    CMapEntity* pEntity = GetEntity(i);
    assert(pEntity);
    if (strcasecmp(pEntity->GetClassname(), "worldspawn") == 0)
    {
      return pEntity;
    }
  }

  return 0;
}

bool CCSWorld::NeedSkysector()
{
  // in future, this should be more elaborate: it should check all polygons,
  // and return only true, if there is at least one portal to the skysector.
  return true;
}

void CCSWorld::AllocateSkytextures()
{
  CMapEntity* pEntity = GetWorldspawn();
  if (!pEntity) return;

  CTextureManager* pTexMan  = m_pMap->GetTextureManager();

  if (pEntity->GetValueOfKey("skydome"))
  {
    pTexMan->GetTexture(pEntity->GetValueOfKey("skydome", "sky"));
  }
  else if (pEntity->GetValueOfKey("skybox"))
  {
    static const char ext[] = "frblud";
    const char* basename    = pEntity->GetValueOfKey("skybox", "sky");

	int i;
    for (i=0; i<6; i++)
    {
      csString name;
      name.Format ("%s_%c", basename, ext[i]);
      CTextureFile* pTexture = pTexMan->GetTexture(name);
      CS_ASSERT(pTexture);

      pTexture->SetMipmapped(false);
    }
  }
}

void CCSWorld::WriteSkysector(csRef<iDocumentNode> node)
{
  CMapEntity* pEntity = GetWorldspawn();
  if (!pEntity) return;

  // Only write a skydome, if there has been specified a skysector in the
  // mapfile.  There will likely be problems, if you are using sky-portals, but
  // didn't specify a skysector.  (to handle this correctly, you should check
  // if there is a skysector, when writing portals to it.)
  if (pEntity->GetValueOfKey("skybox") ||
      pEntity->GetValueOfKey("skydome"))
  {
    DocNode sector = CreateNode (node, "sector");
    sector->SetAttribute ("name", "cs_skysector");

    DocNode meshobj = CreateNode (sector, "meshobj");
    meshobj->SetAttribute ("name", "sky");

    CreateNode (meshobj, "plugin", "thing");
    CreateNode (meshobj, "zfill");
    CreateNode (meshobj, "priority", "sky");
    CreateNode (meshobj, "camera");

    DocNode params = CreateNode (meshobj, "params");

    WriteSky(params);
  }
}

void CCSWorld::WriteSky(csRef<iDocumentNode> node)
{
  CMapEntity* pEntity = GetWorldspawn();
  if (!pEntity) return;

  if (pEntity->GetValueOfKey("skybox"))
  {
    WriteSkybox(node);
  }
  else if (pEntity->GetValueOfKey("skydome"))
  {
    WriteSkydome(node);
  }
}

void CCSWorld::WriteSkydome(csRef<iDocumentNode> node)
{
  CMapEntity* pEntity = GetWorldspawn();
  if (!pEntity) return;

  //const char* DomeName   = pEntity->GetValueOfKey   ("skydome",      "sky");
  //double      DomeRadius = pEntity->GetNumValueOfKey("skydomeradius", 800);

  //CTextureManager* pTexMan  = m_pMap->GetTextureManager();
  //CTextureFile*    pTexture = pTexMan->GetTexture(DomeName);

/* @@@ TODO!
  
  WriteIndent();
  //csFPrintf(m_fd, "CIRCLE (0,0,0:%g,0,%g,-12)\n", DomeRadius, DomeRadius);
  csFPrintf(m_fd,
    "<circle>0,0,0:%g,0,%g,-12</circle>\n", DomeRadius, DomeRadius);
  WriteIndent();
  csFPrintf(m_fd,
    "<material>%s</material><texlen>1</texlen>\n", pTexture->GetTexturename());
  WriteIndent();
  // appears to be unsupported now...
  csFPrintf(m_fd,
    "<skydome name=\"up\"><lighting>NO</lighting><radius>%g</radius> "
    "<vertices>0,1,2,3,4,5,6,7,8,9,10,11</vertices></skydome>\n", DomeRadius);
  WriteIndent();
  csFPrintf(m_fd,
    "<skydome name=\"down\"><lighting>NO</lighting><radius>%g</radius> "
    "<vertices>0,11,10,9,8,7,6,5,4,3,2,1</vertices></skydome>\n", -DomeRadius);
*/
}

void CCSWorld::WriteSkybox(csRef<iDocumentNode> node)
{
  CMapEntity* pEntity = GetWorldspawn();
  if (!pEntity) return;

  CTextureManager* pTexMan  = m_pMap->GetTextureManager();
  const char* BoxName = pEntity->GetValueOfKey   ("skybox",    "sky");
  double      BoxSize = pEntity->GetNumValueOfKey("skyboxsize", 1600) / 2;

  //Matze: This is my own version, I can't realize crackles so I'm using the
  // simple version here... If it's ok in all cases you can remove the above
  static const struct {int x; int y; int z;} Vertices [] =
  {{-1,-1, 1}, { 1,-1, 1},
   {-1,-1,-1}, { 1,-1,-1},
   {-1, 1, 1}, { 1, 1, 1},
   {-1, 1,-1}, { 1, 1,-1}};

  struct ThingSide
  {
    const char*   ext;
    const char*   vertices[4];
    CTextureFile* pTex;
  };
  
  static ThingSide ThingSides[] = {
    {"f", { "4", "5", "1", "0"} , 0},
    {"r", { "5", "7", "3", "1"}, 0},
    {"b", { "7", "6", "2", "3"}, 0},
    {"l", { "6", "4", "0", "2"}, 0},
    {"u", { "6", "7", "5", "4"}, 0},
    {"d", { "3", "2", "0", "1"}, 0}
  };

  int smallest_size = 0x7fffffff;
  //assign texture pointers to the sides of the skybox
  int s,v;
  for (s=0; s<int(sizeof(ThingSides)/sizeof(ThingSides[0])); s++)
  {
    csString name;
    name.Format ("%s_%s", BoxName, ThingSides[s].ext);
    ThingSides[s].pTex = pTexMan->GetTexture(name);

    smallest_size = MIN(smallest_size,ThingSides[s].pTex->GetOriginalWidth());
    smallest_size = MIN(smallest_size,ThingSides[s].pTex->GetOriginalHeight());
  }
 
  for (v = 0; v<int(sizeof(Vertices)/sizeof(Vertices[0])); v++)
  {
    DocNode vertex = CreateNode (node, "v");
    vertex->SetAttributeAsFloat ("x", BoxSize*Vertices[v].x);
    vertex->SetAttributeAsFloat ("y", BoxSize*Vertices[v].y);
    vertex->SetAttributeAsFloat ("z", BoxSize*Vertices[v].z);
  }

  // Texture coordinates for the sky sides. 
  // They are offset a bit as seams could be visible otherwise.
  float min_tc = (0.5f) / ((float)smallest_size);
  float max_tc = ((float)smallest_size - 0.5f) / ((float)smallest_size);
  //write the skyboxes polygons
  for (s=0; s<int(sizeof(ThingSides)/sizeof(ThingSides[0])); s++)
  {
    DocNode p = CreateNode (node, "p");

    for (v = 0; v < 4; v++)
    {
      DocNode vertex = CreateNode (p, "v", ThingSides[s].vertices[v]);
    }

    DocNode texmap = CreateNode (p, "texmap");
    
    DocNode uv;
    uv = CreateNode (texmap, "uv");
    uv->SetAttributeAsInt ("idx", 0);
    uv->SetAttributeAsFloat ("u", min_tc);
    uv->SetAttributeAsFloat ("v", min_tc);

    uv = CreateNode (texmap, "uv");
    uv->SetAttributeAsInt ("idx", 1);
    uv->SetAttributeAsFloat ("u", max_tc);
    uv->SetAttributeAsFloat ("v", min_tc);

    uv = CreateNode (texmap, "uv");
    uv->SetAttributeAsInt ("idx", 2);
    uv->SetAttributeAsFloat ("u", max_tc);
    uv->SetAttributeAsFloat ("v", max_tc);

    CreateNode (p, "material", ThingSides[s].pTex->GetTexturename());
    CreateNode (p, "lighting", "no");
  }
}

void CCSWorld::FindAdditionalTextures()
{
  // Thomas Hieber, 16.05.2001:
  // Please don't ask me about this function.  I didn't create it, nor do I
  // know the deeper truth about it.  All I did was to move the code to a
  // separate function for better maintainablility.  (And I have made the
  // textures really register with the texture manager, which should really
  // help to get the textures in the zip file, if map2cs can find them.  Also
  // it makes handling of materials much easier, because I don't have to skip
  // through all entities twice, and it avoids adding some textures multiple
  // times with the same name.)

  size_t j=0;

  // Look for cs_sprite entities
  for (j=0; j<m_pMap->GetNumEntities(); j++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity(j);
    if (strcmp(pEntity->GetClassname(), "cs_sprite")==0)
    {
      if (pEntity->GetValueOfKey("texture"))
      {
        const char* basename = pEntity->GetValueOfKey("texture", "white");

        // the texture manager will create a new texture for us, if the given
        // texture is missing
        CTextureFile* pTexture =
	  m_pMap->GetTextureManager()->GetTexture(basename);
        assert(pTexture);

        const char* keycolor = pEntity->GetValueOfKey("keycolor", "");
        if (*keycolor)
        {
          //if the string is not empty
          float r = 0.0f;
          float g = 0.0f;
          float b = 0.0f;
          if (sscanf(keycolor, "%f %f %f", &r, &g, &b) == 3)
          {
            pTexture->SetKeyColor(r, g, b);
          }
        }
      }
    }
  }

  //texture for 3D sprites (cs_model)
  //requires models subdir under data. also, need to be added to vfs
  size_t NumEntities = GetNumEntities();
  for (j=0; j<NumEntities; j++)
  {
    CMapEntity* pEntity = GetEntity(j);
    assert(pEntity);
    if (strcmp(pEntity->GetClassname(), "cs_model") == 0)
    {
      const char* csnamevalue = pEntity->GetValueOfKey("cs_name");
      if (csnamevalue)
      {
        const char* modelnamevalue = pEntity->GetValueOfKey("modelname");
        if (modelnamevalue)
        {
          if (pEntity->GetValueOfKey("texture"))
          {
            CTextureFile* pTexture =
	      m_pMap->GetTextureManager()->GetTexture(modelnamevalue);
            assert(pTexture);

            csString name;
            const char* basename = pEntity->GetValueOfKey("texture", "white");
            name.Format ("/lib/models/%s", basename);

            pTexture->SetFilename(name);
          }
        }
      }
    }
  }
}

bool CCSWorld::WriteTextures(csRef<iDocumentNode> node)
{
  CMapEntity* pEntity = GetWorldspawn();

  FindAdditionalTextures();

  csSet<csStrKey> userMaterials;
  csSet<csStrKey> userTextures;

  const char* mats = pEntity->GetValueOfKey("usermaterials");
  csString mat;
  while (mats && *mats)
  {
    mat.Clear();
    const char* comma = strchr (mats, ',');
    if (comma)
      mat.Append (mats, comma - mats);
    else
      mat.Append (mats);
    
    userMaterials.Add ((const char*)mat);
    mats = comma ? comma + 1 : 0;
  }

  const char* txts = pEntity->GetValueOfKey("usertextures");
  csString txt;
  while (txts && *txts)
  {
    txt.Clear();
    const char* comma = strchr (txts, ',');
    if (comma)
      txt.Append (txts, comma - txts);
    else
      txt.Append (txts);
    
    userTextures.Add ((const char*)txt);
    txts = comma ? comma + 1 : 0;
  }

  DocNode textures = CreateNode (node, "textures");
  DocNode materials = CreateNode (node, "materials");

  size_t i=0;
  for (i=0; i<m_pMap->GetTextureManager()->GetTextureCount(); i++)
  {
    CTextureFile* pTexture = m_pMap->GetTextureManager()->GetTexture(i);
    assert(pTexture);

    if (userTextures.In (pTexture->GetTexturename()))
    {
      pTexture->SetStored (false);
    }
    else if (pTexture->IsStored())
    {
      csString replacename;
      const char *newtexfile;
      replacename.Format ("filename_%s", pTexture->GetTexturename());
      if ((newtexfile = pEntity->GetValueOfKey(replacename)))
	pTexture->SetStored (false);
      
      DocNode texture = CreateNode (textures, "texture");
      texture->SetAttribute ("name", pTexture->GetTexturename());
      CreateNode (texture, "file", 
        newtexfile ? newtexfile : pTexture->GetFilename());

      if (pTexture->IsColorKeyed())
      {
	float r, g, b;
	pTexture->GetKeyColor(r, g, b);

	DocNode transparent = CreateNode (texture, "transparent");
	transparent->SetAttributeAsFloat ("red", r);
	transparent->SetAttributeAsFloat ("green", g);
	transparent->SetAttributeAsFloat ("blue", b);
      }

      if (!pTexture->IsMipmapped())
      {
	CreateNode (texture, "mipmap", "no");
      }
    }

    if (pTexture->IsVisible() && 
      !userMaterials.In (pTexture->GetTexturename()))
    {
      DocNode material = CreateNode (materials, "material");
      material->SetAttribute ("name", pTexture->GetTexturename());
      if (pTexture->IsStored())
      {
	CreateNode (material, "texture", pTexture->GetTexturename());
      }
      else
      {
	DocNode color = CreateNode (material, "color");
	color->SetAttributeAsFloat ("red", 0.5f);
	color->SetAttributeAsFloat ("green", 0.5f);
	color->SetAttributeAsFloat ("blue", 0.5f);
      }
    }
  }

  return true;
}

bool CCSWorld::WritePlugins(csRef<iDocumentNode> node)
{
  DocNode plugins = CreateNode (node, "plugins");

  DocNode plugin;
  plugin = CreateNode (plugins, "plugin", "crystalspace.mesh.loader.thing");
  plugin->SetAttribute ("name", "thing");
  plugin = CreateNode (plugins, "plugin",
    "crystalspace.mesh.loader.factory.thing");
  plugin->SetAttribute ("name", "thingFact");

  plugin = CreateNode (plugins, "plugin", "crystalspace.mesh.loader.bezier");
  plugin->SetAttribute ("name", "bezier");
  plugin = CreateNode (plugins, "plugin",
    "crystalspace.mesh.loader.factory.bezier");
  plugin->SetAttribute ("name", "bezierFact");

  return true;
}


bool CCSWorld::WritePlayerStart(csRef<iDocumentNode> node)
{
  size_t i;

  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity(i);
    assert(pEntity);

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "info_player_start")==0 ||
        strcmp(classname, "info_player_deathmatch")==0)
    {
      double x, y, z;
      if (pEntity->GetTripleNumValueOfKey("origin", x,y,z))
      {
        CdVector3 vec(x,y,z);
        CISector* pSector = FindSectorForPoint(vec);
        if (pSector)
        {
          //The strings format matched
	  CdVector3 forw(0,0,1);
	  CdVector3 up(0,1,0);
	  // rotate angle
	  double angle = 0.0;
	  double a = 0.0, b = 0.0, c = 0.0; // pitch, yaw, roll
	  if (pEntity->GetTripleNumValueOfKey("angles", a, b, c))
	  {
	    const double sa = sin (a);
	    const double ca = cos (a);
	    const double sb = sin (b);
	    const double cb = cos (b);
	    const double sc = sin (c);
	    const double cc = cos (c);
	    CdMatrix3 rot (cb*cc, -cb*sc, sb,
			   ca*sc + sa*sb*cc, ca*cc - sa*sb*sc, -sa*cb,
			   sa*sc - ca*sb*cc, sa*cc + ca*sb*sc, ca*cb);
	    up = rot * up;
	    forw = rot * forw;
	  }
	  else if (pEntity->GetNumValueOfKey("angle", angle))
	  {
	    angle = PI * angle/180;
	    forw = CdMatrix3 (cos(angle), 0, -sin(angle),
			       0,          1,           0,
			       sin(angle), 0, cos(angle)) * forw;
	  }
	  
          DocNode start = CreateNode (node, "start");
	  CreateNode (start, "sector", pSector->GetName());

	  DocNode attr;
	  attr = CreateNode (start, "position");
	  attr->SetAttributeAsFloat ("x", x*m_ScaleFactor);
	  attr->SetAttributeAsFloat ("y", z*m_ScaleFactor);
	  attr->SetAttributeAsFloat ("z", y*m_ScaleFactor);

	  attr = CreateNode (start, "forward");
	  attr->SetAttributeAsFloat ("x", forw.x);
	  attr->SetAttributeAsFloat ("y", forw.y);
	  attr->SetAttributeAsFloat ("z", forw.z);

	  attr = CreateNode (start, "up");
	  attr->SetAttributeAsFloat ("x", up.x);
	  attr->SetAttributeAsFloat ("y", up.y);
	  attr->SetAttributeAsFloat ("z", up.z);
          return true;
        }
        else
        {
          csPrintf("warning: info_player_start not inside a valid sector.\n");
          csPrintf("         no startinfo is written!\n");
        }
      }
      else
      {
        csPrintf("warning: info_player_start has no origin.\n");
        csPrintf("         no startinfo is written!\n");
      }
    }
  }
  return true;
}

bool CCSWorld::WriteSettings(csRef<iDocumentNode> node)
{

  CMapEntity* pEntity = GetWorldspawn();

  DocNode settings = CreateNode (node, "settings");
  CreateNode(settings, "clearzbuf", "yes");

  if (const char *clsc = pEntity->GetValueOfKey ("clearscreen"))
  {
    if (strcmp(clsc, "yes")==0) {
      CreateNode(settings, "clearscreen", "yes");
    }
  }

  if (const char *mxlmsize = pEntity->GetValueOfKey ("lightmapsize"))
  {
    char dummy;
    int w, h;

    if (sscanf(mxlmsize, "%d %d%c", &w, &h, &dummy)==2) {
      DocNode maxLightmap = CreateNode(settings, "maxlightmapsize");
      maxLightmap->SetAttributeAsInt("horizontal", w);
      maxLightmap->SetAttributeAsInt("vertical", h);
    }
  }

  if (const char *ambient = pEntity->GetValueOfKey ("ambient"))
  {
    char dummy;
    float r = 0.0;
    float g = 0.0;
    float b = 0.0;

    if (sscanf(ambient, "%f %f %f%c", &r, &g, &b, &dummy)==3) {
      DocNode amb = CreateNode(settings, "ambient");
      amb->SetAttributeAsFloat("red", r);
      amb->SetAttributeAsFloat("green", g);
      amb->SetAttributeAsFloat("blue", b);
    }
  }

  if (const char *lmcs = pEntity->GetValueOfKey ("lightmapcellsize"))
  {
    CreateNode(settings, "lightmapcellsize", lmcs);
  }
  if (const char *rloop = pEntity->GetValueOfKey ("renderloopname"))
  {
    CreateNode(settings, "renderloop", rloop);
  }
  
  return true;
}

bool CCSWorld::WriteSectors(csRef<iDocumentNode> node)
{
  size_t i;
  for (i=0; i<m_Sectors.Length(); i++)
  {
    if (!m_Sectors[i]->Write(node, this)) return false;
  }
  return true;
}

bool CCSWorld::WriteVertex(csRef<iDocumentNode> node, double x, double y, 
			   double z)
{
  WriteVector(node, "v", x,y,z);
  return true;
}

bool CCSWorld::WriteVector(csRef<iDocumentNode> node, const char* name, 
			   double x, double y, double z)
{
  DocNode vector = CreateNode (node, name);
  vector->SetAttributeAsFloat ("x", x*m_ScaleFactor);
  vector->SetAttributeAsFloat ("y", z*m_ScaleFactor);
  vector->SetAttributeAsFloat ("z", y*m_ScaleFactor);
  return true;
}

bool CCSWorld::WriteVector(csRef<iDocumentNode> node, const char* name, 
			   const CdVector3& v)
{
  return WriteVector(node, name, v.x, v.y, v.z);
}


bool CCSWorld::WriteCurvetemplates(csRef<iDocumentNode> node)
{
  size_t i, c;
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity(i);
    for (c=0; c<pEntity->GetCurveCount(); c++)
    {
      CMapCurve*    pCurve   = pEntity->GetCurve(c);
      CTextureFile* pTexture = pCurve->GetTexture();
      if (pTexture->IsVisible())
      {
        pCurve->Write(node, this);
      }
    }
  }

  return true;
}

bool CCSWorld::WriteLibs (csRef<iDocumentNode> node, const char* type)
{
  const char* libs = GetWorldspawn()->GetValueOfKey("libraries");
  csString lib;
  while (libs && *libs)
  {
    lib.Clear();
    const char* comma = strchr (libs, ',');
    if (comma)
      lib.Append (libs, comma - libs);
    else
      lib.Append (libs);

    const char* colon = strchr (lib, ':');
    bool writeLib = (colon == 0) && (type == 0);
    if ((colon != 0) && (type != 0))
    {
      size_t cPos = colon - (const char*)lib;
      csString prefix = lib.Slice (0, cPos);
      writeLib = strcmp (prefix, type) == 0;
      lib.DeleteAt (0, cPos + 1);
    }

    if (writeLib) CreateNode (node, "library", lib);
    libs = comma ? comma + 1 : 0;
  }
  return true;
}

bool CCSWorld::WriteKeys(csRef<iDocumentNode> node, CIWorld* pWorld, 
			 CMapEntity* pEntity)
{
  if (pEntity)
  {
    size_t i;
    for (i=0; i<pEntity->GetNumberOfKeyValuePairs(); i++)
    {
      CMapKeyValuePair* pKV = pEntity->GetKeyValuePair(i);
      DocNode key = CreateNode (node, "key");
      key->SetAttribute ("name", pKV->GetKey());
      key->SetAttribute ("value", pKV->GetValue());
    }
  }
  return true;
}

bool CCSWorld::WriteTexMap (CMapTexturedPlane* plane, DocNode& poly)
{
  //Check if the plane has a texture. Otherwise, we will not write
  //thi info to the map file.
  if (plane->GetTexture())
  {
    CdVector3 Origin = plane->GetTextureCoordinates(0);
    CdVector3 First  = plane->GetTextureCoordinates(1);
    CdVector3 Second = plane->GetTextureCoordinates(2);

    DocNode params = CreateNode (poly, "texmap");
    WriteVector(params, "orig", Origin);
    WriteVector(params, "first", First);
    WriteVector(params, "second", Second);
    CreateNode (params, "firstlen",
	(float)((First-Origin).Norm()*m_ScaleFactor));
    CreateNode (params, "secondlen",
	(float)((Second-Origin).Norm()*m_ScaleFactor));
  }
  return true;
}

bool CCSWorld::WritePolygon(csRef<iDocumentNode> node, CMapPolygon* pPolygon, 
			    CCSSector* pSector, bool SectorPolygon, 
			    const CVertexBuffer& Vb,
			    bool &Sky)
{
  const CMapTexturedPlane* pPlane   = pPolygon->GetBaseplane();
  CMapEntity*              pEntity  = pPolygon->GetEntity();
  size_t l;

  DocNode poly = CreateNode (node, "p");
  if (SectorPolygon)
  {
    //Because for a sector we draw the _inside_ of the brush, we spit out the
    //vertices in reverse order, so they will have proper orientation for
    //backface culling in the engine.
    for (l=pPolygon->GetVertexCount()-1; l>=0; l--)
    {
      CreateNode (poly, "v", (int)Vb.GetIndex(pPolygon->GetVertex(l)));
    }
  }
  else
  {
    //regular vertex order
    for (l=0; l<pPolygon->GetVertexCount(); l++)
    {
      CreateNode (poly, "v", (int)Vb.GetIndex(pPolygon->GetVertex(l)));
    }
  }

  CTextureFile* pTexture = pPlane->GetTexture();
  Sky = false;
  if (!pTexture)
  {
    //@@@ doesn't seem to be supported any more
/*    int r, g, b;
    pPlane->GetColor(r,g,b);
    csFPrintf(m_fd, "<flatcol red=\"%g\" green=\"%g\" blue=\"%g\" />",
                r/(float)255, g/(float)255, b/(float)255);*/
  }
  else
  {
    //print textureinfo
    CreateNode (poly, "material", pTexture->GetTexturename());
    WriteTexMap (pPolygon->GetBaseplane (), poly);
    Sky = (strcasecmp (pTexture->GetTexturename(), "sky") == 0);
  }

  
  Sky = Sky || (pEntity && pEntity->GetBoolValueOfKey("sky", false));
  if (Sky)
  {
    DocNode portal = CreateNode (poly, "portal");
    CreateNode (portal, "sector", "cs_skysector");
    CreateNode (portal, "clip");
    CreateNode (poly, "lighting", "no");
  } else if (pEntity)
  {
    //support for special rendering flags. Ideally, these properties would be
    //assigned to surfaces instead of entities, but the Quake/ Half-Life map
    //format doesn't have a really portable way to do so.
    bool   Solid  = pEntity->GetBoolValueOfKey("solid",  true);
    bool   Mirror = pEntity->GetBoolValueOfKey("mirror", false);
    if (!pEntity->GetBoolValueOfKey("lighting", true))
    {
      //if the polygon is not supposed to be lighted...
      CreateNode (poly, "lighting", "no");
    }
    
    //Not supported anymore
    /*double Alpha  = pEntity->GetNumValueOfKey ("alpha",  100);
    if (Alpha < 100)
    {
      CreateNode (poly, "alpha", (float)Alpha);
    }
    */

    if (Mirror || !Solid)
    {
      // We have a special case, where we need to turn this polygon into a
      // portal.
      const char* targetsector =
        pEntity->GetValueOfKey("targetsector", pSector->GetName());

      DocNode portal = CreateNode (poly, "portal");
      CreateNode (portal, "sector", targetsector);
      
      if ( Mirror )
      {
	CreateNode (portal, "mirror");
	CreateNode (portal, "clip");
      }
      else
      {
        if (Solid)
        {
  	  CreateNode (portal, "clip");
        }
      }
    } //if polygon is a portal of some special kind
  } // if there is a entity to define some special flags.

  return true;
}

CIThing* CCSWorld::CreateNewThing(CMapEntity* pEntity)
{
  return new CCSThing(pEntity);
}

CISector* CCSWorld::CreateNewSector(CMapBrush* pBrush)
{
  return new CCSSector(pBrush);
}

void CCSWorld::WriteSpritesTemplate(csRef<iDocumentNode> node)
{
  size_t i;
  char   mdlname[99] = "none";
  char   action[99] = "none";

  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity(i);
    assert(pEntity);

    CTextureManager* pTexMan  = m_pMap->GetTextureManager();

    if (pEntity->GetValueOfKey("skydome"))
    {
      pTexMan->GetTexture(pEntity->GetValueOfKey("skydome", "sky"));
    }

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "cs_model")==0)
    {
      const char* csnamevalue = pEntity->GetValueOfKey("cs_name");

      if (!csnamevalue)
      {
        return;
      }

      const char* modelnamevalue = pEntity->GetValueOfKey("modelname");
      if (modelnamevalue)
      {
        char dummy;
        if (sscanf(modelnamevalue, "%s%c",mdlname, &dummy) == 1)
	{
	  DocNode meshfact = CreateNode (node, "meshfact");
	  meshfact->SetAttribute ("name", csnamevalue);
	  CreateNode (meshfact, "plugin", "spr3d");
          //TODO: see if we can initalize .3ds files
	  CreateNode (meshfact, "file", mdlname);
          //csFPrintf(m_fd, "<file>/lib/models/%s.mdl</file>\n", mdlname);
	  CreateNode (meshfact, "material", csnamevalue);

          //TODO: add loop for multiple actions, action2, action3, ...
          //search for all frames from base name would be better??
          const char* actionvalue = pEntity->GetValueOfKey("action");

          if (sscanf(actionvalue, "%s%c",action, &dummy) == 1)
          {
	    DocNode action = CreateNode (meshfact, "action");
	    action->SetAttribute ("name", actionvalue);

            int i = 1;
            csString frameaction;
            frameaction.Format ("action%d", i);
            while (pEntity->GetValueOfKey(frameaction))
            {
              actionvalue = pEntity->GetValueOfKey(frameaction);
              if (actionvalue && *actionvalue)
              {
		DocNode f = CreateNode (meshfact, "f");
		f->SetAttribute ("name", actionvalue);
		f->SetAttributeAsInt ("delay", 100);//hardcode 100 for now
              }
              i++;
              frameaction.Format ("action%d", i);
            }

          }

        }
      }
    }

  } //for entity
  return;
}

void CCSWorld::WriteScriptsTemplate(csRef<iDocumentNode> node)
{
  //int i;

  //iterate all entities, brushes, polygons and vertices:
/*  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity(i);
    assert(pEntity);

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "cs_script")==0)
    {
      const char* cs_scriptname = pEntity->GetValueOfKey("cs_name");
      assert(cs_scriptname);
      const char* scriptfilename = pEntity->GetValueOfKey("script_cmdfile");
      assert(scriptfilename);

      if (!cs_scriptname || strcmp(scriptfilename, "none")==0 )
      {
        return;
      }

      //if scriptfile does not exist
      CScriptParser parser;
      if (!parser.Open(scriptfilename))
      {
        return;
      }
      //exit

      int found = 0;
      char Buffer[1000] = "";
      char spritecode[1000] = "";

      while (found == 0)
      {
        parser.GetTextToken(Buffer);
        if (strcmp(cs_scriptname, Buffer) == 0)
        {
          found = 1;
        }
      }

      //else look for script if script does not exist
      if (found == 0)
      {
        return;
      }
      else
      {
        int count = 1;

        parser.GetLineToken(Buffer);
        strcpy(spritecode, Buffer);
        while (count != 0 )
        {
          parser.GetLineToken(Buffer);

          if ('(' == Buffer[0])
          {
            count++;
            strcat(spritecode, Buffer);
          }
          else if (')' == Buffer[0])
          {
            count--;
            if(count != 0)
            {
              strcat(spritecode, Buffer);
            }
          }
          else
          {
            strcat(spritecode, Buffer);
          }
        }
        if (found == 1)
        {
          WriteIndent();
          csFPrintf(m_fd, "SCRIPT '%s'\n", cs_scriptname);
          Indent();
          WriteIndent();
          csFPrintf(m_fd, "%s", spritecode);
          WriteIndent();
          csFPrintf(m_fd, ") \n\n");
          Unindent();
        }
      }
    }//if classname
  } //for entity*/
  return;
}

//Sounds template section
    //SOUNDS (
    //  SOUND 'track4' (FILE (track4.wav))
    //)
//add filename.zip to vfs in lib/sounds
void CCSWorld::WriteSounds(csRef<iDocumentNode> node)
{
  size_t i;
  char sfname[99] = "none";
  size_t found = 0;
  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity(i);
    assert(pEntity);

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "cs_sound")==0)
    {
      found = 1;
      break;
    }
  }

  if (found == 1)
  {
    DocNode sounds = CreateNode (node, "sounds");

    for (i=0; i<m_pMap->GetNumEntities(); i++)
    {
      CMapEntity* pEntity = m_pMap->GetEntity(i);
      assert(pEntity);

      const char* classname = pEntity->GetClassname();
      assert(classname);

      if (strcmp(classname, "cs_sound")==0)
      {
        const char* soundfileval = pEntity->GetValueOfKey("soundfile");
        if (soundfileval)
        {
          char dummy;
          if (sscanf(soundfileval, "%s%c",sfname, &dummy) == 1)
          {
	    DocNode sound = CreateNode (sounds, "sound");
	    sound->SetAttribute ("name", sfname);
	    CreateNode (sound, "file", sfname);
          }
        }
      }
    }//end for
  }
}//end sounds
