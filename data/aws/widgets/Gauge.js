/** Gauge factory. */
function Gauge(settings)
{
	var _widget = new Widget;
	var prefs = Skin.current;
	
	if (settings==null) settings={};
	
	// Give it a pen
	_widget.SetPen(new Pen);
			
	// Setup the gauge
	_widget._value=new Number(settings.min);	
	_widget.min =  new Number(settings.min);
	_widget.max =  new Number(settings.max);
	_widget.step = new Number(settings.step);
	_widget.title = settings.title;
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
	_widget.onDraw = settings.type;
				
	return _widget;
}
