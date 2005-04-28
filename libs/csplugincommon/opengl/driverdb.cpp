/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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
#include "imap/services.h"
#include "iutil/cfgmgr.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "csutil/cfgdoc.h"
#include "csutil/hash.h"
#include "csutil/regexp.h"
#include "csutil/xmltiny.h"

#include "csplugincommon/opengl/driverdb.h"
#include "csplugincommon/opengl/glcommon2d.h"

CS_LEAKGUARD_IMPLEMENT (csGLDriverDatabase);

class csDriverDBReader
{
private:
  csGLDriverDatabase* db;
  csStringHash& tokens;
  iConfigManager* cfgmgr;
  iSyntaxService* synsrv;
  int usedCfgPrio;

  enum FulfillConditions
  {
    All,
    One
  };

  csHash<csRef<csConfigDocument>, csStrKey> configs;
public:
  CS_LEAKGUARD_DECLARE (csDriverDBReader);

  csDriverDBReader (csGLDriverDatabase* db, iConfigManager* cfgmgr, 
    iSyntaxService* synsrv, int usedCfgPrio);

  bool Apply (iDocumentNode* node);

  bool ParseConditions (iDocumentNode* node, 
    bool& result, bool negate = false);
  bool ParseRegexp (iDocumentNode* node, bool& result);
  bool ParseCompareVer (iDocumentNode* node, bool& result);

  bool ParseConfigs (iDocumentNode* node);
  bool ParseRules (iDocumentNode* node);
};

CS_LEAKGUARD_IMPLEMENT (csDriverDBReader);

csDriverDBReader::csDriverDBReader (csGLDriverDatabase* db, 
				    iConfigManager* cfgmgr, 
				    iSyntaxService* synsrv, 
				    int usedCfgPrio) :
  tokens(db->tokens)
{
  csDriverDBReader::db = db;
  csDriverDBReader::cfgmgr = cfgmgr;
  csDriverDBReader::synsrv = synsrv;
  csDriverDBReader::usedCfgPrio = usedCfgPrio;
}

bool csDriverDBReader::Apply (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it (node->GetNodes ());
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID token = tokens.Request (child->GetValue ());

    switch (token)
    {
      case csGLDriverDatabase::XMLTOKEN_USECFG:
	{
	  const char* cfgname = child->GetContentsValue ();
	  csRef<csConfigDocument> cfg (configs.Get (cfgname, 0));
	  if (!cfg.IsValid ())
	  {
	    synsrv->Report (
	      "crystalspace.canvas.openglcommon.driverdb",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "unknown config %s", cfgname);
	  }
	  else
	  {
	    cfgmgr->AddDomain (cfg, usedCfgPrio);
	    db->addedConfigs.Push (cfg);
	  }
	}
	break;
      default:
	synsrv->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

bool csDriverDBReader::ParseConditions (iDocumentNode* node, 
					bool& result, 
					bool negate)
{
  FulfillConditions fulfill = All;
  const char* fulfillAttr = node->GetAttributeValue ("fulfill");
  if (fulfillAttr != 0)
  {
    if (strcmp (fulfillAttr, "one") == 0)
      fulfill = One;
    else if (strcmp (fulfillAttr, "all") == 0)
      fulfill = All;
    else
    {
      synsrv->Report (
	"crystalspace.canvas.openglcommon.driverdb",
	CS_REPORTER_SEVERITY_WARNING,
	node,
	"Invalid 'fulfill' attribute '%s'", fulfillAttr);
      return false;
    }
  }

  csRef<iDocumentNodeIterator> it (node->GetNodes ());
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID token = tokens.Request (child->GetValue ());

    bool lastResult = false;

    switch (token)
    {
      case csGLDriverDatabase::XMLTOKEN_MATCH:
	if (!ParseConditions (child, lastResult))
	  return false;
	break;
      case csGLDriverDatabase::XMLTOKEN_NEGATE:
	if (!ParseConditions (child, lastResult, true))
	  return false;
	break;
      case csGLDriverDatabase::XMLTOKEN_REGEXP:
	if (!ParseRegexp (child, lastResult))
	  return false;
	break;
      case csGLDriverDatabase::XMLTOKEN_COMPAREVER:
	if (!ParseCompareVer (child, lastResult))
	  return false;
	break;
      default:
	synsrv->ReportBadToken (child);
	return false;
    }

    switch (fulfill)
    {
      case One:
	if (lastResult ^ negate)
	{
	  result = true;
	  return true;
	}
	break;
      case All:
	if (!(lastResult ^ negate))
	{
	  result = false;
	  return true;
	}
	break;
    }
  }
  result = (fulfill == All);
  return true;
}

bool csDriverDBReader::ParseRegexp (iDocumentNode* node, bool& result)
{
  const char* string = node->GetAttributeValue ("string");
  if (string == 0)
  {
    synsrv->Report (
      "crystalspace.canvas.openglcommon.driverdb",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No 'string' attribute");
    return false;
  }
  const char* pattern = node->GetAttributeValue ("pattern");
  if (pattern == 0)
  {
    synsrv->Report (
      "crystalspace.canvas.openglcommon.driverdb",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No 'pattern' attribute");
    return false;
  }
  
  const char* str = db->ogl2d->GetRendererString (string);
  if (str == 0)
  {
    result = false;
    return true;
  }

  csRegExpMatcher re (pattern);
  result = (re.Match (str) == csrxNoError);
  return true;
}

enum Relation
{
  eq = 0,
  neq,
  lt,
  le,
  gt,
  ge
};
static const Relation nonLastDigitRels[6] = {eq, neq, le, le, ge, ge};

static bool Compare (int a, int b, Relation rel)
{
  switch (rel)
  {
    case eq: return a == b;
    case neq: return a != b;
    case lt: return a < b;
    case le: return a <= b;
    case gt: return a > b;
    case ge: return a >= b;
  }
  return false;
}

bool csDriverDBReader::ParseCompareVer (iDocumentNode* node, bool& result)
{
  const char* version = node->GetAttributeValue ("version");
  if (version == 0)
  {
    synsrv->Report (
      "crystalspace.canvas.openglcommon.driverdb",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No 'version' attribute");
    return false;
  }
  const char* relation = node->GetAttributeValue ("relation");
  if (relation == 0)
  {
    synsrv->Report (
      "crystalspace.canvas.openglcommon.driverdb",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No 'relation' attribute");
    return false;
  }

  const char* space = strchr (relation, ' ');
  if (space == 0)
  {
    synsrv->Report (
      "crystalspace.canvas.openglcommon.driverdb",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Malformed 'relation'");
    return false;
  }
  int rellen = space - relation;

  Relation rel;
  if (strncmp (relation, "eq", rellen) == 0)
    rel = eq;
  else if (strncmp (relation, "neq", rellen) == 0)
    rel = neq;
  else if (strncmp (relation, "lt", rellen) == 0)
    rel = lt;
  else if (strncmp (relation, "le", rellen) == 0)
    rel = le;
  else if (strncmp (relation, "gt", rellen) == 0)
    rel = gt;
  else if (strncmp (relation, "ge", rellen) == 0)
    rel = ge;
  else
  {
    csString relstr;
    relstr.Append (relation, rellen);

    synsrv->Report (
      "crystalspace.canvas.openglcommon.driverdb",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "Unknown relation '%s'", relstr.GetData());
    return false;
  }

  result = false;
  const char* curpos1 = db->ogl2d->GetVersionString (version);
  const char* curpos2 = relation + rellen + 1;

  if (curpos1 && (*curpos1 != 0) && curpos2 && (*curpos2 != 0))
  {
    while (1)
    {
      size_t nextpos1 = strspn (curpos1, "0123456789");
      if (nextpos1 == 0) break;
      size_t nextnextpos1 = strspn (curpos1 + nextpos1 + 1, "0123456789");
      size_t nextpos2 = strspn (curpos2, "0123456789");
      if (nextpos2 == 0) break;
      size_t nextnextpos2 = strspn (curpos2 + nextpos2 + 1, "0123456789");

      bool last = (nextnextpos1 == 0) || (nextnextpos2 == 0);

      int v1, v2;
      if (sscanf (curpos1, "%d", &v1) != 1) break;
      if (sscanf (curpos2, "%d", &v2) != 1) break;

      if (!Compare (v1, v2, last ? rel : nonLastDigitRels[rel]))
	break;

      if (last) 
      {
	result = true;
	break;
      }

      curpos1 += nextpos1 + 1;
      curpos2 += nextpos2 + 1;
    }
  }

  return true;
}

bool csDriverDBReader::ParseConfigs (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it (node->GetNodes ());
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID token = tokens.Request (child->GetValue ());

    switch (token)
    {
      case csGLDriverDatabase::XMLTOKEN_CONFIG:
	{
	  const char* name = child->GetAttributeValue ("name");
	  if (!name)
	  {
	    synsrv->Report (
	      "crystalspace.canvas.openglcommon.driverdb",
	      CS_REPORTER_SEVERITY_WARNING,
	      child,
	      "<config> has no name");
	    return false;
	  }
	  csRef<csConfigDocument> cfg (configs.Get (name, 0));
	  if (!cfg.IsValid ())
	  {
	    cfg.AttachNew (new csConfigDocument ());
	    configs.Put (name, cfg);
	  }
	  cfg->LoadNode (child, true, true);
	}
	break;
      default:
	synsrv->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

bool csDriverDBReader::ParseRules (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it (node->GetNodes ());
  while (it->HasNext())
  {
    csRef<iDocumentNode> child = it->Next();
    if (child->GetType() != CS_NODE_ELEMENT) continue;
    csStringID token = tokens.Request (child->GetValue ());

    switch (token)
    {
      case csGLDriverDatabase::XMLTOKEN_RULE:
	{
	  const char* rulePhase = child->GetAttributeValue ("phase");
	  if (rulePhase == 0) rulePhase = "";
	  if (strcmp (db->rulePhase, rulePhase) != 0)
	    continue;

	  csRef<iDocumentNode> conditions = child->GetNode ("conditions");
	  csRef<iDocumentNode> applicable = child->GetNode ("applicable");
	  csRef<iDocumentNode> notapplicable = child->GetNode ("notapplicable");

	  bool condTrue = true;
	  bool applied = false;
	  if (conditions.IsValid())
	  {
	    if (!ParseConditions (conditions, condTrue))
	      return false;
	  }
	  if (condTrue)
	  {
	    if (applicable.IsValid ())
	    {
	      if (!Apply (applicable)) return false;
	      applied = true;
	    }
	  }
	  else
	  {
	    if (notapplicable.IsValid ())
	    {
	      if (!Apply (notapplicable)) return false;
	      applied = true;
	    }
	  }
	  if (applied)
	  {
	    const char* descr = child->GetAttributeValue ("description");
	    if (descr != 0)
	    {
	      db->Report (CS_REPORTER_SEVERITY_NOTIFY,
		"Applied: %s", descr);
	    }
	  }
	}
	break;
      default:
	synsrv->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------------//

void csGLDriverDatabase::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (ogl2d->object_reg, severity, 
    "crystalspace.canvas.openglcommon.driverdb", msg, args);
  va_end (args);
}

csGLDriverDatabase::csGLDriverDatabase ()
{
  InitTokenTable (tokens);
}

csGLDriverDatabase::~csGLDriverDatabase ()
{
}

void csGLDriverDatabase::Open (csGraphics2DGLCommon* ogl2d, const char* phase)
{
  csGLDriverDatabase::ogl2d = ogl2d;
  rulePhase = phase ? phase : "";

  csRef<iConfigManager> cfgmgr = CS_QUERY_REGISTRY (ogl2d->object_reg,
    iConfigManager);

  const char* driverDB = cfgmgr->GetStr ("Video.OpenGL.DriverDB.Path",
    "/config/gldrivers.xml");
  int driverDBprio = cfgmgr->GetInt ("Video.OpenGL.DriverDB.Priority",
    iConfigManager::ConfigPriorityPlugin + 10);

  csRef<iVFS> vfs = CS_QUERY_REGISTRY (ogl2d->object_reg, iVFS);
  csRef<iFile> dbfile = vfs->Open (driverDB, VFS_FILE_READ);
  if (!dbfile)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Could not open driver database file '%s'", driverDB);
    return;
  }

  csRef<iDocumentSystem> docsys = CS_QUERY_REGISTRY (ogl2d->object_reg,
    iDocumentSystem);
  if (!docsys.IsValid())
    docsys.AttachNew (new csTinyDocumentSystem ());
  csRef<iDocument> doc (docsys->CreateDocument ());

  const char* err = doc->Parse (dbfile, true);
  if (err != 0)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Error parsing driver database: %s", err);
    return;
  }

  csRef<iDocumentNode> dbRoot (doc->GetRoot()->GetNode ("gldriverdb"));
  if (!dbRoot.IsValid())
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Driver database lacks <gldriverdb> node");
    return;
  }

  csRef<iSyntaxService> synsrv;
  CS_QUERY_REGISTRY_PLUGIN (synsrv, ogl2d->object_reg,
    "crystalspace.syntax.loader.service.text", iSyntaxService);

  csDriverDBReader reader (this, cfgmgr, synsrv, driverDBprio);

  csRef<iDocumentNodeIterator> it (dbRoot->GetNodes ());
  while (it->HasNext())
  {
    csRef<iDocumentNode> node = it->Next();
    if (node->GetType() != CS_NODE_ELEMENT) continue;
    csStringID token = tokens.Request (node->GetValue ());

    switch (token)
    {
      case XMLTOKEN_CONFIGS:
	if (!reader.ParseConfigs (node))
	  return;
	break;
      case XMLTOKEN_RULES:
	if (!reader.ParseRules (node))
	  return;
	break;
      default:
	synsrv->ReportBadToken (node);
	return;
    }
  }
}

void csGLDriverDatabase::Close ()
{
  csRef<iConfigManager> cfgmgr = CS_QUERY_REGISTRY (ogl2d->object_reg,
    iConfigManager);
  for (size_t i = 0; i < addedConfigs.Length(); i++)
  {
    cfgmgr->RemoveDomain (addedConfigs[i]);
  }
  addedConfigs.DeleteAll();
}
