/*
    Copyright (C) 2002 by Manjunath Sripadarao

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

    Description: md32spr is a Quake III md3 model to CS XML converter.
    Type md32spr for a short blurb of help. 
    It only accepts models within zip files (for now).
*/

#include "cssysdef.h"
#include <stdarg.h>
#include <ctype.h>

#include "md32spr.h"
#include "bufio.h"
#include "ivaria/reporter.h"
#include "iutil/document.h"
#include "iutil/vfs.h"
#include "iutil/databuff.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "csgeom/transfrm.h"
#include "cstool/initapp.h"
#include "csutil/xmltiny.h"
#include "csutil/csstring.h"
#include "csutil/memfile.h"
#include "csutil/archive.h"

CS_IMPLEMENT_APPLICATION

#if defined (CS_PLATFORM_DOS)
  #define IS_A_PATH_SEPARATOR(c)	\
    (((c) == CS_PATH_SEPARATOR) || ((c) == '/') || ((c) == ':'))
#else
  #define IS_A_PATH_SEPARATOR(c)	\
    (((c) == CS_PATH_SEPARATOR) || ((c) == '\\') || ((c) == '/'))
#endif

#define THRESHOLD 0.0000001

/* This is csplitpath with no modifications for our use. */
void splitpath (const char *iPathName, char *oPath, size_t iPathSize,
		char *oName, size_t iNameSize);
char *stristr(const char *String, const char *Pattern);
char *basename(const char *path, char *base);
char *filename(char *path, char *file);
char *lowercase(char *str);

void Usage() {
  csPrintf("MD3 to CS XML converter\n");
  csPrintf("by Manjunath Sripadarao\n");
  csPrintf("Usage: md32spr [options] <model-file.zip> [-o=<output-file.zip>]\n");
  csPrintf("Options:\n");
  csPrintf("-wdir=<dir>\tDirectory in which the weapon files are stored.\n");
  csPrintf("\t\texample: -wdir=railgun\n");
  csPrintf("-o=<file.zip>\tOutput zip file name. \n");
  csPrintf("-scale=<float>\tHow much the model should be scaled. Default is 1.\n");
  csPrintf("\t\texample: -scale=4096. Model will be scaled as 1/4096.\n");
}

const int MD32spr::NUM_ACTIONS = 25;
const int MD3Model::NUM_ACTIONS = 25;

csRef < iDocumentNode > MD32spr::CreateValueNode(csRef < iDocumentNode >
						 &parent, const char *name,
						 const char *value)
{
  csRef < iDocumentNode > child =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  child->SetValue(name);
  csRef < iDocumentNode > text =
    child->CreateNodeBefore(CS_NODE_TEXT, 0);
  text->SetValue(value);
  return child;
}

csRef < iDocumentNode > MD32spr::CreateValueNodeAsInt(csRef <
						      iDocumentNode >
						      &parent,
						      const char *name,
						      int value)
{
  csRef < iDocumentNode > child =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  child->SetValue(name);
  csRef < iDocumentNode > text =
    child->CreateNodeBefore(CS_NODE_TEXT, 0);
  text->SetValueAsInt(value);
  return child;
}

csRef < iDocumentNode > MD32spr::CreateValueNodeAsFloat(csRef <
							iDocumentNode >
							&parent,
							const char *name,
							float value)
{
  csRef < iDocumentNode > child =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  child->SetValue(name);
  csRef < iDocumentNode > text =
    child->CreateNodeBefore(CS_NODE_TEXT, 0);
  text->SetValueAsFloat(value);
  return child;
}

MD3Model::MD3Model(iObjectRegistry * object_reg)
{
  MD3Model::object_reg = object_reg;
  animInfo = (AnimInfo *) malloc(sizeof(AnimInfo) * (int) NUM_ELEMENTS);
  for (int i = 0; i < NUM_ACTIONS; i++) {
    animInfo[i].action = (Actions) i;
    animInfo[i].startFrame = -1;
    animInfo[i].numFrames = -1;
    animInfo[i].loopFrames = -1;
    animInfo[i].fps = -1;
  }

}

MD3Model::~MD3Model()
{
  // This line intentionally left blank.
}

void MD3Model::Init(csRef < iVFS > *vfsRef)
{
  vfs = vfsRef;
}

const char *MD3Model::GetFileName()
{
  return fileName.GetData();
}

void MD32spr::ReportError(const char *description, ...)
{
  va_list arg;
  va_start(arg, description);
  csReportV(object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.apps.md32spr", description, arg);
  va_end(arg);
}

void MD3Model::ReportError(const char *description, ...)
{
  va_list arg;
  va_start(arg, description);
  csReportV(object_reg, CS_REPORTER_SEVERITY_ERROR,
	    "crystalspace.apps.md32spr", description, arg);
  va_end(arg);
}

MD32spr::MD32spr(iObjectRegistry * object_reg)
{
  MD32spr::object_reg = object_reg;
  player = false;
  headModel = upperModel = lowerModel = 0;
  scaleFactor = 1;
}

MD32spr::~MD32spr()
{
  //MD32spr::object_reg = object_reg;
}

void MD32spr::Main()
{
  char *fileName, *dirName;
  csString mountName, weaponPath;
  size_t i = 0;
  bool head = false, torso = false, leg = false;
  //csString baseName;

  cmdline = CS_QUERY_REGISTRY(object_reg, iCommandLineParser);

  vfs = CS_QUERY_REGISTRY(object_reg, iVFS);
  out = CS_QUERY_REGISTRY(object_reg, iVFS);
  /*
     Currently we handle only .zip files. Because of the difficulty in
     accessing and reading directories consistently across platforms.
     We read in the name in the command line and do some error checking and
     doctoring for our usage. Path name and file name splitting and creation 
     of a string which will srever us as the name of our temporary vfs file
     name are some of them.
   */

  csString name(cmdline->GetName());
  if(!name) {
    Usage();
    return;
  }
  if (!stristr(name.GetData(), ".zip")) {
    ReportError
      ("Please put your model files in a zip file and give the name of the zip file.");
    ReportError
      ("Currently we handle only zip files.");
    return;
  }
  
  /*
     A command line option for weapon file. This should be given in the form of
     -weapon mp5, where mp5 is the prefix, as in mp5.md3, mp5.skin and
     texture files.
   */
  weaponDir = cmdline->GetOption("wdir");

  if (cmdline->GetOption("o")) {
    outZipName = cmdline->GetOption("o");
  } else {
    char *outName = new char[strlen(name.GetData()) + 1];
    outZipName = basename(name.GetData(), outName);
    outZipName += "_out.zip";
  }

  if (cmdline->GetOption("scale")) {
    scaleFactor = atof(cmdline->GetOption("scale"));
  }

  fileName = (char *) malloc(sizeof(char) * name.Length() + 1);
  dirName = (char *) malloc(sizeof(char) * name.Length() + 1);

  /*
     splitpath(name.GetData(), dirName, name.Length(), fileName, name.Length());
     char *dot = stristr(fileName, ".");
     char *ptr = fileName;
     while(ptr != dot)
     baseName += (char)*ptr++;
   */
  basename(name.GetData(), fileName);

  /* This doesn't seem to work if you have the name as /tmp/%s_data, but works when
     there is an ending slash, like /tmp/%s_data/. It mounts the zip file but vfs->FindFiles(char *dirname)
     doesn't work without the ending slash.
   */
  mountName.Format ("/tmp/%s_data/", fileName);
  mountPath = mountName;
  /* Now we mount the .zip file for further processing */
  vfs->Mount(mountName, name.GetData());
  fileNames = vfs->FindFiles(mountName);
  
  /* If the user didn't tell if this is a player model, then try to guess */
  if(!player) {
    md3Files = 0;

    for (i = 0; i < fileNames->Length(); i++) {
      const char *str = fileNames->Get(i);
      if(stristr(str,".md3"))
	if(stristr(str,"head"))
	  head = true;
	else if(stristr(str,"upper"))
	  torso = true;
	else if(stristr(str,"lower"))
	  leg = true;
    }
    if(head && torso && leg)
      player = true;
    else
      player = false;
  }

  for (i = 0; i < fileNames->Length(); i++) {
    const char *str = fileNames->Get(i);

    /* If the user forgot to give the -player option we just process all .md3 files as generic models.
     * We will not read the animation.cfg and .skin files, so the player model will not work unles -player
     * option is explicitly given.
     */

    if (!player && !weaponDir)
      if (stristr(str, ".md3"))
      {
	md3Model *tmpModel = new md3Model(object_reg);
	if (tmpModel)
	{
	  generic.Push(tmpModel);
	  tmpModel->Init(&vfs);
	  tmpModel->fileName = str;
	  tmpModel->LoadMD3();
	  md3Files++;
	  continue;
	}
	else
	{
	  ReportError
	    ("Fatal Error: Allocating memory for generic model.!");
	  return;
	}
      }

    /* If all 3 files are present we assume this is a player model file. */
    /* Trying to make it as bullet proof as possible. */
    if (player) {
      if (stristr(str, "upper") && stristr(str, ".md3")) {
	upperModel = new md3Model(object_reg);
	if (upperModel) {
	  upperModel->Init(&vfs);
	  upperModel->fileName = str;
	  upperModel->LoadMD3();
	  torso = true;
	  md3Files++;
	  continue;
	} else {
	  ReportError("Fatal Error: Allocating memory for upperModel!");
	  return;
	}
      } else if (stristr(str, "lower") && stristr(str, ".md3")) {
	lowerModel = new md3Model(object_reg);
	if (lowerModel) {
	  lowerModel->Init(&vfs);
	  lowerModel->fileName = str;
	  lowerModel->LoadMD3();
	  leg = true;
	  md3Files++;
	  continue;
	} else {
	  ReportError("Fatal Error: Allocating memory for lowerModel!");
	  return;
	}
      } else if (stristr(str, "head") && stristr(str, ".md3")) {
	headModel = new md3Model(object_reg);
	if (headModel) {
	  headModel->Init(&vfs);
	  headModel->fileName = str;
	  headModel->LoadMD3();
	  head = true;
	  md3Files++;
	  continue;
	} else {
	  ReportError("Fatal Error: Allocating memory for headModel!");
	  return;
	}
      }
    }

  }

  if (weaponDir.Length() > 0) 
  {
    weaponPath.Format ("%s%s/", mountName.GetData(), weaponDir.GetData());
    vfs->ChDir(weaponPath);
    weaponFiles = vfs->FindFiles(weaponPath);
    for (i = 0; i < weaponFiles->Length(); i++) {
      const char *str = weaponFiles->Get(i);
      /* If this is an md3 file and it contains the weaponName, Init() it. */
      if (stristr(str, ".md3")) {
	md3Model *tmpModel = new md3Model(object_reg);
	if (tmpModel)
	{
	  generic.Push(tmpModel);
	  tmpModel->Init(&vfs);
	  tmpModel->fileName = str;
	  tmpModel->LoadMD3();
	  md3Files++;
	}
	else
	{
	  ReportError
	    ("Fatal Error: Allocating memory for generic model.!");
	  return;
	}
      }
    }
  }

  if (weaponDir && !generic.Length())
    ReportError
      ("Fatal Error: Weapon Directory \"%s\" not found or error allocating memory",
       weaponDir.GetData());

  /* Read the model, skin and animation files. */
  ReadVfsDir();

  Write();
}

bool MD3Model::ReadHeader(char **buf)
{
  char *bufPtr = *buf;
  /* The header is 108 bytes long... */
  header = (md3Header *) malloc(sizeof(md3Header));
  memcpy(header, bufPtr, sizeof(md3Header));
  //csPrintf("%c%c%c%c\n", header->ID[0], header->ID[1], 
  // header->ID[2], header->ID[3]);

  if (header->ID[0] != 'I' || header->ID[1] != 'D'
      || header->ID[2] != 'P' || header->ID[3] != '3') {
    ReportError("File doesn't seem to be MD3. ID mismatch!");
    return false;
  }

  if (header->version != 15) {
    ReportError("File doesn't seem to be MD3. Version mismatch!");
    return false;
  }

  return true;
}

bool MD32spr::ReadVfsDir()
{
  char *str;
  size_t i = 0;

  for (i = 0; i < fileNames->Length(); i++) 
  {
    str = (char*)fileNames->Get(i);

    if (str) {
      if (player) {

	if (stristr(str, "head") && stristr(str, ".skin"))
	  if (!headModel->LoadSkin(str)) {
	    ReportError("Fatal Error: Reading skin File of head.");
	    return false;
	  }

	if (stristr(str, "upper") && stristr(str, ".skin"))
	  if (!upperModel->LoadSkin(str)) {
	    ReportError("Fatal Error: Reading skin File of Upper Body.");
	    return false;
	  }

	if (stristr(str, "lower") && stristr(str, ".skin"))
	  if (!lowerModel->LoadSkin(str)) {
	    ReportError("Fatal Error: Reading skin File of Lower Body.");
	    return false;
	  }

	if (stristr(str, ".cfg"))
	  if (!LoadAnimation(str)) {
	    ReportError("Fatal Error: Reading animation file.");
	    return false;
	  }
      }
    }
  }
  return true;
}


bool MD3Model::LoadMD3()
{
  int i = 0;
  char *bufPtr;
  csRef < iDataBuffer > buf((*vfs)->ReadFile(fileName.GetData()));
  if (!buf || !buf->GetSize())
    return false;

  bufPtr = **buf;

  if (!ReadHeader(&bufPtr)) {
    ReportError("Fatal Error: In LoadMD3 ReadHeader failed!");
    return false;
  }

  /*
   * We don't use the bones info right now. In the future if somebody 
   * plans to extend it. Then you got uncomment them here.
   *
   */
  /*
     bones = (md3Bone*)malloc(sizeof(md3Bone) * header->numFrames);
     memcpy(bones, bufPtr, sizeof(md3Bone) * header->numFrames);
   */
  tags =
    (md3Tag *) malloc(sizeof(md3Tag) * header->numFrames *
		      header->numTags);

  memcpy(tags, (bufPtr + header->tagStart),
	 sizeof(md3Tag) * header->numFrames * header->numTags);

  meshes = (md3Mesh *) malloc(sizeof(md3Mesh) * header->numMeshes);

  size_t memOffset =
    header->tagStart +
    sizeof(md3Tag) * header->numFrames * header->numTags;
  size_t dynOffset = memOffset;

  for (i = 0; i < header->numMeshes; i++) {
    meshes[i].meshHeader = (md3MeshHeader *) malloc(sizeof(md3MeshHeader));
    memcpy(meshes[i].meshHeader, (bufPtr + dynOffset),
	   sizeof(md3MeshHeader));
    dynOffset += meshes[i].meshHeader->headerSize;

    meshes[i].skins =
      (md3Skin *) malloc(sizeof(md3Skin) * meshes[i].meshHeader->numSkins);
    memcpy(meshes[i].skins, (bufPtr + dynOffset),
	   sizeof(md3Skin) * meshes[i].meshHeader->numSkins);
    dynOffset += sizeof(md3Skin) * meshes[i].meshHeader->numSkins;

    meshes[i].triangles =
      (md3Triangle *) malloc(sizeof(md3Triangle) *
			     meshes[i].meshHeader->numTriangles);
    memcpy(meshes[i].triangles,
	   (bufPtr + memOffset + meshes[i].meshHeader->triangleStart),
	   sizeof(md3Triangle) * meshes[i].meshHeader->numTriangles);

    meshes[i].texCoords =
      (md3TexCoord *) malloc(sizeof(md3TexCoord) *
			     meshes[i].meshHeader->numVertices);
    memcpy(meshes[i].texCoords,
	   (bufPtr + memOffset + meshes[i].meshHeader->texCoordStart),
	   sizeof(md3TexCoord) * meshes[i].meshHeader->numVertices);

    meshes[i].vertices =
      (md3Vertices *) malloc(sizeof(md3Vertices) *
			     meshes[i].meshHeader->numVertices *
			     meshes[i].meshHeader->numMeshFrames);
    memcpy(meshes[i].vertices,
	   (bufPtr + memOffset + meshes[i].meshHeader->vertexStart),
	   sizeof(md3Vertices) * meshes[i].meshHeader->numVertices *
	   meshes[i].meshHeader->numMeshFrames);

    memOffset += meshes[i].meshHeader->meshSize;
    dynOffset = memOffset;
  }
  return true;
}

bool MD3Model::LoadSkin(char *skinFile)
{
  char *bufPtr, fileName[100];
  csString line;
  int i = 0, j = 0;
  csRef < iDataBuffer > buf((*vfs)->ReadFile(skinFile));
  if (!buf || !buf->GetSize())
    return false;

  bufPtr = **buf;
  for (i = 0; i < (int) buf->GetSize(); i++) 
  {
    if (bufPtr[i] == '\n' || bufPtr[i] == '\r') 
    {
      for (j = 0; j < header->numMeshes; j++)
	if (stristr(line, meshes[j].meshHeader->name)) 
	{
	  size_t k = line.Length();
	  char *name = new char[k + 1];
	  strcpy(name, line.GetData());
	  if(strtok(name, ",") != 0)
	    strcpy(meshes[j].skins[0].name, strtok(0, ","));
	  k--;
	  while (line.GetAt(k) != '/' && line.GetAt(k) != '\\')
	    k--;
	  strcpy(fileName, (line.GetData() + k + 1));
	}
      line.Clear();
    } 
    else 
    {
      line.Append(bufPtr[i]);
    }
  }
  return true;
}


bool MD32spr::LoadAnimation(char *animFile)
{
  size_t lineLen = 0;
  int tmp = -1, i = 0;
  int headActions = 1, upperActions = 0, lowerActions = 0;
  char *line;
  size_t fileSz;
  vfs->GetFileSize(animFile, fileSz);
  csRef < iDataBuffer > buf(vfs->ReadFile(animFile));
  DataBuffer dataBuf(**buf, fileSz);
  lineLen = dataBuf.GetMaxLineLength();
  line = (char *) malloc(sizeof(char) * lineLen);
  char junk[4], name[15];
  while (!dataBuf.eof()) {
    dataBuf.GetLine(line);
    junk[0] = '\0';
    if (sscanf
	(line, "%d%d%d%d%s%s", &tmp, &tmp, &tmp, &tmp, junk, name) == 6) {
      if(stristr(name, "BOTH") || stristr(name, "TORSO")) {
	sscanf(line, "%d%d%d%d%s%s", &upperModel->animInfo[upperActions].startFrame,
	       &upperModel->animInfo[upperActions].numFrames, &upperModel->animInfo[upperActions].loopFrames,
	       &upperModel->animInfo[upperActions].fps, junk, upperModel->animInfo[upperActions].actionName);
	upperActions++;
      }
      if(stristr(name, "BOTH") || stristr(name, "LEGS")) {
	sscanf(line, "%d%d%d%d%s%s", &lowerModel->animInfo[lowerActions].startFrame,
	       &lowerModel->animInfo[lowerActions].numFrames, &lowerModel->animInfo[lowerActions].loopFrames,
	       &lowerModel->animInfo[lowerActions].fps, junk, lowerModel->animInfo[lowerActions].actionName);
	lowerActions++;
      }
    }
  }
  headModel->numActions = headActions;
  upperModel->numActions = upperActions;
  lowerModel->numActions = lowerActions;
  /*
   * Post Processing:
   * In some of the animation config files, the numbering leg animations of start frames continue 
   * after torso animations. To correct this we sum the number of torso animations and negate them
   * from leg animations thus resetting the start frames of leg animations.
   */
  int upperActionStart = 0, lowerActionStart = 0;
  for(i = 0; i < NUM_ACTIONS; i++)
    if(strcmp(upperModel->animInfo[i].actionName, "TORSO_GESTURE") == 0)
      break;
  upperActionStart = i;

  for(i = 0; i < NUM_ACTIONS; i++)
    if(strcmp(lowerModel->animInfo[i].actionName, "LEGS_WALKCR") == 0)
      break;
  lowerActionStart = i;

  if (upperModel->animInfo[upperActionStart].startFrame !=
      lowerModel->animInfo[lowerActionStart].startFrame) {
    int numTorsoFrames = 0;
    for (i = 0; i < upperActions; i++)
      if (stristr(upperModel->animInfo[i].actionName, "TORSO"))
	numTorsoFrames += upperModel->animInfo[i].numFrames;

    for (i = 0; i < lowerActions; i++)
      if (stristr(lowerModel->animInfo[i].actionName, "LEGS")) {
	lowerModel->animInfo[i].startFrame -= numTorsoFrames;
      }
  }
  return true;
}

void MD32spr::Write()
{
  size_t i = 0;
  csString vfspath;
  char* fileName = 0;
  char* mdlName = 0;

  if (outZipName)
  {
    if (outZipName.Length() > 0)
    {
      csArchive* zipFile = new csArchive (outZipName.GetData());

      if(!zipFile)
      {
        ReportError("Error creating output zip file.\n");
        return;
      }

      delete zipFile;
      fileName = new char[outZipName.Length() + 1];
      basename(outZipName.GetData(), fileName);
      vfspath.Format ("/tmp/%s_out/", fileName);

      if (!out->Mount(vfspath, outZipName.GetData()))
      {
        ReportError("Error mounting output zip file.\n");
        return;
      }
    }
  }

  if (!player && !weaponDir)
  {
    if (generic.Length())
    {
      for (i = 0; i < generic.Length(); i++)
      {
        md3Model *mdl = generic.Get(i);
        mdlName = new char[strlen(mdl->GetFileName()) + 1];
        basename(mdl->GetFileName(), mdlName);
        csRef<iDocumentSystem> xml(csPtr <iDocumentSystem>
          (new csTinyDocumentSystem()));
        csRef <iDocument> doc = xml->CreateDocument();
        csRef <iDocumentNode> root = doc->CreateRoot();
        csRef <iDocumentNode> parent =
          root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        parent->SetValue("library");
	
        WriteGeneric(mdl, parent);
        csString outFile(vfspath);
        outFile += mdlName;
        doc->Write(out, outFile.GetData());
      }
    }
  }

  if (player) 
  {
    if (headModel) 
    {
      char *headName = new char[strlen(headModel->GetFileName()) + 1];
      csString tagFileName;
      basename(headModel->GetFileName(), headName);
      tagFileName.Format ("%s%s.tag", vfspath.GetData(), headName);
      WriteXMLTags(headModel, tagFileName);
      csRef < iDocumentSystem > xml(csPtr < iDocumentSystem >
				    (new csTinyDocumentSystem()));
      csRef < iDocument > doc = xml->CreateDocument();
      csRef < iDocumentNode > root = doc->CreateRoot();
      csRef < iDocumentNode > parent =
	root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      parent->SetValue("library");
      WriteGeneric(headModel, parent);
      csString outFile(vfspath);
      outFile += headName;
      doc->Write(out, outFile.GetData());
    }
    if (upperModel) {
      char *upperName = new char[strlen(headModel->GetFileName()) + 1];
      csString tagFileName;
      basename(upperModel->GetFileName(), upperName);
      tagFileName.Format ("%s%s.tag", vfspath.GetData(), upperName);
      WriteXMLTags(upperModel, tagFileName);
      csRef < iDocumentSystem > xml(csPtr < iDocumentSystem >
				    (new csTinyDocumentSystem()));
      csRef < iDocument > doc = xml->CreateDocument();
      csRef < iDocumentNode > root = doc->CreateRoot();
      csRef < iDocumentNode > parent =
	root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      parent->SetValue("library");

      WriteGeneric(upperModel, parent);
      csString outFile(vfspath);
      outFile += upperName;
      doc->Write(out, outFile.GetData());
    }
    if (lowerModel) {
      char *lowerName = new char[strlen(headModel->GetFileName()) + 1];
      csString tagFileName;
      basename(lowerModel->GetFileName(), lowerName);
      tagFileName.Format ("%s%s.tag", vfspath.GetData(), lowerName);
      WriteXMLTags(lowerModel, tagFileName);
      csRef < iDocumentSystem > xml(csPtr < iDocumentSystem >
				    (new csTinyDocumentSystem()));
      csRef < iDocument > doc = xml->CreateDocument();
      csRef < iDocumentNode > root = doc->CreateRoot();
      csRef < iDocumentNode > parent =
	root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      parent->SetValue("library");

      WriteGeneric(lowerModel, parent);
      csString outFile(vfspath);
      outFile += lowerName;
      doc->Write(out, outFile.GetData());
    }
    if (weaponDir)
    {
      for (i = 0; i < generic.Length(); i++)
      {
	md3Model *mdl = generic.Get(i);
	csRef < iDocumentSystem > xml(csPtr < iDocumentSystem >
				      (new csTinyDocumentSystem()));
	csRef < iDocument > doc = xml->CreateDocument();
	csRef < iDocumentNode > root = doc->CreateRoot();
	csRef < iDocumentNode > parent =
	  root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	parent->SetValue("library");

	WriteGeneric(mdl, parent);
	char *fileName = new char[mdl->fileName.Length() + 1];
	csString tagFileName;
	basename(mdl->fileName.GetData(), fileName);


	csString outFile(vfspath);
	outFile += weaponDir;
	outFile += "/";
	outFile += fileName;
	doc->Write(out, outFile.GetData());
	tagFileName.Format ("%s%s/%s.tag", vfspath.GetData(), weaponDir.GetData(), fileName);
	WriteXMLTags(mdl, tagFileName);
      }
    }
  }
  WriteTextures(mountPath.GetData(),vfspath);
  if(weaponDir) {
    csString weaponInPath(mountPath.GetData());
    csString weaponOutPath(vfspath);
    weaponInPath += weaponDir;
    weaponInPath += "/";
    weaponOutPath += weaponDir;
    weaponOutPath += "/";
    WriteTextures(weaponInPath.GetData(), weaponOutPath.GetData());
  }
}

void MD32spr::WriteXMLTags(md3Model * model,
			const char *tagFileName) 
{
  int i = 0, j = 0, k = 0;
  float *rotationMatrix, *translationVector;
  csRef < iDocumentSystem > xml(csPtr < iDocumentSystem >
				(new csTinyDocumentSystem()));
  csRef < iDocument > doc = xml->CreateDocument();
  csRef < iDocumentNode > root = doc->CreateRoot();
  csRef < iDocumentNode > child;

  for (i = 0; i < model->header->numFrames; i++)
    for(j = 0; j < model->header->numTags; j++) {
      csString rotation;
      csString position;
      csString taginfo;
      child = 
	root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      child->SetValue("key");
      child->SetAttribute("name", "md3tag");
      rotationMatrix = &model->tags[i * model->header->numTags + j].rotation[0][0];
      translationVector = &model->tags[i * model->header->numTags + j].position[0];
      // Convert rotation matrix to string.
      k = 0;
      while (k < 9) {
	rotation += ' ';
	rotation += (fabs(rotationMatrix[k]) < THRESHOLD) ? 0 : rotationMatrix[k];
	k++;
      }
      // Convert position vector to string.
      k = 0;
      while (k < 3) {
	position += ' ';
	position += (fabs(translationVector[k]) < THRESHOLD) ? 0 : translationVector[k];
	k++;
      }
      // Merge tag name, frame num, tag num, rotation matrix and position vector into one string.
      taginfo += model->tags[i * model->header->numTags + j].name;
      taginfo += ' ';
      taginfo += i; 
      taginfo += ' ';
      taginfo += j;
      taginfo += ' ';
      taginfo += rotation;
      taginfo += ' ';
      taginfo += position;
      // Write out tag info.
      child->SetAttribute("tag", taginfo);
    }
  doc->Write(out, tagFileName);
}

void MD32spr::WriteXMLTextures(md3Model * model,
			csRef < iDocumentNode > &parent) 
{

  int i = 0, j = 0;
  for (i = 0; i < model->header->numMeshes; i++) {
    for (j = 0; j < model->meshes[i].meshHeader->numSkins; j++) {
      csRef < iDocumentNode > child =
	parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      child->SetValue("texture");
      char *name = new char[strlen(model->meshes[i].skins[j].name) + 1];
      if(basename(model->meshes[i].skins[j].name, name) == 0)
	continue;
      child->SetAttribute("name", lowercase(name));
      char *fname = new char[strlen(model->meshes[i].skins[j].name) + 1];
      if(filename(model->meshes[i].skins[j].name, fname) == 0)
	continue;
      CreateValueNode(child, "file", lowercase(fname));
    }
  }
}

void MD32spr::WriteXMLMaterials(md3Model * model,
			csRef < iDocumentNode > &parent) 
{
  int i = 0, j = 0;
  for (i = 0; i < model->header->numMeshes; i++)
    for (j = 0; j < model->meshes[i].meshHeader->numSkins; j++) {
      csRef < iDocumentNode > child =
	parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      child->SetValue("material");
      char *name = new char[strlen(model->meshes[i].skins[j].name) + 1];
      if(basename(model->meshes[i].skins[j].name, name) == 0)
	continue;
      child->SetAttribute("name", strcat(lowercase(name), ".mat"));
      char *tname = new char[strlen(model->meshes[i].skins[j].name) + 1];
      if(basename(model->meshes[i].skins[j].name, tname) == 0)
	continue;
      CreateValueNode(child, "texture", lowercase(tname));
    }
}

void MD32spr::WriteGeneric(md3Model * model,
			   csRef < iDocumentNode > &parent)
{
  int i = 0, j = 0, k = 0;
  csRef < iDocumentNode > localParent;

  localParent =
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  localParent->SetValue("textures");

  if (model->header->numMeshes > 0) {
    WriteXMLTextures(model, localParent);
      
    localParent = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    localParent->SetValue("materials");
    WriteXMLMaterials(model, localParent);


    for (i = 0; i < model->header->numMeshes; i++) {
      localParent = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      localParent->SetValue("meshfact");
      localParent->SetAttribute("name", model->meshes[i].meshHeader->name);
      CreateValueNode(localParent, "plugin",
		      "crystalspace.mesh.loader.factory.sprite.3d");
      csRef < iDocumentNode > child =
	localParent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      child->SetValue("params");

      for (j = 0; j < model->meshes[i].meshHeader->numSkins; j++) {
	char *matName = new char[strlen(model->meshes[i].skins[j].name) + 1];
	if(basename(model->meshes[i].skins[j].name, matName) == 0)
	  continue;
	CreateValueNode(child, "material",
			strcat(matName, ".mat"));
      }

      csRef < iDocumentNode > frame;
      for (j = 0; j < model->meshes[i].meshHeader->numMeshFrames; j++) {
	csString fNum;
	int numVertices = model->meshes[i].meshHeader->numVertices;

	frame = child->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	frame->SetValue("frame");
	fNum.Format ("f%d", j);
	frame->SetAttribute("name", fNum);

	for (k = 0; k < model->meshes[i].meshHeader->numVertices; k++) {
	  float u, v, x, y, z;
	  csRef < iDocumentNode > texel =
	    frame->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  texel->SetValue("v");
	  x = model->meshes[i].vertices[j * numVertices + k].vec[0];
	  y = model->meshes[i].vertices[j * numVertices + k].vec[1];
	  z = model->meshes[i].vertices[j * numVertices + k].vec[2];
	  u = model->meshes[i].texCoords[k].texCoord[0];
	  v = model->meshes[i].texCoords[k].texCoord[1];
	  texel->SetAttributeAsFloat("x", x / scaleFactor);
	  texel->SetAttributeAsFloat("y", y / scaleFactor);
	  texel->SetAttributeAsFloat("z", z / scaleFactor);
	  texel->SetAttributeAsFloat("u", u);
	  texel->SetAttributeAsFloat("v", v);
	}
      }
      if((stristr(model->fileName.GetData(), "head") || 
	 stristr(model->fileName.GetData(), "upper") || 
	 stristr(model->fileName.GetData(), "lower")) && player) {
	// Write all the actions.
	for(j = 0; j < model->numActions; j++) {
	  if(model->numActions == 1) {
	    csRef < iDocumentNode > action =
	      child->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	    action->SetValue("action");
	    action->SetAttribute("name", "default");

	    for (k = 0; k < model->meshes[i].meshHeader->numMeshFrames; k++) 
	    {
	      csString fNum;
	      csRef < iDocumentNode > f =
		action->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	      fNum.Format ("f%d", k);
	      f->SetValue("f");
	      f->SetAttribute("name", fNum);
	      f->SetAttribute("delay", "1000");	/// 1000 is the default delay.
	    }
	  } else {
	    csRef < iDocumentNode > action =
	      child->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	    action->SetValue("action");
	    action->SetAttribute("name", model->animInfo[j].actionName);
	  
	    for(k = model->animInfo[j].startFrame; 
		k < (model->animInfo[j].startFrame + model->animInfo[j].numFrames); 
		k++) 
	    {
	      csString fNum;
	      csRef < iDocumentNode > f =
		action->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	      fNum.Format ("f%d", k);
	      f->SetValue("f");
	      f->SetAttribute("name", fNum);
	      float delay = (1.0f/(float)model->animInfo[j].fps) * 1000;
	      f->SetAttributeAsFloat("delay", delay);  
	    }
	  }
	}
      } else {
	csRef < iDocumentNode > action =
	  child->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	action->SetValue("action");
	action->SetAttribute("name", "default");

	for (j = 0; j < model->meshes[i].meshHeader->numMeshFrames; j++) 
	{
	  csString fNum;
	  csRef < iDocumentNode > f =
	    action->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	  fNum.Format ("f%d", j);
	  f->SetValue("f");
	  f->SetAttribute("name", fNum);
	  f->SetAttribute("delay", "1000");	/// 1000 is the default delay.
	}
      }
      for (j = 0; j < model->meshes[i].meshHeader->numTriangles; j++) {
	csRef < iDocumentNode > triangles =
	  child->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	triangles->SetValue("t");
	triangles->SetAttributeAsInt("v1",
				     model->meshes[i].triangles[j].
				     triangle[2]);
	triangles->SetAttributeAsInt("v2",
				     model->meshes[i].triangles[j].
				     triangle[1]);
	triangles->SetAttributeAsInt("v3",
				     model->meshes[i].triangles[j].
				     triangle[0]);
      }
    }
    for (i = 0; i < model->header->numMeshes; i++) {
      localParent = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      localParent->SetValue("meshobj");
      char *name =
	new char[strlen(model->fileName.GetData()) +
		 strlen(model->meshes[i].meshHeader->name) + 1];
      basename(model->fileName.GetData(), name);
      localParent->SetAttribute("name",
				strcat(name,
				       model->meshes[i].meshHeader->name));
      CreateValueNode(localParent, "plugin",
		      "crystalspace.mesh.loader.sprite.3d");
      csRef < iDocumentNode > params =
	localParent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      params->SetValue("params");
      CreateValueNode(params, "factory",
		      model->meshes[i].meshHeader->name);
      CreateValueNode(params, "action", "default");
    }
  }
}

void MD32spr::WriteTextures(const char *inPath, const char *outPath)
{
  size_t i = 0;
  size_t fileSize;
  csRef <iStringArray> files = vfs->FindFiles(inPath);
  char fileName[100];
  for(i = 0; i < files->Length(); i++) 
  {
    char *str = (char*)files->Get(i);

    if(stristr(str,".jpg") || stristr(str, ".png") || stristr(str, ".bmp") || stristr(str, ".tga")) 
    {
      filename(str, fileName);
      lowercase(fileName);
      csString csStr(outPath);
      csStr += fileName;
      csRef < iDataBuffer > buf(vfs->ReadFile(str));
      vfs->GetFileSize(str, fileSize);
      if(out->Exists(csStr.GetData()))
	continue;
      out->WriteFile(csStr.GetData(), **buf, fileSize);
    }
  }
}
      

int main(int argc, char *argv[])
{
  // Initialize the random number generator
  srand(time(0));

  iObjectRegistry *object_reg =
    csInitializer::CreateEnvironment(argc, argv);
  if (!object_reg)
    return -1;

  if (!csInitializer::RequestPlugins(object_reg,
				     CS_REQUEST_VFS, CS_REQUEST_END))
    return -1;

  MD32spr *lt = new MD32spr(object_reg);
  lt->Main();
  delete lt;

  csInitializer::DestroyApplication(object_reg);

  return 0;
}

/*
 * I found this code on the internet, this code is in the public domain.
 * Here is the info found in the header. This is part of snippets.

 * +++Date last modified: 24-Nov-1996
 *
 ** Designation:  StriStr
 **
 ** Call syntax:  char *stristr(char *String, char *Pattern)
 **
 ** Description:  This function is an ANSI version of strstr() with
 **               case insensitivity.
 **
 ** Return item:  char *pointer if Pattern is found in String, else
 **               pointer to 0
 **
 ** Rev History:  07/04/95  Bob Stout  ANSI-fy
 **               02/03/94  Fred Cole  Original
 **
 ** Hereby donated to public domain.
 */

char *stristr(const char *String, const char *Pattern)
{
  char *pptr, *sptr, *start;
  size_t slen, plen;

  for (start = (char *) String,
       pptr = (char *) Pattern,
       slen = strlen(String), plen = strlen(Pattern);
       /* while string length not shorter than pattern length */
       slen >= plen; start++, slen--) {
    /* find start of pattern in string */
    while (toupper(*start) != toupper(*Pattern)) {
      start++;
      slen--;

      /* if pattern longer than string */

      if (slen < plen)
	return (0);
    }

    sptr = start;
    pptr = (char *) Pattern;

    while (toupper(*sptr) == toupper(*pptr)) {
      sptr++;
      pptr++;

      /* if end of pattern then pattern was found */

      if ('\0' == *pptr)
	return (start);
    }
  }
  return (0);
}

/*----------------------------------------------------------------------------*/

char *basename(const char *path, char *base)
{
  char *dir, *file;
  char *ptr, *point;
  size_t sz = 0;
  if (!path || !base)
    return 0;
  if(!strlen(path))
    return 0;
  dir = new char[strlen(path)];
  file = new char[strlen(path)];
  splitpath(path, dir, strlen(path), file, strlen(path));
  point = stristr(file, ".");
  ptr = file;
  if(point != 0) 
  {
    while (ptr++ != point)
      sz++;
  } 
  else 
  {
    sz = strlen(file);
  }
  strncpy(base, file, sz);
  base[sz] = '\0';
  return base;
}

char *filename(char *path, char *file)
{
  char *dir, *fname;
  size_t sz = 0;
  if (!path || !file)
    return 0;
  if(!strlen(path))
    return 0;
  dir = new char[strlen(path)];
  fname = new char[strlen(path)];
  splitpath(path, dir, strlen(path), fname, strlen(path));
  sz = strlen(fname);
  strncpy(file, fname, sz);
  file[sz] = '\0';
  return file;
}

char *lowercase(char *str)
{
  int i = 0;
  while(str[i] != '\0') {
   str[i] =  tolower(str[i]);
    i++;
  }
  return str;
}

void splitpath (const char *iPathName, char *oPath, size_t iPathSize,
  char *oName, size_t iNameSize)
{
  size_t sl = strlen (iPathName);
  size_t maxl = sl;
  while (sl && (!IS_A_PATH_SEPARATOR (iPathName [sl - 1])))
    sl--;

  if (iPathSize)
    if (sl >= iPathSize)
    {
      memcpy (oPath, iPathName, iPathSize - 1);
      oPath [iPathSize - 1] = 0;
    }
    else
    {
      memcpy (oPath, iPathName, sl);
      oPath [sl] = 0;
    }

  if (iNameSize)
    if (maxl - sl >= iNameSize)
    {
      memcpy (oName, &iPathName [sl], iNameSize - 1);
      oName [iNameSize - 1] = 0;
    }
    else
      memcpy (oName, &iPathName [sl], maxl - sl + 1);
}
