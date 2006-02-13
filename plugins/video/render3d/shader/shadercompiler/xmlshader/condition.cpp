/*
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

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

#include "condition.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

bool csConditionConstants::AddConstant (const char* name, float value)
{
  if (constants.Contains (name)) return false;

  CondOperand op;
  op.type = operandFloat;
  op.floatVal = value;

  constants.Put (name, op);
  return true;
}

bool csConditionConstants::AddConstant (const char* name, int value)
{
  if (constants.Contains (name)) return false;

  CondOperand op;
  op.type = operandInt;
  op.intVal = value;

  constants.Put (name, op);
  return true;
}

bool csConditionConstants::AddConstant (const char* name, bool value)
{
  if (constants.Contains (name)) return false;

  CondOperand op;
  op.type = operandBoolean;
  op.boolVal = value;

  constants.Put (name, op);
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
