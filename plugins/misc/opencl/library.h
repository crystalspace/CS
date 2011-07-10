#ifndef __CS_OPENCL_LIBRARY_IMPL_H__
#define __CS_OPENCL_LIBRARY_IMPL_H__

#include <csutil/hash.h>
#include <iutil/stringarray.h>
#include <ivaria/clprogram.h>
#include <ivaria/clconsts.h>
#include "context.h"
#include "kernel.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Library : public scfImplementation1<Library,iLibrary>
  {
  public:
    Library(Manager* parent, iStringArray* source);
    ~Library();

    void Precache();
    csRef<Event> GetHandle(Context*, cl_program&);
    csPtr<iKernel> CreateKernel(const char*);

    const iStringArray* GetSource() const
    {
      return source;
    }

  private:
    csRef<Manager> parent;
    csRef<iStringArray> source;
    csHash<csRef<Context>, cl_program> handles;
    csHash<cl_program, csRef<Event> > buildEvents;
    csHash<csString, KernelMetaData> kernelData;

    // callback for clBuildProgram
    static void BuildHandler(cl_program, void*);

    // helper to retrieve the device list for a
    // given program - returns nullptr and sets
    // count to 0 on error
    static cl_device_id* GetDevices(cl_program, cl_uint& count);

    // helper to print the build log of a program
    // for the given device
    static void PrintLog(cl_program, cl_device_id);
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif __CS_OPENCL_LIBRARY_IMPL_H__
