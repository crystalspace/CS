/*
 * mpg123 defines 
 * used source: musicout.h from mpegaudio package
 */

#ifndef _MPG123_H_
#define _MPG123_H_

#define SKIP_JUNK 1
#define NOXFERMEM 1
#define real float

#ifdef _WIN32	/* Win32 Additions By Tony Million */
# undef WIN32
# define WIN32

#pragma warning(disable:4244)   // conversion from 'double' to 'float'
#pragma warning(disable:4514)   // Removal of unreferenced inline function


# define NEW_DCT9

# undef MPG123_REMOTE           /* Get rid of this stuff for Win32 */
#endif

#ifndef M_PI
# define M_PI       3.14159265358979323846
#endif
# define M_SQRT2	1.41421356237309504880

#define byte unsigned char

#ifndef WIN32
  #include "xfermem.h"
#endif

#define         SBLIMIT                 32
#define         SCALE_BLOCK             12
#define         SSLIMIT                 18

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

#define MAXOUTBURST 32768

/* Pre Shift fo 16 to 8 bit converter table */
#define AUSHIFT (3)


struct al_table 
{
  short bits;
  short d;
};

#ifndef WIN32
  extern txfermem *buffermem;
#endif

#ifndef NOXFERMEM
extern void buffer_loop(struct audio_info_struct *ai,sigset_t *oldsigset);
#endif

/* ------ Declarations from "common.c" ------ */

extern void (*catchsignal(int signum, void(*handler)()))();

#ifdef VARMODESUPPORT
extern int varmode;
extern int playlimit;
#endif

struct gr_info_s {
      int scfsi;
      unsigned part2_3_length;
      unsigned big_values;
      unsigned scalefac_compress;
      unsigned block_type;
      unsigned mixed_block_flag;
      unsigned table_select[3];
      unsigned subblock_gain[3];
      unsigned maxband[3];
      unsigned maxbandl;
      unsigned maxb;
      unsigned region1start;
      unsigned region2start;
      unsigned preflag;
      unsigned scalefac_scale;
      unsigned count1table_select;
      real *full_gain[3];
      real *pow2gain;
};

struct III_sideinfo
{
  unsigned main_data_begin;
  unsigned private_bits;
  struct {
    struct gr_info_s gr[2];
  } ch[2];
};

#ifdef PENTIUM_OPT
extern int synth_1to1_pent (real *,int,unsigned char *);
#endif
#ifdef USE_3DNOW
extern int synth_1to1_3dnow (real *,int,unsigned char *);
#endif
extern int synth_1to1 (real *,int,unsigned char *,int *);
extern int synth_1to1_8bit (real *,int,unsigned char *,int *);
extern int synth_1to1_mono (real *,unsigned char *,int *);
extern int synth_1to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_1to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_1to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_2to1 (real *,int,unsigned char *,int *);
extern int synth_2to1_8bit (real *,int,unsigned char *,int *);
extern int synth_2to1_mono (real *,unsigned char *,int *);
extern int synth_2to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_2to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_4to1 (real *,int,unsigned char *,int *);
extern int synth_4to1_8bit (real *,int,unsigned char *,int *);
extern int synth_4to1_mono (real *,unsigned char *,int *);
extern int synth_4to1_mono2stereo (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono (real *,unsigned char *,int *);
extern int synth_4to1_8bit_mono2stereo (real *,unsigned char *,int *);

extern int synth_ntom (real *,int,unsigned char *,int *);
extern int synth_ntom_8bit (real *,int,unsigned char *,int *);
extern int synth_ntom_mono (real *,unsigned char *,int *);
extern int synth_ntom_mono2stereo (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono (real *,unsigned char *,int *);
extern int synth_ntom_8bit_mono2stereo (real *,unsigned char *,int *);

extern void rewindNbits(int bits);
extern int  hsstell(void);

extern void huffman_decoder(int ,int *);
extern void huffman_count1(int,int *);

extern void dct64(real *,real *,real *);

extern void synth_ntom_set_step(long,long);

extern unsigned char *conv16to8;
extern real muls[27][64];
#ifdef USE_3DNOW
extern real decwin[2*(512+32)];
#else
extern real decwin[512+32];
#endif
extern real *pnts[5];

/* 486 optimizations */
#define FIR_BUFFER_SIZE  128
extern void dct64_486(int *a,int *b,real *c);
extern int synth_1to1_486(real *bandPtr,int channel,unsigned char *out,int nb_blocks);

#endif
