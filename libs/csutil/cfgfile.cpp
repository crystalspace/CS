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
#include "csutil/databuf.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "ivfs.h"
#include <ctype.h>

/* config node */

class csConfigNode
{
public:
  // create a new config node. Set name to NULL to create the initial node.
  csConfigNode(const char *Name);
  // delete this node
  ~csConfigNode();
  // delete all nodes will non-NULL name
  void DeleteDataNodes();
  // insert this node after the given node
  void InsertAfter(csConfigNode *Where);
  // remove this node from its list
  void Remove();
  // get the name of this node
  const char *GetName() const;
  // set the data for this key
  void SetStr(const char*);
  void SetInt(int);
  void SetFloat(float);
  void SetBool(bool);
  void SetComment(const char*);
  // get data
  const char *GetStr() const;
  int GetInt() const;
  float GetFloat() const;
  bool GetBool() const;
  const char *GetComment() const;
  // return prev and next node
  csConfigNode *GetPrev() const;
  csConfigNode *GetNext() const;

private:
  // previous and next node
  csConfigNode *Prev, *Next;
  // key name
  char *Name;
  // key data
  char *Data;
  // comment
  char *Comment;
};

csConfigNode::csConfigNode(const char *Keyname)
{
  Prev = Next = NULL;
  Name = strnew(Keyname);
  Data = Comment = NULL;
}

csConfigNode::~csConfigNode()
{
  Remove();
  if (Name) delete[] Name;
  if (Data) delete[] Data;
  if (Comment) delete[] Comment;
}

const char *csConfigNode::GetName() const
{
  return Name;
}

csConfigNode *csConfigNode::GetPrev() const
{
  return Prev;
}

csConfigNode *csConfigNode::GetNext() const
{
  return Next;
}

void csConfigNode::DeleteDataNodes()
{
  if (Next) Next->DeleteDataNodes();
  if (Name) delete this;
}

void csConfigNode::InsertAfter(csConfigNode *Where)
{
  if (!Where) return;
  Next = Where->Next;
  Prev = Where;
  Where->Next = this;
  if (Next) Next->Prev = this;
}

void csConfigNode::Remove()
{
  if (Next) Next->Prev = Prev;
  if (Prev) Prev->Next = Next;
  Prev = Next = NULL;
}

void csConfigNode::SetStr(const char *s)
{
  if (Data) delete[] Data;
  Data = strnew(s);
}

void csConfigNode::SetInt(int n)
{
  char output [32];
  sprintf (output, "%d", n);
  SetStr (output);
}

void csConfigNode::SetFloat(float f)
{
  char output [64];
  sprintf (output, "%g", f);
  SetStr (output);
}

void csConfigNode::SetBool(bool b)
{
  SetStr(b ? "true" : "false");
}

void csConfigNode::SetComment(const char *s)
{
  if (Comment) delete[] Comment;
  Comment = strnew(s);
}

const char *csConfigNode::GetStr() const
{
  return (Data ? Data : "");
}

int csConfigNode::GetInt() const
{
  return (Data ? atoi(Data) : 0);
}

float csConfigNode::GetFloat() const
{
  return (Data ? atof(Data) : 0);
}

bool csConfigNode::GetBool() const
{
  return (Data &&
    (strcasecmp(Data, "true") == 0 ||
     strcasecmp(Data, "yes" ) == 0 ||
     strcasecmp(Data, "on"  ) == 0 ||
     strcasecmp(Data, "1"   ) == 0));
}

const char *csConfigNode::GetComment() const
{
  return Comment;
}

/* config iterator */

class csConfigIterator : public iConfigIterator
{
public:
  DECLARE_IBASE;

  /// Returns the configuration object for this iterator.
  virtual iConfigFileNew *GetConfigFile() const;
  /// Returns the subsection in the configuruation.
  virtual const char *GetSubsection() const;

  /// Rewind the iterator (points to nowhere after this)
  virtual void Rewind ();
  /// Move to previous item and return true if the position is valid
  virtual bool Prev ();
  /// Move to the next valid key. Returns false if no more keys exist.
  virtual bool Next();

  /**
   * Get the current key name.  Set Local to true to return only the local name
   * inside the iterated subsection.  This is the portion of the key string
   * which follows the subsection prefix which was used to create this
   * iterator.
   */
  virtual const char *GetKey(bool Local = false) const;
  /// Get an integer value from the configuration.
  virtual int GetInt() const;
  /// Get a float value from the configuration.
  virtual float GetFloat() const;
  /// Get a string value from the configuration.
  virtual const char *GetStr() const;
  /// Get a boolean value from the configuration.
  virtual bool GetBool() const;
  /// Get the comment of the given key, or NULL if no comment exists.
  virtual const char *GetComment() const;

private:
  friend class csConfigFile;
  csConfigFile *Config;
  csConfigNode *Node;
  char *Subsection;
  int SubsectionLength;

  // Create a new iterator.
  csConfigIterator(csConfigFile *Config, const char *Subsection);
  // Delete this iterator
  virtual ~csConfigIterator();
  // Utility function to check if a key meets subsection requirement
  bool CheckSubsection(const char *Key) const;
  // Move to the previous node, ignoring subsection
  bool DoPrev();
  // Move to the next node, ignoring subsection
  bool DoNext();
};

IMPLEMENT_IBASE(csConfigIterator);
  IMPLEMENTS_INTERFACE(iConfigIterator);
IMPLEMENT_IBASE_END;

csConfigIterator::csConfigIterator(csConfigFile *c, const char *sub)
{
  CONSTRUCT_IBASE(NULL);
  Config = c;
  Node = Config->FirstNode;
  Subsection = strnew(sub);
  SubsectionLength = (Subsection ? strlen(Subsection) : 0);
  Config->IncRef();
}

csConfigIterator::~csConfigIterator()
{
  Config->RemoveIterator(this);
  if (Subsection) delete[] Subsection;
  Config->DecRef();
}

iConfigFileNew *csConfigIterator::GetConfigFile() const
{
  return (iConfigFileNew*)Config;
}

const char *csConfigIterator::GetSubsection() const
{
  return Subsection;
}

void csConfigIterator::Rewind ()
{
  Node = Config->FirstNode;
}

bool csConfigIterator::DoPrev()
{
  if (!Node->GetPrev()) return false;
  Node = Node->GetPrev();
  return (Node->GetName() != NULL);
}

bool csConfigIterator::DoNext()
{
  if (!Node->GetNext()) return false;
  Node = Node->GetNext();
  return (Node->GetName() != NULL);
}

bool csConfigIterator::CheckSubsection(const char *Key) const
{
  return (SubsectionLength == 0 ||
    strncmp(Key, Subsection, SubsectionLength) == 0);
}

bool csConfigIterator::Prev ()
{
  if (!Subsection) return DoPrev();

  while (1)
  {
    if (!DoPrev()) return false;
    if (CheckSubsection(Node->GetName())) return true;
  }
}

bool csConfigIterator::Next()
{
  if (!Subsection) return DoNext();

  while (1)
  {
    if (!DoNext()) return false;
    if (CheckSubsection(Node->GetName())) return true;
  }
}

const char *csConfigIterator::GetKey(bool Local) const
{
  return Node->GetName() + (Local ? SubsectionLength : 0);
}

int csConfigIterator::GetInt() const
{
  return Node->GetInt();
}

float csConfigIterator::GetFloat() const
{
  return Node->GetFloat();
}

const char *csConfigIterator::GetStr() const
{
  return Node->GetStr();
}

bool csConfigIterator::GetBool() const
{
  return Node->GetBool();
}

const char *csConfigIterator::GetComment() const
{
  return Node->GetComment();
}

/* configuation object */

IMPLEMENT_IBASE(csConfigFile);
  IMPLEMENTS_INTERFACE(iConfigFileNew);
IMPLEMENT_IBASE_END;

csConfigFile::csConfigFile(iBase *pBase)
{
  csConfigFile (NULL, NULL);
  CONSTRUCT_IBASE (pBase);
}

csConfigFile::csConfigFile(const char *file, iVFS *vfs)
{
  CONSTRUCT_IBASE(NULL);

  // create the first and last nodes
  FirstNode = new csConfigNode(NULL);
  LastNode = new csConfigNode(NULL);
  LastNode->InsertAfter(FirstNode);

  Iterators = new csVector();
  Filename = NULL;
  VFS = NULL;
  Dirty = false;

  if (file)
    Load(file, vfs);
}

csConfigFile::~csConfigFile()
{
  Clear();
  delete FirstNode;
  delete LastNode;
  // every iterator holds a reference to this object, so when this is
  // deleted there shouldn't be any iterators left.
  CS_ASSERT(Iterators->Length() == 0);
  delete Iterators;
  if (Filename) delete[] Filename;
  if (VFS) VFS->DecRef();
}

const char* csConfigFile::GetFileName() const
{
  return Filename;
}

iVFS* csConfigFile::GetVFS() const
{
  return VFS;
}

void csConfigFile::SetFileName(const char *fName, iVFS *vfs)
{
  if (Filename) delete[] Filename;
  if (VFS) VFS->DecRef();

  Filename = strnew(fName);
  VFS = vfs;
  if (VFS) VFS->IncRef();
  Dirty = true;
}

bool csConfigFile::Load(const char* fName, iVFS *vfs, bool Merge, bool NewWins)
{
  // We want these changes even if the new config file does not exist:
  if (!Merge)
  {
    // Replace current configuration.  Clear all options, change file name and
    // set the dirty flag.  In case the file cannot be opened our configuration
    // IS dirty.  If we can open it successfully, the flag will be cleared
    // later.
    Clear();
    SetFileName(fName, vfs);
    Dirty = true;
  } // else: Only set dirty flag if we really insert something.

  // load the file buffer
  iDataBuffer *Filedata;
  if (vfs)
  {
    Filedata = vfs->ReadFile(fName);
    if (!Filedata) return false;
  }
  else
  {
    FILE *fp = fopen(fName, "rb");
    if (!fp) return false;
    fseek(fp, 0, SEEK_END);
    size_t Size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    Filedata = new csDataBuffer(Size + 1);
    fread(Filedata->GetData(), sizeof(char), Size, fp);
    fclose(fp);
    Filedata->GetInt8()[Size] = 0;
  }

  // parse the data
  LoadFromBuffer(Filedata->GetInt8(), NewWins);
  Filedata->DecRef();

  if (!Merge)
  {
    // the file has successfully replaced the old configuration. Now the
    // configuration is in sync with the file, so clear the dirty flag.
    Dirty = false;
  }
  // else: LoadFromBuffer has already set the dirty flag if any options
  // have been inserted.
  return true;
}

bool csConfigFile::Save()
{
  if (!Dirty) return true;
  Dirty = false;
  return SaveNow(Filename, VFS);
}

bool csConfigFile::Save(const char *file, iVFS *vfs)
{
  // detect bad parameters
  if (!file) return false;

  // look if we are trying to save to 'our' file. This is only a rough
  // check and will not detect if these are the same files in all cases.
  if (strcmp(Filename, file)==0 && VFS==vfs)
  {
    if (!Dirty) return true;
    Dirty = false;
  }

  return SaveNow(file, vfs);
}

bool csConfigFile::SaveNow(const char *file, iVFS *vfs) const
{
  // @@@ handle different newline codes on different systems
  // @@@ handle base64-encoded data (?)
  csString Filedata;
  csConfigNode *n;

  for (n = FirstNode; n != NULL; n = n->GetNext())
  {
    // don't write first and last nodes
    if (n->GetName() == NULL) continue;

    // write comment
    const char* s = n->GetComment();
    if (s != 0)
    {
      for (const char* b = strchr(s,'\n'); b != NULL; b = strchr(s,'\n'))
      {
        if (*s != '\n' && *s != ';') // Prepend comment character if absent.
          Filedata << "; ";
        Filedata.Append(s, b - s + 1);
        s = b + 1;
      }
      if (*s)
      {
        if (*s != ';')
          Filedata << "; ";
        Filedata << s;
      }
      if (!Filedata.IsEmpty() && Filedata.GetAt(Filedata.Length()-1) != '\n')
        Filedata << '\n';
    }

    // write key line
    Filedata << n->GetName() << " = " << n->GetStr() << '\n';
  }

  if (vfs)
  {
    return vfs->WriteFile(file, Filedata.GetData(), Filedata.Length());
  }
  else
  {
    FILE *fp = fopen(file, "wb");
    if (!fp) return false;
    fwrite(Filedata.GetData(), sizeof(char), Filedata.Length(), fp);
    fclose(fp);
    return true;
  }
}

void csConfigFile::Clear()
{
  // delete all nodes but the first and last one
  FirstNode->DeleteDataNodes();
  // rewind all iterators
  for (long i = 0; i < Iterators->Length(); i++)
  {
    csConfigIterator *it = (csConfigIterator*)Iterators->Get(i);
    it->Rewind();
  }
  Dirty = true;
}

iConfigIterator *csConfigFile::Enumerate(const char *Subsection)
{
  iConfigIterator *it = new csConfigIterator(this, Subsection);
  Iterators->Push(it);
  return it;
}

bool csConfigFile::KeyExists(const char *Key) const
{
  return (FindNode(Key) != 0);
}

bool csConfigFile::SubsectionExists(const char *Subsection) const
{
  return (FindNode(Subsection, true) != 0);
}

int csConfigFile::GetInt(const char *Key, int Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetInt() : Def;
}

float csConfigFile::GetFloat(const char *Key, float Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetFloat() : Def;
}

const char *csConfigFile::GetStr(const char *Key, const char *Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetStr() : Def;
}

bool csConfigFile::GetBool(const char *Key, bool Def) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetBool() : Def;
}

const char *csConfigFile::GetComment(const char *Key) const
{
  csConfigNode *Node = FindNode(Key);
  return Node ? Node->GetComment() : NULL;
}

void csConfigFile::SetStr (const char *Key, const char *Val)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) Node = CreateNode(Key);
  if (Node)
  {
    const char* s = Node->GetStr();
    if ((s && !Val) || (!s && Val) || (Val && strcmp(s,Val) != 0))
    { // The new value differs from the old.
      Node->SetStr(Val);
      Dirty = true;
    }
  }
}

void csConfigFile::SetInt (const char *Key, int Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);
  if (Node && (Create || Value != Node->GetInt()))
  {
    Node->SetInt(Value);
    Dirty = true;
  }
}

void csConfigFile::SetFloat (const char *Key, float Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);
  if (Node && (Create || Value != Node->GetFloat()))
  {
    Node->SetFloat(Value);
    Dirty = true;
  }
}

void csConfigFile::SetBool (const char *Key, bool Value)
{
  csConfigNode *Node = FindNode(Key);
  bool const Create = !Node;
  if (Create) Node = CreateNode(Key);
  if (Node && (Create || Value != Node->GetBool()))
  {
    Node->SetBool(Value);
    Dirty = true;
  }
}

bool csConfigFile::SetComment (const char *Key, const char *Text)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) return false;
  const char* s = Node->GetComment();
  if ((s && !Text) || (!s && Text) || (Text && strcmp(s,Text) != 0))
  { // The new comment differs from the old.
    Node->SetComment(Text);
    Dirty = true;
  }
  return true;
}

void csConfigFile::DeleteKey(const char *Name)
{
  csConfigNode *Node = FindNode(Name);
  if (!Node) return;

  // look for iterators on that node
  for (long i = 0; i < Iterators->Length(); i++)
  {
    csConfigIterator *it = (csConfigIterator*)Iterators->Get(i);
    if (it->Node == Node) it->Prev();
  }

  // remove and delete the node
  Node->Remove();
  delete Node;
  Dirty = true;
}

void csConfigFile::LoadFromBuffer(char *Filedata, bool overwrite)
{
  csString CurrentComment;
  char* s = 0;
  int SkipCount = 0;
  bool LastLine = false;
  for (int Line = 1; !LastLine; Line++, Filedata = s + SkipCount)
  {
    s = Filedata + strcspn(Filedata, "\n\r");
    LastLine = (*s == 0);
    // Advance past LF, CR, or CRLF.
    SkipCount = (!LastLine && *s == '\r' && *(s+1) == '\n' ? 2 : 1);
    *s = 0;

    // Filedata is now a null-terminated string containing the current line
    // skip initial whitespace
    while (isspace(*Filedata)) Filedata++;
    // delete whitespace at end of line
    char* t = s;
    if (Filedata + 1 != t)
      while (isspace(*(t-1))) t--;
        *t = 0;

    // check if this is a comment or a blank line
    if (*Filedata == '\0' || *Filedata == ';')
      CurrentComment << Filedata << '\n';
    else
    {
      // this is a key. Find equal sign
      t = strchr(Filedata, '=');
      // if no equal sign, this is an invalid line
      if (!t)
      {
        fprintf(stderr, "Missing `=' on line %d of %s\n", Line,
	  (Filename ? Filename : "configuration data"));
        continue;
      }
      // check for missing key name
      if (t == Filedata)
      {
        fprintf(stderr, "Missing key name (before `=') on line %d of %s\n",
	  Line, (Filename ? Filename : "configuration data"));
        continue;
      }
      // delete whitespace before equal sign
      char *u = t;
      while (isspace(*(u-1))) u--;
      *u = 0;
      // check if node already exists and create node
      csConfigNode *Node = FindNode(Filedata);
      if (Node && !overwrite) continue;
      if (!Node) Node = CreateNode(Filedata);
      // skip whitespace after equal sign
      Filedata = t+1;
      while (isspace(*Filedata)) Filedata++;
      // extract key value
      Node->SetStr(Filedata);
      // apply comment
      if (!CurrentComment.IsEmpty())
      {
        Node->SetComment(CurrentComment);
        CurrentComment.Clear();
      }
      // set dirty flag
      Dirty = true;
    }
  }
}

csConfigNode *csConfigFile::FindNode(const char *Name, bool isSubsection) const
{
  if (!Name) return NULL;

  csConfigNode *n = FirstNode;
  const int sz = (isSubsection ? strlen(Name) : 0);
  while (n)
  {
    const char* s = n->GetName();
    if (s && ((isSubsection && strncmp(s, Name, sz) == 0) ||
       (strcmp(s, Name) == 0)))
      return n;
    n = n->GetNext();
  }
  return NULL;
}

csConfigNode *csConfigFile::CreateNode(const char *Name)
{
  if (!Name) return NULL;

  csConfigNode *n = new csConfigNode(Name);
  n->InsertAfter(LastNode->GetPrev());
  Dirty = true;
  return n;
}

void csConfigFile::RemoveIterator(csConfigIterator *it) const
{
  int n = Iterators->Find(it);
  CS_ASSERT(n != -1);
  Iterators->Delete(n);
}
