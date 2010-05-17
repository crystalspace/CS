/*
    Copyright (C) 2009 by Jorrit Tyberghein

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

#include "cstool/materialbuilder.h"

#include "csgeom/math3d.h"
#include "csgfx/imagememory.h"
#include "iutil/objreg.h"
#include "iutil/strset.h"
#include "ivideo/shader/shader.h"
#include "ivideo/material.h"
#include "imap/loader.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "csutil/cscolor.h"

namespace CS
{
namespace Material
{

//------------------------ ParallaxMaterial ----------------------

void MaterialBuilder::SetupParallaxMaterial (
    iObjectRegistry* object_reg,
    iMaterialWrapper* material,
    iTextureHandle* normalmap, iTextureHandle* heightmap,
    const csVector4& specular)
{
  iMaterial* mat = material->GetMaterial ();

  csRef<iLoader> ldr = csQueryRegistry<iLoader> (object_reg);
  csRef<iShaderVarStringSet> svStrings =
    csQueryRegistryTagInterface<iShaderVarStringSet> (object_reg,
      "crystalspace.shader.variablenameset");
  csRef<iStringSet> stringSet = csQueryRegistryTagInterface<iStringSet> (
      object_reg, "crystalspace.shared.stringset");

  csRef<iShaderManager> shaderMgr = csQueryRegistry<iShaderManager> (object_reg);
  iShader* parallaxShader = shaderMgr->GetShader ("parallaxAtt");
  if (!parallaxShader)
    ldr->LoadShader ("/shader/parallaxAtt/parallaxAtt.xml");
  parallaxShader = shaderMgr->GetShader ("parallaxAtt");

  iShader* ambientShader = shaderMgr->GetShader ("ambient");
  if (!ambientShader)
    ldr->LoadShader ("/shader/ambient.xml");
  ambientShader = shaderMgr->GetShader ("ambient");

  // Setup shaders.
  mat->SetShader (stringSet->Request ("diffuse"), parallaxShader);
  mat->SetShader (stringSet->Request ("ambient"), ambientShader);

  // Add a normal map to the material.
  normalmap->SetTextureClass ("normalmap");
  csShaderVariable* svNormalMap = mat->GetVariableAdd (svStrings->Request ("tex normal"));
  svNormalMap->SetValue (normalmap);

  // Heightmap
  heightmap->SetTextureClass ("normalmap");
  csShaderVariable* svShinyMap = mat->GetVariableAdd (svStrings->Request ("tex height"));
  svShinyMap->SetValue (heightmap);

  // Set a specular reflection color.
  csShaderVariable* svSpecColor = mat->GetVariableAdd (svStrings->Request ("specular"));
  svSpecColor->SetValue (specular);
}

iMaterialWrapper* MaterialBuilder::CreateParallaxMaterial (iObjectRegistry* object_reg,
      const char* matname, const char* matfile, const char* normalfile,
      const char* heightfile, const csVector4& specular)
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iLoader> ldr = csQueryRegistry<iLoader> (object_reg);
  if (!engine || !ldr) return 0;

  iMaterialWrapper* material = engine->GetMaterialList ()->FindByName (matname);
  if (!material)
  {
    if (!ldr->LoadTexture (matname, matfile)) return 0;
    material = engine->GetMaterialList ()->FindByName (matname);
  }
  csRef<iTextureHandle> normalmap = ldr->LoadTexture (normalfile);
  if (!normalmap) return 0;
  csRef<iTextureHandle> heightmap = ldr->LoadTexture (heightfile);
  if (!heightmap) return 0;

  SetupParallaxMaterial (object_reg, material, normalmap, heightmap, specular);
  return material;
}

//------------------------ ColorMaterial ----------------------

iMaterialWrapper* MaterialBuilder::CreateColorMaterial(iObjectRegistry* object_reg,
      const char* matname, csColor color)
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine) return 0;

  // Test for existing material with given name
  iMaterialWrapper* material = engine->GetMaterialList ()->FindByName (matname);
  if (material) return material;

  // Create texture & register material
  csRef<csImageMemory> colorImg;
  colorImg.AttachNew (new csImageMemory (1, 1));
  static_cast<csRGBpixel*> (colorImg->GetImagePtr())->Set (
    (int) (color.red * 255.0),
    (int) (color.green * 255.0),
    (int) (color.blue * 255.0),
    255);
  
  csRef<iTextureWrapper> colorTexWrapper =
    engine->GetTextureList()->CreateTexture (colorImg);
  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (object_reg);
  colorTexWrapper->Register (g3d->GetTextureManager ());
  return engine->CreateMaterial (matname, colorTexWrapper);
}

} // namespace Material
} // namespace CS

//---------------------------------------------------------------------------
