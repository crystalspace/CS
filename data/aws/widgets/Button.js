/** Button factory. */
function Button()
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
	
	_widget.Resize(40, 20);	
			
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
		_widget.over=true;
		this.Invalidate();	
	}
	
	_widget.onMouseExit = function()
	{
		_widget.over=false;
		this.Invalidate();	
	}
		
	return _widget;
}
