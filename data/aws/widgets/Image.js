/** Image factory. */
function Image(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);	
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("image", function(t) { this._image = t; this.AdjustSizeForImage(); this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("image", function() { return this._image; });			
			
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = prefs.Style.Image;
	
	// Set the default size.
	_widget.ResizeTo(prefs.ScrollBarWidth, prefs.ScrollBarHeight);
		
	//Adjust's the size of the label
	_widget.AdjustSizeForImage = function()
	{
		var dim = this._image.GetDimensions();
									
		this.ResizeTo(dim.width, dim.height);		
	}	
	
	// Setup needed info	
	if (settings.image!=undefined && settings.image!=null)
	{
		if (typeof(settings.image)=="object")
		{
			_widget.image=settings.image;
		}
		else
		{
			_widget.image = new Texture(String(settings.image));	
		}
	}
	 
	if (settings.border!=undefined)
		_widget.border=Boolean(settings.border);
	else
		_widget.border=false;
		
	return _widget;
}
