/*
    Crystal Space 3d format converter 


    Based on IVCON - converts various 3D graphics file
	Author: John Burkardt - used with permission
	CS adaption and conversion to C++ classes  Bruce Williams
	MD2 converter -- Gary Haussmann

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
#include "cstool/impexp.h"

// all int's in an MD2 file are little endian
#include "cssys/csendian.h"

// upper bound onsize of biggest data element (vertex, polygon) in an MD2 file
static int const MAX_DATAELEMENT_SIZE = 8192;

// size of various MD2 elements
static int const SIZEOF_MD2SHORT = 2;
static int const SIZEOF_MD2LONG = 4;
static int const SIZEOF_MD2FLOAT = 4;
static int const SIZEOF_MD2INFOBLOCK = 15 * SIZEOF_MD2LONG;
static int const SIZEOF_MD2SKINNAME = 64;
static int const SIZEOF_MD2FRAMENAME = 16;
static int const SIZEOF_MDLINFOBLOCK = 11 * SIZEOF_MD2FLOAT + 8 * SIZEOF_MD2LONG;


struct md2framedata {
  char name[16];
  float *vertexcoords;
};

// frame manipulator class, to stick a specific frame from the quake
// file into the converter class
class csConverter_MD2FrameManipulator :
  public csConverter_FrameManipulator
{
  public:
    csConverter_MD2FrameManipulator(converter *data_target, md2framedata* data_source, int numframes);
    ~csConverter_MD2FrameManipulator();

    int GetMaxAllowedFrame() const;
    int SetFrame(int setto);

  protected:
    md2framedata *m_frame_array;    
    int m_total_frame_count;
};

csConverter_MD2FrameManipulator::csConverter_MD2FrameManipulator(converter* data_target, md2framedata* data_source, int numframes)
  : csConverter_FrameManipulator(data_target),
    m_frame_array(data_source), m_total_frame_count(numframes)
{
}

csConverter_MD2FrameManipulator::~csConverter_MD2FrameManipulator()
{
  if (m_frame_array)
  {
	int frameindex;
    for (frameindex=0; frameindex < m_total_frame_count; frameindex++)
    {
      if (m_frame_array[frameindex].vertexcoords)
        delete [] m_frame_array[frameindex].vertexcoords;
    }
    delete[] m_frame_array;
  }
}

int csConverter_MD2FrameManipulator::GetMaxAllowedFrame() const
{
  // frame indices are 0-based so we subtract one
  return m_total_frame_count-1;
}

int csConverter_MD2FrameManipulator::SetFrame(int framenumber)
{
  // is it a legal frame number?
  if ( (framenumber < 0) ||
       (framenumber > GetMaxAllowedFrame() ) )
  { framenumber = 0; }

  md2framedata *setframeto = &m_frame_array[framenumber];

  // copy over the frame name
  memset(m_data_target->object_name,0,sizeof(char)*81);
  strncpy(m_data_target->object_name,setframeto->name,16);

  // copy over the vertex location data--all other data remains unchanged
  float *md2vertexwalk = setframeto->vertexcoords;
  int coordindex;
  for (coordindex=0; coordindex < m_data_target->num_cor3; coordindex++)
  {
    m_data_target->cor3[0][coordindex] = *md2vertexwalk++;
    m_data_target->cor3[1][coordindex] = *md2vertexwalk++;
    m_data_target->cor3[2][coordindex] = *md2vertexwalk++;
  }

  // according to the function definition, we return the highest
  // allowed frame
  return GetMaxAllowedFrame();
}

// helper class to manage the vertex mappings.  This class takes in
// index pairs--one indexes into an array of xyz coordinates, the other
// indexes into an array of texture coordinates--and remaps them into
// an array of combined xyz+texture coordinate vertices.  The final
// size of the combined vertex array will be equal to the number of
// unique xyz/texture coordinate index pairs used in the original array.
// This class is needed because the Quake2 MD2 file format uses
// indices into two separate coordinate arrays, one for xyz coordinates, one
// for texture coordinates.  Since CS uses a combined format, where every
// vertex contains xyz+texture coordinates bound together, we must build
// a set of vertices differently than the quake2 format.

class md2vertexset {
  public:
    md2vertexset() : numindices(0), xyzcoordindices(NULL), texcoordindices(NULL) {};
    ~md2vertexset()
    { if (xyzcoordindices) delete [] xyzcoordindices;
      if (texcoordindices) delete [] texcoordindices; }

    int vertexindexcount() const { return numindices; }

    // go from xyz_index and texcoord_index to a single cs vertex_index
    short get_csvertexindex(short xyzindex, short texindex)
    {
      if (numindices == 0)
      {
        xyzcoordindices = new short[1]; texcoordindices = new short[1];
        xyzcoordindices[0] = xyzindex; texcoordindices[0] = texindex;
	return numindices++;
      }
      int csindex;
      for (csindex=0; csindex < numindices; csindex++)
      {
        if (   (xyzcoordindices[csindex] == xyzindex)
	    && (texcoordindices[csindex] == texindex) )
	    return csindex;
      }
      // no match found, make another spot
      short *newxyzindices = new short[numindices+1];
      short *newtexindices = new short[numindices+1];
      for (csindex=0; csindex < numindices; csindex++)
      { newxyzindices[csindex] = xyzcoordindices[csindex];
        newtexindices[csindex] = texcoordindices[csindex]; }
      delete [] xyzcoordindices;
      delete [] texcoordindices;
      newxyzindices[numindices] = xyzindex;
      newtexindices[numindices] = texindex;
      xyzcoordindices = newxyzindices;
      texcoordindices = newtexindices;
      return numindices++;
    }

    // go from a single cs vertex_index to the xyz,texcoord index pair
    void get_md2vertexmap(short csindex, short &xyzindex, short &texindex)
    {
      xyzindex = xyzcoordindices[csindex];
      texindex = texcoordindices[csindex];
    }

  private:
    int numindices;
    short *xyzcoordindices;
    short *texcoordindices;
};

/*****************************************************************************/

int converter::mdl_read ( FILE *filein ) {

/*****************************************************************************/

/* Read in a Quake MDL model file */
  unsigned char readbuffer[MAX_DATAELEMENT_SIZE];

  // Read in header and check for a correct file.  The
  // header consists of two longs, containing
  // the magic identifier 'IDPO' as the first long,
  // followed by a version number (6)
  fread(readbuffer, SIZEOF_MD2LONG, 2, filein);
  unsigned long fileid = get_le_long(readbuffer), 
  		fileversion = get_le_long(readbuffer+4);
  if (fileid != ( ('O'<<24)+('P'<<16)+('D'<<8)+'I' ) )
  {
    return ERROR;
  }
  if (fileversion != 6)
  {
    return ERROR;
  }

  // read in sprite info:
  //0  scale: 3 floats--scale all coordinates by this
  //3  origin: 3 floats--translate all coordinates by this
  //6  radius: float--radius of bounding sphere for the model
  //7  offsets: 3 floats--position of player camera for this model
  //10 numskins: long--number of textures for skin
  //11 skinwidth: long--width of skin texture in pixels
  //12 skinheight: long--height of skin texture in pixels
  //13 numverts: long--number of vertices
  //14 numtris: long--number of faces
  //15 numframesets: long--number of animation frame sets (actions)
  //16 synctype: long--???
  //17 flags: long--???
  //18 size: float--average triangle size (in pixels?)

  fread(readbuffer,SIZEOF_MDLINFOBLOCK,1,filein);

  num_face = get_le_long(readbuffer + 14*SIZEOF_MD2LONG); // numtris
  num_texmap = get_le_long(readbuffer + 10*SIZEOF_MD2LONG); // numskins

  long skinwidth = get_le_long(readbuffer + 11*SIZEOF_MD2LONG),
	skinheight = get_le_long(readbuffer + 12*SIZEOF_MD2LONG),
  	framesetcount = get_le_long(readbuffer + 15*SIZEOF_MD2LONG), // numframesets
	vertexcount = get_le_long(readbuffer + 13*SIZEOF_MD2LONG);

  // get scale and translate values.  These will be applied to every
  // frame coordinate value read in later.
  float scale[3], translate[3];
  float *floatvals = (float *)readbuffer;
  int transformindex;
  for (transformindex = 0; transformindex < 3; transformindex++)
  {
    scale[transformindex] = convert_endian(floatvals[transformindex]);
    translate[transformindex] = convert_endian(floatvals[3 + transformindex]);
  }

  // skip over all skin data.  Skins are read in separately; this module
  // is for reading in geometry
  unsigned char *skinholder = new unsigned char[skinwidth*skinheight];
  int skinindex, multiskinindex;
  for (skinindex = 0; skinindex < num_texmap; skinindex++)
  {
    fread(readbuffer,SIZEOF_MD2LONG,1,filein);
    int group = get_le_long(readbuffer);
    if (group == 0) // one skin
    {
      fread(skinholder,skinwidth*skinheight,sizeof(unsigned char),filein);
    }
    else if (group == 1) // multiskin
    {
      fread(readbuffer,SIZEOF_MD2LONG,1,filein);
      int multiskincount = get_le_long(readbuffer);
      fread(readbuffer,SIZEOF_MD2FLOAT,multiskincount,filein);
      for (multiskinindex = 0; multiskinindex < multiskincount; multiskinindex++)
      {
        fread(skinholder,skinwidth*skinheight,sizeof(unsigned char),filein);
      }
    }
    else //broken
    {
      delete [] skinholder;
      return ERROR;
    }
  }

  delete [] skinholder;

  // read in vertex texture coordinate info.
  // we also need to remember the 'seamage' information for later
  float *packed_s = new float[vertexcount*2];
  float *packed_t = new float[vertexcount*2];
  int *packed_seam = new int[vertexcount];
  int vertexindex;
  for(vertexindex = 0; vertexindex < vertexcount; vertexindex++)
  { 
    fread(readbuffer, SIZEOF_MD2LONG, 3, filein);
    packed_seam[vertexindex] = get_le_long(readbuffer);
    packed_s[vertexindex] = get_le_long(readbuffer + SIZEOF_MD2LONG) / (float)skinwidth;
    packed_t[vertexindex] = get_le_long(readbuffer + 2 * SIZEOF_MD2LONG) /(float)skinheight;
   
    // if on a seam, also record texture coords for the other part of the seam
    if (packed_seam[vertexindex])
    {
      packed_s[vertexindex+vertexcount] = packed_s[vertexindex] + 0.5;
      packed_t[vertexindex+vertexcount] = packed_t[vertexindex];
    }
  }

  // read in face connectivity data
  // if a vertex is on a seam, we may have to create a second vertex
  // for faces on opposite sides of the seam.  These are marked as
  // 'front' and 'back' vertices
  md2vertexset vertexlist;
  int faceindex, partindex;
  for (faceindex = 0; faceindex < num_face; faceindex++)
  {
    fread(readbuffer, SIZEOF_MD2LONG, 4, filein);
    int frontpoly = get_le_long(readbuffer);
    face_order[faceindex] = 3;
    for (partindex = 0; partindex < 3; partindex++)
    {
      int cornerindex = 
      	get_le_long(readbuffer + (1+partindex) * SIZEOF_MD2LONG);
      int realindex;
      if (packed_seam[cornerindex] && !frontpoly)
      {
        realindex = vertexlist.get_csvertexindex(cornerindex,cornerindex+vertexcount);
      }
      else
      {
        realindex = vertexlist.get_csvertexindex(cornerindex,cornerindex);
      }
      face[partindex][faceindex] = realindex;
    }
  }

  // now we know the real count of vertices, including seam breaks
  // build up the vertex data
  num_cor3 = vertexlist.vertexindexcount();

  // build up texture coord data
  for (vertexindex = 0; vertexindex < num_cor3; vertexindex++)
  {
    short xyz_index, tex_index;
    vertexlist.get_md2vertexmap(vertexindex,xyz_index, tex_index);
    cor3_uv[0][vertexindex] = packed_s[tex_index];
    cor3_uv[1][vertexindex] = packed_t[tex_index];
  }
  delete [] packed_s;
  delete [] packed_t;
  delete [] packed_seam;

  // next read in xyz data
  md2framedata **file_framesetdata = new md2framedata *[framesetcount];
  int *framesetsizes = new int[framesetcount];

  int framecount = 0;
  int framesetindex;
  int subframeindex;
  for (framesetindex = 0; framesetindex < framesetcount; framesetindex++)
  {
    fread(readbuffer, SIZEOF_MD2LONG, 1, filein);
    int framesettype = get_le_long(readbuffer);
    int framesetsize;
    if (framesettype == 0) // single frame action
    {
      framesetsize = 1;
    }
    else // multi-frame action
    {
      fread(readbuffer, SIZEOF_MD2LONG, 1, filein);
      framesetsize = get_le_long(readbuffer);

      // skip some stuff--general min/max, frame timings
      fread(readbuffer, SIZEOF_MD2LONG, (2 + framesetsize), filein);
    }

    md2framedata *file_framedata = new md2framedata[framesetsize];
    file_framesetdata[framesetindex] = file_framedata;
    framesetsizes[framesetindex] = framesetsize;

    for (subframeindex = 0; subframeindex < framesetsize; subframeindex++)
    {
      // skip min/max info
      fread(readbuffer, SIZEOF_MD2LONG, 2, filein);

      // get frame name
      fread(file_framedata[subframeindex].name, 16, 1, filein);

      // allocate space for vertex coordinates
      float *raw_vertexcoords = new float[3*num_cor3];

      // read in xyz data and spread that out to the cs vertex set
      fread(readbuffer, 1, 4*vertexcount, filein);
      for(vertexindex = 0; vertexindex < num_cor3; vertexindex++)
      {
        short xyz_index, texindex;
	vertexlist.get_md2vertexmap(vertexindex,xyz_index,texindex);

	raw_vertexcoords[vertexindex*3] = 
		readbuffer[xyz_index*4] * scale[0] + translate[0];
	raw_vertexcoords[vertexindex*3+1] = 
		readbuffer[xyz_index*4+1] * scale[1] + translate[1];
	raw_vertexcoords[vertexindex*3+2] = 
		readbuffer[xyz_index*4+2] * scale[2] + translate[2];
      }

      // store the vertex coordinate data
      file_framedata[subframeindex].vertexcoords = raw_vertexcoords;

      // next frame
      framecount++;
    }
  }

  // need to remember the number of frames
  num_object = framecount;

  // flatten all the framesets into one large batch of frames
  md2framedata *file_framedata = new md2framedata[framecount];
  md2framedata *curframe = file_framedata;
  for (framesetindex = 0; framesetindex < framesetcount; framesetindex++)
  {
    for (subframeindex = 0; subframeindex < framesetsizes[framesetindex]; subframeindex++)
    {
      curframe->vertexcoords = file_framesetdata[framesetindex][subframeindex].vertexcoords;
      strcpy (curframe->name, file_framesetdata[framesetindex][subframeindex].name);
      curframe++;
    }
    delete [] file_framesetdata[framesetindex];
  }

  delete [] file_framesetdata;
  delete [] framesetsizes;

  // stuff all the generated data in a manipulator for later use
  frame_builder = new csConverter_MD2FrameManipulator(this, file_framedata,num_object);

  return SUCCESS;
}
