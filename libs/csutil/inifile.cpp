/*
    Crystal Space .INI file management
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include <malloc.h>
#include "sysdef.h"
#include "types.h"
#include "csutil/inifile.h"
#include "csutil/archive.h"

// Use 8-bit characters in INI files
#define CS_8BITINIFILES
// Characters ignored in INI files (except in middle of section & key names)
#define INISPACE   " \t"

// branch->Type values
#define TYPE_SECTION    1
#define TYPE_DATA       2
#define TYPE_COMMENT    3

static char *INIbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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
        CHK (delete (((PrvINInode *)Item)->Section.Vector));
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
      CHKB (delete ((PrvINInode *)Item)->Comments);
    CHK (delete (PrvINInode *)Item);
  }
  return true;
}

csIniFile::csIniFile (char iCommentChar)
{
  CommentChar = iCommentChar;
  Dirty = true;
}

csIniFile::csIniFile (const char *fName, char iCommentChar)
{
  CommentChar = iCommentChar;
  Dirty = true;
  Load (fName);
}

csIniFile::~csIniFile ()
{
}

static bool ReadFileLine (csSome Stream, void *data, size_t size)
{
  if (!fgets ((char *)data, size, (FILE *)Stream))
    return false;
  size_t sl = strlen ((char *)data);
  if (sl >= size)
    sl = size - 1;
  while (sl && (((char *)data) [sl - 1] < ' '))
    sl--;
  ((char *)data) [sl] = 0;
  return true;
}

bool csIniFile::Load (const char *fName)
{
  FILE *f = fopen (fName, "r");
  if (!f)
    return false;
  bool rc = Load (ReadFileLine, f);
  fclose (f);
  return rc;
}

struct __datastream
{
  const char *data;
  int dataleft;
};

static bool ReadMemoryLine (csSome Stream, void *data, size_t size)
{
  __datastream *ds = (__datastream *)Stream;
  if (!ds->dataleft)
    return false;

  // Get by one character until we get a character with code less than space
  while ((*ds->data >= ' ') && ds->dataleft)
  {
    if (size)
    {
      if (size > 1)
        *((char *)data) = *ds->data;
      else
        *((char *)data) = 0;
      data = ((char *)data) + 1;
      size--;
    } /* endif */
    ds->data++;
    ds->dataleft--;
  } /* endwhile */
  // Put ending zero
  if (size)
    *((char *)data) = 0;
  // Skip \n, \r and so on
  while ((*ds->data < ' ') && ds->dataleft)
  {
    ds->data++;
    ds->dataleft--;
  } /* endwhile */
  return true;
}

bool csIniFile::Load (const char *Data, size_t DataSize)
{
  __datastream ds = { Data, DataSize };
  return Load (ReadMemoryLine, &ds);
}

bool csIniFile::Load (bool (*ReadLine) (csSome Stream, void *data, size_t size), csSome Stream)
{
  char buff[CS_MAXINILINELEN];
  char tmp[CS_MAXINILINELEN];
  int LineNo = 0;
  PrvINInode *branch;
  PrvINIbranch *CurBranch = NULL;
  PrvINIbranch *Comments = NULL;
  bool SkipComment = false, PSkipComment = false;

  // Assume object contents is syncronised
  Dirty = false;

  // Base64 decoding variables
  bool b64mode = false;
  bool b64init = true;
  int b64top = 0;
  unsigned char b64acc = 0;

  for ( ; ; )
  {
    char *cur = buff;

    if (!ReadLine (Stream, cur, CS_MAXINILINELEN))
      break;
    LineNo++;

    cur += strspn (cur, INISPACE);

    if (b64mode)                        // Base64 mode
    {
      int i, len;
      char *s;
      bool finish = false;

      // Check for end-of-base64-sequence character(s) -- '='
      for (i = strlen (cur); i > 0; i--)
        if (cur[i - 1] == '=')
          finish = true;
        else
          break;
      cur[i] = 0;
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
      branch->Data.Pointer = realloc (branch->Data.Pointer, branch->Data.Size + 1);
      memcpy (((char *)branch->Data.Pointer) + i, tmp, len);
      ((char *) branch->Data.Pointer)[branch->Data.Size] = 0;

      if (finish)
      {
        b64init = true;
        b64mode = false;
      }
    } else
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
        CHK (branch = new PrvINInode);
        branch->Type = TYPE_COMMENT;
        branch->Comments = NULL;
        if (*cur)
          branch->Comment.Text = strdup (cur + 1);
        else
          branch->Comment.Text = NULL;
        if (!Comments)
          CHKB (Comments = new PrvINIbranch);
        Comments->Push (branch);
      } else if (*cur == '[')           // Section
      {
        char *cb;
        int i;

        cur++;
        cur += strspn (cur, INISPACE);
        cb = strchr (cur, ']');
        if (!cb)
          goto error;

        strncpy (tmp, cur, i = int (cb - cur));
        while (i && strchr (INISPACE, tmp[i - 1]))
          i--;
        tmp[i] = 0;

        CHK (branch = new PrvINInode);
        branch->Type = TYPE_SECTION;
        branch->Comments = Comments;
        branch->Section.Name = strdup (tmp);
        CHK (branch->Section.Vector = CurBranch = new PrvINIbranch ());
        Root.Push (branch);
        Comments = NULL;
      }                                 // else if Section
      else
      {                                 // Key = Value
        char *eq;
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
        while (i && strchr (INISPACE, tmp[i - 1]))
          i--;
        tmp[i] = 0;

        CHK (branch = new PrvINInode);
        branch->Type = TYPE_DATA;
        branch->Comments = Comments;
        branch->Data.Name = strdup (tmp);

        cur = eq + 1;
        cur += strspn (cur, INISPACE);

        strcpy (tmp, cur);
        i = strlen (cur);
        if (i)
        {
          while (i && strchr (INISPACE, tmp[i - 1]))
            i--;
          tmp[i] = 0;
          branch->Data.Pointer = malloc ((branch->Data.Size = i) + 1);
          strcpy ((char *) branch->Data.Pointer, tmp);
        } else
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
    //for (int i = 0; i < Comments->Length (); i++)
      //Root.Push ((*Comments)[i]);
    CHK (delete Comments);
    Comments = NULL;
  }
  return (true);
}

//---------------------------------------------------------- csIniFile::Save ---
typedef struct
{
  csIniFile *ini;
  FILE *f;
  char *Section;
} EnumStruc, *pEnumStruc;

bool csIniFile::SaveEnumComments (csSome struc, char *Text)
{
  if (Text)
  {
    fwrite (&pEnumStruc (struc)->ini->CommentChar, 1, 1, pEnumStruc (struc)->f);
    fputs (Text, pEnumStruc (struc)->f);
  }
  fputs ("\n", pEnumStruc (struc)->f);

  return (false);
}

bool csIniFile::SaveEnumData (csSome struc, char *Name, size_t DataSize, csSome Data)
{
  char *data = (char *) Data;

  pEnumStruc (struc)->ini->EnumComments (pEnumStruc (struc)->Section, Name,
    SaveEnumComments, struc);

  fputs (Name, pEnumStruc (struc)->f);
  fputs (" = ", pEnumStruc (struc)->f);
  if (Data && DataSize)
  {
    size_t i;
    bool binary = false;

    if ((data[0] <= ' ') || (data[DataSize - 1] <= ' '))
      binary = true;
    else
      for (i = 0; i < DataSize; i++)
        if ((data[i] < ' ')
#if !defined(CS_8BITINIFILES)
          || (data[i] > 127)
#endif
          )
        {
          binary = true;
          break;
        }
    if (binary)                         // Save in Base64 mode
    {
      char tmp[CS_B64INILINELEN];
      UInt accbits = 0, top = 0, total = 0;
      unsigned char acc = 0;
      char endofbin[3] = {pEnumStruc (struc)->ini->CommentChar, '/', 0};

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
            fputs ("\n", pEnumStruc (struc)->f);
            fwrite (tmp, 1, top, pEnumStruc (struc)->f);
            memset (tmp, 0, sizeof (tmp));
            top = 0;
          }
          acc |= (byte >> (2 + accbits));
          byte = (byte <<= (6 - accbits)) & 0xff;
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
        fputs ("\n", pEnumStruc (struc)->f);
        fwrite (tmp, 1, top, pEnumStruc (struc)->f);
        fwrite ("===", 1, tail[total & 3], pEnumStruc (struc)->f);
      }
      fputs ("\n", pEnumStruc (struc)->f);
      fputs (endofbin, pEnumStruc (struc)->f);
      fputs (Name, pEnumStruc (struc)->f);
    } // else if Save in Base64 mode
    else
      fwrite (Data, 1, DataSize, pEnumStruc (struc)->f);
  }
  fputs ("\n", pEnumStruc (struc)->f);
  return (false);
}

bool csIniFile::SaveEnumSec (csSome struc, char *Name)
{
  pEnumStruc (struc)->Section = Name;
  pEnumStruc (struc)->ini->EnumComments (Name, NULL, SaveEnumComments, struc);

  fputs ("[", pEnumStruc (struc)->f);
  fputs (Name, pEnumStruc (struc)->f);
  fputs ("]\n", pEnumStruc (struc)->f);
  pEnumStruc (struc)->ini->EnumData (Name, SaveEnumData, struc);

  return (false);
}

bool csIniFile::Save (const char *fName)
{
  if (Dirty)
  {
    EnumStruc s = { this };
    s.f = fopen (fName, "w");
    if (!s.f)
      return false;
    s.ini->EnumSections (NULL, SaveEnumSec, &s);
    fclose (s.f);
    Dirty = false;
  }
  return true;
}

bool csIniFile::Error (int LineNo, const char *Line, int Pos)
{
  (void) LineNo;
  (void) Line;
  (void) Pos;
  return (false);                    // Continue loading
}

bool csIniFile::EnumSections (const char *SectionPath, bool (*iterator)
  (csSome Parm, char *Name), csSome Parm) const
{
  int i, j;
  PrvINIbranch *Sec = FindNode (SectionPath);

  if (!Sec)
    return (false);

  j = Sec->Length ();

  for (i = 0; i < j; i++)
  {
    PrvINInode *cn = (PrvINInode *)(*Sec)[i];

    if (cn->Type == TYPE_SECTION)
      if (iterator (Parm, cn->Section.Name))
        break;
  }
  return (true);
}

bool csIniFile::EnumData (const char *SectionPath, bool (*iterator)
  (csSome Parm, char *Name, size_t DataSize, csSome Data), csSome Parm) const
{
  int i, j;
  PrvINIbranch *Sec = FindNode (SectionPath);

  if (!Sec)
    return (false);

  j = Sec->Length ();

  for (i = 0; i < j; i++)
  {
    PrvINInode *cn = (PrvINInode *)(*Sec)[i];

    if (cn->Type == TYPE_DATA)
      if (iterator (Parm, cn->Data.Name, cn->Data.Size, cn->Data.Pointer))
        break;
  }
  return (true);
}

bool csIniFile::EnumComments (const char *SectionPath, const char *KeyName,
  bool (*iterator) (csSome Parm, char *Text), csSome Parm) const
{
  int i, len;
  PrvINIbranch *Sec;
  PrvINIbranch *Comments = NULL;

  if (KeyName)
  {
    Sec = FindNode (SectionPath);
    if (!Sec)
      return (false);

    len = Sec->Length ();
    for (i = 0; i < len; i++)
    {
      PrvINInode *cn = (PrvINInode *)(*Sec)[i];

      if ((cn->Type == TYPE_DATA) && (strcmp (KeyName, cn->Data.Name) == 0))
      {
        Comments = cn->Comments;
        break;
      }
    }
  } else
  {
    len = Root.Length ();
    for (i = 0; i < len; i++)
    {
      PrvINInode *cn = (PrvINInode *)Root [i];

      if ((cn->Type == TYPE_SECTION) && (strcmp (SectionPath, cn->Section.Name) == 0))
      {
        Comments = cn->Comments;
        break;
      }
    }
  }

  if (!Comments)
    return (false);

  len = Comments->Length ();
  for (i = 0; i < len; i++)
  {
    PrvINInode *cn = (PrvINInode *)(*Comments)[i];

    if (cn->Type == TYPE_COMMENT)
      if (iterator (Parm, cn->Comment.Text))
        break;
  }
  return (true);
}

csIniFile::PrvINIbranch *csIniFile::FindNode (const char *SectionPath) const
{
  if (!SectionPath)
    return (CONST_CAST(PrvINIbranch*)(&Root));

  for (int i = 0, len = Root.Length (); i < len; i++)
  {
    PrvINInode *cn = (PrvINInode *)Root [i];

    if ((cn->Type == TYPE_SECTION) && (!strcmp (SectionPath, cn->Section.Name)))
      return cn->Section.Vector;
  }
  return NULL;
}

bool csIniFile::GetData (const char *SectionPath, const char *KeyName,
  csSome &Data, size_t &DataSize) const
{
  PrvINIbranch *Sec = FindNode (SectionPath);
  int i, len;

  Data = NULL;
  DataSize = 0;

  if (!Sec)
    return (false);

  for (i = 0, len = Sec->Length (); i < len; i++)
  {
    PrvINInode *cn = (PrvINInode *)(*Sec)[i];

    if ((cn->Type == TYPE_DATA) && (strcmp (KeyName, cn->Data.Name) == 0))
    {
      Data = cn->Data.Pointer;
      DataSize = cn->Data.Size;
      return (true);
    }
  }
  return (false);
}

bool csIniFile::SetData (const char *SectionPath, const char *KeyName,
  csConstSome Data, size_t DataSize)
{
  PrvINIbranch *Sec = FindNode (SectionPath);
  PrvINInode *branch;
  int i, len;

  Dirty = true;
  if (!Sec)
  {
    CHK (branch = new PrvINInode);
    branch->Type = TYPE_SECTION;
    branch->Comments = NULL;
    branch->Section.Name = strdup (SectionPath);
    CHK (branch->Section.Vector = Sec = new PrvINIbranch ());
    Root.Push (branch);

    // Add a empty line before section
    if (Root.Length () > 1)
      SetComment (SectionPath, NULL, NULL);
  }
  for (i = 0, len = Sec->Length (); i < len; i++)
  {
    branch = (PrvINInode *)(*Sec)[i];

    if ((branch->Type == TYPE_DATA) && (strcmp (KeyName, branch->Data.Name) == 0))
    {
      if (Data && DataSize)
      {
        free (branch->Data.Pointer);
        goto setval;
      } else
        Sec->Delete (i);
      return (true);
    }
  }

  CHK (branch = new PrvINInode);
  Sec->Push (branch);
  branch->Type = TYPE_DATA;
  branch->Comments = NULL;
  branch->Data.Name = strdup (KeyName);
setval:
  branch->Data.Pointer = malloc (DataSize + 1);
  branch->Data.Size = DataSize;
  memcpy (branch->Data.Pointer, Data, DataSize);
  ((char *) branch->Data.Pointer)[DataSize] = 0;
  return (true);
}

bool csIniFile::SetStr (const char *SectionPath, const char *KeyName,
  const char *Value)
{
  return (SetData (SectionPath, KeyName, Value, strlen (Value)));
}

bool csIniFile::SetComment (const char *SectionPath, const char *KeyName,
  const char *Text)
{
  int i, len;
  PrvINIbranch *Sec;
  PrvINIbranch **Comments = NULL;
  PrvINInode *branch;

  Dirty = true;
  if (KeyName)
  {
    Sec = FindNode (SectionPath);
    if (!Sec)
      return (false);

    for (i = 0, len = Sec->Length (); i < len; i++)
    {
      PrvINInode *cn = (PrvINInode *)(*Sec)[i];

      if ((cn->Type == TYPE_DATA) && (strcmp (KeyName, cn->Data.Name) == 0))
      {
        Comments = &cn->Comments;
        break;
      }
    }
  } else
  {
    len = Root.Length ();
    for (i = 0; i < len; i++)
    {
      PrvINInode *cn = (PrvINInode *)Root [i];

      if ((cn->Type == TYPE_SECTION) && (strcmp (SectionPath, cn->Section.Name) == 0))
      {
        Comments = &cn->Comments;
        break;
      }
    }
  }

  if (!Comments)
    return (false);

  if (!*Comments)
    CHKB (*Comments = new PrvINIbranch ());

  CHK (branch = new PrvINInode);
  branch->Type = TYPE_COMMENT;
  branch->Comments = NULL;
  if (Text)
    branch->Comment.Text = strdup (Text);
  else
    branch->Comment.Text = NULL;
  (*Comments)->Push (branch);
  return (true);
}

bool csIniFile::Delete (const char *SectionPath, const char *KeyName)
{
  return SetData (SectionPath, KeyName, NULL, 0);
}

bool csIniFile::SectionExists (const char *SectionPath) const
{
  PrvINIbranch *Sec = FindNode (SectionPath);
  return (Sec != NULL);
}

bool csIniFile::KeyExists (const char *SectionPath, const char *KeyName) const
{
  PrvINIbranch *Sec = FindNode (SectionPath);

  if (Sec)
    for (int i = 0, len = Sec->Length (); i < len; i++)
    {
      PrvINInode *cn = (PrvINInode *)(*Sec)[i];
      if ((cn->Type == TYPE_DATA) && (strcmp (KeyName, cn->Data.Name) == 0))
        return true;
    }
  return false;
}

char *csIniFile::GetStr (const char *SectionPath, const char *KeyName,
  char *def) const
{
  csSome c;
  size_t s;

  GetData (SectionPath, KeyName, c, s);
  if (c)
    return (char *)c;
  else
    return def;
}

float csIniFile::GetFloat (const char *SectionPath, const char *KeyName,
  float def) const
{
  char *s = GetStr (SectionPath, KeyName, NULL);
  if (!s)
    return def;
  float rc;
  sscanf (s, "%f", &rc);
  return rc;
}

int csIniFile::GetInt (const char *SectionPath, const char *KeyName, int def) const
{
  char *s = GetStr (SectionPath, KeyName, NULL);
  if (!s)
    return def;
  int rc;
  sscanf (s, "%d", &rc);
  return rc;
}

bool csIniFile::GetYesNo (const char *SectionPath, const char *KeyName,
  bool def) const
{
  char *s = GetStr (SectionPath, KeyName, NULL);
  if (!s)
    return def;
  return (!strcasecmp (s, "yes")) || (!strcasecmp (s, "true")) || (!strcasecmp (s, "on"));
}
