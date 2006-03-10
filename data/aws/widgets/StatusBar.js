/** StatusBar factory. */
function StatusBar()
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
	
	// The text of the titlebar.		
	_widget._text = "";
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("text", function(t) { this._text = t; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("text", function() { return this._text; });	
	
	_widget.ResizeTo(50, prefs.StatusBarHeight);
	
	_widget.onDraw = prefs.Style.StatusBar;
	
	return _widget;
}
