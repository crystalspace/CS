/** Button factory. */
function Button(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
			
	// Setup the button
	_widget._state=false;
	_widget.over=false;
	_widget.draw_init=false;
		
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("state", function(v) { this._state = v; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("state", function() { return this._state; });	
	
	// Set the initial width and height
	_widget.Resize(prefs.Button.w, prefs.Button.h);	
			
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.Button;
	
	// Set the content drawing function.
	_widget.onDrawContent = null;
	
	// If we get a mouse down, change the button's state.
	_widget.onMouseDown = function(buttons, x, y)
	{
		this.state=true;
		this._active=true;
		this.CaptureMouse();		
	}
	
	// If the mouse is up, change the state.
	_widget.onMouseUp = function(buttons, x, y)
	{
		this.state=false;
		this._active=false;	
		this.ReleaseMouse();		
	}
	
	_widget.onMouseEnter = function()
	{
		this.over=true;
		this.Invalidate();	
	}
	
	_widget.onMouseExit = function()
	{
		this.over=false;
		this.Invalidate();	
	}
		
	return _widget;
}

function ButtonWithText(settings)
{
	var _widget = Button(settings);
	var dim;
	
	_widget.text = settings.text;	
	
	if (settings.padding!=undefined) _widget.padding=Number(settings.padding); 
	else _widget.padding=1;
		
	// Resize the button appropriately.
	dim = Skin.current.Font.GetDimensions(settings.text);
	
	_widget.ResizeTo(dim.width+_widget.padding, dim.height+_widget.padding);
	
	_widget.onDrawContent = function(pen) 
	{ 
		pen.SetColor(1,1,1,1); 
		pen.WriteBoxed(Skin.current.Font, 0,0,this.width, this.height, Pen.ALIGN_CENTER, Pen.ALIGN_CENTER, this.text); 		
	}
	
	return _widget;
}

