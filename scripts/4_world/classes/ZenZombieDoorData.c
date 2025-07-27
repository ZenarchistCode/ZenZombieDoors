class ZenZombieDoorData
{
	vector m_LocalPos;	// Position relative to building
	int m_DoorIndex;    // Index used with OpenDoor() etc
	int m_HitCount;		// Zombie hit counter

	void ZenZombieDoorData(vector localPos, int doorIndex, int hitCount = 0)
	{
		m_LocalPos = localPos;
		m_DoorIndex = doorIndex;
		m_HitCount = hitCount;
	}
}