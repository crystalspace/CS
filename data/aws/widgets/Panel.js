/** Panel factory. */
function Panel(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	_widget.is_active=false;
	_widget.draw_init=false;
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	_widget.onDraw = settings.style;	
	
	if (settings.border!=undefined) _widget.border=Boolean(settings.border);
	else _widget.border=false;
		
	return _widget;
}
