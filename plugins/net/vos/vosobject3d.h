#ifndef _CSVOSOBJECT3D_H_
#define _CSVOSOBJECT3D_H_

#include <vos/metaobjects/a3dl/object3d.hh>

#include "inetwork/vosa3dl.h"
#include "iutil/objreg.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iengine/engine.h"
#include "iengine/sector.h"

#include "csvosa3dl.h"

class csVosObject3D : public iVosObject3D
{
private:
    csRef<iMeshWrapper> meshwrapper;

public:
    SCF_DECLARE_IBASE;

    virtual ~csVosObject3D();

    virtual csRef<iMeshWrapper> GetMeshWrapper();

    void SetMeshWrapper(iMeshWrapper* mw);
};

class csMetaObject3D : public virtual A3DL::Object3D
{
protected:
    csVosObject3D* csvobj3d;
public:
    csMetaObject3D(VOS::VobjectBase* superobject);

    static VOS::MetaObject* new_csMetaObject3D(VOS::VobjectBase* superobject, const std::string& type);

    virtual void setup(csVosA3DL* vosa3dl);
    csRef<iVosObject3D> getCSinterface();
};

#endif
