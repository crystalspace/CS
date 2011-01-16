/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#ifndef __CELLRENDERPROPERTIES_H__
#define __CELLRENDERPROPERTIES_H__

#include "iengine/engine.h"
#include "imesh/terrain2.h"
#include "csgfx/shadervarcontext.h"
#include "csutil/scf_implementation.h"
#include "csutil/stringconv.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

//-- Per cell properties class
class TerrainBBCellRenderProperties :
  public scfImplementation2<TerrainBBCellRenderProperties,
                            iTerrainCellRenderProperties,
                            scfFakeInterface<iShaderVariableContext> >,
  public CS::Graphics::ShaderVariableContextImpl
{
public:
  TerrainBBCellRenderProperties (iEngine* engine)
    : scfImplementationType (this), visible (true), blockResolution (32), 
    minSteps (1), splitDistanceCoeff (128), splatDistance (100),
    engine (engine)
  {
  }

  TerrainBBCellRenderProperties (const TerrainBBCellRenderProperties& other)
    : scfImplementationType (this), 
    CS::Graphics::ShaderVariableContextImpl (other), visible (other.visible), 
    blockResolution (other.blockResolution), minSteps (other.minSteps), 
    splitDistanceCoeff (other.splitDistanceCoeff), splatDistance (other.splatDistance),
    splatPrio (other.splatPrio), engine (other.engine)
  {

  }

  virtual bool GetVisible () const
  {
    return visible;
  }

  virtual void SetVisible (bool value)
  {
    visible = value;
  }

  size_t GetBlockResolution () const 
  {
    return blockResolution;
  }
  void SetBlockResolution (int value)
  {
    blockResolution = ptrdiff_t(1) << csLog2 (value);
  }

  size_t GetMinSteps () const
  {
    return minSteps;
  }
  void SetMinSteps (int value)
  {
    minSteps = value > 0 ? value : 1;
  }

  float GetLODSplitCoeff () const 
  {
    return splitDistanceCoeff;
  }
  void SetLODSplitCoeff (float value) 
  {
    splitDistanceCoeff = value;
  }

  float GetSplatDistance () const 
  {
    return splatDistance;
  }
  void SetSplatDistance (float value) 
  {
    splatDistance = value;
  }

  void SetSplatRenderPriority (const char* prio)
  {
    splatPrio = engine->GetRenderPriority (prio);
  }
  CS::Graphics::RenderPriority GetSplatRenderPriorityValue() const
  {
    return splatPrio;
  }


  virtual void SetParameter (const char* name, const char* value)
  {
    if (strcmp (name, "visible") == 0)
      SetVisible (strcmp(value, "true") == 0);
    else if (strcmp (name, "block resolution") == 0)
      SetBlockResolution (atoi (value));
    else if (strcmp (name, "min steps") == 0)
      SetMinSteps (atoi (value));
    else if (strcmp (name, "lod splitcoeff") == 0)
      SetLODSplitCoeff (CS::Utility::strtof (value));
    else if (strcmp (name, "splat distance") == 0)
      SetSplatDistance (CS::Utility::strtof (value));
    else if (strcmp (name, "splat render priority") == 0)
      SetSplatRenderPriority (value);

  }

  virtual size_t GetParameterCount() { return 6; }

  virtual const char* GetParameterName (size_t index)
  {
    switch (index)
    {
      case 0: return "visible";
      case 1: return "block resolution";
      case 2: return "min steps";
      case 3: return "lod splitcoeff";
      case 4: return "splat distance";
      case 5: return "splat render priority";
      default: return 0;
    }
  }

  virtual const char* GetParameterValue (size_t index)
  {
    return GetParameterValue (GetParameterName (index));
  }
  virtual const char* GetParameterValue (const char* name)
  {
    // @@@ Not nice
    static char scratch[32];
    if (strcmp (name, "visible") == 0)
      return visible ? "true" : "false";
    else if (strcmp (name, "block resolution") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%u", (uint)blockResolution);
      return scratch;
    }
    else if (strcmp (name, "min steps") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%u", (uint)minSteps);
      return scratch;
    }
    else if (strcmp (name, "lod splitcoeff") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%f", splitDistanceCoeff);
      return scratch;
    }
    else if (strcmp (name, "splat distance") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%f", splatDistance);
      return scratch;
    }
    else if (strcmp (name, "splat render priority") == 0)
    {
      if (!splatPrio.IsValid())
	return 0;
      else
	return engine->GetRenderPriorityName (splatPrio);
    }
    else
      return 0;
  }

  virtual csPtr<iTerrainCellRenderProperties> Clone ()
  {
    return csPtr<iTerrainCellRenderProperties> (
      new TerrainBBCellRenderProperties (*this));
  }

private:
  // Per cell properties
  bool visible;

  // Block resolution in "gaps".. should be 2^n
  size_t blockResolution;

  // Grid steps for lowest tessellation setting
  size_t minSteps;

  // Lod splitting coefficient
  float splitDistanceCoeff;

  // Splatting end distance
  float splatDistance;
  
  // Splat render priority
  CS::Graphics::RenderPriority splatPrio;
  
  // Engine (needed for render prio...)
  iEngine* engine;
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CELLRENDERPROPERTIES_H__
