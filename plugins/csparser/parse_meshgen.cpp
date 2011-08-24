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

#include "csthreadedloader.h"

#include "iengine/meshgen.h"
#include "imap/services.h"
#include "iutil/vfs.h"
#include "ivaria/terraform.h"

#include "csutil/stringquote.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  bool csThreadedLoader::LoadMeshGen (iLoaderContext* ldr_context,
    iDocumentNode* node, iSector* sector)
  {
    const char* name = node->GetAttributeValue ("name");
    iMeshGenerator* meshgen = sector->CreateMeshGenerator (name);
    ldr_context->AddToCollection(meshgen->QueryObject ());

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_GEOMETRY:
        if (!LoadMeshGenGeometry (ldr_context, child, meshgen))
          return false;
        break;
      case XMLTOKEN_MESHOBJ:
        {
          const char* meshname = child->GetContentsValue ();
          iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
          if (!mesh)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find mesh object %s for mesh generator!",
              CS::Quote::Single (meshname));
            return false;
          }
          meshgen->AddMesh (mesh);
        }
        break;
      case XMLTOKEN_DENSITYSCALE:
        {
          float mindist = child->GetAttributeValueAsFloat ("mindist");
          float maxdist = child->GetAttributeValueAsFloat ("maxdist");
          float maxfactor = child->GetAttributeValueAsFloat ("maxfactor");
          meshgen->SetDensityScale (mindist, maxdist, maxfactor);
        }
        break;
      case XMLTOKEN_ALPHASCALE:
        {
          float mindist = child->GetAttributeValueAsFloat ("mindist");
          float maxdist = child->GetAttributeValueAsFloat ("maxdist");
          meshgen->SetAlphaScale (mindist, maxdist);
        }
        break;
      case XMLTOKEN_NUMBLOCKS:
        meshgen->SetBlockCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_CELLDIM:
        meshgen->SetCellCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_SAMPLEBOX:
        {
          csBox3 b;
          if (!SyntaxService->ParseBox (child, b))
            return false;
          meshgen->SetSampleBox (b);
        }
        break;
      case XMLTOKEN_DEFAULTDENSITY:
        meshgen->SetDefaultDensityFactor (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_DENSITYFACTORMAP:
	{
	  if (!LoadMeshGenDensityFactorMap (child, meshgen))
	    return false;
	}
	break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }
    return true;
  }

  bool csThreadedLoader::LoadMeshGenGeometry (iLoaderContext* ldr_context,
    iDocumentNode* node,
    iMeshGenerator* meshgen)
  {
    iMeshGeneratorGeometry* geom = meshgen->CreateGeometry ();

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_FACTORY:
        {
          const char* factname = child->GetAttributeValue ("name");
          float maxdist = child->GetAttributeValueAsFloat ("maxdist");
          iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
          if (!fact)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find mesh factory %s for mesh generator!",
              CS::Quote::Single (factname));
            return false;
          }
          geom->AddFactory (fact, maxdist);
        }
        break;
      case XMLTOKEN_POSITIONMAP:
        {
          const char* map_name = child->GetAttributeValue ("mapname");
          csRef<iTerraFormer> map = csQueryRegistryTagInterface<iTerraFormer> 
            (object_reg, map_name);
          if (!map)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find map position map terraformer %s!", CS::Quote::Single (map_name));
            return false;
          }

          csVector2 min, max;
          csRef<iDocumentNode> region_node = child->GetNode ("region");

          SyntaxService->ParseVector (region_node->GetNode ("min"), min);
          SyntaxService->ParseVector (region_node->GetNode ("max"), max);

          float value = child->GetAttributeValueAsFloat ("value");

          uint resx = child->GetAttributeValueAsInt ("resx");
          uint resy = child->GetAttributeValueAsInt ("resy");

          csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
            object_reg, "crystalspace.shared.stringset");

          geom->AddPositionsFromMap (map, csBox2 (min.x, min.y, max.x, max.y), 
            resx, resy, value, strings->Request ("heights"));
        }
        break;
      case XMLTOKEN_DENSITYMAP:
        {
          const char* map_name = child->GetContentsValue ();
          csRef<iTerraFormer> map_tf = csQueryRegistryTagInterface<iTerraFormer> 
            (object_reg, map_name);
          if (!map_tf)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find map density map terraformer %s!", CS::Quote::Single (map_name));
            return false;
          }
          float factor = child->GetAttributeValueAsFloat ("factor");
          csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
            object_reg, "crystalspace.shared.stringset");
          geom->SetDensityMap (map_tf, factor, strings->Request ("densitymap"));
        }
        break;
      case XMLTOKEN_DENSITYFACTORMAP:
        {
	  const char* mapID = child->GetContentsValue();
	  if (!mapID || !*mapID)
	  {
	    ReportWarning (
	      "crystalspace.maploader.parse.meshgen",
	      child, "No name of the density factor map to use was given.");
	    return false;
	  }
	  float factor = 1;
	  csRef<iDocumentAttribute> factorAttr (child->GetAttribute ("factor"));
	  if (factorAttr) factor  = factorAttr->GetValueAsFloat();
	  if (!geom->UseDensityFactorMap (mapID, factor))
	  {
	    ReportWarning (
	      "crystalspace.maploader.parse.meshgen",
	      child, "Could not use density factor map %s. Invalid name?",
	      CS::Quote::Single (mapID));
	    return false;
	  }
	}
	break;
      case XMLTOKEN_MATERIALFACTOR:
        {
          const char* matname = child->GetAttributeValue ("material");
          if (!matname)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "%s attribute is missing!",
	      CS::Quote::Single ("material"));
            return false;
          }
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
          if (!mat)
          {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Can't find material %s!", CS::Quote::Single (matname));
            return false;
          }
          float factor = child->GetAttributeValueAsFloat ("factor");
          geom->AddDensityMaterialFactor (mat, factor);
        }
        break;
      case XMLTOKEN_DEFAULTMATERIALFACTOR:
        {
          float factor = child->GetContentsValueAsFloat ();
          geom->SetDefaultDensityMaterialFactor (factor);
        }
        break;
      case XMLTOKEN_RADIUS:
        geom->SetRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_DENSITY:
        geom->SetDensity (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_WINDBIAS:
        geom->SetWindBias(child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_WINDDIRECTION:
        geom->SetWindDirection(child->GetAttributeValueAsFloat("x"),
          child->GetAttributeValueAsFloat("y"));
        break;
      case XMLTOKEN_WINDSPEED:
        geom->SetWindSpeed(child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MINDRAWDIST:
        geom->SetMinimumDrawDistance(child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MINOPAQUEDIST:
        geom->SetMinimumOpaqueDistance(child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_MAXOPAQUEDIST:
        geom->SetMaximumOpaqueDistance(child->GetContentsValueAsFloat ());
        break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }
    return true;
  }

  bool csThreadedLoader::LoadMeshGenDensityFactorMap (iDocumentNode* mapNode,
						      iMeshGenerator* meshgen)
  {
    const char* mapID = mapNode->GetAttributeValue ("name");
    if (!mapID || !*mapID)
    {
      ReportWarning (
	"crystalspace.maploader.parse.meshgen",
	mapNode, "%s atttribute required",
	CS::Quote::Single ("name"));
      return false;
    }
    
    csRef<iImage> image;
    CS::Math::Matrix4 world2map;
    bool world2map_given = false;

    csRef<iDocumentNodeIterator> it = mapNode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_IMAGE:
	{
	  const char* imageFile = child->GetContentsValue();
	  csRef<iThreadReturn> itr;
	  itr.AttachNew (new csLoaderReturn (threadman));
	  LoadImageTC (itr, false, vfs->GetCwd(), imageFile, CS_IMGFMT_ANY, true);
	  image = scfQueryInterfaceSafe<iImage> (itr->GetResultRefPtr());
	  if (!image)
	    // LoadImageTC reported error
	    return false;
	}
	break;
      case XMLTOKEN_SCALE:
	{
	  float w = child->GetAttributeValueAsFloat ("w");
	  float h = child->GetAttributeValueAsFloat ("h");
	  if (w <= 0.0f || h <= 0.0f)
	  {
            SyntaxService->ReportError (
              "crystalspace.maploader.parse.meshgen",
              child, "Bad values for 'w' and 'h' in 'scale'. Should be > 0!");
	    return false;
	  }
	  float ox = child->GetAttributeValueAsFloat ("offsetx");
	  float oy = child->GetAttributeValueAsFloat ("offsety");
	  world2map.Set (CS::Math::Matrix4 ());
	  world2map.m11 = 1.0f / w;
	  world2map.m23 = -1.0f / h;
	  world2map.m14 = 0.5 - world2map.m11 + ox;
	  world2map.m24 = 0.5 + world2map.m11 + oy;
	  world2map_given = true;
	}
	break;
      case XMLTOKEN_WORLD2IMAGE:
	{
	  if (!SyntaxService->ParseMatrix (child, world2map))
	    // ParseMatrix reported error
	    return false;
	  world2map_given = true;
	}
	break;
      default:
        SyntaxService->ReportBadToken (child);
        return false;
      }
    }
    
    if (!image)
    {
      ReportWarning ("crystalspace.maploader.parse.meshgen",
		     mapNode,
		     "An <image> node is required.");
      return false;
    }
    if (!world2map_given)
    {
      ReportWarning ("crystalspace.maploader.parse.meshgen",
		     mapNode,
		     "No <world2map> matrix given. Probably not what you want.");
    }
    
    meshgen->AddDensityFactorMap (mapID, image, world2map);
    
    return true;
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
