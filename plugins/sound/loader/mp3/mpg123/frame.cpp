#include "cssysdef.h"

#include "mpg123.h"
#include "genre.h"
#include "frame.h"

csPCMBuffer::csPCMBuffer ()
{
  pos = 0;
  size = 0;
  buffer = NULL;
  Resize (8192*2 + 1024);
}

csPCMBuffer::~csPCMBuffer ()
{
  free (buffer);
}

bool csPCMBuffer::Resize (int newsize)
{
  buffer = (unsigned char*)realloc (buffer, newsize);
  if (pos > newsize)
    pos = newsize;
  size = newsize;
  return true;
}

int csMPGFrame::tabsel_123[2][3][16] = 
  {
    { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
      {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
      {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} 
    },
    { {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,} 
    }
  };

long csMPGFrame::freqs[9] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 , 12000 , 8000 };
char *csMPGFrame::mpeg_modes[4] = { "Stereo", "Joint-Stereo", "Dual-Channel", "Single-Channel" };
char *csMPGFrame::mpeg_layers[4] = { "Unknown" , "I", "II", "III" };
char *csMPGFrame::mpeg_types[2] = { "1.0" , "2.0" };


#ifdef VARMODESUPPORT
/*
 *   This is a dirty hack!  It might burn your PC and kill your cat!
 *   When in "varmode", specially formatted layer-3 mpeg files are
 *   expected as input -- it will NOT work with standard mpeg files.
 *   The reason for this:
 *   Varmode mpeg files enable my own GUI player to perform fast
 *   forward and backward functions, and to jump to an arbitrary
 *   timestamp position within the file.  This would be possible
 *   with standard mpeg files, too, but it would be a lot harder to
 *   implement.
 *   A filter for converting standard mpeg to varmode mpeg is
 *   available on request, but it's really not useful on its own.
 *
 *   Oliver Fromme  <oliver.fromme@heim3.tu-clausthal.de>
 *   Mon Mar 24 00:04:24 MET 1997
 */
int varmode = FALSE;
int playlimit;
#endif

#define CSMPEG_OUTSCALE 32768

bool csMPGFrame::layer_n_table_done = false;

csMPGFrame::csMPGFrame (void *datasource, ioCallback *io, int outformat, int flags, long usebuffer)
{
  synth = NULL;
  synthMono = NULL;
  layer = NULL;

  (void)usebuffer;
  this->io = io;
  this->datasource = datasource;
  this->flags = flags;
  Init ();
  pcm = new csPCMBuffer ();

  channels = -1;
  down_sample = 0;
  bits = 16;

  if (!layer_n_table_done)
  {
    make_decode_tables(CSMPEG_OUTSCALE);
    init_layer2(); /* inits also shared tables with layer1 */
    layer_n_table_done = true;
  }
  this->outformat = outformat;
}

csMPGFrame::~csMPGFrame ()
{
  delete pcm;
  if (io) io->close (datasource);
}

void csMPGFrame::Init ()
{
  oldhead = firsthead = 0;
  bsi.Init ();
}

void csMPGFrame::Rewind ()
{
  io->seek (0, SEEK_SET, datasource);
  Init ();
  pcm->pos = 0;
}

int csMPGFrame::Decode ()
{
  if (channels == -1)
    Initialize (bits, sampling_frequency, 2, down_sample);
  else if(header_change > 1)
    Initialize (bits, sampling_frequency, channels, down_sample);

  if (error_protection)
    bsi.GetBits (16); // skip crc
  return layer(this);
}

bool csMPGFrame::HeadValid (uint32 head)
{
  if ((head & 0xffe00000) != 0xffe00000)
    return false;
  if (!((head>>17)&3))
    return false;
  if (((head>>10)&0x3f) == 0x3f)
    return false;
  if ((head & 0xffff0000) == 0xfffe0000)
    return false;

  return true;
}

/*****************************************************************
 * read next frame
 */

bool csMPGFrame::ReadHead (uint32 &h)
{
  if (io->read (&h,4, datasource) != 4)
    return false;

#ifdef CS_LITTLE_ENDIAN
  h = (h >> 24) | ((h >> 8) & 0xff00) | ((h << 8) & 0xff0000) | (h << 24);
#endif

  return true;
}

bool csMPGFrame::ReadHeadShift (uint32 &h)
{
  h <<= 8;

#ifdef CS_LITTLE_ENDIAN
  if (io->read (&h,1, datasource) != 1)
    return false;
#else
  if (io->read (((unsigned char*)&h)+3,1, datasource) != 1)
    return false;
#endif

  return true;
}

bool csMPGFrame::ReadFrameBody ()
{
  size_t l;
  if ((l=io->read (bsi.bsbuf, framesize, datasource)) != framesize)
  {
    if (l <= 0)
      return false;
    memset (bsi.bsbuf+l, 0, framesize-l);
  }
  return true;
}
  
bool csMPGFrame::Read ()
{
  uint32 newhead;

  bsi.FrameSize (framesize);       /* for Layer3 */

read_again:
  if(!ReadHead(newhead))
    return false;

  if(1 || oldhead != newhead || !oldhead)
  {

init_resync:

    header_change = 2;
    if (oldhead) 
    {
      if ((oldhead & 0xc00) == (newhead & 0xc00)) 
      {
        if ((oldhead & 0xc0) == 0 && (newhead & 0xc0) == 0)
    	  header_change = 1; 
        else if ((oldhead & 0xc0) && (newhead & 0xc0))
	  header_change = 1;
      }
    }


    if (!firsthead && !HeadValid (newhead) ) 
    {
      int i;

      /* I even saw RIFF headers at the beginning of MPEG streams ;( */
      if(newhead == ('R'<<24)+('I'<<16)+('F'<<8)+'F') 
      {
	if (!ReadHead (newhead))
	  return false;
	while (newhead != ('d'<<24)+('a'<<16)+('t'<<8)+'a') 
	{
	  if (!ReadHeadShift (newhead))
	    return false;
	}
	if (!ReadHead (newhead))
	  return false;
	goto read_again;
      }

      {
	/* step in byte steps through next 64K */
	for (i=0; i<65536; i++) 
	{
	  if (!ReadHeadShift (newhead))
	    return false;
	  if (HeadValid(newhead))
	    break;
	}
	if (i == 65536) 
	{
	  fprintf(stderr,"Giving up searching valid MPEG header\n");
	  return false;
	}
      }

      /* 
       * should we additionaly check, whether a new frame starts at
       * the next expected position? (some kind of read ahead)
       * We could implement this easily, at least for files.
       */
    }

    if ((newhead & 0xffe00000) != 0xffe00000) 
    {
      /* and those ugly ID3 tags */
      if ((newhead & 0xffffff00) == ('T'<<24)+('A'<<16)+('G'<<8)) 
      {
	if (io->seek (124, SEEK_CUR, datasource) == 0)
	{
	  fprintf (stderr,"Skipped ID3 Tag!\n");
	  goto read_again;
	}
	fprintf (stderr,"could not Skip ID3 Tag!\n");
      }

      if (flags & CSMPEG_RESYNC) 
      {
        int ntry = 0;
	/* Read more bytes until we find something that looks
	   reasonably like a valid header.  This is not a
	   perfect strategy, but it should get us back on the
	   track within a short time (and hopefully without
	   too much distortion in the audio output).  
	*/
        do 
	{
          ntry++;
          if (!ReadHeadShift (newhead))
	    return false;
          if (!oldhead)
            goto init_resync;       /* "considered harmful", eh? */

        } while ((newhead & HDRCMPMASK) != (oldhead & HDRCMPMASK)
              && (newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK));
      }
      else
        return false;
    }

    if (!firsthead) 
    {
      if (!DecodeHeader (newhead))
        goto read_again;
      firsthead = newhead;
    }
    else
      if (!DecodeHeader (newhead))
        return false;
  }
  else
    header_change = 0;

  /* flip/init buffer for Layer 3 */
  bsi.FlipBuffer ();

  /* read main data into memory */
  if (!ReadFrameBody ())
    return false;

  bsi.Rewind ();

  return true;
}

/****************************************
 * HACK,HACK,HACK: step back <num> frames
 * can only work if the 'stream' isn't a real stream but a file
 */

bool csMPGFrame::BackSkipFrame (int num)
{
  long bytes;
  uint32  newhead;
  
  if (!firsthead)
    return true;
  
  bytes = (framesize+8)*(num+2);
  
  if (io->seek (-bytes, SEEK_CUR, datasource) < 0)
    return false;

  if (flags & CSMPEG_BUFFERS)
    Resync ();

  if (!ReadHead (newhead))
    return false;
  
  while ((newhead & HDRCMPMASK) != (firsthead & HDRCMPMASK)) 
  {
    if (!ReadHeadShift (newhead))
      return false;
  }
  
  if (io->seek (-4, SEEK_CUR, datasource) <0)
    return false;

  if (flags & CSMPEG_BUFFERS)
    Resync ();

  Read ();
  Read ();
  
  if (layer_num == 3)
    bsi.SetPointer (512);
  
  return 0;
}


/*
 * decode a header and write the information
 * into the frame structure
 */
bool csMPGFrame::DecodeHeader (uint32 newhead)
{
  int skipsize;

  if (!HeadValid (newhead))
    return false;

  if (newhead & (1<<20)) 
  {
    lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
    mpeg25 = 0;
  }
  else 
  {
    lsf = 1;
    mpeg25 = 1;
  }
    
  if (!(flags & CSMPEG_RESYNC) || !oldhead) 
  {
    /* If "tryresync" is true, assume that certain
       parameters do not change within the stream! */

    layer_num = 4-((newhead>>17)&3);
    if (((newhead>>10)&0x3) == 0x3) 
    {
      fprintf(stderr,"Stream error\n");
      return false;
    }

    if (mpeg25)
      sampling_frequency = 6 + ((newhead>>10)&0x3);
    else
      sampling_frequency = ((newhead>>10)&0x3) + (lsf*3);

    error_protection = ((newhead>>16)&0x1)^0x1;
  }

  bitrate_index = ((newhead>>12)&0xf);
  padding       = ((newhead>>9)&0x1);
  extension     = ((newhead>>8)&0x1);
  mode          = ((newhead>>6)&0x3);
  mode_ext      = ((newhead>>4)&0x3);
  copyright     = ((newhead>>3)&0x1);
  original      = ((newhead>>2)&0x1);
  emphasis      = newhead & 0x3;

  stereo    = mode == MPG_MD_MONO ? 1 : 2;

  oldhead = newhead;

  if(!bitrate_index) 
  {
    fprintf (stderr,"Free format not supported: (head %08lx)\n",newhead);
    return false;
  }

  switch (layer_num) 
  {
  case 1:
    layer = do_layer1;
#ifdef VARMODESUPPORT
    if (varmode) 
    {
      fprintf (stderr,"Sorry, layer-1 not supported in varmode.\n"); 
      return false;
    }
#endif
    framesize  = (long) tabsel_123[lsf][0][bitrate_index] * 12000;
    framesize /= freqs[sampling_frequency];
    framesize  = ((framesize+padding)<<2)-4;
    break;
  case 2:
    layer = do_layer2;
#ifdef VARMODESUPPORT
    if (varmode) 
    {
      fprintf (stderr,"Sorry, layer-2 not supported in varmode.\n"); 
      return false;
    }
#endif
    framesize = (long) tabsel_123[lsf][1][bitrate_index] * 144000;
    framesize /= freqs[sampling_frequency];
    framesize += padding - 4;
    break;
  case 3:
    layer = do_layer3;
    if (lsf)
      skipsize = ((stereo == 1) ? 9 : 17);
    else
      skipsize =  ((stereo == 1) ? 17 : 32);
    if (error_protection)
      skipsize += 2;
    bsi.SkipSize (skipsize);
    framesize  = (long) tabsel_123[lsf][2][bitrate_index] * 144000;
    framesize /= freqs[sampling_frequency]<<lsf;
    framesize = framesize + padding - 4;
    break; 
  default:
    fprintf (stderr, "Sorry, unknown layer type.\n"); 
    return false;
  }

  return true;
}

void csMPGFrame::Resync ()
{
  // resync buffers
}

void csMPGFrame::PrintRemoteHeader ()
{
  /* version, layer, freq, mode, channels, bitrate, BPF */
  fprintf (stderr,"@I %s %s %ld %s %d %d %d\n",
	   mpeg_types[lsf], 
	   mpeg_layers[layer_num],
	   freqs[sampling_frequency],
	   mpeg_modes[mode],
	   stereo,
	   tabsel_123[lsf][layer_num-1][bitrate_index],
	   framesize+4
	  );
}

void csMPGFrame::PrintHeader ()
{
  fprintf (stderr,"MPEG %s, Layer: %s, Freq: %ld, mode: %s, modext: %d, BPF : %d\n", 
	   mpeg25 ? "2.5" : (lsf ? "2.0" : "1.0"),
	   mpeg_layers[layer_num],
	   freqs[sampling_frequency],
	   mpeg_modes[mode],
	   mode_ext,
	   framesize+4
	   );
  fprintf (stderr,"Channels: %d, copyright: %s, original: %s, CRC: %s, emphasis: %d.\n",
	   stereo,
	   copyright?"Yes":"No",
	   original?"Yes":"No",
	   error_protection?"Yes":"No",
	   emphasis
	   );
  fprintf (stderr,"Bitrate: %d Kbits/s, Extension value: %d\n",
	   tabsel_123[lsf][layer_num-1][bitrate_index],
	   extension
	   );
}

void csMPGFrame::PrintHeaderCompact ()
{
  fprintf (stderr,"MPEG %s layer %s, %d kbit/s, %ld Hz %s\n",
	   mpeg25 ? "2.5" : (lsf ? "2.0" : "1.0"),
	   mpeg_layers[layer_num],
	   tabsel_123[lsf][layer_num-1][bitrate_index],
	   freqs[sampling_frequency], 
	   mpeg_modes[mode]
	   );
}

void csMPGFrame::PrintID3 (csTagID3 *buf)
{
  char title[31]={0,};
  char artist[31]={0,};
  char album[31]={0,};
  char year[5]={0,};
  char comment[31]={0,};
  char genre[31]={0,};

  strncpy (title,   buf->title,30);
  strncpy (artist,  buf->artist,30);
  strncpy (album,   buf->album,30);
  strncpy (year,    buf->year,4);
  strncpy (comment, buf->comment,30);

  if (buf->genre <= sizeof (genre_table)/sizeof(*genre_table)) 
  {
    strncpy (genre, genre_table[buf->genre], 30);
  } 
  else 
  {
    strncpy (genre, "Unknown",30);
  }
	
  fprintf (stderr, "Title  : %-30s  Artist: %s\n",  title,   artist);
  fprintf (stderr, "Album  : %-30s  Year  : %4s\n", album,   year);
  fprintf (stderr, "Comment: %-30s  Genre : %s\n",  comment, genre);
}

/********************************/

double csMPGFrame::bpf ()
{
  double bpf;

  switch (layer_num)
  {
  case 1:
    bpf = tabsel_123[lsf][0][bitrate_index];
    bpf *= 12000.0 * 4.0;
    bpf /= freqs[sampling_frequency] <<(lsf);
    break;
  case 2:
  case 3:
    bpf = tabsel_123[lsf][layer_num-1][bitrate_index];
    bpf *= 144000;
    bpf /= freqs[sampling_frequency] << (lsf);
    break;
  default:
    bpf = 1.0;
  }
  
  return bpf;
}

double csMPGFrame::tpf ()
{
  int bs[4] = { 0,384,1152,1152 };
  double tpf;
  
  tpf = (double) bs[layer_num];
  tpf /= freqs[sampling_frequency] << (lsf);
  return tpf;
}

/*
 * Returns number of frames queued up in output buffer, i.e. 
 * offset between currently played and currently decoded frame.
 */

long csMPGFrame::compute_buffer_offset ()
{
  return 0;
#ifdef NOPE_IGNORE_ME_GO_AWAY
  long bufsize;
	
  /*
   * buffermem->buf[0] holds output sampling rate,
   * buffermem->buf[1] holds number of channels,
   * buffermem->buf[2] holds audio format of output.
   */
	
  if (!(flags & CSMPEG_BUFEFRS) || !(bufsize=xfermem_get_usedspace(buffermem))
     || !buffermem->buf[0] || !buffermem->buf[1])
    return 0;

  bufsize = (long)((double) bufsize / buffermem->buf[0] / 
		   buffermem->buf[1] / tpf ());
  
  if((buffermem->buf[2] & AUDIO_FORMAT_MASK) == AUDIO_FORMAT_16)
    return bufsize/2;
  else
    return bufsize;
#endif
}

bool csMPGFrame::Initialize (int bits, int frequency, int channels, int down_sample)
{
  this->down_sample = down_sample;

  switch (down_sample) 
  {
  case 0:
  case 1:
  case 2:
    down_sample_sblimit = SBLIMIT>>down_sample;
    break;
  case 3:
    {
      long n = freqs[sampling_frequency];
      long m = frequency;

      synth_ntom_set_step(n,m);

      if(n>m) 
      {
	down_sample_sblimit = SBLIMIT * m;
	down_sample_sblimit /= n;
      }
      else
	down_sample_sblimit = SBLIMIT;
    }
    break;
  }

  if (this->frequency != frequency || this->channels != channels || this->bits != bits)
  {
    if (channels == 1)
      single = 3;
    else
      single = -1;

    this->frequency = frequency;
    this->channels = channels;
    this->bits = bits;

    SelectSynth ();
    init_layer3 (down_sample_sblimit);
  }

  return true;
}

void csMPGFrame::SelectSynth ()
{
  int p8=0;

  synthFunc funcs[2][4] = { 
    { synth_1to1,
      synth_2to1,
      synth_4to1,
      synth_ntom } ,
    { synth_1to1_8bit,
      synth_2to1_8bit,
      synth_4to1_8bit,
      synth_ntom_8bit } 
  };

  synthFuncMono funcs_mono[2][2][4] = {    
    { { synth_1to1_mono2stereo ,
	synth_2to1_mono2stereo ,
	synth_4to1_mono2stereo ,
	synth_ntom_mono2stereo } ,
      { synth_1to1_8bit_mono2stereo ,
	synth_2to1_8bit_mono2stereo ,
	synth_4to1_8bit_mono2stereo ,
	synth_ntom_8bit_mono2stereo } } ,
    { { synth_1to1_mono ,
	synth_2to1_mono ,
	synth_4to1_mono ,
	synth_ntom_mono } ,
      { synth_1to1_8bit_mono ,
	synth_2to1_8bit_mono ,
	synth_4to1_8bit_mono ,
	synth_ntom_8bit_mono } }
  };

  if (bits==8)
    p8 = 1;

  synth = funcs[p8][down_sample];
  synthMono = funcs_mono [channels == 2?0:1][p8][down_sample];
  
  if (p8)
    make_conv16to8_table (outformat);
}

