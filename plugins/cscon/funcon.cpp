/*
    Copyright (C) 2000 by Norman Krämer

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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cssysdef.h"
#include "iconsole.h"
#include "icursor.h"
#include "igraph2d.h"
#include "isystem.h"
#include "itxtmgr.h"
#include "cssys/csevent.h"
#include "csutil/csrect.h"
#include "csutil/scf.h"
#include "csutil/csstring.h"
#include "funcon.h"
#include "conbuffr.h"
#include "csutil/inifile.h"
#include "csgfxldr/csimage.h"

#define G3D piG3D

funConsole::funConsole (iBase *base) : csConsole (NULL)
{
  CONSTRUCT_IBASE (base);
  border_computed = false;
  pix_loaded = false;
}

funConsole::~funConsole ()
{
  if (piG3D)
    piG3D->DecRef ();
  if (piVFS)
    piVFS->DecRef ();
}

bool funConsole::Initialize (iSystem *system) 
{
  bool succ = csConsole::Initialize ( system );
  piG3D = QUERY_PLUGIN_ID (piSystem, CS_FUNCID_VIDEO, iGraphics3D);

  outersize.Set ( size );
  if (!piG3D)
    return false;
  piVFS = QUERY_PLUGIN_ID (piSystem, CS_FUNCID_VFS, iVFS);
  if (!piVFS)
    return false;

  // Tell system driver that we want to handle broadcast events
  if (!piSystem->CallOnEvents (this, CSMASK_Broadcast))
    return false;

  return succ;
}

bool funConsole::HandleEvent (csEvent &Event)
{
  if (Event.Type == csevBroadcast
   && Event.Command.Code == cscmdSystemOpen
   && !pix_loaded)
  {
    LoadPix ();
    pix_loaded = true;
  }
  return false;
}


void funConsole::Draw3D (csRect *)
{
  bool btext, bgour;
  int i;
  long int zBuf;
  G3DPolygonDPFX poly;
  if ( !border_computed )
  {
    // determine what space left to draw the actual console
    memset( &bordersize, 0, sizeof(bordersize) );
    if ( deco.border[0].mat )
      deco.border[0].mat->GetTexture ()->GetMipMapDimensions( 0, bordersize.xmin, bordersize.ymin );
    if ( deco.border[2].mat )
      deco.border[2].mat->GetTexture ()->GetMipMapDimensions( 0, bordersize.xmax, bordersize.ymax );

    SetTransparency( true ); // other wise 2D-part will overdraw all we paint here
    border_computed = true;
    SetPosition( outersize.xmin, outersize.ymin, outersize.Width(), outersize.Height() );
  }

  zBuf = G3D->GetRenderState (G3DRENDERSTATE_ZBUFFERMODE);
  btext = G3D->GetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE);
  bgour = G3D->GetRenderState (G3DRENDERSTATE_GOURAUDENABLE);

  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILL );

  int height = G3D->GetHeight()-1;

  // first draw the background
  // do we draw gouraud/flat or with texture ?
 
  bool with_color = deco.bgnd.mat == NULL;

  poly.num = 4;
  poly.vertices[0].sx = size.xmin;
  poly.vertices[0].sy = height-size.ymin;
  poly.vertices[1].sx = size.xmax;
  poly.vertices[1].sy = height-size.ymin;
  poly.vertices[2].sx = size.xmax;
  poly.vertices[2].sy = height-size.ymax;
  poly.vertices[3].sx = size.xmin;
  poly.vertices[3].sy = height-size.ymax;
  poly.use_fog = false;

  float u_stretch=1.0, v_stretch=1.0;

  if ( !with_color && !deco.bgnd.do_stretch )
  {
    int w, h;
    deco.bgnd.mat->GetTexture ()->GetMipMapDimensions( 0, w, h );
    u_stretch = ((float)(size.xmax - size.xmin)) / ((float)w);
    v_stretch = ((float)(size.ymax - size.ymin)) / ((float)h);
  }

  poly.vertices[0].u = 0;
  poly.vertices[0].v = 0;
  poly.vertices[1].u = u_stretch;
  poly.vertices[1].v = 0;
  poly.vertices[2].u = u_stretch;
  poly.vertices[2].v = v_stretch;
  poly.vertices[3].u = 0;
  poly.vertices[3].v = v_stretch;
    
  for (i=0; i<poly.num; i++)
  {
    poly.vertices[i].r=((float)deco.bgnd.kr)/255.0;
    poly.vertices[i].g=((float)deco.bgnd.kg)/255.0;
    poly.vertices[i].b=((float)deco.bgnd.kb)/255.0;

    poly.vertices[i].z=1;
  }

  poly.mat_handle = deco.bgnd.mat;
  if (with_color)
    G3D->SetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, false);
  
  float alpha = deco.bgnd.do_alpha ? deco.bgnd.alpha : 0.0;

  G3D->StartPolygonFX (poly.mat_handle, CS_FX_SETALPHA (alpha) |
    CS_FX_COPY | (with_color && deco.bgnd.do_keycolor ? CS_FX_GOURAUD : 0));

  G3D->DrawPolygonFX (poly);
  G3D->FinishPolygonFX ();

  if (with_color)
    G3D->SetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, true);
  
  // draw the top left decoration
  DrawBorder (outersize.xmin, height-outersize.ymin, bordersize.xmin, bordersize.ymin, deco.border[0], 0);
  // draw the top decoration
  DrawBorder (p2size.xmin-deco.p2lx, height-outersize.ymin, p2size.Width()+deco.p2lx+deco.p2rx, bordersize.ymin,  deco.border[1], 1);
  // draw the top right decoration
  DrawBorder (p2size.xmax, height-outersize.ymin, bordersize.xmax, bordersize.ymin, deco.border[2], 0);
  // draw the right decoration
  DrawBorder (p2size.xmax, height-p2size.ymin+deco.p2ty, bordersize.xmax, p2size.Height()+deco.p2by+deco.p2ty, deco.border[3], 2);
  // draw the bottom right decoration
  DrawBorder (p2size.xmax, height-p2size.ymax, bordersize.xmax, bordersize.ymax, deco.border[4], 0);
  // draw the bottom decoration
  DrawBorder (p2size.xmin-deco.p2lx, height-p2size.ymax, p2size.Width()+deco.p2lx+deco.p2rx, bordersize.ymax, deco.border[5], 3);
  // draw the bottom left decoration
  DrawBorder (outersize.xmin, height-p2size.ymax, bordersize.xmin, bordersize.ymax, deco.border[6], 0);
  // draw the left decoration
  DrawBorder (outersize.xmin, height-p2size.ymin+deco.p2ty, bordersize.xmin, p2size.Height()+deco.p2by+deco.p2ty, deco.border[7], 4);
  
  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, zBuf );
  G3D->SetRenderState (G3DRENDERSTATE_TEXTUREMAPPINGENABLE, btext);
  G3D->SetRenderState (G3DRENDERSTATE_GOURAUDENABLE, bgour);

}

void funConsole::DrawBorder ( int x, int y, int width, int height, ConDecoBorder &border, int align )
{
  if ( border.mat )
  {
    G3DPolygonDPFX poly;
    int i;

    float u_stretch=1.0, v_stretch=1.0;
    int w, h;
    
    border.mat->GetTexture ()->GetMipMapDimensions( 0, w, h );
    switch ( align ){
    case 1:
      height = MIN( height, h );
      h = height;
      break;
    case 2:
      x += MAX( 0, width-w );
      width = MIN( width, w );
      w = width;
      break;
    case 3:
      y -= MAX( 0, height-h );
      height = MIN( h, height );
      h = height;
      break;
    case 4:
      width = MIN( width, w );
      w = width;
      break;
    }
    //    printf("%d, %d,%d,%d,%d\n", align, x,y,w,height);
    
    if ( !border.do_stretch ){
      u_stretch = ((float)width) / ((float)w);
      v_stretch = ((float)height) / ((float)h);
    }

    poly.num=4;
    poly.use_fog = false;
    poly.vertices[0].u = 0;
    poly.vertices[0].v = 0;
    poly.vertices[1].u = u_stretch;
    poly.vertices[1].v = 0;
    poly.vertices[2].u = u_stretch;
    poly.vertices[2].v = v_stretch;
    poly.vertices[3].u = 0;
    poly.vertices[3].v = v_stretch;

    poly.vertices[0].sx = x;
    poly.vertices[0].sy = y;
    poly.vertices[1].sx = x+width ;
    poly.vertices[1].sy = y;
    poly.vertices[2].sx = x+width;
    poly.vertices[2].sy = y-height;
    poly.vertices[3].sx = x;
    poly.vertices[3].sy = y-height;

    for ( i=0; i < 4; i++ ){
      poly.vertices[i].sx -= border.offx;
      poly.vertices[i].sy += border.offy;
      poly.vertices[i].z=1;
      poly.vertices[i].r=1;
      poly.vertices[i].g=1;
      poly.vertices[i].b=1;
    }

    poly.mat_handle = border.mat;

    float alpha = border.do_alpha ? border.alpha : 0.0;
    G3D->StartPolygonFX ( poly.mat_handle, CS_FX_SETALPHA( alpha ) |  (border.do_keycolor ? CS_FX_KEYCOLOR : 0 ) );
    G3D->DrawPolygonFX (poly);
    G3D->FinishPolygonFX ();
  }
  
}

void funConsole::SetPosition( int x, int y, int width, int height )
{
  csConsole::SetPosition ( x, y, width, height );

  outersize.Set( size );
  p2size.Set( size );
  p2size.xmin = p2size.xmin + bordersize.xmin;// - deco.p2lx;
  p2size.xmax = p2size.xmax - bordersize.xmax;// + deco.p2rx;
  p2size.ymin = p2size.ymin + bordersize.ymin;// - deco.p2ty;
  p2size.ymax = p2size.ymax - bordersize.ymax;// + deco.p2by;

  if ( border_computed ){
    size.xmin = size.xmin + bordersize.xmin - deco.p2lx -  deco.lx;
    size.xmax = size.xmax - bordersize.ymax + deco.p2rx +  deco.rx;
    size.ymin = size.ymin + bordersize.ymin - deco.p2ty -  deco.ty;
    size.ymax = size.ymax - bordersize.ymax + deco.p2by + deco.by;
    //    printf("%d %d %d %d %d %d %d\n", bordersize.xmin, deco.p2lx,   deco.lx, size.xmin, size.xmax, size.ymin, size.ymax );
    //    printf("%d %d %d %d\n", outersize.xmin, outersize.xmax, outersize.ymin, outersize.ymax );
    // call again with the final size
    csConsole::SetPosition ( size.xmin, size.ymin, size.Width(), size.Height() );
  }
}

void funConsole::GetPosition(int &x, int &y, int &width, int &height) const
{
  x = outersize.xmin;
  y = outersize.ymin;
  width = outersize.Width();
  height = outersize.Height();
}

void funConsole::LoadPix()
{
  csIniFile *ini = new csIniFile( piVFS, "/config/funcon.cfg" );
  const char* dir = ini->GetStr( "funcon", "zip" );
  const char* mountdir = ini->GetStr( "funcon", "mount" );
  if ( piVFS->Mount( mountdir, dir ) )
  {
    piVFS->PushDir();
    piVFS->ChDir( mountdir );

    // scan in all sections
    PrepPix ( ini, "background", deco.bgnd, true );
    PrepPix ( ini, "topleft", deco.border[0], false );
    PrepPix ( ini, "top", deco.border[1], false );
    PrepPix ( ini, "topright", deco.border[2], false );
    PrepPix ( ini, "right", deco.border[3], false );
    PrepPix ( ini, "bottomright", deco.border[4], false );
    PrepPix ( ini, "bottom", deco.border[5], false );
    PrepPix ( ini, "bottomleft", deco.border[6], false );
    PrepPix ( ini, "left", deco.border[7], false );

    // internal increase/decrease
    deco.p2lx = ini->GetInt( "funcon", "p2lx" );
    deco.p2rx = ini->GetInt( "funcon", "p2rx" );
    deco.p2ty = ini->GetInt( "funcon", "p2ty" );
    deco.p2by = ini->GetInt( "funcon", "p2by" );
    deco.lx = ini->GetInt( "funcon", "lx" );
    deco.rx = ini->GetInt( "funcon", "rx" );
    deco.ty = ini->GetInt( "funcon", "ty" );
    deco.by = ini->GetInt( "funcon", "by" );

    piVFS->PopDir();
    piVFS->Unmount( mountdir, dir );
  }
  else
    printf("Couldn't mount %s on %s\n", dir, mountdir );

  delete ini;
}

void funConsole::PrepPix( csIniFile *ini, const char *sect, ConDecoBorder &border, bool bgnd )
{
  const char* pix = ini->GetStr( sect, "pic", "" );

  border.mat = NULL;
  border.do_keycolor = false;
  border.do_alpha = false;
  border.do_stretch = false;

  if ( strlen( pix ) ){
    size_t len=0;
    char *data = NULL;
    iFile *F = piVFS->Open ( pix, VFS_FILE_READ );
    if ( F ){
      len = F->GetSize ();
      data = new char [len];
      if ( data ) len = F->Read ( data, len );
      F->DecRef();
    }
    if ( len ){
      iTextureManager *tm = piG3D->GetTextureManager();
      iImage *image = csImageLoader::Load( (UByte*)data, len, tm->GetTextureFormat() );
      if ( image ){
	iTextureHandle* txt = tm->RegisterTexture ( image, CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS );
	@@@@@@@@@@@@ NEED A MATERIAL HERE!!??
	border.mat = 
	image->DecRef();

	border.offx=ini->GetInt( sect, "x", 0 );
        border.offy=ini->GetInt( sect, "y", 0 );

	border.do_keycolor = ini->GetYesNo( sect, "do_keycolor", false );
	if ( border.do_keycolor ){
	  int r,g,b;
	  const char *kc = ini->GetStr( sect, "keycolor", "0,0,0" );
	  sscanf( kc, "%d,%d,%d", &r, &g, &b );
	  border.kr=r; border.kg=g; border.kb=b;
	  border.txt->SetKeyColor ( border.kr, border.kg, border.kb );
	}

	border.do_stretch = ini->GetYesNo( sect, "do_stretch", false );
      }

      delete [] data;
    }else{
      printf("couldnt read %s\n", pix );
    }
  }

  border.do_alpha = ini->GetYesNo( sect, "do_alpha", false );
  if ( border.do_alpha ){
    border.alpha = ini->GetFloat( sect, "alpha", 0.0 );
  }
  
  if (bgnd){
    int r,g,b;
    border.do_keycolor = ini->GetYesNo( sect, "do_keycolor", false );
    const char *kc = ini->GetStr( sect, "keycolor", "1,1,1" );
    sscanf( kc, "%d,%d,%d", &r, &g, &b );
    border.kr=r; border.kg=g; border.kb=b;
  }
  
}

IMPLEMENT_IBASE(funConsole)
  IMPLEMENTS_INTERFACE(iConsole)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY(funConsole)
