/** ComboBox factory. */
function ComboBox(settings)
{
	if (settings==null) settings={};
	
	var _widget = TextBox(settings);
	var prefs = Skin.current;
		
	// Initial listbox state
	_widget.listbox_visible = false;
			
	// Setup the listbox
	_widget.ListBox = ListBox(settings);
	_widget.ListBox.Dock(_widget, Widget.DOCK_SOUTH);

	_widget.ListBox.Hide();
	
	// Setup the button
	_widget.Button = Button(settings);
	_widget.Button.Dock(_widget, Widget.DOCK_EAST);
	_widget.Button.combo_box = _widget;
	_widget.Button.onMouseClick = function () 
	{ 
		if (this.combo_box.listbox_visible)
		{
			this.combo_box.ListBox.Hide(); 
			this.combo_box.listbox_visible=false;
		}
		else 
		{
			this.combo_box.ListBox.Show(); 
			this.combo_box.listbox_visible=true;
		}		
	}
			
	return _widget;
}
