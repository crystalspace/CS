/******* Frame Styles *********
 *
 *  This file is automatically included
 * so that widgets can use these "predefined"
 * styles. 
 */

Frames = {  
	// Draw a square border
	Rect : function (pen)
	{
		prefs = Skin.current
		
		pen.SetColor(prefs["Border"]);
		pen.DrawRect(0, 0, this.width, this.height, false);
		
		pen.SetColor(prefs["Fill"]);
		pen.DrawRect(0, 0, this.width, this.height, true);
	},
	
	// Draw a round border
	RoundedRect : function (pen)
	{
		prefs = Skin.current
		
		pen.SetColor(prefs["Fill"]);
		pen.DrawRoundRect(0, 0, this.width, this.height, prefs["Roundness"],  true);
		
		pen.SetColor(prefs["Border"]);
		pen.DrawRoundRect(0, 0, this.width, this.height, prefs["Roundness"], false);
		
		
	}
};

Style3D = 
{
	TitleBar : function(pen)
	{
		prefs = Skin.current;
		
		var w = this.width, h = this.height;
		var cw = w * prefs.TitleBarRoundness, aw=cw*2.0;		
		
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
		
		pen.SetColor(prefs.HighlightColor);		
		pen.DrawArc(1,1,aw,h, Math.PI, Math.PI*1.6, false);
		pen.DrawLine(1,h>>1,1,h);
		pen.DrawLine(cw,1,w-cw,1);
		
		pen.SetColor(prefs.ShadowColor);
		pen.DrawArc(w-aw,1,w,h, Math.PI*1.5, Math.PI*2.1, false);
		pen.DrawLine(w-1,h>>1,w-1,h);
		pen.DrawLine(0,h-1,w,h-1);
		
	}	
	
	
};