#ifndef __CS_OPENCL_PLATFORM_IMPL_H__
#define __CS_OPENCL_PLATFORM_IMPL_H__

#include <ivaria/clconsts.h>
#include <csutil/refarr.h>
#include <csutil/refcount.h>

#include "device.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Platform : public CS::Utility::FastRefCount<Platform>
  {
  public:
    Platform(cl_platform_id p) : platform(p)
    {
    }

    bool Initialize();

    csRefArray<Device> GetDevices()
    {
      return devices;
    }

    bool IsFullProfile() const
    {
      return fullProfile;
    }

    const csString& GetVersion() const
    {
      return version;
    }

    const csString& GetName() const
    {
      return name;
    }

    const csString& GetVendor() const
    {
      return vendor;
    }

    bool HasExtension(const char* ext) const
    {
      return extensions.Contains(ext);
    }

    cl_platform_id GetHandle() const
    {
      return platform;
    }

  private:
    csSet<csString> extensions;
    csRefArray<Device> devices;
    csString version;
    csString name;
    csString vendor;
    bool fullProfile;
    cl_platform_id platform;

    csString QueryInfo(cl_platform_info);
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif // __CS_OPENCL_PLATFORM_IMPL_H__
