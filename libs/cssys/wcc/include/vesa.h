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

int   VBE_Detect(char *str_ver);
short VESA_ScanMode(int x,int y,int Depth,HiColor *Color);
short VESA_InitGraphics(int x, int y,int Depth);
void  VESA_EndGraphics();
void  VESA_ScreenCopy(void *src);
void  AvailableModes(void);

#ifdef __cplusplus
}
#endif

#endif
