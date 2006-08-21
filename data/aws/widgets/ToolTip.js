/** ToolTip factory. */
function ToolTip(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);	
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("text", function(t) { this._text = t; this.AdjustSizeForText(); this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("text", function() { return this._text; });			
	
	// Whether we are over the focus point, or under it.
	_widget.over_focus_point=false;
	
	// If mouse is over.
	_widget.over=false;
	
	// The place to point to.
	_widget.fx = 30;
	_widget.fy = 0;
		
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = prefs.Style.ToolTip;
	
	// Set the default size.
	_widget.ResizeTo(prefs.ScrollBarWidth, prefs.ScrollBarHeight);
	
	// Catch mouse enter event
	_widget.onMouseEnter = function()
	{
		this.over=true;
		this.Invalidate();	
	}
	
	// Catch mouse exit event
	_widget.onMouseExit = function()
	{
		this.over=false;
		this.Invalidate();	
	}
	
	//  Set's the widget's focus point so that the widget will know where
	// to place itself.
	_widget.SetFocusPoint = function(x,y)
	{
		// We want the arrow to point to x,y.  So we need to make sure that our 
		// width and height aren't going to cause us to be offscreen.
		
		// We assume that our xmin and ymin properties represent screen coordinates,
		// which they will so long as we aren't a child of anyone.
		
		this.MoveTo(x-30, y);
		this.over_focus_point=false;
		
		// First make sure we're not off screen already.
		
		if (this.xmax > Sys.GetWidth())
			this.Move(Sys.GetWidth() - this.xmax, 0);
			
		if (this.xmin < 0)
			this.Move(-this.xmin, 0);
		
		if (this.ymin < 0)
			this.Move(0,  -this.ymin);
			
		if (this.ymax > Sys.GetHeight())
		{
			this.over_focus_point=true;
			this.MoveTo(this.xmin, y-this.height);	
		}		
		
				
		this.fx = x - this.xmin;
		this.fy = y;
		
		this.Invalidate();
	}
	
	//Adjust's the size of the tooltip
	_widget.AdjustSizeForText = function()
	{
		var fnt = Skin.current.Font;		
		var th=0, mw=0, dim;
		var tmp = new String(this._text);
		
		this.info = tmp.split("\n");
						
		for (var line in this.info)
		{						
			dim = fnt.GetDimensions(this.info[line]);
			
			th+=dim.height;
			if (dim.width>mw) mw=dim.width;							
		}
				
		this.title_size = Skin.current.TitleFont.GetDimensions(this.title);
		th+=this.title_size.height;
						
		this.ResizeTo(mw+20, th+20);
		
		this.SetFocusPoint(this.fx, this.fy);		
	}
	
	
	// Setup needed info
	_widget.title=settings.title;
	_widget.text=settings.text;
		
	return _widget;
}
