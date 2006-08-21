/** RadioButton factory. */
function RadioButton(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);
			
	// Setup the scroll bar
	_widget._state=false;
	_widget.over=false;
	_widget.draw_init=false;
			
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("state", function(v) { this._state = v; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("state", function() { return this._state; });	
	
	// Invalidate when the text changes.
	_widget.__defineSetter__("text", function(v) { this._text = v; this.Invalidate();} );	
	_widget.__defineGetter__("text", function() { return this._text; });	
	
	_widget.text = SafeDefault(settings.text, String(settings.text), "");	
	_widget.align = SafeDefault(settings.align, Number(settings.align), Pen.ALIGN_LEFT);
	
	// Set the size	
	var dim = prefs.Font.GetDimensions(_widget.text);
    _widget.Resize(prefs.RadioButton.w + 5 + dim.width, dim.height);
			
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.RadioButton;
			
	// If we get a mouse down, change the button's state.
	_widget.onMouseDown = function(buttons)
	{
		this.parent.Broadcast("onRadioGroupChange");
		
		this.state=true;	
		this._active=true;
		this.CaptureMouse();		
	}
	
	// If the mouse is up, change the state.
	_widget.onMouseUp = function(buttons)
	{						
		this._active=false;	
		this.ReleaseMouse();
		this.Invalidate();		
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
	
	_widget.onRadioGroupChange = function()
	{		
		this.state=false;	
	}
		
	return _widget;
}
