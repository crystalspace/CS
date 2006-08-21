// Setup some helpers for the particleFountain object.
particleFountain.Stop = function()
{
	var stopCmd = 0;
	particleFountain.onEvent(stopCmd);	
}

particleFountain.Start = function()
{
	var startCmd = 1;
	particleFountain.onEvent(startCmd);	
}

function PartEdit2()
{
	_o = new Object;
	
	_o.main_window = Window({title:"Particle Editor"});
	_o.main_window.ResizeTo(300,100);
	_o.main_window.MoveTo(200,200);
	_o.main_window.StatusBar.text = "Ready";
		
	contents = 	 
	  <layout type="Vertical">		 
		 
		 <layout type="Horizontal" id="testHorz">		  
		 	<padding>5</padding>
		 
		 	<widget type="Button">		 		
		 		<setup>widget.Resize(10,0);</setup>
		 		<event name="onDrawContent" action="buttonDrawText('Reset')" />			 		
		 	</widget>
		 	<widget type="Label">
		 		<text>Reset all particle fountain variables</text>
		 		<border>{false}</border>		 		
		 	</widget>		 	
		 	
		 </layout>
		 
		 <layout type="Horizontal">
		    <padding>5</padding>
		  
		 	<widget type="Button">
		 		<setup>widget.Resize(10,0);</setup>
		 		<event name="onDrawContent" action="buttonDrawText('Pause')" />
		 		<event name="onMouseClick" action="particleFountain.Stop;" />
		 	</widget>
		 	<widget type="Label" id="pauseDescLabel">
		 		<setup>
		 			<![CDATA[
		 						 				
		 				pauseDescLabel.text = "Pause the particle fountain.";
		 				pauseDescLabel.border = false;
		 			
		 			]]>
		 		</setup>
		 	</widget>
		 	
		 </layout>		 
	  </layout>;
			
	_o.main_window.AddChild(ParseXMLInterface(contents, null));				
	_o.main_window.Show();	
			
	return _o;
}

part_edit2 = PartEdit2();

