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

#include "sysdef.h"
#include "csengine/iextensn.h"

// Check the extension against the ExtensionMask
bool ExtensionLoaderInfo::CheckMask(const char* ext) {
//TODO
	if(!ext)
		return 0;

	return 1;
}

void csExtensions::DeleteAll() {
	for(int i=0; i<Extensions.Length(); i++) {
		ExtensionInfo* Item=(ExtensionInfo*)Extensions[i];
		if(Item)
			DeleteItem(Item);
	}
}

void csExtensions::DeleteItem(ExtensionInfo *Item) {
  delete Item;
}

void csLoaderExtensions::DeleteItem(ExtensionInfo *Item) {
	ExtensionLoaderInfo* eli=(ExtensionLoaderInfo*)Item;
	if(eli->Plugin)
		eli->Plugin->Release();
  delete eli;
}

// Get the list of IExtensions
void csExtensions::EnumExtensions() {
	csIniFile cfg("plugins.cfg");
	csSTRList Sections;
	cfg.EnumSections(&Sections);

	for(int i=0; i<Sections.Length(); i++) {
		AddExtension(Sections[i], cfg.GetStr(Sections[i], "TYPE", ""), csSTR(cfg.GetStr(Sections[i], "GUID", "")).ToGuid(), cfg);
	}
}

void csExtensions::AddExtension(const char* Name, const char* Type, const GUID& guid, csIniFile&) {
	Extensions.Push(new ExtensionInfo(Name, Type, guid));
}

void csLoaderExtensions::AddExtension(const char* Name, const char* Type, const GUID& guid, csIniFile& Config) {
	if(csSTR("Sprite").CompNoCase(Type))
		Extensions.Push(new ExtensionLoaderInfo(Config.GetStr(Name, "MASK", "*"), Name, Type, guid));
}

void csLanguageExtensions::AddExtension(const char* Name, const char* Type, const GUID& guid, csIniFile&) {
	if(csSTR("Language").CompNoCase(Type))
		Extensions.Push(new ExtensionInfo(Name, Type, guid));
}

IExtensionLoader* ExtensionLoaderInfo::GetLoaderPlugin() {
//TODO Azverkan: Check the extension against the ExtensionMask

	IExtensionLoader *el;
	if(FAILED(GetIExtension()->QueryInterface(IID_IExtensionLoader, (void**)&el)))
		return NULL;

	return el;
}

// Open the file using VFS and cache it
bool csLoaderExtensions::CacheFile(const char* file, csSTR *data) {
	if(!world->vfs->Exists(file)) {
		world->isys->Print(MSG_WARNING, csSTR("Couldn't find the file: ")+file+"\n");
		return 0;
	}

	csFile *vfsfile=world->vfs->Open(file, VFS_FILE_READ);
	data->SetSize(vfsfile->GetSize());
	if(!data->Length()) {
		delete vfsfile;
		return 0;
	}

	long read=vfsfile->Read((char*)data, data->Length());
	delete vfsfile;

	if(read!=data->Length()) {
		return 0;
	}

	return 1;
}

// Retrieve the correct loader
IExtensionLoader* csLoaderExtensions::GetLoader(const char* ext, csSTR *data) {
	for(int i=0; i<Extensions.Length(); i++) {
		ExtensionLoaderInfo* eli=(ExtensionLoaderInfo*)Extensions[i];
		if(eli->CheckMask(ext)) {
			IExtensionLoader *el=eli->GetLoaderPlugin();

			if(!el)
				continue;

			if(el->IsThisType(data)==S_OK)
				return el;
		}
	}
	return NULL;
}

// Call Cache file, then GetLoader
IExtensionLoader* csLoaderExtensions::CacheAndGetLoader(const char* file, csSTR *data) {
	char *ext="mdl";

	if(!CacheFile(file, data))
		return NULL;

	IExtensionLoader *plugin=GetLoader(ext, data);
	if(!plugin) {
		return NULL;
	}

	return plugin;
}

// Function to CacheFile, and dispatch to the correct loader
bool csLoaderExtensions::LoadFile(const char* file, const char* name) {
	csSTR data;

	IExtensionLoader *plugin=CacheAndGetLoader(file, &data);
	if(!plugin)
		return 0;

	if(plugin->LoadToWorld(world->cs, world->GetIWorld(), csSTR(name), &data)!=S_OK) {
		return 0;
	}

	return 1;
}

// Load template, but do not add to world
ISpriteTemplate* csLoaderExtensions::LoadSprite(const char* file, const char* name) {
	csSTR data;

	IExtensionLoader *plugin=CacheAndGetLoader(file, &data);
	if(!plugin)
		return NULL;

	ISpriteExtensionLoader *sel;

	if(FAILED(plugin->QueryInterface(IID_ISpriteExtensionLoader, (void**)&sel))) {
		return NULL;
	}

	ISpriteTemplate *itmpl;
	if(sel->LoadToSprite(world->cs, &itmpl, world->GetIWorld(), csSTR(name), &data)!=S_OK) {
		return NULL;
	}

	return itmpl;
}

/*
something* csLoaderExtensions::LoadMap(char *file) {

}

something* csLoaderExtensions::LoadSound(char *file) {

}

something* csLoaderExtensions::LoadTexture(char *file) {

}
*/

IExtension* ExtensionInfo::GetIExtension() {
//Loadup plugin if we need to
	if(!Plugin) {
		HRESULT hr=csCoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IExtension, (void**)&Plugin);
		if(FAILED(hr))
		{
//			System->Printf(MSG_WARNING, "Couldn't load the IExtension: %s of %s with %s\n", Name, Type, GuidToStr(clsid).GetData()); 
		}
	}
	return Plugin;
}

IExtension* csExtensions::GetExtension(const char* Name) {
	for(int i=0; i<Extensions.Length(); i++) {
		ExtensionInfo* ei=(ExtensionInfo*)Extensions[i];
		if(ei->Name.CompNoCase(Name)) {
			return ei->GetIExtension();
		}
	}
	return NULL;
}

ILanguageExtension* csLanguageExtensions::GetLanguage(const char* Name) {
	ILanguageExtension *ile;
	IExtension* ie=GetExtension(Name);
	if(!ie)
		return NULL;

	ie->QueryInterface(IID_ILanguageExtension, (void**)&ile);
	return ile;
}
