class ZenBuildingData
{
	House m_Building;
	ref array<ref ZenZombieDoorData> m_Doors;

	void ZenBuildingData(House building, array<ref ZenZombieDoorData> doors)
	{
		m_Building = building;
		m_Doors = doors;
	}
}