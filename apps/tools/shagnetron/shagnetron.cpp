/*
    Copyright (C) 2008 by Frank Richter

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

#include "shagnetron.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

Shagnetron::Shagnetron (iObjectRegistry* object_reg) : object_reg (object_reg),
  doVerbose (false)
{
}

bool Shagnetron::Initialize ()
{
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (object_reg);
  
  csRef<iVerbosityManager> verbose = csQueryRegistry<iVerbosityManager> (object_reg);
  doVerbose = verbose->Enabled ("shagnetron");
  // Implicitly enable verboseness for shaders.
  verbose->Parse ("+renderer.shader.precache");
  
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_PLUGIN("crystalspace.graphics3d.shadermanager",
	  iShaderManager),
	CS_REQUEST_PLUGIN("crystalspace.documentsystem.multiplexer",
	  iDocumentSystem),
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.shagnetron",
	"Can't initialize plugins!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    PrintHelp ();
    return false;
  }

  return true;
}

bool Shagnetron::FileBlacklisted (const char* file)
{
  size_t fileLen = strlen (file);
  if ((fileLen >= 5) && (strcmp (file+fileLen - 5, ".svn/") == 0))
    return true;
  if (file[fileLen-1] == '~') return true;
  return false;
}

bool Shagnetron::PrecacheShaderFile (const char* file, bool doQuick)
{
  if (doVerbose)
    csPrintf ("%s ... ", file);

  csRef<iFile> fileObj = vfs->Open (file, VFS_FILE_READ);
  if (!fileObj.IsValid())
  {
    if (doVerbose) csPrintf ("can't open\n");
    return false;
  }

  csRef<iDocument> doc (docsys->CreateDocument());
  const char* err = doc->Parse (fileObj);
  if (err != 0)
  {
    if (doVerbose) csPrintf ("parse error: %s\n", err);
    return false;
  }
  
  csRef<iDocumentNode> shaderNode = doc->GetRoot()->GetNode ("shader");
  if (!shaderNode.IsValid())
  {
    if (doVerbose) csPrintf ("not a shader\n");
    return false;
  }
  
  if (!doVerbose)
    csPrintf ("%s ... ", file);
  fflush (stdout);
    
  const char* type = shaderNode->GetAttributeValue ("compiler");
  if (type == 0)
    type = shaderNode->GetAttributeValue ("type");
  if (type == 0)
  {
    csPrintf ("%s attribute is missing!\n",
	      CS::Quote::Single ("compiler"));
    return false;
  }
  csRef<iShaderCompiler> shcom = shaderMgr->GetCompiler (type);
  if (!shcom.IsValid()) 
  {
    csPrintf ("Could not get shader compiler %s", CS::Quote::Single (type));
    return false;
  }

  bool result;
  {
    csVfsDirectoryChanger dirChanger (vfs);
    dirChanger.ChangeTo (file);
    
    result = shcom->PrecacheShader (shaderNode,
      shaderMgr->GetShaderCache(), doQuick);
  }

  csPrintf ("%s\n", result ? "ok" : "failed");

  return result;
}

bool Shagnetron::Run ()
{
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (object_reg);
  
  csFIFO<csString> toScan;
  for (size_t i = 0; ; i++)
  {
    csString dir (cmdline->GetName (i));
    if (dir.IsEmpty()) break;
    
    toScan.Push (dir);
  }
  if (toScan.GetSize() == 0)
  {
    PrintHelp ();
    return false;
  }
  
  docsys = csQueryRegistry<iDocumentSystem> (object_reg);
  vfs = csQueryRegistry<iVFS> (object_reg);
  shaderMgr = csQueryRegistry<iShaderManager> (object_reg);
  iHierarchicalCache* shaderCache = shaderMgr->GetShaderCache();
  if (!shaderCache)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.shagnetron",
	"Shader cache is disabled. Please enable to use this tool");
    // @@@ It might be sensible to forcibly enable shader caching
    return false;
  }
  
  if (cmdline->GetBoolOption ("nodefaultcaches", false))
    shaderMgr->RemoveAllSubShaderCaches();
  for (size_t i = 0; ; i++)
  {
    const char* opt = cmdline->GetOption ("cachedir", i);
    if (!opt || !*opt) break;
    
    shaderMgr->AddSubCacheDirectory (opt,
      iShaderManager::cachePriorityHighest);
  }
  if (cmdline->GetBoolOption ("cacheclear", false))
  {
    csPrintf ("Clearing cache ..."); fflush (stdout);
    shaderCache->ClearCache ("/");
    csPrintf (" ok\n");
  }

  bool doQuick = cmdline->GetBoolOption ("quick", false);
  
  csSet<csString> seenDirectories;
  while (toScan.GetSize() > 0)
  {
    csString obj (toScan.PopTop());
    
    if (obj[obj.Length()-1] == '/')
    {
      if (!seenDirectories.Contains (obj))
      {
        csRef<iStringArray> vfsFiles (vfs->FindFiles (obj));
        for (size_t i = 0; i < vfsFiles->GetSize(); i++)
        {
	  const char* file = vfsFiles->Get (i);
  	
	  if (FileBlacklisted (file)) continue;
  	
	  toScan.Push (file);
        }
        seenDirectories.AddNoTest (obj);
      }
    }
    else
      PrecacheShaderFile (obj, doQuick);
  }

  return true;
}

void Shagnetron::PrintHelp ()
{
  const char* appname = "shagnetron";

  csPrintf ("Usage: %s [options] <VFS directory | shader file name> <...>\n", appname);
  csPrintf ("\n");
  csPrintf ("Pre-warms a shader cache with all the shaders from the given directories.\n");
  csPrintf ("\n");
  csPrintf ("The name is a portmanteau of %s and %s.\n",
	    CS::Quote::Single ("shader"), CS::Quote::Single ("magnetron"));
  csPrintf ("\n");
  csPrintf ("Available options:\n");
  csPrintf (" -cachedir=<dir>   Specify additional cache VFS directories.\n");
  csPrintf ("                   The later a directory is specified the lower its priority.\n");
  csPrintf ("                   The first directory is written to.\n");
  csPrintf (" -nodefaultcaches  Do not use default caches set in config\n");
  csPrintf (" -cacheclear       Clear used caches\n");
  csPrintf (" -quick            Do a %s precache.\n",
	    CS::Quote::Single ("quick"));
  csPrintf ("                   Precaching will take less time, but some shader processing\n");
  csPrintf ("                   will still happen at run time.\n");
  csPrintf ("\n");
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;

  int res = 1;
  {
    CS::Utility::ScopedDelete<Shagnetron> app (new Shagnetron (object_reg));

    if (app->Initialize ())
      res = app->Run () ? 0 : 1;
  }

  csInitializer::DestroyApplication (object_reg);
  return res;
}
