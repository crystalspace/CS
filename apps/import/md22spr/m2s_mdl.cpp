/*
  Crystal Space Quake MDL/MD2 convertor
  Copyright (C) 1998 by Nathaniel Saint Martin <noote@bigfoot.com>
  Significant overhaul by Eric Sunshine <sunshine@sunshineco.com> in Feb 2000

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "m2s_mdl.h"
#include "m2s_pal.h"
#include "m2s_img.h"
#include "csutil/csendian.h"
#include "csutil/sysfunc.h"
#include "iutil/databuff.h"
#include "igraphic/imageio.h"

extern csRef<iImageIO> mdl2spr_imageio;

bool Mdl::IsFileMDLModel(const char* mdlfile)
{
  return superclass::CheckMagic(mdlfile, "IDPO");
}

Mdl::Mdl() : QModel()
{
  Clear();
}

Mdl::Mdl(const char* mdlfile)
{
  Clear();
  ReadMDLFile(mdlfile);
}

Mdl::~Mdl()
{
  // @@@ FIXME: Unimplemented :-)
}

void Mdl::Clear()
{
  nbskins = 0;
  skinheight = skinwidth = 0;
  skins = 0;
  nbframesets = 0;
  framesets = 0;
  nbtriangles = 0;
  triangles = 0;
  nbvertices = 0;
  vertices = 0;
  originX = originY = originZ = 0.0;
  scaleX = scaleY = scaleZ = 1.0;
  skinheight = skinwidth = 0;
}

bool Mdl::ReadMDLFile(const char* mdlfile)
{
  FILE *f;
  int i, j, ii;

  clearError();

  if (mdlfile == 0 || strlen(mdlfile) == 0)
    return setError("MDL filename is 0");

  if ((f = fopen(mdlfile, "rb")) == 0)
    return setError("Cannot find MDL file");

  // read mdl magic
  magic_t magic;
  if (fread(&magic, sizeof(magic_t), 1, f) != 1)
    return setError("Cannot read magic header", f);

  // check if is a correct magic
  if (strncmp(magic.id, "IDPO", 4) != 0)
    return setError("Invalid mdl magic", f);

  // check if is a correct version
  magic.version = csConvertEndian (magic.version);
  if (magic.version != 6)
    return setError("Invalid mdl version", f);

  // read mdl header
  mdl_t header;
  if (fread(&header, sizeof(mdl_t), 1, f) != 1)
    return setError("Cannot read mdl header", f);

  header.scale.x = csConvertEndian (header.scale.x);
  header.scale.y = csConvertEndian (header.scale.y);
  header.scale.z = csConvertEndian (header.scale.z);
  header.origin.x = csConvertEndian (header.origin.x);
  header.origin.y = csConvertEndian (header.origin.y);
  header.origin.z = csConvertEndian (header.origin.z);
  header.radius = csConvertEndian (header.radius);
  header.offsets.x = csConvertEndian (header.offsets.x);
  header.offsets.y = csConvertEndian (header.offsets.y);
  header.offsets.z = csConvertEndian (header.offsets.z);
  header.numskins = csConvertEndian (header.numskins);
  header.skinwidth = csConvertEndian (header.skinwidth);
  header.skinheight = csConvertEndian (header.skinheight);
  header.numverts = csConvertEndian (header.numverts);
  header.numtris = csConvertEndian (header.numtris);
  header.numframes = csConvertEndian (header.numframes);
  header.synctype = csConvertEndian (header.synctype);
  header.flags = csConvertEndian (header.flags);
  header.size = csConvertEndian (header.size);

  // sprite ops
  radiusbound = header.radius;
  originX = header.origin.x;
  originY = header.origin.y;
  originZ = header.origin.z;
  scaleX = header.scale.x;
  scaleY = header.scale.y;
  scaleZ = header.scale.z;

  // skins ops
  skinheight = header.skinheight;
  skinwidth = header.skinwidth;
  nbskins = header.numskins;
  skins = new skin_t[nbskins];
  for (i = 0; i < nbskins; i++)
  {
    int32 group = 0;
    if (fread(&group, sizeof(group), 1, f) != 1)
      return setError("Error reading mdl file", f);
    group = csConvertEndian (group);

    if (group != 1 && group != 0)
      return setError("Incoherence in skin model properties", f);

    if (group == 0) // one text skin
    {
      skins[i].group = false;
      skins[i].nbtexs = 1;
      skins[i].texs = new unsigned char*[1];
      skins[i].texs[0] = new unsigned char [skinheight * skinwidth];
      if (fread(skins[i].texs[0], skinheight * skinwidth, 1, f) != 1)
        return setError("Error reading tex skin", f);
    }
    else // multi-tex skin
    {
      skins[i].group = true;
      if (fread(&skins[i].nbtexs, sizeof(skins[i].nbtexs), 1, f) != 1)
        return setError("Error reading mdl file", f);
      skins[i].nbtexs = csConvertEndian (skins[i].nbtexs);

      // read time between frame
      skins[i].timebtwskin = new float [skins[i].nbtexs];
      if (fread(skins[i].timebtwskin, sizeof(float)*skins[i].nbtexs, 1, f)!=1)
        return setError("Error reading multi-tex skin", f);
      for (ii = 0 ; ii < skins[i].nbtexs ; ii++)
        skins[i].timebtwskin[ii] = csConvertEndian (skins[i].timebtwskin[ii]);

      // read all texture of group
      skins[i].texs = new unsigned char *[skins[i].nbtexs];
      for (j = 0; j < skins[i].nbtexs; j++)
      {
        skins[i].texs[j] = new unsigned char [skinheight * skinwidth];
        if (fread(skins[i].texs[j], skinheight * skinwidth, 1, f) != 1)
          return setError("Error reading multi-tex skin", f);
      }
    }
  }

  // vertices ops
  nbvertices = header.numverts;
  vertices = new vertice_t[nbvertices];
  // read all vertices
  for (i = 0; i < nbvertices; i++)
  {
    if (fread(&vertices[i], sizeof(vertice_t), 1, f) != 1)
      return setError("Error reading mdl file", f);
    vertices[i].onseam = csConvertEndian (vertices[i].onseam);
    vertices[i].s = csConvertEndian (vertices[i].s);
    vertices[i].t = csConvertEndian (vertices[i].t);
  }

  // triangles ops
  nbtriangles = header.numtris;
  triangles = new triangle_t[nbtriangles];
  // read all triangles
  for (i = 0; i < nbtriangles; i++)
  {
    if (fread(&triangles[i], sizeof(triangle_t), 1, f) != 1)
      return setError("Error reading mdl file", f);
    triangles[i].facefront = csConvertEndian (triangles[i].facefront);
    triangles[i].vertice[0] = csConvertEndian (triangles[i].vertice[0]);
    triangles[i].vertice[1] = csConvertEndian (triangles[i].vertice[1]);
    triangles[i].vertice[2] = csConvertEndian (triangles[i].vertice[2]);
  }

  // frames ops
  nbframesets = header.numframes;
  framesets = new frameset_t [nbframesets];
  // check all framessets
  for (i = 0; i < nbframesets; i++)
  {
    int32 typeframe = 0;
    if (fread(&typeframe, sizeof(typeframe), 1, f) != 1)
      return setError("Error reading mdl file", f);
    typeframe = csConvertEndian (typeframe);

    if (typeframe == 0) // one animation frame
    {
      framesets[i].group = false;
      framesets[i].nbframes = 1;
      framesets[i].delay = new float[1];
      framesets[i].delay[0] = 0.0;
      framesets[i].frames = new frame_t[1];
      framesets[i].frames[0].trivert = new trivertx_t[nbvertices];

      // read min bound
      if (fread(&framesets[i].frames[0].min, sizeof(trivertx_t), 1, f) != 1)
        return setError("Error reading mdl file", f);
      memcpy(&framesets[i].min,&framesets[i].frames[0].min,sizeof(trivertx_t));

      // read max bound
      if (fread(&framesets[i].frames[0].max, sizeof(trivertx_t), 1, f) != 1)
        return setError("Error reading mdl file", f);
      memcpy(&framesets[i].max,&framesets[i].frames[0].max,sizeof(trivertx_t));

      // name of frame
      memset(framesets[i].frames[0].name, 0, MDL_FRAME_NAME_MAX + 1);
      if (fread(framesets[i].frames[0].name, MDL_FRAME_NAME_MAX, 1, f) != 1)
        return setError("Error reading mdl file", f);

      // vertices
      if (fread(framesets[i].frames[0].trivert,
        sizeof(trivertx_t) * nbvertices, 1, f) != 1)
        return setError("Error reading mdl file", f);
    }
    else  // multi-frame animation
    {
      framesets[i].group = true;

      if (fread(&framesets[i].nbframes, sizeof(framesets[i].nbframes),
                1, f) != 1)
        return setError("Error reading mdl file", f);
      framesets[i].nbframes = csConvertEndian (framesets[i].nbframes);

      framesets[i].delay = new float[framesets[i].nbframes];
      framesets[i].frames = new frame_t[framesets[i].nbframes];

      // read general min bound
      if (fread(&framesets[i].min, sizeof(trivertx_t), 1, f) != 1)
        return setError("Error reading mdl file", f);

      // read general max bound
      if (fread(&framesets[i].max, sizeof(trivertx_t), 1, f) != 1)
        return setError("Error reading mdl file", f);

      // read time between frame
      if (fread(framesets[i].delay,sizeof(float)*framesets[i].nbframes,1,f)!=1)
        return setError("Error reading mdl file", f);
      for (ii = 0 ; ii < framesets[i].nbframes; ii++)
        framesets[i].delay[ii] = csConvertEndian (framesets[i].delay[ii]);

      // read all frames in frameset
      for (j = 0; j < framesets[i].nbframes; j++)
      {
        // read min bound
        if (fread(&framesets[i].frames[j].min, sizeof(trivertx_t), 1, f) != 1)
          return setError("Error reading mdl file", f);

        // read max bound
        if (fread(&framesets[i].frames[j].max, sizeof(trivertx_t), 1, f) != 1)
          return setError("Error reading mdl file", f);

        // frame name
        memset(framesets[i].frames[j].name, 0, MDL_FRAME_NAME_MAX + 1);
        if (fread(framesets[i].frames[j].name, MDL_FRAME_NAME_MAX, 1, f) != 1)
          return setError("Error reading mdl file", f);

        // frame vertices
        framesets[i].frames[j].trivert = new trivertx_t[nbvertices];
        if (fread(framesets[i].frames[j].trivert,
	  sizeof(trivertx_t) * nbvertices, 1, f) != 1)
          return setError("Error reading mdl file", f);
      }
    }
  }

  fclose(f);
  return true;
}

void Mdl::dumpstats(FILE* s) const
{
  csFPrintf(s, "\nQuake Model (MDL) Statistics:\n");
  csFPrintf(s, "Skins:     %d\n", nbskins);
  csFPrintf(s, "Frames:    %d\n", nbframesets);
  csFPrintf(s, "Triangles: %d\n", nbtriangles);
  csFPrintf(s, "Vertices:  %d\n", nbvertices);
}

bool Mdl::WriteSPR(const char* spritename, float scaleMdl, int delayMdl,
  float positionMdlX, float positionMdlY, float positionMdlZ,
  bool actionNamingMdl, bool resizeSkin, int maxFrames) const
{
  FILE *f;
  char *spritefilename;
  Mdl spr;
  int i = 0, j = 0, k = 0, v = 0, vertex = 0;
  float x = 0;
  float y = 0;

  if (spritename == 0 || strlen(spritename) == 0)
  {
    csFPrintf(stderr, "Unable to save: 0 sprite name\n");
    return false;
  }

  spritefilename = new char [strlen(spritename) + 5];
  strcpy(spritefilename, spritename);
  strcat(spritefilename, ".spr");

  // generate skin texture
  if (nbskins < 1)
    csFPrintf(stderr, "Warning: no skin in this model\n");
  else
  {
    //if (mdl2spr_imageio == 0)
    if (!mdl2spr_imageio.IsValid ())
    {
      mdl2spr_imageio =
        SCF_CREATE_INSTANCE ("crystalspace.graphic.image.io.png", iImageIO);
    }
    if (mdl2spr_imageio == 0)
      csFPrintf (stderr, "Unable to load PNG plugin; can't save skins.\n");
    else
    {
      csPrintf("Generate skin texture file\n");

      if (resizeSkin)
      {
	csPrintf("\tOld Skin size = %dx%d\n", skinwidth, skinheight);

	x = (float) skinwidth;
	y = (float) skinheight;
	spr.skinwidth  = 1;
	spr.skinheight = 1;
	while (x > 1) { x /= 2; spr.skinwidth  *= 2; }
	while (y > 1) { y /= 2; spr.skinheight *= 2; }

	csPrintf("\tNew Skin size = %dx%d\n", spr.skinwidth, spr.skinheight);
      }
      else // resizeSkin == false
      {
	csPrintf("\tSkin size = %dx%d\n", skinwidth, skinheight);
	spr.skinwidth  = skinwidth;
	spr.skinheight = skinheight;
      }

      // @@@ FIXME: Support option to load user-specified palette.lmp file.
      const unsigned char* const palette = DefaultQuakePalette;
      spr.skins = new skin_t [nbskins];

      csString skinfilename;

      for (i = 0; i < nbskins; i++)
      {
	spr.skins[i].texs = new unsigned char*[skins[i].nbtexs];
	for (j = 0; j < skins[i].nbtexs; j++)
	{
	  spr.skins[i].texs[j] =
	    new unsigned char [spr.skinwidth * spr.skinheight];

	  if (skins[i].nbtexs > 1)
	    skinfilename.Format ("%s%d%c.png", spritename, i, j + 'a');
	  else if (nbskins > 1)
	    skinfilename.Format ("%s%d.png", spritename, i);
	  else
	    skinfilename.Format ("%s.png", spritename);

	  if (!resizeSkin)
	    for (k = 0; k < skinwidth * skinheight; k++)
	      spr.skins[i].texs[j][k] = skins[i].texs[j][k];
	   else
	  {
	    float yscale = (float)skinheight / (float)spr.skinheight;
	    float xscale = (float)skinwidth / (float)spr.skinwidth;
	    y = 0.5;
		int row, col;
	    for (row = 0; row < spr.skinheight; row++, y += yscale)
	    {
	      x = 0.5;
	      for (col = 0; col < spr.skinwidth; col++, x += xscale)
		spr.skins[i].texs[j][col + row * spr.skinwidth] =
		  skins[i].texs[j][((int)x) + ((int)y) * skinwidth];
	    }
	  }

	  SkinImage img (
	    spr.skins[i].texs[j], palette, spr.skinwidth, spr.skinheight);
	  csRef<iDataBuffer> db (mdl2spr_imageio->Save (&img, "image/png"));
	  if (db)
	  {
	    FILE *f = fopen (skinfilename, "w+");
	    size_t n = 0;
	    if (f)
	      n = fwrite (db->GetData (), 1, db->GetSize (), f);
	    if (!f || n != db->GetSize ())
	      csFPrintf(stderr, "Error when writing `%s'.\n", skinfilename.GetData());
	    if (f) fclose (f);
	  }
	}
      }
    }
  }

  if ((f = fopen(spritefilename, "w")) == 0)
  {
    csFPrintf(stderr, "Cannot open sprite file %s for writing\n", spritename);
    return false;
  }

  // begin hard work now
  csFPrintf(f, "<meshfact name=\"%s\">\n", spritename);
  csFPrintf(f, "\t<plugin>crystalspace.mesh.loader.factory.sprite.3d</plugin>\n");
  csFPrintf(f, "\t<params>\n");
  csFPrintf(f, "\t\t<material>%s</material>\n", spritename);
  csPrintf("Generate MDL/SPR vertex correspondence\n");

  // count back seam vertices
  long back_seam_verts = 0;
  long* BS_verts = new long [nbvertices];
  unsigned char* verts = new unsigned char [nbvertices];
  memset (verts, false, nbvertices * sizeof(verts[0]));

  // detect which vertices are back seam vertices
  for (i = 0; i < nbtriangles; i++)
    if (!triangles[i].facefront)
      for (j = 0; j < 3; j++)
      {
        vertex = triangles[i].vertice[j];
        if (vertices[vertex].onseam)
	  verts[vertex] = true;
      }

  // assign a new, unique vertex number to each back skin vertex
  for (i = 0; i < nbvertices; i++)
    if (verts[i])
    {
      BS_verts[i] = nbvertices + back_seam_verts;
      back_seam_verts++;
    }

  csPrintf("\t%ld back seam vertices detected\n", back_seam_verts);

  // create sprite skin vertices
  spr.triangles = new triangle_t [nbtriangles];
  spr.vertices =
    new vertice_t  [nbvertices * 2 * sizeof(vertice_t) + back_seam_verts];
  memset(spr.triangles, 0, nbtriangles * sizeof(triangle_t));
  memset(spr.vertices,  0, nbvertices * 2 * sizeof(vertice_t));

  // find corresponding mdl skin vertices
  for (i = 0; i < nbtriangles; i++)
    for (j = 0; j < 3; j++)
    {
      vertex = triangles[i].vertice[j];

      // copy mdl vertices to sprite
      spr.vertices[vertex].s = vertices[vertex].s;
      spr.vertices[vertex].t = vertices[vertex].t;
      spr.triangles[i].vertice[j] = vertex;

      // create a duplicate vertex for back seam triangles
      if (vertices[vertex].onseam && !triangles[i].facefront)
      {
        spr.vertices[BS_verts[vertex]].s = vertices[vertex].s + (skinwidth/2);
        spr.vertices[BS_verts[vertex]].t = vertices[vertex].t;
        spr.triangles[i].vertice[j] = BS_verts[vertex];
      }
    }

  // create sprite frameset
  spr.framesets = new frameset_t [nbframesets];
  for (i = 0; i < nbframesets; i++)
  {
    spr.framesets[i].nbframes = framesets[i].nbframes;
    spr.framesets[i].frames = new frame_t [spr.framesets[i].nbframes];
    for (j = 0; j < framesets[i].nbframes; j++)
      spr.framesets[i].frames[j].trivert =
        new trivertx_t [nbvertices+back_seam_verts];
  }

  // copy corresponding mdl framesets
  for (i = 0; i < nbframesets; i++)
    for (j = 0; j < framesets[i].nbframes; j++)
      for (k = 0; k < nbtriangles; k++)
        for (v=0; v<3; v++)
        {
          long SPR_vertex = spr.triangles[k].vertice[v];
          long MDL_vertex = triangles[k].vertice[v];

          spr.framesets[i].frames[j].trivert[SPR_vertex].packedposition[0] =
            framesets[i].frames[j].trivert[MDL_vertex].packedposition[0];
          spr.framesets[i].frames[j].trivert[SPR_vertex].packedposition[1] =
            framesets[i].frames[j].trivert[MDL_vertex].packedposition[1];
          spr.framesets[i].frames[j].trivert[SPR_vertex].packedposition[2] =
            framesets[i].frames[j].trivert[MDL_vertex].packedposition[2];
        }

  // extract only n frames if maxFrames was specified
  int outFrames = nbframesets;
  if (maxFrames!=-1)
	outFrames = maxFrames;

  csPrintf("Generate Frames\n");
  for (i = 0; i < outFrames; i++)
  {
    for (j = 0; j < framesets[i].nbframes; j++)
    {
      csFPrintf(f, "\t\t<frame name=\"%s\">", framesets[i].frames[j].name);
      for (k = 0; k < nbvertices + back_seam_verts; k++)
      {
        // it seem y and z are switched
        float x=(float)spr.framesets[i].frames[j].trivert[k].packedposition[0];
        float z=(float)spr.framesets[i].frames[j].trivert[k].packedposition[1];
        float y=(float)spr.framesets[i].frames[j].trivert[k].packedposition[2];
        float u=(float)spr.vertices[k].s;
        float v=(float)spr.vertices[k].t;

        x = ((x * scaleX) + originX) * scaleMdl;
        y = ((y * scaleZ) + originZ) * scaleMdl;
        z = ((z * scaleY) + originY) * scaleMdl;

        x += positionMdlX;
        y += positionMdlY;
        z += positionMdlZ;

        u = u / (float) skinwidth;
        v = v / (float) skinheight;

        csFPrintf(f, " <v x=\"%.3f\" y=\"%.3f\" z=\"%.3f\" u=\"%.2f\" v=\"%.2f\"/>", x, y, z, u, v);
      }
      csFPrintf(f, " </frame>\n");
    }
  }

  csPrintf("Generate Actions\n");
  for (i = 0; i < outFrames; i++)
  {
    if (framesets[i].group)
    {
      csString name_action;

      size_t base_action = strlen(framesets[i].frames[0].name);
      for (j = (int)strlen(framesets[i].frames[0].name) - 1; j > 1; j--)
        if (!isdigit(framesets[i].frames[0].name[j]))
	  break;
      base_action=j + 1;
      name_action.Append (framesets[i].frames[0].name, base_action);

      csFPrintf(f, "\t\t<action name=\"%s\">", name_action.GetData());

      for (j = 0; j < framesets[i].nbframes; j++)
      {
        float delay = framesets[i].delay[j];
        if (j > 0)
	  delay -= framesets[i].delay[j - 1];
        delay *= 1000.0;
        csFPrintf(f, " <f name=\"%s\" delay=\"%d\"/>", framesets[i].frames[j].name, (int)delay);
      }
      csFPrintf(f, " </action>\n");
    }
    else if (actionNamingMdl)
    {
      if (!isdigit(
        framesets[i].frames[0].name[strlen(framesets[i].frames[0].name) - 1]))
        csFPrintf(f, "\t\t<action name=\"%s\"> <f name=\"%s\" delay=\"%d\"/> </action>\n",
	  framesets[i].frames[0].name, framesets[i].frames[0].name, delayMdl);
      else
      {
        csString name_action;

        int base_action;
        for (j = (int)strlen(framesets[i].frames[0].name) - 1; j > 1; j--)
          if (!isdigit(framesets[i].frames[0].name[j]))
	    break;
        base_action=j + 1;
        name_action.Append (framesets[i].frames[0].name, base_action);

        csFPrintf(f, "\t\t<action name=\"%s\">", name_action.GetData());

        csFPrintf(f, " <f name=\"%s\" delay=\"%d\"/>", framesets[i].frames[0].name, delayMdl);
        for (j = i + 1; j < outFrames; j++, i++)
        {
          if (framesets[j].group)
	    break;
          char toy[64], toy2[64];
          bool scrash = false;
          strcpy(toy, name_action);
          strcat(toy, "%s");
          sscanf(framesets[j].frames[0].name, toy, &toy2);
          for (k = 0; k < (int)strlen(toy2); k++)
          {
            if (!isdigit(toy2[k]))
            {
              scrash = true;
              break;
            }
          }
          if (scrash)
	    break;

          if (strncmp(name_action,framesets[j].frames[0].name,base_action)==0)
            csFPrintf(f, " <f name=\"%s\" delay=\"%d\"/>", framesets[j].frames[0].name, delayMdl);
          else
	    break;
        }
        csFPrintf(f, " </action>\n");
      }
    }
  }

  csPrintf("Generate Triangles\n");
  for (i = 0; i < nbtriangles; i++)
  {
    csFPrintf(f, "\t\t<t v1=\"%" PRId32 "\" v2=\"%" PRId32 "\" v3=\"%" PRId32 "\"/>\n",
      spr.triangles[i].vertice[0],
      spr.triangles[i].vertice[1],
      spr.triangles[i].vertice[2]);
  }

  csFPrintf(f, "\t</params>\n</meshfact>\n");
  fclose(f);
  return true;
}
