// Used as a place to store functions that return somewhat complex objects, or the code to create them.
skin_AmalgamRes =
{
    _createGlassGradient : function() 
    {
				var gr = new Gradient();
    			
				gr.AddColor(new Color(1,1,1,1), 0.0);	
				gr.AddColor(new Color(1,1,1,1), 0.1);	
				gr.AddColor(new Color(0.6,0.6,0.6,1), 0.5);					
				gr.AddColor(new Color(0.4,0.4,0.4,1), 0.51);				
				gr.AddColor(new Color(0.7, 0.7, 0.7, 1), 0.9);
				gr.AddColor(new Color(0.0, 0.0, 0.0, 1), 1.0);
				
				return gr;
	},
	
	_createFrostedGlassGradient : function() 
    {
				var gr = new Gradient();
    						
				gr.AddColor(new Color(1.0, 1.0, 1.0, 1.0), 1.0);						
				gr.AddColor(new Color(1.0, 1.0, 1.0, 0.0), 0.0);
	
				return gr;
	},
	
	_createDropShadowGradient : function() 
    {
				var gr = new Gradient();
    						
				gr.AddColor(new Color(0.1, 0.1, 0.1, 1.0), 0.0);						
				gr.AddColor(new Color(0.1, 0.1, 0.1, 0.0), 1.0);
	
				return gr;
	},
	
		
}


skin_Amalgam = 
{
    Name    : "Amalgam",
    Style   : Style3D,
    
    TitleFont : new Font("VeraSans", 14),
    Font 	  : new Font("VeraSans", 10),
    GaugeFont : new Font("VeraSans", 6),
    ClockFont : new Font("VeraSerif", 10),    
    	
	OverlayGradient : skin_AmalgamRes._createGlassGradient(),
	DropShadowGradient : skin_AmalgamRes._createDropShadowGradient(),
	
	Arrows :
	{
		Left  : new Texture("/aws/arrow_lt.png"),
		Right : new Texture("/aws/arrow_rt.png"),
		Up    : new Texture("/aws/arrow_up.png"),
		Down  : new Texture("/aws/arrow_dn.png"),
	},
	
    HighlightColor    : new Color(0.76, 0.96, 1, 1.0),
    ShadowColor	      : new Color(0.26, 0.56, 0.64, 1.0),
    FillColor		  : new Color(0.56, 0.86, 0.94, 1.0),
    TextDisabledColor : new Color(0.5,  0.5,  0, 1),
    TextForeColor	  : new Color(0, 0, 0, 1),
    TextBackColor	  : new Color(1, 1, 1, 1),            
    
    ScrollBarHeight		 : 24,
    ScrollBarWidth		 : 24,
    
    TitleBar			 :
    {
    	h:30,
    	Text   : new Color(1,1,1,1),
    	Base   : new Color(1,1,1,1),    	
    	Border : new Color(1,1,1,0.75),
    	Active : new Color(0.16, 0.46, 0.54, 1.0),
    	Inactive : new Color(0.5,0.5,0.5, 0.5),
    	BkgGrad : skin_AmalgamRes._createFrostedGlassGradient()
    },
        
    StatusBar			 :
    {
    	h:20,
    	Text   : new Color(0,0,0,0),
    	Border : new Color(1,    1,    1,    0.75),
    	Base   : new Color(0.56, 0.86, 0.94, 1.0),
    	BkgGrad : skin_AmalgamRes._createFrostedGlassGradient()
    },
    
    // The draw member can be a function or a filename.
    // If a function is specified, it will be called to draw the item.
    
    WindowMin   : { w:20, h:20 },
  	WindowZoom  : { w:20, h:20 },
  	WindowClose : { w:20, h:20 },  	  	
  	
  	CheckBox    : 
  	{   w:15, h:15, 
  		Base:new Color(0.56, 0.86, 0.94), 
  		Border	: new Color(0.85, 0.85, 0.85, 1.0),
  		Over:new Color(0.35,0.35,0.35,0.75) 
  	},
  	
  	RadioButton : 
  	{ 
  		w:15, h:15, 
  		Base	: new Color(0.56, 0.86, 0.94, 1.0), 
  		Border	: new Color(0.85, 0.85, 0.85, 1.0),
  		Over	: new Color(0.94, 0.86, 0.56, 1.0),
  		BkgGrad : skin_AmalgamRes._createGlassGradient(),
  	},
  	
  	Button : 
  	{   	
	  	w:40, h:20,	
  		Base  : new Color(0.56, 0.86, 0.94, 1.0),  
  		Border	: new Color(0,    0,    0,    1.0),
  		Over	: new Color(0.56, 0.86, 0.94, 0.5), 
  		Down	: new Color(0.16, 0.36, 0.44, 0.5) 
  	},
  	
  	Slider : 
  	{   		
  		Base	: new Color(0.06, 0.36, 0.44, 1.0), 
  		Border	: new Color(0,    0,    0,    1.0),
  		Over	: new Color(0.94, 0.86, 0.56, 1.0),  		
  		Thickness : 15,
  		TickThickness : 5,
  	},
  	
  	RoundedPanel : 
  	{   		
  		Base	: new Color(0.85, 0.85, 0.85, 1.0), 
  		Border	: new Color(0,    0,    0,    1.0),  		
  		Roundness   : 10,
  	},
  	
  	MiteredPanel : 
  	{   		
  		Base	: new Color(0.85, 0.85, 0.85, 1.0), 
  		Border	: new Color(0,    0,    0,    1.0),  		
  		Miter   : 10,
  	},
  	
  	ListBox : 
  	{   	
	  	w:100, h:50,	
  		Base  	: new Color(0.56, 0.86, 0.94, 1.0),  
  		Border	: new Color(0,    0,    0,    1.0),
  		Color1  : new Color(0.75, 0.75, 0.75, 1.0),
  		Color2  : new Color(0.70, 0.7,  0.7,  1.0),
  		Selected: new Color(0.0,  0.0,  1.0,  0.5),
  	},  
  	  	
  	
  	FileChooser :
  	{
		w:200, h:100,
						
		DocIcon: new Texture("/aws/regular-doc.png"),
		CompressedDocIcon: new Texture("/aws/compressed-doc.png"),
		AudioDocIcon: new Texture("/aws/audio-doc.png"),
		VideoDocIcon: new Texture("/aws/video-doc.png"),
		ImageDocIcon: new Texture("/aws/image-doc.png"),
		MeshDocIcon: new Texture("/aws/mesh-doc.png"),
		
		FolderNavUpIcon: new Texture("/aws/folder-nav-up.png"),
		ActiveFolderIcon: new Texture("/aws/active-folder.png"),
		VisitedFolderIcon: new Texture("/aws/visited-folder.png"),
		FolderIcon: new Texture("/aws/folder.png"),
		
		
		CompressedDocTypes: [".zip", ".gz", ".bz2", ".tgz"],
		AudioDocTypes: [".mp3", ".wav", ".ogg", ".wma", ".au"],
		VideoDocTypes: [".wmv", ".avi", ".rv"],
		ImageDocTypes: [".jpg", ".png", ".bmp", ".gif"],
		MeshDocTypes: [".blend", ".max", ".3ds"],		
				 	
  	}      
	
};
  
// Add it into the list of skins
Skin.Add(skin_Amalgam);
