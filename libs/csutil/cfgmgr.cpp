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
#include "csutil/cfgmgr.h"
#include "csutil/cfgfile.h"
#include "csutil/util.h"

/* helper classes */

class csConfigDomain {
public:
  iConfigFileNew *Cfg;
  int Pri;
  csConfigDomain *Prev, *Next;

  void InsertAfter(csConfigDomain *Where) {
    Next = Where->Next;
    Prev = Where;
    Where->Next = this;
    if (Next) Next->Prev = this;
  }
  void InsertPriority(csConfigDomain *Where) {
    if (!Where->Next)
      InsertAfter(Where);
    else if (Pri < Where->Next->Pri)
      InsertAfter(Where);
    else
      InsertPriority(Where->Next);
  }
  void Remove() {
    if (Next) Next->Prev = Prev;
    if (Prev) Prev->Next = Next;
    Prev = Next = NULL;
  }
  csConfigDomain(iConfigFileNew *c, int p) {
    Cfg = c;
    if (Cfg) Cfg->IncRef();
    Pri = p;
    Prev = Next = NULL;
  }
  ~csConfigDomain() {
    Remove();
    if (Cfg) Cfg->DecRef();
  }
};

/* config iterator */

// Note: the iterator starts at the last (highest-priority) domain and loops
// back to the first one.

class csConfigManagerIterator : public iConfigIterator
{
private:
  csConfigManager *Config;
  csConfigDomain *CurrentDomain;
  iConfigIterator *CurrentIterator;
  char *Subsection;
  csVector Iterated;
public:
  DECLARE_IBASE;

  void ClearIterated() {
    while (Iterated.Length()>0) {
      char *n = (char*)Iterated.Pop();
      delete[] n;
    }
  }

  csConfigManagerIterator(csConfigManager *cfg, const char *sub) {
    CONSTRUCT_IBASE(NULL);
    Config = cfg;
    Config->IncRef();
    CurrentDomain = Config->LastDomain;
    CurrentIterator = NULL;
    Subsection = strnew(sub);
  }
  virtual ~csConfigManagerIterator() {
    Config->RemoveIterator(this);
    Config->DecRef();
    if (CurrentIterator) CurrentIterator->DecRef();
    if (Subsection) delete[] Subsection;
    ClearIterated();
  }
  virtual iConfigFileNew *GetConfigFile() const {
    return Config;
  }
  virtual const char *GetSubsection() const {
    return Subsection;
  }
  virtual void Rewind () {
    if (CurrentIterator) delete CurrentIterator;
    CurrentIterator = NULL;
    CurrentDomain = Config->LastDomain;
    ClearIterated();
  }
  virtual bool Next() {
    if (CurrentIterator) {
      if (CurrentIterator->Next()) {
        // look if key was already iterated
        for (long i=0; i<Iterated.Length(); i++) {
          if (strcmp((char*)Iterated.Get(i), CurrentIterator->GetKey()) == 0)
            return Next();
        }
        Iterated.Push(strnew(CurrentIterator->GetKey()));
        return true;
      } else {
        CurrentIterator->DecRef();
        CurrentIterator = NULL;
      }
    }
    // move to next domain (which actually means previous domain!)
    if (CurrentDomain->Prev == NULL) return false;
    CurrentDomain = CurrentDomain->Prev;
    if (CurrentDomain->Cfg == NULL) return false;
    CurrentIterator = CurrentDomain->Cfg->Enumerate(Subsection);
    return Next();
  }
  virtual const char *GetKey(bool Local) const {
    return (CurrentIterator ? CurrentIterator->GetKey(Local) : NULL);
  }
  virtual int GetInt() const {
    return (CurrentIterator ? CurrentIterator->GetInt() : 0);
  }
  virtual float GetFloat() const {
    return (CurrentIterator ? CurrentIterator->GetFloat() : 0.0);
  }
  virtual const char *GetStr() const {
    return (CurrentIterator ? CurrentIterator->GetStr() : "");
  }
  virtual bool GetBool() const {
    return (CurrentIterator ? CurrentIterator->GetBool() : false);
  }
  virtual const char *GetComment() const {
    return (CurrentIterator ? CurrentIterator->GetComment() : NULL);
  }
};

IMPLEMENT_IBASE(csConfigManagerIterator);
  IMPLEMENTS_INTERFACE(iConfigIterator);
IMPLEMENT_IBASE_END;

/* config manager object */

IMPLEMENT_IBASE(csConfigManager);
  IMPLEMENTS_INTERFACE(iConfigManager);
  IMPLEMENTS_INTERFACE(iConfigFileNew);
IMPLEMENT_IBASE_END;

csConfigManager::csConfigManager(iConfigFileNew *dyn)
{
  CONSTRUCT_IBASE(NULL);
  FirstDomain = new csConfigDomain(NULL, PriorityMin);
  LastDomain = new csConfigDomain(NULL, PriorityMax);
  LastDomain->InsertAfter(FirstDomain);
  AddDomain(dyn, PriorityMedium);
  DynamicDomain = FindConfig(dyn);
}

csConfigManager::~csConfigManager()
{
  csConfigDomain *i, *Next;
  for (i=FirstDomain; i!=NULL; i=Next) {
    Next = i->Next;
    delete i;
  }
  // every iterator holds a reference to this object, so when this is
  // deleted there shouldn't be any iterators left.
  CS_ASSERT(Iterators.Length() == 0);
}

void csConfigManager::AddDomain(iConfigFileNew *Config, int Priority)
{
  if (Config) {
    csConfigDomain *d = new csConfigDomain(Config, Priority);
    d->InsertPriority(FirstDomain);
  }
}

void csConfigManager::AddDomain(char const* path, iVFS* vfs, int priority)
{
  AddDomain(new csConfigFile(path, vfs), priority);
}

void csConfigManager::RemoveDomain(iConfigFileNew *cfg)
{
  // prevent removal of dynamic domain
  if (cfg == DynamicDomain->Cfg) return;
  csConfigDomain *d = FindConfig(cfg);
  if (d) delete d;
}

void csConfigManager::RemoveDomain(char const *path, iVFS *vfs)
{
  csConfigDomain *d = FindConfig(path, vfs);
  // prevent removal of dynamic domain
  if (d == NULL || d == DynamicDomain) return;
  delete d;
}

iConfigFileNew* csConfigManager::LookupDomain(char const *path, iVFS *vfs) const
{
  csConfigDomain *d = FindConfig(path, vfs);
  return d ? d->Cfg : NULL;
}

void csConfigManager::SetDomainPriority(char const* path, iVFS *vfs, int priority)
{
  csConfigDomain *d = FindConfig(path, vfs);
  if (d) {
    d->Pri = priority;
    d->Remove();
    d->InsertPriority(FirstDomain);
  }
}

void csConfigManager::SetDomainPriority(iConfigFileNew *cfg, int priority)
{
  csConfigDomain *d = FindConfig(cfg);
  if (d) {
    d->Pri = priority;
    d->Remove();
    d->InsertPriority(FirstDomain);
  }
}

int csConfigManager::GetDomainPriority(char const *path, iVFS *vfs) const
{
  csConfigDomain *d = FindConfig(path, vfs);
  return d ? d->Pri : (int)PriorityMedium;
}

int csConfigManager::GetDomainPriority(iConfigFileNew *cfg) const
{
  csConfigDomain *d = FindConfig(cfg);
  return d ? d->Pri : (int)PriorityMedium;
}

iConfigFileNew *csConfigManager::GetDynamicDomain() const
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
  for (csConfigDomain *d=DynamicDomain; d!=NULL; d=d->Next)
    if (d->Cfg) d->Cfg->Clear();
}

iConfigIterator *csConfigManager::Enumerate(const char *Subsection)
{
  iConfigIterator *it = new csConfigManagerIterator(this, Subsection);
  Iterators.Push(it);
  return it;
}

bool csConfigManager::KeyExists(const char *Key) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key)) return true;
  return false;
}

bool csConfigManager::SubsectionExists(const char *Subsection) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev)
    if (d->Cfg && d->Cfg->SubsectionExists(Subsection)) return true;
  return false;
}

int csConfigManager::GetInt(const char *Key, int Def) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetInt(Key, Def);
  return Def;
}

float csConfigManager::GetFloat(const char *Key, float Def) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetFloat(Key, Def);
  return Def;
}

const char *csConfigManager::GetStr(const char *Key, const char *Def) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetStr(Key, Def);
  return Def;
}

bool csConfigManager::GetBool(const char *Key, bool Def) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev)
    if (d->Cfg && d->Cfg->KeyExists(Key))
      return d->Cfg->GetBool(Key, Def);
  return Def;
}

const char *csConfigManager::GetComment(const char *Key) const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev) {
    const char *c = d->Cfg ? d->Cfg->GetComment(Key) : NULL;
    if (c) return c;
  }
  return NULL;
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
  for (csConfigDomain *d=DynamicDomain->Next; d!=NULL; d=d->Next)
    if (d->Cfg) d->Cfg->SetComment(Key, NULL);
  return true;
}

void csConfigManager::DeleteKey(const char *Key)
{
  DynamicDomain->Cfg->DeleteKey(Key);
  ClearKeyAboveDynamic(Key);
}

const char *csConfigManager::GetEOFComment() const
{
  for (csConfigDomain *d=LastDomain; d!=NULL; d=d->Prev) {
    const char *c = d->Cfg ? d->Cfg->GetEOFComment() : NULL;
    if (c) return c;
  }
  return NULL;
}

void csConfigManager::SetEOFComment(const char *Text)
{
  DynamicDomain->Cfg->SetEOFComment(Text);
  for (csConfigDomain *d=DynamicDomain->Next; d!=NULL; d=d->Next)
    if (d->Cfg) d->Cfg->SetEOFComment(NULL);
}

void csConfigManager::ClearKeyAboveDynamic(const char *Key)
{
  for (csConfigDomain *d=DynamicDomain->Next; d!=NULL; d=d->Next)
    if (d->Cfg) d->Cfg->DeleteKey(Key);
}

csConfigDomain *csConfigManager::FindConfig(iConfigFileNew *cfg) const
{
  if (!cfg) return NULL;
  for (csConfigDomain *d=FirstDomain; d!=NULL; d=d->Next)
    if (d->Cfg == cfg) return d;
  return NULL;
}

csConfigDomain *csConfigManager::FindConfig(const char *Name, iVFS *vfs) const
{
  for (csConfigDomain *d=FirstDomain; d!=NULL; d=d->Next) {
    if (d->Cfg &&
        strcmp(d->Cfg->GetFileName(), Name)==0 &&
        d->Cfg->GetVFS() == vfs)
      return d;
  }
  return NULL;
}

void csConfigManager::RemoveIterator(csConfigManagerIterator *it)
{
  int n = Iterators.Find(it);
  CS_ASSERT(n != -1);
  Iterators.Delete(n);
}
