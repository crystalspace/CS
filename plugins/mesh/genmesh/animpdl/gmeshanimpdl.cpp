/*
    Copyright (C) 2007 by Jorrit Tyberghein
              (C) 2007 by Frank Richter

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
#include "csgeom/math3d.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/util.h"
#include "csutil/event.h"
#include "iengine/engine.h"
#include "iutil/document.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "imap/services.h"
#include "gmeshanimpdl.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(GMeshAnimPDL)
{

SCF_IMPLEMENT_FACTORY (GenmeshAnimationPDLType)

void GenmeshAnimationPDL::Prepare ()
{
  if (prepared) return;
  prepared = true;

  csRef<iEngine> engine = csQueryRegistry<iEngine> (factory->type->object_reg);
  if (!engine)
  {
    factory->Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine!");
    return;
  }

  for (size_t i = 0; i < factory->lights.GetSize(); i++)
  {
    const GenmeshAnimationPDLFactory::MappedLight& factoryLight = 
      factory->lights[i];
    bool success = false;
    MappedLight newLight;
    newLight.colors = factoryLight.colors;
    newLight.light = engine->FindLightID (factoryLight.lightId);
    if (newLight.light)
    {
      success = true;
      newLight.light->AddAffectedLightingInfo (
        static_cast<iLightingInfo*> (this));
    }
    else
    {
      csString hexId;
      for (int i = 0; i < 16; i++)
        hexId.AppendFmt ("%02x", factoryLight.lightId[i]);
      factory->Report (CS_REPORTER_SEVERITY_WARNING, 
        "Could not find light with ID '%s'", hexId.GetData());
    }
    if (success)
    {
      lights.Push (newLight);
    }
  }
  lights.ShrinkBestFit();
}

GenmeshAnimationPDL::GenmeshAnimationPDL (
  GenmeshAnimationPDLFactory* fact) : scfImplementationType(this), 
  factory (fact), prepared (false), lightsDirty (true), lastMeshVersion (~0)
{
}

GenmeshAnimationPDL::~GenmeshAnimationPDL ()
{
}

const csVector3* GenmeshAnimationPDL::UpdateVertices (csTicks /*current*/,
	const csVector3* verts, int /*num_texels*/, uint32 /*version_id*/)
{
  return verts;
}

const csVector2* GenmeshAnimationPDL::UpdateTexels (csTicks /*current*/,
	const csVector2* texels, int /*num_texels*/, uint32 /*version_id*/)
{
  return texels;
}

const csVector3* GenmeshAnimationPDL::UpdateNormals (csTicks /*current*/,
	const csVector3* normals, int /*num_normals*/, uint32 /*version_id*/)
{
  return normals;
}

const csColor4* GenmeshAnimationPDL::UpdateColors (csTicks current,
	const csColor4* colors, int num_colors, uint32 version_id)
{
  if (!prepared) Prepare();

  if (lightsDirty || (lastMeshVersion != version_id))
  {
    size_t numUpdate = csMin (factory->staticColors->GetElementCount(), 
      (size_t)num_colors);

    combinedColors.SetSize (numUpdate);
    {
      csVertexListWalker<float, csColor> color (factory->staticColors, 3);
      for (size_t n = 0; n < numUpdate; n++)
      {
        combinedColors[n].Set ((*color).red, (*color).green, (*color).blue, 
          colors[n].alpha);
        ++color;
      }
    }

    for (size_t l = 0; l < lights.GetSize(); l++)
    {
      const MappedLight& light = lights[l];
      
      csVertexListWalker<float, csColor> color (light.colors, 3);
      csColor lightColor = light.light->GetColor();

      for (size_t n = 0; n < numUpdate; n++)
      {
        combinedColors[n] += *color * lightColor;
        ++color;
      }
    }

    lightsDirty = false;
    lastMeshVersion = version_id;
  }

  return combinedColors.GetArray();
}

void GenmeshAnimationPDL::LightDisconnect (iLight* light)
{
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (lights[i].light == light)
    {
      lights.DeleteIndexFast (i);
      lightsDirty = true;
      return;
    }
  }
}

//-------------------------------------------------------------------------

void GenmeshAnimationPDLFactory::Report (iSyntaxService* synsrv, 
                                         int severity, iDocumentNode* node, 
                                         const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  csString text;
  text.FormatV (msg, arg);
  synsrv->Report ("crystalspace.mesh.anim.pdlight", severity, node, "%s", 
    (const char*)text);

  va_end (arg);
}

void GenmeshAnimationPDLFactory::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (type->object_reg, severity, "crystalspace.mesh.anim.pdlight", 
    msg, arg);
  va_end (arg);
}

bool GenmeshAnimationPDLFactory::HexToLightID (char* lightID, 
                                               const char* lightIDHex)
{
  bool valid = strlen (lightIDHex) == 32;
  if (valid)
  {
    for (size_t i = 0; i < 16; i++)
    {
      uint8 v;
      char digit16 = lightIDHex[i*2];
      char digit1 = lightIDHex[i*2+1];

      if ((digit16 >= '0') && (digit16 <= '9'))
        v = 16*(digit16-'0');
      else if ((digit16 >= 'a') && (digit16 <= 'f'))
        v = 16*(digit16-'a'+10);
      else if ((digit16 >= 'A') && (digit16 <= 'F'))
        v = 16*(digit16-'A'+10);
      else
      {
        valid = false; 
        break;
      }

      if ((digit1 >= '0') && (digit1 <= '9'))
        v += (digit1-'0');
      else if ((digit1 >= 'a') && (digit1 <= 'f'))
        v += (digit1-'a'+10);
      else if ((digit1 >= 'A') && (digit1 <= 'F'))
        v += (digit1-'A'+10);
      else
      {
        valid = false; 
        break;
      }
      lightID[i] = char (v);
    }
  }
  return valid;
}

const char* GenmeshAnimationPDLFactory::ParseLight (iSyntaxService* synsrv,
                                                      iDocumentNode* node)
{
  const char* lightId = node->GetAttributeValue ("lightid");
  if (!lightId)
    return "No light ID given";

  MappedLight light;
  light.colors = synsrv->ParseRenderBuffer (node);
  if (!light.colors)
  {
    return "Could not parse light colors";
  }

  light.lightId = new char[16];
  if (!HexToLightID (light.lightId, lightId))
  {
    parseError.Format ("Invalid light ID '%s'", lightId);
    return parseError;
  }
  else
  {
    lights.Push (light);
  }
  return 0;
}

const char* GenmeshAnimationPDLFactory::ValidateBufferSizes()
{
  size_t staticNum = staticColors->GetElementCount();

  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (lights[i].colors->GetElementCount() != staticNum)
      return "Not all light colors sizes match the static colors size";
  }

  return 0;
}

GenmeshAnimationPDLFactory::GenmeshAnimationPDLFactory (
  GenmeshAnimationPDLType* type) :
  scfImplementationType(this), type (type)
{
  InitTokenTable (xmltokens);
}

GenmeshAnimationPDLFactory::~GenmeshAnimationPDLFactory ()
{
}

csPtr<iGenMeshAnimationControl> GenmeshAnimationPDLFactory::
	CreateAnimationControl (iMeshObject* /*mesh*/)
{
  GenmeshAnimationPDL* ctrl = new GenmeshAnimationPDL (this);
  return csPtr<iGenMeshAnimationControl> (ctrl);
}

const char* GenmeshAnimationPDLFactory::Load (iDocumentNode* node)
{
  csRef<iSyntaxService> synsrv = 
    csQueryRegistry<iSyntaxService> (type->object_reg);
  if (!synsrv) return "No iSyntaxService";

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_STATICCOLORS:
        {
          staticColors = synsrv->ParseRenderBuffer (child);
          if (!staticColors)
            return "Could not parse <staticcolors>";
	}
        break;
      case XMLTOKEN_LIGHT:
        {
	  const char* err = ParseLight (synsrv, child);
	  if (err)
          {
            Report (synsrv, CS_REPORTER_SEVERITY_WARNING,
              child, "Could not parse light: %s", err);
          }
	}
        break;
      default:
        parseError.Format ("Unknown token '%s'", value);
        return parseError;
    }
  }

  if (!staticColors)
    return "No <staticcolors> given";

  const char* err = ValidateBufferSizes();
  if (err) return err;

  return 0;
}

const char* GenmeshAnimationPDLFactory::Save (iDocumentNode* parent)
{
  csRef<iFactory> plugin = scfQueryInterface<iFactory> (type);
  if (!plugin) return "Couldn't get Class ID";
  parent->SetAttribute("plugin", plugin->QueryClassID());
  return "Not implemented yet!";
}

//-------------------------------------------------------------------------

GenmeshAnimationPDLType::GenmeshAnimationPDLType (
  iBase* pParent) :
  scfImplementationType(this, pParent), object_reg(0)
{
}

GenmeshAnimationPDLType::~GenmeshAnimationPDLType ()
{
}

bool GenmeshAnimationPDLType::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}

csPtr<iGenMeshAnimationControlFactory> GenmeshAnimationPDLType::
	CreateAnimationControlFactory ()
{
  GenmeshAnimationPDLFactory* ctrl = new GenmeshAnimationPDLFactory (this);
  return csPtr<iGenMeshAnimationControlFactory> (ctrl);
}

}
CS_PLUGIN_NAMESPACE_END(GMeshAnimPDL)
