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
#include "mapstd.h"
#include "brush.h"
#include "mpoly.h"
#include "mpolyset.h"
#include "mcurve.h"
#include "iportal.h"
#include "map.h"
#include "texplane.h"
#include "entity.h"
#include "vertbuf.h"
#include "cthing.h"
#include "csector.h"
#include "cworld.h"



static char* BuildMaterialKey (int i, CMapEntity* pEntity)
{
  char* key = new char[1024];
  char* returnvalue = new char[1024];
  sprintf(key,"MaterialName%d",i);
  printf(key);
  if(!pEntity->GetValueOfKey(key)) return NULL;
  sprintf(returnvalue,"KEY(\"defmaterial\",\"%s\")\n",
	      pEntity->GetValueOfKey(key));
  return returnvalue;
}

//Auxiliar funtion to check if the file name is a mdl file
static bool isMdlFile(const char* csmodelfile)
{
  int i = 0;
  bool dot = false;
  char *extension = new char[strlen(csmodelfile)];
  for (i = 0; i < (int)strlen(csmodelfile); i++)
  {
    if(dot) extension[i] = csmodelfile[i];
    else if (csmodelfile[i] == '.') dot = true;
  }
  if(!strcmp(extension,"MDL")) return true;
  if(!strcmp(extension,"mdl")) return true;
  if(!strcmp(extension,"MDl")) return true;
  if(!strcmp(extension,"MdL")) return true;
  if(!strcmp(extension,"mDL")) return true;
  if(!strcmp(extension,"mdL")) return true;
  if(!strcmp(extension,"mDl")) return true;
  if(!strcmp(extension,"Mdl")) return true;
  return false;
};

//Auxiliar Function to contruct the Factory name of a cs_model
static char* GetFactoryName(const char * csmodelfile)
{
  /*
   * Searchs for the extension and substitutes ".extension" by "Fact" and
   * returns this new string
   */
  char* aux = new char[1024];
  bool found = false;
  int slashIndex = 0; //Last '/' in path file
  int i;

  for( i = 0;i < (int)strlen(csmodelfile); i++)
  {
    if(csmodelfile[i] == '/') slashIndex = i;
  }

  i = slashIndex + 1;
  while(i < (int)strlen(csmodelfile) && !found)
  {
    if(csmodelfile[i] == '.')
    {
      found = true;
    }
    else
    {
      aux[i-slashIndex - 1] = csmodelfile[i];
      i++;
    }
  }
  aux[i-slashIndex - 1] = 'F';aux[i-slashIndex] = 'a';
  aux[i-slashIndex + 1] = 'c';aux[i-slashIndex +2] = 't';
  aux[i-slashIndex + 3] = 0;
  return aux;
}


CCSSector::CCSSector(CMapBrush* pBrush)
  : CISector(pBrush)
{
}

CCSSector::~CCSSector()
{
}

bool CCSSector::Write(CIWorld* pIWorld)
{
  assert(pIWorld);
  CCSWorld* pWorld  = (CCSWorld*)  pIWorld;

  CMapFile* pMap = pWorld->GetMap();
  FILE*     fd   = pWorld->GetFile();

  assert(pMap);
  assert(fd);

  pWorld->WriteIndent();
  fprintf(fd, "SECTOR '%s' (\n", GetName());
  pWorld->Indent();

  pWorld->WriteIndent();
  fprintf(fd, "MESHOBJ 'static'(\n");
  pWorld->Indent();

  pWorld->WriteIndent();
  fprintf(fd, "PLUGIN('thing')\n");
  pWorld->WriteIndent();
  fprintf(fd, "ZFILL()\n");
  pWorld->WriteIndent();
  fprintf(fd, "PRIORITY('wall')\n");

  CMapEntity* pEntity = m_pOriginalBrush->GetEntity();
  CCSWorld::WriteKeys(pWorld, pEntity);

  pWorld->WriteIndent();
  fprintf(fd, "PARAMS(\n");
  pWorld->Indent();

  if (pMap->GetConfigInt("Map2CS.General.UseBSP", 0))
  {
    pWorld->WriteIndent();
    fprintf(fd, "VISTREE()\n");
  }

  if (m_IsDefaultsector && pWorld->NeedSkysector())
  {
    /*pWorld->WriteIndent();
    fprintf(fd, "PART 'p1' (\n");
    pWorld->Indent();*/

    pWorld->WriteSky();

    /*pWorld->Unindent();
    pWorld->WriteIndent();
    fprintf(fd, ")\n");*/ //PART
  }
  else
  {
    if (m_Walls.Length() > 0 ||
        m_Portals.Length() > 0)
    {
      pWorld->WriteIndent();
      fprintf(fd, "PART 'p1' (\n");
      pWorld->Indent();

      int i, j, l;

      CVertexBuffer Vb;
      Vb.AddVertices(&m_Walls);
      Vb.AddVertices(&m_Portals);
      Vb.WriteCS(pWorld);

      for (i=0; i<m_Walls.Length(); i++)
      {
        CMapPolygonSet* pPolySet = m_Walls[i];
        for (j=0; j<pPolySet->GetPolygonCount(); j++)
        {
          CMapPolygon* pPolygon = pPolySet->GetPolygon(j);
          pWorld->WritePolygon(pPolygon, this, true, Vb);
        }
      }

      for (i=0; i<m_Portals.Length(); i++)
      {
        CIPortal* pPortal = m_Portals[i];
        for (j=0; j<pPortal->GetPolygonCount(); j++)
        {
          CMapPolygon*             pPolygon = pPortal->GetPolygon(j);
          const CMapTexturedPlane* pPlane   = pPortal->GetBaseplane();

          CTextureFile* pTexture = pPlane->GetTexture();

          //Because for a sector we draw the _inside_ of the brush, we spit out the
          //vertices in reverse order, so they will have proper orientation for
          //backface culling in the engine.
          pWorld->WriteIndent();
          fprintf(fd, "POLYGON '' ( VERTICES (");
          for (l=pPolygon->GetVertexCount()-1; l>=0; l--)
          {
              fprintf(fd, "%d%s", Vb.GetIndex(pPolygon->GetVertex(l)),
                                  ((l==0) ? "" : ","));
          }
          fprintf(fd, ") "); //End of Vertices

          //print textureinfo
          fprintf(fd, "MATERIAL('%s') ", pTexture->GetTexturename());
          fprintf(fd, "TEXTURE(PLANE ('%s')) ", pPolygon->GetBaseplane()->GetName());
          fprintf(fd, "PORTAL ('%s') ", pPortal->GetTargetSector()->GetName());
          fprintf(fd, ")\n"); //End of Polygon
        }
      }

      pWorld->Unindent();
      pWorld->WriteIndent();
      fprintf(fd, ")\n"); //PART
    } //if contains any polygons
  }

  WriteWorldspawn(pWorld);

  pWorld->Unindent();
  pWorld->WriteIndent();
  fprintf(fd, ")\n"); //End of PARAMS

  pWorld->Unindent();
  pWorld->WriteIndent();
  fprintf(fd, ")\n"); //End of MESHOBJ

  WriteFog(pWorld);
  WriteLights(pWorld);
  WriteCurves(pWorld);
  WriteThings(pWorld);
  WriteSprites(pWorld);
  WriteSprites2D(pWorld);
  WriteNodes (pWorld);

  if (pMap->GetConfigInt("Map2CS.General.UseBSP", 0))
  {
    pWorld->WriteIndent();
    fprintf(fd, "CULLER('static')\n");
  }

  pWorld->Unindent();
  pWorld->WriteIndent();
  fprintf(fd, ")\n\n"); //End of SECTOR(
  return true;
}

bool CCSSector::WriteWorldspawn(CIWorld* pWorld)
{
  int i;
  for (i=0; i<m_Things.Length(); i++)
  {
    if (strcasecmp(m_Things[i]->GetClassname(), "worldspawn")==0)
    {
      if (!((CCSThing*)m_Things[i])->WriteAsPart(pWorld, this))
      {
        return false;
      }
    }
  }
  return true;
}

bool CCSSector::WriteThings(CIWorld* pWorld)
{
  int i;
  for (i=0; i<m_Things.Length(); i++)
  {
    if (strcasecmp(m_Things[i]->GetClassname(), "worldspawn")!=0)
    {
      if (!m_Things[i]->Write(pWorld, this))
      {
        return false;
      }
    }
  }
  return true;
}

bool CCSSector::WriteLights(CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  FILE*     fd          = pWorld->GetFile();
  double    ScaleFactor = pWorld->GetScalefactor();

  assert(pMap);
  assert(fd);

  fprintf(fd, "\n");

  int i;

  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity(i);
    if (strcmp(pEntity->GetClassname(), "light")==0)
    {
      CdVector3 origin;
      if (pEntity->GetOrigin(origin))
      {
        if (IsInside(origin))
        {
          //The entity is identified as light and has a valid position.
          //There seem to be multiple versions of light formats, so I will
          //try out every format I know of.
          //I will prefer the "_light" format provided by Half-Life, because
          //this will allow colored light. If there is no "_light" statement,
          //we will look for a "light" key, which will only contain the radius.

          bool   LightOk         = false;
          double r               = 255;
          double g               = 255;
          double b               = 255;
          double radius          = 200;
          bool   dynamic         = false;
          double halo            = 0;
          double halointen       = 0;
          double halocross       = 0;
          char   attenuation[99] = "none";

          const char* lightvalue = pEntity->GetValueOfKey("_light");
          if (lightvalue)
          {
            char dummy;
            if (sscanf(lightvalue, "%lf %lf %lf %lf%c", &r, &g, &b, &radius, &dummy)==4)
            {
              LightOk = true;
            }
          }

          if (!LightOk)
          {
            radius = pEntity->GetNumValueOfKey("light", 100);
            r      = 255;
            g      = 255;
            b      = 255;
          }

          //dynamic light key
          dynamic = pEntity->GetBoolValueOfKey("dynamic", false);

          //halo light key
          pEntity->GetTripleNumValueOfKey("halo", halo, halointen, halocross);

          //attenuation light key
          const char* attenuationlightvalue = pEntity->GetValueOfKey("attenuation");
          if (attenuationlightvalue)
          {
            char dummy;
            if (sscanf(attenuationlightvalue, "%s%c",attenuation, &dummy) == 1)
            {
              if (!(strcmp(attenuation,"none")    || strcmp(attenuation,"linear") ||
                    strcmp(attenuation,"inverse") || strcmp(attenuation,"realistic")))
              {
                strcpy( attenuation , "realistic" );
              }
            }
            else
            {
              strcpy( attenuation , "realistic" );
            }
          }

          //We have a correct light definition
          float lightscale = pWorld->GetMap()->GetConfigFloat(
                "Map2CS.General.LightScale", 3);

          //Anyway, if we scale light normally they will be far to small.
          //It looks like Crystal Space uses a different algo for lights,
          //so we will scale them up a bit. (Maybe the factor should be
          //configurable in the future)
          pWorld->WriteIndent();

          fprintf(fd, "LIGHT  (");
          fprintf(fd, "CENTER (%g,%g,%g) ",
                  origin.x*ScaleFactor,
                  origin.z*ScaleFactor,
                  origin.y*ScaleFactor);
          fprintf(fd, "RADIUS (%g) ",radius*ScaleFactor*lightscale);
          fprintf(fd, "COLOR  (%g,%g,%g) ",
                  (r/255) * (radius/128),
                  (g/255) * (radius/128),
                  (b/255) * (radius/128));
          if (dynamic)
          {
            fprintf(fd, "DYNAMIC () ");
          }
          fprintf(fd, "HALO    (%g,%g,%g) ",
                  (halo),
                  (halointen),
                  (halocross));
          fprintf(fd, "ATTENUATION (%s) ",attenuation);
          fprintf(fd, ")\n\n");
        } //if (light is inside this sector)
      } // if (entity had origin)
    } //if (classname == "light")
  } //for (all entities)
  return true;
}

/// Write all nodes inside the sector
bool CCSSector::WriteNodes(CIWorld* pWorld)
{
  assert(pWorld);

  FILE* fd = pWorld->GetFile();
  assert(fd);

  double ScaleFactor = pWorld->GetScalefactor();

  //iterate all nodes
  int i;
  for (i=0; i<m_Nodes.Length(); i++)
  {
    CMapEntity* pEntity = m_Nodes[i];
    CdVector3 Origin(0,0,0);
    pEntity->GetOrigin(Origin);
    if (strcmp(pEntity->GetValueOfKey("classname"),"cs_model")){
    pWorld->WriteIndent();
    fprintf(fd, "NODE '%s' (\n", pEntity->GetName());
    pWorld->Indent();
    CCSWorld::WriteKeys(pWorld, pEntity);
    pWorld->WriteIndent();
    fprintf(fd, "POSITION(%g,%g,%g)\n",
                Origin.x*ScaleFactor,
                Origin.z*ScaleFactor,
                Origin.y*ScaleFactor);
    pWorld->Unindent();
    pWorld->WriteIndent();
    fprintf(fd, ")\n");
	}
  }

  return true;
}

bool CCSSector::WriteCurves(CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap         = pWorld->GetMap();
  FILE*     fd           = pWorld->GetFile();

  assert(pMap);
  assert(fd);

  int i, curve;
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity(i);
    int const ncurves = pEntity->GetCurveCount();
    if (ncurves>0)
    {
      for (curve=0; curve<ncurves; curve++)
      {
        CMapCurve*    pCurve   = pEntity->GetCurve(curve);
        CTextureFile* pTexture = pCurve->GetTexture();
        if (pTexture->IsVisible())
        {
          pWorld->WriteIndent();
          fprintf(fd, "MESHOBJ '%s_e%d_c%d'(\n",pEntity->GetName(), i, curve);
          pWorld->Indent();

          pWorld->WriteIndent();
          fprintf(fd, "PLUGIN('thing')\n");
          pWorld->WriteIndent();
          fprintf(fd, "ZUSE()\n");
          pWorld->WriteIndent();
          fprintf(fd, "PRIORITY('object')\n");

          CCSWorld::WriteKeys(pWorld, pEntity);

          pWorld->WriteIndent();
          fprintf(fd, "PARAMS(\n");
          pWorld->Indent();

          pWorld->WriteIndent();
          fprintf(fd, "FACTORY ('curve_%s') \n", pCurve->GetName());

          pWorld->Unindent();
          pWorld->WriteIndent();
          fprintf(fd, ")\n"); //PARAMS

          pWorld->WriteIndent();
          fprintf(fd, "MOVE (V (0,0,0))\n");

          pWorld->Unindent();
          pWorld->WriteIndent();
          fprintf(fd, ")\n\n"); //MESHOBJ
        }
      }
    }
  }
  return true;
}

/// Write all 3D sprites inside the sector
//SPRITE 'testbot'
//    (
//      TEMPLATE ('tree.mdl', 'base')
//      MOVE (V (-3,-3,2) )
//    )

    //also added MIXMODE for 3Dsprites
    //MIXMODE arguments (can be combined)
    //  COPY ()         //=SRC
    //  ADD ()          //=SRC+DST
    //  MULTIPLY ()     //=SRC*DST
    //  MULTIPLY2 ()    //=2*SRC*DST
    //  ALPHA (alpha)   //=(1-alpha)*SRC + alpha*DST
    //  TRANSPARENT ()  //=DST
    //  KEYCOLOR ()     // color 0 is transparent

bool CCSSector::WriteSprites(CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  FILE*     fd          = pWorld->GetFile();
  double    ScaleFactor = pWorld->GetScalefactor();
  //const char*   modelscale = "1";   //df added matrix scaler

  assert(pMap);
  assert(fd);

  fprintf(fd, "\n");

  int i;
  char   mdlname[99] = "none";
  //char   action[99] = "none";
//   char    alphastr[99] = "none";

  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity(i);
    assert(pEntity);

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "cs_model")==0)
    {
      const char* csnamevalue = pEntity->GetValueOfKey("cs_name");
      const char* csmodelfile = pEntity->GetValueOfKey("ModelFile");
      const char* csactionsfile = pEntity->GetValueOfKey("ActionsFile");
      const char* csscalefactor = pEntity->GetValueOfKey("ScaleFactor");
      const char* csspritematerial =
		          pEntity->GetValueOfKey("SpriteMaterial");
      const char* csfactmaterial =
		          pEntity->GetValueOfKey("FactoryMaterial");
      const char* csstaticmodel	= pEntity->GetValueOfKey("StaticModel");
      CdVector3 Origin;

      pEntity->GetOrigin(Origin);
      char* csfactname = GetFactoryName(csmodelfile);

      if (csnamevalue)
      {
        //TEMPLATE ('tree.mdl', 'base')
        char dummy;
        if (sscanf(csnamevalue, "%s%c",mdlname, &dummy) == 1)
        {
          pWorld->WriteIndent();
          fprintf(fd,"NODE 'SEED_MESH_OBJ' (\n");
          // Key/Value pairs writting
          pWorld->Indent();
          pWorld->WriteIndent();
          fprintf(fd,"KEY(\"classname\",\"SEED_MESH_OBJ\")\n");
          pWorld->WriteIndent();
          fprintf(fd,"KEY(\"cs_name\",\"%s\")\n",csnamevalue);

          /*Lets build all material definition
           *keys KEY("defmaterial","materialname, texturefile")
           * It will declare materials until BuildMateriaKey returns a NULL
           */
          int mIndex; //material Index
          char* key;
          for (mIndex = 1; (key = BuildMaterialKey(mIndex,pEntity)); mIndex++)
	  {
            pWorld->WriteIndent();
            fprintf(fd,key);
	  }

          pWorld->WriteIndent();
	  fprintf(fd,"KEY(\"spritematerial\",\"%s\")\n",csspritematerial);
          pWorld->WriteIndent();
          fprintf(fd,"KEY(\"factory\",\"%s\")\n",csfactname);
          pWorld->WriteIndent();
          fprintf(fd,"KEY(\"factorymaterial\",\"%s\")\n",csfactmaterial);
          pWorld->WriteIndent();

          /*
	   * CAUTION!!! If factory isn't loaded for this sprite
           * the material must be loaded already, or, at least in material
	   * list of this model. Otherwise you will obtain a crash
	   * (factory without material error)
           */

          fprintf(fd,"KEY(\"factoryfile\",\"%s\")\n",csmodelfile);

          /*
	   * MDL files have actions defined so if model file is an mdl we
	   * don't need action file. Here we check for file extension, it
	   * won't work if you put a non mdl files with mdl extension
           */
          pWorld->WriteIndent();
          if(!isMdlFile(csmodelfile))
            fprintf(fd,"KEY(\"actionsfile\",\"%s\")\n",csactionsfile);
          else fprintf(fd,"KEY(\"actionfile\",\"NONE\"\n");

	  pWorld->WriteIndent();
          fprintf(fd,"KEY(\"scalefactor\",\"%s\")\n",csscalefactor);

          pWorld->WriteIndent();
          fprintf(fd,"KEY(\"staticflag\",\"%s\")\n",csstaticmodel);

          pWorld->WriteIndent();
          fprintf(fd, "POSITION( %g, %g, %g)\n",Origin.x*ScaleFactor,
                  Origin.z*ScaleFactor, Origin.y*ScaleFactor);


          pWorld->Unindent();
          pWorld->WriteIndent();

          fprintf(fd,")\n\n");
          pWorld->Unindent();
          pWorld->WriteIndent();

		/*
		  pWorld->WriteIndent();
          fprintf(fd, "MESHREF '%s' \n", csnamevalue);
          pWorld->WriteIndent();
          fprintf(fd, "( \n");
          pWorld->Indent();
          pWorld->WriteIndent();
          fprintf(fd, "FACTORY ('%s')\n ", csnamevaluefact);
		  fprintf(fd, "PRIORITY ('Object')\n ");
		  fprintf(fd, "ZUSE( )\n ");
		*/


          /*const char* actionvalue = pEntity->GetValueOfKey("action");
          if (sscanf(actionvalue, "%s%c",action, &dummy) == 1)
          {
            fprintf(fd, "'%s' )\n", action);
          }
	  */
	  //END OF MODIFIED CODE
        }

        /*double x, y, z;
        if (pEntity->GetTripleNumValueOfKey("origin", x,y,z))
        {
          //The strings format matched
          pWorld->WriteIndent();
          fprintf(fd, "MOVE (V ( %g, %g, %g) ",
                  x*ScaleFactor,
                  z*ScaleFactor,
                  y*ScaleFactor);
          //TODO: mixmode; add other keys
          //const char* alphavalue = pEntity->GetValueOfKey("alpha");
          //if (sscanf(alphavalue, "%s%c",alphastr, &dummy) == 1)
          //{
            //pWorld->WriteIndent();
            //fprintf(fd, "MIXMODE (ALPHA (%s))\n", alphastr);
          //}
          //added single value matrix scaler
          //need to also add 9 value matrix for rotation etc
          //if (pEntity->GetValueOfKey("uniformscale", modelscale))
          //{
            //fprintf(fd, " MATRIX ( %s ) )\n", modelscale);
          //}
          //else
          //{
            //fprintf(fd, " )\n");  //add newline if no scale value
          //}
//
          //pWorld->Unindent();
          //pWorld->WriteIndent();
          //fprintf(fd, ") \n\n");
	  */
//        }
      }
    }
  }
  return true;
}

//Write all 2D sprites inside the sector
//SPRITE 'spritename'
//    (
//      VERTICES (-1,1,1,1,1,-1)    //triangular polygon
//      UV (0,0,1,0,1,1)            //texture coordinates
//      TEXNR (texture.gif)         //texture name
//      MOVE (-3,-3,2)              //initial start position
//      MIXMODE (ALPHA (.5))        //settings below
//      COLORS (1,0,0,0,1,0,0,0,1)  //vertex color array
//    )

//MIXMODE arguments (can be combined)
//  COPY ()         //=SRC
//  ADD ()          //=SRC+DST
//  MULTIPLY ()     //=SRC*DST
//  MULTIPLY2 ()    //=2*SRC*DST
//  ALPHA (alpha)   //=(1-alpha)*SRC + alpha*DST
//  TRANSPARENT ()  //=DST
//  KEYCOLOR ()     // color 0 is transparent

bool CCSSector::WriteSprites2D(CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  FILE*     fd          = pWorld->GetFile();
  double    ScaleFactor = pWorld->GetScalefactor();
  assert(pMap);
  assert(fd);

  fprintf(fd, "\n");

  int i;

  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity(i);
    assert(pEntity);

    char   sprname[99] = "none";
    char   dummy;

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "cs_sprite")==0)
    {
      const char* csnamevalue = pEntity->GetValueOfKey("cs_name");
      if (csnamevalue)
      {
        if (sscanf(csnamevalue, "%s%c",sprname, &dummy) == 1)
        {
          pWorld->WriteIndent();
          fprintf(fd, "SPRITE2D '%s' \n", sprname);
          pWorld->WriteIndent();
          fprintf(fd, "( \n");

          //vertices; add square sprite polygon
          pWorld->Indent();
          pWorld->WriteIndent();
          fprintf(fd, "VERTICES (-1,1,1,1,1,-1,-1,-1)\n");

          //UV; add square texture coordinates
          pWorld->WriteIndent();
          fprintf(fd, "UV (0,0,1,0,1,1,0,1)\n");

          pWorld->WriteIndent();
          const char* spritetexture = pEntity->GetValueOfKey("texture");
          fprintf(fd, "MATERIAL (%s)\n", spritetexture);

          //initial start position
          double x, y, z;
          if (pEntity->GetTripleNumValueOfKey("origin", x,y,z))
          {
            //The strings format matched
            pWorld->WriteIndent();
            fprintf(fd, "MOVE ( %g, %g, %g )\n ",
                    x*ScaleFactor,
                    z*ScaleFactor,
                    y*ScaleFactor);
          }

          //mixmode
          pWorld->WriteIndent();
          fprintf(fd, "MIXMODE ( ");
          bool copy = pEntity->GetBoolValueOfKey("copy", false);
          if (copy)
          {
            fprintf(fd, "COPY () ");
          }
          bool add = pEntity->GetBoolValueOfKey("add", false);
          if (add)
          {
            fprintf(fd, "ADD () ");
          }
          bool multiply = pEntity->GetBoolValueOfKey("multiply", false);
          if (multiply)
          {
            fprintf(fd, "MULTIPLY ( ) ");
          }
          bool multiply2 = pEntity->GetBoolValueOfKey("multiply2", false);
          if (multiply2)
          {
            fprintf(fd, "MULTIPLY2 ( ) ");
          }
          char alphastr[99] = "none";
          const char* alphavalue = pEntity->GetValueOfKey("alpha");
          if (sscanf(alphavalue, "%s%c",alphastr, &dummy) == 1)
          {
            fprintf(fd, "ALPHA (%s) ", alphastr);
          }
          bool transparent = pEntity->GetBoolValueOfKey("transparent", false);
          if (transparent)
          {
            fprintf(fd, "TRANSPARENT ( ) ");
          }
          bool keycolor = pEntity->GetBoolValueOfKey("keycolor", false);
          if (keycolor)
          {
            fprintf(fd, "KEYCOLOR ( ) ");
          }

          fprintf(fd, ")\n");

          pWorld->Unindent();
          pWorld->WriteIndent();
          fprintf(fd, ") \n\n");
        }
      }
    }
  }
  return true;
}

bool CCSSector::WriteFog(CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  FILE*     fd          = pWorld->GetFile();

  assert(pMap);
  assert(fd);

  fprintf(fd, "\n");


  double r=255.0;
  double g=255.0;
  double b=255.0;

  //iterate all entities, brushes, polygons and vertices:
  int i;
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity(i);
    assert(pEntity);

    const char* classname = pEntity->GetClassname();
    assert(classname);

    if (strcmp(classname, "fog")==0)
    {

      double fogdensity = pEntity->GetNumValueOfKey("fogdensity", 0.0);
      if (fogdensity>0.0)
      {
        pEntity->GetTripleNumValueOfKey("fogcolor", r, g, b);
        pWorld->WriteIndent();
        fprintf(fd, "FOG(%g,%g,%g,%g)\n", r/255.0, g/255.0, b/255.0, fogdensity);
      }
    }
  }
  return true;
}
