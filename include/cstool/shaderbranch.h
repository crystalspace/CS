/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSTOOL_SHADERBRANCH_H__
#define __CS_CSTOOL_SHADERBRANCH_H__

#include "csutil/symtable.h"
#include "ivideo/shader/shader.h"

/// @@@ Document me!
class csBaseShaderBranch : public iShaderBranch
{
  csSymbolTable symtab;
public:
  virtual ~csBaseShaderBranch ();

  virtual void AddChild (iShaderBranch *child);
  virtual void AddVariable (csShaderVariable* variable);
  virtual csShaderVariable* GetVariable (csStringID);
  virtual csSymbolTable* GetSymbolTable ();
  virtual csSymbolTable* GetSymbolTable (int index);
  virtual void SelectSymbolTable (int index);
};

/// @@@ Document me!
class csShaderBranch : public csBaseShaderBranch
{
public:
  SCF_DECLARE_IBASE;
  
  csShaderBranch();
};


#endif // __CS_CSTOOL_SHADERBRANCH_H__
