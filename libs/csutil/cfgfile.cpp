
#include "cssysdef.h"
#include "ivfs.h"
#include "csutil/cfgfile.h"
#include "csutil/databuf.h"
#include "csutil/csstring.h"

/* config node */

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

const char *csConfigNode::GetName()
{
  return Name;
}

csConfigNode *csConfigNode::GetPrev()
{
  return Prev;
}

csConfigNode *csConfigNode::GetNext()
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
  if (Data) delete[] Data;

  char output [20];
  sprintf (output, "%d", n);
  SetStr (output);
}

void csConfigNode::SetFloat(float f)
{
  if (Data) delete[] Data;

  char output [20];
  sprintf (output, "%g", f);
  SetStr (output);
}

void csConfigNode::SetBool(bool b)
{
  if (Data) delete[] Data;

  if (b) SetStr("true");
  else SetStr("false");
}

void csConfigNode::SetComment(const char *s)
{
  if (Comment) delete[] Comment;
  Comment = strnew(s);
}

const char *csConfigNode::GetStr()
{
  if (!Data) return "";
  return Data;
}

int csConfigNode::GetInt()
{
  if (!Data) return 0;
  return atoi(Data);
}

float csConfigNode::GetFloat()
{
  if (!Data) return 0.0;
  return atof(Data);
}

bool csConfigNode::GetBool()
{
  if (!Data) return false;
  if (strcasecmp(Data, "yes") == 0) return true;
  if (strcasecmp(Data, "true") == 0) return true;
  if (strcasecmp(Data, "on") == 0) return true;
  return false;
}

const char *csConfigNode::GetComment()
{
  return Comment;
}

/* config iterator */

IMPLEMENT_IBASE(csConfigIterator);
  IMPLEMENTS_INTERFACE(iConfigIterator);
IMPLEMENT_IBASE_END;

csConfigIterator::csConfigIterator(const csConfigFile *c, const char *sub)
{
  CONSTRUCT_IBASE(NULL);
  Config = c;
  Node = Config->FirstNode;
  Subsection = strnew(sub);
  if (Subsection) SubsectionLength = strlen(Subsection);
  else SubsectionLength = 0;
  ((csConfigFile*)Config)->IncRef();
}

csConfigIterator::~csConfigIterator()
{
  Config->RemoveIterator(this);
  if (Subsection) delete[] Subsection;
  ((csConfigFile*)Config)->DecRef();
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

bool CheckSubsection(const char *Key, const char *Subsection) {
  return (strstr(Key, Subsection) == Key);
}

bool csConfigIterator::Prev ()
{
  if (!Subsection) return DoPrev();

  while (1) {
    if (!DoPrev()) return false;
    if (CheckSubsection(Node->GetName(), Subsection)) return true;
  }
}

bool csConfigIterator::Next()
{
  if (!Subsection) return DoNext();

  while (1) {
    if (!DoNext()) return false;
    if (CheckSubsection(Node->GetName(), Subsection)) return true;
  }
}

const char *csConfigIterator::GetKey(bool Local) const
{
  if (Local) return Node->GetName() + SubsectionLength;
  else return Node->GetName();
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

csConfigFile::csConfigFile(const char *file, iVFS *vfs)
{
  CONSTRUCT_IBASE(NULL);

  // create the first and last nodes
  FirstNode = new csConfigNode(NULL);
  LastNode = new csConfigNode(NULL);
  LastNode->InsertAfter(FirstNode);

  Iterators = new csVector();
  Filename = NULL;
  vfs = NULL;
  Dirty = false;
  
  if (file) Load(file, vfs);
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

const char *csConfigFile::GetFileName() const
{
  return Filename;
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

bool csConfigFile::Load (const char* fName, iVFS *vfs, bool Insert, bool Overwrite)
{
  // We want these changes even if the new config file does not exist:
  if (!Insert) {
    // replace current configuration. Clear all options, change file name and
    // set the dirty flag. In case the file cannot be opened our configuration
    // IS dirty. If we can open it successfully, the flag will be cleared later.
    Clear();
    SetFileName(fName, vfs);
    Dirty = true;
  } // else: Only set dirty flag if we really insert something.

  // load the file buffer
  iDataBuffer *Filedata;
  if (vfs) {
    Filedata = vfs->ReadFile(fName);
    if (!Filedata) return false;
  } else {
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
  LoadFromBuffer(Filedata->GetInt8(), Overwrite);
  Filedata->DecRef();

  if (!Insert) {
    // the file has successfully replaced the old configuration. Now the
    // configuration is in sync with the file, so clear the dirty flag.
    Dirty = false;
  }
  // else: LoadFromBuffer has already set the dirty flag if any options
  // have been inserted.
  return true;
}

bool csConfigFile::Save() const
{
  if (!Dirty) return true;
  ((csConfigFile*)this)->Dirty = false;
  return SaveNow(Filename, VFS);
}

bool csConfigFile::Save(const char *file, iVFS *vfs) const
{
  // detect bad parameters
  if (!file) return false;

  // look if we are trying to save to 'our' file. This is only a rough
  // check and will not detect if these are the same files in all cases.
  if (strcmp(Filename, file)==0 && VFS==vfs) {
    if (!Dirty) return true;
    ((csConfigFile*)this)->Dirty = false;
  }

  return SaveNow(file, vfs);
}

bool csConfigFile::SaveNow(const char *file, iVFS *vfs) const
{
  // @@@ handle different newline codes on different systems
  // @@@ handle base64-encoded data (?)
  csString Filedata;
  csConfigNode *n;

  for (n=FirstNode;n!=NULL;n=n->GetNext())
  {
    // don't write first and last nodes
    if (n->GetName() == NULL) continue;

    // write comment
    if (n->GetComment())
    {
      char *a = (char*)(n->GetComment()), *b;

      for (b=strchr(a,'\n'); b!=NULL; b=strchr(a,'\n'))
      {
        *b = NULL;
        Filedata += "; ";
        Filedata += a;
        Filedata += '\n';
        *b = '\n';
        a = b+1;
      }
      Filedata += "; ";
      Filedata += a;
      Filedata += '\n';
    }

    // write key line
    Filedata += n->GetName();
    Filedata += " = ";
    Filedata += n->GetStr();
    Filedata += '\n';
  }

  if (VFS) {
    return vfs->WriteFile(file, Filedata.GetData(), Filedata.Length());
  } else {
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
  long i;
  for (i=0;i<Iterators->Length();i++)
  {
    csConfigIterator *it = (csConfigIterator*)Iterators->Get(i);
    it->Rewind();
  }

  Dirty = true;
}

iConfigIterator *csConfigFile::Enumerate(const char *Subsection) const
{
  iConfigIterator *it = new csConfigIterator(this, Subsection);
  Iterators->Push(it);
  return it;
}
  
bool csConfigFile::KeyExist(const char *Key) const
{
  return (FindNode(Key) != 0);
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
  if (Node) {
    Node->SetStr(Val);
    Dirty = true;
  }
}

void csConfigFile::SetInt (const char *Key, int Value)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) Node = CreateNode(Key);
  if (Node) {
    Node->SetInt(Value);
    Dirty = true;
  }
}

void csConfigFile::SetFloat (const char *Key, float Value)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) Node = CreateNode(Key);
  if (Node) {
    Node->SetFloat(Value);
    Dirty = true;
  }
}

void csConfigFile::SetBool (const char *Key, bool Value)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) Node = CreateNode(Key);
  if (Node) {
    Node->SetBool(Value);
    Dirty = true;
  }
}

bool csConfigFile::SetComment (const char *Key, const char *Text)
{
  csConfigNode *Node = FindNode(Key);
  if (!Node) return false;
  Node->SetComment(Text);
  Dirty = true;
  return true;
}

void csConfigFile::DeleteKey(const char *Name)
{
  csConfigNode *Node = FindNode(Name);
  if (!Node) return;
  
  // look for iterators on that node
  long i;
  for (i=0;i<Iterators->Length();i++) {
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

  while (1) {
    char *s = Filedata + strcspn(Filedata, "\n\r"), *t;
    bool LastLine = (*s == 0);
    *s = 0;

    // Filedata is now a null-terminated string containing the current line
    // skip initial whitespace
    while (*Filedata == ' ') Filedata++;
    // check for empty line
    if (*Filedata == 0) goto NextLine;
    // delete whitespace at end of line
    t = s;
    while (*(t-1) == ' ') t--;
    *t = 0;

    // check if this is a comment
    if (*Filedata == ';') {
      // this is a comment. skip whitespace between comment sign and text
      Filedata++;
      while (*Filedata == ' ') Filedata++;

      if (CurrentComment.Length() > 0)
        CurrentComment += '\n';
      CurrentComment += Filedata;
    } else {
      // this is a key. Find equal sign
      t = strchr(Filedata, '=');
      // if no equal sign, this is an invalid line
      if (!t) goto NextLine;
      // check for missing key name
      if (t == Filedata) goto NextLine;
      // delete whitespace before equal sign
      char *u = t;
      while (*(u-1) == ' ') u--;
      *u = 0;
      // check if node already exists and create node
      csConfigNode *Node = FindNode(Filedata);
      if (Node && !overwrite) goto NextLine;
      if (!Node) Node = CreateNode(Filedata);
      // skip whitespace after equal sign
      Filedata = t+1;
      while (*Filedata == ' ') Filedata++;
      // extract key value
      Node->SetStr(Filedata);
      // apply comment
      Node->SetComment(CurrentComment);
      CurrentComment.Clear();
      // set dirty flag
      Dirty = true;
    }

    NextLine:
    if (LastLine) break;
    Filedata = s+1;
  }
}

csConfigNode *csConfigFile::FindNode(const char *Name) const
{
  if (!Name) return NULL;

  csConfigNode *n = FirstNode;
  while (n) {
    if (n->GetName())
      if (strcmp(n->GetName(), Name) == 0)
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
