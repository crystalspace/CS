#ifndef __CS_OPENCL_DEVICE_IMPL_H__
#define __CS_OPENCL_DEVICE_IMPL_H__

#include <ivaria/clconsts.h>
#include <csutil/typetraits.h>

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  class Device
  {
  public:
    struct Version
    {
      size_t major;
      size_t minor;
    };

    Device(cl_device_id d) : device(d)
    {
    }

    bool Initialize();

    cl_device_id GetHandle() const
    {
      return device;
    }

    // general properties
    bool IsAvailable() const { return available; } // CL_DEVICE_AVAILABLE
    const char* GetName() const { return name; } // CL_DEVICE_NAME
    const char* GetVendor() const { return vendor; } // CL_DEVICE_VENDOR
    cl_device_type GetType() const { return type; } // CL_DEVICE_TYPE
    size_t GetID() const { return id; } // CL_DEVICE_VENDOR_ID

    // versions
    Version GetDiverVersion() const { return driverVersion; }
    Version GetLanguageVersion() const { return languageVersion; }
    Version GetVersion() const { return deviceVersion; }

    // general features
    bool IsFullProfile() const { return fullProfile; }
    bool HasCompiler() const { return hasCompiler; }
    bool HasExtension(const char* ext) const
    {
      return extensions.Contains(ext);
    }

    // execution features
    bool ExecKernel() const { return execKernel; }
    bool ExecNativeKernel() const { return execNativeKernel; }
    bool ImageSupport() const { return imageSupport; }
    bool OutOfOrderSupport() const { return oooSupport; }
    bool ProfilingSupport() const { return profilingSupport; }

    // execution properties
    size_t GetComputeUnits() const { return computeUnits; }
    size_t GetFrequency() const { return Frequency; } // in MHz

    // parameter memory properties
    size_t GetParameterSize() const { return paramSize; }
    size_t GetConstSize() const { return constSize; }
    size_t GetConstCount() const { return constCount; }

    // image processing properties
    size_t ImageReadArgs() const { return imageReadArgs; }
    size_t ImageWriteArgs() const { return imageWriteArgs; }
    size_t ImageSamplers() const; { return samplerArgs; }
    const size_t* GetImage2dSize() const { return image2DSize; }
    const size_t* GetImage3dSize() const { return image3DSize; }

  private:
    cl_device_id device;

    // general properties
    bool available;
    csString name;
    csString vendor;
    cl_device_type type;
    size_t id;

    // version info
    Version driverVersion;
    Version deviceVersion;
    Version languageVersion;

    // general features
    bool hasCompiler;
    bool fullProfile;

    // execution features
    bool execKernel;
    bool execNativeKernel;
    bool imageSupport;
    bool oooSupport;
    bool profilingSupport;

    // execution properties
    size_t computeUnits;
    size_t frequency;

    // memory properties
    size_t paramSize;
    size_t constSize;
    size_t constCount;

    // image properties
    size_t imageReadArgs;
    size_t imageWriteArgs;
    size_t samplerArgs;
    size_t image2DSize[2];
    size_t image3DSize[3];

    csSet<csString> extensions;

    void* QueryRawInfo(cl_device_info);
    template<typename T, typename U> bool QueryInfo(cl_device_info i, U& obj)
    {
      if(CS::Meta::IsPointer<T>::value)
      {
        obj = static_cast<T>(QueryRawInfo(i));
        return (obj != nullptr);
      }
      else
      {
        T* src = static_cast<T*>(QueryRawInfo(i));
        if(src != nullptr)
        {
          obj = *src;
          csFree(src);
          return true;
        }
      }
      return false;
    }
  };
}
CS_PLUGIN_NAMESPACE_END(CL)

#endif //__CS_OPENCL_DEVICE_IMPL_H__
