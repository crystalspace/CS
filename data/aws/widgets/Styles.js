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
			
	FlatAdjust : 2,		
	Flat : function(pen, x, y, x2, y2)
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
	WindowMin : function(pen)
	{
		var prefs = Skin.current;
		
		if (this.over) pen.SetColor(0,0,1,1);
		else pen.SetColor(1,1,1,1);
		
		pen.DrawRect(5, this.height-10, 15, this.height-5, true);
	},
	
	WindowZoom : function(pen)
	{
		var prefs = Skin.current;
		var cx = this.width>>1, cy=this.height>>1;
		var qw = this.width>>2, qh=this.height>>2;
		
		if (this.over) pen.SetColor(0,1,0,1);
		else pen.SetColor(1,1,1,1);
				
		pen.DrawRect(cx-qw, cy-2, cx+qw, cy+2, true);
		pen.DrawRect(cx-2, cy-qh, cx+2, cy+qh, true);
	},
	
	WindowClose : function(pen)
	{
		var prefs = Skin.current;
		var x=5, y;
		
		if (this.over) pen.SetColor(1,0,0,1);
		else pen.SetColor(1,1,1,1);
		
		// Left to right line
		pen.DrawTriangle(6,4, this.width-4, this.height-6, 4, 6, true);
		pen.DrawTriangle(4,6, this.width-4, this.height-6, this.width-6, this.height-4, true);
		
		// Right to left line
		pen.DrawTriangle(this.width-6,4, 6, this.height-4, 4, this.height-6, true);
		pen.DrawTriangle(this.width-6,4, this.width-4, 6, 6, this.height-4, true);
	},
	
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
		pen.WriteBoxed(prefs.TitleFont, cw+10, 5, w-tbw-5, h-5, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);	
		
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
		
		function drawButtons(sb)
		{
			// Bar
			if (sb.scroll_button_down)
				frame.Inset(pen, sb.ButtonScroll.x1, sb.ButtonScroll.y1, 
							 	  sb.ButtonScroll.x2, sb.ButtonScroll.y2);
			else
				frame.Outset(pen, sb.ButtonScroll.x1, sb.ButtonScroll.y1, 
							 	  sb.ButtonScroll.x2, sb.ButtonScroll.y2);
			
			// Dec button
			if (sb.dec_button_down) 
				frame.Inset(pen, sb.ButtonDec.x1, sb.ButtonDec.y1, 
							 	  sb.ButtonDec.x2, sb.ButtonDec.y2);
			else
				frame.Outset(pen, sb.ButtonDec.x1, sb.ButtonDec.y1, 
							 	  sb.ButtonDec.x2, sb.ButtonDec.y2);
			
			// IncButton
			if (sb.inc_button_down) 
				frame.Inset(pen, sb.ButtonInc.x1, sb.ButtonInc.y1, 
							 	  sb.ButtonInc.x2, sb.ButtonInc.y2);
			else
				frame.Outset(pen, sb.ButtonInc.x1, sb.ButtonInc.y1, 
							 	  sb.ButtonInc.x2, sb.ButtonInc.y2);			
				
		}
		
		function arrowPre(flag)
		{
			if (flag) 
			{
				pen.PushTransform();
				pen.Translate(1,1,0);	
			}
		}
		
		function arrowPost(flag)
		{
			if (flag)
				pen.PopTransform();	
		}
			
		
		//  value      start_pos
		// ------- =  -----------
		//  max         height (or width)
		
		if (this.orientation_vertical)
		{
			var y = (pos*(h-frame.InsetAdjust-size-(btn_h*3))) / max;
			
			// Setup button areas for scrollbar controller.
			this.ButtonDec = { x1:0, y1:0, x2:w, y2:btn_h };
			this.ButtonInc  = { x1:0, y1:btn_h, x2:w, y2:btn_h*2 };
			this.ButtonScroll = { x1:frame.InsetAdjust, y1:y+frame.InsetAdjust+btn_h*2, x2:w-frame.InsetAdjust, y2:y+frame.InsetAdjust+(btn_h*2)+size };
						
			// Background
			frame.Inset(pen,0,btn_h*2,w,h-btn_h);
			
			drawButtons(this);
			
			// Arrows
			pen.SetColor(0,0,0,1);							
			arrowPre(this.dec_button_down); pen.DrawTriangle(btn_w*0.5, tri_qh, btn_w-tri_qw, btn_h-tri_qh, tri_qw, btn_h-tri_qh, true);   arrowPost(this.dec_button_down); 
			arrowPre(this.inc_button_down); pen.DrawTriangle(btn_w*0.5, tri_qh*5, tri_qw, btn_h+tri_qh, btn_w-tri_qw, btn_h+tri_qh, true); arrowPost(this.inc_button_down); 			
			
		}
		else
		{
			var x = (pos*(w-frame.InsetAdjust-size-(btn_w*3))) / max;
			
			// Setup button areas for scrollbar controller.
			this.ButtonDec = { x1:0, y1:0, x2:btn_w, y2:h };
			this.ButtonInc = { x1:btn_w, y1:0, x2:btn_w*2, y2:h };
			this.ButtonScroll = { x1:x+frame.InsetAdjust+btn_w*2, y1:frame.InsetAdjust, x2:x+frame.InsetAdjust+(btn_w*2)+size, y2:h-frame.InsetAdjust };
						
			
			// Background 
			frame.Inset(pen,btn_w*2,0, w-btn_w, h);
			
			drawButtons(this);
			
			// Arrows
			pen.SetColor(0,0,0,1);			
			arrowPre(this.dec_button_down); pen.DrawTriangle(tri_qw, btn_h*0.5,  btn_w-tri_qw, tri_qh, btn_w-tri_qw, btn_h-tri_qh, true);	arrowPost(this.dec_button_down); 		
			arrowPre(this.inc_button_down); pen.DrawTriangle(tri_qw*5, btn_h*0.5, btn_w+tri_qw, btn_h-tri_qh, btn_w+tri_qw, tri_qh, true);  arrowPost(this.inc_button_down); 
		}
	},
	
	Button : function(pen)
	{
		var w = this.width, h = this.height;
		var frame = Frames3D;
		
		pen.Clear();
		
		if (this.state) frame.Inset(pen,0,0,w,h);
		else 
		{
			frame.Outset(pen,0,0,w,h);
			if (this.over)
			{	
				pen.SetColor(1,1,1,0.25);			
				pen.DrawRect(0,0,w,h,true);
			}
		}
		
		if (this.onDrawContent)
		{
			if (this.state)
			{
				pen.PushTransform();
				pen.Translate(1,1,0);
			}
			
			this.onDrawContent(pen);			
			
			if (this.state) pen.PopTransform();
		}
	},
	
	Clock : function(pen)
	{
		var angle, steps=Math.PI/6.0;
		var w = this.width, h = this.height;
		var cx = w/2, cy = h/2;
		var r=w/2, num=12;
		var d = new Date();
		
		
		pen.Clear();
		pen.SetWidth(2.5);
		pen.SetColor(0,0,0,1);
		pen.DrawArc(0,0,w,h,0,Math.PI*2.1,false);
		
		pen.SwapColors();		
		pen.SetColor(1,1,1,1);
		pen.SetWidth(5);						
		pen.DrawArc(2,2,w-2,h-2,0,Math.PI*2.1,false);				
		pen.SetColor(0.5,0.5,0.5,1);				
		pen.DrawArc(5,5,w-5,h-5,0,Math.PI*2.1,false);				
		pen.SetColor(1,1,1,1);
		pen.DrawArc(10,10,w-10,h-10,0,Math.PI*2.1,true);
		pen.SetWidth(2);
		pen.SwapColors();
		pen.DrawArc(15,15,w-15,h-15,0,Math.PI*2.1,false);
		
		pen.SetWidth(1.0);
		
		for(angle=Math.PI*1.5; num>0; angle-=steps, --num)
		{
			var ca = Math.cos(angle),
				sa = Math.sin(angle);
			
			var lx1 = ca*(r-15),
				ly1 = sa*(r-15),
				lx2 = ca*(r-20),
				ly2 = sa*(r-20),
				x2 = ca*(r-30),
				y2 = sa*(r-30);
			
			pen.DrawLine(cx+lx1,cy+ly1,cx+lx2,cy+ly2);
			pen.WriteBoxed(Skin.current.ClockFont, cx+x2-5, cy+y2-5, cx+x2+5, cy+y2+5, Pen.ALIGN_CENTER, Pen.ALIGN_CENTER, num); 
		}
		
		steps=Math.PI/30.0;
		for(angle=0; angle<Math.PI*2.0; angle+=steps)
		{
			var ca = Math.cos(angle),
				sa = Math.sin(angle);
			
			var lx1 = ca*(r-40),
				ly1 = sa*(r-40),
				lx2 = ca*(r-45),
				ly2 = sa*(r-45);
			
			pen.DrawLine(cx+lx1,cy+ly1,cx+lx2,cy+ly2);
		}				
		
				
		// Draw hour hand.
		steps=Math.PI/6.0;
		angle = (d.getHours()) * steps;
		angle -= Math.PI * 0.5;		
		pen.SetColor(0,0,0.75,0.5);
		pen.DrawArc(cx-r,cy-r,cx+r,cy+r,angle-(steps/2.0), angle+(steps/2.0), true);
		
		steps = Math.PI/30.0;
		angle = (d.getMinutes()) * steps;		
		angle -= Math.PI * 0.5;
		pen.SetColor(0.75,0,0,0.5);
		pen.DrawArc(0,0,w,h,angle-(steps/2.0), angle+(steps/2.0), true);
					
		angle = (d.getSeconds()) * steps;		
		angle -= Math.PI * 0.5;
		pen.SetColor(0.75,0.75,0,0.5);
		pen.DrawArc(0,0,w,h,angle-(steps/2.0), angle+(steps/2.0), true);
		
				
		pen.SetColor(0,0,0,1);
		pen.WriteBoxed(Skin.current.ClockFont, 0,0,w,h,Pen.ALIGN_CENTER, Pen.ALIGN_CENTER, d.toLocaleTimeString());
		
	},
	
	StatusBar : function(pen)
	{
		var w = this.width, h = this.height;
				
		var prefs = Skin.current;
		var frame = Frames3D;
		
		pen.Clear();
		
		frame.Outset(pen, 0,0,w,h);
		
		if (this.text!=null)
		{
			pen.SetColor(prefs.TextForeColor);
			pen.WriteBoxed(prefs.Font, frame.OutsetAdjust,frame.OutsetAdjust,w-frame.OutsetAdjust,h-frame.OutsetAdjust,
						   Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
		}		
	},
	
	ResizeKnob : function(pen)
	{
		var w = this.width, h = this.height;
				
		var prefs = Skin.current;
		var frame = Frames3D;
		var adjust, offset;
		var drawfr1;
		
		if (this.is_dragging)
		{
		  offset = 1;
		  adjust = frame.InsetAdjust+3;		
		  drawfr1 = frame.Inset;		  
		}
		else
		{
		  offset = 0;
		  adjust = frame.ValleyAdjust;		
		  drawfr1 = frame.Valley;		  
	 	}
	 	
		pen.Clear();
		
		drawfr1(pen, 0,0,w,h);
		
		var x,y;
		
		pen.SetColor(0.35,0.35,0.35,1);
		pen.SwapColors();
		pen.SetColor(0.85,0.85,0.85,1);
		
		for(x=adjust; x<w-adjust; x+=3)
		{
			for(y=adjust; y<h-adjust; y+=3)
			{								
				pen.DrawRect(x+offset,y+offset,x+1+offset,y+1+offset, false, true);
			}
		}				
		
	},
	
	Window : function(pen)
	{
		var w = this.width, h = this.height;
				
		var prefs = Skin.current;
		var frame = Frames3D;
		
		pen.Clear();
		
		frame.Ridge(pen, 0,0,w,h);						
	},
	
	ToolTip : function(pen)
	{
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
				
		pen.Clear();
		
		var start_y, arrow_x=this.fx;
		
		// If we are below the item we are highlighting...
		if (!this.over)
		{		
			start_y=10;
			
			// Fill
			pen.SetColor(prefs.TextBackColor);
			pen.DrawRect(0,start_y, w, h, true);
			pen.DrawTriangle(arrow_x, 0, arrow_x+10, start_y, arrow_x-10, start_y, true);
			
			// Decoration
			pen.SetColor(prefs.ActiveTitleBarColor1);
			pen.DrawRect(3,start_y+3, 8, h-3, true);
					
			// Border
			pen.SetWidth(1.5);
			pen.SetColor(prefs.TextForeColor);		
			pen.DrawLine(0,start_y, arrow_x-10, start_y);
			pen.DrawLine(arrow_x-10, start_y, arrow_x, 0);
			pen.DrawLine(arrow_x, 0, arrow_x+10, start_y);
			pen.DrawLine(arrow_x+10, start_y, w, start_y);
			pen.DrawLine(w, start_y, w,h);
			pen.DrawLine(w, h, 0,h);
			pen.DrawLine(0,h,0,start_y);
		}
		else
		{
			start_y=h-10;
				
			// Fill
			pen.SetColor(prefs.TextBackColor);
			pen.DrawRect(0,0, w, start_y, true);
			pen.DrawTriangle(arrow_x, h, arrow_x-10, start_y, arrow_x+10, start_y, true);
			
			// Decoration
			pen.SetColor(prefs.ActiveTitleBarColor1);
			pen.DrawRect(3, 3, 8, start_y-3, true);
					
			// Border
			pen.SetWidth(1.5);
			pen.SetColor(prefs.TextForeColor);		
			pen.DrawLine(0,0, w, 0);
			pen.DrawLine(w, 0, w, start_y);
			pen.DrawLine(w, start_y, arrow_x+10, start_y);
			pen.DrawLine(arrow_x+10, start_y, arrow_x, h);
			pen.DrawLine(arrow_x, h, arrow_x-10,start_y);
			pen.DrawLine(arrow_x-10,start_y, 0,start_y);
			pen.DrawLine(0,start_y,0,0);
			
			start_y=0;
		}
					
		pen.Write(prefs.TitleFont, 10, start_y+5, this.title);
		
		// Setup for the information.
		var ty=start_y+5+this.title_size.height;
		
		for (var line in this.info)
		{						
			pen.Write(prefs.Font, 15, ty+5, this.info[line]);
			
			ty+=prefs.Font.GetTextHeight();
		}		
	},
	
	CheckBox : function(pen)
	{
		
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
						
		pen.Clear();
		
		if (this.over) pen.SetColor(prefs.CheckBox.OverColor);
		else 		   pen.SetColor(prefs.TextBackColor);
		pen.DrawRect(2,1,prefs.CheckBox.w-1, prefs.CheckBox.h-2, true);		
		
		if (!this._active)
		{
			pen.SetColor(prefs.HighlightColor);
			pen.SwapColors();
			pen.SetColor(prefs.ShadowColor);
			
			pen.DrawRect(1,1,prefs.CheckBox.w-1, prefs.CheckBox.h-1, false, true);
				
			pen.SetColor(0,0,0,1);
			pen.DrawRect(0,0,prefs.CheckBox.w, prefs.CheckBox.h, false);					
		}

		pen.SetColor(0,0,0,1);
		
		if (this.state)
		{			
			pen.SetWidth(2.0);
			
			pen.DrawLine(4, 4, prefs.CheckBox.w-4, prefs.CheckBox.h-4); 
			pen.DrawLine(prefs.CheckBox.w-4, 4, 4, prefs.CheckBox.h-4);
		}
						
		pen.WriteBoxed(prefs.Font, prefs.CheckBox.w+5, 0, w, h, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
	},
	
	RadioButton : function(pen)
	{
		
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
						
		pen.Clear();
		
		if (this.over) pen.SetColor(prefs.CheckBox.OverColor);
		else 		   pen.SetColor(prefs.TextBackColor);
		pen.DrawMiteredRect(2,1,prefs.CheckBox.w-1, prefs.CheckBox.h-2,0.5, true);		
		
		if (!this._active)
		{
			pen.SetColor(prefs.HighlightColor);
			pen.SwapColors();
			pen.SetColor(prefs.ShadowColor);
			
			pen.DrawMiteredRect(1,1,prefs.CheckBox.w-1, prefs.CheckBox.h-1, 0.5,false, true);
				
			pen.SetColor(0,0,0,1);
			pen.DrawMiteredRect(0,0,prefs.CheckBox.w, prefs.CheckBox.h, 0.5,false);					
		}

		pen.SetColor(0,0,0,1);
		
		if (this.state)
		{			
			pen.SetWidth(2.0);
			
			pen.DrawMiteredRect(4, 4, prefs.CheckBox.w-4, prefs.CheckBox.h-4, 0.5,true); 			
		}
						
		pen.WriteBoxed(prefs.Font, prefs.CheckBox.w+5, 0, w, h, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
	}		
	
	
	
}; // end Style3D