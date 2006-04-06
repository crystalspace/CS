/** Gauge factory. */
function Gauge(title, gauge_type, min, max, step)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	// Give it a pen
	_widget.SetPen(new Pen);
			
	// Setup the gauge
	_widget._value=min;	
	_widget.min = min;
	_widget.max = max;
	_widget.step = step;
	_widget.title = title;
	_widget.draw_init=false;
		
	// Invalidate and fire onChange when the value property is set.
	_widget.__defineSetter__("value", function(v) 
									  { 
										  if (v<this.min) v=this.min;
										  if (v>this.max) v=this.max;
										  
										  this._value = v; 										  
										  this.Invalidate(); 
										  if (this.onChange) this.onChange(this); 
									   }
							);	
									   
	_widget.__defineGetter__("value", function() { return this._value; });	
	
	_widget.Resize(100, 50);	
			
	// Set the drawing function to be whatever the current style dictates.
	_widget.onDraw = gauge_type;
				
	return _widget;
}
