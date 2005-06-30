#ifndef __CS_PEN_H__
#define __CS_PEN_H__

  /** A pen is used to draw vector shapes. */
  struct iPen
  {
    /** Draws a single line. */
    virtual void line(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color)=0;

    /** Draws a single point. */
    virtual void point(int32 x1, int32 y2, uint32 color)=0;

    /** Draws a rectangle. */
    virtual void rect(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color, bool fill=false)=0;
    
    /** Draws a mitered rectangle. */
    virtual void mitered_rect(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color, bool fill=false)=0;

    /** Draws a rounded rectangle. */
    virtual void rounded_rect(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color, bool fill=false)=0;    
  };



#endif
