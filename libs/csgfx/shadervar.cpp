/*
    Copyright (C) 2002-2003 by Mårten Svanfeldt
                  2002      by Anders Stenberg

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
#include "csgfx/shadervar.h"

static int ShaderVariableWrapperCompare (csShaderVariableProxy const &item1,
                                         csShaderVariableProxy const &item2)
{
  if (item1.Name < item2.Name) return -1;
  else if (item1.Name > item2.Name) return 1;
  else return 0;
}


csShaderVariable::csShaderVariable (csStringID name) :
  csRefCount(), TextureHandValue(0), TextureWrapValue(0), VectorValue(0), 
  Name (name) , accessor(0)
{
}

csShaderVariable& csShaderVariable::operator= (csShaderVariable& copyFrom)
{
  switch (copyFrom.Type)
  {
    case INT:
      {
        int val;
        copyFrom.GetValue (val); 
        SetValue (val);
      }
      break;
    case TEXTURE:
      {
	if (copyFrom.TextureWrapValue != 0)
        {
          iTextureWrapper *val;
          copyFrom.GetValue (val);  SetValue (val);
        }
	else
        {
          iTextureHandle* val;
	  copyFrom.GetValue (val);  SetValue (val);
        }
      }
      break;
    case RENDERBUFFER:
      {
        iRenderBuffer* val;
	copyFrom.GetValue (val);  SetValue (val);
      }
      break;
    case FLOAT:
    case COLOR:
    case VECTOR2:
    case VECTOR3:
    case VECTOR4:
      {
	csVector4 v; 
	copyFrom.GetValue (v); SetValue (v);
	Type = copyFrom.Type;
      }
      break;
  }
  return *this;
}

int csShaderVariableProxyList::InsertSorted (csShaderVariableProxy item)
{
  return csArray<csShaderVariableProxy>::InsertSorted(
    item, ShaderVariableWrapperCompare);
}

int csShaderVariableProxyList::Push (csShaderVariableProxy item)
{
  return csArray<csShaderVariableProxy>::InsertSorted(
    item, ShaderVariableWrapperCompare);
}

void csShaderVariableProxyList::PrepareFill ()
{
  csShaderVariableProxyList::Iterator it (GetIterator ());
  while(it.HasNext())
  {
    csShaderVariableProxy *cur = (csShaderVariableProxy*)&it.Next();
    cur->shaderVariable = 0;
    if (cur->realLocation) *cur->realLocation = 0;
  }
}
