#ifndef _VOSCUBE_H_
#define _VOSCUBE_H_

#include <vos/metaobjects/a3dl/cube.hh>

#include "inetwork/vosa3dl.h"
#include "iutil/objreg.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iengine/engine.h"
#include "iengine/sector.h"

#include "csa3dl.h"
#include "csvosobject3d.h"

class csMetaCube : public virtual csMetaObject3D, A3DL::Cube
{
public:
    csMetaCube(VOS::VobjectBase* superobject);

    static VOS::MetaObject* new_csMetaCube(VOS::VobjectBase* superobject, const std::string& type);

    virtual void setup(csVosA3DL* vosa3dl);
};

#endif
