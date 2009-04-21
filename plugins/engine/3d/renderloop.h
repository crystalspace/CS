/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg
              (C) 2004 by Marten Svanfeldt

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

#ifndef __CS_CSENGINE_RENDERLOOP_H__
#define __CS_CSENGINE_RENDERLOOP_H__

#include "csutil/hashr.h"
#include "csutil/refarr.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/csobject.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iutil/strset.h"
#include "iutil/selfdestruct.h"
#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

class csEngine;
class csRenderLoop;

class csRenderLoop : public scfImplementationExt3<csRenderLoop,
	csObject,
	iRenderLoop,
	scfFakeInterface<iRenderStepContainer>,
	iSelfDestruct>
{
protected:
  friend class csLightIteratorRenderStep;
  friend class csGenericRenderStep;

  csEngine* engine;

  csRefArray<iRenderStep> steps;
  csRef<iShaderManager> shadermanager;

protected:
  virtual void InternalRemove() { SelfDestruct(); }

public:

  csRenderLoop (csEngine* engine);
  virtual ~csRenderLoop();

  virtual void Draw (iRenderView *rview, iSector *s, iMeshWrapper* mesh = 0);

  virtual size_t AddStep (iRenderStep* step);
  virtual bool DeleteStep (iRenderStep* step);
  virtual iRenderStep* GetStep (size_t n) const;
  virtual size_t Find (iRenderStep* step) const;
  virtual size_t GetStepCount () const;

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
};

class csRenderLoopManager : public scfImplementation1<csRenderLoopManager,
                                                      iRenderLoopManager>
{
  typedef csHashReversible<csRef<iRenderLoop>, const char*> LoopsHash;
  LoopsHash loops;
  csStringSet strings;

  csEngine* engine;
public:

  csRenderLoopManager(csEngine* engine);
  virtual ~csRenderLoopManager();

  virtual csPtr<iRenderLoop> Create ();
  
  virtual bool Register (const char* name, iRenderLoop* loop, bool checkDupes = false);
  virtual iRenderLoop* Retrieve (const char* name);
  virtual const char* GetName (iRenderLoop* loop);
  virtual bool Unregister (iRenderLoop* loop);

  /**
   * Load a renderloop from VFS file named \p file.
   */
  virtual csPtr<iRenderLoop> Load (const char* fileName);
  
  void UnregisterAll (bool evenDefault);
};

#endif
