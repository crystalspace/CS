#include "cssysdef.h"

#include "iengine/mesh.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"

#include "csvosa3dl.h"
#include "voscube.h"
#include "vosmaterial.h"

using namespace VOS;

class ConstructCubeTask : public Task
{
public:
    csRef<iObjectRegistry> object_reg;
    vRef<csMetaMaterial> metamat;
    vRef<csMetaCube> cube;
    std::string name;
    csRef<iSector> sector;

    ConstructCubeTask(csRef<iObjectRegistry> objreg, vRef<csMetaMaterial> mat,
                      csMetaCube* c, std::string n, csRef<iSector> s);
    virtual ~ConstructCubeTask();
    virtual void doTask();
};

ConstructCubeTask::ConstructCubeTask(csRef<iObjectRegistry> objreg, vRef<csMetaMaterial> mat,
                                     csMetaCube* c, std::string n, csRef<iSector> s)
    : object_reg(objreg), metamat(mat), cube(c, true), name(n), sector(s)
{
}

ConstructCubeTask::~ConstructCubeTask()
{
}

void ConstructCubeTask::doTask()
{
    csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

    //if(! cube_factory) {
    csRef<iMeshFactoryWrapper> cube_factory = engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh", "cube_factory");
    //}

    csRef<iGeneralFactoryState> cubeLook = SCF_QUERY_INTERFACE(cube_factory->GetMeshObjectFactory(),
                                                               iGeneralFactoryState);
    if(cubeLook) {
        cubeLook->SetMaterialWrapper(metamat->getMaterialWrapper());
        cubeLook->GenerateBox(csBox3(-.5, -.5, -.5, .5, .5, .5));

        csRef<iMeshWrapper> meshwrapper = engine->CreateMeshWrapper (cube_factory, name.c_str(),
                                                 sector, csVector3(0, 0, 0));
        cube->getCSinterface()->SetMeshWrapper(meshwrapper);
    }
}

/// csMetaCube ///

csMetaCube::csMetaCube(VobjectBase* superobject)
    : A3DL::Object3D(superobject),
      csMetaObject3D(superobject),
      A3DL::Cube(superobject)
{
}

MetaObject* csMetaCube::new_csMetaCube(VobjectBase* superobject, const std::string& type)
{
    return new csMetaCube(superobject);
}

void csMetaCube::setup(csVosA3DL* vosa3dl, csVosSector* sect)
{
    vRef<A3DL::Material> m = getMaterial();
    for(TypeSetIterator ti = m->getTypes(); ti.hasMore(); ti++) {
        LOG("voscube", 2, "type " << (*ti));
    }
    vRef<csMetaMaterial> mat = meta_cast<csMetaMaterial>(getMaterial());
    LOG("csMetaCube", 2, "getting material " << mat.isValid());
    mat->setup(vosa3dl);
    LOG("csMetaCube", 2, "setting up cube");
    vosa3dl->mainThreadTasks.push(new ConstructCubeTask(vosa3dl->GetObjectRegistry(), mat,
                                                        this, getURLstr(), sect->GetSector()));

    LOG("csMetaCube", 2, "calling csMetaObject3D::setup");
    csMetaObject3D::setup(vosa3dl, sect);
}

