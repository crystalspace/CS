/** Label factory. */
function Label(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);	
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("text", function(t) { this._text = t; this.AdjustSizeForText(); this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("text", function() { return this._text; });			
			
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = prefs.Style.Label;
	
	// Set the default size.
	_widget.ResizeTo(prefs.ScrollBarWidth, prefs.ScrollBarHeight);
		
	//Adjust's the size of the label
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
						
		this.ResizeTo(mw, th);	
		
		this.text_height = th;
		this.text_width  = mw;
	}
	
	
	// Setup needed info	
	_widget.text=settings.text;
	_widget.border=settings.border;
		
	return _widget;
}
