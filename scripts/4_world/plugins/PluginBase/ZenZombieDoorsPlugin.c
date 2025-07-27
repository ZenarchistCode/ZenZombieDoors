class ZenZombieDoorsPlugin extends PluginBase
{
	protected ref array<ref ZenBuildingData> m_CachedBuildingsInWorld;

	override void OnInit()
	{
		Print("[ZenZombieDoorsPlugin] :: OnInit");
		m_CachedBuildingsInWorld = new array<ref ZenBuildingData>();
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DumpHouseStats, 60000, false);
	}

	override void OnDestroy()
	{
		if (m_CachedBuildingsInWorld)
		{
			m_CachedBuildingsInWorld.Clear();
		}

		if (GetGame() && GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM))
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(DumpHouseStats);
		}
	}

	void DumpHouseStats()
	{
		if (m_CachedBuildingsInWorld)
		{
			Print("[ZenZombieDoorsPlugin] :: Total building instances tracked: " + m_CachedBuildingsInWorld.Count());
		}
	}

	void RegisterBuildingInstance(House building, array<ref ZenZombieDoorData> doorList)
	{
		m_CachedBuildingsInWorld.Insert(new ZenBuildingData(building, doorList));
	}

	array<ref ZenBuildingData> GetAllBuildings()
	{
		return m_CachedBuildingsInWorld;
	}

	ZenZombieDoorData GetNearestDoor(House building, vector worldDoorPos, float maxDistSq = 0.05)
	{
		foreach (ZenBuildingData data : m_CachedBuildingsInWorld)
		{
			if (data.m_Building != building)
				continue;

			foreach (ZenZombieDoorData door : data.m_Doors)
			{
				vector worldPos = GetGame().ObjectModelToWorld(building, door.m_LocalPos);
				if (vector.DistanceSq(worldDoorPos, worldPos) <= maxDistSq)
					return door;
			}
		}

		return null;
	}

	int GetDoorHitCount(House building, vector worldDoorPos, bool increase = true)
	{
		ZenZombieDoorData door = GetNearestDoor(building, worldDoorPos);
		if (!door)
			return 0;

		if (increase)
			door.m_HitCount++;

		return door.m_HitCount;
	}

	void ResetDoorHitCount(ZenBuildingData bData, vector doorPos)
	{
		foreach (ZenZombieDoorData door : bData.m_Doors)
		{
			if (vector.Distance(door.m_LocalPos, doorPos) < 0.1)
			{
				door.m_HitCount = 0;
				break;
			}
		}
	}
}