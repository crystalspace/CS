/*
 * mpg123 defines 
 * used source: musicout.h from mpegaudio package
 */

#ifndef _CS_MPGFRAME_H_
#define _CS_MPGFRAME_H_

#include "bitstrm.h"

#define CSMPEG_BUFFERS 1
#define CSMPEG_RESYNC  2

#define HDRCMPMASK 0xfffffd00

#define AUDIO_FORMAT_UNSIGNED_8  0x1
#define AUDIO_FORMAT_SIGNED_8    0x2
#define AUDIO_FORMAT_ULAW_8      0x4
#define AUDIO_FORMAT_ALAW_8      0x8
#define AUDIO_FORMAT_SIGNED_16   0x110
#define AUDIO_FORMAT_UNSIGNED_16 0x120

class csMPGFrame;

typedef int (*synthFunc)(real *,int,unsigned char *,int *);
typedef int (*synthFuncMono)(real *,unsigned char *,int *);
typedef int (*layerFunc)(csMPGFrame *frame);

extern int do_layer1 (csMPGFrame *fr);
extern int do_layer2 (csMPGFrame *fr);
extern int do_layer3 (csMPGFrame *fr);
extern void init_layer3 (int);
extern void init_layer2 ();
extern void make_decode_tables (long);
extern void make_conv16to8_table (int);

struct ioCallback
{
  size_t (*read)  (void *ptr, size_t size, void *datasource);
  int    (*seek)  (int offset, int whence, void *datasource);
  int    (*close) (void *datasource);
  long   (*tell)  (void *datasource);
};

typedef struct {
  char tag[3];
  char title[30];
  char artist[30];
  char album[30];
  char year[4];
  char comment[30];
  unsigned char genre;
} csTagID3;


class csPCMBuffer
{
  int size;
 public:
  unsigned char *buffer;
  int pos;

  csPCMBuffer ();
  ~csPCMBuffer ();

  bool Resize (int newsize);
  int GetSize (int size) {return size;}
};


class csMPGFrame 
{
 protected:
  uint32 oldhead, firsthead;
  csPCMBuffer *pcm;
  int flags;
  int outformat;
  ioCallback *io;
  void *datasource;
  int channels;
  int bits;
  int frequency;

  static int tabsel_123[2][3][16];
  static long freqs[9];
  static bool layer_n_table_done;

  static char *mpeg_modes[4];
  static char *mpeg_layers[4];
  static char *mpeg_types[2];

  void Init ();
  bool HeadValid (uint32 head);
  bool ReadHead (uint32 &h);
  bool ReadHeadShift (uint32 &h);
  bool ReadFrameBody ();
  bool BackSkipFrame (int num);
  bool DecodeHeader (uint32 newhead);
  void PrintRemoteHeader ();
  void PrintHeaderCompact ();
  void PrintID3 (csTagID3 *buf);
  void SelectSynth ();

  double bpf ();
  double tpf ();
  long compute_buffer_offset ();
  void Resync ();
  
 public:

  csMPGFrame (void *datasource, ioCallback *io, int outformat, int flags, long usebuffer=0);

  ~csMPGFrame ();

  synthFunc synth;
  synthFuncMono synthMono;
  layerFunc layer;

  csBitstreamInfo bsi;
  al_table *alloc;
  int junk;

  int stereo;
  int jsbound;
  int single;
  int II_sblimit;
  int down_sample_sblimit;
  int lsf;
  int mpeg25;
  int down_sample;
  int header_change;
  int layer_num;

  int error_protection;
  int bitrate_index;
  int sampling_frequency;
  int padding;
  int extension;
  int mode;
  int mode_ext;
  int copyright;
  int original;
  int emphasis;
  int framesize; /* computed framesize */

  csPCMBuffer *GetPCMBuffer () {return pcm;}

  bool Read ();
  int Decode ();
  bool Initialize (int bits, int frequency, int channels, int down_sample);
  void Rewind ();

  void PrintHeader ();

};

#endif
