#ifndef _VOSCUBE_H_
#define _VOSCUBE_H_

#include <vos/metaobjects/a3dl/cube.hh>
#include "inetwork/vosa3dl.h"
#include "csvosa3dl.h"
#include "vosobject3d.h"

class csMetaCube : public virtual csMetaObject3D, public A3DL::Cube
{
public:
    csMetaCube(VOS::VobjectBase* superobject);

    static VOS::MetaObject* new_csMetaCube(VOS::VobjectBase* superobject, const std::string& type);

    virtual void setup(csVosA3DL* vosa3dl);
};

#endif
