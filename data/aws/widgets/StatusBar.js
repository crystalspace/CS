/** StatusBar factory. */
function StatusBar()
{
	var _widget = new Widget;
	var prefs = Skin.current;
	var sb = prefs.StatusBar;
	
	// Give it a pen
	_widget.SetPen(new Pen);
	
	// The text of the titlebar.		
	_widget._text = "";
	_widget.draw_init=false;
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("text", function(t) { this._text = t; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("text", function() { return this._text; });	
	
	_widget.ResizeTo(50, sb.h);
	
	_widget.onDraw = prefs.Style.StatusBar;
	
	return _widget;
}
