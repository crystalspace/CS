/** ScrollBar factory. */
function ScrollBar(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	// Set the orientation of the ScrollBar
	if (settings.orientation_vertical!=undefined)
		_widget.orientation_vertical = Boolean(settings.orientation_vertical);
	else
		_widget.orientation_vertical = false;
	
	// Setup the scroll bar
	_widget._value=0;
	_widget.bar_size=50;
	_widget.max=100;
	_widget.min=0;
	_widget.inc_button_down=false;
	_widget.dec_button_down=false;
	_widget.scroll_bar_down=false;
	
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("value", function(v) { this._value = v; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("value", function() { return this._value; });	
		
	//  Set initial size
	if (_widget.orientation_vertical)
	{	// vertical		
		_widget.Resize(prefs.ScrollBarWidth, 20);
		_widget.SetFrameAnchor(Widget.STICK_NORTH | Widget.STICK_SOUTH | Widget.TRACK_EAST);
	}
	else
	{	// horizontal
		_widget.Resize(20, prefs.ScrollBarHeight);		
		_widget.SetFrameAnchor(Widget.STICK_EAST | Widget.STICK_WEST | Widget.TRACK_SOUTH);
	}
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.ScrollBar;
	
	// Set the mouse down handler
	_widget.onMouseDown = function(buttons, wx, wy, sx, sy)
	{		
		if (wx>=this.ButtonInc.x1 && wx<=this.ButtonInc.x2 &&
		    wy>=this.ButtonInc.y1 && wy<=this.ButtonInc.y2)
		    {
				this.inc_button_down=true;  
				this.Invalidate();  
				this.CaptureMouse();
				
				++this.value;
				return;
		    }
		
		if (wx>=this.ButtonDec.x1 && wx<=this.ButtonDec.x2 &&
		    	wy>=this.ButtonDec.y1 && wy<=this.ButtonDec.y2)
		    {
				this.dec_button_down=true;    
				this.Invalidate();
				this.CaptureMouse();
				
				--this.value;
				return;
		    }			
		    
		if (wx>=this.ButtonScroll.x1 && wx<=this.ButtonScroll.x2 &&
		    wy>=this.ButtonScroll.y1 && wy<=this.ButtonScroll.y2)
		    {
				this.scroll_button_down=true;    
				this.Invalidate();
				this.CaptureMouse();			
				
				this.last_x = sx;
				this.last_y = sy;
				
				return;
		    }		
	}
	
	_widget.onMouseUp = function(buttons)
	{
		this.inc_button_down=false;
		this.dec_button_down=false;				
		this.scroll_button_down=false;			
		
		this.ReleaseMouse();
		this.Invalidate();
	}
	
	_widget.onMouseMove = function(buttons, wx, wy, sx, sy)
	{
		if (this.scroll_button_down)
		{
			if (this.orientation_vertical)
			{
				var dy = sy - this.last_y;
				
				this.value+=dy;				
			}
			else
			{
				var dx = sx - this.last_x;
				
				this.value+=dx;								
			}	
			
			this.last_x = sx;
			this.last_y = sy;
			
			if (this.value>this.max) this.value=this.max;
			if (this.value<this.min) this.value=this.min;					
		}
	}
	
	
	
	return _widget;
}
