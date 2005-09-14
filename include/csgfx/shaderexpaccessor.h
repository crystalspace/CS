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

/**\file
 * Shader variable accessor evaluating a shader expression.
 */
 
#ifndef __CS_CSGFX_SHADEREXPACCESSOR_H__
#define __CS_CSGFX_SHADEREXPACCESSOR_H__

#include "csextern.h"

#include "csgfx/shadervar.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"

struct iObjectRegistry;

class csShaderExpression;

/**\addtogroup gfx
 * @{ 
 */

/**
 * Shader variable accessor that evaluates a shader expression when the SV
 * value is queried.
 */
class CS_CRYSTALSPACE_EXPORT csShaderExpressionAccessor : 
  public scfImplementation1<csShaderExpressionAccessor, iShaderVariableAccessor>
{
private:
  iObjectRegistry* object_reg;
  csShaderExpression* expression;
  csWeakRef<iShaderManager> shaderMgr;
public:
  /**
   * Construct accessor. 
   * \remarks The accessor object will take ownership of \a expression. You
   *   should not further use \a expression (especially not delete it) after
   *   using it to construct an instance of this class.
   * \remarks \a object_reg can be 0.
   */
  csShaderExpressionAccessor (iObjectRegistry* object_reg,
    csShaderExpression* expression);
  virtual ~csShaderExpressionAccessor();

  virtual void PreGetValue (csShaderVariable *variable);
};
 
/** @} */

#endif // __CS_CSGFX_SHADEREXPACCESSOR_H__
