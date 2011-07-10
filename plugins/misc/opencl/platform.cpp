#include "platform.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  bool Platform::Initialize()
  {
    csString profile = QueryInfo(CL_PLATFORM_PROFILE);
    if(profile == "FULL_PROFILE")
    {
      fullProfile = true
    }
    else if(profile == "EMBEDDED_PROFILE")
    {
      fullProfile = false;
    }
    else
    {
      return false;
    }

    csString version = QueryInfo(CL_PLATFORM_VERSION);
    // "OpenCL <major>.<minor> <platform specific info>"
    csStringArray parts(version, ' ');
    if(parts[0] != "OpenCL")
    {
      return false;
    }
    version = parts[1];

    name = QueryInfo(CL_PLATFORM_NAME);
    vendor = QueryInfo(CL_PLATFORM_VENDOR);

    // parse extensions
    csString extString = QueryInfo(CL_PLATFORM_EXTENSION);
    csStringArray extArray(extString, ' ');
    for(size_t i = 0; i < extArray.GetSize(); ++i)
    {
      extensions.Push(extArray[i]);
    }

    // get size of the device list
    cl_int result;
    cl_uint size;
    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, nullptr, &size);

    // check for errors
    switch(result)
    {
    case CL_SUCCESS:
      // fine
      break;
    case CL_INVALID_PLATFORM:
      // we shouldn't have been constructed
    case CL_INVALID_DEVICE_TYPE:
      // implementation issue
    case CL_INVALID_VALUE:
      // shouldn't occur
    case CL_DEVICE_NOT_FOUND:
      // empty platform
    case CL_OUT_OF_RESSOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      return false;
    }

    if(size == 0)
    {
      // driver bug, result should've been CL_DEVICE_NOT_FOUND
      return false;
    }

    // allocate memory for device list
    cl_device_id* deviceIDs = new cl_device_id[size];
    if(deviceIDs == nullptr)
    {
      // OOM
      return false;
    }

    // obtain the actual deivce list
    result = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, size, deviceIDs, nullptr);

    // check for errors (shouldn't occur in practice)
    switch(result)
    {
    case CL_SUCCESS:
      // fine
      break;
    case CL_INVALID_PLATFORM:
      // we shouldn't have been constructed
    case CL_INVALID_DEVICE_TYPE:
      // implementation issue
    case CL_INVALID_VALUE:
      // shouldn't occur
    case CL_DEVICE_NOT_FOUND:
      // empty platform
    case CL_OUT_OF_RESSOURCES:
      // device OOM
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      delete [] deviceIDs;
      return false;
    }

    // construct device objects and add them to the list
    for(cl_uint i = 0; i < size; ++i)
    {
      csRef<Device> d;
      d.AttachNew(new Device(deviceIDs[i]));
      if(d->Initialize() && d->IsAvailable())
      {
        devices.Push(d);
      }
      else
      {
        // maybe warn here that a device was found to be invalid
      }
    }

    // free memory for the device list
    delete [] deviceIDs;

    // check again whether this platform is valid (contains any devices)
    return !devices.IsEmpty();
  }

  csString QueryInfo(cl_platform_info info)
  {
    cl_int result;
    cl_uint size;

    result = clGetPlatformInfo(platform, info, 0, nullptr, &size);

    switch(result)
    {
    case CL_SUCCESS:
      // everything fine
      break;

    case CL_INVALID_PLATFORM:
      // we shouldn't have been constructed...
    case CL_INVALID_VALUE:
      // info is not a proper value
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    case default:
      // driver bug
      return csString();
    }

    char* data = new char[size];
    if(data == nullptr)
    {
      return csString();
    }

    result = clGetPlatformInfo(platform, info, size, data, nullptr);

    switch(result)
    {
    case CL_SUCCESS:
      // everything fine
      break;

    case CL_INVALID_PLATFORM:
      // we shouldn't have been constructed...
    case CL_INVALID_VALUE:
      // info is not a proper value
    case CL_OUT_OF_HOST_MEMORY:
      // host OOM
    default:
      // driver bug
      delete [] data;
      return csString();
    }

    csString infoStr(data);
    delete [] data;

    return infoStr;
  }
}