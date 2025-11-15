modded class House
{
#ifdef SERVER
	//SERVER-SIDE ONLY CODE: 
	
	override void DeferredInit()
	{
		super.DeferredInit();

		if (GetType().Contains("Wreck") || GetType().Contains("wreck"))
			return;

		Zen_PrepareDoors();
	}

	void Zen_PrepareDoors()
	{
		#ifdef ZENMODPACK
		if (!ZenModEnabled("ZenZombieDoors"))
			return;
		#endif

		ZenZombieDoorsPlugin plugin = ZenZombieDoorsPlugin.Cast(GetPlugin(ZenZombieDoorsPlugin));
		if (!plugin)
		{
			return;
		}

		string houseType = GetType();
		int doorsCount = GetGame().ConfigGetChildrenCount("CfgVehicles " + houseType + " Doors");
		if (doorsCount == 0)
		{
			return;
		}

		ref array<ref ZenZombieDoorData> doorDataList = new array<ref ZenZombieDoorData>();

		for (int i = 0; i < doorsCount; i++)
		{
			vector worldDoorPos = GetDoorSoundPos(i);
			vector localDoorPos;

			if (!Zen_TryGetCorrectedLocalDoorPos(houseType, i, localDoorPos))
				localDoorPos = GetGame().ObjectWorldToModel(this, worldDoorPos);

			float groundY = GetGame().SurfaceY(worldDoorPos[0], worldDoorPos[2]);
			float doorHeight = worldDoorPos[1] - groundY;

			if (doorHeight < 0.7)
			{
				// Ignore oven doors etc
				continue;
			}

			doorDataList.Insert(new ZenZombieDoorData(localDoorPos, i, 0));
		}

		if (doorDataList.Count() > 0)
		{
			plugin.RegisterBuildingInstance(this, doorDataList);
		}
	}

	// Some building doors have the incorrect sound position memory points in model cfg - fix em here.
	bool Zen_TryGetCorrectedLocalDoorPos(string buildingType, int doorIndex, out vector localPos)
	{
		if (buildingType == "Land_Factory_Lathes")
		{
			if (doorIndex == 0) { localPos = "9.541016 -4.383759 -7.566406"; return true; }
		}

		if (buildingType == "Land_Garage_Small")
		{
			if (doorIndex == 0) { localPos = "2.206055 -1.015228 -3.611816"; return true; }
		}

		if (buildingType == "Land_Mil_ATC_Small")
		{
			if (doorIndex == 0) { localPos = "0.398438 -2.602600 4.620117"; return true; }
		}

		if (buildingType == "Land_Mil_AircraftShelter")
		{
			if (doorIndex == 0) { localPos = "-6.035156 -2.432983 -32.334961"; return true; }
			if (doorIndex == 1) { localPos = "-6.006836 -2.398254 -26.590820"; return true; }
			if (doorIndex == 2) { localPos = "1.466797 -2.398254 0.285156"; return true; }
		}

		if (buildingType == "Land_Prison_Side")
		{
			if (doorIndex == 12) { localPos = "1.635742 -1.974426 -8.293945"; return true; }
			if (doorIndex == 13) { localPos = "2.198242 -2.008179 -7.684570"; return true; }
		}

		if (buildingType == "Land_Rail_Station_Big")
		{
			if (doorIndex == 0) { localPos = "-3.408691 -3.987488 -3.997559"; return true; }
		}

		if (buildingType == "Land_Rail_Station_Small")
		{
			if (doorIndex == 0) { localPos = "-0.745117 -0.215073 2.135742"; return true; }
			if (doorIndex == 1) { localPos = "2.212891 -0.215073 -0.978516"; return true; }
			if (doorIndex == 2) { localPos = "3.514648 -0.215012 -3.110840"; return true; }
		}

		if (buildingType == "Land_Rail_Warehouse_Small")
		{
			if (doorIndex == 1) { localPos = "3.244141 -0.442261 -6.870117"; return true; }
			if (doorIndex == 2) { localPos = "1.809570 -0.442261 -12.404297"; return true; }
		}

		if (buildingType == "Land_Tisy_Barracks")
		{
			if (doorIndex == 0) { localPos = "-0.026367 -2.765686 -3.760742"; return true; }
			if (doorIndex == 1) { localPos = "0.401367 -2.797028 -0.500977"; return true; }
			if (doorIndex == 2) { localPos = "-1.064453 4.352539 -1.153320"; return true; }
			if (doorIndex == 3) { localPos = "-2.916016 1.353027 -3.669922"; return true; }
		}

		if (buildingType == "Land_Village_Pub")
		{
			if (doorIndex == 4) { localPos = "-1.795898 -1.222656 -2.440430"; return true; }
		}

		return false;
	}
#endif
}