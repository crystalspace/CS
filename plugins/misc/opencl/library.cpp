#include <csutil/regexp.h>
#include "library.h"
#include "kernel.h"

CS_PLUGIN_NAMESPACE_BEGIN(CL)
{
  Library::Library(Manager* p, iStringArray* source) : scfImplementationType(this, nullptr),
                                                       parent(p), source(source)
  {
    // base regex to find the kernel declarations
    csRegExpMatcher kernelExp("(__)?kernel[:space:]+(__attribute__\({2}.+\){2}[:space:]+)*void[:space:]+\(.*?\)[:space:]+{",true);

    // regex to find macros/etc.
    csRegExpMatcher macroExp("[:alpha:]+\(.*\)",true);

    // access mode expression
    csRegExpMatcher accessExp("(^|[[:space:]*])(__)?(write|read)_only([[:space:]*]|$)",true);

    // address mode expression
    csRegExpMatcher addressExp("(^|[[:space:]*])(__)?(global|local|constant|private)([[:space:]*]|$)",true);

    // type expressions
    csRegExpMatcher imageExp("(^|[[:space:]*])image[23]d_t([[:space:]*]|$)",true);
    csRegExpMatcher samplerExp("(^|[[:space:]*])sampler_t([[:space:]*]|$)",true);

    for(size_t i = 0; i < source->GetSize(); ++i)
    {
      csString part(source->Get(i));
      csArray<csRegExpMatch> matches;

      // find matches
      csRegExpMatchError error = kernelExp.Match(part.GetData(), matches);
      CS_ASSERT(error == csrxNoError || error == csrxNoMatch);

      // process matches further
      for(size_t j = 0; j < matches.GetSize(); ++j)
      {
        size_t start = matches[j].startOffset + 8; // skip leading __kernel
        size_t size = matches[j].endOffset - start - 1; // skip trailing {
        csString match = part.Slice(start, size);
        match.LTrim(); // strip leading whitespaces
        while(match[0] == '_')
        {
          // we got an attribute here - strip it
          match.DeleteAt(0,13); // remove "__attribute__"
          size_t end = 2;
          size_t brackets = 2;
          while(brackets > 0)
          {
            if(match[end] == '(')
            {
              ++brackets;
            }
            else if(match[end] == ')')
            {
              --brackets; 
            }
            ++end;
          }
          match.DeleteAt(0,end); // remove the arguments for the attribute
          match.LTrim(); // remove whitespaces following the attribute
        }

        match.DeleteAt(0, 4); // strip leading "void"
        match.LTrim(); // strip whitespaces following the void

        // get name
        size_t open = match.FindFirst("(");
        CS_ASSERT(open != (size_t)-1); // regex error
        csString name = match.Slice(0, open);
        name.RTrim(); // strip trailing whitespaces from the name

        match.DeleteAt(0, open+1); // strip name and leading (
        match.DeleteAt(match.GetSize() - 1); // strip trailing )
        match.Trim();

        // check for macro calls
        if(macroExp.Match(match.GetData()) != csrxNoMatch)
        {
          // we don't allow macro function calls in the kernel defintion
          // (actually we don't even allow macros or even some typedefs,
          // but we can't check for those)
          CS_ASSERT(false);
        }

        // split arguments up
        csStringArray arguments(match.GetData(), ",");

        // create meta data
        KernelMetaData data;
        data.name = name;
        data.argCount = arguments.GetSize();
        data.args = new int[data.argCount];
        kernelData.PutUnique(data.name, data);

        // parse arguments
        for(size_t k = 0; k < arguments.GetSize(); ++k)
        {
          data.args[k] = 0;

          csString argString(arguments[k]);
          argString.Trim();
          csArray<csRegExpMatch> result;

          // parse access mode
          if(accessExp.Match(argString.GetData(), result) == csrxNoError)
          {
            if(argString.FindFirst("write", result[0].startOffset)
               < result[0].endOffset)
            {
              data.args[k] |= ARG_WRITE;
            }
            else
            {
              data.args[k] |= ARG_READ;
            }
          }
          else
          {
            data.args[k] |= ARG_READ_WRITE;
          }

          // check for image types
          if(imageExp.Match(argString.GetData(), result) == csrxNoError)
          {
            if(argString.FindFirst("image2d_t", result[0].startOffset)
               < result[0].endOffset)
            {
              data.args[k] |= ARG_IMAGE2D;
            }
            else
            {
              data.args[k] |= ARG_IMAGE3D;
            }
          }
          // check for sampler type
          else if(samplerExp.Match(argString.GetData()) == csrxNoError)
          {
            data.args[k] |= ARG_SAMPLER;
          }
          else if(argString.FindFirst("*") != (size_t)-1)
          {
            data.args[k] |= ARG_BUFFER;
          }
          else
          {
            data.args[k] |= ARG_DATA;
          }

          // parse address mode
          if(addressExp.Match(argString.GetData(), result) == csrxNoError)
          {
            size_t start = result[0].startOffset;
            size_t end = result[0].endOffseT;

            if(argString.FindFirst("global", start) < end)
            {
              data.args[k] |= ARG_GLOBAL;
            }
            else if(argString.FindFirst("local", start) < end)
            {
              data.args[k] |= ARG_LOCAL;
            }
            else if(argString.FindFirst("constant", start) < end)
            {
              data.args[k] |= ARG_CONST;
            }
            else
            {
              data.args[k] &= ~ARG_STORAGE_MASK; // ARG_PRIVATE
            }
          }
          else
          {
            switch(data.args[k] & ARG_TYPE_MASK)
            {
            case ARG_IMAGE2D:
            case ARG_IMAGE3D:
              data.args[k] |= ARG_GLOBAL;
              break;

            default:
              data.args[k] &= ~ARG_STORAGE_MASK; // ARG_PRIVATE
            }
          }
        }
      }
    }
  }

  Library::~Library()
  {
    csHash<csRef<Context>, cl_program>::Iterator it = handles.GetIterator();
    while(it.HasNext())
    {
      cl_program p = it.Next();
      cl_int error = clReleaseProgram(p);
      CS_ASSERT(error == CL_SUCCESS);
    }

    csHash<csString, KernelMetaData>::Iterator kernelIt = kernelData.GetIterator();
    while(it.HasNext())
    {
      delete [] it.Next().args;
    }
  }

  csRef<Event> GetHandle(Context*, cl_program& p);
  {
    if(handles.Contains(c))
    {
      p = *handles.GetElementPointer(c);
      return *buildEvents.GetElementPointer(p);
    }
    else
    {
      const char** strings = new char*[source->GetSize()];
      if(strings == nullptr)
      {
        // OOM
        return csRef<Event>(nullptr);
      }

      for(size_t i = 0; i < source->GetSize(); ++i)
      {
        strings[i] = source->Get(i);
      }

      cl_int error;
      p = clCreateProgramWithSource(c->GetHandle(), source->GetSize(),
                                    strings, nullptr, &error);
      if(error != CL_SUCCESS)
      {
        return csRef<Event>(nullptr);
      }

      // build for all devices in context
      //@@@todo: maybe we want to somehow specify options here?
      csRef<Event> e = csPtr<Event>(new Event());
      error = clBuildProgram(p, /* devices */ nullptr, /* deviceCount */ 0,
                             /* options */ "", BuildHandler, (void*)e);

      if(error != CL_SUCCESS)
      {
        clReleaseProgram(p);
        return csRef<Event>(nullptr);
      }

      // the event has to stay alive until the build is done
      e->IncRef();

      handles.PutUnique(c, p);
      buildEvents.PutUnique(p, e);
      return e;
    }
  }

  void Library::Precache()
  {
    // create the program in all contexts
    // (will also start the builds for all devices)
    const csRefArray<Context>& contexts = parent->GetContexts();
    csRefArray<Context>::ConstIterator it = contexts.GetIterator();
    while(it.HasNext())
    {
      cl_program p;
      GetHandle(it.Next(), p);
    }
  }

  csPtr<iKernel> Library::CreateKernel(const char* name)
  {
    if(!kernelData.Contains(name))
    {
      // unknown kernel
      csPtr<iKernel>(nullptr);
    }

    const KernelMetaData& data = *kernelData.GetElementPointer(name);

    // create new kernel
    csRef<iKernel> kernel;
    kernel.AttachNew(new Kernel(this, data));
    return csPtr<iKernel>(kernel);
  }

  static void Library::BuildHandler(cl_program p, void* data)
  {
    csRef<Event> e = static_cast<Event*>(data);

    // release the reference we retained
    // to keep the event alive during build
    // (it's kept alive by the ref while this function
    // is executed)
    e->DecRef();

    // keep track whether the build
    // was successful
    bool success = true;

    // obtain device list
    cl_uint deviceCount;
    cl_device_id* devices = GetDevices(p, deviceCount);
    if(devices == nullptr)
    {
      // error retrieving device list
      success = false;
    }

    // check whether it built properly on all devices
    for(cl_uint i = 0; i < deviceCount; ++i)
    {
      cl_build_status status;
      cl_int error = clGetProgramBuildInfo(p, devices[i], CL_PROGRAM_BUILD_STATUS,
                                           sizeof(cl_build_status), &status, nullptr);
      if(error != CL_SUCCESS)
      {
        success = false;
        continue;
      }

      if(status != CL_BUILD_SUCCESS)
      {
        // build failed, try to retrieve the error
        success = false;

        PrintLog(p, devices[i]);
      }
    }

    // free device list
    delete [] devices;

    // fire the build event
    e->Fire(success);
  }

  static cl_device_id* Library::GetDevices(cl_program p, cl_uint& deviceCount)
  {
    // get devices this program is associated with
    cl_int error;
    error = clGetProgramInfo(p, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint),
                             &deviceCount, nullptr);
    if(error != CL_SUCCESS)
    {
      // failed to retrieve the number of devices
      deviceCount = 0;
      return nullptr;
    }

    cl_device_id* devices = new cl_device_id[deviceCount];
    if(devices == nullptr)
    {
      // OOM
      deviceCount = 0;
      return nullptr;
    }

    error = clGetProgramInfo(p, CL_PROGRAM_DEVICES, deviceCount*sizeof(cl_device_id),
                             devices, nullptr);
    if(error != CL_SUCCESS)
    {
      // failed to retrieve device list
      delete [] devices;
      deviceCount = 0;
      return nullptr;
    }

    return devices;
  }

  static void Library::PrintLog(cl_program p, cl_device_id d)
  {
    // get the size of the build log
    size_t logSize;
    error = clGetProgramBuildInfo(p, d, CL_PROGRAM_BUILD_LOG,
                                  0, nullptr, &logSize);
    if(error != CL_SUCCESS)
    {
      return;
    }

    // allocate resssources to hold the log
    char* buildLog = csAlloc(logSize);
    if(buildLog == nullptr)
    {
      // OOM
      return;
    }

    // obtain the log
    error = clGetProgramBuildInfo(p, d, CL_PROGRAM_BUILD_LOG,
                                  logSize, buildLog, nullptr);
    if(error == CL_SUCCESS)
    {
      // print the build log here - maybe only if verbose?
    }

    // free ressources allocated for the build log
    csFree(buildLog);
  }
}
CS_PLUGIN_NAMESPACE_END(CL)
