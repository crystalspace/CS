#ifndef __VESA_H
#define __VESA_H

typedef struct
         {
          char    RedMaskSize;            /* Size of direct color red mask    */
          char    RedFieldPosition;       /* Bit posn of lsb of red mask      */

          char    GreenMaskSize;          /* Size of direct color green mask  */
          char    GreenFieldPosition;     /* Bit posn of lsb of green mask    */

          char    BlueMaskSize;           /* Size of direct color blue mask   */
          char    BlueFieldPosition;      /* Bit posn of lsb of blue mask     */
         } HiColor;

#ifdef __cplusplus
extern "C" {
#endif

short GetGraphMode(int *X,int *Y,int *Depth, HiColor *Color);
short SetGraphMode(int X,int Y,int Depth,unsigned char *Memory);
void  RestoreVMode(void);
void  CopyShadowToScreen(unsigned char *Memory);
void  SetPalette(int i,int r,int g,int b);
void  UpdateScreen(long X1, long Y1, long X2, long Y2, void * src);

#ifdef __cplusplus
}
#endif

#endif
