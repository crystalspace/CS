#include <cssysdef.h>
#include "decalmanager.h"
#include "decal.h"
#include "decaltemplate.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iutil/object.h>
#include <iengine/mesh.h>
#include <iengine/engine.h>
#include <csgfx/renderbuffer.h>
#include <iengine/movable.h>
#include <imesh/genmesh.h>
#include <iengine/material.h>
#include <imesh/object.h>
#include <cstool/collider.h>
#include <ivideo/material.h>
#include "csgfx/shadervarcontext.h"
#include "iutil/eventq.h"
#include "csutil/event.h"

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENT_FACTORY(csDecalManager)

csDecalManager::csDecalManager(iBase * parent)
              : scfImplementationType(this, parent), 
                objectReg(0)
{
}

csDecalManager::~csDecalManager()
{
  for (size_t a=0; a<decals.Length(); ++a)
    delete decals[a];

  if (objectReg)
  {
    csRef<iEventQueue> q(csQueryRegistry<iEventQueue> (objectReg));
    if (q)
      CS::RemoveWeakListener (q, weakEventHandler);
  }
}

bool csDecalManager::Initialize(iObjectRegistry * objectReg)
{
  this->objectReg = objectReg;
  vc = csQueryRegistry<iVirtualClock> (objectReg);

  CS_INITIALIZE_EVENT_SHORTCUTS (objectReg);
  csRef<iEventQueue> q(csQueryRegistry<iEventQueue> (objectReg));
  if (q)
    CS::RegisterWeakListener(q, this, PreProcess, weakEventHandler);
  return true;
}

bool csDecalManager::CreateDecal(csRef<iDecalTemplate> & decalTemplate, 
    iSector * sector, const csVector3 * pos, const csVector3 * up, 
    const csVector3 * normal, float width, float height)
{
  // compute the maximum distance the decal can reach
  float radius = sqrt(width*width + height*height) * 2.0f;

  csVector3 n = normal->Unit();

  // our generated up vector depends on the primary axis of the normal
  csVector3 normalAxis = normal->UnitAxisClamped(); 

  csVector3 right = n % *up;
  csVector3 correctUp = right % n;

  if (!engine)
  {
    engine = csQueryRegistry<iEngine>(objectReg);
    if (!engine)
    {
      csReport(objectReg, CS_REPORTER_SEVERITY_ERROR, 
	  "crystalspace.decal", "Couldn't query engine");
      return false;
    }
  }

  // get all meshes that could be affected by this decal
  csDecal * decal = 0;
  csVector3 relPos;
  csRef<iMeshWrapperIterator> it = engine->GetNearbyMeshes(sector, *pos, 
                                                           radius, true);
  if (!it->HasNext())
      return false;

  decal = new csDecal(objectReg, this);
  decal->Initialize(decalTemplate, n, *pos, correctUp, right, width, height);
  decals.Push(decal);
  while (it->HasNext())
  {
    iMeshWrapper* mesh = it->Next();
    csVector3 relPos = 
        mesh->GetMovable()->GetFullTransform().Other2This(*pos);

    decal->BeginMesh(mesh);
    mesh->GetMeshObject()->BuildDecal(&relPos, radius, 
            (iDecalBuilder*)decal);
    decal->EndMesh();
  }
  return true;
}

csRef<iDecalTemplate> csDecalManager::CreateDecalTemplate(
        iMaterialWrapper* pMaterial)
{
    csRef<iDecalTemplate> ret;

  if (!engine)
  {
    engine = csQueryRegistry<iEngine>(objectReg);
    if (!engine)
    {
      csReport(objectReg, CS_REPORTER_SEVERITY_ERROR, 
	  "crystalspace.decal", "Couldn't query engine");
      return false;
    }
  }

    ret.AttachNew(new csDecalTemplate);
    ret->SetMaterialWrapper(pMaterial);
    ret->SetRenderPriority(engine->GetAlphaRenderPriority());
    return ret;
}

void csDecalManager::DeleteDecal(const iDecal * decal)
{
  for (size_t a=0; a<decals.GetSize(); ++a)
  {
    if (decals[a] != decal)
      continue;

    delete decals[a];
    decals.DeleteIndexFast(a);
    return;
  }
}

size_t csDecalManager::GetDecalCount() const
{
  return decals.GetSize();
}

iDecal * csDecalManager::GetDecal(size_t idx) const
{
  return decals[idx];
}

bool csDecalManager::HandleEvent(iEvent & ev)
{
  if (ev.Name != PreProcess)
    return false;

  csTicks elapsed = vc->GetElapsedTicks();
  size_t a=0;

  while (a < decals.GetSize())
  {
    if (!decals[a]->Age(elapsed))
    {
      delete decals[a];
      decals.DeleteIndexFast(a);
    }
    else
      ++a;
  }
  return true;
}

