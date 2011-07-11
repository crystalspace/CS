#include "cssysdef.h"

#include <csutil/stringarray.h>

#include "device.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  template<typename T, typename U> bool Device::QueryInfo(cl_device_info i, U*& obj)
  {
    obj = static_cast<T>(QueryRawInfo(i));
    return (obj != nullptr);
  }

  template<typename T, typename U> bool Device::QueryInfo(cl_device_info i, U& obj)
  {
    T* src = static_cast<T*>(QueryRawInfo(i));
    if(src != nullptr)
    {
      obj = *src;
      cs_free(src);
      return true;
    }
    return false;
  }

  bool Device::Initialize()
  {
    bool success = true;

    // obtain general properties
    success &= QueryInfo<cl_bool>(CL_DEVICE_AVAILABLE, available);
    success &= QueryInfo<cl_uint>(CL_DEVICE_VENDOR_ID, id);
    success &= QueryInfo<cl_device_type>(CL_DEVICE_TYPE, type);
    {
      char* name = nullptr;
      if(QueryInfo<char*>(CL_DEVICE_VENDOR, name))
      {
        vendor = name;
        cs_free(name);
      }
      else
      {
        success = false;
      }

      if(QueryInfo<char*>(CL_DEVICE_NAME, name))
      {
        this->name = name;
        cs_free(name);
      }
      else
      {
        success = false;
      }
    }

    // obtain version info
    {
      char* version = nullptr;
      if(QueryInfo<char*>(CL_DEVICE_VERSION, version))
      {
        csStringArray split(version, " ");
        cs_free(version);

        if(csString("OpenCL") == split[0])
        {
          split.SplitString(split[1],".");
          deviceVersion.minor = /* to int split.Pop() */0;
          deviceVersion.major = /* to int split.Pop() */0;
        }
        else
        {
          success = false;
        }
      }
      else
      {
        success = false;
      }

      if(QueryInfo<char*>(CL_DRIVER_VERSION, version))
      {
        csStringArray split(version, " ");
        cs_free(version);

        if(csString("OpenCL") == split[0])
        {
          split.SplitString(split[1],".");
          driverVersion.minor = /* to int split.Pop() */0;
          driverVersion.major = /* to int split.Pop() */0;
        }
        else
        {
          success = false;
        }
      }
      else
      {
        success = false;
      }

      if(QueryInfo<char*>(CL_DEVICE_OPENCL_C_VERSION, version))
      {
        csStringArray split(version, " ");
        cs_free(version);

        if(csString("OpenCL") != split[0] || csString("C") != split[1])
        {
          split.SplitString(split[2],".");
          languageVersion.minor = /* to int split.Pop() */0;
          languageVersion.major = /* to int split.Pop() */0;
        }
        else
        {
          success = false;
        }
      }
      else
      {
        success = false;
      }
    }

    // obtain general features
    success &= QueryInfo<cl_bool>(CL_DEVICE_COMPILER_AVAILABLE, hasCompiler);
    {
      char* profile = nullptr;
      if(QueryInfo<char*>(CL_DEVICE_PROFILE, profile))
      {
        fullProfile = csString("FULL_PROFILE") == profile;
        cs_free(profile);
      }
      else
      {
        success = false;
      }
    }

    // obtain execution features
    {
      cl_device_exec_capabilities exec;
      success &= QueryInfo<cl_device_exec_capabilities>(CL_DEVICE_EXECUTION_CAPABILITIES, exec);
      execKernel = exec & CL_EXEC_KERNEL;
      execNativeKernel = exec & CL_EXEC_NATIVE_KERNEL;
    }
    {
      cl_command_queue_properties queue;
      success &= QueryInfo<cl_command_queue_properties>(CL_DEVICE_QUEUE_PROPERTIES, queue);
      oooSupport = queue & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
      profilingSupport = queue & CL_QUEUE_PROFILING_ENABLE;
    }
    success &= QueryInfo<cl_bool>(CL_DEVICE_IMAGE_SUPPORT, imageSupport);

    // obtain execution properties
    success &= QueryInfo<cl_uint>(CL_DEVICE_MAX_COMPUTE_UNITS, computeUnits);
    success &= QueryInfo<cl_uint>(CL_DEVICE_MAX_CLOCK_FREQUENCY, frequency);

    // obtain memory properties
    success &= QueryInfo<size_t>(CL_DEVICE_MAX_PARAMETER_SIZE, paramSize);
    success &= QueryInfo<cl_ulong>(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, constSize);
    success &= QueryInfo<cl_uint>(CL_DEVICE_MAX_CONSTANT_ARGS, constCount);

    // obtain image properties
    success &= QueryInfo<cl_uint>(CL_DEVICE_MAX_READ_IMAGE_ARGS, imageReadArgs);
    success &= QueryInfo<cl_uint>(CL_DEVICE_MAX_WRITE_IMAGE_ARGS, imageWriteArgs);
    success &= QueryInfo<cl_uint>(CL_DEVICE_MAX_SAMPLERS, samplerArgs);

    success &= QueryInfo<size_t>(CL_DEVICE_IMAGE2D_MAX_WIDTH, image2DSize[0]);
    success &= QueryInfo<size_t>(CL_DEVICE_IMAGE2D_MAX_HEIGHT, image2DSize[1]);

    success &= QueryInfo<size_t>(CL_DEVICE_IMAGE3D_MAX_WIDTH, image3DSize[0]);
    success &= QueryInfo<size_t>(CL_DEVICE_IMAGE3D_MAX_HEIGHT, image3DSize[1]);
    success &= QueryInfo<size_t>(CL_DEVICE_IMAGE3D_MAX_DEPTH, image3DSize[2]);


    // obtain extension list
    char* extensionString = nullptr;
    if(QueryInfo<char*>(CL_DEVICE_EXTENSIONS, extensionString))
    {
      csStringArray extensionList(extensionString, " ");
      cs_free(extensionString);

      for(size_t i = 0; i < extensionList.GetSize(); ++i)
      {
        extensions.Add(extensionList[i]);
      }
    }
    else
    {
      success = false;
    }

    return success;
  }

  void* Device::QueryRawInfo(cl_device_info info)
  {
    size_t size;
    cl_int error = clGetDeviceInfo(device, info, 0, nullptr, &size);
    if(error != CL_SUCCESS)
    {
      // failed to retrieve size of info
      return nullptr;
    }

    void* data = cs_malloc(size);
    if(data == nullptr)
    {
      // OOM
      return nullptr;
    }

    error = clGetDeviceInfo(device, info, size, data, nullptr);
    if(error != CL_SUCCESS)
    {
      // failed to retrieve info
      cs_free(data);
      return nullptr;
    }

    return data;
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
