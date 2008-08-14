/*
Copyright (C) 2002 by Marten Svanfeldt
                      Anders Stenberg

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

#include "csgeom/vector3.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "softshader_vp.h"

CS_PLUGIN_NAMESPACE_BEGIN(SoftShader)
{

void csSoftShader_VP::Activate()
{
}

void csSoftShader_VP::Deactivate()
{
}

bool csSoftShader_VP::Load (iShaderDestinationResolver*, 
			    iDocumentNode* program)
{
  if(!program)
    return false;

#if 0
  csRef<iDocumentNode> variablesnode = program->GetNode("softvp");
  if(variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while(it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      (void)id;
      // @@@ FIXME: Implement me.
    }
  }
#endif

  return true;
}

bool csSoftShader_VP::Compile()
{
  // @@@ FIXME: Implement me.
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(SoftShader)
