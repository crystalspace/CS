/*
  Copyright (C) 2010 by Matthieu Kraus

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_OPENCL_PROGRAM_H__
#define __CS_OPENCL_PROGRAM_H__

namespace CS
{
namespace CL
{
  struct iEvent;
  struct iBuffer;
  struct iLibrary;

  struct iKernel : public virtual iBase
  {
    SCF_INTERFACE(iKernel, 0, 0, 1);

    virtual iLibrary* GetLibrary() const = 0;

    template<typename T>
    bool SetArg(size_t id, T value)
    {
      return SetArg(id, &value, sizeof(T));
    }

    virtual bool SetArg(size_t id, const void* arg, size_t size) = 0;
    virtual bool SetArg(size_t id, iMemoryObject*) = 0;

    virtual bool SetDimension(size_t) = 0;
    virtual void SetWorkSize(const size_t*) = 0;
    virtual void SetWorkOffset(const size_t*) = 0;
    virtual int SetGroupSize(const size_t*) = 0;

    virtual size_t GetDimension() const = 0;
    virtual size_t* GetWorkSize() const = 0;
    virtual size_t* GetWorkOffset() const = 0;
    virtual size_t* GetGroupSize() const = 0;
  };

  struct iLibrary : public virtual iBase
  {
    SCF_INTERFACE(iLibrary, 0, 0, 1);

    virtual iStringArray* GetSource() const = 0;

    virtual iKernel* GetKernel(const char*) = 0;
    virtual void Precache() = 0;
  };

  struct iStructure : public virtual iBase
  {
    SCF_INTERFACE(iStructure, 0, 0, 1);

    virtual iStringArray* GetSource() const = 0;

    virtual iEvent* Write(iBuffer*, size_t offset = 0) = 0;
    virtual iEvent* Read(iBuffer*, size_t offset = 0) = 0;
    virtual iEvent* WriteToBuffer() = 0;
  };
} // namespace CL
} // namespace CS

#endif // __CS_OPENCL_PROGRAM_H__

