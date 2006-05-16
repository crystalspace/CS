function Vertical(settings)
{
	l = new Widget;
	
	l._current_width = 0;
	l._current_height = 0;
	
	l.padding = new Number(settings.padding);
	l.kids = new Array();
	
	l.onDraw = Skin.current.Style.EmptyLayout;
		
	// When a child is added, grow the box.
	l.onAddChild = function(child)
	{
		var new_w = child.width + (this.padding*2);
		
		child.MoveTo(this.padding, this._current_height + this.padding);
						
		this._current_height += child.height + this.padding;
		
		if (new_w > this._current_width) 
		{	
			this._current_width = new_w;
						
			// Resize the width of all children.
			for(var kid in this.kids)
			{				
				kid.ResizeTo(new_w, kid.height);									
			}
		}
		else
		{
			// Make it the same height as the box.
			child.ResizeTo(this._current_width - (this.padding*2), child.height);			
		}
		
				
		this.ResizeTo(this._current_width, this._current_height);		
						
		this.kids.push(child);
	}	
	
	return l;
}