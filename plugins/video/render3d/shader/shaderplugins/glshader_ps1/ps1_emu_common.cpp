/*
Copyright (C) 2002 by John Harger

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

#include "csutil/databuf.h"
#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/csmd5.h"
#include "csgeom/vector3.h"
#include "csutil/xmltiny.h"

#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"

#include "glshader_ps1.h"
#include "ps1_emu_common.h"

void csShaderGLPS1_Common::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (shaderPlug->object_reg, severity, 
    "crystalspace.graphics3d.shader.glps1", msg, args);
  va_end (args);
}

bool csShaderGLPS1_Common::Load (iShaderTUResolver*, iDocumentNode* program)
{
  if(!program)
    return false;

  csRef<iDocumentNode> variablesnode = program->GetNode("ps1fp");
  if (variablesnode)
  {
    csRef<iDocumentNodeIterator> it = variablesnode->GetNodes ();
    while (it->HasNext())
    {
      csRef<iDocumentNode> child = it->Next();
      if(child->GetType() != CS_NODE_ELEMENT) continue;
      if (!ParseCommon (child))
	return false;
    }
  }

  return true;
}

bool csShaderGLPS1_Common::Load (iShaderTUResolver*, const char* program, 
				 csArray<csShaderVarMapping> &mappings)
{
  programBuffer.AttachNew (new csDataBuffer (csStrNew (program),
    strlen (program)));

  for (size_t i = 0; i < mappings.Length(); i++)
  {
    variablemap.Push (VariableMapEntry (mappings[i]));
  }

  return true;
}


bool csShaderGLPS1_Common::Compile(
	csArray<iShaderVariableContext*> &staticContexts)
{
  ResolveStaticVars (staticContexts);
  
  for (size_t i = 0; i < variablemap.Length (); i++)
  {
    int dest;
    if ((sscanf (variablemap[i].destination, "register %d", &dest) != 1) &&
      (sscanf (variablemap[i].destination, "c%d", &dest) != 1))
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Unknown variable destination %s", 
	variablemap[i].destination.GetData());
      continue;
    }

    if ((dest < 0) || (dest >= MAX_CONST_REGS))
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Invalid constant register number %d, must be in range [0..%d]", 
	dest, MAX_CONST_REGS);
      continue;
    }

    constantRegs[dest].statlink = variablemap[i].statlink;
    constantRegs[dest].varID = variablemap[i].name;
  }

  variablemap.DeleteAll();

  return LoadProgramStringToGL();
}

