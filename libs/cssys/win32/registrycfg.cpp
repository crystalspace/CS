/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "registrycfg.h"

csPtr<iConfigFile> csGetPlatformConfig (const char* key)
{
  csRegistryConfig* cfg = new csRegistryConfig ();
  if (!cfg->Open (key))
  {
    delete cfg; cfg = 0;
  }
  return csPtr<iConfigFile> (cfg);
}

SCF_IMPLEMENT_IBASE (csRegistryConfig)
  SCF_IMPLEMENTS_INTERFACE (iConfigFile)
SCF_IMPLEMENT_IBASE_END

csRegistryConfig::csRegistryConfig ()
{
  SCF_CONSTRUCT_IBASE (0);

  Prefix = 0;
  hKey = 0;
  Key = 0;
  status = new rcStatus ();
  writeAccess = false;
}

csRegistryConfig::~csRegistryConfig()
{
  delete status;
  delete[] Key;

  if (hKey != 0)
  {
    RegCloseKey (hKey);
  }
}

void csRegistryConfig::ReplaceSeparators (char* key) const
{
  size_t len = (size_t)strlen (key);
  size_t p;
  while ((p = strcspn (key, "./")) < len)
  {
    *(key + p) = '\\';
  }
}

bool csRegistryConfig::TryOpen (HKEY& regKey, DWORD access, 
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

  CS_ALLOC_STACK_ARRAY (char, key, 9 + strlen (Key) + 1); // 9 = Length "Software\"
  sprintf (key, "Software\\%s", Key);
  ReplaceSeparators (key);

  LONG err;
  if (create)
  {
    err = RegCreateKeyEx (HKEY_CURRENT_USER,
      key, 0, 0, 0, 
      access, 
      0, &regKey, 0);
  }
  else
  {
    err = RegOpenKeyEx (HKEY_CURRENT_USER,
      key, 0, access, &regKey);
  }

  return true;
}

bool csRegistryConfig::Open (const char* Key)
{
  return TryOpen (hKey, KEY_READ, Key, false);
}

const char* csRegistryConfig::GetFileName () const
{
  return "Win32Registry";
}

iVFS* csRegistryConfig::GetVFS () const
{
  return 0;
}

void csRegistryConfig::SetFileName (const char*, iVFS*)
{
}

bool csRegistryConfig::Load (const char* iFileName, iVFS*, bool Merge,
    bool NewWins)
{
  return false;
}

bool csRegistryConfig::Save ()
{
  return false;
}

bool csRegistryConfig::Save (const char *iFileName, iVFS*)
{
  return false;
}

void csRegistryConfig::Clear ()
{
  int i;
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

csPtr<iConfigIterator> csRegistryConfig::Enumerate (const char *Subsection)
{
  if (!SubsectionExists (Subsection)) return 0;

  csRegistryIterator* it = new csRegistryIterator (this, Subsection);
  iters.Push (it);
  return it;
}

bool csRegistryConfig::KeyExists (const char *Key) const
{
  if (hKey == 0) return false;

  LONG err = RegQueryValueEx (hKey,
    Key, 0, 0, 0, 0);

  return (err == ERROR_SUCCESS);
}

bool csRegistryConfig::SubsectionExists (const char *Subsection) const
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

bool csRegistryConfig::InternalGetValue (const char* Key,
    DWORD& type, Block_O_Mem& data) const
{
  DWORD datasize;
  LONG err = RegQueryValueEx (hKey,
    Key, 0, &type, 0, &datasize);

  if (err == ERROR_SUCCESS)
  {
    data.SetSize (datasize);
    err = RegQueryValueEx (hKey,
      Key, 0, 0, data.data, &data.size);
  }

  return (err == ERROR_SUCCESS);
}

int csRegistryConfig::RegToInt (DWORD type, Block_O_Mem& data, int Def) const
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

float csRegistryConfig::RegToFloat (DWORD type, Block_O_Mem& data, float Def) const
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

const char* csRegistryConfig::RegToStr (DWORD type, Block_O_Mem& data,
					const char* Def) const
{
  char buf[64];
  switch (type)
  {
  case REG_SZ:
    return status->strings.Register ((char*)data.data, 0);
    break;
  case REG_DWORD:
    sprintf (buf, "%d", *((int*)data.data));
    return status->strings.Register (buf, 0);
    break;
  case REG_BINARY:
    sprintf (buf, "%g", *((float*)data.data));
    return status->strings.Register (buf, 0);
    break;
  default:
    return Def;
  }
}

bool csRegistryConfig::RegToBool (DWORD type, Block_O_Mem& data,
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

bool csRegistryConfig::WriteAccess()
{
  if (writeAccess) return true;

  RegCloseKey (hKey);
  if (!TryOpen (hKey, KEY_READ | KEY_WRITE, Key, true))
  {
    TryOpen (hKey, KEY_READ, Key, false);
    return false;
  }
  else
  {
    writeAccess = true;
    return true;
  }
}

int csRegistryConfig::GetInt (const char *Key, int Def) const
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

float csRegistryConfig::GetFloat (const char *Key, float Def) const
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

const char *csRegistryConfig::GetStr (const char *Key, const char *Def) const
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

bool csRegistryConfig::GetBool (const char *Key, bool Def) const
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

const char *csRegistryConfig::GetComment (const char *Key) const
{
  return 0;
}

bool csRegistryConfig::InternalSetValue (const char* Key,
    DWORD type, const void* data, DWORD datasize)
{
  if (!WriteAccess()) return false;

  LONG err = RegSetValueEx (hKey,
    Key, 0, type, (BYTE*)data, datasize);

  return (err == ERROR_SUCCESS);
}

void csRegistryConfig::SetStr (const char *Key, const char *Val)
{
  if (Val == 0)
    DeleteKey (Key);
  else
    InternalSetValue (Key, REG_SZ, Val, strlen (Val) + 1);
}

void csRegistryConfig::SetInt (const char *Key, int Value)
{
  InternalSetValue (Key, REG_DWORD, &Value, sizeof (Value));
}

void csRegistryConfig::SetFloat (const char *Key, float Value)
{
  //InternalSetValue (Key, REG_BINARY, &Value, sizeof (Value));
  // for better readability:
  char buf[128];
  sprintf (buf, "%g", Value);
  InternalSetValue (Key, REG_SZ, buf, strlen (buf) + 1);
}

void csRegistryConfig::SetBool (const char *Key, bool Value)
{
  int i = (Value ? 1 : 0);
  InternalSetValue (Key, REG_DWORD, &i, sizeof (i));
}

bool csRegistryConfig::SetComment (const char *Key, const char *Text)
{
  return false;
}

void csRegistryConfig::DeleteKey (const char *Key)
{
  if (!WriteAccess()) return;

  RegDeleteValue (hKey, Key);
}

const char *csRegistryConfig::GetEOFComment () const
{
  return 0;
}

void csRegistryConfig::SetEOFComment (const char *Text)
{
}

SCF_IMPLEMENT_IBASE (csRegistryIterator)
  SCF_IMPLEMENTS_INTERFACE (iConfigIterator)
SCF_IMPLEMENT_IBASE_END

csRegistryIterator::csRegistryIterator (csRegistryConfig* Owner, 
  const char* Subsection)
{
  SCF_CONSTRUCT_IBASE (0);

  status = new riStatus();

  owner = Owner;
  SubsectionName = new char[strlen(Subsection) + 1];
  strcpy (SubsectionName, Subsection);

  EnumIndex = 0;
}

csRegistryIterator::~csRegistryIterator()
{
  owner->iters.Delete (this);
  delete[] SubsectionName;
  delete status;
}

iConfigFile* csRegistryIterator::GetConfigFile () const
{
  return owner;
}

const char *csRegistryIterator::GetSubsection () const
{
  return SubsectionName;
}

void csRegistryIterator::Rewind ()
{
  EnumIndex = 0;
}

// navigate though the reg key to the next value entry
bool csRegistryIterator::Next()
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

const char *csRegistryIterator::GetKey (bool Local) const
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

bool csRegistryIterator::GetCurrentData (DWORD& type, 
  csRegistryConfig::Block_O_Mem& data) const
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

int csRegistryIterator::GetInt () const
{
  const int Def = 0;

  DWORD type;
  csRegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToInt (type, data, Def);
}

float csRegistryIterator::GetFloat () const
{
  const float Def = 0.0f;

  DWORD type;
  csRegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToFloat (type, data, Def);
}

const char *csRegistryIterator::GetStr () const
{
  const char* Def = "";

  DWORD type;
  csRegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToStr (type, data, Def);
}

bool csRegistryIterator::GetBool () const
{
  const bool Def = false;

  DWORD type;
  csRegistryConfig::Block_O_Mem data;

  if (!GetCurrentData (type, data))
    return Def;

  return owner->RegToBool (type, data, Def);
}

const char *csRegistryIterator::GetComment () const
{
  return 0;
}


