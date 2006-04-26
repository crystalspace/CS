/** Slider factory. */
function Slider(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
		
	// Set the orientation of the Slider
	_widget.orientation_vertical = settings.vertical;
	
	// Setup the slider
	_widget._value=new Number(settings.min);
	_widget.bar_size=50;
	_widget.max=new Number(settings.max); 
	_widget.min=new Number(settings.min);	
	_widget.tick_step = new Number(settings.step);
	_widget.tick_lock = new Boolean(settings.lock);
	_widget.slide_button_down=false;
	_widget.init_draw=false;
		
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("value", function(v) { this._value = v; this.Invalidate(); if (this.onChange) this.onChange(this); });	
	_widget.__defineGetter__("value", function() { return this._value; });	
		
	//  Set initial size
	if (settings.vertical)
	{	// vertical		
		_widget.Resize(10, 20);		
	}
	else
	{	// horizontal
		_widget.Resize(20, 10);		
	}
	
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = Skin.current.Style.Slider;
	
	// Set the mouse down handler
	_widget.onMouseDown = function(buttons, wx, wy, sx, sy)
	{		    
		if (wx>=this.ButtonScroll.x1 && wx<=this.ButtonScroll.x2 &&
		    wy>=this.ButtonScroll.y1 && wy<=this.ButtonScroll.y2)
		    {
				this.slide_button_down=true;    
				this.Invalidate();
				this.CaptureMouse();			
				
				this.last_x = sx;
				this.last_y = sy;
				
				return;
		    }		
	}
	
	_widget.onMouseUp = function(buttons)
	{
		this.slide_button_down=false;			
		
		this.ReleaseMouse();
		this.Invalidate();
	}
	
	_widget.onMouseMove = function(buttons, wx, wy, sx, sy)
	{
		if (this.slide_button_down)
		{
			if (this.orientation_vertical)
			{
				var dy = sy - this.last_y;
				
				// Find out how many values to move based on the change in y.
				var vc = ((this.max-this.min) * dy) / (this.height - Skin.current.Slider.Thickness);
								
				this.value+=vc;				
			}
			else
			{
				var dx = sx - this.last_x;
				
				// Find out how many values to move based on the change in x.
				var vc = ((this.max-this.min) * dx) / (this.width);
				
				this.value+=vc;								
			}	
			
			this.last_x = sx;
			this.last_y = sy;
			
			if (this.value>this.max) this.value=this.max;
			if (this.value<this.min) this.value=this.min;					
		}
	}
	
	
	
	return _widget;
}
