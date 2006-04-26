function PartEdit2()
{
	_o = new Object;
	
	_o.main_window = Window("Particle Editor");
	_o.main_window.ResizeTo(300,100);
	_o.main_window.MoveTo(200,200);
	_o.main_window.StatusBar.text = "Ready";
		
	contents = 	 
		<layout type="Vertical">
		 <layout type="Horizontal">
		 	<widget type="Button">		 		
		 		<event name="onDrawContent" action="buttonDrawText('Reset')" />			 		
		 	</widget>
		 	<widget type="Label">
		 		<text>Reset all particle fountain variables</text>		 		
		 	</widget>
		 </layout>
		 
		 <layout type="Horizontal">
		 	<widget type="Button">
		 		<event name="onDrawContent" action="buttonDrawText('Pause')" />
		 	</widget>
		 	<widget type="Label">
		 		<text>Pause the particle fountain</text>		 		
		 	</widget>
		 </layout>		 
		</layout>;
			
	_o.main_window.AddChild(ParseXMLInterface(contents, null));				
	_o.main_window.Show();	
			
	return _o;
}

part_edit2 = PartEdit2();


testit =
 <parent>
   <item>
     <child>5</child>
   </item>
   <item>
     <child>10</child>
   </item>
 </parent>;
	