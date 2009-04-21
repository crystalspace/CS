/*
  Copyright (C) 2008 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#include "csutil/refarr.h"

struct iDocumentSystem;
struct iObjectRegistry;
struct iVFS;

class OptimiseData
{
public:
  OptimiseData(iObjectRegistry* objReg, iVFS* vfs);

  void Run(csString in, csString out);

private:
  void CollectData(csString in);
  void SortData();
  void WriteData(csString out);

  void ParseMeshObj(csArray<csString>& libsNeeded, csArray<csString>& materialsNeeded,
    csString& mapInPath, csRef<iDocumentNode> meshobj);
  void ParseMeshFact(csRef<iDocumentNode>& meshFact, csRef<iDocumentNode>& tempDocRoot,
    csRefArray<iDocumentNode>& tempMats);

  csRef<iDocumentSystem> docSys;
  iObjectRegistry* objReg;
  iVFS* vfs;

  // For collection.
  csArray<csString> mapNames;
  csArray<csString> mapInPaths;
  csRefArray<iDocumentNode> maps;
  csRefArray<iDocumentNode> materials;
  csRefArray<iDocumentNode> meshFacts;
  csRefArray<iDocumentNode> textures;
  csRefArray<iDocumentNode> addons;
  csRefArray<iDocumentNode> lightmaps;
  csArray<csString> addonNames;
  bool addonLib;

  // For sorting.
  csRefArray<iDocument> meshFactsOut;
  csRefArray<iDocument> mapsOut;

  // False if we want to have a seperate file for each meshfact, addon, etc.
  bool compact;
};
