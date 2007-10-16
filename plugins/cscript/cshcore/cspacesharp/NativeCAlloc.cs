/*
    Copyright (C) 2007 by Ronie Salgado <roniesalg@gmail.com>

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
using System;
using System.Runtime.InteropServices;

namespace CrystalSpace.InteropServices
{
  // This is class is for do unmanaged memory related task, more specific
  // related to the crystal space heap. We cannot use Marshall.AllocHGlobal()
  // for alloc memory for a pointer that will be freed in the c side and vice-versa,
  // so we alloc the memory in the C side, and we freed it in the C side also.
  // We also cannot free a C pointer allocated in the C side by Crystal Space
  // using Marshal.FreeHGlobal(). In case that we try to do that, we will receive
  // a Segmentation Fault.
  public class NativeCAlloc
  {
    public static IntPtr Malloc (int size)
    {
      return corePINVOKE.csMalloc(size);
    }

    public static void Free (IntPtr ptr)
    {
      corePINVOKE.csFree(ptr);
    }
  }
}; //namespace CrystalSpace
