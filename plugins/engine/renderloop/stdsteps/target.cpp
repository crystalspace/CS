/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg

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

#include "iutil/document.h"
#include "ivideo/rndbuf.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "target.h"

SCF_IMPLEMENT_FACTORY(csTargetRSType);
SCF_IMPLEMENT_FACTORY(csTargetRSLoader);

//---------------------------------------------------------------------------

csTargetRSType::csTargetRSType (iBase* p) : csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csTargetRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csTargetRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csTargetRSLoader::csTargetRSLoader (iBase* p) : csBaseRenderStepLoader (p)
{
  InitTokenTable (tokens);
}

bool csTargetRSLoader::Initialize (iObjectRegistry* object_reg)
{
  if (csBaseRenderStepLoader::Initialize (object_reg))
  {
    return rsp.Initialize (object_reg);
  }
  else
  {
    return false;
  }
}

csPtr<iBase> csTargetRSLoader::Parse (iDocumentNode* node, 
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csRef<iRenderStep> newstep;
  csTargetRenderStep* step = new csTargetRenderStep (object_reg);
  newstep.AttachNew (step);

  csRef<iRenderStepContainer> steps =
    SCF_QUERY_INTERFACE (step, iRenderStepContainer);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_TARGET:
        step->SetTarget (child->GetContentsValue ());
        break;
      case XMLTOKEN_STEPS:
	{
	  if (!rsp.ParseRenderSteps (steps, child))
	    return 0;
	}
	break;
      case XMLTOKEN_CREATETEXTURE:
	{
	  int width = child->GetAttributeValueAsInt ("width");
	  if (width <= 0)
	  {
	    synldr->Report (
	      "crystalspace.renderloop.step.rendertarget",
	      CS_REPORTER_SEVERITY_WARNING, child,
	      "Bogus width %d", width);
	    return 0;
	  }
	  int height = child->GetAttributeValueAsInt ("height");
	  if (height <= 0)
	  {
	    synldr->Report (
	      "crystalspace.renderloop.step.rendertarget",
	      CS_REPORTER_SEVERITY_WARNING, child,
	      "Bogus height %d", height);
	    return 0;
	  }
	  step->SetCreate (width, height);
	}
	break;
      case XMLTOKEN_PERSISTENT:
	{
	  bool p;
	  if (!synldr->ParseBool (child, p, true))
	  {
	    return 0;
	  }
	  step->SetPersistent (p);
	}
	break;
      default:
	if (synldr) synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (newstep);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csTargetRenderStepFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csTargetRenderStepFactory::csTargetRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  csTargetRenderStepFactory::object_reg = object_reg;
}

csTargetRenderStepFactory::~csTargetRenderStepFactory ()
{
  SCF_DESTRUCT_IBASE();
}

csPtr<iRenderStep> csTargetRenderStepFactory::Create ()
{
  return csPtr<iRenderStep> 
    (new csTargetRenderStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csTargetRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepContainer);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csTargetRenderStep::csTargetRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  doCreate = false;
  persistent = false;
}

csTargetRenderStep::~csTargetRenderStep ()
{
  SCF_DESTRUCT_IBASE();
}

void csTargetRenderStep::Perform (iRenderView* rview, iSector* sector,
  csShaderVarStack &stacks)
{
  iGraphics3D* g3d = rview->GetGraphics3D();

  csRef<iTextureWrapper> tex = 
    engine->GetTextureList ()->FindByName (target);
  csRef<iTextureHandle> oldcontext;
  if (!tex.IsValid() && doCreate && !target.IsEmpty())
  {
    tex = engine->CreateBlackTexture (target, newW, newH, 0, 
      CS_TEXTURE_3D);
    tex->Register (g3d->GetTextureManager ());
  }
  if (tex != 0)
  {
    g3d->SetRenderTarget (tex->GetTextureHandle (), persistent);
    oldcontext = engine->GetContext ();
    engine->SetContext (tex->GetTextureHandle ());
  }
  //g3d->BeginDraw (CSDRAW_3DGRAPHICS | CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER);
  for (size_t i = 0; i < steps.Length(); i++)
  {
    steps[i]->Perform (rview, sector, stacks);
  }
  
  if (tex != 0)
  {
    //g3d->FinishDraw ();
    engine->SetContext (oldcontext);
  }
}

size_t csTargetRenderStep::AddStep (iRenderStep* step)
{
  if (!step) return csArrayItemNotFound;
  return steps.Push (step);
}

size_t csTargetRenderStep::GetStepCount ()
{
  return steps.Length();
}


