#include "cssysdef.h"
#include "csvosa3dl.h"
#include "vossector.h"
#include "vosobject3d.h"
#include "voscube.h"

#include <vos/metaobjects/a3dl/a3dl.hh>

using namespace VOS;
using namespace A3DL;

SCF_IMPLEMENT_IBASE (csVosA3DL)
    SCF_IMPLEMENTS_INTERFACE (iVosA3DL)
    SCF_IMPLEMENTS_INTERFACE (iComponent)
    SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csVosA3DL)

/// csVosA3DL ///

csVosA3DL::csVosA3DL (iBase *parent)
{
    SCF_CONSTRUCT_IBASE (parent);
}

csVosA3DL::~csVosA3DL()
{
    SCF_DESTRUCT_IBASE();
}

csRef<iVosSector> csVosA3DL::GetSector(const char* s)
{
    return new csVosSector(objreg, this, s);
}

bool csVosA3DL::Initialize (iObjectRegistry *o)
{
    Site::removeRemoteMetaObjectFactory("a3dl:object3D", &A3DL::Object3D::new_Object3D);
    Site::removeRemoteMetaObjectFactory("a3dl:object3D.cube", &A3DL::Cube::new_Cube);
    Site::addRemoteMetaObjectFactory("a3dl:object3D", "a3dl:object3D", &csMetaObject3D::new_csMetaObject3D);
    Site::addRemoteMetaObjectFactory("a3dl:object3D.cube", "a3dl:object3D.cube", &csMetaCube::new_csMetaCube);

    objreg = o;
    eventq = CS_QUERY_REGISTRY (objreg, iEventQueue);
    if (! eventq) return false;
    eventq->RegisterListener (this, CSMASK_Nothing);

    localsite.assign(new Site(true), false);
    localsite->addSiteExtension(new LocalSocketSiteExtension());

    return true;
}

bool csVosA3DL::HandleEvent (iEvent &ev)
{
    if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess){
        while(! mainThreadTasks.empty()) {
            Task* t = mainThreadTasks.pop();
            t->doTask();
            delete t;
        }
    }
    return false;
}


