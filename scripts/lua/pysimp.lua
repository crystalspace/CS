function CreateRoom(matname)
	print('Start creating polygons...')
	system=GetSystem()
	engine=iSystem_Query_iEngine(system)
	room=iEngine_FindSector(engine, "room")
	material=iEngine_FindMaterial(engine, matname, 0)
--	polyset=iRoom_Query_iPolygonSet()
	if not polyset then
		print 'No polyset support, Jorrit broke it!'
		iSystem_Print(system, 4, 'Instead we can print to the console\n')

		g3d=iSystem_Query_iGraphics3D(system)
		mathandle=iMaterialWrapper_GetMaterialHandle(material)
		if not mathandle then
			print("Hrmmm, couldn't retrieve mathandle", mathandle)
			return
		end
		txthandle=iMaterialHandle_GetTexture(mathandle)
		iGraphics3D_BeginDraw(g3d, 51) --2D, 3D, ClearColor, ClearZ
		iGraphics3D_DrawPixmap(g3d, txthandle, 0, 0, 128, 128, 0, 0, 128, 128)
		iGraphics3D_FinishDraw(g3d)
	end

--	poly=polyset.CreatePolygon('floor')
--	poly.CreateVertex(csVector3(-5,0,5))
--	poly.CreateVertex(csVector3(5,0,5))
--	poly.CreateVertex(csVector3(5,0,-5))
--	poly.CreateVertex(csVector3(-5,0,-5))
--	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
--	poly.SetMaterial(material)

--	poly=polyset.CreatePolygon('ceiling')
--	poly.CreateVertex(csVector3(-5,20,-5))
--	poly.CreateVertex(csVector3(5,20,-5))
--	poly.CreateVertex(csVector3(5,20,5))
--	poly.CreateVertex(csVector3(-5,20,5))
--	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
--	poly.SetMaterial(material)

--	poly=polyset.CreatePolygon('w1')
--	poly.CreateVertex(csVector3(-5,20,5))
--	poly.CreateVertex(csVector3(5,20,5))
--	poly.CreateVertex(csVector3(5,0,5))
--	poly.CreateVertex(csVector3(-5,0,5))
--	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
--	poly.SetMaterial(material)

--	poly=polyset.CreatePolygon('w2')
--	poly.CreateVertex(csVector3(5,20,5))
--	poly.CreateVertex(csVector3(5,20,-5))
--	poly.CreateVertex(csVector3(5,0,-5))
--	poly.CreateVertex(csVector3(5,0,5))
--	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
--	poly.SetMaterial(material)

--	poly=polyset.CreatePolygon('w3')
--	poly.CreateVertex(csVector3(-5,20,-5))
--	poly.CreateVertex(csVector3(-5,20,5))
--	poly.CreateVertex(csVector3(-5,0,5))
--	poly.CreateVertex(csVector3(-5,0,-5))
--	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
--	poly.SetMaterial(material)

--	poly=polyset.CreatePolygon('w4')
--	poly.CreateVertex(csVector3(5,20,-5))
--	poly.CreateVertex(csVector3(-5,20,-5))
--	poly.CreateVertex(csVector3(-5,0,-5))
--	poly.CreateVertex(csVector3(5,0,-5))
--	poly.SetTextureSpace(poly.GetVertex(0), poly.GetVertex(1), 3)
--	poly.SetMaterial(material)
	print 'Finished!'
	
end
