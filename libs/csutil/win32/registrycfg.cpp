/*
    Copyright (C) 2003 by Frank Richter

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


#include "cssysdef.h"
#include "csutil/util.h"
#include "csutil/sysfunc.h"
#include "csutil/win32/registrycfg.h"


csWin32RegistryConfig::csWin32RegistryConfig ()
  : scfImplementationType (this), hKey (0), hKeyParent (HKEY_CURRENT_USER), 
  Prefix (0), writeAccess (false), Key (0), status (new rcStatus)
{
}

csWin32RegistryConfig::~csWin32RegistryConfig()
{
  Close();
  delete status;
}

void csWin32RegistryConfig::ReplaceSeparators (char* key) const
{
  size_t len = (size_t)strlen (key);
  size_t p;
  while ((p = strcspn (key, "./")) < len)
  {
    *(key + p) = '\\';
  }
}

bool csWin32RegistryConfig::TryOpen (HKEY parent, HKEY& regKey, DWORD access, 
			             const char* keyName, bool create)
{
  regKey = 0;

  // Empty string would cause messing around in HKCU\Software -> dangerous
  if (!keyName || (*keyName == 0))
    return false;

  if (Key != keyName)
  {
    delete[] Key;
    Key = csStrNew (keyName);
  }

/*  csString key;
  key.Format ("Software\\%s", Key);
  ReplaceSeparators (key);*/

  hKeyParent = parent;
  LONG err;
  if (create)
  {
    err = RegCreateKeyEx (parent,
      Key/*key*/, 0, 0, 0, 
      access, 
      0, &regKey, 0);
  }
  else
  {
    err = RegOpenKeyEx (parent,
      Key/*key*/, 0, access, &regKey);
  }

  return (err == ERROR_SUCCESS);
}

bool csWin32RegistryConfig::Open (const char* Key, HKEY parent)
{
  Close();
  writeAccess = false;
  return TryOpen (parent, hKey, KEY_READ, Key, false);
}

void csWin32RegistryConfig::Close ()
{
  delete[] Key; Key = 0;

  if (hKey != 0)
  {
    RegCloseKey (hKey);
    hKey = 0;
  }
}

const char* csWin32RegistryConfig::GetFileName () const
{
  return "Win32Registry";
}

iVFS* csWin32RegistryConfig::GetVFS () const
{
  return 0;
}

void csWin32RegistryConfig::SetFileName (const char*, iVFS*)
{
}

bool csWin32RegistryConfig::Load (const char* /*iFileName*/, iVFS*,
  bool /*Merge*/, bool /*NewWins*/)
{
  return true;
}

bool csWin32RegistryConfig::Save ()
{
  return true;
}

bool csWin32RegistryConfig::Save (const char * /*iFileName*/, iVFS*)
{
  return true;
}

void csWin32RegistryConfig::Clear ()
{
  size_t i;
  for (i = 0; i < iters.Length(); i++)
  {
    iters.Get(i)->Rewind();
  }

  if (WriteAccess())
  {
    int index = 0;
    char Name[256];
    LONG err;
    DWORD nlen = sizeof (Name);

    while ((err = RegEnumValue (hKey, index, Name,
      &nlen, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
      RegDeleteValue (hKey, Name);

      nlen = sizeof (Name);
      index++;
    }
  }
}

csPtr<iConfigIterator> csWin32RegistryConfig::Enumerate (const char *Subsection)
{
  if (!SubsectionExists (Subsection)) return 0;

  csWin32RegistryIterator* it = new csWin32RegistryIterator (this, Subsection);
  iters.Push (it);
  return it;
}

bool csWin32RegistryConfig::KeyExists (const char *Key) const
{
  if (hKey == 0) return false;

  LONG err = RegQueryValueEx (hKey,
    Key, 0, 0, 0, 0);

  return (err == ERROR_SUCCESS);
}

bool csWin32RegistryConfig::SubsectionExists (const char *Subsection) const
{
  if (hKey == 0) return false;

  char Name[256];
  DWORD namelen;
  DWORD index = 0;
  LONG err;

  do 
  {
    namelen = sizeof (Name);
    if ((err = RegEnumValue (hKey, index++, Name, &namelen, 0, 0,
      0, 0)) != ERROR_SUCCESS) 
    {
      return false;
    }
      
    if (strncasecmp (Name, Subsection,
      strlen (Subsection)) == 0)
    {
      return true;
    }
  }
  while (true);

  return false;
}

bool csWin32RegistryConfig::InternalGetValue (const char* Key,
    DWORD& type, Block_O_Mem& data) const
{
  DWORD datasize;
  LONG err = RegQueryValueEx (hKey,
    Key, 0, &type, 0, &datasize);

  if (err == ERROR_SUCCESS)
  {
    data.SetSize (datasize);
    err = RegQueryValueEx (hKey,
      Key, 0, 0, data.data, &datasize);
  }

  return (err == ERROR_SUCCESS);
}

int csWin32RegistryConfig::RegToInt (DWORD type, Block_O_Mem& data, int Def) const
{
  int n, v;
  switch (type)
  {
  case REG_SZ:
    n = sscanf ((char*)data.data, "%d", &v);
    return (n != 0) ? v : Def;
    break;
  case REG_DWORD:
    return *((int*)data.data);
    break;
  case REG_BINARY:
    return (int)*((float*)data.data);
    break;
  default:
    return Def;
  }
}

float csWin32RegistryConfig::RegToFloat (DWORD type, Block_O_Mem& data, float Def) const
{
  int n;
  float v;

  switch (type)
  {
  case REG_SZ:
    n = sscanf ((char*)data.data, "%f", &v);
    return (n != 0) ? v : Def;
    break;
  case REG_DWORD:
    return *((int*)data.data);
    break;
  case REG_BINARY:
    return *((float*)data.data);
    break;
  default:
    return Def;
  }
}

const char* csWin32RegistryConfig::RegToStr (DWORD type, Block_O_Mem& data,
					const char* Def) const
{
  csString buf;
  switch (type)
  {
  case REG_SZ:
    return status->strings.Register ((char*)data.data, 0);
    break;
  case REG_DWORD:
    buf.Format ("%d", *((int*)data.data));
    return status->strings.Register (buf, 0);
    break;
  case REG_BINARY:
    buf.Format ("%g", *((float*)data.data));
    return status->strings.Register (buf, 0);
    break;
  default:
    return Def;
  }
}

bool csWin32RegistryConfig::RegToBool (DWORD type, Block_O_Mem& data,
				  bool Def) const
{
  switch (type)
  {
  case REG_SZ:
    return (
      strcasecmp((char*)data.data, "true") == 0 ||
      strcasecmp((char*)data.data, "yes" ) == 0 ||
      strcasecmp((char*)data.data, "on"  ) == 0 ||
      strcasecmp((char*)data.data, "1"   ) == 0);
    break;
  case REG_DWORD:
    return (*((int*)data.data) != 0);
    break;
  case REG_BINARY:
    return *((float*)data.data) != 0.0f;
    break;
  default:
    return Def;
  }
}

bool csWin32RegistryConfig::WriteAccess()
{
  if (writeAccess) return true;

  RegCloseKey (hKey);
  if (!TryOpen (hKeyParent, hKey, KEY_READ | KEY_WRITE, Key, true))
  {
    TryOpen (hKeyParent, hKey, KEY_READ, Key, false);
    return false;
  }
  else
  {
    writeAccess = true;
    return true;
  }
}

int csWin32RegistryConfig::GetInt (const char *Key, int Def) const
{
  if (hKey == 0) return Def;

  DWORD type;

  Block_O_Mem data;
  if (InternalGetValue (Key, type, data))
  {
    return RegToInt (type, data, Def);
  }
  else
  {
    return Def;
  }
}

float csWin32RegistryConfig::GetFloat (const char *Key, float Def) const
{
  if (hKey == 0) return Def;

  DWORD type;

  Block_O_Mem data;
  if (InternalGetValue (Key, type, data))
  {
    return RegToFloat (type, data, Def);
  }
  else
  {
    return Def;
  }
}

const char *csWin32RegistryConfig::GetStr (const char *Key, const char *Def) const
{
  if (hKey == 0) return Def;

  DWORD type;

  Block_O_Mem data;
  if (InternalGetValue (Key, type, data))
  {
    return RegToStr (type, data, Def);
  }
  else
  {
    return Def;
  }
}

bool csWin32RegistryConfig::GetBool (const char *Key, bool Def) const
{
  if (hKey == 0) return Def;

  DWORD type;

  Block_O_Mem data;
  if (InternalGetValue (Key, type, data))
  {
    return RegToBool (type, data, Def);
  }
  else
  {
    return Def;
  }
}

const char *csWin32RegistryConfig::GetComment (const char * /*Key*/) const
{
  return 0;
}

bool csWin32RegistryConfig::InternalSetValue (const char* Key,
    DWORD type, const void* data, size_t datasize)
{
  if (!WriteAccess()) return false;

  LONG err = RegSetValueEx (hKey,
    Key, 0, type, (BYTE*)data, (DWORD)datasize);

  return (err == ERROR_SUCCESS);
}

void csWin32RegistryConfig::SetStr (const char *Key, const char *Val)
{
  if (Val == 0)
    DeleteKey (Key);
  else
    InternalSetValue (Key, REG_SZ, Val, strlen (Val) + 1);
}

void csWin32RegistryConfig::SetInt (const char *Key, int Value)
{
  InternalSetValue (Key, REG_DWORD, &Value, sizeof (Value));
}

void csWin32RegistryConfig::SetFloat (const char *Key, float Value)
{
  //InternalSetValue (Key, REG_BINARY, &Value, sizeof (Value));
  // for better readability:
  csString buf;
  buf.Format ("%g", Value);
  InternalSetValue (Key, REG_SZ, buf, strlen (buf) + 1);
}

void csWin32RegistryConfig::SetBool (const char *Key, bool Value)
{
  int i = (Value ? 1 : 0);
  InternalSetValue (Key, REG_DWORD, &i, sizeof (i));
}

bool csWin32RegistryConfig::SetComment (
  const char * /*Key*/, const char * /*Text*/)
{
  return false;
}

void csWin32RegistryConfig::DeleteKey (const char *Key)
{
  if (!WriteAccess()) return;

  RegDeleteValue (hKey, Key);
}

const char *csWin32RegistryConfig::GetEOFComment () const
{
  return 0;
}

void csWin32RegistryConfig::SetEOFComment (const char * /*Text*/)
{
}

csWin32RegistryIterator::csWin32RegistryIterator (csWin32RegistryConfig* Owner, 
  const char* Subsection)
  : scfImplementationType (this), owner (Owner), status (new riStatus), EnumIndex (0), 
  SubsectionName (csStrNew (Subsection))
{
}

csWin32RegistryIterator::~csWin32RegistryIterator()
{
  owner->iters.Delete (this);
  delete[] SubsectionName;
  delete status;
}

iConfigFile* csWin32RegistryIterator::GetConfigFile () const
{
  return owner;
}

const char *csWin32RegistryIterator::GetSubsection () const
{
  return SubsectionName;
}

void csWin32RegistryIterator::Rewind ()
{
  EnumIndex = 0;
}

// navigate though the reg key to the next value entry
bool csWin32RegistryIterator::Next()
{
  LONG err;

  char Name[256];
  DWORD namelen;
    
  do
  {
    namelen = sizeof (Name);

    if ((err = RegEnumValue (owner->hKey, EnumIndex,
      Name, &namelen, 0, 0, 0, 0)) != ERROR_SUCCESS)
    {
      return false;
    }
    EnumIndex++;
    if (strncasecmp (Name, SubsectionName, strlen (SubsectionName)) == 0)
    {
      return true;
    }
  }
  while (true);
}

const char *csWin32RegistryIterator::GetKey (bool Local) const
{
  char Name[256];
  DWORD namelen = sizeof (Name);

  LONG err;

  if ((err = RegEnumValue (owner->hKey, 
    EnumIndex - 1, Name, 
    &namelen, 0, 0, 0, 0)) != ERROR_SUCCESS)
  {
    return 0;
  }

  const char* str = status->strings.Register (Name, 0);

  return (Local ? (str + strlen (SubsectionName)) : str);
}

bool csWin32RegistryIterator::GetCurrentData (DWORD& type, 
  csWin32RegistryConfig::Block_O_Mem& data) const
{
  DWORD datasize;
  char Name[256];
  DWORD namelen = sizeof(Name);

  if (RegEnumValue (owner->hKey, 
    EnumIndex - 1, Name, 
    &namelen, 0, &type, 0, &datasize) != ERROR_SUCCESS)
  {
    return false;
  }
  data.SetSize (datasize);

  if (RegQueryValueEx (owner->hKey, Name, 0,
    0, data.data, &datasize) != ERROR_SUCCESS)
  {
    return false;
  }

  return true;
}

int csWin32RegistryIterator::GetInt () const
{
  const int Def = 0;

  DWORD type;
  csWin32RegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToInt (type, data, Def);
}

float csWin32RegistryIterator::GetFloat () const
{
  const float Def = 0.0f;

  DWORD type;
  csWin32RegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToFloat (type, data, Def);
}

const char *csWin32RegistryIterator::GetStr () const
{
  const char* Def = "";

  DWORD type;
  csWin32RegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToStr (type, data, Def);
}

bool csWin32RegistryIterator::GetBool () const
{
  const bool Def = false;

  DWORD type;
  csWin32RegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToBool (type, data, Def);
}

const char *csWin32RegistryIterator::GetComment () const
{
  return 0;
}


