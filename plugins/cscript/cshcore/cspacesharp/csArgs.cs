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

namespace CrystalSpace.InteropServices
{
  // This structure holds the data for be passed for 
  // an (int argc, char *argv) wrapped style method
  public struct csArgs
  {
    public int free;
    public int argc;
    public IntPtr argv;
  }

  // This structure is used for P/Invoke calls, as a
  // return value for return generic interfaces pointer.
  public struct csRetInterface
  {
    public IntPtr ifaceptr;
    public IntPtr ifacename;
    public int free;
  }

  // This structure is used for pack the data of a interface
  public struct csInterfaceData
  {
    public int free;
    public int version;
    public IntPtr ifacename;
  }

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
	handle.argc = 0;
	handle.free = 0;
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
      handle.argc = argc;
      handle.argv = argv;
      return handle;
    }

    // Puts the interface data into a structure, which will be passed to
    // c functions.
    static public csInterfaceData PackInterfaceData (Type iface_type)
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

    // Creates an interface wrapper using the interface data packed in
    // a structure
    static public Object CreateInterface (csRetInterface iret)
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
