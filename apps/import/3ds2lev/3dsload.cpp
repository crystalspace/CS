
/*
 *  3D Studio object reader
 */

#include "cssysdef.h"
#include "cssys/csendian.h"
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "3dsload.h"
#include "3dsco.h"

extern int flags;
extern int curmodel;
extern int modelnum;

static inline dword convert_endian2 (dword l)
{
  return (dword)convert_endian ((ULong)l);
}

static inline word convert_endian2 (word w)
{
  return (word)convert_endian ((UShort)w);
}

static inline float32 convert_endian2 (float32 f)
{
  return (float32)convert_endian ((float)f);
}

static void  (* dread)  (void * dest, int len);
static void  (* dsetpos)(dword pos);
static dword (* dgetpos)(void);

static jmp_buf EnvState;


#include <stdio.h>

static FILE * InFile=0;

static void FileRead (void * dest, int len)
{
  if (fread(dest, len, 1, InFile) != 1)
  {
    //printf("Error reading file\n");
    longjmp (EnvState, 1);
  }
}

static void FileSetpos (dword pos)
{
  if (fseek(InFile, (long)pos, SEEK_SET) != 0)
  {
    //printf("Error moving filepointer\n");
    longjmp (EnvState, 1);
  }
}

static dword FileGetpos (void)
{
  long pos;
  if ((pos=ftell(InFile)) == -1L)
  {
    //printf("Error getting fileposition\n");
    longjmp (EnvState, 1);
  }
  return (dword) pos;
}

static H3dsScene * Scene;

H3dsMeshObj * GetMeshObj(void)
{
  void * mem;
  if ((mem=realloc(Scene->meshobjlist,
          sizeof(H3dsMeshObj)*(Scene->meshobjs+1))) == 0)
  {
    //printf("Error reallocating mem\n");
    longjmp (EnvState, 1);
  }
  Scene->meshobjlist=(H3dsMeshObj *) mem;
  H3dsMeshObj * obj=&Scene->meshobjlist[Scene->meshobjs++];
  memset (obj, 0, sizeof(H3dsMeshObj));
  return obj;
}

static void * getmem (int size)
{
  void * mem;
  if((mem=malloc(size))==0)
  {
    //printf("Error allocating mem\n");
    longjmp (EnvState, 1);
  }
  return mem;
}

// Each 3DS data-chunk starts with a 6 byte header.
// The first item in the header is a 2 byte (word) id-number.
// After that follows a dword wich gives the size of
// the data-chunk including the header. The size can be used
// as an relative offset to the next chunk.

// tab 4
enum {
  CHUNK_RGBF        = 0x0010,
  CHUNK_RGBB        = 0x0011,
//  CHUNK_RBGB2       = 0x0012,  // ?? NOT HLS.
  CHUNK_MAIN        = 0x4D4D,
  CHUNK_OBJMESH     = 0x3D3D,
  CHUNK_BKGCOLOR    = 0x1200,
  CHUNK_AMBCOLOR    = 0x2100,
  CHUNK_OBJBLOCK    = 0x4000,
  CHUNK_TRIMESH     = 0x4100,
  CHUNK_VERTLIST    = 0x4110,
  CHUNK_FACELIST    = 0x4120,
  CHUNK_FACEMAT     = 0x4130,
  CHUNK_MAPLIST     = 0x4140,
  CHUNK_SMOOLIST    = 0x4150,
  CHUNK_TRMATRIX    = 0x4160,
  CHUNK_LIGHT       = 0x4600,
  CHUNK_SPOTLIGHT   = 0x4610,
  CHUNK_CAMERA      = 0x4700,
  CHUNK_VPLAYOUT    = 0x7001,
  CHUNK_VPDATA      = 0x7011,
  CHUNK_VPDATA3     = 0x7012,
  CHUNK_VPSIZE      = 0x7020,
  CHUNK_MATERIAL    = 0xAFFF,
  CHUNK_MATNAME     = 0xA000,
  CHUNK_AMBIENT     = 0xA010,
  CHUNK_DIFFUSE     = 0xA020,
  CHUNK_SPECULAR    = 0xA030,
  CHUNK_TEXTURE     = 0xA200,
  CHUNK_BUMPMAP     = 0xA230,
  CHUNK_MAPFILE     = 0xA300,
  CHUNK_KEYFRAMER   = 0xB000,
  CHUNK_OBJDES      = 0xB002,
  CHUNK_KFFRAMES    = 0xB008,
  CHUNK_KFCURTIME   = 0xB009,
  CHUNK_KFHEADER    = 0xB00A,
  CHUNK_NODEHDR     = 0xB010,
  CHUNK_INSTNAME    = 0xB011,
  CHUNK_PIVOT       = 0xB013,
  CHUNK_BOUNDBOX    = 0xB014,
  CHUNK_MORPHSMOOTH = 0xB015,
  CHUNK_POSTRACKTAG = 0xB020,
  CHUNK_ROTTRACKTAG = 0xB021,
  CHUNK_SCLTRACKTAG = 0xB022,
  CHUNK_NODEID      = 0xB030
};

static char* GetChunkName (int id)
{
  switch (id)
  {
    case CHUNK_RGBF: return "RGBF";
    case CHUNK_RGBB: return "RGBB";
    case CHUNK_MAIN: return "MAIN";
    case CHUNK_OBJMESH: return "OBJMESH";
    case CHUNK_BKGCOLOR: return "BKGCOLOR";
    case CHUNK_AMBCOLOR: return "AMBCOLOR";
    case CHUNK_OBJBLOCK: return "OBJBLOCK";
    case CHUNK_TRIMESH: return "TRIMESH";
    case CHUNK_VERTLIST: return "VERTLIST";
    case CHUNK_FACELIST: return "FACELIST";
    case CHUNK_FACEMAT: return "FACEMAT";
    case CHUNK_MAPLIST: return "MAPLIST";
    case CHUNK_SMOOLIST: return "SMOOLIST";
    case CHUNK_TRMATRIX: return "TRMATRIX";
    case CHUNK_LIGHT: return "LIGHT";
    case CHUNK_SPOTLIGHT: return "SPOTLIGHT";
    case CHUNK_CAMERA: return "CAMERA";
    case CHUNK_VPLAYOUT: return "VPLAYOUT";
    case CHUNK_VPDATA: return "VPDATA";
    case CHUNK_VPDATA3: return "VPDATA3";
    case CHUNK_VPSIZE: return "VPSIZE";
    case CHUNK_MATERIAL: return "MATERIAL";
    case CHUNK_MATNAME: return "MATNAME";
    case CHUNK_AMBIENT: return "AMBIENT";
    case CHUNK_DIFFUSE: return "DIFFUSE";
    case CHUNK_SPECULAR: return "SPECULAR";
    case CHUNK_TEXTURE: return "TEXTURE";
    case CHUNK_BUMPMAP: return "BUMPMAP";
    case CHUNK_MAPFILE: return "MAPFILE";
    case CHUNK_KEYFRAMER: return "KEYFRAMER";
    case CHUNK_OBJDES: return "OBJDES";
    case CHUNK_KFFRAMES: return "KFFRAMES";
    case CHUNK_KFCURTIME: return "KFCURTIME";
    case CHUNK_KFHEADER: return "KFHEADER";
    case CHUNK_NODEHDR: return "NODEHDR";
    case CHUNK_INSTNAME: return "INSTNAME";
    case CHUNK_PIVOT: return "PIVOT";
    case CHUNK_BOUNDBOX: return "BOUNDBOX";
    case CHUNK_MORPHSMOOTH: return "MORPHSMOOTH";
    case CHUNK_POSTRACKTAG: return "POSTRACKTAG";
    case CHUNK_ROTTRACKTAG: return "ROTTRACKTAG";
    case CHUNK_SCLTRACKTAG: return "SCLTRACKTAG";
    case CHUNK_NODEID: return "NODEID";
    default: return "?";
  }
}

static void VerbosePrintChunk (const char* where, int indent, int id, int len,
	const char* name = NULL)
{
  if (flags & FLAG_VERYVERBOSE)
  {
    static char *def_indstr = "\
                                                  \
                                                  \
                                                  \
                                                  \
                                                  ";
    char indstr[255];
    strcpy (indstr, def_indstr);
    indstr[indent] = 0;
    fprintf (stderr, "%s%s (%s): Read chunk '%s' id=%04x len=%d\n",
      indstr, where,
      name ? name : "",
      GetChunkName (id), id, len);
  }
}

static void ReadVertList (dword, H3dsMeshObj * meshobj)
{
  word nv;
  dread(&nv, sizeof(nv));
  nv = convert_endian2 (nv);
  meshobj->verts=nv;
  int k=nv;
  meshobj->vertlist=(H3dsVert *) getmem(sizeof(H3dsVert)*k);
  int n;
  for (n=0; n<k; n++)
  {
    dread (&meshobj->vertlist[n], sizeof(float32)*3);
    meshobj->vertlist[n].x = convert_endian2 (meshobj->vertlist[n].x);
    meshobj->vertlist[n].y = convert_endian2 (meshobj->vertlist[n].y);
    meshobj->vertlist[n].z = convert_endian2 (meshobj->vertlist[n].z);
  }
}

static void ReadFaceList (dword, H3dsMeshObj * meshobj)
{
  word nv;
  dread(&nv, sizeof(nv));
  nv = convert_endian2 (nv);
  meshobj->faces=nv;
  int k=nv;
  meshobj->facelist=(H3dsFace *) getmem(sizeof(H3dsFace)*k);
  int n;
  for (n=0; n<k; n++)
  {
    dread (&meshobj->facelist[n], sizeof(word)*4);
    meshobj->facelist[n].p0 = convert_endian2 (meshobj->facelist[n].p0);
    meshobj->facelist[n].p1 = convert_endian2 (meshobj->facelist[n].p1);
    meshobj->facelist[n].p2 = convert_endian2 (meshobj->facelist[n].p2);
    meshobj->facelist[n].flags = convert_endian2 (meshobj->facelist[n].flags);
  }
}

static void ReadMapList (dword, H3dsMeshObj * meshobj)
{
  word nv;
  dread (&nv, sizeof(nv));
  nv = convert_endian2 (nv);
  meshobj->maps=nv;
  int k=nv;
  meshobj->maplist=(H3dsMap *) getmem(sizeof(H3dsMap)*k);
  int n;
  for (n=0; n<k; n++)
  {
    dread (&meshobj->maplist[n], sizeof(float32)*2);
    meshobj->maplist[n].u = convert_endian2 (meshobj->maplist[n].u);
    meshobj->maplist[n].v = convert_endian2 (meshobj->maplist[n].v);
  }
}

static void ReadTraMatrix (dword, H3dsMeshObj * meshobj)
{
  dread (&meshobj->TraMatrix, sizeof(float32)*12);
  int i;
  for (i = 0 ; i < 12 ; i++)
    meshobj->TraMatrix[i] = convert_endian2 (meshobj->TraMatrix[i]);
  meshobj->matrix=1;
}

static void ReadViewSection (int indent, dword p)
{
  word id;
  dword len, pc;
  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadViewSection", indent, id, len);
#if 0
    switch ((int)id)
    {
      default: dsetpos(pc+len);
    }
#else
    dsetpos(pc+len);
#endif
  }
}

static void ReadObjDesSection (int indent, dword p)
{
  word id;
  dword len, pc;
  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadObjDesSection", indent, id, len);
#if 0
    switch ((int)id)
    {
      default: dsetpos(pc+len);
    }
#else
    dsetpos(pc+len);
#endif
  }
}

static void ReadKeyFramer (int indent, dword p)
{
  word id;
  dword len, pc;
  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadKeyFramer", indent, id, len);
    switch ((int)id)
    {
      case CHUNK_VPLAYOUT: ReadViewSection (indent+2, pc+len); break;
      case CHUNK_OBJDES: ReadObjDesSection (indent+2, pc+len); break;
      default: dsetpos(pc+len);
    }
  }
}

static void ReadTriMeshBlocks (int indent, dword p, char * name)
{
  word id;
  dword len, pc;
  H3dsMeshObj * meshobj=GetMeshObj();
  strcpy(meshobj->name, name);
  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadTriMeshBlocks", indent, id, len);
    switch ((int)id)
    {
      case CHUNK_VERTLIST: ReadVertList (pc+len, meshobj); break;
      case CHUNK_FACELIST: ReadFaceList (pc+len, meshobj); break;
      case CHUNK_MAPLIST:  ReadMapList  (pc+len, meshobj); break;
      case CHUNK_TRMATRIX: ReadTraMatrix(pc+len, meshobj); break;

      //case CHUNK_FACEMAT:
      //case CHUNK_SMOOLIST:
      default: dsetpos(pc+len);
    }
  }
}

static void ReadObjBlocks (int indent, dword p)
{
  word id;
  dword len, pc;
  char name[16];

  // The object name is the first item
  int n=0;
  do
  {
    dread (&name[n++], 1);
  } while (name[n-1]!='\0' && n<(int)sizeof(name));
  name[n-1]='\0';
  if (flags & FLAG_LIST && !(flags & FLAG_VERYVERBOSE))
    fprintf (stderr, "  Object: %d: '%s'\n", curmodel, name);

  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadObjBlocks", indent, id, len, name);
    switch ((int)id)
    {
      case CHUNK_TRIMESH:  ReadTriMeshBlocks(indent+2, pc+len, name); break;

      //case CHUNK_LIGHT:
      //case CHUNK_CAMERA:
      default: dsetpos(pc+len);
    }
  }
}

static void ReadObjMeshBlocks (int indent, dword p)
{
  word id;
  dword len, pc;

  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadObjMeshBlocks", indent, id, len);
    switch ((int)id)
    {
      case CHUNK_OBJBLOCK:
	if (!(flags & FLAG_MODEL) || curmodel == modelnum)
	  ReadObjBlocks (indent+2, pc+len);
	else
	  dsetpos (pc+len);
	curmodel++;
	break;

      //case CHUNK_AMBCOLOR:
      //case CHUNK_BKGCOLOR:
      default: dsetpos(pc+len);
    }
  }
}

static void ReadMainBlocks (int indent, dword p)
{
  word id;
  dword len, pc;

  while ((pc=dgetpos()) < p)
  {
    dread (&id, sizeof(id));
    id = convert_endian2 (id);
    dread (&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("ReadMainBlocks", indent, id, len);
    switch ((int)id)
    {
      case CHUNK_OBJMESH: ReadObjMeshBlocks (indent+2, pc+len); break;
      case CHUNK_KEYFRAMER: ReadKeyFramer (indent+2, pc+len); break;
      //case CHUNK_MATERIAL:
      default: dsetpos(pc+len);
    }
  }
}

void HFree3dsScene (H3dsScene * scene)
{
  if (scene)
  {
    if (scene->meshobjlist)
    {
	  int n;
      for (n=scene->meshobjs-1; n>=0; n--)
      {
        H3dsMeshObj * mobj = &scene->meshobjlist[n];
        if(mobj->maplist)  free(mobj->maplist);
        if(mobj->facelist) free(mobj->facelist);
        if(mobj->vertlist) free(mobj->vertlist);
      }
      free (scene->meshobjlist);
    }
    free (scene);
  }
}

H3dsScene * HRead3dsScene (void * ptr, int what, dword /*size*/)
{
  if (ptr==0) return 0;

  if (what==0)
  {
    // Load from file
    InFile=(FILE *) ptr;
    dread=FileRead;
    dsetpos=FileSetpos;
    dgetpos=FileGetpos;
  }
  else
  {
    return 0;
  }

  if ((Scene=(H3dsScene *) malloc(sizeof(H3dsScene)))==0)
    return 0;
  memset (Scene, 0, sizeof(H3dsScene));

  int retval = setjmp(EnvState);

  if (retval==0)
  {
    // Return address set, start loading 3DS data.
    word id;
    dword len, pc;
    pc=dgetpos();
    dread(&id, sizeof(id));
    id = convert_endian2 (id);
    dread(&len, sizeof(len));
    len = convert_endian2 (len);
    VerbosePrintChunk ("HRead3dsScene", 0, id, len);
    if ((int)id!=CHUNK_MAIN)
    {
      HFree3dsScene (Scene);
      //printf("Not 3ds data\n");
      return 0;
    }
    ReadMainBlocks (2, pc+len);
  }
  else
  {
    // There was an error, free the allocated mem and return NULL.
    HFree3dsScene (Scene);
    return 0;
  }

  return Scene;
}
