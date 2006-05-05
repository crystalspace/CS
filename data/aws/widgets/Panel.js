/** Panel factory. */
function Panel(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	_widget.is_active=false;
	_widget.draw_init=false;
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	_widget.onDraw = settings.style;	
		
	return _widget;
}
