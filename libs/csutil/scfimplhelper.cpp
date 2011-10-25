/*
    Copyright (C) 2008 by Frank Richter

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

#include "csutil/scf_implementation.h"

scfImplementationHelper::~scfImplementationHelper()
{
  if (CS::Threading::AtomicOperations::Read ((void**)(void*)&scfAuxData) != 0)
    FreeAuxData();
}

iBase* scfImplementationHelper::GetSCFParent()
{
  bool hasAuxData (CS::Threading::AtomicOperations::Read ((void**)(void*)&scfAuxData) != 0);
  return hasAuxData ? scfAuxData->scfParent : 0;
}

void scfImplementationHelper::EnsureAuxData()
{
  if (CS::Threading::AtomicOperations::Read ((void**)(void*)&scfAuxData) != 0)
    return;
  ScfImplAuxData* newAuxData = new ScfImplAuxData;
  // Double-cast to cheat strict-aliasing rules
  if (CS::Threading::AtomicOperations::CompareAndSet ((void**)(void*)&scfAuxData,
      newAuxData, 0) != 0)
  {
    // A concurrent thread was faster with creating aux data
    delete newAuxData;
  }
}

void scfImplementationHelper::FreeAuxData()
{
  scfAuxData->DecRef ();
}

void scfImplementationHelper::AllocMetadata (size_t numEntries)
{
  CleanupMetadata ();

  uint8* ptr = (uint8*)cs_malloc (sizeof (scfInterfaceMetadataList) + 
                                  sizeof (scfInterfaceMetadata)*numEntries);

  scfInterfaceMetadataList* metadataList = (scfInterfaceMetadataList*)ptr;

  metadataList->metadata = (scfInterfaceMetadata*)(ptr + sizeof (scfInterfaceMetadataList));
  metadataList->metadataCount = numEntries;

  scfAuxData->metadataList = metadataList;
}

void scfImplementationHelper::CleanupMetadata ()
{
  CS_ASSERT(CS::Threading::AtomicOperations::Read ((void**)(void*)&scfAuxData) != 0);

  scfInterfaceMetadataList* metadataList = scfAuxData->metadataList;
  if (metadataList)
  {
    cs_free (metadataList);
    metadataList = 0;
  }
}

size_t scfImplementationHelper::GetInterfaceMetadataCount () const
{
  return 1; /* iBase is always there for you ... */
}
