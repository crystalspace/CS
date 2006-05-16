/** FileChooser factory. */
function FileChooser(settings)
{
	var _widget = Vertical({padding:2});
	var prefs = Skin.current;
	
	if (settings==null) settings={};
		
	// If they specify a width, set that here.
	var w = SafeDefault(settings.width, Number(settings.width), 200);
	
	// Make it a decent size.
	_widget.ResizeTo(w,0);
	
	// Add a label to identify the widget.
	_widget.AddChild(Label({text:"File Selection"}));
	
	// Setup a horizontal layout
	var hz = Horizontal({padding:2});	
	hz.ResizeTo(0,28);
	
	// Add a control button
	_widget.up_btn = ButtonWithIcon({icon:prefs.FileChooser.FolderNavUpIcon, padding:2});	
	_widget.up_btn.ResizeTo(26,26);
	_widget.up_btn.chooser = _widget;
	_widget.up_btn.onMouseClick = function() { this.chooser.GoUp(); }
	
	// Add the textbox here when it exists.
	_widget.filepath = TextBox();
 	_widget.filepath.SetFrameAnchor(Widget.STICK_EAST);
 	_widget.filepath.SetMargin(2,Widget.MARGIN_EAST);
		
	// Add the horizontal layout.
	hz.AddChild(_widget.up_btn);		
	hz.AddChild(_widget.filepath);
 	hz.SetFrameAnchor(Widget.STICK_EAST);
	_widget.AddChild(hz);
		
	// Add the listbox.
	_widget.file_list = ListBox();
	_widget.file_list.chooser = _widget;
	_widget.file_list.onChange = function () { this.chooser.EnterFolder(this.items[this.selected].text); }
	_widget.AddChild(_widget.file_list);	
	
		
	// Fill it
	_widget.fs = new Vfs();
	_widget.RefreshFileList = function ()
	{
		this.file_list.Clear();
		
		var files = this.fs.FindFiles(this.filter);
		var fcp = Skin.current.FileChooser;
		var cwd = this.fs.GetCwd();
		
		cwd = cwd.replace(/\//g, "\\/");
		files.sort();
				
		for each(var file in files)
		{
			var icon;
			
			if (file.slice(-1) == '/') icon = fcp.FolderIcon;
			else
			{
				var ext_pos = file.lastIndexOf('.');
				var ext = file.slice(ext_pos);
				
				if (ext in fcp.CompressedDocTypes) icon = fcp.CompressedDocIcon;
				else if (ext in fcp.AudioDocTypes) icon = fcp.AudioDocIcon;
				else if (ext in fcp.ImageDocTypes) icon = fcp.ImageDocIcon;
				else if (ext in fcp.MeshDocTypes)  icon = fcp.MeshDocIcon;
				else icon = fcp.DocIcon;				
			}
			
			file=file.replace(eval("/"+cwd+"/"), "");
			
			var item = ListBoxTextandIconItem({text:file, icon:icon});
			this.file_list.AddItem(item);	
		}				
	}
	
	_widget.GoUp = function ()
	{
		this.fs.PopDir();
		this.filepath.text = this.fs.GetCwd();
		this.RefreshFileList();				
	}
	
	_widget.EnterFolder = function (new_dir)
	{
		this.filepath.text = this.fs.GetCwd() + new_dir;
		
		if (new_dir.slice(-1) != '/') return;		
		this.fs.PushDir(new_dir);
		
		this.RefreshFileList();						
	}
	
	_widget.filter = SafeDefault(settings.filter, String(settings.filter), "*");
	_widget.fs.PushDir("this");
	_widget.filepath.text = _widget.fs.GetCwd();
	
	// Fill the file list
	_widget.RefreshFileList();	
	
	// Set the drawing background.
	_widget.SetPen(new Pen);
	_widget.onDraw = SafeDefaultEval(settings.panel, "prefs.Style."+String(settings.panel), prefs.Style.RoundedPanel); 	
	
	// Add sticky stuff and tracking AFTER the layout is filled.
	_widget.file_list.SetFrameAnchor(Widget.STICK_EAST | Widget.STICK_WEST | Widget.STICK_SOUTH);
	_widget.file_list.SetMargin(10, Widget.MARGIN_SOUTH);
	_widget.file_list.SetMargin(2, Widget.MARGIN_WEST);
	_widget.file_list.SetMargin(2, Widget.MARGIN_EAST);
				
	return _widget;
}
