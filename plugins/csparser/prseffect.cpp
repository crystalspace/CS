/*
    Copyright (C) 2002 Marten Svanfeldt

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
#include "csloader.h"
#include "iutil/databuff.h"
#include "csutil/scanstr.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "imap/ldrctxt.h"

#include "ivideo/effects/efserver.h"
#include "ivideo/effects/efdef.h"
#include "ivideo/effects/efvector4.h"
#include "ivideo/effects/efstring.h"

bool csLoader::LoadEffectFile(const char* filename)
{
  csRef<iDataBuffer> buf (VFS->ReadFile (filename));

  if (!buf || !buf->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.effect",
    	      "Could not open effect file '%s' on VFS!", filename);
    return false;
  }

  ResolveOnlyRegion = false;
  SCF_DEC_REF (ldr_context); ldr_context = NULL;

  csRef<iDocument> doc;
  bool er = TestXml (filename, buf, doc);
  if (!er) return false;
  if (doc)
  {
    return ParseEffectList (doc->GetRoot ());
  }
  else
  {
    ReportNotify ("crystalspace.maploader.parse.effect",
		    "All effectfiles MUST be xml-files (file '%s')!", filename);
    return false;
  }
}

bool csLoader::ParseEffectList(iDocumentNode *node)
{
  csRef<iEffectServer> effect_server = CS_QUERY_REGISTRY(csLoader::object_reg, iEffectServer);

  if(!effect_server.IsValid())
  {
    ReportNotify ("No effectserver found. Ignoring effects!");
    return false;
  }

  csRef<iDocumentNodeIterator> it = node->GetNode("effects")->GetNodes();
  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if(child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID id = xmltokens.Request(child->GetValue());
    switch(id)
    {
    case XMLTOKEN_EFFECT:
      if(!ParseEffect(child, effect_server))
        return false;
      break;
    default:
      SyntaxService->ReportBadToken(child);
      return false;
    }
  }

  return true;
}

bool csLoader::ParseEffect(iDocumentNode *node, iEffectServer *pParent)
{
  if(pParent == NULL) return false;

  const char* effectname = node->GetAttributeValue("name");

  csRef<iEffectDefinition> efdef = pParent->CreateEffect();
  efdef->SetName(effectname);

  csRef<iDocumentNodeIterator> it = node->GetNodes();
  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if(child->GetType() != CS_NODE_ELEMENT) return false;
    csStringID id = xmltokens.Request(child->GetValue());

    switch(id)
    {
    case XMLTOKEN_VARIABLE:
      {
        int vn = efdef->GetVariableID( pParent->RequestString(child->GetAttributeValue("name")), true);
        const char* type = child->GetAttributeValue("type");
        if (strcasecmp(type, "float") == 0)
        {
          //is float
          float def = child->GetAttributeValueAsFloat("default");
          efdef->SetVariableFloat(vn, def);
        }else if (strcasecmp(type, "vec4") == 0)
        {
          //is a vec4
          const char* def = child->GetAttributeValue("default");
          csEffectVector4 vec;
          csScanStr(def, "%f,%f,%f,%f", &vec.x,&vec.y,&vec.z,&vec.w);
          efdef->SetVariableVector4(vn, vec);
        }
        break;
      }
    case XMLTOKEN_TECHNIQUE:
      {
        csRef<iEffectTechnique> tech = efdef->CreateTechnique();
       
        if(!ParseEffectTech(child, tech))
          return false;

        break;
      }
    default:
      SyntaxService->ReportBadToken(child);
      return false;
    }
  }
  if(!pParent->Validate(efdef))
    ReportNotify( "The effect %s couldn't be validated, and is not going to be used!", efdef->GetName());
  
  return true;
}

bool csLoader::ParseEffectTech(iDocumentNode* node, iEffectTechnique *tech)
{
  if(tech == NULL) return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes();
  
  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();

    if(child->GetType() != CS_NODE_ELEMENT) break;
    csStringID id = xmltokens.Request(child->GetValue());

    switch(id)
    {
    case XMLTOKEN_QUALITY:
      tech->SetQuality(child->GetContentsValueAsFloat());
      break;
    case XMLTOKEN_PASS:
      {
        csRef<iEffectPass> pass = tech->CreatePass();

        if(!ParseEffectPass(child, pass))
          return false;

        break;
      }
    default:
      SyntaxService->ReportBadToken(child);
      return false;
    }
  }
  return true;
}

bool csLoader::ParseEffectPass(iDocumentNode* node, iEffectPass* pass)
{
  if(pass == NULL) return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes();
  
  csRef<iEffectServer> effect_server = CS_QUERY_REGISTRY(csLoader::object_reg, iEffectServer);
  csEffectStrings* ef_strings = effect_server->GetStandardStrings();

  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();

    if(child->GetType() != CS_NODE_ELEMENT) break;
    csStringID id = xmltokens.Request(child->GetValue());

    switch(id)
    {
    case XMLTOKEN_BLENDING:
      pass->SetStateString(ef_strings->blending, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_SHADING:
      pass->SetStateString(ef_strings->shade_mode, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_SOURCEBLEND:
      pass->SetStateString(ef_strings->source_blend_mode, effect_server->RequestString(child->GetContentsValue()));
      break;  
    case XMLTOKEN_DESTINATIONBLEND:
      pass->SetStateString(ef_strings->destination_blend_mode, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_VERTEXCOLOR:
      pass->SetStateString(ef_strings->vertex_color_source, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_VERTEXPROGRAM: 
      {
        csRef<iDataBuffer> db = VFS->ReadFile(child->GetContentsValue());
        if(!db || !db->GetSize())
        {
          ReportNotify ("Couldn't find vertex-program! Ignoring it!");
          break;
        }

        pass->SetStateString(ef_strings->nvvertex_program_gl, effect_server->RequestString( (char*)db->GetData()));
      }
      break;
    case XMLTOKEN_VERTEXPROGRAMCONST:
      {
        int num = child->GetAttributeValueAsInt("number");
        char* mybuff = new char[30];

        sprintf(mybuff, "vertex program constant %d", num);
        pass->SetStateString(effect_server->RequestString(mybuff), effect_server->RequestString(child->GetContentsValue()));

        delete []mybuff;
      }
      break;
    case XMLTOKEN_LAYER:
      {
        csRef<iEffectLayer> layer = pass->CreateLayer();
        
        if(!ParseEffectLayer(child, layer))
          return false;
        break;
      }
    default:
      SyntaxService->ReportBadToken(child);
      return false;
    }
  }
  return true;
}

bool csLoader::ParseEffectLayer(iDocumentNode* node, iEffectLayer* layer)
{
  if(layer == NULL) return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes();
  csRef<iEffectServer> effect_server = CS_QUERY_REGISTRY(csLoader::object_reg, iEffectServer);
  csEffectStrings* ef_strings = effect_server->GetStandardStrings();

  
  while(it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();

    if(child->GetType() != CS_NODE_ELEMENT) break;
    csStringID id = xmltokens.Request(child->GetValue());

    switch(id)
    {
    case XMLTOKEN_TEXTURESOURCE:
      {
        if(strcasecmp(child->GetContentsValue(), "fog")==0)
          layer->SetStateString(ef_strings->texture_source, ef_strings->fog);
        else
          layer->SetStateFloat(ef_strings->texture_source, child->GetContentsValueAsFloat());
      }
      break;
    case XMLTOKEN_TEXTURECOORDSOURCE:
      {
        if(strcasecmp(child->GetContentsValue(), "fog")==0)
          layer->SetStateString(ef_strings->texture_coordinate_source, ef_strings->fog);
        else if(strcasecmp(child->GetContentsValue(), "mesh")==0)
          layer->SetStateString(ef_strings->texture_coordinate_source, ef_strings->mesh);
        else
          layer->SetStateFloat(ef_strings->texture_coordinate_source, child->GetContentsValueAsFloat());
      }
      break;
      //ALOT OF REPEATING STUFF (ALMOST IDENTICAL)
    case XMLTOKEN_COLORSOURCE1:
      layer->SetStateString(ef_strings->color_source_1, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_COLORSOURCE2:
      layer->SetStateString(ef_strings->color_source_2, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_COLORSOURCE3:
      layer->SetStateString(ef_strings->color_source_3, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_COLORMODIFIER1:
      layer->SetStateString(ef_strings->color_source_modifier_1, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_COLORMODIFIER2:
      layer->SetStateString(ef_strings->color_source_modifier_2, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_COLORMODIFIER3:
      layer->SetStateString(ef_strings->color_source_modifier_3, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHASOURCE1:
      layer->SetStateString(ef_strings->alpha_source_1, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHASOURCE2:
      layer->SetStateString(ef_strings->alpha_source_2, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHASOURCE3:
      layer->SetStateString(ef_strings->alpha_source_3, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHAMODIFIER1:
      layer->SetStateString(ef_strings->alpha_source_modifier_1, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHAMODIFIER2:
      layer->SetStateString(ef_strings->alpha_source_modifier_2, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHAMODIFIER3:
      layer->SetStateString(ef_strings->alpha_source_modifier_3, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_COLOROPERATION:
      layer->SetStateString(ef_strings->color_operation, effect_server->RequestString(child->GetContentsValue()));
      break;
    case XMLTOKEN_ALPHAOPERATION:
      layer->SetStateString(ef_strings->alpha_operation, effect_server->RequestString(child->GetContentsValue()));
      break;
    default:
      SyntaxService->ReportBadToken(child);
      return false;
    }
  }
  return true;
}