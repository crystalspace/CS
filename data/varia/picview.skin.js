///////////////////////////////////////////////
////////// Picview skin definition file    ////
///////////////////////////////////////////////

// Load the AWS2 default widgets and styles.
Sys.Load("/aws/Aws2Default.js");

// Load the skin
Sys.Load("/aws/skins/Amalgam.js");

// Set it as the default skin
Skin.current = skin_Amalgam;

// Setup signal handlers for the picView object.
picView.First = function() { this.onEvent(this.cmdFirst); }
picView.Prev  = function() { this.onEvent(this.cmdPrev); }
picView.Next  = function() { this.onEvent(this.cmdNext); }
picView.Quit  = function() { this.onEvent(this.cmdQuit); }
picView.Scale = function() { this.onEvent(this.cmdScale); }

// Create a new widget with the picview controls in it.
function PicViewControls()
{
	button_bar = 	 	  	 
		 <layout type="Horizontal" id="button_bar">		  
		 	<padding>5</padding>
		 
		 	<widget type="Button">		 				 		
		 		<event name="onDrawContent" action="buttonDrawText('First')" />	
		 		<event name="onMouseClick" action="picView.First;" />		 		
		 	</widget>
		 	<widget type="Button">		 				 		
		 		<event name="onDrawContent" action="buttonDrawText('Prev')" />			 		
		 		<event name="onMouseClick" action="picView.Prev;" />		 		
		 	</widget>
		 	<widget type="Button">		 				 		
		 		<event name="onDrawContent" action="buttonDrawText('Next')" />			 		
		 		<event name="onMouseClick" action="picView.Next;" />		 		
		 	</widget>
		 	<widget type="Button">		 				 		
		 		<event name="onDrawContent" action="buttonDrawText('Quit')" />			 		
		 		<event name="onMouseClick" action="picView.Quit;" />		 				 		
		 	</widget>		 	
		 	<widget type="Button">		 				 		
		 		<event name="onDrawContent" action="buttonDrawText('Quit')" />			 		
		 		<event name="onMouseClick" action="picView.Scale;" />		 				 		
		 	</widget>		 	
		 </layout>;	
	
	return ParseXMLInterface(button_bar, null);
}

// Create a new controls widget
controls = PicViewControls();

// Center the controls on the top of the screen
controls.MoveTo((Sys.GetWidth()/2) - (controls.width/2), 0);

