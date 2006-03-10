/** ResizeKnob factory. */
function ResizeKnob()
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);	
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = prefs.Style.ResizeKnob;
	
	// Set the default size.
	_widget.ResizeTo(prefs.ScrollBarWidth, prefs.ScrollBarHeight);
	
	// Set default position
	_widget.SetFrameAnchor(Widget.TRACK_EAST | Widget.TRACK_SOUTH);
		
	_widget.is_dragging=false;
	_widget.target = null;
	
	// Set the mouse down handler
	_widget.onMouseDown = function(buttons, wx, wy, sx, sy)
	{		    
		this.is_dragging=true;    
		this.Invalidate();
		this.CaptureMouse();			
		
		this.last_x = sx;
		this.last_y = sy;	
	}
	
	_widget.onMouseUp = function(buttons)
	{		
		this.is_dragging=false;			
		
		this.ReleaseMouse();
		this.Invalidate();
	}
	
	_widget.onMouseMove = function(buttons, wx, wy, sx, sy)
	{
		if (this.is_dragging && this.target)
		{			
			this.target.Resize(sx - this.last_x, sy - this.last_y);
			
			this.last_x = sx;
			this.last_y = sy;
		}				
	}
	
	
	
	return _widget;
}
