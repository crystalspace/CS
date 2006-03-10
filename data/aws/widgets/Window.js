/** Window factory. */
function Window(title)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	// Setup a titlebar
	_widget.TitleBar = TitleBar(title);
	_widget.TitleBar.Dock(this, Widget.DOCK_SOUTH);
	
	// Setup the statusbar
	_widget.StatusBar = StatusBar();
	_widget.Dock(_widget.StatusBar, Widget.DOCK_SOUTH);
			
	
	return _widget;
}
