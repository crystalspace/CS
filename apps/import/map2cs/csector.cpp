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
#include "csutil/csstring.h"

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

#include "dochelp.h"

namespace
{

char* BuildMaterialKey (int i, CMapEntity* pEntity)
{
  char* key = new char[1024];
  char* returnvalue = new char[1024];
  sprintf(key,"MaterialName%d",i);
  printf(key);
  if(!pEntity->GetValueOfKey(key)) return 0;
  sprintf(returnvalue,"<key name=\"defmaterial\" value=\"%s\" />\n",
	      pEntity->GetValueOfKey(key));
  return returnvalue;
}

//Auxiliar funtion to check if the file name is a mdl file
bool isMdlFile(const char* csmodelfile)
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
char* GetFactoryName(const char * csmodelfile)
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

} // end of anonymous namespace


CCSSector::CCSSector(CMapBrush* pBrush)
  : CISector(pBrush)
{
}

CCSSector::~CCSSector()
{
}

bool CCSSector::Write(csRef<iDocumentNode> node, CIWorld* pIWorld)
{
  assert(pIWorld);
  CCSWorld* pWorld  = (CCSWorld*)  pIWorld;

  CMapFile* pMap = pWorld->GetMap();

  assert(pMap);

  DocNode sector = CreateNode (node, "sector");
  sector->SetAttribute ("name", GetName());
  CMapEntity* worldspawn = pWorld->GetWorldspawn ();
  if (worldspawn->GetBoolValueOfKey ("dynavis"))
  {
    CreateNode (sector, "cullerp", "crystalspace.culling.dynavis");
  }

  /*if (m_IsDefaultsector && pWorld->NeedSkysector())
  {
    pWorld->WriteSky(node);
  }
  else*/

  if (m_IsDefaultsector && 
    !worldspawn->GetBoolValueOfKey ("skyportalsonly", false))
  {
    bool hasSky = worldspawn->GetValueOfKey("skybox") || 
      worldspawn->GetValueOfKey("skydome");

    if (hasSky)
    {
      DocNode meshobj = CreateNode (sector, "meshobj");
      meshobj->SetAttribute ("name", "_defaultsky");

      CreateNode (meshobj, "plugin", "thing");
      CreateNode (meshobj, "zfill");
      CreateNode (meshobj, "priority", "sky");
      CreateNode (meshobj, "camera");

      DocNode params = CreateNode (meshobj, "params");

      pWorld->WriteSky (params);
    }
  }

  {
    if (m_Portals.Length() > 0)
    {
      CMapEntity* pEntity = m_pOriginalBrush->GetEntity();

      DocNode meshobj = CreateNode (sector, "portals");
      csString meshname;
      meshname.Format ("%s_portals", GetName());
      if (pEntity)
      {
	meshname = pEntity->GetValueOfKey ("cs_name", meshname);
      }

      size_t i;
      size_t j;

      for (i=0; i<m_Portals.Length(); i++)
      {
        CIPortal* pPortal = m_Portals[i];
        for (j=0; j<pPortal->GetPolygonCount(); j++)
        {
	  csString portalName;
	  portalName.Format ("%s_%zu_%zu", meshname.GetData(), i, j);

	  DocNode        portal = CreateNode(meshobj, "portal");
	  portal->SetAttribute ("name", portalName);
 	  CMapPolygon*   pPolygon = pPortal->GetPolygon(j);
 	  
 	  CVertexBuffer Vb;
 	  for (size_t l = pPolygon->GetVertexCount() - 1 ; l >= 0; l-- ) 
	  {
 	    Vb.AddVertex(pPolygon->GetVertex(l));
 	  }
 	  Vb.WriteCS(portal, pWorld);
 	  
 	  CreateNode(portal, "sector", pPortal->GetTargetSector()->GetName());        
	}
      }
    } //if contains any polygons
  }

  //WriteWorldspawn(params, pWorld);
  WriteWorldspawn(sector, pWorld);

  WriteFog(sector, pWorld);
  WriteLights(sector, pWorld);
  WriteCurves(sector, pWorld);
  WriteThings(sector, pWorld);
  WriteSprites(sector, pWorld);
  WriteSprites2D(sector, pWorld);
  WriteNodes (sector, pWorld);

//  if (pMap->GetConfigInt("Map2CS.General.UseBSP", 0))
//  {
//    pWorld->WriteIndent();
//    fprintf(fd, "CULLER('static')\n");
//  }

  return true;
}

bool CCSSector::WriteWorldspawn(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  size_t i;
  for (i=0; i<m_Things.Length(); i++)
  {
    if (strcasecmp(m_Things[i]->GetClassname(), "worldspawn")==0)
    {
      CMapEntity* pEntity = m_pOriginalBrush->GetEntity();

      DocNode meshobj = CreateNode (node, "meshobj");
      csString meshname;
      meshname.Format ("%s_walls", GetName());
      if (pEntity)
      {
	meshname = pEntity->GetValueOfKey ("cs_name", meshname);
      }
      meshobj->SetAttribute ("name", meshname);
      CreateNode (meshobj, "plugin", "thing");
      CreateNode (meshobj, "zuse");

      CCSWorld::WriteKeys(meshobj, pWorld, pEntity);

      DocNode params = CreateNode (meshobj, "params");

      bool Sky;
      if (!((CCSThing*)m_Things[i])->WriteAsPart(params, pWorld, this, Sky))
      {
        return false;
      }

      DocNode priority = meshobj->CreateNodeBefore (CS_NODE_ELEMENT,
	params);
      priority->SetValue ("priority");
      priority = priority->CreateNodeBefore (CS_NODE_TEXT);
      priority->SetValue ("wall");
    }
  }
  return true;
}

bool CCSSector::WriteThings(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  size_t i;
  for (i=0; i<m_Things.Length(); i++)
  {
    if (strcasecmp(m_Things[i]->GetClassname(), "worldspawn")!=0)
    {
      if (!m_Things[i]->Write(node, pWorld, this))
      {
        return false;
      }
    }
  }
  return true;
}

bool CCSSector::WriteLights(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  double    ScaleFactor = pWorld->GetScalefactor();

  assert(pMap);

  size_t i;

  //iterate all entities, brushes, polygons and vertices:
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity((int)i);
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
	  //Also added in: "_color" from Quake3 for more support with visuals
	  //in GtkRadiant.

          bool   LightOk         = false;
	  bool	 ColorOk	 = false;
          double r               = 255;
          double g               = 255;
          double b               = 255;
          double radius          = 200;
	  double influenceRadius = -1;
	  double brightness      = 1.0;
          bool   dynamic         = false;
          double halo            = 0;
          double halointen       = 0;
          double halocross       = 0;
          char   attenuation[99] = "none";

          const char* lightvalue = pEntity->GetValueOfKey("_light");
          if (lightvalue)
          {
            char dummy;
            if (sscanf(lightvalue, "%lf %lf %lf %lf%c",
	    	&r, &g, &b, &radius, &dummy)==4)
            {
              LightOk = true;
	      ColorOk = true;
            }
          }

	  const char* colorvalue = pEntity->GetValueOfKey("_color");
	  if (colorvalue)
	  {
	    char dummy;
	    double r2            = 1.0;
	    double g2            = 1.0;
	    double b2            = 1.0;

	    if (sscanf(colorvalue, "%lf %lf %lf%c", &r2, &g2, &b2, &dummy)==3)
	    {
	      r = r2 * 255;
	      g = g2 * 255;
	      b = b2 * 255;
	      ColorOk = true;
	    }
	  }

          if (!LightOk)
          {
            radius = pEntity->GetNumValueOfKey("light", 100);
	    if (!ColorOk) 
	    {
              r      = 255;
              g      = 255;
              b      = 255;
	    }
          }

	  brightness = pEntity->GetNumValueOfKey("brightness", 1.0);

	  influenceRadius = pEntity->GetNumValueOfKey("influence", -1.0);

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
              if (!(strcmp(attenuation,"none") ||
	            strcmp(attenuation,"linear") ||
                    strcmp(attenuation,"inverse") ||
		    strcmp(attenuation,"realistic") ))
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

	  DocNode light = CreateNode (node, "light");
	  DocNode center = CreateNode (light, "center");
	  center->SetAttributeAsFloat ("x", origin.x*ScaleFactor);
	  center->SetAttributeAsFloat ("y", origin.z*ScaleFactor);
	  center->SetAttributeAsFloat ("z", origin.y*ScaleFactor);
	  
	  // New Radius / Color code
	  DocNode radiusNode = CreateNode (light, "radius", (float)(radius*lightscale*ScaleFactor));
	  radiusNode->SetAttributeAsFloat ("brightness", brightness);
	  
	  DocNode color = CreateNode (light, "color");
	  color->SetAttributeAsFloat("red", (r/255));
	  color->SetAttributeAsFloat("green", (g/255));
	  color->SetAttributeAsFloat("blue", (b/255));

	  /* This code is Old (keeping it incase it somehow stops working)
	  CreateNode (light, "radius", 
	    (float)(radius*ScaleFactor*lightscale));
	  DocNode color = CreateNode (light, "color");
	  color->SetAttributeAsFloat ("red", (r/255) * (radius/128));
	  color->SetAttributeAsFloat ("green", (g/255) * (radius/128));
	  color->SetAttributeAsFloat ("blue", (b/255) * (radius/128));
	  */

	  if (influenceRadius != -1.0)
	  {
	    CreateNode (light, "influenceradius", (float)(influenceRadius*ScaleFactor*lightscale));
	  }

	  if (dynamic)
          {
            CreateNode (light, "dynamic");
          }
	  {
	    DocNode halo = CreateNode (light, "halo");
	    CreateNode (halo, "type", "cross");
	    CreateNode (halo, "intensity", (float)halointen);
	    CreateNode (halo, "cross", (float)halointen);
	  }

	  CreateNode (light, "attenuation", attenuation);
        } //if (light is inside this sector)
      } // if (entity had origin)
    } //if (classname == "light")
  } //for (all entities)
  return true;
}

/// Write all nodes inside the sector
bool CCSSector::WriteNodes(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  assert(pWorld);

  double ScaleFactor = pWorld->GetScalefactor();

  //iterate all nodes
  size_t i;
  for (i=0; i<m_Nodes.Length(); i++)
  {
    CMapEntity* pEntity = m_Nodes[i];
    CdVector3 Origin(0,0,0);
    pEntity->GetOrigin(Origin);
    if (strcmp(pEntity->GetValueOfKey("classname"),"cs_model")){
      DocNode Node = CreateNode (node, "node");
      Node->SetAttribute ("name", pEntity->GetName());
      CCSWorld::WriteKeys(Node, pWorld, pEntity);
      DocNode position = CreateNode (Node, "position");
      position->SetAttributeAsFloat ("x", Origin.x*ScaleFactor);
      position->SetAttributeAsFloat ("y", Origin.z*ScaleFactor);
      position->SetAttributeAsFloat ("z", Origin.y*ScaleFactor);
    }
  }

  return true;
}

bool CCSSector::WriteCurves(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap         = pWorld->GetMap();

  assert(pMap);

  size_t i, curve;
  for (i=0; i<pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = pMap->GetEntity(i);
    size_t const ncurves = pEntity->GetCurveCount();
    if (ncurves>0)
    {
      for (curve=0; curve<ncurves; curve++)
      {
        CMapCurve*    pCurve   = pEntity->GetCurve(curve);
        CTextureFile* pTexture = pCurve->GetTexture();
        if (pTexture->IsVisible())
        {
	  DocNode meshobj = CreateNode (node, "meshobj");
	  meshobj->SetAttribute ("name",
	    csString().Format ("%s_e%zu_c%zu", pEntity->GetName(), i, curve));
	  CreateNode (meshobj, "plugin", "bezier");
	  //CreateNode (meshobj, "zuse");
	  //CreateNode (meshobj, "priority", pEntity->GetValueOfKey("priority", "object"));

	  DocNode params = CreateNode (meshobj, "params");
          CreateNode (params, "factory", 
	    csString().Format ("curve_%s", pCurve->GetName()));

	  DocNode move = CreateNode (meshobj, "move");
	  DocNode v = CreateNode (move, "v");
	  v->SetAttributeAsInt ("x", 0);
	  v->SetAttributeAsInt ("y", 0);
	  v->SetAttributeAsInt ("z", 0);
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

bool CCSSector::WriteSprites(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  // @@@ check if this still works
#if 0
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  FILE*     fd          = pWorld->GetFile();
  double    ScaleFactor = pWorld->GetScalefactor();
  //const char*   modelscale = "1";   //df added matrix scaler

  assert(pMap);

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
          fprintf(fd,"<node name=\"SEED_MESH_OBJ\">\n");
          // Key/Value pairs writting
          pWorld->Indent();
          pWorld->WriteIndent();
          fprintf(fd,"<key name=\"classname\" value=\"SEED_MESH_OBJ\" />\n");
          pWorld->WriteIndent();
          fprintf(fd,"<key name=\"cs_name\" value=\"%s\" />\n",csnamevalue);

          /*Lets build all material definition
           *keys KEY("defmaterial","materialname, texturefile")
           * It will declare materials until BuildMateriaKey returns a 0
           */
          int mIndex; //material Index
          char* key;
          for (mIndex = 1; (key = BuildMaterialKey(mIndex,pEntity)); mIndex++)
	  {
            pWorld->WriteIndent();
            fprintf(fd,key);
	  }

          pWorld->WriteIndent();
	  fprintf(fd,"<key name=\"spritematerial\" value=\"%s\" />\n",csspritematerial);
          pWorld->WriteIndent();
          fprintf(fd,"<key name=\"factory\" value=\"%s\" />\n",csfactname);
          pWorld->WriteIndent();
          fprintf(fd,"<key name=\"factorymaterial\" value=\"%s\" />\n",csfactmaterial);
          pWorld->WriteIndent();

          /*
	   * CAUTION!!! If factory isn't loaded for this sprite
           * the material must be loaded already, or, at least in material
	   * list of this model. Otherwise you will obtain a crash
	   * (factory without material error)
           */

          fprintf(fd,"<key name=\"factoryfile\" value=\"%s\" />\n",csmodelfile);

          /*
	   * MDL files have actions defined so if model file is an mdl we
	   * don't need action file. Here we check for file extension, it
	   * won't work if you put a non mdl files with mdl extension
           */
          pWorld->WriteIndent();
          if(!isMdlFile(csmodelfile))
            fprintf(fd,"<key name=\"actionsfile\" value=\"%s\" />\n",csactionsfile);
          else fprintf(fd,"<key name=\"actionfile\" value=\"NONE\" />\n");

	  pWorld->WriteIndent();
          fprintf(fd,"<key name=\"scalefactor\" value=\"%s\" />\n",csscalefactor);

          pWorld->WriteIndent();
          fprintf(fd,"<key name=\"staticflag\" value=\"%s\" />\n",csstaticmodel);

          pWorld->WriteIndent();
          fprintf(fd, "<position x=\"%g\" y=\"%g\" z=\"%g\" />\n",Origin.x*ScaleFactor,
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
#endif
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

bool CCSSector::WriteSprites2D(csRef<iDocumentNode> node, CIWorld* pWorld)
{
  assert(pWorld);

  CMapFile* pMap        = pWorld->GetMap();
  //double    ScaleFactor = pWorld->GetScalefactor();
  assert(pMap);

  //int i;

  //iterate all entities, brushes, polygons and vertices:
/*  for (i=0; i<pMap->GetNumEntities(); i++)
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
  }*/
  return true;
}

bool CCSSector::WriteFog(csRef<iDocumentNode> node, CIWorld* pWorld)
{
/*  assert(pWorld);

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
  }*/
  return true;
}
