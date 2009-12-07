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
#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "csutil/util.h"
#include "csutil/event.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "imesh/object.h"
#include "iutil/document.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "imap/services.h"
#include "gmeshanimpdl.h"



CS_PLUGIN_NAMESPACE_BEGIN(GMeshAnimPDL)
{

SCF_IMPLEMENT_FACTORY (GenmeshAnimationPDLType)

void GenmeshAnimationPDL::PrepareBuffer (iEngine* engine,
  const GenmeshAnimationPDLFactory::ColorBuffer& factoryBuf,
  ColorBuffer& buffer)
{
  buffer.name = factoryBuf.name;
  buffer.staticColors = factoryBuf.staticColors;
  for (size_t i = 0; i < factoryBuf.lights.GetSize(); i++)
  {
    const GenmeshAnimationPDLFactory::ColorBuffer::MappedLight& factoryLight = 
      factoryBuf.lights[i];
    ColorBuffer::MappedLight newLight;
    newLight.colors = factoryLight.colors;
    if (!factoryLight.lightId->sectorName.IsEmpty()
      && !factoryLight.lightId->lightName.IsEmpty())
    {
      iSector* sector = engine->FindSector (factoryLight.lightId->sectorName);
      if (sector)
      {
        iLight* engLight = sector->GetLights()->FindByName (
          factoryLight.lightId->lightName);
        newLight.light = engLight;
        if (!newLight.light)
        {
          factory->type->Report (CS_REPORTER_SEVERITY_WARNING, 
            "Could not find light '%s' in sector '%s'", 
            factoryLight.lightId->lightName.GetData(),
            factoryLight.lightId->sectorName.GetData());
        }
      }
      else
      {
        factory->type->Report (CS_REPORTER_SEVERITY_WARNING, 
          "Could not find sector '%s' for light '%s'", 
          factoryLight.lightId->sectorName.GetData(),
          factoryLight.lightId->lightName.GetData());
      }
    }
    else
    {
      newLight.light = engine->FindLightID (
        (const char*)factoryLight.lightId->lightId);
      if (!newLight.light)
      {
        csString hexId;
        for (int i = 0; i < 16; i++)
          hexId.AppendFmt ("%02x", factoryLight.lightId->lightId[i]);
        factory->type->Report (CS_REPORTER_SEVERITY_WARNING, 
          "Could not find light with ID '%s'", hexId.GetData());
      }
    }
    if (newLight.light)
    {
      newLight.light->SetLightCallback (this);
      buffer.lights.Push (newLight);
    }
  }
  buffer.lights.ShrinkBestFit();
  buffer.lightsDirty = true;
}

void GenmeshAnimationPDL::Prepare ()
{
  if (prepared) return;
  prepared = true;

  csRef<iEngine> engine = csQueryRegistry<iEngine> (factory->type->object_reg);
  if (!engine)
  {
    factory->type->Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine!");
    return;
  }

  for (size_t b = 0; b < factory->buffers.GetSize(); b++)
  {
    if (factory->buffers[b].name == factory->type->colorsID)
      PrepareBuffer (engine, factory->buffers[b], colorsBuffer);
    else
    {
      ColorBuffer& buffer = buffers.GetExtend (buffers.GetSize ());
      PrepareBuffer (engine, factory->buffers[b], buffer);
    }
  }
  buffers.ShrinkBestFit ();
}
  
void GenmeshAnimationPDL::UpdateBuffer (ColorBuffer& buffer, csTicks current, 
                                        const csColor4* colors, int num_colors, 
                                        uint32 version_id)
{
  if (buffer.lightsDirty || (buffer.lastMeshVersion != version_id))
  {
    size_t numUpdate = csMin (buffer.staticColors->GetElementCount(), 
      (size_t)num_colors);

    csDirtyAccessArray<csColor4>& combinedColors = buffer.combinedColors;
    combinedColors.SetSize (num_colors);
    {
      csVertexListWalker<float, csColor> color (buffer.staticColors, 3);
      if (colors)
      {
        for (size_t n = 0; n < numUpdate; n++)
        {
          combinedColors[n].Set ((*color).red, (*color).green, (*color).blue, 
            colors[n].alpha);
          ++color;
        }
      }
      else
      {
        for (size_t n = 0; n < numUpdate; n++)
        {
          combinedColors[n].Set ((*color).red, (*color).green, (*color).blue, 
            1.0f);
          ++color;
        }
      }
    }

    for (size_t l = 0; l < buffer.lights.GetSize(); l++)
    {
      const ColorBuffer::MappedLight& light = buffer.lights[l];
      
      csVertexListWalker<float, csColor> color (light.colors, 3);
      csColor lightColor = light.light->GetColor();

      for (size_t n = 0; n < numUpdate; n++)
      {
        combinedColors[n] += *color * lightColor;
        ++color;
      }
    }

    colorsBuffer.lightsDirty = false;
    colorsBuffer.lastMeshVersion = version_id;
  }
}

GenmeshAnimationPDL::GenmeshAnimationPDL (
  GenmeshAnimationPDLFactory* fact, iGeneralMeshState* genmesh) : 
  scfImplementationType(this),  factory (fact), genmesh (genmesh),
  prepared (false)
{
}

GenmeshAnimationPDL::~GenmeshAnimationPDL ()
{
}

void GenmeshAnimationPDL::Update(csTicks current, int num_verts, 
                                 uint32 version_id)
{
  if (!prepared) Prepare();

  if (buffers.GetSize() == 0) return;

  // @@@ FIXME: Bit of a waste here to always update custom buffers...
  for (size_t b = 0; b < buffers.GetSize(); b++)
  {
    bool updateRB = (buffers[b].lastMeshVersion != version_id)
      || !buffers[b].rbuf.IsValid();

    UpdateBuffer (buffers[b], current, 0, num_verts, version_id);
    if (updateRB)
    {
      buffers[b].rbuf = csRenderBuffer::CreateRenderBuffer (
        buffers[b].combinedColors.GetSize(), CS_BUF_DYNAMIC, CS_BUFCOMP_FLOAT,
        4);

      const char* rbufname = factory->type->strings->Request (buffers[b].name);
      genmesh->RemoveRenderBuffer (rbufname);
      genmesh->AddRenderBuffer (rbufname, buffers[b].rbuf);
    }
    buffers[b].rbuf->SetData (buffers[b].combinedColors.GetArray());
  }
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

  if (colorsBuffer.name == 0) return colors;
  UpdateBuffer (colorsBuffer, current, colors, num_colors, version_id);
  return colorsBuffer.combinedColors.GetArray();
}

void GenmeshAnimationPDL::OnColorChange (iLight* light, const csColor& newcolor)
{
  colorsBuffer.lightsDirty = true; 
  for (size_t i = 0; i < buffers.GetSize(); i++) 
  { 
    buffers[i].lightsDirty = true; 
  } 
}

void GenmeshAnimationPDL::OnDestroy (iLight* light)
{
  for (size_t b = 0; b < buffers.GetSize(); b++) 
  { 
    csSafeCopyArray<ColorBuffer::MappedLight>& lights = buffers[b].lights; 
    for (size_t i = 0; i < lights.GetSize(); i++) 
    { 
      if (lights[i].light == light) 
      { 
	lights.DeleteIndexFast (i); 
	buffers[b].lightsDirty = true; 
	break;
      }
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

bool GenmeshAnimationPDLFactory::HexToLightID (uint8* lightID, 
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
      lightID[i] = v;
    }
  }
  return valid;
}

const char* GenmeshAnimationPDLFactory::ParseBuffer (iSyntaxService* synsrv,
                                                     ColorBuffer& buffer, 
                                                     iDocumentNode* node)
{
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
          buffer.staticColors = synsrv->ParseRenderBuffer (child);
          if (!buffer.staticColors)
            return "Could not parse <staticcolors>";
	}
        break;
      case XMLTOKEN_LIGHT:
        {
          ColorBuffer::MappedLight light;
	  const char* err = ParseLight (light, synsrv, child);
	  if (err)
          {
            Report (synsrv, CS_REPORTER_SEVERITY_WARNING,
              child, "Could not parse light: %s", err);
          }
          else
            buffer.lights.Push (light);
	}
        break;
      default:
        parseError.Format ("Unknown token '%s'", value);
        return parseError;
    }
  }

  if (!buffer.staticColors)
    return "No <staticcolors> given";

  const char* err = ValidateBufferSizes (buffer);
  if (err) return err;

  return 0;
}

const char* GenmeshAnimationPDLFactory::ParseLight (ColorBuffer::MappedLight& light,
                                                    iSyntaxService* synsrv,
                                                    iDocumentNode* node)
{
  const char* sector = node->GetAttributeValue ("lightsector");
  const char* lightName = node->GetAttributeValue ("lightname");
  bool hasSector = sector && *sector;
  bool hasLightName = lightName && *lightName;
  if ((hasSector || hasLightName) && (!hasSector || !hasLightName))
  {
    return "Both 'lightsector' and 'lightname' attributes need to be specified";
  }
  const char* lightId = node->GetAttributeValue ("lightid");
  bool hasLightID = lightId && *lightId;
  if (!hasSector && !hasLightName && !hasLightID)
  {
    return "'lightsector' and 'lightname' attributes or a 'lightid' attribute "
      "need to be specified";
  }

  light.colors = synsrv->ParseRenderBuffer (node);
  if (!light.colors)
  {
    return "Could not parse light colors";
  }

  light.lightId = new ColorBuffer::MappedLight::LightIdentity;
  if (hasSector && hasLightName)
  {
    light.lightId->sectorName = sector;
    light.lightId->lightName = lightName;
  }
  else
  {
    if (!HexToLightID (light.lightId->lightId, lightId))
    {
      parseError.Format ("Invalid light ID '%s'", lightId);
      return parseError;
    }
    return 0;
  }
  return 0;
}

const char* GenmeshAnimationPDLFactory::ValidateBufferSizes (const ColorBuffer& buffer)
{
  size_t staticNum = buffer.staticColors->GetElementCount();

  for (size_t i = 0; i < buffer.lights.GetSize(); i++)
  {
    if (buffer.lights[i].colors->GetElementCount() != staticNum)
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
	CreateAnimationControl (iMeshObject* mesh)
{
  csRef<iGeneralMeshState> genmesh = 
    scfQueryInterface<iGeneralMeshState> (mesh);
  if (!genmesh.IsValid()) return 0;
  GenmeshAnimationPDL* ctrl = new GenmeshAnimationPDL (this, genmesh);
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

    if (id == XMLTOKEN_BUFFER)
    {
      // "New" style: a number of <buffer> tags
      it = node->GetNodes ();
      while (it->HasNext ())
      {
        child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);

        if (id != XMLTOKEN_BUFFER)
        {
          parseError.Format ("Unknown token '%s'", value);
          return parseError;
        }

        const char* name = child->GetAttributeValue ("name");
        if (!name || !*name)
          return "No 'name' attribute";

        ColorBuffer buf;
        buf.name = type->strings->Request (name);
        const char* err = ParseBuffer (synsrv, buf, child);
        if (err != 0) return err;
        buffers.Push (buf);
      }

      buffers.ShrinkBestFit ();
    }
    else
    {
      // "Old" style: "color" buffer, at 'top level'
      ColorBuffer colors;
      colors.name = type->colorsID;
      const char* err = ParseBuffer (synsrv, colors, node);
      if (err != 0) return err;
      buffers.Push (colors);
      buffers.ShrinkBestFit ();
    }
    break;
  }
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

  strings = csQueryRegistryTagInterface<iStringSet> (object_reg,
    "crystalspace.shared.stringset");
  if (!strings.IsValid()) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No shared string set");
    return false;
  }
  colorsID = strings->Request ("color");

  return true;
}

csPtr<iGenMeshAnimationControlFactory> GenmeshAnimationPDLType::
	CreateAnimationControlFactory ()
{
  GenmeshAnimationPDLFactory* ctrl = new GenmeshAnimationPDLFactory (this);
  return csPtr<iGenMeshAnimationControlFactory> (ctrl);
}

void GenmeshAnimationPDLType::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, "crystalspace.mesh.anim.pdlight", 
    msg, arg);
  va_end (arg);
}

}
CS_PLUGIN_NAMESPACE_END(GMeshAnimPDL)
