/*
  DOS support for Crystal Space video library
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Slavik Levtchenko <Smirnov@bbs.math.spbu.ru>

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

#include "global.h"
#include "scrctrl.h"
#include "global.c"

short GetGraphMode(int *X,int *Y,int *Depth, HiColor *Color)
{
  int VBE_ver;

  if(VBE_ver =VBE_Detect("VBE2"))
   {
//    AvailableModes(); //debug info
     VBE_DetectFlag =2;
     if(VESA_ScanMode(*X,*Y,*Depth,Color)) return 0; //VBE_detect() need call untill call SetGraphSMode !!!
   }
   else
     if(VBE_ver =VBE_Detect("VESA"))
      {
//       AvailableModes(); //debug info
        VBE_DetectFlag =1;
        if(VESA_ScanMode(*X,*Y,*Depth,Color)) return 0; //VBE_detect() need call untill call SetGraphSMode !!!
      }

  *Depth =8;
  VBE_DetectFlag =0;

  if(vgaDetect()) {GetNearXMode(X,Y); return 0;}
  return 1;
}

short SetGraphMode(int X,int Y,int Depth,unsigned char *Memory)
{
  if(VBE_DetectFlag) return SetGraphSMode(X,Y,Depth,Memory);
   else return SetGraphXMode(X,Y,Memory);
}

short SetGraphSMode(int X,int Y,int Depth,unsigned char *Memory)
{
  //Set global graphical variable
  FlagFlipVMode =0;
  XModeFlag =0;
  VesaFlag =1;

  ClipMinX =0;
  ClipMinY =0;
  ClipMaxX =X;
  ClipMaxY =Y;
  SizeScreenX =X;
  SizeScreenY =Y;
  RecalcWidthAdressTable(X,Y,Memory);

  return VESA_InitGraphics(X,Y,Depth);
}

void RestoreVMode(void)
{
  if(XModeFlag)
   {
    bout(SC_INDEX,1);
    bout(SC_DATA,(bin(SC_DATA)|0x20));
    WaitVrt(1);
    RestoreScreen();
   }

   if(VesaFlag) VESA_EndGraphics();
}

void RestoreScreen(void)
{
        char *ad = (char *)0x449;

        regs.w.ax = ad[0];
        int386(0x10,&regs,&regs);
}

short vgaDetect(void)
{
  union REGS   Regs;

  Regs.x.eax=0x1a00;
  int386(0x10, &Regs, &Regs);
  if((Regs.h.al==0x1a&&Regs.h.bl==7)||(Regs.h.al==0x1a&&Regs.h.bl==8)) return 1;

  return 0;
}

void WaitVrt(int loop)
{
  while(loop--)
   {
    //while((bin(INPUT_STATUS_1)&8)){};
    while(!(bin(INPUT_STATUS_1)&8)){};
   }
}

void RecalcWidthAdressTable(int X,int Y,unsigned char *Memory)
{
  int i;

  for(i=0;i<=Y;i++) WidthAdressTable[i] =(char *)((int)Memory +(i*X));
}

  int SizeXTable[MaxXMode][2]={{256,200},{256,240},{320,200},{320,240},{320,400},
                               {320,480},{360,200},{360,240},{360,360},{360,400},
                               {360,480},{376,282},{376,308},{376,564},{360,350},
                               {320,350}};

  int OffPageTable[MaxXMode] = {0x3200,0x3c00,0x3e80,0x4b00,0x7d00,0x0000,0x4650,
                                0x5460,0x7e90,0x0000,0x0000,0x678c,0x7118,0x0000,
                                0x7b0c,0x6d60};

void CopyShadowToScreen(unsigned char *Memory)
{
  if (XModeFlag)
   {
    if (!OffPageTable [numXmode])
      CopyShadowHorRasterToCurVPage_NoFlip(Memory);
    else
    {
      CopyShadowHorRasterToCurVPage(Memory);
      SwitchVideoPage ();
    }

    return;
   }

  if (VesaFlag)
    VESA_ScreenCopy(Memory);
}

void CopyShadowHorRasterToCurVPage(unsigned char *Memory)
{
  register int CurOffPage, CurOffShadow, i;

  CurOffShadow = (int)Memory+(ClipMinY*SizeScreenX);
  CurOffPage = 0xa0000+(ClipMinY*WidthPageFactor)+OffsetVPage;

  for(i=0;i<4;i++)
   {
    wout (SC_INDEX,(1<<(i+8))|CPWER);
    mover (CurOffPage,CurOffShadow,WidthPageFactor*(ClipMaxY-ClipMinY));
    CurOffShadow++;
   }
}

void CopyShadowHorRasterToCurVPage_NoFlip(unsigned char *Memory)
{
  register int CurOffPage, CurOffShadow, count_plane;
  register int Y1, Y2=SizeXTable[numXmode][1];

  CurOffPage = 0xa0000+(ClipMinY*WidthPageFactor)+OffsetVPage;

  for(Y1=0;Y1<Y2;Y1++)
   {
    CurOffShadow = (int)Memory+(Y1 * SizeXTable[numXmode][0]);

    for(count_plane=0;count_plane<4;count_plane++)
     {
      wout (SC_INDEX, (0x0100 << count_plane) | CPWER);
      mover (CurOffPage, CurOffShadow, WidthPageFactor);
      CurOffShadow++;
     }
    CurOffPage +=WidthPageFactor;
   }
}

void CopyXModeLine(long X1, long Y1, long offset, void * src, long ncopy)
{
  register int count_plane, plane =X1&3;

  offset = 0xa0000 +offset +OffsetVPage ^OffPageTable[numXmode];

  for(count_plane=0;count_plane<4;count_plane++)
   {
    wout(SC_INDEX,(1<<(plane+8))|CPWER);
    mover(offset,src,(ncopy-count_plane+3)>>2);
    src =(void*)((int)src +1);
    plane =(plane+1)&3;
   }
}

void XMode_UpdateScreen(long X1, long Y1, long X2, long Y2, void * src)
/****************************************************************************
 * Function:     XMode_UpdateScreen
 * Description:  Copies a block of memory to a location on the screen
 *               in X-mode
 ****************************************************************************/
{
  long ncopy =X2 -X1;
  long offset;

  for(Y1;Y1<=Y2;Y1++)
   {
    offset =Y1 * WidthPageFactor + (X1>>2);
    CopyXModeLine(X1, Y1, offset, src, ncopy);
    src =(void*)((int)src + SizeXTable[numXmode][0]); //Set on next line
   }
}

void UpdateScreen(long X1, long Y1, long X2, long Y2, void * src)
{
 if(VesaFlag) {VESA_UpdateScreen(X1, Y1, X2, Y2, src); return;}
 if(XModeFlag) {XMode_UpdateScreen(X1, Y1, X2, Y2, src); return;}
}

void SwitchVideoPage()
{
  if(numXmode<MaxXMode&&OffPageTable[numXmode])
   {
    wout(CRTC_INDEX,(OffsetVPage&0xff00)|0xc);
    wout(CRTC_INDEX,(OffsetVPage<<8)|0xd);
    OffsetVPage ^= OffPageTable[numXmode]; //XOR
   }
}

int GetNearXMode(int *X,int *Y)
{
  unsigned int i,BestX=-1,BestY=-1,BestDXY=-1,BestNum=1,CurDXY,RastrFactor;
           int CurFactor;

  if((*X-*Y)>0) RastrFactor =0;
   else RastrFactor=1;

  for(i=0;i<MaxXMode;i++)
   {
    CurFactor =SizeXTable[i][0]-SizeXTable[i][1];
    CurDXY =ABS(*X-SizeXTable[i][0])+ABS(*Y-SizeXTable[i][1]);

    if(CurDXY<BestDXY)
     {
      if(CurFactor>0 && RastrFactor==0)
       {
        BestDXY =CurDXY;
        BestX =SizeXTable[i][0];
        BestY =SizeXTable[i][1];
        BestNum =i;
       }
        else
         if(CurFactor<=0 && RastrFactor==1)
          {
           BestDXY =CurDXY;
           BestX =SizeXTable[i][0];
           BestY =SizeXTable[i][1];
           BestNum =i;
          }
     }
   }

   *X =BestX;
   *Y =BestY;
   return BestNum;
}

  char Sequencer[5] ={0x03,0x23,0x0f,0x00,0x06}; //For all modes
  char Graphics[9] = {0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0f,0xff}; //For all modes
  char Attrib[5] =   {0x41,0x00,0x0f,0x00,0x00}; //For all modes
                          // 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15
  char MORValue[MaxXMode] = {0xe3,0xe3,0x63,0xe3,0x63,0xe3,0x67,0xe7,0xa7,0x67,0xe7,0xe7,0xe7,0xe7,0x67,0x67};
  char CRTCValue[MaxXMode][25] ={
                          {0x5f,0x3f,0x42,0x9f,0x4c,0x00,0xcc,0x1f,
                           0x00,0x41,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xe3
                           0xa0,0x2c,0x8f,0x20,0x00,0x96,0xc6,0xe3,0xff},//256*200*256c
                          {0x5f,0x3f,0x42,0x9f,0x4c,0x00,0x0d,0x3e,
                           0x00,0x41,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xe3
                           0xea,0x2c,0xdf,0x20,0x00,0xe7,0x06,0xe3,0xff},//256*240*256c
                          {0x5f,0x4f,0x50,0x82,0x55,0x80,0xcc,0x1f,
                           0x00,0x41,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0x63
                           0xa0,0x2c,0x8f,0x28,0x00,0x96,0xc6,0xe3,0xff},//320*200*256c
                          {0x5f,0x4f,0x50,0x82,0x54,0x80,0x0d,0x3e,
                           0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xe3
                           0xea,0x2c,0xdf,0x28,0x00,0xe7,0x06,0xe3,0xff},//320*240*256c
                          {0x5f,0x4f,0x50,0x82,0x55,0x80,0xcc,0x1f,
                           0x00,0x40,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0x63
                           0xa0,0x2c,0x8f,0x28,0x00,0x96,0xc6,0xe3,0xff},//320*400*256c
                          {0x5f,0x4f,0x50,0x82,0x55,0x80,0x0d,0x3e,
                           0x00,0x40,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xe3
                           0xea,0x2c,0xdf,0x28,0x00,0xe7,0x06,0xe3,0xff},//320*480*256c
                          {0x6b,0x59,0x5a,0x8e,0x5e,0x8a,0xcc,0x1f,
                           0x00,0x41,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0x67
                           0xa0,0x2c,0x8f,0x2d,0x00,0x96,0xc6,0xe3,0xff},//360*200*256c
                          {0x6b,0x59,0x5a,0x8e,0x5e,0x8a,0x0d,0x3e,
                           0x00,0x41,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xe7
                           0xea,0x2c,0xdf,0x2d,0x00,0xe7,0x06,0xe3,0xff},//360*240*256c
                          {0x6b,0x59,0x5a,0x8e,0x5e,0x8a,0xcc,0x1f,
                           0x00,0x40,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0x67
                           0x88,0x05,0x67,0x2d,0x00,0x6d,0xba,0xe3,0xff},//360*360*256c
                          {0x6b,0x59,0x5a,0x8e,0x5e,0x8a,0xcc,0x1f,
                           0x00,0x40,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0x67
                           0xa0,0x2c,0x8f,0x2d,0x00,0x96,0xc6,0xe3,0xff},//360*400*256c
                          {0x6b,0x59,0x5a,0x8e,0x5e,0x8a,0x0d,0x3e,
                           0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xe7
                           0xea,0x2c,0xdf,0x2d,0x00,0xe7,0x06,0xe3,0xff},//360*480*256c
                          {0x6e,0x5d,0x5e,0x91,0x62,0x8f,0x62,0xf0,
                           0x00,0x61,0x1e,0x00,0x00,0x00,0x00,0x31,      //MOR = 0xe7
                           0x37,0x09,0x33,0x2f,0x00,0x3c,0x5c,0xe3,0xff},//376*282*256c
                          {0x6e,0x5d,0x5e,0x91,0x62,0x8f,0x62,0x0f,
                           0x00,0x40,0x1e,0x00,0x00,0x00,0x00,0x31,      //MOR = 0xe7
                           0x37,0x09,0x33,0x2f,0x00,0x3c,0x5c,0xe3,0xff},//376*308*256c
                          {0x6e,0x5d,0x5e,0x91,0x62,0x8f,0x62,0xf0,
                           0x00,0x60,0x1e,0x00,0x00,0x00,0x00,0x31,       //MOR = 0xe7
                           0x37,0x09,0x33,0x2f,0x00,0x3c,0x5c,0xe3,0xff},//376*564*256c
                          {0x6b,0x59,0x5a,0x8e,0x5e,0x8a,0xbf,0x1f,
                           0x00,0x40,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xA7
                           0x83,0x85,0x5d,0x2d,0x00,0x63,0xba,0xe3,0xff},//360*350*256c
                          {0x5f,0x4f,0x50,0x82,0x55,0x80,0xbf,0x1f,
                           0x00,0x41,0x1e,0x00,0x00,0x00,0x00,0x00,      //MOR = 0xA3
                           0x83,0x85,0x5d,0x28,0x00,0x63,0xba,0xe3,0xff}};//320*350*256c


short SetGraphXMode(int X,int Y,unsigned char *Memory)
{
  register int i,h;

  numXmode =GetNearXMode(&X,&Y);

  //Set global variable
  FlagFlipVMode =1;
  XModeFlag =1;
  VesaFlag =0;
  OffsetVPage =OffPageTable[numXmode];

  WidthPageFactor = X>>2;
  ClipMinX =0;
  ClipMinY =0;
  ClipMaxX =X;
  ClipMaxY =Y;
  SizeScreenX =X;
  SizeScreenY =Y;
  RecalcWidthAdressTable(X,Y,Memory);

  WaitVrt(1);
  bout(SC_INDEX,1);
  bout(SC_DATA,(bin(SC_DATA)|0x20));

  for(i=5;i>0;i--)
   {
    bout(ACW_FF,0x2f+i);
    bout(ACW_FF,Attrib[i-1]);
   }

  for(i=0x10;i>0;i--)
   {
    bout(ACW_FF,i-1);
    bout(ACW_FF,i-1);
   }

  bout(ACW_FF,0x30);
  wout(SC_INDEX,0x604);
  wout(SC_INDEX,0x100);
  bout(MISC_OUTPUT,MORValue[numXmode]);

  for(i=5;i>0;i--)
   {
    h=Sequencer[i-1]<<8;
    h|=(i-1);
    wout(SC_INDEX,h);
   }

  for(i=0x19;i>0;i--)
   {
    h=CRTCValue[numXmode][i-1]<<8;
    h|=(i-1);
    wout(CRTC_INDEX,h);
   }

  for(i=9;i>0;i--)
   {
    h=Graphics[i-1]<<8;
    h|=(i-1);
    wout(GC_INDEX,h);
   }

  wout(SC_INDEX,0xf02);
  stos(0xa0000,0xffff);
  wout(SC_INDEX,0x101);

  return 3; //Xmode installed success
}

void SetPalette(i,r,g,b)
{
  //WaitVrt(1);
  bout(PEL_MASK,0xff);
  bout(PEL_SET,i);
  bout(PEL_DATA,r>>2);
  bout(PEL_DATA,g>>2);
  bout(PEL_DATA,b>>2);
}

//without clipping
void SetDotHorRaster(register int XPos,register int YPos,int Color)
{
  //if((ClipMinX <XPos< ClipMaxX)&&(ClipMinY <YPos< ClipMaxY))
  set_dot(WidthAdressTable[YPos] + XPos,Color);
}

//with clipping
void MoveColumn2HorRaster(register int XBeg,register int YBeg,register int Hiest)
{
  if((YBeg>ClipMaxY)||(XBeg>ClipMaxX)) return;
  if(YBeg+Hiest>ClipMaxY) Hiest =ClipMaxY-YBeg;

  movecol(WidthAdressTable[YBeg] + XBeg,ColumnHorRaster,Hiest,SizeScreenX);
}

//with clipping
void MoveString2HorRaster(register int XBeg,register int YBeg,register int Lenght)
{
  if((YBeg>ClipMaxY)||(XBeg>ClipMaxX)) return;
  if(XBeg+Lenght>ClipMaxX) Lenght =ClipMaxX-XBeg;

  Lenght&0xfffffffc?move(WidthAdressTable[YBeg] + XBeg,StringHorRaster,Lenght):movsb(WidthAdressTable[YBeg] + XBeg,StringHorRaster,Lenght);
}

short TestVMode(int X,int Y,unsigned char *Memory)
{
        int XPos=0,YPos=0,Color=0;
        short i;

        if(i =SetGraphMode(X,Y,8,Memory)) return i;

        for(XPos=ClipMinX;XPos<ClipMaxX;XPos++)
         {
          for(YPos=ClipMinY;YPos<ClipMaxY;YPos++)
           SetDotHorRaster(XPos,YPos,Color);
          Color++;
         }

        CopyShadowToScreen(Memory);
        SwitchVideoPage();

        for(i=0;i<0xffffff;i++)

        RestoreVMode();
        return 0;
}
