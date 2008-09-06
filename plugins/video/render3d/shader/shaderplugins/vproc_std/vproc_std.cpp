/*
  Copyright (C) 2005 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"

#include "csgfx/vertexlight.h"
#include "vproc_std.h"
#include "vproc_program.h"




CS_PLUGIN_NAMESPACE_BEGIN(VProc_std)
{

CS_LEAKGUARD_IMPLEMENT (csVProc_Std);

SCF_IMPLEMENT_FACTORY (csVProc_Std)


csVProc_Std::csVProc_Std(iBase* parent) : scfImplementationType (this, parent)
{
  isOpen = false;
  memset (&lightCalculatorMatrix, 0, sizeof(iVertexLightCalculator*)*16);
}

csVProc_Std::~csVProc_Std()
{
  for (uint i = 0; i < 16; i++)
    delete lightCalculatorMatrix[i];
}

bool csVProc_Std::Initialize (iObjectRegistry *reg)
{
  objreg = reg;

  shaderManager = csQueryRegistry<iShaderManager> (objreg);
  csRef<iShaderVarStringSet> strings = 
    csQueryRegistryTagInterface<iShaderVarStringSet> 
    (objreg, "crystalspace.shader.variablenameset");
  string_object2world = strings->Request ("object2world transform");
  string_world2camera = strings->Request ("world2camera transform");

  //setup the matrix of light calculators
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_POINTLIGHT, CS_ATTN_NONE)] =
    new csVertexLightCalculator<csPointLightProc<csNoAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_POINTLIGHT, CS_ATTN_LINEAR)] =
    new csVertexLightCalculator<csPointLightProc<csLinearAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_POINTLIGHT, CS_ATTN_INVERSE)] =
    new csVertexLightCalculator<csPointLightProc<csInverseAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_POINTLIGHT, CS_ATTN_REALISTIC)] =
    new csVertexLightCalculator<csPointLightProc<csRealisticAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_POINTLIGHT, CS_ATTN_CLQ)] =
    new csVertexLightCalculator<csPointLightProc<csCLQAttenuation> > ();

  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_DIRECTIONAL, CS_ATTN_NONE)] =
    new csVertexLightCalculator<csDirectionalLightProc<csNoAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_DIRECTIONAL, CS_ATTN_LINEAR)] =
    new csVertexLightCalculator<csDirectionalLightProc<csLinearAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_DIRECTIONAL, CS_ATTN_INVERSE)] =
    new csVertexLightCalculator<csDirectionalLightProc<csInverseAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_DIRECTIONAL, CS_ATTN_REALISTIC)] =
    new csVertexLightCalculator<csDirectionalLightProc<csRealisticAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_DIRECTIONAL, CS_ATTN_CLQ)] =
    new csVertexLightCalculator<csDirectionalLightProc<csCLQAttenuation> > ();

  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_SPOTLIGHT, CS_ATTN_NONE)] =
    new csVertexLightCalculator<csSpotLightProc<csNoAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_SPOTLIGHT, CS_ATTN_LINEAR)] =
    new csVertexLightCalculator<csSpotLightProc<csLinearAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_SPOTLIGHT, CS_ATTN_INVERSE)] =
    new csVertexLightCalculator<csSpotLightProc<csInverseAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_SPOTLIGHT, CS_ATTN_REALISTIC)] =
    new csVertexLightCalculator<csSpotLightProc<csRealisticAttenuation> > ();
  lightCalculatorMatrix[GetMatrixIndex (CS_LIGHT_SPOTLIGHT, CS_ATTN_CLQ)] =
    new csVertexLightCalculator<csSpotLightProc<csCLQAttenuation> > ();

  lsvCache.SetStrings (strings);

  return true;
}

void csVProc_Std::Open ()
{
  if (isOpen) return;
  isOpen = true;
}

bool csVProc_Std::SupportType (const char *type)
{
  if (strcasecmp(type, "vproc") == 0) return true;

  return false;
}

csPtr<iShaderProgram> csVProc_Std::CreateProgram (const char *type)
{
  if (strcasecmp(type, "vproc") == 0)
    return csPtr<iShaderProgram> (new csVProcStandardProgram (this));
  return 0;
}

iVertexLightCalculator* csVProc_Std::GetLightCalculator (
  const csLightProperties& light, bool useAttenuation)
{
  return lightCalculatorMatrix[GetMatrixIndex 
    (light.type, (useAttenuation ? light.attenuationMode : CS_ATTN_NONE))];
}

}
CS_PLUGIN_NAMESPACE_END(VProc_std)
