/***************************************************************************
                          cfclasses.cpp  -  APPmaster source
                             -------------------
    begin                : Tue Jul 25 2000
    copyright            : (C) 2000 by George Yohng
    email                : yohng@drivex.dosware.8m.com
 ***************************************************************************/


/* WARNING: This program is not general purpose - it converts 32x32
            non-compressed 32-bit TGA files only. */

#include <stdio.h>

unsigned char px[32][32][4];

int main(int argc, char *argv[])
{
    int t,ct=0,x,y,d;

    if (argc!=2)
      return -1;

    for(t=0;t<18;t++)
      fgetc(stdin);

    fread(px,4096,1,stdin);

    fprintf(stdout,"unsigned char %s[4096]=\n{",argv[1]);

    for(y=31;y>=0;y--)
      for(x=0;x<32;x++)
        for(d=0;d<4;d++)
        {
            t = px[y][x][d];

            if (d==3)
    	      t=255-t;

            if (ct)
  	          fprintf(stdout,",");

	        if ((!(ct%14))&&(ct))
	          fprintf(stdout,"\n ");

            fprintf(stdout,"0x%02X",t);

	        ct++;
        }
    fprintf(stdout,"};\n\n");
    return 0;
}
