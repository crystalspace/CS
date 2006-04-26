/** Window factory. */
function Window(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	_widget.is_active=false;
	_widget.draw_init=false;
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	// Setup a titlebar
	_widget.TitleBar = TitleBar(settings);
	_widget.TitleBar.Dock(_widget, Widget.DOCK_SOUTH);
	
	// Setup the statusbar
	_widget.StatusBar = StatusBar();
	_widget.Dock(_widget.StatusBar, Widget.DOCK_SOUTH);
	
	// Setup the resize knob
	_widget.ResizeKnob = ResizeKnob();
	_widget.AddChild(_widget.ResizeKnob);
	_widget.ResizeKnob.target = _widget;
		
	_widget.onDraw = prefs.Style.Window;	
	
	// Override some functions to make it behave as you'd expect.
	_widget.Show = function() { this.TitleBar.Show(); }
	_widget.Hide = function() { this.TitleBar.Hide(); }
		
	return _widget;
}
