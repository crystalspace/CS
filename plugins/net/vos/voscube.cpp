#include "cssysdef.h"
#include "csa3dl.h"
#include "voscube.h"

using namespace VOS;

class ConstructCubeTask : public Task
{
private:
    char* url;
public:
    ConstructCubeTask(const char* u);
    virtual ~ConstructCubeTask();
    virtual void doTask();
};

ConstructCubeTask::ConstructCubeTask(const char* u)
{
    url = strdup(u);
}

ConstructCubeTask::~ConstructCubeTask()
{
    free(url);
}

void ConstructCubeTask::doTask()
{
    std::cout << "Going to create a cube " << url << std::endl;
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

void csMetaCube::setup(csVosA3DL* vosa3dl)
{
    vosa3dl->mainThreadTasks.push(new ConstructCubeTask(getURLstr().c_str()));
}

