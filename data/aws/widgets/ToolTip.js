/** ToolTip factory. */
function ToolTip(title, text)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);	
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("text", function(t) { this._text = t; this.AdjustSizeForText(); this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("text", function() { return this._text; });			
		
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = prefs.Style.ToolTip;
	
	// Set the default size.
	_widget.ResizeTo(prefs.ScrollBarWidth, prefs.ScrollBarHeight);
	
	//  Set's the widget's focus point so that the widget will know where
	// to place itself.
	_widget.SetFocusPoint = function(x,y)
	{
				
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
	}
	
	
	// Setup needed info
	_widget.title=title;
	_widget.text=text;
		
	return _widget;
}
