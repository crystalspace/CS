#ifndef _CSA3DL_H_
#define _CSA3DL_H_

#include <vos/vos/site.hh>
#include <vos/vos/taskqueue.hh>

#include "inetwork/vosa3dl.h"
#include "iutil/objreg.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iengine/engine.h"
#include "iengine/sector.h"

class csVosA3DL : public iComponent, iEventHandler, iVosA3DL
{
private:
    csRef<iEventQueue> eventq;
    csRef<iObjectRegistry> objreg;

    VOS::vRef<VOS::Site> localsite;
public:
    SCF_DECLARE_IBASE;

    VOS::SynchronizedQueue<VOS::Task*> mainThreadTasks;

    csVosA3DL (iBase *);
    virtual ~csVosA3DL();

    virtual csRef<iVosSector> GetSector(const char* s);
    virtual bool Initialize (iObjectRegistry *objreg);
    virtual bool HandleEvent (iEvent &ev);
};

#endif
