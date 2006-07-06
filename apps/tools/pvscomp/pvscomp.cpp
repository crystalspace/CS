/*
    Copyright (C) 2006 by Benjamin Stover

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

#include "csplugincommon/pvsdata/pvsdata.h"
#include "cstool/initapp.h"
#include "csgeom/statickdtree.h"
#include "iengine/mesh.h"
#include "iutil/object.h"

CS_IMPLEMENT_APPLICATION

static csStaticKDTree* MakeTestPVSTree () 
{
  // Here are the objects in pvstest
  // Object "floor"
  // extends in x and z directions from about -3 to +3, y=-1
  // around (0, -1, 0)
  // Object "wallN"
  // around (0, 0, 5)
  // Object "Suzanne"
  // around (0,0,-6)

  // The tree we will construct for them
  // Root:  (0,0,4) on Z-axis (fws) []
  //   <4: (0,0,0) on X-axis (fs) []
  //     <0: leaf node (fs) [fs]
  //     >0: (0,1,0) on Y-axis (fs) []
  //       <1: leaf node (f) [fs]
  //       >1: leaf node (s) [s]
  //   >4: (1,0,0) on X-axis (w) [w]
  //     <1: leaf node (w) [w]
  //     >1: leaf node (w) [w]
  csArray<csStaticKDTreeObject*> emptylist;
  csStaticKDTree *root, *frnt1, *back1, *frnt2, *back2, *frnt3, *back3;

  root = new csStaticKDTree(NULL, 1, CS_ZAXIS, 4);
    frnt1 = new csStaticKDTree(root, 1, CS_XAXIS, 0);
      frnt2 = new csStaticKDTree(frnt1, 1, emptylist);
      back2 = new csStaticKDTree(frnt1, 0, CS_YAXIS, .5);
        frnt3 = new csStaticKDTree(back2, 1, emptylist);
        back3 = new csStaticKDTree(back2, 0, emptylist);
    back1 = new csStaticKDTree(root, 0, CS_XAXIS, 1);
      frnt2 = new csStaticKDTree(back1, 1, emptylist);
      back2 = new csStaticKDTree(back1, 0, emptylist);

  return root;
}

static void CreateDummyNodeData (csStaticKDTree *node)
{
  csPVSNodeData *data = new csPVSNodeData (NULL, 0);
  node->SetNodeData (data);

  if (!node->IsLeafNode ())
  {
    CreateDummyNodeData (node->GetChild1 ());
    CreateDummyNodeData (node->GetChild2 ());
  }
}

class Compiler
{
  csRef<iEngine> engine;
  iObjectRegistry* reg;
  iMeshList* meshlist;
  csStaticKDTree* pvstree;

public:
  Compiler ();
  ~Compiler ();
  bool Initialize (int argc, char** argv);
  bool LoadWorld (const char* c);
  void PrintObjects ();
  void Save (const char* name);
};

Compiler::Compiler () : engine(NULL), reg(NULL), meshlist(NULL), pvstree(NULL)
{
}

Compiler::~Compiler ()
{
  delete pvstree;
}

bool Compiler::Initialize (int argc, char** argv)
{
  reg = csInitializer::CreateEnvironment (argc, argv);
  if (!csInitializer::RequestPlugins (reg,
      CS_REQUEST_VFS,
      CS_REQUEST_NULL3D,
      CS_REQUEST_ENGINE,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_END))
    return false;

//  csBaseEventHandler::Initialize (reg);

  engine = csQueryRegistry<iEngine> (reg);
  if (!engine.IsValid())
    return false;

  csInitializer::OpenApplication (reg);

  return true;
}

bool Compiler::LoadWorld (const char* c)
{
  csStaticKDTree* tree = MakeTestPVSTree ();
  CreateDummyNodeData (tree);
  csRef<iLoader> loader = csQueryRegistry<iLoader> (reg);
  if (loader.IsValid ())
  {
    bool ret = loader->LoadMapFile (c);
    engine->PrepareMeshes ();
    meshlist = engine->GetMeshes ();
    return ret;
  }
  else
  {
    printf("Warning:  loader plugin not loaded.\n");
    return false;
  }
}

void Compiler::PrintObjects ()
{
  int count = meshlist->GetCount ();
  for (int i = 0; i < count; i++)
  {
    iMeshWrapper* wrapper = meshlist->Get(i);
    printf("Name is %s\n", wrapper->QueryObject ()->GetName ());
  }
}

void Compiler::Save (const char* name)
{
  csSavePVSDataFile (reg, "/this/test.pvs", pvstree);
}

int main (int argc, char** argv)
{
  Compiler pvscomp;
  if (!pvscomp.Initialize (argc, argv))
  {
    printf("Couldn't initialize CS.\n");
    exit (1);
  }
  if (!pvscomp.LoadWorld ("/lev/pvstest/world"))
  {
    printf("Couldn't load world file.\n");
    exit (1);
  }

  pvscomp.PrintObjects ();
//  pvscomp.Save ("/this/test.pvs");

  return 0;
}
