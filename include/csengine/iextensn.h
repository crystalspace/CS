/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Brandon Ehle

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
// iextensn.h: Interface for loading file extension modules
//
//////////////////////////////////////////////////////////////////////

#ifndef __IEXTENSN_H__
#define __IEXTENSN_H__

#include "cscom/com.h"
#include "csengine/world.h"
#include "csutil/vfs.h"
#include "csengine/newclass.h"
#include "csengine/intrface.h"
#include "csutil/inifile.h"

class csWorld;

/*
interface IMapExtensionLoader:public IExtensionLoader;
interface ISoundExtensionLoader:public IExtensionLoader;
interface ITextureExtensionLoader:public IExtensionLoader;
*/

class ExtensionInfo {
public:
	/// Extension Description
	csSTR Name;

	/// Extension Type
	csSTR Type;

	/// The COM interface for the plugin
	IExtension *Plugin;

	/// The CLSID for the plugin
	CLSID clsid;

	IExtension* GetIExtension();

	ExtensionInfo(const char* tName, const char* tType, const CLSID& Tclsid)
		:Name(tName), Type(tType), Plugin(NULL), clsid(Tclsid) {}
};

class ExtensionLoaderInfo:public ExtensionInfo {
public:
	/// List of Extensions supported (queried from registry), "*" denotes all extensions
	csSTR Mask;

	/// Check the extension against the ExtensionMask
	bool CheckMask(const char* ext);

	/// Check the extension against the ExtensionMask
	IExtensionLoader* GetLoaderPlugin();

	ExtensionLoaderInfo(const char* tMask, const char* Name, const char* Type, const CLSID& clsid)
		:ExtensionInfo(Name, Type, clsid), Mask(tMask) {}
};

class csExtensionInfoVector:public csVector {};

class csExtensions:public csObject {
protected:
	/// Array of IExtensionLoaders & their supported file extensions (* would be a valid extension)
	csExtensionInfoVector Extensions;
public:
	~csExtensions() {
		DeleteAll();
	}

	/// Get the list of IExtensions
	void EnumExtensions();

	/// Virtual Specialized Extension Add
	virtual void AddExtension(const char* Name, const char* Type, const GUID& guid, csIniFile& Config);

	/// Delete all the Extensions from the list
	void DeleteAll();

	/// Delete an items memory before remove from list
	virtual void DeleteItem(ExtensionInfo *Item);

	IExtension* GetExtension(const char* Name);
};

class csLanguageExtensions:public csExtensions {
public:
	/// Virtual Specialized Extension Add
	virtual void AddExtension(const char* Name, const char* Type, const GUID& guid, csIniFile& Config);

	ILanguageExtension* GetLanguage(const char* Name);
};

class csLoaderExtensions:public csExtensions {
	/// World pointer
	csWorld *world;

public:
	/// Takes a pointer to an IWorld
	csLoaderExtensions(csWorld *Tworld)
		:world(Tworld) {}

	/// Virtual Specialized Extension Add
	virtual void AddExtension(const char* Name, const char* Type, const GUID& guid, csIniFile& Config);

	/// Open the file using VFS and cache it
	bool CacheFile(const char* file, csSTR *data);

	/// Retrieve the correct loader
	IExtensionLoader* GetLoader(const char* ext, csSTR *data);

	/// Call Cache file, then GetLoader
	IExtensionLoader* CacheAndGetLoader(const char* file, csSTR *data);

	/// Function to CacheFile, and dispatch to the correct loader
	bool LoadFile(const char* file, const char* name);

	/// Load template, but do not add to world
	ISpriteTemplate* LoadSprite(const char* file, const char* name);
/*
	/// Load map, but do not add to world
	something* LoadMap(const char* *file);

	/// Load sound, but do not add to world
	something* LoadSound(const char* *file);

	/// Load texture, but do not add to world
	something* LoadTexture(const char* *file);
*/

	/// Delete an items memory before remove from list
	void DeleteItem(ExtensionInfo *Item);
};

#endif
