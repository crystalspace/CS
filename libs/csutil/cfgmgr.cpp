/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/cfgfile.h"
#include "csutil/cfgmgr.h"
#include "csutil/strhash.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "csutil/sysfunc.h"

/* helper classes */

class csConfigDomain
{
public:
  csRef<iConfigFile> Cfg;
  int Pri;
  csConfigDomain *Prev, *Next;

  void InsertAfter(csConfigDomain *Where)
  {
    Next = Where->Next;
    Prev = Where;
    Where->Next = this;
    if (Next != 0)
      Next->Prev = this;
  }
  void InsertPriority(csConfigDomain *Where)
  {
    if (Where->Next == 0)
      InsertAfter(Where);
    else if (Pri < Where->Next->Pri)
      InsertAfter(Where);
    else
      InsertPriority(Where->Next);
  }
  void Remove()
  {
    if (Next != 0)
      Next->Prev = Prev;
    if (Prev != 0)
      Prev->Next = Next;
    Prev = Next = 0;
  }
  csConfigDomain(iConfigFile *c, int p)
  {
    Cfg = c;
    Pri = p;
    Prev = Next = 0;
  }
  ~csConfigDomain()
  {
    Remove();
  }
};

/* config iterator */

// Note: the iterator starts at the last (highest-priority) domain and loops
// back to the first one.

class csConfigManagerIterator : public iConfigIterator
{
private:
  csRef<csConfigManager> Config;
  csConfigDomain *CurrentDomain;
  csRef<iConfigIterator> CurrentIterator;
  char *Subsection;
  csStringHash Iterated;
public:
  SCF_DECLARE_IBASE;

  void ClearIterated()
  {
    Iterated.Clear();
  }
  bool FindIterated(const char *Key)
  {
    csString k(Key);
    k.Downcase();
    return Iterated.Contains(k);
  }
  void AddIterated(const char *Key)
  {
    csString k(Key);
    k.Downcase();
    Iterated.Register(k);
  }

  csConfigManagerIterator(csConfigManager *cfg, const char *sub)
  {
    SCF_CONSTRUCT_IBASE(0);
    Config = cfg;
    CurrentDomain = Config->LastDomain;
    Subsection = csStrNew(sub);
  }
  virtual ~csConfigManagerIterator()
  {
    Config->RemoveIterator(this);
    delete[] Subsection;
    ClearIterated();
    SCF_DESTRUCT_IBASE ();
  }
  virtual iConfigFile *GetConfigFile() const
  {
    return Config;
  }
  virtual const char *GetSubsection() const
  {
    return Subsection;
  }
  virtual void Rewind ()
  {
    CurrentIterator = 0;
    CurrentDomain = Config->LastDomain;
    ClearIterated();
  }
  virtual bool Next()
  {
    if (CurrentIterator)
    {
      if (CurrentIterator->Next())
      {
        if (FindIterated(CurrentIterator->GetKey()))
          return Next();
        AddIterated(CurrentIterator->GetKey());
        return true;
      }
      else
      {
        CurrentIterator = 0;
      }
    }
    // move to next domain (which actually means previous domain!)
    if (CurrentDomain->Prev == 0)
      return false;
    CurrentDomain = CurrentDomain->Prev;
    if (CurrentDomain->Cfg == 0)
      return false;
    CurrentIterator = CurrentDomain->Cfg->Enumerate(Subsection);
    return Next();
  }
  virtual const char *GetKey(bool Local) const
  {
    return (CurrentIterator ? CurrentIterator->GetKey(Local) : 0);
  }
  virtual int GetInt() const
  {
    return (CurrentIterator ? CurrentIterator->GetInt() : 0);
  }
  virtual float GetFloat() const
  {
    return (CurrentIterator ? CurrentIterator->GetFloat() : 0.0f);
  }
  virtual const char *GetStr() const
  {
    return (CurrentIterator ? CurrentIterator->GetStr() : "");
  }
  virtual bool GetBool() const
  {
    return (CurrentIterator ? CurrentIterator->GetBool() : false);
  }
  virtual const char *GetComment() const
  {
    return (CurrentIterator ? CurrentIterator->GetComment() : 0);
  }
};

SCF_IMPLEMENT_IBASE(csConfigManagerIterator);
  SCF_IMPLEMENTS_INTERFACE(iConfigIterator);
SCF_IMPLEMENT_IBASE_END;

/* config manager object */

SCF_IMPLEMENT_IBASE(csConfigManager);
  SCF_IMPLEMENTS_INTERFACE(iConfigManager);
  SCF_IMPLEMENTS_INTERFACE(iConfigFile);
SCF_IMPLEMENT_IBASE_END;

csConfigManager::csConfigManager(iConfigFile *dyn, bool opt)
{
  SCF_CONSTRUCT_IBASE(0);
  Optimize = opt;
  FirstDomain = new csConfigDomain(0, PriorityMin);
  LastDomain = new csConfigDomain(0, PriorityMax);
  LastDomain->InsertAfter(FirstDomain);

  csRef<iConfigFile> dyndom(dyn);
  if (!dyndom.IsValid())
    dyndom.AttachNew(new csConfigFile);
  AddDomain(dyndom, PriorityMedium);
  DynamicDomain = FindConfig(dyndom);
}

csConfigManager::~csConfigManager()
{
  // save our config.
  if (!Save())
    csPrintf("Error saving configuration '%s'.\n",
	    DynamicDomain->Cfg->GetFileName());
  CleanUp ();
  SCF_DESTRUCT_IBASE ();
}

void csConfigManager::CleanUp ()
{
  FlushRemoved();

  csConfigDomain *i, *Next;
  for (i=FirstDomain; i!=0; i=Next)
  {
    Next = i->Next;
    delete i;
  }
  // every iterator holds a reference to this object, so when this is
  // deleted there shouldn't be any iterators left.
  CS_ASSERT(Iterators.Length() == 0);
}

void csConfigManager::AddDomain(iConfigFile *Config, int Priority)
{
  if (Config)
  {
    csConfigDomain *d = new csConfigDomain(Config, Priority);
    d->InsertPriority(FirstDomain);
  }
}

iConfigFile* csConfigManager::AddDomain(
  char const* path, iVFS* vfs, int priority)
{
  if (Optimize)
  {
    csConfigDomain *d = FindConfig(path);
    if (d)
    {
      AddDomain(d->Cfg, priority);
      return d->Cfg;
    }

    size_t n = FindRemoved(path);
    if (n != (size_t)-1)
    {
      iConfigFile* cfg = Removed[n];
      AddDomain(cfg, priority);
      FlushRemoved(n);
      return cfg;
    }
  }

  csRef<iConfigFile> cfg = csPtr<iConfigFile>(new csConfigFile(path, vfs));
  AddDomain(cfg, priority);
  return cfg; // Safe since we still hold a reference.
}

void csConfigManager::RemoveDomain(iConfigFile *cfg)
{
  // prevent removal of dynamic domain
  if (cfg == 0 || cfg == DynamicDomain->Cfg)
    return;
  csConfigDomain *d = FindConfig(cfg);
  if (d != 0)
    RemoveDomain(d);
}

void csConfigManager::RemoveDomain(char const *path)
{
  RemoveDomain(FindConfig(path));
}

iConfigFile* csConfigManager::LookupDomain(char const *path) const
{
  csConfigDomain *d = FindConfig(path);
  if (d != 0)
    return d->Cfg;
  return 0;
}

void csConfigManager::SetDomainPriority(char const* path, int priority)
{
  csConfigDomain *d = FindConfig(path);
  if (d != 0)
  {
    d->Pri = priority;
    d->Remove();
    d->InsertPriority(FirstDomain);
  }
}

void csConfigManager::SetDomainPriority(iConfigFile *cfg, int priority)
{
  csConfigDomain *d = FindConfig(cfg);
  if (d != 0)
  {
    d->Pri = priority;
    d->Remove();
    d->InsertPriority(FirstDomain);
  }
}

int csConfigManager::GetDomainPriority(char const *path) const
{
  csConfigDomain *d = FindConfig(path);
  return d != 0 ? d->Pri : (int)PriorityMedium;
}

int csConfigManager::GetDomainPriority(iConfigFile *cfg) const
{
  csConfigDomain *d = FindConfig(cfg);
  return d != 0 ? d->Pri : (int)PriorityMedium;
}

bool csConfigManager::SetDynamicDomain(iConfigFile *cfg)
{
  csConfigDomain *d = FindConfig(cfg);
  if (!d)
    return false;
  DynamicDomain = d;
  return true;
}

iConfigFile *csConfigManager::GetDynamicDomain() const
{
  return DynamicDomain->Cfg;
}

void csConfigManager::SetDynamicDomainPriority(int priority)
{
  DynamicDomain->Pri = priority;
  DynamicDomain->Remove();
  DynamicDomain->InsertPriority(FirstDomain);
}

int csConfigManager::GetDynamicDomainPriority() const
{
  return DynamicDomain->Pri;
}

const char* csConfigManager::GetFileName () const
{
  return DynamicDomain->Cfg->GetFileName();
}

iVFS* csConfigManager::GetVFS () const
{
  return DynamicDomain->Cfg->GetVFS();
}

void csConfigManager::SetFileName (const char *fn, iVFS *vfs)
{
  DynamicDomain->Cfg->SetFileName(fn, vfs);
}

bool csConfigManager::Load (const char* iFileName, iVFS *vfs, bool Merge,
        bool NewWins)
{
  return DynamicDomain->Cfg->Load(iFileName, vfs, Merge, NewWins);
}

bool csConfigManager::Save ()
{
  return DynamicDomain->Cfg->Save();
}

bool csConfigManager::Save (const char *iFileName, iVFS *vfs)
{
  return DynamicDomain->Cfg->Save(iFileName, vfs);
}

void csConfigManager::Clear()
{
  for (csConfigDomain *d=DynamicDomain; d!=0; d=d->Next)
    if (d->Cfg)
      d->Cfg->Clear();
}

csPtr<iConfigIterator> csConfigManager::Enumerate(const char *Subsection)
{
  iConfigIterator *it = new csConfigManagerIterator(this, Subsection);
  Iterators.Push(it);
  return csPtr<iConfigIterator> (it);
}

bool csConfigManager::KeyExists(const char *Key) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return true;
  return false;
}

bool csConfigManager::SubsectionExists(const char *Subsection) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev)
    if (d->Cfg && d->Cfg->SubsectionExists(Subsection))
      return true;
  return false;
}

int csConfigManager::GetInt(const char *Key, int Def) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetInt(Key, Def);
  return Def;
}

float csConfigManager::GetFloat(const char *Key, float Def) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetFloat(Key, Def);
  return Def;
}

const char *csConfigManager::GetStr(const char *Key, const char *Def) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetStr(Key, Def);
  return Def;
}

bool csConfigManager::GetBool(const char *Key, bool Def) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetBool(Key, Def);
  return Def;
}

const char *csConfigManager::GetComment(const char *Key) const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev) {
    const char *c = d->Cfg ? d->Cfg->GetComment(Key) : 0;
    if (c != 0)
      return c;
  }
  return 0;
}

void csConfigManager::SetStr (const char *Key, const char *Value)
{
  DynamicDomain->Cfg->SetStr(Key, Value);
  ClearKeyAboveDynamic(Key);
}

void csConfigManager::SetInt (const char *Key, int Value)
{
  DynamicDomain->Cfg->SetInt(Key, Value);
  ClearKeyAboveDynamic(Key);
}

void csConfigManager::SetFloat (const char *Key, float Value)
{
  DynamicDomain->Cfg->SetFloat(Key, Value);
  ClearKeyAboveDynamic(Key);
}

void csConfigManager::SetBool (const char *Key, bool Value)
{
  DynamicDomain->Cfg->SetBool(Key, Value);
  ClearKeyAboveDynamic(Key);
}

bool csConfigManager::SetComment (const char *Key, const char *Text)
{
  if (!DynamicDomain->Cfg->SetComment(Key, Text)) return false;
  for (csConfigDomain *d=DynamicDomain->Next; d!=0; d=d->Next)
    if (d->Cfg)
      d->Cfg->SetComment(Key, 0);
  return true;
}

void csConfigManager::DeleteKey(const char *Key)
{
  DynamicDomain->Cfg->DeleteKey(Key);
  ClearKeyAboveDynamic(Key);
}

const char *csConfigManager::GetEOFComment() const
{
  for (csConfigDomain *d=LastDomain; d!=0; d=d->Prev) {
    const char *c = d->Cfg ? d->Cfg->GetEOFComment() : 0;
    if (c != 0)
      return c;
  }
  return 0;
}

void csConfigManager::SetEOFComment(const char *Text)
{
  DynamicDomain->Cfg->SetEOFComment(Text);
  for (csConfigDomain *d=DynamicDomain->Next; d!=0; d=d->Next)
    if (d->Cfg)
      d->Cfg->SetEOFComment(0);
}

void csConfigManager::ClearKeyAboveDynamic(const char *Key)
{
  for (csConfigDomain *d=DynamicDomain->Next; d!=0; d=d->Next)
    if (d->Cfg)
      d->Cfg->DeleteKey(Key);
}

csConfigDomain *csConfigManager::FindConfig(iConfigFile *cfg) const
{
  if (!cfg)
    return 0;
  for (csConfigDomain *d=FirstDomain; d!=0; d=d->Next)
    if (d->Cfg == cfg)
      return d;
  return 0;
}

csConfigDomain *csConfigManager::FindConfig(const char *Name) const
{
  for (csConfigDomain *d=FirstDomain; d!=0; d=d->Next) {
    if (d->Cfg && d->Cfg->GetFileName() &&
        strcmp(d->Cfg->GetFileName(), Name)==0)
      return d;
  }
  return 0;
}

void csConfigManager::RemoveIterator(csConfigManagerIterator *it)
{
  size_t n = Iterators.Find(it);
  CS_ASSERT(n != (size_t)-1);
  Iterators.DeleteIndex(n);
}

void csConfigManager::FlushRemoved()
{
  Removed.DeleteAll();
}

void csConfigManager::FlushRemoved(size_t n)
{
  Removed.DeleteIndex (n);
}

size_t csConfigManager::FindRemoved(const char *Name) const
{
  for (size_t i=0; i<Removed.Length(); i++)
  {
    iConfigFile *cfg = Removed[i];
    if (cfg->GetFileName())
      if (strcmp(cfg->GetFileName(), Name)==0)
        return i;
  }
  return (size_t)-1;
}

void csConfigManager::RemoveDomain(csConfigDomain *d)
{
  d->Remove();
  if (Optimize && d->Cfg && d->Cfg->GetFileName()!=0 && FindConfig(d->Cfg)==0)
  {
    Removed.Push(d->Cfg);
  }
  delete d;
}
