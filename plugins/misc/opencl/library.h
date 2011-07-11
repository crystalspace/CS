#ifndef __CS_OPENCL_LIBRARY_IMPL_H__
#define __CS_OPENCL_LIBRARY_IMPL_H__

#include <iutil/stringarray.h>
#include <ivaria/clprogram.h>
#include <ivaria/clconsts.h>
#include <csutil/hash.h>
#include <csutil/scf_implementation.h>

#include "kernel.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Manager;
  class Context;
  class Event;

  class Library : public scfImplementation1<Library,CS::CL::iLibrary>
  {
  public:
    Library(Manager* parent, iStringArray* source);
    ~Library();

    void Precache();
    csRef<Event> GetHandle(Context*, cl_program&);
    csPtr<CS::CL::iKernel> CreateKernel(const char*);

    const iStringArray* GetSource() const
    {
      return source;
    }

  private:
    csRef<Manager> parent;
    csRef<iStringArray> source;
    csHash<cl_program, csRef<Context> > handles;
    csHash<csRef<Event>, cl_program > buildEvents;
    csHash<KernelMetaData, csString> kernelData;

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

#endif // __CS_OPENCL_LIBRARY_IMPL_H__
