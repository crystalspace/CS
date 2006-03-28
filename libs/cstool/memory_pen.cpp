#include "cssysdef.h"
#include "cstool/pen.h"


enum { PEN_OP_NOP = 0, PEN_OP_SETFLAG, PEN_OP_CLEARFLAG,
	   PEN_OP_SETCOLOR, PEN_OP_SETTEXTURE, PEN_OP_SWAPCOLORS, PEN_OP_SETWIDTH, 
	   PEN_OP_CLEARTRANSFORM, PEN_OP_PUSHTRANSFORM, PEN_OP_POPTRANSFORM,
       PEN_OP_SETORIGIN, PEN_OP_TRANSLATE, PEN_OP_ROTATE, PEN_OP_DRAWLINE, PEN_OP_DRAWPOINT,
       PEN_OP_DRAWRECT, PEN_OP_DRAWMITEREDRECT, PEN_OP_DRAWROUNDRECT,
       PEN_OP_DRAWARC, PEN_OP_DRAWTRIANGLE, PEN_OP_WRITE, PEN_OP_WRITEBOXED };
       
/** Clears the draw buffer out. */
void csMemoryPen::Clear() 
{
	if (buf) delete buf;
	
	buf = new csMemFile();
	buf->SetPos(0);	
	
	textures.DeleteAll();
	
	SetPenWidth(1.0); 
	ClearFlag(0xffffffff);
}


void  csMemoryPen::Draw(iPen *_pen) 
{	
	buf->SetPos(0);
	
	while(!(buf->AtEOF()))
	{
		uint8 op;
		
		buf->Read((char *)&op, sizeof(uint8));
		
		switch(op)
		{
			case PEN_OP_NOP:
				// Error!
				return;
			break;
			
			case PEN_OP_SETFLAG:
			{
				uint f;
				
				buf->Read((char *)&f, sizeof(uint));
				
				_pen->SetFlag(f);
			} break;
			
			case PEN_OP_CLEARFLAG:
			{
				uint f;
				
				buf->Read((char *)&f, sizeof(uint));
				
				_pen->ClearFlag(f);
			} break;
			
			case PEN_OP_SETCOLOR:
			{
				float r,g,b,a;
				
				buf->Read((char *)&r, sizeof(float));
				buf->Read((char *)&g, sizeof(float));	
				buf->Read((char *)&b, sizeof(float));	
				buf->Read((char *)&a, sizeof(float));
				
				_pen->SetColor(r,g,b,a);				
			} break;
			
			case PEN_OP_SETTEXTURE:
			{			
				iTextureHandle *th;	
				
				buf->Read((char *)&th, sizeof(iTextureHandle *));
				
				_pen->SetTexture(csRef<iTextureHandle>(th));
			}; break;
			
			case PEN_OP_SWAPCOLORS:
				_pen->SwapColors();
				break;
				
			case PEN_OP_SETWIDTH:
			{
				float w;
				
				buf->Read((char *)&w, sizeof(float));				
				_pen->SetPenWidth(w);
			}	break;
				
			case PEN_OP_CLEARTRANSFORM:
				_pen->ClearTransform();
				break;
				
			case PEN_OP_PUSHTRANSFORM:
				_pen->PushTransform();
				break;
				
			case PEN_OP_POPTRANSFORM:
				_pen->PopTransform();
				break;
				
			case PEN_OP_SETORIGIN:
			{
				float x,y,z;
				
				buf->Read((char *)&x, sizeof(float));
				buf->Read((char *)&y, sizeof(float));	
				buf->Read((char *)&z, sizeof(float));	
				
				_pen->SetOrigin(csVector3(x,y,z));				
			} break;
			
			case PEN_OP_TRANSLATE:
			{
				float x,y,z;
				
				buf->Read((char *)&x, sizeof(float));
				buf->Read((char *)&y, sizeof(float));	
				buf->Read((char *)&z, sizeof(float));	
				
				_pen->Translate(csVector3(x,y,z));				
			} break;
			
			case PEN_OP_ROTATE:
			{
				float a;
				
				buf->Read((char *)&a, sizeof(float));
				
				_pen->Rotate(a);				
			} break;
			
			case PEN_OP_DRAWLINE:
			{
				uint x1, y1, x2, y2;
						
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));								
								
				_pen->DrawLine(x1,y1,x2,y2);					
		    } break;
		    
		    case PEN_OP_DRAWRECT:
			{
				uint x1, y1, x2, y2;				
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
								
				_pen->DrawRect(x1,y1,x2,y2);
		    } break;
		    
		    case PEN_OP_DRAWMITEREDRECT:
			{
				uint x1, y1, x2, y2;
				uint miter;
					
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&miter, sizeof(uint));
								
				_pen->DrawMiteredRect(x1,y1,x2,y2,miter);
		    } break;
		    
		    case PEN_OP_DRAWROUNDRECT:
			{
				uint x1, y1, x2, y2;
				uint roundness;
				
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&roundness, sizeof(uint));
								
				_pen->DrawRoundedRect(x1,y1,x2,y2,roundness);
		    } break;
		    
		    case PEN_OP_DRAWARC:
			{
				uint x1, y1, x2, y2;
				float start_angle, end_angle;				
	
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&start_angle, sizeof(float));
				buf->Read((char *)&end_angle, sizeof(float));				
				
				_pen->DrawArc(x1,y1,x2,y2,start_angle,end_angle);
		    } break;
		    
		    case PEN_OP_DRAWTRIANGLE:
			{
				uint x1, y1, x2, y2, x3, y3;
					
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));	
				buf->Read((char *)&x3, sizeof(uint));
				buf->Read((char *)&y3, sizeof(uint));									
												
				_pen->DrawTriangle(x1,y1,x2,y2,x3,y3);
		    } break;
		    
		    case PEN_OP_WRITE:
			{
				uint x1, y1, len;
				iFont *font;				
				const char *text;
	
				buf->Read((char *)&font, sizeof(iFont *));
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));				
				buf->Read((char *)&len, sizeof(uint));						
				
				// Read the text right out of the buffer.
				text = buf->GetData() + buf->GetPos();
				
				// Seek forward, skipping the null.
				buf->SetPos(buf->GetPos()+len+1);
				
				_pen->Write(font,x1,y1,(char *)text);
		    } break;
		    
		    case PEN_OP_WRITEBOXED:
			{
				uint x1, y1, x2, y2, h_align, v_align, len;
				iFont *font;				
				const char *text;
	
				buf->Read((char *)&font, sizeof(iFont *));
				buf->Read((char *)&x1, sizeof(uint));
				buf->Read((char *)&y1, sizeof(uint));				
				buf->Read((char *)&x2, sizeof(uint));
				buf->Read((char *)&y2, sizeof(uint));				
				buf->Read((char *)&h_align, sizeof(uint));									
				buf->Read((char *)&v_align, sizeof(uint));				
				buf->Read((char *)&len, sizeof(uint));									
				
				// Read the text right out of the buffer.
				text = buf->GetData() + buf->GetPos();
				
				// Seek forward, skipping the null.
				buf->SetPos(buf->GetPos()+len+1);
				
				_pen->WriteBoxed(font,x1,y1,x2,y2,h_align, v_align,(char *)text);
		    } break;
		    
		    default:
		    	// An unrecognized op is bad, so abort.
		    	return;
		    				
		} // end switch op		
		
	} // end while not eof.	
}

/** 
* Sets the given flag. 
* @param flag The flag to set. 
*/
void csMemoryPen::SetFlag(uint flag)
{
	uint8 op = PEN_OP_SETFLAG;
	
	buf->Write((const char *)&op, sizeof(uint8));	
	buf->Write((const char *)&flag, sizeof(uint));
}

/** 
* Clears the given flag. 
* @param flag The flag to clear. 
*/
void csMemoryPen::ClearFlag(uint flag)
{
	uint8 op = PEN_OP_CLEARFLAG;
	
	buf->Write((const char *)&op, sizeof(uint8));	
	buf->Write((const char *)&flag, sizeof(uint));	
}


/** 
* Sets the current color. 
*/
void csMemoryPen::SetColor (float r, float g, float b, float a) 
{ 
	uint8 op = PEN_OP_SETCOLOR;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&r, sizeof(float));
	buf->Write((const char *)&g, sizeof(float));	
	buf->Write((const char *)&b, sizeof(float));	
	buf->Write((const char *)&a, sizeof(float));	
}

/** 
* Sets the current color. 
*/
void csMemoryPen::SetColor(const csColor4 &color)
{ 
	uint8 op = PEN_OP_SETCOLOR;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&color.red, sizeof(float));	
	buf->Write((const char *)&color.green, sizeof(float));	
	buf->Write((const char *)&color.blue, sizeof(float));	
	buf->Write((const char *)&color.alpha, sizeof(float));		
}

void csMemoryPen::SetTexture(csRef<iTextureHandle> _tex)
{
	uint8 op = PEN_OP_SETTEXTURE;
	
	iTextureHandle *th = (iTextureHandle *)_tex;
	
	textures.Push(csRef<iTextureHandle>(th));
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)th, sizeof(iTextureHandle *));
}

/**
* Swaps the current color and the alternate color. 
*/
void csMemoryPen::SwapColors() 
{ 
	uint8 op = PEN_OP_SWAPCOLORS;
	
	buf->Write((const char *)&op, sizeof(uint8));	
}

/**
* Sets the width of the pen for line drawing. 
*/
void csMemoryPen::SetPenWidth(float width)
{
	uint8 op = PEN_OP_SETWIDTH;
		
	buf->Write((const char *)&op, sizeof(uint8));	
	buf->Write((const char *)&width, sizeof(float));	
}


/**    
* Clears the current transform, resets to identity.
*/
void csMemoryPen::ClearTransform() 
{ 
	uint8 op = PEN_OP_CLEARTRANSFORM;
	
	buf->Write((const char *)&op, sizeof(uint8));		
}

/** 
* Pushes the current transform onto the stack. *
*/
void csMemoryPen::PushTransform() 
{ 
	uint8 op = PEN_OP_PUSHTRANSFORM;
	
	buf->Write((const char *)&op, sizeof(uint8));	
}

/**
* Pops the transform stack. The top of the stack becomes the current
* transform. 
*/
void csMemoryPen::PopTransform() 
{ 
	uint8 op = PEN_OP_POPTRANSFORM;
	
	buf->Write((const char *)&op, sizeof(uint8));	
}

/** 
* Sets the origin of the coordinate system. 
*/
void csMemoryPen::SetOrigin(const csVector3 &o) 
{ 
	uint8 op = PEN_OP_SETORIGIN;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&o.x, sizeof(float));	
	buf->Write((const char *)&o.y, sizeof(float));	
	buf->Write((const char *)&o.z, sizeof(float));		
}

/** 
* Translates by the given vector
*/
void csMemoryPen::Translate(const csVector3 &t) 
{ 
	uint8 op = PEN_OP_TRANSLATE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&t.x, sizeof(float));	
	buf->Write((const char *)&t.y, sizeof(float));	
	buf->Write((const char *)&t.z, sizeof(float));	
}

/**
* Rotates by the given angle.
*/
void csMemoryPen::Rotate(const float &a) 
{ 
	uint8 op = PEN_OP_ROTATE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&a, sizeof(float));		
}

/** 
* Draws a single line. 
*/
void csMemoryPen::DrawLine (uint x1, uint y1, uint x2, uint y2) 
{ 
	uint8 op = PEN_OP_DRAWLINE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));		
}

/** 
* Draws a single point. 
*/
void csMemoryPen::DrawPoint (uint x1, uint y2)
{
	uint8 op = PEN_OP_DRAWPOINT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));		
}

/** 
* Draws a rectangle. 
*/
void csMemoryPen::DrawRect (uint x1, uint y1, uint x2, uint y2) 
{ 
	uint8 op = PEN_OP_DRAWRECT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));	
}

/** 
* Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
* and determines how much of the corner is mitered off and beveled. 
*/
void csMemoryPen::DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
	uint miter)
{
	uint8 op = PEN_OP_DRAWMITEREDRECT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&miter, sizeof(uint));	
}

/** 
* Draws a rounded rectangle. The roundness value should be between
* 0.0 and 1.0, and determines how much of the corner is rounded off. 
*/
void csMemoryPen::DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
	uint roundness) 
{ 
	uint8 op = PEN_OP_DRAWROUNDRECT;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&roundness, sizeof(uint));	
} 

/** 
* Draws an elliptical arc from start angle to end angle.  Angle must be
* specified in radians. The arc will be made to fit in the given box.
* If you want a circular arc, make sure the box is a square.  If you want
* a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
* angle.
*/
void csMemoryPen::DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle,
	float end_angle) 	
{ 
	uint8 op = PEN_OP_DRAWARC;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&start_angle, sizeof(float));
	buf->Write((const char *)&end_angle, sizeof(float));	
}

/**
* Draws a triangle around the given vertices. 
*/
void csMemoryPen::DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3) 
{ 
	uint8 op = PEN_OP_DRAWTRIANGLE;
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&x3, sizeof(uint));
	buf->Write((const char *)&y3, sizeof(uint));		
}

/**
* Writes text in the given font at the given location.
*/
void csMemoryPen::Write(iFont *font, uint x1, uint y1, char *text) 
{ 
	uint8 op = PEN_OP_WRITE;
	uint len = strlen(text);
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&font, sizeof(iFont *));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&len, sizeof(uint));
	buf->Write((const char *)text, len+1); // include the zero terminator
}

/**
* Writes text in the given font, in the given box.  The alignment
* specified in h_align and v_align determine how it should be aligned.  
*/
void csMemoryPen::WriteBoxed(iFont *font, uint x1, uint y1, uint x2, uint y2, 
	uint h_align, uint v_align, char *text) 
{ 
	uint8 op = PEN_OP_WRITEBOXED;
	uint len = strlen(text);
	
	buf->Write((const char *)&op, sizeof(uint8));
	buf->Write((const char *)&font, sizeof(iFont *));
	buf->Write((const char *)&x1, sizeof(uint));
	buf->Write((const char *)&y1, sizeof(uint));
	buf->Write((const char *)&x2, sizeof(uint));
	buf->Write((const char *)&y2, sizeof(uint));
	buf->Write((const char *)&h_align, sizeof(uint));
	buf->Write((const char *)&v_align, sizeof(uint));
	buf->Write((const char *)&len, sizeof(uint));
	buf->Write((const char *)text, len+1); // include the zero terminator
}
