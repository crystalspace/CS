/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csutil/sysfunc.h"
#include "ivaria/reporter.h"

#include "csgfx/shaderexpaccessor.h"

SCF_IMPLEMENT_IBASE(csShaderExpressionAccessor)
  SCF_IMPLEMENTS_INTERFACE(iShaderVariableAccessor)
SCF_IMPLEMENT_IBASE_END

csShaderExpressionAccessor::csShaderExpressionAccessor (
  iObjectRegistry* object_reg, csShaderExpression* expression) : 
  object_reg (object_reg), expression (expression)
{
  SCF_CONSTRUCT_IBASE(0);
}

csShaderExpressionAccessor::~csShaderExpressionAccessor()
{
  delete expression;
  SCF_DESTRUCT_IBASE();
}

void csShaderExpressionAccessor::PreGetValue (csShaderVariable *variable)
{
  if (expression)
  {
    if (!expression->Evaluate (variable))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	"crystalspace.shader.expressionaccessor",
	"eval error: %s", expression->GetError());
      // Prevent flooding with errors...
      delete expression; expression = 0;
    }
  }
}
