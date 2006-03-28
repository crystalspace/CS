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
				pen.SetFlag(Pen.FLAG_FILL);
				pen.DrawRect(x+2,y+2, x2-2,y2-2);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawRect(x+4,y+4, x2-4,y2-4);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.ClearFlag(Pen.FLAG_FILL);
				pen.DrawRect(x,y, x2,y2);		
				
				// Setup pen
				pen.SetColor(prefs.ShadowColor);
				pen.SwapColors();
				pen.SetColor(prefs.HighlightColor);
				pen.SetFlag(Pen.FLAG_SWAPCOLORS);
						
				// Ridge outer
				pen.DrawRect(x+1,y+1, x2-1,y2-1);
				
				pen.SwapColors();
				
				// Ridge inner
				pen.DrawRect(x+3,y+3, x2-3,y2-3);				
			},	

	ValleyAdjust : 5,			
	Valley : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.SetFlag(Pen.FLAG_FILL);
				pen.DrawRect(x+2,y+2, x2-2,y2-2);
				
				// Darken valley a little
				pen.SetColor(0,0,0,0.25);
				pen.ClearFlag(Pen.FLAG_FILL);
				pen.DrawRect(x+2,y+2, x2-2,y2-2);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.DrawRect(x,y, x2,y2);		
				
				// Setup pen
				pen.SetColor(prefs.HighlightColor);
				pen.SwapColors();
				pen.SetColor(prefs.ShadowColor);
				pen.SetFlag(Pen.FLAG_SWAPCOLORS);
										
				// Ridge outer
				pen.DrawRect(x+1,y+1, x2-1,y2-1);
				
				pen.SwapColors();
				
				// Ridge inner
				pen.DrawRect(x+3,y+3, x2-3,y2-3);				
			},
	
	InsetAdjust : 2,		
	Inset : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.SetFlag(Pen.FLAG_FILL);
				pen.DrawRect(x+2,y+1, x2-1,y2-2);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawRect(x+2,y+1, x2-1,y2-2);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.ClearFlag(Pen.FLAG_FILL);
				pen.DrawRect(x,y, x2,y2);		
				
				// Setup pen
				pen.SetColor(prefs.HighlightColor);
				pen.SwapColors();				
				pen.SetColor(prefs.ShadowColor);
				pen.SetFlag(Pen.FLAG_SWAPCOLORS);
						
				// Dip
				pen.DrawRect(x+1,y+1, x2-1,y2-1);
			},
			
	OutsetAdjust : 2,		
	Outset : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.SetFlag(Pen.FLAG_FILL);
				pen.DrawRect(x+2,y+1, x2-1,y2-2);
						
				// Black border
				pen.SetColor(0,0,0,1);
				pen.ClearFlag(Pen.FLAG_FILL);
				pen.DrawRect(x,y, x2,y2);		
				
				// Setup pen
				pen.SetColor(prefs.ShadowColor);
				pen.SwapColors();
				pen.SetColor(prefs.HighlightColor);
				pen.SetFlag(Pen.FLAG_SWAPCOLORS);
						
				// Bump
				pen.DrawRect(x+1,y+1, x2-1,y2-1);
			},		
			
	FlatAdjust : 2,		
	Flat : function(pen, x, y, x2, y2)
			{				
				var prefs = Skin.current;
				
				// Back of panel
				pen.SetColor(prefs.FillColor);
				pen.SetFlag(Pen.FLAG_FILL);
				pen.DrawRect(x+2,y+1, x2-1,y2-2);
				
				// Darken back a little
				pen.SetColor(0,0,0,0.25);
				pen.DrawRect(x+2,y+1, x2-1,y2-2);
				
				// Black border
				pen.SetColor(0,0,0,1);
				pen.ClearFlag(Pen.FLAG_FILL);
				pen.DrawRect(x,y, x2,y2);				
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
			
	FlatAdjust : 2,		
	Flat : function(pen, x, y, x2, y2)
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
			},	
}
