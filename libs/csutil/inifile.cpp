/*
    Crystal Space .INI file management
    Copyright (C) 1998,1999,2000 by Andrew Zabolotny <bit@eltech.ru>
    Extensive functional overhaul by Eric Sunshine <sunshine@sunshineco.com>
    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#include <string.h>
#include <stdlib.h>

#include "cssysdef.h"
#include "cstypes.h"
#include "csutil/inifile.h"
#include "csutil/csstrvec.h"
#include "csutil/util.h"
#include "istrvec.h"

// Maximal INI line length
#define CS_MAXINILINELEN 1024
// Maximal line length for lines containing BASE64 encoded data
#define CS_B64INILINELEN 76
// Characters ignored in INI files (except in middle of section & key names)
#define CS_INISPACE " \t"
// Use 8-bit characters in INI files
#define CS_8BITCFGFILES

static const char* const INIbase64 =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

csIniFile::PrvINIbranch::~PrvINIbranch ()
{
  DeleteAll ();
}

bool csIniFile::PrvINIbranch::FreeItem (csSome Item)
{
  if (Item)
  {
    switch (((PrvINInode *)Item)->Type)
    {
      case TYPE_SECTION:
        delete (((PrvINInode *)Item)->Section.Vector);
        free (((PrvINInode *)Item)->Section.Name);
        break;
      case TYPE_DATA:
        free (((PrvINInode *)Item)->Data.Pointer);
        free (((PrvINInode *)Item)->Data.Name);
        break;
      case TYPE_COMMENT:
        free (((PrvINInode *)Item)->Comment.Text);
        break;
    }
    if (((PrvINInode *)Item)->Comments)
      delete ((PrvINInode *)Item)->Comments;
    delete (PrvINInode *)Item;
  }
  return true;
}

//--------------------------------------------------------------- Iterators ---

bool csIniFile::Iterator::NextItem()
{
  if (Current < Limit)
    for (Current++; Current < Limit; Current++)
    {
      Node = (PrvINInode*)Branch->Get(Current);
      if (Type == Node->Type)
        return true;
    }
  Node = NULL;
  return false;
}

void csIniFile::Iterator::Clone(const Iterator& i)
{
  if (this != &i)
  {
    Branch = i.Branch;
    Type = i.Type;
    Node = NULL;
    Current = -1;
    Limit = (Branch ? Branch->Length() : 0);
  }
}

void csIniFile::Iterator::RemoveItem()
{
  if (Current >= 0 && Current < Limit)
  {
    (CONST_CAST(PrvINIbranch*)(Branch))->Delete(Current);
    Node = NULL;
    Limit--;
  }
}

csIniFile::DataIterator::~DataIterator() { free (Section); }
csIniFile::DataIterator::DataIterator(const csIniFile& s,
  const PrvINIbranch* b, const char* sec) : Iterator(s, b, TYPE_DATA)
{
  Section = strdup (sec);
}
void csIniFile::DataIterator::Clone(const DataIterator& i)
{
  if (this != &i)
  {
    if (Section != NULL) free (Section);
    Section = strdup (i.Section);
  }
}

csIniFile::CommentIterator::~CommentIterator() { free (Section); free (Key); }
csIniFile::CommentIterator::CommentIterator(const csIniFile& s,
  const PrvINIbranch* b, const char* SectionPath, const char* KeyName) :
  Iterator(s, b, TYPE_COMMENT)
{
  Section = strdup (SectionPath);
  Key = strdup (KeyName);
}
void csIniFile::CommentIterator::Clone(const CommentIterator& i)
{
  if (this != &i)
  {
    if (Section != NULL) free (Section);
    if (Key != NULL) free (Key);
    Section = strdup (i.Section);
    Key = strdup (i.Key);
  }
}

//------------------------------------------------------------ Constructors ---

csIniFile::csIniFile (const char* path, char Comment) :
  CommentChar(Comment), Dirty(false) { Load (path); }

csIniFile::csIniFile (iVFS *vfs, const char* path, char Comment) :
  CommentChar(Comment), Dirty(false) { Load (vfs, path); }

csIniFile::csIniFile (iFile* f, char Comment) :
  CommentChar(Comment), Dirty(false) { Load (f); }

csIniFile::~csIniFile () {}

//--------------------------------------------------------- csIniFile::Load ---

static bool NextLine (csString& line, const char*& source, const char* limit)
{
  line.Clear();
  if (source >= limit) return false;
  const char* const start = source;
  while (source < limit && *source != '\n' && *source != '\r') source++;
  line.Append (start, source - start);
  if (source < limit)
  {  // Strip line terminator (handles DOS:CRLF, Unix:LF, Macintosh:CR)
    source++;   // Strip initial LF or CR
    if (*(source - 1) == '\r' && source < limit && *source == '\n')
      source++; // Strip CRLF.
  }
  return true;
}

bool csIniFile::Load (const char* path)
{
  bool rc = false;
  FILE* f = fopen (path, "rb");
  if (f)
  {
    size_t size = 0;
    if (fseek (f, 0, SEEK_END) == 0 && (size = ftell(f)) != size_t(-1) &&
        fseek (f, 0, SEEK_SET) == 0)
    {
      if (size == 0)
        rc = true; // Empty file, but not an error.
      else
      {
        char* data = new char[size];
        if (fread (data, size, 1, f) == 1)
          rc = Load (data, size);
        delete[] data;
      }
    }
    fclose (f);
  }
  return rc;
}

bool csIniFile::Load (const csString& s)
{
  return Load (s.GetData(), s.Length());
}

bool csIniFile::Load (iVFS* vfs, const char* path)
{
  bool rc = false;
  if (vfs)
  {
    iFile* file = vfs->Open (path, VFS_FILE_READ);
    if (file)
    {
      rc = Load (file);
      file->DecRef();
    }
  }
  return rc;
}

bool csIniFile::Load (iFile* file)
{
  bool rc = false;
  if (file)
  {
    const size_t size = file->GetSize();
    if (size == 0)
      rc = true; // Empty file, but not an error.
    else
    {
      char* data = new char[size];
      if (file->Read (data, size) == size)
        rc = Load (data, size);
      delete[] data;
    }
  }
  return rc;
}

bool csIniFile::Load (const char *Data, size_t DataSize)
{
  csString record;
  const char* const DataEnd = Data + DataSize;
  char tmp[CS_MAXINILINELEN];
  int LineNo = 0;
  PrvINInode *branch;
  PrvINIbranch *CurBranch = NULL;
  PrvINIbranch *Comments = NULL;
  bool SkipComment = false;
  bool PSkipComment = false;

  // Assume object contents is syncronised
  Dirty = false;

  // Base64 decoding variables
  bool b64mode = false;
  bool b64init = true;
  int b64top = 0;
  unsigned char b64acc = 0;

  for (;;)
  {
    if (!NextLine (record, Data, DataEnd))
      break;
    LineNo++;
    const char* buff = record;
    const char* cur = buff + strspn (buff, CS_INISPACE);

    if (b64mode)                        // Base64 mode
    {
      int i, len;
      char const *s;
      bool finish = false;

      // Check for end-of-base64-sequence character(s) -- '='
      for (i = strlen (cur); i > 0; i--)
        if (cur[i - 1] == '=')
          finish = true;
        else
          break;
      record.SetAt (cur - buff + i, '\0');
      len = i;

      // Check if string is a valid BASE64 string
      for (i = 0; i < len; i++)
        if (strchr (INIbase64, cur[i]) == NULL)
          goto plain;

      if (!CurBranch)
        goto error;

      if (b64init)
      {
        b64top = 0;
        b64acc = 0;
        b64init = false;
      }
      memset (tmp, 0, sizeof (tmp));
      tmp[0] = b64acc;

      while (*cur)
      {
        s = strchr (INIbase64, *cur);   // We already checked it above
        i = int (s - INIbase64) << 2;
        tmp[b64top / 8 + 0] |= i >> (b64top & 7);
        tmp[b64top / 8 + 1] |= (i << (8 - (b64top & 7))) & 0xff;
        b64top += 6;
        cur++;
      }
      len = b64top / 8;
      b64top &= 7;
      b64acc = tmp[len];
      branch = (PrvINInode *)((*CurBranch)[CurBranch->Length () - 1]);
      i = branch->Data.Size;
      branch->Data.Size += len;
      branch->Data.Pointer =
        realloc (branch->Data.Pointer, branch->Data.Size + 1);
      memcpy (((char *)branch->Data.Pointer) + i, tmp, len);
      ((char *) branch->Data.Pointer)[branch->Data.Size] = 0;

      if (finish)
      {
        b64init = true;
        b64mode = false;
      }
    }
    else
    {
plain:
      PSkipComment = SkipComment;
      SkipComment = false;

      // end-of-line ?
      if ((*cur == 0) || (*cur == CommentChar))
      {
        if (PSkipComment && CurBranch)
        {
          branch = (PrvINInode *) ((*CurBranch)[CurBranch->Length () - 1]);
          if ((branch->Type == TYPE_DATA) && strstr (cur, branch->Data.Name))
            continue;
        }
        branch = new PrvINInode;
        branch->Type = TYPE_COMMENT;
        branch->Comments = NULL;
        if (*cur)
          branch->Comment.Text = strdup (cur + 1);
        else
          branch->Comment.Text = NULL;
        if (!Comments)
          Comments = new PrvINIbranch;
        Comments->Push (branch);
      }
      else if (*cur == '[')           // Section
      {
        const char *cb;
        int i;

        cur++;
        cur += strspn (cur, CS_INISPACE);
        cb = strchr (cur, ']');
        if (!cb)
          goto error;

        strncpy (tmp, cur, i = int (cb - cur));
        while (i && strchr (CS_INISPACE, tmp[i - 1]))
          i--;
        tmp[i] = 0;

        branch = new PrvINInode;
        branch->Type = TYPE_SECTION;
        branch->Comments = Comments;
        branch->Section.Name = strdup (tmp);
        branch->Section.Vector = CurBranch = new PrvINIbranch ();
        Root.Push (branch);
        Comments = NULL;
      }                                 // else if Section
      else
      {                                 // Key = Value
        const char *eq;
        int i;

        eq = strchr (cur, '=');
        if (!eq || !CurBranch)
        {
error:    if (Error (LineNo, buff, int (cur - buff)))
            goto out;
          else
            continue;
        }
        strncpy (tmp, cur, i = int (eq - cur));
        while (i && strchr (CS_INISPACE, tmp[i - 1]))
          i--;
        tmp[i] = 0;

        branch = new PrvINInode;
        branch->Type = TYPE_DATA;
        branch->Comments = Comments;
        branch->Data.Name = strdup (tmp);

        cur = eq + 1;
        cur += strspn (cur, CS_INISPACE);

        strcpy (tmp, cur);
        i = strlen (cur);
        if (i)
        {
          while (i && strchr (CS_INISPACE, tmp[i - 1]))
            i--;
          tmp[i] = 0;
          branch->Data.Pointer = malloc ((branch->Data.Size = i) + 1);
          strcpy ((char *) branch->Data.Pointer, tmp);
        }
        else
        {
          branch->Data.Size = 0;
          branch->Data.Pointer = NULL;
          b64mode = true;
          SkipComment = true;
        }
        CurBranch->Push (branch);
        Comments = NULL;
      } // Key = Value
    } // else if (b64mode)
  } // for (;;)

out:
  if (Comments)
  {
    while (Comments->Length ())
      Root.Push (Comments->Pop ());
    delete Comments;
//    Comments = NULL;    // not needed but left for clarity
  }
  return true;
}

//--------------------------------------------------------- csIniFile::Save ---

void csIniFile::SaveComment (const char* Text, csString& s) const
{
  if (Text)
    s << CommentChar << Text;
  s << '\n';
}

void csIniFile::SaveComments (const PrvINIbranch* branch, csString& s) const
{
  CommentIterator iterator (*this, branch, "", "");
  while (iterator.NextItem())
    SaveComment (iterator.GetText(), s);
}

void csIniFile::SaveData (const char* Name, csSome Data, size_t DataSize,
  const PrvINIbranch* comments, csString& s) const
{
  const char* data = (const char*)Data;
  SaveComments (comments, s);
  s << Name << " = ";
  if (Data && DataSize)
  {
    size_t i;
    bool binary = false;

    if ((data [0] <= ' ') || (data [DataSize - 1] <= ' '))
      binary = true;
    else
      for (i = 0; i < DataSize; i++)
        if ((data[i] < ' ')
#if !defined(CS_8BITCFGFILES)
          || (data[i] > 127)
#endif
          )
        {
          binary = true;
          break;
        }
    if (!binary)
      s.Append ((const char*)Data, DataSize);
    else                         // Save in Base64 mode
    {
      char tmp[CS_B64INILINELEN];
      UInt accbits = 0, top = 0, total = 0;
      unsigned char acc = 0;
      char endofbin[3] = {CommentChar, '/', 0};

      memset (tmp, 0, sizeof (tmp));
      while (DataSize)
      {
        int bits = 8;
        unsigned char byte = *data;

        data++;
        DataSize--;

        while (accbits + bits >= 6)
        {
          if (top >= CS_B64INILINELEN)
          {
            s << '\n';
	    s.Append (tmp, top);
            memset (tmp, 0, sizeof (tmp));
            top = 0;
          }
          acc |= (byte >> (2 + accbits));
          byte = (byte << (6 - accbits)) & 0xff;
          bits -= (6 - accbits);
          tmp[top++] = INIbase64[acc];
          total++;
          accbits = 0;
          acc = 0;
        } // while (accbits + bits >= 6)
        accbits = bits;
        acc = (byte >> 2);
      } // while (DataSize)
      if (top || accbits)
      {
        static char tail[4] = {0, 3, 2, 1};

        if (accbits)
        {
          tmp[top++] = INIbase64[acc];
          total++;
        }
        s << '\n';
	s.Append (tmp, top);
	s.Append ("===", tail[total & 3]);
      }
      s << '\n' << endofbin << Name;
    } // else if Save in Base64 mode
  }
  s << '\n';
}

void csIniFile::SaveSection (const PrvINInode* node, csString& s) const
{
  SaveComments (node->Comments, s);
  s << '[' << node->Section.Name << "]\n";

  DataIterator iterator (*this, node->Section.Vector, node->Section.Name);
  while (iterator.NextItem())
    SaveData (iterator.GetName(), iterator.GetData(), iterator.GetDataSize(),
      iterator.GetComments(), s);
}

bool csIniFile::Save (const char* path) const
{
  bool ok = false;
  FILE* file = fopen (path, "w");
  if (file)
  {
    csString s(Save());
    const size_t n = s.Length();
    ok = (n == 0 || fwrite (s.GetData(), 1, n, file) > 0);
    fclose (file);
  }
  return ok;
}

bool csIniFile::Save (iVFS* vfs, const char* path) const
{
  bool ok = false;
  if (vfs && path)
  {
    csString s(Save());
    ok = vfs->WriteFile (path, s.GetData(), s.Length());
  }
  return ok;
}

bool csIniFile::Save (iFile* file) const
{
  bool ok = false;
  if (file)
  {
    csString s(Save());
    ok = (file->Write (s.GetData(), s.Length()) == s.Length());
  }
  return ok;
}

csString csIniFile::Save () const
{
  csString s;
  SectionIterator iterator (EnumSections());
  while (iterator.NextItem())
    SaveSection (iterator.Node, s);
  return s;
}

#define SAVE_IF_DIRTY(P,A) \
bool csIniFile::SaveIfDirty P { if (Dirty) Dirty = !Save A; return !Dirty; }
SAVE_IF_DIRTY((const char* path), (path))
SAVE_IF_DIRTY((iVFS* vfs, const char* path), (vfs, path))
SAVE_IF_DIRTY((iFile* file), (file))
#undef SAVE_IF_DIRTY

csString csIniFile::SaveIfDirty ()
{
  ClearDirty();
  return Save();
}

bool csIniFile::Error (int LineNo, const char *Line, int Pos)
{
  (void)LineNo; (void)Line; (void)Pos;
  return false; // 'false' means continue loading
}

//-----------------------------------------------------------------------------

csIniFile::SectionIterator csIniFile::EnumSections(bool& Found) const
{
  Found = (Root.Length() != 0);
  return SectionIterator (*this, &Root);
}

bool csIniFile::EnumSections (csStrVector *oList) const
{
  bool Found;
  SectionIterator iterator (EnumSections(Found));
  while (iterator.NextItem())
    oList->Push (strnew(iterator.GetName()));
  return Found;
}

csIniFile::DataIterator csIniFile::EnumData (const char* SectionPath,
  bool& Found) const
{
  PrvINInode* Sec = FindNode (SectionPath, NULL);
  Found = (Sec != NULL);
  return DataIterator (*this, (Sec ? Sec->Section.Vector : 0), SectionPath);
}

bool csIniFile::EnumData (const char *SectionPath, csStrVector *oList) const
{
  bool Found;
  DataIterator iterator (EnumData(SectionPath, Found));
  while (iterator.NextItem())
    oList->Push (strnew(iterator.GetName()));
  return Found;
}

bool csIniFile::EnumData (const char *SectionPath, iStrVector *oList) const
{
  bool Found;
  DataIterator iterator (EnumData(SectionPath, Found));
  while (iterator.NextItem())
    oList->Push (strnew(iterator.GetName()));
  return Found;
}

csIniFile::CommentIterator csIniFile::EnumComments(const char* SectionPath,
  const char* KeyName, bool& Found) const
{
  const PrvINIbranch* Comments = NULL;

  if (KeyName)
  {
    DataIterator iterator (EnumData(SectionPath));
    while (iterator.NextItem())
      if (strcmp (KeyName, iterator.GetName()) == 0)
      {
        Comments = iterator.GetComments();
	break;
      }
  }
  else
  {
    SectionIterator iterator (EnumSections());
    while (iterator.NextItem())
      if (strcmp (SectionPath, iterator.GetName()) == 0)
      {
        Comments = iterator.GetComments();
	break;
      }
  }

  Found = (Comments != NULL);
  return CommentIterator (*this, Comments, SectionPath, KeyName);
}

bool csIniFile::EnumComments (const char *SectionPath, const char *KeyName,
  csStrVector *oList) const
{
  bool Found;
  CommentIterator iterator (EnumComments(SectionPath, KeyName, Found));
  while (iterator.NextItem())
    oList->Push (strnew(iterator.GetText()));
  return Found;
}

csIniFile::PrvINInode* csIniFile::FindNode (const char* SectionPath,
  const char* KeyName) const
{
  SectionIterator sections (EnumSections());
  while (sections.NextItem())
  {
    if (strcmp (SectionPath, sections.GetName()) == 0)
    {
      if (KeyName == 0)
        return CONST_CAST(PrvINInode*)(sections.Node);
      else
      {
        DataIterator data (*this, sections.Node->Section.Vector, SectionPath);
	while (data.NextItem())
	  if (strcmp (KeyName, data.GetName()) == 0)
	    return CONST_CAST(PrvINInode*)(data.Node);
      }
    }
  }
  return NULL;
}

bool csIniFile::GetData (const char *SectionPath, const char *KeyName,
  csSome &Data, size_t &DataSize) const
{
  DataIterator iterator (EnumData(SectionPath));
  while (iterator.NextItem())
    if (strcmp (KeyName, iterator.GetName()) == 0)
    {
    Data = iterator.GetData();
    DataSize = iterator.GetDataSize();
    return true;
    }
  Data = NULL;
  DataSize = 0;
  return (false);
}

void csIniFile::SetData (PrvINInode* node, csConstSome Data, size_t DataSize)
{
  node->Data.Size = DataSize;
  node->Data.Pointer = malloc (DataSize + 1);
  memcpy (node->Data.Pointer, Data, DataSize);
  ((char*)node->Data.Pointer)[DataSize] = '\0';
}

bool csIniFile::SetData (const char *SectionPath, const char *KeyName,
  csConstSome Data, size_t DataSize)
{
  PrvINInode* Sec = FindNode (SectionPath, NULL);
  PrvINIbranch* Vec;
  if (Sec)
    Vec = Sec->Section.Vector;
  else
  {
    Dirty = true;
    Vec = new PrvINIbranch();
    PrvINInode* branch = new PrvINInode;
    branch->Type = TYPE_SECTION;
    branch->Comments = NULL;
    branch->Section.Name = strdup (SectionPath);
    branch->Section.Vector = Vec;
    Root.Push (branch);
    if (Root.Length () > 1) // Add a empty line before section
      SetComment (SectionPath, NULL, NULL);
  }

  DataIterator iterator (*this, Vec, SectionPath);
  while (iterator.NextItem())
    if (strcmp (KeyName, iterator.GetName()) == 0)
    {
      if (Data == NULL || DataSize == 0)
        iterator.RemoveItem();
      else
      {
        free (iterator.Node->Data.Pointer);
	SetData (CONST_CAST(PrvINInode*)(iterator.Node), Data, DataSize);
      }
      Dirty = true;
      return true;
    }

  if (Data && DataSize != 0)
  {
    PrvINInode* branch = new PrvINInode;
    Vec->Push (branch);
    branch->Type = TYPE_DATA;
    branch->Comments = NULL;
    branch->Data.Name = strdup (KeyName);
    SetData (branch, Data, DataSize);
    Dirty = true;
  }
  return true;
}

bool csIniFile::SetStr (const char *SectionPath, const char *KeyName,
  const char *Value)
{
  return (SetData (SectionPath, KeyName, Value, Value ? strlen (Value) : 0));
}

bool csIniFile::SetInt(const char *SectionPath, const char *KeyName, int Value)
{
  char output [20];
  sprintf (output, "%d", Value);
  return SetStr (SectionPath, KeyName, output);
}

bool csIniFile::SetFloat (const char *SectionPath, const char *KeyName,
  float Value)
{
  char output [20];
  sprintf (output, "%g", Value);
  return SetStr (SectionPath, KeyName, output);
}

bool csIniFile::SetComment (const char *SectionPath, const char *KeyName,
  const char *Text)
{
  PrvINInode* node = FindNode (SectionPath, KeyName);
  if (node != NULL)
  {
    Dirty = true;
    if (node->Comments == NULL)
      node->Comments = new PrvINIbranch();

    PrvINInode* branch = new PrvINInode;
    branch->Type = TYPE_COMMENT;
    branch->Comments = NULL;
    branch->Comment.Text = (Text ? strdup(Text) : NULL);
    node->Comments->Push (branch);
    return true;
  }
  return false;
}

bool csIniFile::DeleteComment (const char *SectionPath, const char *KeyName)
{
  PrvINInode* node = FindNode (SectionPath, KeyName);
  if (node != NULL && node->Comments != NULL)
  {
    Dirty = true;
    delete node->Comments;
    node->Comments = NULL;
    return true;
  }
  return false;
}

bool csIniFile::Delete (const char *SectionPath, const char *KeyName)
{
  return SetData (SectionPath, KeyName, NULL, 0);
}

bool csIniFile::SectionExists (const char *SectionPath) const
{
  return (FindNode(SectionPath, NULL) != NULL);
}

bool csIniFile::KeyExists (const char *SectionPath, const char *KeyName) const
{
  return (FindNode (SectionPath, KeyName) != NULL);
}

const char *csIniFile::GetStr (const char *SectionPath, const char *KeyName,
  const char *def) const
{
  csSome c;
  size_t s;
  GetData (SectionPath, KeyName, c, s);
  return (c ? (const char*)c : def);
}

float csIniFile::GetFloat (const char *SectionPath, const char *KeyName,
  float def) const
{
  const char *s = GetStr (SectionPath, KeyName, NULL);
  if (!s)
    return def;
  float rc;
  sscanf (s, "%f", &rc);
  return rc;
}

int csIniFile::GetInt (const char *SectionPath, const char *KeyName,
  int def) const
{
  const char *s = GetStr (SectionPath, KeyName, NULL);
  if (!s)
    return def;
  int rc;
  sscanf (s, "%d", &rc);
  return rc;
}

bool csIniFile::GetYesNo (const char *SectionPath, const char *KeyName,
  bool def) const
{
  const char *s = GetStr (SectionPath, KeyName, NULL);
  if (!s)
    return def;
  return !strcasecmp(s,"yes") || !strcasecmp(s,"true") || !strcasecmp(s,"on");
}
