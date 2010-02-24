/*
    Copyright (C) 2006 by Dariusz Dawidowski

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
#include "trans.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"



CS_PLUGIN_NAMESPACE_BEGIN(TransStd)
{

SCF_IMPLEMENT_FACTORY (csTranslator)

csTranslator::csTranslator (iBase* parent) :
	scfImplementationType (this, parent), object_reg (0)
{
}

csTranslator::~csTranslator ()
{
}

bool csTranslator::Initialize (iObjectRegistry *object_reg)
{
  csTranslator::object_reg = object_reg;
  return true;
}

const char* csTranslator::GetMsg (const char* src) const
{
  const csString* dst = messages.GetElementPointer (src);
  if (dst)
    return *dst;
  else
    return src;
}

void csTranslator::SetMsg (const char* src, const char* dst)
{
  messages.Put (src, dst);
}

}
CS_PLUGIN_NAMESPACE_END(TransStd)
