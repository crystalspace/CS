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
        delete [] ((PrvINInode *)Item)->Section.Name;
        break;
      case TYPE_DATA:
        delete [] (char *)((PrvINInode *)Item)->Data.Pointer;
        delete [] ((PrvINInode *)Item)->Data.Name;
        break;
      case TYPE_COMMENT:
        delete [] ((PrvINInode *)Item)->Comment.Text;
        break;
    }
    if (((PrvINInode *)Item)->Comments)
      delete ((PrvINInode *)Item)->Comments;
    delete (PrvINInode *)Item;
  }
  return true;
}

//--------------------------------------------------------------- Iterators ---

bool csIniFile::Iterator::Next ()
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

bool csIniFile::Iterator::Prev ()
{
  if (Current >= 0)
    for (Current--; Current >= 0; Current--)
    {
      Node = (PrvINInode*)Branch->Get(Current);
      if (Type == Node->Type)
        return true;
    }
  Node = NULL;
  return false;
}

void csIniFile::Iterator::RemoveItem ()
{
  if (Current >= 0 && Current < Limit)
  {
    CONST_CAST(PrvINIbranch*, Branch)->Delete (Current);
    Node = NULL;
    Limit--;
  }
}

IMPLEMENT_IBASE (csIniFile::SectionIterator)
  IMPLEMENTS_INTERFACE (iConfigSectionIterator);
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csIniFile::DataIterator)
  IMPLEMENTS_INTERFACE (iConfigDataIterator);
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csIniFile::CommentIterator)
  IMPLEMENTS_INTERFACE (iConfigCommentIterator);
IMPLEMENT_IBASE_END

// Place destructors for all kinds of iterators here so that
// virtual method table will reside in this object file
csIniFile::SectionIterator::~SectionIterator () {}
csIniFile::DataIterator::~DataIterator() {}
csIniFile::CommentIterator::~CommentIterator() {}

//------------------------------------------------------------ Constructors ---

IMPLEMENT_IBASE (csIniFile)
  IMPLEMENTS_INTERFACE (iConfigFile);
IMPLEMENT_IBASE_END

csIniFile::csIniFile (const char* iFileName, char iCommentChar) :
  CommentChar (iCommentChar), Dirty (false), FileName (NULL), VFS (NULL)
{
  CONSTRUCT_IBASE (NULL);
  Load (iFileName);
}

csIniFile::csIniFile (const char* iFileName, iVFS *vfs, char iCommentChar) :
  CommentChar (iCommentChar), Dirty (false), FileName (NULL), VFS (NULL)
{
  CONSTRUCT_IBASE (NULL);
  Load (iFileName, vfs);
}

csIniFile::~csIniFile ()
{
  delete [] FileName;
  if (VFS) VFS->DecRef ();
}

//--------------------------------------------------------- csIniFile::Load ---

static bool NextLine (csString& line, const char*& source, const char* limit)
{
  line.Clear();
  if (source >= limit) return false;
  const char* const start = source;
  while (source < limit && *source != '\n' && *source != '\r')
    source++;
  line.Append (start, source - start);
  if (source < limit)
  {
    // Strip line terminator (handles DOS:CRLF, Unix:LF, Macintosh:CR)
    source++;   // Strip initial LF or CR
    if (*(source - 1) == '\r' && source < limit && *source == '\n')
      source++; // Strip CRLF.
  }
  return true;
}

bool csIniFile::Load (const char *iFileName, iVFS *vfs)
{
  bool rc = false;
  if (vfs)
  {
    iDataBuffer *data = vfs->ReadFile (iFileName);
    if (data)
    {
      rc = data->GetSize () ? _Load (**data, data->GetSize ()) : true;
      data->DecRef ();
    }
  }
  else
  {
    FILE* f = fopen (iFileName, "rb");
    if (f)
    {
      size_t size = 0;
      if (fseek (f, 0, SEEK_END) == 0
       && (size = ftell(f)) != size_t(-1)
       && fseek (f, 0, SEEK_SET) == 0)
      {
        if (size == 0)
          rc = true; // Empty file, but not an error.
        else
        {
          char *data = new char [size];
          if (fread (data, size, 1, f) == 1)
            rc = _Load (data, size);
          delete [] data;
        }
      }
      fclose (f);
    }
  }

  delete [] FileName;
  FileName = NULL;
  if (VFS) VFS->DecRef ();
  VFS = NULL;

  if (rc)
  {
    FileName = strnew (iFileName);
    if ((VFS = vfs))
      vfs->IncRef ();
  }
  return rc;
}

bool csIniFile::_Load (const char *Data, size_t DataSize)
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
    if (!buff) continue;
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
        {
          b64mode = false;
          goto plain;
        }

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

      char *newdata = new char [branch->Data.Size + len + 1];
      memcpy (newdata, branch->Data.Pointer, branch->Data.Size);
      memcpy (newdata + branch->Data.Size, tmp, len);
      newdata [branch->Data.Size + len] = 0;

      delete [] (char *)branch->Data.Pointer;
      branch->Data.Size += len;
      branch->Data.Pointer = newdata;

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
          branch->Comment.Text = strnew (cur + 1);
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
        branch->Section.Name = strnew (tmp);
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
        b64mode = (eq [1] == '=') && (!eq [2]);
        while (i && strchr (CS_INISPACE, tmp[i - 1]))
          i--;
        tmp[i] = 0;

        branch = new PrvINInode;
        branch->Type = TYPE_DATA;
        branch->Comments = Comments;
        branch->Data.Name = strnew (tmp);

        cur = eq + 1;
        cur += strspn (cur, CS_INISPACE);

        strcpy (tmp, cur);
        i = strlen (cur);
        if (b64mode)
        {
          branch->Data.Size = 0;
          branch->Data.Pointer = NULL;
          b64mode = true;
          SkipComment = true;
        }
        else
        {
          while (i && strchr (CS_INISPACE, tmp[i - 1]))
            i--;
          tmp[i] = 0;
	  branch->Data.Size = i;
          branch->Data.Pointer = strnew (tmp);
        }
        CurBranch->Push (branch);
        Comments = NULL;
      } // Key = Value
    } // if (b64mode)
  } // for (;;)

out:
  if (Comments)
  {
    while (Comments->Length ())
      Root.Push (Comments->Pop ());
    delete Comments;
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
  CommentIterator iterator (this, branch, "", "");
  while (iterator.Next())
    SaveComment (iterator.GetComment (), s);
}

void csIniFile::SaveData (const char* Name, csSome Data, size_t DataSize,
  const PrvINIbranch* comments, csString& s) const
{
  const char* data = (const char*)Data;
  SaveComments (comments, s);
  s << Name << " =";
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
    {
      s.Append (" ", 1);
      s.Append ((const char*)Data, DataSize);
    }
    else                         // Save in Base64 mode
    {
      char tmp[CS_B64INILINELEN];
      UInt accbits = 0, top = 0, total = 0;
      unsigned char acc = 0;
      char endofbin[3] = {CommentChar, '/', 0};

      s.Append ("=", 1);
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

  DataIterator iterator (this, node);
  while (iterator.Next ())
    SaveData (iterator.GetKey (), iterator.GetData (), iterator.GetDataSize (),
      iterator.GetComments (), s);
}

bool csIniFile::Save () const
{
  return FileName ? Save (FileName, VFS) : false;
}

bool csIniFile::Save (const char *iFileName, iVFS *vfs) const
{
  bool ok = false;
  if (iFileName)
    if (vfs)
    {
      csString s (_Save ());
      ok = vfs->WriteFile (iFileName, s.GetData (), s.Length ());
    }
    else
    {
      FILE* file = fopen (iFileName, "w");
      if (file)
      {
        csString s (_Save ());
        const size_t n = s.Length ();
        ok = (n == 0 || fwrite (s.GetData (), 1, n, file) > 0);
        fclose (file);
      }
    }
  return ok;
}

csString csIniFile::_Save () const
{
  csString s;
  SectionIterator iterator (this, &Root);
  while (iterator.Next ())
    SaveSection (iterator.Node, s);
  return s;
}

bool csIniFile::Error (int LineNo, const char *Line, int Pos)
{
  (void)LineNo; (void)Line; (void)Pos;
  return false; // 'false' means continue loading
}

//-----------------------------------------------------------------------------

csIniFile::SectionIterator *csIniFile::_EnumSections () const
{
  return Root.Length () ? new SectionIterator (this, &Root) : 0;
}

csIniFile::DataIterator *csIniFile::_EnumData (const char* iSection) const
{
  PrvINInode *Sec = FindNode (iSection, NULL);
  return Sec ? new DataIterator (this, Sec) : 0;
}

csIniFile::CommentIterator *csIniFile::_EnumComments(const char* iSection,
  const char* iKey) const
{
  PrvINInode *Sec = FindNode (iSection, iKey);
  return Sec ? new CommentIterator (this, Sec->Comments, iSection, iKey) : 0;
}

csIniFile::PrvINInode *csIniFile::FindNode (const char* iSection,
  const char* iKey) const
{
  SectionIterator sections (this, &Root);
  while (sections.Next ())
  {
    if (!strcmp (iSection, sections.GetSection ()))
    {
      if (iKey == 0)
        return CONST_CAST(PrvINInode*, sections.Node);
      else
      {
        DataIterator data (this, sections.Node);
	while (data.Next ())
	  if (!strcmp (iKey, data.GetKey ()))
	    return CONST_CAST(PrvINInode*, data.Node);
      }
    }
  }
  return NULL;
}

bool csIniFile::GetData (const char *iSection, const char *iKey,
  csSome &Data, size_t &DataSize) const
{
  PrvINInode *Sec = FindNode (iSection, iKey);
  if (Sec && Sec->Type == TYPE_DATA)
  {
    Data = Sec->Data.Pointer;
    DataSize = Sec->Data.Size;
    return true;
  }

  Data = NULL;
  DataSize = 0;
  return false;
}

void csIniFile::SetData (PrvINInode* node, csConstSome Data, size_t DataSize)
{
  node->Data.Size = DataSize;
  node->Data.Pointer = new char [DataSize + 1];
  memcpy (node->Data.Pointer, Data, DataSize);
  ((char*)node->Data.Pointer)[DataSize] = '\0';
}

bool csIniFile::SetData (const char *iSection, const char *iKey,
  csConstSome Data, size_t DataSize)
{
  PrvINInode *Sec = FindNode (iSection, NULL);
  PrvINIbranch *Vec;
  if (Sec)
    Vec = Sec->Section.Vector;
  else
  {
    // Create a new section if it does not exist
    Dirty = true;
    Vec = new PrvINIbranch();
    Sec = new PrvINInode;
    Sec->Type = TYPE_SECTION;
    Sec->Comments = NULL;
    Sec->Section.Name = strnew (iSection);
    Sec->Section.Vector = Vec;
    Root.Push (Sec);
    if (Root.Length () > 1) // Add a empty line before section
      SetComment (iSection, NULL, NULL);
  }

  DataIterator iterator (this, Sec);
  while (iterator.Next ())
    if (!strcmp (iKey, iterator.GetKey ()))
    {
      if (Data == NULL || DataSize == 0)
        iterator.RemoveItem ();
      else
      {
        delete [] (char *)iterator.Node->Data.Pointer;
	SetData (CONST_CAST(PrvINInode*, iterator.Node), Data, DataSize);
      }
      Dirty = true;
      return true;
    }

  if (Data && DataSize != 0)
  {
    PrvINInode *branch = new PrvINInode;
    Vec->Push (branch);
    branch->Type = TYPE_DATA;
    branch->Comments = NULL;
    branch->Data.Name = strnew (iKey);
    SetData (branch, Data, DataSize);
    Dirty = true;
  }
  return true;
}

bool csIniFile::SetStr (const char *iSection, const char *iKey,
  const char *Value)
{
  return (SetData (iSection, iKey, Value, Value ? strlen (Value) : 0));
}

bool csIniFile::SetInt(const char *iSection, const char *iKey, int Value)
{
  char output [20];
  sprintf (output, "%d", Value);
  return SetStr (iSection, iKey, output);
}

bool csIniFile::SetFloat (const char *iSection, const char *iKey,
  float Value)
{
  char output [20];
  sprintf (output, "%g", Value);
  return SetStr (iSection, iKey, output);
}

bool csIniFile::SetComment (const char *iSection, const char *iKey,
  const char *Text)
{
  PrvINInode *node = FindNode (iSection, iKey);
  if (node)
  {
    Dirty = true;
    if (node->Comments == NULL)
      node->Comments = new PrvINIbranch ();

    PrvINInode *branch = new PrvINInode;
    branch->Type = TYPE_COMMENT;
    branch->Comments = NULL;
    branch->Comment.Text = strnew (Text);
    node->Comments->Push (branch);
    return true;
  }
  return false;
}

bool csIniFile::DeleteComment (const char *iSection, const char *iKey)
{
  PrvINInode *node = FindNode (iSection, iKey);
  if (node && node->Comments)
  {
    Dirty = true;
    delete node->Comments;
    node->Comments = NULL;
    return true;
  }
  return false;
}

bool csIniFile::SectionExists (const char *iSection) const
{
  return (FindNode(iSection, NULL) != NULL);
}

bool csIniFile::KeyExists (const char *iSection, const char *iKey) const
{
  return (FindNode (iSection, iKey) != NULL);
}

const char *csIniFile::GetStr (const char *iSection, const char *iKey,
  const char *def) const
{
  csSome c;
  size_t s;
  GetData (iSection, iKey, c, s);
  return (c ? (const char*)c : def);
}

float csIniFile::GetFloat (const char *iSection, const char *iKey,
  float def) const
{
  const char *s = GetStr (iSection, iKey, NULL);
  if (!s)
    return def;
  float rc;
  sscanf (s, "%f", &rc);
  return rc;
}

int csIniFile::GetInt (const char *iSection, const char *iKey,
  int def) const
{
  const char *s = GetStr (iSection, iKey, NULL);
  if (!s)
    return def;
  int rc;
  sscanf (s, "%d", &rc);
  return rc;
}

bool csIniFile::GetYesNo (const char *iSection, const char *iKey,
  bool def) const
{
  const char *s = GetStr (iSection, iKey, NULL);
  if (!s)
    return def;
  return !strcasecmp (s,"yes") || !strcasecmp (s,"true") ||
    !strcasecmp (s,"on") || !strcasecmp (s,"1");
}
