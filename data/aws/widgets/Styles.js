

Style3D = 
{
	WindowMin : function(pen)
	{
		var prefs = Skin.current;
			
		if (this.over) pen.SetColor(0,0,1,1);
		else pen.SetColor(0,0,0,1);
		
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawRect(5, this.height-10, 15, this.height-5);
	},
	
	WindowZoom : function(pen)
	{
		var prefs = Skin.current;
		var cx = this.width>>1, cy=this.height>>1;
		var qw = this.width>>2, qh=this.height>>2;
		
		if (this.over) pen.SetColor(0,1,0,1);
		else pen.SetColor(0,0,0,1);
				
		pen.SetFlag(pen.FLAG_FILL);
		pen.DrawRect(cx-qw, cy-2, cx+qw, cy+2);
		pen.DrawRect(cx-2, cy-qh, cx+2, cy+qh);
	},
	
	WindowClose : function(pen)
	{
		var prefs = Skin.current;
		var x=5, y;
		
		if (this.over) pen.SetColor(1,0,0,1);
		else pen.SetColor(0,0,0,1);
		
		pen.SetFlag(Pen.FLAG_FILL);
		
		// Left to right line
		pen.DrawTriangle(6,4, this.width-4, this.height-6, 4, 6);
		pen.DrawTriangle(4,6, this.width-4, this.height-6, this.width-6, this.height-4);
		
		// Right to left line
		pen.DrawTriangle(this.width-6,4, 6, this.height-4, 4, this.height-6);
		pen.DrawTriangle(this.width-6,4, this.width-4, 6, 6, this.height-4);
	},
	
	TitleBar : function(pen)
	{
		var prefs = Skin.current;
		var tb = prefs.TitleBar;
				
		var w = this.width, h = this.height;
		var tbw = prefs.WindowMin.w + prefs.WindowZoom.w + prefs.WindowClose.w + 10;
		
		pen.Clear();
		
		// Draw the background.
		pen.SetColor(tb.Base)
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawMiteredRect(0,0,w,h,5);
		
		pen.SetColor(0,0,0,0.4)
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawMiteredRect(0,0,w,h,5);
		pen.SetColor(0,0,0,0.3)
		pen.DrawMiteredRect(1,1,w-1,h-1,5);
				
		dim = prefs.TitleFont.GetDimensions(this.text);
		
		var cx = (w-tbw-5)>>1;
		var thw = dim.width>>1;
		
		pen.SetColor(tb.Active);
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawMiteredRect(cx-thw-5, 0, cx+thw+15, dim.height+2, 5);
		pen.DrawRect(cx-thw-5, 0, cx+thw+15, 5);
		
		// Glassy shine
		pen.SetFlag(Pen.FLAG_FILL | Pen.FLAG_SWAPCOLORS);
		pen.SetColor(1,1,1,0.1);
		pen.SwapColors();
		pen.SetColor(1,1,1,0.9);
		pen.DrawRect(7, 2, w-7, 10);
		
		// Draw the text.		
		pen.SetColor(tb.Text);
		pen.WriteBoxed(prefs.TitleFont, 5, 5, w-tbw-5, h-5, Pen.ALIGN_CENTER, Pen.ALIGN_CENTER, this.text);	
		
		pen.ClearFlag(Pen.FLAG_FILL | Pen.FLAG_SWAPCOLORS);		
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
			pen.SetFlag(Pen.FLAG_FILL);						
			arrowPre(this.dec_button_down); pen.DrawTriangle(btn_w*0.5, tri_qh, btn_w-tri_qw, btn_h-tri_qh, tri_qw, btn_h-tri_qh);   arrowPost(this.dec_button_down); 
			arrowPre(this.inc_button_down); pen.DrawTriangle(btn_w*0.5, tri_qh*5, tri_qw, btn_h+tri_qh, btn_w-tri_qw, btn_h+tri_qh); arrowPost(this.inc_button_down); 			
			
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
			pen.SetFlag(Pen.FLAG_FILL);
			arrowPre(this.dec_button_down); pen.DrawTriangle(tri_qw, btn_h*0.5,  btn_w-tri_qw, tri_qh, btn_w-tri_qw, btn_h-tri_qh);	arrowPost(this.dec_button_down); 		
			arrowPre(this.inc_button_down); pen.DrawTriangle(tri_qw*5, btn_h*0.5, btn_w+tri_qw, btn_h-tri_qh, btn_w+tri_qw, tri_qh);  arrowPost(this.inc_button_down); 
		}
	},
	
	Button : function(pen)
	{
		var w = this.width, h = this.height;
		var prefs = Skin.current;
		var b = prefs.Button;
		
		pen.Clear();
		
// 		if (this.state)
// 		{
// 			pen.PushTransform();
// 			pen.Translate(1,1,0);
// 		}
							
		pen.SetColor(b.Base);
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawMiteredRect(1,0,w,h-1,2);
		
		if(!this.state)
		{
			//pen.SetWidth(2);
			for(var i=1, a=0.40; i<8; ++i, a-=0.08)
			{
				pen.SetColor(0,0,0,a);
				pen.DrawLine(i, i, w-i,i);	
				pen.DrawLine(i, h-i, w-i,h-i);
				pen.DrawLine(i, i-1, i,h-i-1);
				pen.DrawLine(w-i, i-1, w-i,h-i);
			}		
			//pen.SetWidth(1);
		}
		
		for(var i=2, a=1; i<9; ++i, a-=0.1)
		{
			var adjust=6-i;
			if (adjust<3) adjust=3;
			
			pen.SetColor(1,1,1,a);
			pen.DrawLine(adjust,i,w-adjust,i);
		}
				
		if (this.over)
		{	
			pen.SetColor(1,1,1,0.25);			
			pen.DrawMiteredRect(0,0,w,h,2);
		}
		
		if (!this.state)
		{
			pen.SetColor(0,0,0,0.25);
			pen.DrawLine(2,h,w,h);
			pen.DrawLine(w,3,w,h);			
			pen.SetWidth(1);
		}	
		
		if (this.onDrawContent)
		{
					
			this.onDrawContent(pen);			
				
		}
		
		pen.SetColor(b.Border);
		pen.ClearFlag(Pen.FLAG_FILL);
 		pen.DrawMiteredRect(0,0,w,h,2,false);
		
// 		if (this.state) pen.PopTransform();
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
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawArc(0,0,w,h,0,Math.PI*2.1);
		
		pen.SwapColors();		
		pen.SetColor(1,1,1,1);
		pen.SetWidth(5);						
		pen.DrawArc(2,2,w-2,h-2,0,Math.PI*2.1);				
		pen.SetColor(0.5,0.5,0.5,1);				
		pen.DrawArc(5,5,w-5,h-5,0,Math.PI*2.1);				
		pen.SetColor(1,1,1,1);
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawArc(10,10,w-10,h-10,0,Math.PI*2.1);
		pen.SetWidth(2);
		pen.SwapColors();
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawArc(15,15,w-15,h-15,0,Math.PI*2.1);
		
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
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawArc(cx-r,cy-r,cx+r,cy+r,angle-(steps/2.0), angle+(steps/2.0));
		
		steps = Math.PI/30.0;
		angle = (d.getMinutes()) * steps;		
		angle -= Math.PI * 0.5;
		pen.SetColor(0.75,0,0,0.5);
		pen.DrawArc(0,0,w,h,angle-(steps/2.0), angle+(steps/2.0));
					
		angle = (d.getSeconds()) * steps;		
		angle -= Math.PI * 0.5;
		pen.SetColor(0.75,0.75,0,0.5);
		pen.DrawArc(0,0,w,h,angle-(steps/2.0), angle+(steps/2.0));
		
				
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
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.SetFlag(Pen.FLAG_SWAPCOLORS);
		
		for(x=adjust; x<w-adjust; x+=3)
		{
			for(y=adjust; y<h-adjust; y+=3)
			{								
				pen.DrawRect(x+offset,y+offset,x+1+offset,y+1+offset);
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
			pen.SetFlag(Pen.FLAG_FILL);
			pen.SetColor(prefs.TextBackColor);
			pen.DrawRect(0,start_y, w, h);
			pen.DrawTriangle(arrow_x, 0, arrow_x+10, start_y, arrow_x-10, start_y);
			
			// Decoration
			pen.SetColor(prefs.ActiveTitleBarColor1);
			pen.DrawRect(3,start_y+3, 8, h-3);
					
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
			pen.SetFlag(Pen.FLAG_FILL);
			pen.DrawRect(0,0, w, start_y);
			pen.DrawTriangle(arrow_x, h, arrow_x-10, start_y, arrow_x+10, start_y);
			
			// Decoration
			pen.SetColor(prefs.ActiveTitleBarColor1);
			pen.DrawRect(3, 3, 8, start_y-3);
					
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
		
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
		var cb = prefs.RadioButton;
						
		pen.Clear();
						
		// Border
		pen.SetColor(cb.Border);
		pen.DrawRect(0,0,cb.w,cb.h);		
		
		// Filling		
		pen.SetColor(prefs.TextBackColor);		
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawRect(2,1,cb.w-1, cb.h-2);	
						
		pen.SetColor(0,0,0,1);

		if (this.state)
		{			
			var hh = cb.h>>1;			
			pen.SetWidth(1.25);
			pen.DrawLine(cb.w-3, 3, 8, cb.h-4); 			
			pen.SetWidth(1);
			pen.DrawLine(8, cb.h-4, 4, cb.h-9);
		}		
			
		if (this.over)
		{
			pen.ClearFlag(Pen.FLAG_FILL);
			
			for(i=0; i<5; ++i)
			{
				pen.SetColor(1.0,1.0,0.74,(5-i)/10)
				pen.DrawRect(i,i,cb.w-i,cb.h-i);	
			}	
		}
						
		pen.SetColor(prefs.TextForeColor);				
		pen.WriteBoxed(prefs.Font, prefs.CheckBox.w+5, 0, w, h, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
	},
	
	RadioButton : function(pen)
	{
		
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
		var rb = prefs.RadioButton;
						
		pen.Clear();
						
		
		// Border
		pen.SetColor(rb.Border);
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawRect(0,0,rb.w,rb.h);		
		
		if (this.state)
		{			
			var hh = rb.h>>1;
			
			pen.SetColor(rb.Base);
			pen.SetFlag(Pen.FLAG_FILL);
			pen.DrawRect(2, 1, rb.w-1, rb.h-2); 			
			pen.SetColor(1,1,1,0.5);
			pen.DrawRect(2, 1, rb.w-1, hh-2);
			pen.DrawArc(3, hh+2, rb.w-3, rb.h-2, Math.PI, Math.PI*2);
			pen.SetColor(0,0,0,0.25);
			pen.DrawLine(2, hh, rb.w-1, hh);
		}
		else
		{
			// Filling		
			pen.SetColor(prefs.TextBackColor);		
			pen.SetFlag(Pen.FLAG_FILL);
			pen.DrawRect(2,1,rb.w-1, rb.h-2);			
		}
		
		if (this.over)
		{
			pen.ClearFlag(Pen.FLAG_FILL);
			for(i=0; i<5; ++i)
			{
				pen.SetColor(1.0,1.0,0.74,(5-i)/10)
				pen.DrawRect(i,i,rb.w-i,rb.h-i);	
			}	
		}
		
		pen.SetColor(prefs.TextForeColor);				
		pen.WriteBoxed(prefs.Font, prefs.CheckBox.w+5, 0, w, h, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
	}		
	
	
	
}; // end Style3D