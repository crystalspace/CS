/*
    Load plugin meta data from object file headers.
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Eric Sunshine
	      (C) 2003 by Frank Richter
	      (C) 2003 by Mat Sutcliffe

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
#include "csutil/csendian.h"
#include "csutil/mmapio.h"
#include "csgeom/math.h"

/* Note: The section name must be of the same value as SECTION_TAG_NAME 
    in unix.jam */
#define SECTION_TAG_NAME      ".crystalspace"

#include <elf.h>

struct EndiannessBig
{
  static uint32 ConvertOff (Elf32_Off u) { return csBigEndianLong ((uint32)u); }
  static uint64 ConvertOff (Elf64_Off u) { return csBigEndianLongLong ((uint64)u); }
  static uint32 ConvertSize (uint32_t u) { return csBigEndianLong ((uint32)u); }
  static uint64 ConvertSize (uint64_t u) { return csBigEndianLongLong ((uint64)u); }
  static uint16 ConvertUI16 (uint16 u) { return csBigEndianShort (u); }
  static uint32 ConvertUI32 (uint32 u) { return csBigEndianLong (u); }
};
struct EndiannessLittle
{
  static uint32 ConvertOff (Elf32_Off u) { return csLittleEndianLong ((uint32)u); }
  static uint64 ConvertOff (Elf64_Off u) { return csLittleEndianLongLong ((uint64)u); }
  static uint32 ConvertSize (uint32_t u) { return csLittleEndianLong ((uint32)u); }
  static uint64 ConvertSize (uint64_t u) { return csLittleEndianLongLong ((uint64)u); }
  static uint16 ConvertUI16 (uint16 u) { return csLittleEndianShort (u); }
  static uint32 ConvertUI32 (uint32 u) { return csLittleEndianLong (u); }
};

template <typename Endianness, typename Ehdr, typename Shdr>
struct ElfReader
{
  static char* GetMetadata (csMemoryMappedIO* mmio, Ehdr* hdr, 
                            const char*& errMsg)
  {
    uint16 sectNum = Endianness::ConvertUI16 (hdr->e_shnum);
    uint16 sectEntSize = Endianness::ConvertUI16 (hdr->e_shentsize);
    if ((hdr->e_shoff == 0) || (sectNum == 0) || (sectEntSize == 0))
    {
      errMsg = "No ELF section table";
      return 0;
    }
    uint16 strTabIndex = Endianness::ConvertUI16 (hdr->e_shstrndx);
    if (strTabIndex == SHN_UNDEF)
    {
      errMsg = "No section name string table";
      return 0;
    }
    csRef<csMemoryMapping> sectHeader = mmio->GetData (
      Endianness::ConvertOff (hdr->e_shoff), sectEntSize * sectNum);
    if (!sectHeader.IsValid())
    {
      errMsg = "Could not map ELF section header";
      return 0;
    }
    uint8* sectTab = (uint8*)sectHeader->GetData();
    Shdr* shStrings = (Shdr*)(sectTab + strTabIndex*sectEntSize);
    csRef<csMemoryMapping> stringTab = mmio->GetData (
      Endianness::ConvertOff (shStrings->sh_offset), 
      Endianness::ConvertSize (shStrings->sh_size));
    if (!stringTab.IsValid())
    {
      errMsg = "Could not map ELF section name string table";
      return 0;
    }
    
    for (uint i = 0; i < sectNum; i++)
    {
      Shdr* shdr = (Shdr*)(sectTab + i*sectEntSize);
      const char* name = ((const char*)stringTab->GetData()) +
        Endianness::ConvertUI32 (shdr->sh_name);
      if (strcmp (name, SECTION_TAG_NAME) == 0)
      {
        size_t size = Endianness::ConvertSize (shdr->sh_size);
        char* buf = new char[size+1];
        csRef<csMemoryMapping> metadata = mmio->GetData (
          Endianness::ConvertOff (shdr->sh_offset), size);
        if (!metadata.IsValid())
        {
          errMsg = "Could not map " SECTION_TAG_NAME " section";
          return 0;
        }
        memcpy (buf, metadata->GetData(), size);
        buf[size] = 0;
        return buf;
      }
    }
    // Not an error since not all .sos are CS plugins.
    //errMsg = "Could not locate " SECTION_TAG_NAME " section";
    return 0;
  }
};

char* csExtractMetadata (const char* fullPath, const char*& errMsg)
{
  csRef<csMemoryMappedIO> mmio;
  mmio.AttachNew (new csMemoryMappedIO (fullPath));
  if (!mmio->IsValid())
  {
    errMsg = "Could not map file into memory";
    return 0;
  }
  
  csRef<csMemoryMapping> elfHeader = mmio->GetData (0, 
    csMax (sizeof (Elf32_Ehdr), sizeof (Elf64_Ehdr)));
  if (!elfHeader.IsValid())
  {
    errMsg = "Could not get ELF header";
    return 0;
  }
  Elf32_Ehdr* hdr = (Elf32_Ehdr*)elfHeader->GetData ();
  if ((hdr->e_ident[EI_MAG0] != ELFMAG0)
    || (hdr->e_ident[EI_MAG1] != ELFMAG1)
    || (hdr->e_ident[EI_MAG2] != ELFMAG2)
    || (hdr->e_ident[EI_MAG3] != ELFMAG3))
  {
    errMsg = "Not an ELF file";
    return 0;
  }
  if (hdr->e_ident[EI_CLASS] == ELFCLASS32)
  {
    if (hdr->e_ident[EI_DATA] == ELFDATA2LSB)
    {
      return ElfReader<EndiannessLittle, Elf32_Ehdr, Elf32_Shdr>::
        GetMetadata (mmio, (Elf32_Ehdr*)hdr, errMsg);
    }
    else if (hdr->e_ident[EI_DATA] == ELFDATA2MSB)
    {
      return ElfReader<EndiannessBig, Elf32_Ehdr, Elf32_Shdr>::
        GetMetadata (mmio, (Elf32_Ehdr*)hdr, errMsg);
    }
    else
      errMsg = "Invalid EI_DATA";
  }
  else if (hdr->e_ident[EI_CLASS] == ELFCLASS64)
  {
    if (hdr->e_ident[EI_DATA] == ELFDATA2LSB)
    {
      return ElfReader<EndiannessLittle, Elf64_Ehdr, Elf64_Shdr>::
        GetMetadata (mmio, (Elf64_Ehdr*)hdr, errMsg);
    }
    else if (hdr->e_ident[EI_DATA] == ELFDATA2MSB)
    {
      return ElfReader<EndiannessBig, Elf64_Ehdr, Elf64_Shdr>::
        GetMetadata (mmio, (Elf64_Ehdr*)hdr, errMsg);
    }
    else
      errMsg = "Invalid EI_DATA";
  }
  else
    errMsg = "Invalid EI_CLASS";
  return 0;
}

