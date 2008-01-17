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
using System.Reflection;
using System.Runtime.InteropServices;
using CrystalSpace;

namespace CrystalSpace.InteropServices
{
  // This structure holds the data for be passed for 
  // an (int argc, char *argv) wrapped style method
  [StructLayout(LayoutKind.Sequential)]
  public struct csArgs
  {
    public int free;
    public int argc;
    public IntPtr exename;
    public IntPtr argv;
  };

  // This structure is used for P/Invoke calls, as a
  // return value for return generic interfaces pointer.
  [StructLayout(LayoutKind.Sequential)]
  public struct csRetInterface
  {
    public IntPtr ifaceptr;
    public IntPtr ifacename;
    public int free;
  };

  // This structure is used for pack the data of a interface
  [StructLayout(LayoutKind.Sequential)]
  public struct csInterfaceData
  {
    public int free;
    public int version;
    public IntPtr ifacename;
  };

  [StructLayout(LayoutKind.Sequential)]
  public struct csArrayPackData
  {
    public int count;
    public IntPtr array;
    public int iface;
    public int free;
  };

  public class csArgsUtils
  {
	  
    // Copy a string to the c heap
    public static IntPtr String2ASCII (String str)
    {
      int length = str.Length;
      // +1 for extra \0
      IntPtr ret = NativeCAlloc.Malloc (length + 1);
      char[] strarray = str.ToCharArray ();
      for (int i = 0; i < length; i++)
	Marshal.WriteByte (ret, i, (byte) strarray[i]);
      Marshal.WriteByte (ret, length, 0);
      return ret;
    }

    public static string ASCII2String(IntPtr str)
    {
      string ret = "";
      char chr = (char)Marshal.ReadByte(str);
      int ofs = 0;
      while(chr != 0)
      {
	ret += chr;
	ofs++;
	Marshal.ReadByte(str, ofs);
      }
      return ret;
    }
    // Takes a string array and return a structure with the
    // argc and argv arguments for C functions
    public static csArgs FromString (String[]args)
    {
      csArgs handle;
      IntPtr temp;
      IntPtr argv;
      int i;
      int argc = args.Length;

      if (argc == 0)
      {
	handle.free = 0;
        handle.exename = String2ASCII(Assembly.GetEntryAssembly().Location);
	handle.argc = 0;
	handle.argv = IntPtr.Zero;
	return handle;
      }

      argv = NativeCAlloc.Malloc (IntPtr.Size * argc);
      for (i = 0; i < argc; i++)
      {
	temp = String2ASCII (args[i]);
	Marshal.WriteIntPtr (argv, IntPtr.Size * i, temp);
      }

      handle.free = 1;
      handle.exename = String2ASCII(Assembly.GetEntryAssembly().Location);
      handle.argc = argc;
      handle.argv = argv;
      return handle;
    }

    // Puts the interface data into a structure, which will be passed to
    // c functions.
    public static csInterfaceData PackInterfaceData (Type iface_type)
    {
      csInterfaceData ret = new csInterfaceData ();

      try
      {
	if (iface_type == null)
	{
	  ret.free = 0;
	  ret.version = 0;
	  ret.ifacename = IntPtr.Zero;
	}
	else
	{
	  ret.ifacename = String2ASCII (iface_type.Name);
	  MethodInfo getVersion =
	    iface_type.GetMethod ("scfGetVersion",
				  BindingFlags.Static | BindingFlags.Public);
		
	  ret.version = (int) getVersion.Invoke (null, null);
	  ret.free = 1;
	}
      }
      catch
      {
	ret.free = 0;
	ret.version = 0;
	ret.ifacename = IntPtr.Zero;
      }

      return ret;
    }


    internal static bool IsSwigObject(object o)
    {
      Type objType = o.GetType();
      MethodInfo[] methods = objType.GetMethods(BindingFlags.Static | BindingFlags.NonPublic);
      foreach(MethodInfo info in methods)
      {
	if (info.Name == "getCPtr")
	{
	  return true;
	}
      }
      return false;
    }

    internal static HandleRef GetSwigHandle(object o)
    {
      Type objType = o.GetType();
      MethodInfo[] methods = objType.GetMethods(BindingFlags.Static | BindingFlags.NonPublic
		                                | BindingFlags.InvokeMethod);

      foreach(MethodInfo info in methods)
      {
	if (info.Name == "getCPtr")
	{
	  return new HandleRef(null, (IntPtr)info.Invoke(null,null));
	}
      }
      return new HandleRef(null, IntPtr.Zero);
    }

    public static csArrayPackData PackArrayData(object[] array)
    {
      csArrayPackData ret = new csArrayPackData();
      int object_len = 0;
      bool ptr = false;
      bool iface = false;
      bool floatv = false;
      bool stringv = true;

      if (array == null)
      {
	ret.count=0;
	ret.array = IntPtr.Zero;
	return ret;
      }

      if (array.Length <= 0)
      {
	ret.count=0;
	ret.array = IntPtr.Zero;
	return ret;
      }

      object test_obj = array[0];
      Type array_basetype = test_obj.GetType();
      if(array_basetype.Name == typeof(char).Name )
      {
	object_len = 1;
      }
      else if(array_basetype.Name == typeof(short).Name )
      {
	object_len = 2;
      }
      else if(array_basetype.Name == typeof(int).Name )
      {
	object_len = 4;
      }
      else if(array_basetype.Name == typeof(long).Name )
      {
	object_len = 8;
      }	
      else if(array_basetype.Name == typeof(float).Name )
      {
	object_len = 4;
	floatv = true;
      }
      else if(array_basetype.Name == typeof(double).Name )
      {
	object_len = 8;
	floatv = true;
      }
      else if(array_basetype.Name == typeof(string).Name )
      {
	object_len = 4;
	stringv = true;
      }
      else if(array_basetype.Name == typeof(IntPtr).Name )
      {
	object_len = IntPtr.Size;
	ptr = true;
      }
      else if(array_basetype.IsSubclassOf(typeof(iBase)))
      {
	object_len = IntPtr.Size;
	iface = true;
      }
      else
      {
	// Find getCPtr with reflection
	if(!IsSwigObject(array[0]))
	{
	  ret.count=0;
	  ret.array = IntPtr.Zero;
	  return ret;
	}
	else
	{
	  object_len = IntPtr.Size;
	}
      }

      int array_len = object_len * array.Length;
      IntPtr array_ptr = NativeCAlloc.Malloc(array_len);

      for(int i = 0; i < array.Length; i++)
      {
	switch(object_len)
	{
	case 1:
	  Marshal.WriteByte(array_ptr, i, (byte)array[i]);
	  break;
	case 2:
	  Marshal.WriteInt16(array_ptr, i*2, (short)array[i]);
	  break;
	case 4:
	  if(ptr)
	    Marshal.WriteIntPtr(array_ptr, i*4, (IntPtr)array[i]);
	  else if(stringv)
	    Marshal.WriteIntPtr(array_ptr, i*4, String2ASCII((string)array[i]));
	  else if(iface)
	    Marshal.WriteIntPtr(array_ptr, i*4, iBase.getCPtr((iBase)array[i]).Handle);
	  else if(IsSwigObject(array[i]))
	    Marshal.WriteIntPtr(array_ptr, i*4, GetSwigHandle((iBase)array[i]).Handle);
	  else if(floatv)
	    Marshal.WriteInt32(array_ptr, i*4, (int)array[i]);
	  else
	    Marshal.WriteInt32(array_ptr, i*4, (int)array[i]);
	  break;
	case 8:
	  if(floatv)
	    Marshal.WriteInt64(array_ptr, i*8, (long)array[i]);
	  else
	    Marshal.WriteInt64(array_ptr, i*8, (long)array[i]);
	  break;
	}
      }

      ret.count = array.Length;
      ret.array = array_ptr;
      ret.iface = iface?1:0;
      ret.free = 0;
      return ret;
    }
    // Creates an interface wrapper using the interface data packed in
		// a structure
    public static Object CreateInterface (csRetInterface iret)
    {
      Object _obj = null;
      iBase ibase;
      Type ifaceType;
      IntPtr arg1;
      Boolean arg2;
      String ifacename;

      try
      {
	if (iret.ifaceptr == IntPtr.Zero || iret.ifacename == IntPtr.Zero)
	  return null;

	ifacename = Marshal.PtrToStringAnsi (iret.ifacename);
	if (ifacename == "<dummy>")
	  return null;

	if (ifacename == "iBase" || ifacename == "CrystalSpace.iBase")
	{
	  ibase = new iBase (iret.ifaceptr, false);
	  ibase.IncRef ();
	  return (Object) ibase;
	}

	ifaceType = Type.GetType (ifacename);
	if (ifaceType == null)
	  return null;

	arg1 = iret.ifaceptr;
	arg2 = false;

	Object[]ifaceArgs = { arg1,  arg2 };
	_obj =
	  Activator.CreateInstance (ifaceType,
				    BindingFlags.
				    CreateInstance | BindingFlags.
				    Instance | BindingFlags.NonPublic, null,
				    ifaceArgs, null);
	ibase = (iBase) _obj;
	if (ibase != null)
	  ibase.IncRef ();
      }
      catch
      {
	_obj = null;
      }
      finally
      {
	if (iret.free == 1)
	{
	  NativeCAlloc.Free (iret.ifacename);
	}
      }
      return _obj;
    }
  }
}; // namespace CrystalSpace
