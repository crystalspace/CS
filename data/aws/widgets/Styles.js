
// Calculates the nearest power of two larger than 'value', returns an integer.
function NearestPow2(value)
{
  var rv = Math.floor(Math.LOG2E * Math.log(value));						
	  rv = Math.pow(2,(rv+1));	
	  
  return rv;
}



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
				
		pen.SetFlag(Pen.FLAG_FILL);
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
		
		// Rebuilds the gradient texture we use for the button.
		function rebuildTextures(widget)
		{
			//  The texture height needs to be the power of 2 greater than
			// or equal to the height of the button.
			
			var tw = NearestPow2(w);
			var th = NearestPow2(h);
			
			widget.gr_tex = new Texture(tw, th);
			widget.gr.Render(widget.gr_tex, Gradient.LDIAG);					
		}

		// Initializes the gradient and texture 
		if (this.draw_init == false)
		{
			this.draw_init = true;
			this.gr = tb.BkgGrad;
						
			rebuildTextures(this);
		}
				
		// Draw the background.
		pen.SetColor(tb.Base)		
		pen.SetFlag(Pen.FLAG_TEXTURE);
		pen.SetTexture(this.gr_tex);
		pen.DrawMiteredRect(0,0,w,h,2);
		pen.ClearFlag(Pen.FLAG_TEXTURE);
		
		pen.SetColor(tb.Border)		
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawMiteredRect(0,0,w,h,2);
		
		// Draw Frame for text.
		dim = prefs.TitleFont.GetDimensions(this.text);
		
		var cx = (w-tbw-5)>>1;
		var thw = dim.width>>1;
		
		if (this.is_active) pen.SetColor(tb.Active);
		else				pen.SetColor(tb.Inactive);
		
		pen.SetFlag(Pen.FLAG_FILL);		
		pen.DrawMiteredRect(cx-thw-5, 0, cx+thw+10, dim.height+2, 5);		
		pen.DrawRect(cx-thw-5, 0, cx+thw+10, 5);
		pen.SetColor(tb.Border);
		pen.DrawLine(cx-thw-5, 0, cx-thw-5, dim.height-2);
		pen.DrawLine(cx-thw-5, dim.height-2, cx-thw, dim.height+2);
		pen.DrawLine(cx+thw+10, 0, cx+thw+10, dim.height-2);
		pen.DrawLine(cx+thw+10, dim.height-2, cx+thw+5, dim.height+2);
		pen.DrawLine(cx-thw, dim.height+2, cx+thw+5, dim.height+2);
		
		
		
// 		// Glassy shine
// 		pen.SetFlag(Pen.FLAG_FILL | Pen.FLAG_SWAPCOLORS);
// 		pen.SetColor(1,1,1,0.1);
// 		pen.SwapColors();
// 		pen.SetColor(1,1,1,0.9);
// 		pen.DrawRect(4, 2, w-3, 10);
		
		// Draw the text.		
		pen.SetColor(tb.Text);
		pen.WriteBoxed(prefs.TitleFont, 5, 5, w-tbw-10, h-10, Pen.ALIGN_CENTER, Pen.ALIGN_CENTER, this.text);			
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

		// Rebuilds the gradient texture we use for the button.
		function rebuildTextures(widget)
		{
			//  The texture height needs to be the power of 2 greater than
			// or equal to the height of the button.
			
			var tw = 1;
			var th = NearestPow2(h);
			
			widget.gr_tex = new Texture(tw, th);
			widget.gr.Render(widget.gr_tex, Gradient.VERTICAL);			
			
			//td = widget.gr_tex.GetDimensions();
			//Sys.Print("Button: created texture: ", td.width, "x", td.height, " ", tw, "x", th);		
		}

		// Initializes the gradient and texture 
		if (this.draw_init == false)
		{
			this.draw_init = true;
			this.gr = prefs.OverlayGradient;
			this.gr_over = new Gradient();			

			rebuildTextures(this);
		}
		
		// Make's sure that the texture hasn't gotten too small.
		td = this.gr_tex.GetDimensions();
		if (h > td.height) rebuildTextures(this);
		
		
 		//pen.SetFlag(Pen.FLAG_FILL);
 		//pen.DrawMiteredRect(0,0,w,h,2);
 		
 		// Draw background
 		pen.SetColor(b.Base);				
		pen.SetMixMode(Pen.MIX_ALPHA);
		pen.SetFlag(Pen.FLAG_TEXTURE);					
		pen.SetTexture(this.gr_tex);
		pen.DrawMiteredRect(0,0,w,h,2);
		pen.ClearFlag(Pen.FLAG_TEXTURE);
 							
		if (this.over==true && this.state==false)
		{
			pen.SetColor(b.Over);			
			pen.SetMixMode(Pen.MIX_ALPHA);
			pen.SetFlag(Pen.FLAG_FILL);
 			pen.DrawMiteredRect(0,0,w,h,2);

		}
 		else if (this.over==true && this.state==true)
 		{
	 		pen.SetColor(b.Down);			
			pen.SetMixMode(Pen.MIX_ALPHA);
			pen.SetFlag(Pen.FLAG_FILL);
 			pen.DrawMiteredRect(0,0,w,h,2);
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
		var sb = prefs.StatusBar;
				
		// Rebuilds the gradient texture we use for the button.
		function rebuildTextures(widget)
		{
			//  The texture height needs to be the power of 2 greater than
			// or equal to the height of the button.
			
			var tw = NearestPow2(w);;
			var th = NearestPow2(h);
			
			widget.gr_tex = new Texture(tw, th);
			widget.gr.Render(widget.gr_tex, Gradient.RDIAG);					
		}

		// Initializes the gradient and texture 
		if (this.draw_init == false)
		{
			this.draw_init = true;
			this.gr = sb.BkgGrad;
			
			rebuildTextures(this);
		}
		
		pen.Clear();		

		// Draw the background.
		pen.SetColor(1.0,1.0,1.0,1.0)
		pen.SetFlag(Pen.FLAG_TEXTURE);
		pen.SetTexture(this.gr_tex);
		pen.DrawRect(0,0,w,h);
		pen.ClearFlag(Pen.FLAG_TEXTURE);
		
		pen.SetColor(sb.Border)				
		pen.DrawRect(0,0,w,h);		
		
		if (this.text!=null)
		{
			pen.SetColor(prefs.TextForeColor);
			pen.WriteBoxed(prefs.Font, 2,2,w-2,h-2,
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
		if (!this.over_focus_point)
		{		
			start_y=10;
			
			// Fill
			pen.SetFlag(Pen.FLAG_FILL);
			pen.SetColor(prefs.TextBackColor);
			pen.DrawRect(0,start_y, w, h);
			pen.DrawTriangle(arrow_x, 0, arrow_x+10, start_y, arrow_x-10, start_y);
			
			// Decoration
			if (this.over) pen.SetColor(prefs.TitleBar.Active);
			else 		   pen.SetColor(prefs.TitleBar.Inactive);
			pen.DrawRect(3,start_y+3, 8, h-3);
					
			// Border
			pen.SetWidth(1.5);
			//pen.SetColor(prefs.TextForeColor);		
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
			if (this.over) pen.SetColor(prefs.TitleBar.Active);
			else 		   pen.SetColor(prefs.TitleBar.Inactive);
			pen.DrawRect(3, 3, 8, start_y-3);
					
			// Border
			pen.SetWidth(1.5);
			//pen.SetColor(prefs.TextForeColor);		
			pen.DrawLine(0,0, w, 0);
			pen.DrawLine(w, 0, w, start_y);
			pen.DrawLine(w, start_y, arrow_x+10, start_y);
			pen.DrawLine(arrow_x+10, start_y, arrow_x, h);
			pen.DrawLine(arrow_x, h, arrow_x-10,start_y);
			pen.DrawLine(arrow_x-10,start_y, 0,start_y);
			pen.DrawLine(0,start_y,0,0);
			
			start_y=0;
		}
					
		pen.SetColor(prefs.TextForeColor);		
		pen.Write(prefs.TitleFont, 10, start_y+5, this.title);
		
		// Setup for the information.
		var ty=start_y+5+this.title_size.height;
		
		for (var line in this.info)
		{						
			pen.Write(prefs.Font, 15, ty+5, this.info[line]);
			
			ty+=prefs.Font.GetTextHeight();
		}		
	},
	
	Label : function(pen)
	{
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
				
		pen.Clear();		
		pen.SetColor(prefs.TextForeColor);
		if (this.border==true) pen.DrawRect(0,0,w,h);
		
		// Setup for the information.
		var ty = (this.height - this.text_height) / 2.0;
		
		for (var line in this.info)
		{						
			pen.Write(prefs.Font, 0, ty, this.info[line]);
			
			ty+=prefs.Font.GetTextHeight();
		}		
		
		
	},
	
	Image : function(pen)
	{
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
				
		pen.Clear();				
		
		if (this.image!=null && this.image!=undefined)
		{
			pen.SetFlag(Pen.FLAG_TEXTURE);
			pen.SetColor(0.5,0.5,0.5,1);
			pen.SetTexture(this.image);
			pen.DrawRect(0,0,w,h);
			pen.ClearFlag(Pen.FLAG_TEXTURE);
		}
		
		if (this.border==true)
		{
			pen.SetColor(prefs.TextForeColor);
			pen.DrawRect(0,0,w,h);		
		}		
	},
	
	RoundedPanel : function(pen)
	{
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
		var rp = prefs.RoundedPanel;
				
		pen.Clear();				
		
		pen.SetFlag(Pen.FLAG_FILL);
		pen.SetColor(rp.Base);		
		pen.DrawRoundedRect(0,0,w,h,rp.Roundness);
		pen.ClearFlag(Pen.FLAG_FILL);
				
		if (this.border==true)
		{
			pen.SetColor(rp.Border);
			pen.DrawRoundedRect(0,0,w,h,rp.Roundess);		
		}		
	},
	
	MiteredPanel : function(pen)
	{
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
		var mp = prefs.MiteredPanel;
				
		pen.Clear();				
		
		pen.SetFlag(Pen.FLAG_FILL);
		pen.SetColor(mp.Base);		
		pen.DrawMiteredRect(0,0,w,h,mp.Miter);
		pen.ClearFlag(Pen.FLAG_FILL);
				
		if (this.border==true)
		{
			pen.SetColor(mp.Border);
			pen.DrawMiteredRect(0,0,w,h,mp.Miter);		
		}		
	},
	
	EmptyLayout : function(pen)
	{
		// empty
		return;	
	},
	
	CheckBox : function(pen)
	{					
		pen.Clear();
	
		var w  = this.width, h = this.height;				
		var prefs = Skin.current;
		var cb = prefs.RadioButton;
						
		pen.Clear();
		
		// Rebuilds the gradient texture we use for the checkbox.
		function rebuildTextures(widget)
		{
			//  The texture height needs to be the power of 2 greater than
			// or equal to the height of the button.
			
			var tw = Math.floor(Math.LOG2E * Math.log(w));
			var th = Math.floor(Math.LOG2E * Math.log(h));
			
			tw = Math.pow(2,(tw+1));			
			th = Math.pow(2,(th+1));
			
			widget.gr_tex = new Texture(tw, th);
			widget.gr.Render(widget.gr_tex, Gradient.LDIAG);			
			
			widget.gr_tex_over = new Texture(tw, th);	
			widget.gr_over.Render(widget.gr_tex_over, Gradient.LDIAG);			
		}
		
		// Initializes the gradient and texture 
		if (this.draw_init == false)
		{
			this.draw_init = true;
			this.gr = new Gradient();
			this.gr_over = new Gradient();
			
			this.gr.AddColor(prefs.TextBackColor, 0);	
			this.gr.AddColor(cb.Base, 0.5);					
			this.gr.AddColor(new Color(0.4, 0.4, 0.4, 1.0), 1.0);
						
			this.gr_over.AddColor(prefs.TextBackColor, 0);	
			this.gr_over.AddColor(cb.Over, 0.5);					
			this.gr_over.AddColor(new Color(0.4, 0.4, 0.4, 1.0), 1.0);			
						
			rebuildTextures(this);
		}				
						
		// Fill
		if (this.state)
		{
			pen.SetColor(prefs.TextBackColor);		
			pen.SetFlag(Pen.FLAG_TEXTURE);
			
			if (this.over) pen.SetTexture(this.gr_tex_over);
			else		   pen.SetTexture(this.gr_tex);
		}
		else
		{			
			pen.SetFlag(Pen.FLAG_FILL);	
			
			if (this.over) pen.SetColor(cb.Over);		
			else		   pen.SetColor(prefs.TextBackColor);		
		}
		
		pen.DrawRect(2,1,cb.w-1, cb.h-2);
		pen.ClearFlag(Pen.FLAG_TEXTURE);							
			
		// Border
		pen.SetColor(cb.Border);
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawRect(0,0,cb.w,cb.h);		
								
		pen.SetColor(prefs.TextForeColor);				
		pen.WriteBoxed(prefs.Font, prefs.CheckBox.w+5, 0, w, h, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
	},
	
	RadioButton : function(pen)
	{
		
		var w  = this.width, h = this.height;
						
		var prefs = Skin.current;
		var rb = prefs.RadioButton;
						
		pen.Clear();
		
		// Rebuilds the gradient texture we use for the radiobutton.
		function rebuildTextures(widget)
		{
			//  The texture height needs to be the power of 2 greater than
			// or equal to the height of the button.
			
			var tw = 1;
			var th = Math.floor(Math.LOG2E * Math.log(h));
						
			th = Math.pow(2,(th+1));
			
			widget.gr_tex = new Texture(tw, th);
			widget.gr.Render(widget.gr_tex, Gradient.VERTICAL);						
			
		}
		
		// Initializes the gradient and texture 
		if (this.draw_init == false)
		{
			this.draw_init = true;
			this.gr = rb.BkgGrad;
			this.gr_over = rb.BkgGrad;					
						
			rebuildTextures(this);
		}				
		
		// Border
		pen.SetColor(rb.Border);
		pen.ClearFlag(Pen.FLAG_FILL);
		pen.DrawRect(0,0,rb.w,rb.h);		
		
		if (this.state)
		{		
			pen.SetFlag(Pen.FLAG_TEXTURE);
				
			if (this.over)
			{
				pen.SetColor(rb.Over);
				pen.SetTexture(this.gr_tex);					
			}
			else
			{
				pen.SetColor(rb.Base);
				pen.SetTexture(this.gr_tex);
			}
			
			pen.DrawRect(2,1,rb.w-1, rb.h-2);			
		}
		else
		{
			if (this.over)  pen.SetColor(rb.Over);
			else			pen.SetColor(prefs.TextBackColor);		
			
			pen.SetFlag(Pen.FLAG_FILL);
			pen.DrawRect(2,1,rb.w-1, rb.h-2);			
		}
				
		pen.SetColor(prefs.TextForeColor);				
		pen.WriteBoxed(prefs.Font, prefs.CheckBox.w+5, 0, w, h, Pen.ALIGN_LEFT, Pen.ALIGN_CENTER, this.text);
	},
	
	GaugeLevel : function(pen)
	{
		var w  = this.width, h = this.height;
		var xradius = (w>>1), yradius = h;
								
		var prefs = Skin.current;
		
		var th = prefs.GaugeFont.GetTextHeight() * 1.5;
								
		pen.Clear();
		
		var start_radian = Math.PI;
		var end_radian   = Math.PI * 2.0;
		var cur_value    = this.min;
		var step_size    = ((this.max - this.min) / this.step);
		var tick=0;
		
		step_size = ((end_radian - start_radian) / step_size) * 0.25;
				
		// Draw Green To Red
		pen.SetWidth(5);
		pen.SetColor(1,0,0,1);
		pen.DrawArc(th,th,w-th,(h<<1)-th, end_radian - (Math.PI*0.25), end_radian);
		pen.SetColor(1,1,0,1);
		pen.DrawArc(th,th,w-th,(h<<1)-th, end_radian - (Math.PI*0.5), end_radian - (Math.PI*0.25));
		pen.SetWidth(1);
		
		// Draw tick marks and numbers
		pen.SetColor(prefs.TextForeColor);			
 		for(var rad = start_radian; rad<=end_radian; rad+=step_size, cur_value+=this.step*0.25)
 		{
 			var x = Math.cos(rad),
 			    y = Math.sin(rad);
 			    
 			++tick;
 			
 			var len;
 						
 			if (tick==4 || cur_value>=this.max || cur_value<=this.min)
 			{
	 			var tx = (x*(xradius))+xradius,
	 				ty = (y*(yradius))+yradius;
	 				
	 			// Center the text on the point.
	 			var dim = prefs.GaugeFont.GetDimensions(cur_value);
	 			
	 			ty-=(dim.height>>1);
	 			if (ty<0) ty=0;
	 			
	 			tx-=(dim.width>>1);
	 			if (tx<0) tx=0;
	 				
	 			pen.Write(prefs.GaugeFont, tx, ty, Math.floor(cur_value));
	 			
	 			len=10;
	 			tick=0;
 			}
 			else
 			{
	 			len=5;	
 			}
 			
 			pen.DrawLine((x*(xradius-th))+xradius, (y*(yradius-th))+yradius, 
 						 (x*(xradius-len-th))+xradius, (y*(yradius-len-th))+yradius);			 				
 			
 		}
		
		// Figure out where the needle goes.
		// cur_angle/total_radians = value/total_values    cur_angle = (value * total_radians) / total_values
		var cur_angle = ((this.value * Math.PI) / (this.max - this.min)) - Math.PI;
		var nleft  =  cur_angle - (Math.PI * 0.5);
		var nright = cur_angle + (Math.PI * 0.5);
				
		var x1=Math.floor(Math.cos(cur_angle) * (xradius-th)) + xradius,
			y1=Math.floor(Math.sin(cur_angle) * (yradius-th)) + yradius,
			x2=Math.floor(Math.cos(nright) * 2.5) + xradius,
			y2=Math.floor(Math.sin(nright) * 2.5) + yradius,
			x3=Math.floor(Math.cos(nleft) * 2.5)  + xradius,
			y3=Math.floor(Math.sin(nleft) * 2.5)  + yradius;
										
		//Sys.Print("Gauge angle: ", cur_angle, " ", x1, ",", y1, " ", x2, ",", y2, " ", x3, ",", y3, " ");		
			
		
		//pen.DrawLine(x1,y1,xradius,yradius);
						 
		pen.DrawArc(th,th,w-th,(h<<1)-th, start_radian, end_radian);										
		
		// Draw the needle.
		pen.SetFlag(Pen.FLAG_FILL);					 
		pen.DrawTriangle(x1,y1,x2,y2,x3,y3);
		
		pen.WriteBoxed(prefs.Font, 0, 0, w, h, Pen.ALIGN_CENTER, Pen.ALIGN_BOT, this.title);		
	},
	
	GaugeHorzBar : function(pen)
	{
		var w  = this.width, h = this.height;
		var step_size = (w / this.step);
		var tick=0;
		var cur_value = this.min;
		var prefs = Skin.current;
				
		var th = prefs.GaugeFont.GetTextHeight();
		var bar_y = h-th-2;
								
		pen.Clear();
		
		// Rebuilds the gradient texture we use for the button.
		function rebuildTextures(widget)
		{
			//  The texture height needs to be the power of 2 greater than
			// or equal to the height of the button.
			
			var th = 1;
			var tw = Math.floor(Math.LOG2E * Math.log(w));
						
			tw = Math.pow(2,(tw+1));
			
			widget.gr_tex = new Texture(tw, th);
			widget.gr.Render(widget.gr_tex, Gradient.HORIZONTAL);						
		}

		// Initializes the gradient and texture 
		if (this.draw_init == false)
		{
			this.draw_init = true;
			this.gr = new Gradient();			
			
			this.gr.AddColor(new Color(0, 0, 1, 1), 0);
			this.gr.AddColor(new Color(0, 1, 0, 1), 0.25);
			this.gr.AddColor(new Color(1, 1, 0, 1), 0.65);
			this.gr.AddColor(new Color(1, 0, 0, 1), 1.0);			
						
			rebuildTextures(this);
		}
		
		pen.SetColor(prefs.TextForeColor);
		
		// Gauge line
		pen.DrawLine(0,bar_y, w, bar_y);		
		
		// Tick marks
		for(var x=0; x<w; x+=step_size)
		{
			++tick;
			var len;
			
			if (tick==4 || cur_value<=this.min)
			{
				var tx=x;				
				
				len=th;
				tick=0;
				
				// Center the text on the point.
	 			var dim = prefs.GaugeFont.GetDimensions(cur_value);
	 				 			
	 			tx-=(dim.width>>1);
	 			if (tx<0) tx=0;
	 			
	 			pen.Write(prefs.GaugeFont, tx, h-th, Math.floor(cur_value));				
	 			
	 			cur_value+=this.step;
			}
			else
			{
				len=4;	
			}
			
			pen.DrawLine(x, bar_y, x, bar_y+len);						
			
		}
		
		var bw = (w * (this.value-this.min)) / (this.max - this.min);
		
		if (bw<0) bw=0;
		if (bw>w) bw=w;
		
		pen.SetFlag(Pen.FLAG_TEXTURE);
		pen.SetTexture(this.gr_tex);
		pen.SetColor(1,1,1,1);
		pen.DrawRect(0,0,w,bar_y-2, 3);
		
		pen.SetColor(0,0,0,0.8);
		pen.ClearFlag(Pen.FLAG_TEXTURE);
		pen.SetFlag(Pen.FLAG_FILL);
		pen.DrawTriangle(bw, 0, bw+5, 0, bw, bar_y);
						
	},
		
	Slider : function(pen)
	{
		var w = this.width, h = this.height;
				
		var prefs = Skin.current;
		var slb = prefs.Slider;
				
		var pos = this.value;
		var size = slb.Thickness;
		var max  = this.max;
		
		var btn_w, btn_h;		
				
		pen.Clear();
		
		function createTextures(widget)
		{			
			var tx_pen = new Pen();			
			var w = widget.ButtonScroll.x2-widget.ButtonScroll.x1;
			var h = widget.ButtonScroll.y2-widget.ButtonScroll.y1;
			var cy = size>>1;
			
			var tw = Math.floor(Math.LOG2E * Math.log(w));
			var th = Math.floor(Math.LOG2E * Math.log(h));
			
			tw = Math.pow(2,(tw+1));			
			th = Math.pow(2,(th+1));
			
			var ButtonScroll = { x1:1, y1:0, x2:tw-slb.TickThickness, y2:th-1 };
			
			widget.tx_knob = new Texture(tw, th);
			widget.gr_tex  = new Texture(1, th);
			widget.gr.Render(widget.gr_tex, Gradient.VERTICAL);
			
			Sys.Print("Slider: created texture: ", tw, "x", th);
									
			// Draw the knob
 			tx_pen.SetFlag(Pen.FLAG_FILL);			
			tx_pen.SetColor(prefs.FillColor);			  			
			tx_pen.DrawRect(ButtonScroll.x1+5, ButtonScroll.y1,
						 ButtonScroll.x2-10, ButtonScroll.y2);
						 
			tx_pen.DrawTriangle(ButtonScroll.x2-10, ButtonScroll.y1,
							 ButtonScroll.x2, cy,
							 ButtonScroll.x2-10, ButtonScroll.y2);							 			
			
			tx_pen.SetColor(slb.Base);			 
			tx_pen.DrawArc(ButtonScroll.x1, ButtonScroll.y1,
						 ButtonScroll.x1+10, ButtonScroll.y1+10, Math.PI, Math.PI*1.5);
						 
			tx_pen.DrawArc(ButtonScroll.x1, ButtonScroll.y2-10,
						 ButtonScroll.x1+10, ButtonScroll.y2, Math.PI*0.5, Math.PI);
						 
			tx_pen.DrawRect(ButtonScroll.x1, ButtonScroll.y1+5,
						 ButtonScroll.x1+5, ButtonScroll.y2-5);
						
			tx_pen.DrawLine(ButtonScroll.x1+5, ButtonScroll.y1,
						 ButtonScroll.x2-10, ButtonScroll.y1);						 
						 
			tx_pen.DrawLine(ButtonScroll.x1+5, ButtonScroll.y2,
						 ButtonScroll.x2-10, ButtonScroll.y2);						 
						 
			tx_pen.DrawLine(ButtonScroll.x2-10, ButtonScroll.y1,
						 ButtonScroll.x2, cy);
						 
			tx_pen.DrawLine(ButtonScroll.x2-10, ButtonScroll.y2,
						 ButtonScroll.x2, cy);
								 
 			
			// Gradient
 			tx_pen.SetFlag(Pen.FLAG_TEXTURE);
  			tx_pen.SetTexture(widget.gr_tex); 			
   			tx_pen.SetColor(0.25,0.25,0.25,1.0);
   			tx_pen.SetMixMode(Pen.MIX_DST_ALPHA_ADD);
   			tx_pen.DrawRect(0,0,tw,th);	
   			tx_pen.ClearFlag(Pen.FLAG_TEXTURE);
   									 			
			tx_pen.Render(widget.tx_knob);			
		}
								
		
		//  value      start_pos
		// ------- =  -----------
		//  max         height (or width)
		
		if (this.orientation_vertical)
		{
			var y = ((pos*(h-size)) / (this.max-this.min)) + (size>>1);
			var step_pix = ((h-size)*this.tick_step) / (this.max-this.min);
									
			// Setup button areas for scrollbar controller.			
			this.ButtonScroll = { x1:0, y1:y-(size>>1), x2:w-slb.TickThickness, y2:y+(size>>1) };
			
			if (this.init_draw == false)
			{
				this.init_draw = true;
				this.gr = prefs.OverlayGradient;
				
				createTextures(this);					
			}	
			
			
			pen.SetColor(prefs.ShadowColor);
			pen.SwapColors();
			pen.SetColor(prefs.HighlightColor);
			pen.SetFlag(Pen.FLAG_SWAPCOLORS);
			pen.DrawRoundedRect((w>>1)-2, 0, (w>>1)+2, h, 2);
			pen.ClearFlag(Pen.FLAG_SWAPCOLORS);
			pen.SetColor(prefs.TextForeColor);			
						
			for(var ty=(size>>1); ty<=(h-(size>>1)); ty+=step_pix)
			{
				pen.DrawLine(w-slb.TickThickness, ty, w, ty);									
			}		
			
			pen.PushTransform();
			pen.Translate(0, this.ButtonScroll.y1, 0);
			
 			pen.SetColor(1,1,1,1);
// 			pen.DrawRect(0,0,this.ButtonScroll.x2-this.ButtonScroll.x1, 
// 							 this.ButtonScroll.y2-this.ButtonScroll.y1);				
			
			pen.SetFlag(Pen.FLAG_TEXTURE);			
			pen.SetTexture(this.tx_knob);
			
			pen.DrawRect(0,0,w, //this.ButtonScroll.x2-this.ButtonScroll.x1, 
							 1+this.ButtonScroll.y2-this.ButtonScroll.y1);
			pen.PopTransform();
			pen.ClearFlag(Pen.FLAG_TEXTURE);			 
			
						
		
		}
		else
		{
			var x = ((pos*(w-size)) / (this.max-this.min)) + (size>>1);
			
			// Setup button areas for scrollbar controller.
			this.ButtonScroll = { x1:x, y1:0, x2:x+size, y2:h };
			
			pen.SetColor(prefs.ShadowColor);
			pen.SwapColors();
			pen.SetColor(prefs.HighlightColor);
			pen.SetFlag(Pen.FLAG_SWAPCOLORS);
			pen.DrawRoundedRect(0, (h>>1)-2, w, (h>>1)+2, 2);
			pen.ClearFlag(Pen.FLAG_SWAPCOLORS);
						
		}
	},
	
	
	
}; // end Style3D