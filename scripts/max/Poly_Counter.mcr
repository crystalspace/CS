-- Credits:
-- Richard Katz
-- http://www.katz3d.com/maxscript.html

macroScript Poly_Counter

	category:"PlaneShift"
	buttontext:"Polygon Counter"
	toolTip:"Polygon Counter"
	icon:#("SubObjectIcons",15)
(
	local PolyCounterOn = false
	local lastViewport
	
	  fn rc_anySel = ($ != undefined)
  
	  fn rc_selfaces = 
	  (
	    local j = 0
	    for i in $ do 
		(
		  j += ((getPolygonCount i)[1])
		)
		return (j as string)
	  )

	  fn rc_allfaces = 
	  (
	    local j = 0
	    for i in $* do 
		(
		  j += ((getPolygonCount i)[1])
		)
		return (j as string)
	  )

	fn polytext = 
	(
		try 
		(
			if viewport.activeViewport != lastViewport do
			(	completeredraw()
				lastViewport = viewport.activeViewport 
			)
			
		  	viewtext = "Nothing selected"
	
			if ((getcommandpaneltaskmode() == #modify) and (subobjectlevel == 1)) then
			(
				Forcecompleteredraw()
				viewtext = ("Verts: " + ($.numverts as string) + ", Selected: " + ( (($.selectedverts).count) as string) )
			)
			else
			(
				if ((getcommandpaneltaskmode() == #modify) and (subobjectlevel > 1)) then
				(
					Forcecompleteredraw()
					viewtext = ("Faces: " + ($.numfaces as string) + ", Selected: " + ( (($.selectedfaces).count) as string) )
				)
		    	else
				(
					if rc_anySel() then
					(
						viewtext = ("Selected Objects: " + rc_selfaces() + " faces")
					)
					else
					(
						viewtext = ("Scene: " + rc_allfaces() + " faces")
					)
				)
			)
				
			gw.wtext [5,40,1000]  viewtext  color:(color 255 234 0)
			gw.enlargeUpdateRect #whole 
			gw.updateScreen() 
		)
		catch ()
	)

	on ischecked return PolyCounterOn 
	
	On execute do
	(	if PolyCounterOn then 
			unregisterRedrawViewsCallback polytext
		else
			registerRedrawViewsCallback polytext
		PolyCounterOn = not PolyCounterOn 
		completeredraw()
		--updateToolbarButtons()
	) 
) 


