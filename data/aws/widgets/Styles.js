/******* Frame Styles *********
 *
 *  This file is automatically included
 * so that widgets can use these "predefined"
 * styles. 
 */
 
Frames3D =
{
	RidgeAdjust : 5,
	Ridge : function(pen, x, y, x2, y2)
			{		
				var prefs = Skin.current;
						
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawRect(x+2,y+2, x2-2,y2-2, true);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawRect(x+4,y+4, x2-4,y2-4, true);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawRect(x,y, x2,y2, false);		
				
				// Setup pen
				pen.SetColor(prefs.ShadowColor);
				pen.SwapColors();
				pen.SetColor(prefs.HighlightColor);
						
				// Ridge outer
				pen.DrawRect(x+1,y+1, x2-1,y2-1, false, true);
				
				pen.SwapColors();
				
				// Ridge inner
				pen.DrawRect(x+3,y+3, x2-3,y2-3, false, true);				
			},	

	ValleyAdjust : 5,			
	Valley : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawRect(x+2,y+2, x2-2,y2-2, true);
				
				// Darken valley a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawRect(x+2,y+2, x2-2,y2-2, false);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawRect(x,y, x2,y2, false);		
				
				// Setup pen
				pen.SetColor(prefs.HighlightColor);
				pen.SwapColors();
				pen.SetColor(prefs.ShadowColor);
										
				// Ridge outer
				pen.DrawRect(x+1,y+1, x2-1,y2-1, false, true);
				
				pen.SwapColors();
				
				// Ridge inner
				pen.DrawRect(x+3,y+3, x2-3,y2-3, false, true);				
			},
	
	InsetAdjust : 2,		
	Inset : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawRect(x+2,y+1, x2-1,y2-2, true);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawRect(x+2,y+1, x2-1,y2-2, true);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawRect(x,y, x2,y2, false);		
				
				// Setup pen
				pen.SetColor(prefs.HighlightColor);
				pen.SwapColors();				
				pen.SetColor(prefs.ShadowColor);
						
				// Dip
				pen.DrawRect(x+1,y+1, x2-1,y2-1, false, true);
			},
			
	OutsetAdjust : 2,		
	Outset : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawRect(x+2,y+1, x2-1,y2-2, true);
						
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawRect(x,y, x2,y2, false);		
				
				// Setup pen
				pen.SetColor(prefs.ShadowColor);
				pen.SwapColors();
				pen.SetColor(prefs.HighlightColor);
						
				// Bump
				pen.DrawRect(x+1,y+1, x2-1,y2-1, false, true);
			},		
}

// At small sizes, these don't look good.  Widths and heights 
// of at least 50 pixels are recommended.
MiteredFrames3D =
{
	RidgeAdjust : 5,
	Ridge : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawMiteredRect(x+2,y+2, x2-2,y2-2, 0.10,true);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawMiteredRect(x+4,y+4, x2-4,y2-4, 0.10,true);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawMiteredRect(x,y, x2,y2, 0.10,false);		
				
				// Setup pen
				pen.SetColor(prefs.ShadowColor);
				pen.SwapColors();
				pen.SetColor(prefs.HighlightColor);
						
				// Ridge outer
				pen.DrawMiteredRect(x+1,y+1, x2-1,y2-1, 0.10,false, true);
				
				pen.SwapColors();
				
				// Ridge inner
				pen.DrawMiteredRect(x+3,y+3, x2-3,y2-3, 0.10,false, true);				
			},	

	ValleyAdjust : 5,			
	Valley : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawMiteredRect(x+2,y+2, x2-2,y2-2, 0.10, true);
				
				// Darken valley a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawMiteredRect(x+2,y+2, x2-2,y2-2, 0.10, false);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawMiteredRect(x,y, x2,y2, 0.10, false);		
				
				// Setup pen
				pen.SetColor(prefs.HighlightColor);
				pen.SwapColors();
				pen.SetColor(prefs.ShadowColor);
										
				// Ridge outer
				pen.DrawMiteredRect(x+1,y+1, x2-1,y2-1, 0.10, false, true);
				
				pen.SwapColors();
				
				// Ridge inner
				pen.DrawMiteredRect(x+3,y+3, x2-3,y2-3, 0.10, false, true);				
			},
	
	InsetAdjust : 2,		
	Inset : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawMiteredRect(x+2,y+2, x2-1,y2-1, 0.10,true);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawMiteredRect(x+2,y+2, x2-1,y2-1, 0.10,true);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawMiteredRect(x,y, x2,y2, 0.10,false);		
				
				// Setup pen
				pen.SetColor(prefs.HighlightColor);
				pen.SwapColors();				
				pen.SetColor(prefs.ShadowColor);
						
				// Dip
				pen.DrawMiteredRect(x+1,y+1, x2-1,y2-1, 0.10,false, true);
			},

	OutsetAdjust : 2,			
	Outset : function(pen, x, y, x2, y2)
			{
				var prefs = Skin.current;
								
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.DrawMiteredRect(x+2,y+2, x2-2,y2-2,0.10, true);
						
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawMiteredRect(x,y, x2,y2, 0.10,false);		
				
				// Setup pen
				pen.SetColor(prefs.ShadowColor);
				pen.SwapColors();
				pen.SetColor(prefs.HighlightColor);
						
				// Bump
				pen.DrawMiteredRect(x+1,y+1, x2-1,y2-1, 0.10,false, true);
			},		
}



StyleFlat = 
{
	TitleBar : function(pen)
	{
		prefs = Skin.current;
		
		var w = this.width, h = this.height;
		var cw = w * prefs.TitleBarRoundness, aw=cw*2.0;		
		
		pen.Clear();
		
		// Draw the frame.
		pen.SetColor(prefs.FillColor);		
		pen.DrawArc(0,0,aw,h, Math.PI, Math.PI*1.6, true);
		pen.DrawArc(w-aw,0,w,h, Math.PI*1.5, Math.PI*2.1, true);
		pen.DrawRect(cw,0,w-cw,h, true);
		pen.DrawRect(0,h>>1, cw,h, true);
		pen.DrawRect(w-cw,h>>1, w,h, true);		
		
		
		pen.SetColor(0,0,0,1);
		pen.DrawArc(0,0,aw,h, Math.PI, Math.PI*1.6, false);
		pen.DrawLine(0,h>>1,0,h);
		pen.DrawLine(cw,0,w-cw,0);
		
		pen.DrawArc(w-aw,0,w,h, Math.PI*1.5, Math.PI*2.1, false);
		pen.DrawLine(w,h>>1,w,h);
		pen.DrawLine(0,h,w,h);		
	}		  
	
};  // End StyleFlat


Style3D = 
{
	TitleBar : function(pen)
	{
		var prefs = Skin.current;
				
		var w = this.width, h = this.height;
		var aw = prefs.TitleBarHeight, cw=aw>>1;		
		var tbw = prefs.WindowMin.w + prefs.WindowZoom.w + prefs.WindowClose.w + 10;
		
		pen.Clear();
		
		// Draw the frame.
		pen.SetColor(prefs.FillColor);		
		pen.DrawArc(0,0,aw,h, Math.PI*0.5, Math.PI*1.5, true);
		pen.DrawRect(cw,0,w,h, true);
		
		// Background color
		pen.SetColor(prefs.ActiveTitleBarColor1);
		pen.DrawRect(cw+5,5, w-tbw, h-5, true);
		
		// Setup pen
		pen.SetColor(prefs.ShadowColor);
		pen.SwapColors();
		pen.SetColor(prefs.HighlightColor);
		
		// Make the highlights
		pen.DrawArc(0,0,aw,h, Math.PI*0.5, Math.PI*1.5, false);
		pen.DrawLine(cw,0, w,0);
		pen.SwapColors();
		pen.DrawLine(w,0, w,h);
		pen.DrawLine(cw,h,w,h);
				
		pen.DrawRect(cw+5,5, w-tbw, h-5, false, true);	
		
		pen.SetColor(prefs.TitleBarTextColor);
		pen.WriteBoxed(prefs.Font, cw+10, 5, w-tbw-5, h-5, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);	
		
	},
	
	ScrollBar : function(pen)
	{
		var w = this.width, h = this.height;
				
		var prefs = Skin.current;
		var frame = Frames3D;
		
		var pos = this.value;
		var size = this.bar_size;
		var max  = this.max;
		var btn_w = prefs.ScrollBarWidth;
		var btn_h = prefs.ScrollBarHeight;		
		var tri_qw = btn_w/3.0;
		var tri_qh = btn_h/3.0;
		
		
		pen.Clear();
			
		
		//  value      start_pos
		// ------- =  -----------
		//  max         height (or width)
		
		if (this.orientation_vertical)
		{
			var y = (pos*(h-frame.InsetAdjust-size-(btn_h*3))) / max;
			
			// Background
			frame.Inset(pen,0,btn_h*2,w,h-btn_h);
			
			// Bar
			frame.Outset(pen, frame.InsetAdjust, y+frame.InsetAdjust+btn_h*2, w-frame.InsetAdjust, y+frame.InsetAdjust+(btn_h*2)+size);
			
			// Buttons
			frame.Outset(pen, 0,0, w, btn_h);
			frame.Outset(pen, 0,btn_h, w, btn_h*2);
			
			// Arrows
			pen.SetColor(0,0,0,1);			
			pen.DrawTriangle(btn_w*0.5, tri_qh, btn_w-tri_qw, btn_h-tri_qh, tri_qw, btn_h-tri_qh, true);			
			pen.DrawTriangle(btn_w*0.5, tri_qh*5, tri_qw, btn_h+tri_qh, btn_w-tri_qw, btn_h+tri_qh, true);
			
			
		}
		else
		{
			var x = (pos*(w-frame.InsetAdjust-size-(btn_w*3))) / max;
			
			// Background 
			frame.Inset(pen,btn_w*2,0, w-btn_w, h);
			
			// Bar
			frame.Outset(pen, x+frame.InsetAdjust+btn_w*2, frame.InsetAdjust, x+frame.InsetAdjust+(btn_w*2)+size, h-frame.InsetAdjust);
			
			// Buttons
			frame.Outset(pen, 0,0, btn_w,h);
			frame.Outset(pen, btn_w, 0, btn_w*2,h);
			
			// Arrows
			pen.SetColor(0,0,0,1);			
			pen.DrawTriangle(tri_qw, btn_h*0.5,  btn_w-tri_qw, tri_qh, btn_w-tri_qw, btn_h-tri_qh, true);			
			pen.DrawTriangle(tri_qw*5, btn_h*0.5, btn_w+tri_qw, btn_h-tri_qh, btn_w+tri_qw, tri_qh, true);
		}
	},
	
	Button : function(pen)
	{
		var w = this.width, h = this.height;
		var frame = Frames3D;
		
		if (this.state) frame.Inset(pen,0,0,w,h);
		else frame.Outset(pen,0,0,w,h);
		
		if (this.onDrawContent) this.onDrawContent(pen);			
	}
	
	
	
	
}; // end Style3D