#define MaxXMode 16

//~~~~~~~~~~~~~~~~~~~~~~~ VGA register's ~~~~~~~~~~~~~~~~~~~~~~~~//

//^^^^^^^^^^^^^^^^^^^^^^^^ General Regs ^^^^^^^^^^^^^^^^^^^^^^^^^//
#define MISC_OUTPUT       0x3c2
#define MISC_OUTPUT_READ  0x3cc
#define FCW               0x3da
#define FCR               0x3ca
#define INPUT_STATUS_0    0x3c2
#define INPUT_STATUS_1    0x3da //;reg of mode: VSYNC is activ

//""""""""""""""""""""""" Sequencer Regs """"""""""""""""""""""""//
#define SC_INDEX          0x3c4
#define SC_DATA           0x3c5
#define CPWER             2
#define MAP_MASK          02

//'''''''''''''''''''''''''' CRTC Regs ``````````````````````````//
#define CRTC_INDEX        0x3d4
#define CRTC_DATA         0x3d5

//------------------------ Graphics Regs ------------------------//
#define GC_INDEX          0x3ce
#define GC_DATA           0x3cf
#define READ_MAP            04

//~~~~~~~~~~~~~~~~~~~~~~~ Attribute Regs ~~~~~~~~~~~~~~~~~~~~~~~~//
#define ACW_FF            0x3c0
#define ACR_FF            0x3c1

//######################### Color Regs ##########################//
#define PEL_MASK          0x3c6
#define PEL_GET           0x3c7
#define PEL_SET           0x3c8
#define PEL_DATA          0x3c9

#define START_ADDRESS_HIGH 0xc   ;bitmap start address high byte
#define START_ADDRESS_LOW  0xd   ;bitmap start address low byte

typedef struct
         {
          char    RedMaskSize;            /* Size of direct color red mask    */
          char    RedFieldPosition;       /* Bit posn of lsb of red mask      */

          char    GreenMaskSize;          /* Size of direct color green mask  */
          char    GreenFieldPosition;     /* Bit posn of lsb of green mask    */

          char    BlueMaskSize;           /* Size of direct color blue mask   */
          char    BlueFieldPosition;      /* Bit posn of lsb of blue mask     */
         } HiColor;

short GetGraphMode(int *X,int *Y,int *Depth, HiColor *Color);
short SetGraphMode(int X,int Y,int Depth,unsigned char *Memory);
short SetGraphSMode(int X,int Y,int Depth,unsigned char *Memory);
short SetGraphXMode(int X,int Y,unsigned char *Memory);
short vgaDetect(void);
void  RestoreVMode(void);
void  RestoreScreen(void);

void  WaitVrt(int loop);
void  RecalcWidthAdressTable(int X,int Y,unsigned char *Memory);
void  CopyShadowToScreen(unsigned char *Memory);
void  CopyShadowHorRasterToCurVPage(unsigned char *Memory);
void  CopyShadowHorRasterToCurVPage_NoFlip(unsigned char *Memory);
void  SwitchVideoPage(void);
int   GetNearXMode(int *X,int *Y);

void  SetPalette(int i,int r,int g,int b);
void  SetDotHorRaster(register int XPos,register int YPos,int Color);
void  MoveColumn2HorRaster(register int XBeg,register int YBeg,register int Hiest);
void  MoveString2HorRaster(register int XBeg,register int YBeg,register int Lenght);
short TestVMode(int X,int Y,unsigned char *Memory);

extern int   VBE_Detect(char *str_ver);
extern short VESA_ScanMode(int x,int y,int Depth,HiColor *Color);
extern short VESA_InitGraphics(int x, int y,int Depth);
extern void  VESA_EndGraphics();
extern void  VESA_ScreenCopy(void *src);
extern void  AvailableModes(void);
extern void  VESA_UpdateScreen(long X1, long Y1, long X2, long Y2, void * src);
