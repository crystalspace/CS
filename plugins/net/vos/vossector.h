#ifndef _VOSSECTOR_H_
#define _VOSSECTOR_H_

#include "inetwork/vosa3dl.h"
#include "iutil/objreg.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iengine/engine.h"
#include "iengine/sector.h"

class csVosSector : public iVosSector
{
private:
    char* url;
    csRef<iObjectRegistry> objreg;
    csRef<iEngine> engine;
    csRef<iSector> sector;
    csVosA3DL* vosa3dl;

public:
    SCF_DECLARE_IBASE;

    csVosSector(csRef<iObjectRegistry> o, csVosA3DL* vosa3dl, const char* s);
    virtual ~csVosSector();

    virtual void Load();
    virtual csRef<iSector> GetSector();
};

#endif
