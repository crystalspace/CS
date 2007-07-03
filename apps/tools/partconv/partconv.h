/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef PARTCONV_H
#define PARTCONV_H

class PartConv
{
public:
  PartConv (iObjectRegistry* objectRegistry);

  // Main function
  void Main ();

private:
  iObjectRegistry* objectRegistry;
  csRef<iVFS> vfs;
  csRef<iCommandLineParser> commandLine;
  csRef<iSyntaxService> syntaxService;

  // Map plugin short-name to full name
  csHash<csString, csString> pluginNameHash;

  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "apps/tools/partconv/partconv.tok"
#include "cstool/tokenlist.h"

  enum
  {
    PARTCONV_PARTSYS_PARTICLES,
    PARTCONV_PARTSYS_PARTICLES_FACT
  };
  csStringHash particlePlugins;
  
  bool unconvertedTags;

  // Helpers
  void ReportError (const char* description, ...);
  void Report (int severity, const char* description, ...);

  // Clone node
  void CloneNode (iDocumentNode* from, iDocumentNode* to) const;

  // Analyze plugin section
  void AnalyzePluginSection (iDocumentNode* topNode);

  // Convert system
  void ConvertDocument (iDocumentNode* root);

  // Convert all mesh factories
  void ConvertMeshFactories (iDocumentNode* topNode);

  // Convert all mesh objects
  void ConvertMeshObjects (iDocumentNode* topNode);

  // Convert mesh factory
  void ConvertMeshFactory (iDocumentNode* factoryNode);

  // Convert mesh object
  void ConvertMeshObject (iDocumentNode* objectNode);

  // Convert from "particles" to "particles"
  void ConvertParticlesFactoryParams (iDocumentNode* paramsNode, 
    iDocumentNode* factNode);
  void ConvertParticlesObjectParams (iDocumentNode* paramsNode,
    iDocumentNode* objNode);
  void ConvertParticlesCommonParams (iDocumentNode* oldParams,
    iDocumentNode* newParams);
};

#endif

