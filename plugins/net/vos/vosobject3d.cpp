#include "cssysdef.h"
#include "iengine/mesh.h"

#include "vosobject3d.h"

#include <vos/metaobjects/a3dl/a3dl.hh>

using namespace VOS;

SCF_IMPLEMENT_IBASE (csVosObject3D)
    SCF_IMPLEMENTS_INTERFACE (iVosObject3D)
SCF_IMPLEMENT_IBASE_END

/// csVosObject3D ///

csVosObject3D::~csVosObject3D()
{
}

csRef<iMeshWrapper> csVosObject3D::GetMeshWrapper()
{
    return meshwrapper;
}

void csVosObject3D::SetMeshWrapper(iMeshWrapper* mw)
{
    meshwrapper = mw;
}

/// csMetaObject3D ///

csMetaObject3D::csMetaObject3D(VobjectBase* superobject) : A3DL::Object3D(superobject)
{
    csvobj3d = new csVosObject3D();
}

MetaObject* csMetaObject3D::new_csMetaObject3D(VobjectBase* superobject, const std::string& type)
{
    return new csMetaObject3D(superobject);
}

void csMetaObject3D::setup(csVosA3DL* vosa3dl)
{
}

csRef<iVosObject3D> csMetaObject3D::getCSinterface()
{
    return csvobj3d;
}
