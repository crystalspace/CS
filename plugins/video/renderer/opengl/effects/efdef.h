/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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

#ifndef __CS_EFFECTDEFINITION_H__
#define __CS_EFFECTDEFINITION_H__

#include "csutil/scf.h"
#include "cstypes.h"
#include "csutil/strset.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "../ieffects/efvector4.h"
#include "../ieffects/eftech.h"

struct iEffectTechnique;

#define CS_EFVARIABLETYPE_UNDEFINED 0
#define CS_EFVARIABLETYPE_FLOAT 1
#define CS_EFVARIABLETYPE_VECTOR4 2

struct efvariable
{
public:
  csStringID id;
  char type;
  float float_value;
  csEffectVector4 vector_value;
  int point_to;

  efvariable(csStringID argid)
  {
    type = CS_EFVARIABLETYPE_UNDEFINED;
    id = argid;
    point_to = -1;
  }
  efvariable(csStringID argid, float value)
  {
    type = CS_EFVARIABLETYPE_FLOAT;
    id = argid;
    float_value = value;
    point_to = -1;
  }
  efvariable(csStringID argid, csEffectVector4 value)
  {
    type = CS_EFVARIABLETYPE_VECTOR4;
    id = argid;
    vector_value = value;
    point_to = -1;
  }
};

class csEffectDefinition : public iEffectDefinition
{
private:
  csRefArray<iEffectTechnique> techniques;
  char* techniquename;
  csArray<efvariable*> variables;

  int GetTopmostVariableID(int id);

public:

  SCF_DECLARE_IBASE;

  csEffectDefinition(): techniquename(0)
  {
    SCF_CONSTRUCT_IBASE(0);
  }
  virtual ~csEffectDefinition ()
  { 
    delete[] techniquename;
    SCF_DESTRUCT_IBASE();
  }

  csPtr<iEffectTechnique> CreateTechnique();
  int GetTechniqueCount();
  iEffectTechnique* GetTechnique (int technique);

  void SetName(const char* name);
  const char* GetName();

  float GetVariableFloat(int variableID);
  csEffectVector4 GetVariableVector4(int variableID);
  char GetVariableType(int variableID);

  void SetVariableFloat(int variableID, float value);
  void SetVariableVector4(int variableID, csEffectVector4 value);

  int GetVariableID(csStringID string, bool create = true);
  //@@@
  //csArray<efvariable*>& GetAllVariableNames();
};

#endif // __CS_EFFECTDEFINITION_H__
