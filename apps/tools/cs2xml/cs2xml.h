/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS2XML_H
#define __CS2XML_H

#include <stdarg.h>
#include "csutil/ref.h"

struct iVFS;
struct iCommandLineParser;
struct iObjectRegistry;
struct iFile;
struct iDocumentNode;
class csParser;

/**
 * Main class.
 */
class Cs2Xml
{
public:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iCommandLineParser> cmdline;

  void ReportError (const char* description, ...);

  void ParseGeneral (const char* parent_token,
  	csParser* parser, csRef<iDocumentNode>& parent, char* buf);
  void ParseGeneral1 (long cmd, char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParsePortal (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname, csRef<iDocumentNode>& portal_node);
  void ParseWarp (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname, csRef<iDocumentNode>& portal_node);
  void ParseAnimate (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseMaterial (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseMaterialGroup (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseAmbient (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseColor (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParsePolygon (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseQ (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseTimes (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseRotPart (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseAttach (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParsePath (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseSetupMesh (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseFade (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseFarPlane (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseUVShift (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseVector (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseCurveControl (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseDuration (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseFrame (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseVertex (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseV (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseColors (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseUVA (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseUV (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseVertices (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseTriangles (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseTexture (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseMatrix (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseFilename (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseTriangle (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseSingle (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseSlope (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseGenerate (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseHalo (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseHaloDefault(csRef<iDocumentNode> child, char* params);
  void ParseDoFlatten (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseFlatten (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseFog (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseKey (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseAging (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseRegularParticles (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseIsoSpace (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseIsoSize (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseParticleSize (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseDropSize (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseCorrectSeams (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseBC (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseBH (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseSysDist (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseGrid (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseBlocks (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseEmitCylinder (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseHazeCone (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseBox (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseEmitFixed (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseEmitSphere (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseF (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseNum (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParsePriority (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseStart (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseLOD (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseLODDistance (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseLODCost (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseDirLight (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseLight (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseTransparent (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseScale (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseType (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);
  void ParseOther (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname);

  bool IsEmpty (const char* in);
  bool IsNumeric (const char* in);
  bool IsString (const char* in);
  bool IsBoolean (const char* in, bool& val);
  int IsNumberList (const char* in);
  // If remove_under is true, underscores are removed.
  char* ToLower (const char* in, bool remove_under);

  csRef<iDocumentNode> CreateValueNode (csRef<iDocumentNode>& parent,
  	const char* name, const char* value);
  csRef<iDocumentNode> CreateValueNodeAsInt (csRef<iDocumentNode>& parent,
  	const char* name, int value);
  csRef<iDocumentNode> CreateValueNodeAsFloat (csRef<iDocumentNode>& parent,
  	const char* name, float value);

  void ParseMatrix (csParser *parser, csRef<iDocumentNode>& parent,
  	char* buf);

  // Convert a file. If backup == true, creates a backup of original with
  // .bak appended. Returns false if failure.
  bool ConvertFile (const char* vfspath, bool backup);
  // Convert a VFS directory.
  void ConvertDir (const char* vfspath, bool backup);

  // Test if a file is a CS file that can be converted to XML.
  bool TestCSFile (const char* vfspath);

public:
  Cs2Xml (iObjectRegistry* object_reg);
  ~Cs2Xml ();

  void Main ();
};

#endif // __CS2XML_H

