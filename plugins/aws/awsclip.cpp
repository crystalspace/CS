#include "cssysdef.h"
#include "awsclip.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"

awsClipper::awsClipper(iGraphics3D *_g3d, iGraphics2D *_g2d):g3d(_g3d), g2d(_g2d)
{
}

awsClipper::~awsClipper()
{
}

void
awsClipper::SetClipRect(csRect &r)
{
  clip.Set(r);
}

void
awsClipper::DrawLine(float x1, float y1, float x2, float y2, int color)
{
 if (g2d->ClipLine(x1, y1, x2, y2, clip.xmin, clip.ymin, clip.xmax, clip.ymax))
   return;

 g2d->DrawLine(x1, y1, x2, y2, color);
}

void
awsClipper::DrawBox (int x, int y, int w, int h, int color)
{
  /* Perform intersection of this box with the clipping rectangle.  Only the intersection will
   * get drawn.
   */

  csRect r(x,y,x+w,y+h);

  r.Intersect(clip);
  if (r.IsEmpty())
    return;
  else
    g2d->DrawBox(r.xmin, r.ymin, r.Width(), r.Height(), color);
}

void
awsClipper::DrawPixel (int x, int y, int color)
{
  if (clip.Contains(x,y))
    g2d->DrawPixel(x,y,color);
}

void
awsClipper::Write (iFont *font, int x, int y, int fg, int bg,const char *str)
{
  // This is pretty complicated, so ignore it for now.
  g2d->Write(font, x, y, fg, bg, str);
}

void
awsClipper::DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  csRect sr(sx, sy, sx+sw, sy+sh);

  // Get the intersection of the two.
  sr.Intersect(clip);

  if (sw==tw) tw=sr.Width();
  if (sh==th) th=sr.Height();

  tx+=(sr.xmin-sx);
  ty+=(sr.ymin-sy);

  g3d->DrawPixmap(hTex, sr.xmin, sr.ymin, sr.Width(), sr.Height(), tx, ty, tw, th, Alpha);
}

