/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
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

class cswinCacheDLL;

#ifndef CS_API_EXPORT
#define CS_API_EXPORT CS_CRYSTALSPACE_EXPORT
#define CS_API_EXPORT_UNDEF
#endif

class CS_API_EXPORT CS_API_NAME
{
private:
  static cswinCacheDLL* dll;
  static bool dllLoaded;
  static int refCount;

  static bool Available ();
public:
  // A refcount mechanism is used to determine when to unload
  // the DLL.
  static void IncRef ();
  static void DecRef ();
  static int GetRefCount ();

  #define FUNC_GROUP_BEGIN(name)	    \
    private: static bool name ## _avail;    \
    public: static bool name ## Available() \
    {					    \
      return Available() && name ## _avail; \
    }
  #define FUNC_GROUP_END
  #define FUNC(ret, name, args)		    \
    typedef ret (WINAPI* PFN ## name) args;
  #include CS_API_FUNCTIONS

  #define FUNC_GROUP_BEGIN(name)	    
  #define FUNC_GROUP_END
  #define FUNC(ret, name, args)		    \
    static PFN ## name name;
  #include CS_API_FUNCTIONS
};

#ifdef CS_API_EXPORT_UNDEF
#undef CS_API_EXPORT_UNDEF
#undef CS_API_EXPORT
#endif
