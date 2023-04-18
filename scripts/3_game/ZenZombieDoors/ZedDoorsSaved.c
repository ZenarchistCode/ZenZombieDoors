// Saves opened/closed doors by their sound position
class ZedDoorsSaved
{
	static ref array<ref ZenDoorData> DOORS = new array<ref ZenDoorData>;

	// Add a saved door from the given building and door index based on vector sound location
	static void AddDoor(Building b, vector v, int index)
	{
		ZenDoorData existingDoor = GetDoor(v);

		// If door doesn't exist, add it - otherwise just reset the door hit counter
		if (existingDoor == NULL)
		{
			DOORS.Insert(new ZenDoorData(b, v, index));
		}
		else
		{
			existingDoor.ResetDoorHitCount();
		}
	}

	// Remove the given door based on vector location, building type & index
	static void RemoveDoor(Building b, vector v, int index)
	{
		int removeIdx = -1;
		for (int idx = 0; idx < DOORS.Count(); idx++)
		{
			ZenDoorData d = DOORS.Get(idx);

			if ( d && d.building.GetType() == b.GetType() && d.pos == v && d.index == index )
			{
				removeIdx = idx;
				break;
			}
		}

		if (removeIdx != -1)
			DOORS.Remove(removeIdx);
	}

	// Get any found saved door at the given vector position
	static ZenDoorData GetDoor(vector pos, float dist = 2.3)
	{
		for (int idx = 0; idx < DOORS.Count(); idx++)
		{
			ZenDoorData d = DOORS.Get(idx);

			if (vector.Distance(d.pos, pos) <= dist)
			{
				return d;
			}
		}

		return NULL;
	}
};

// Define saved 'door data' (building type, position, door index & current number of zombie hits)
class ZenDoorData
{
	Building building;
	vector pos;
	int index;
	int hitCount = 0;

	void ZenDoorData(Building b, vector p, int i)
	{
		building = b;
		pos = p;
		index = i;
	}

	// Called whenever a zombie hits a door
	void HitDoor()
	{
		hitCount = hitCount + 1;

		// After ~X secs of no hits, reset door hit counter
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ResetDoorHitCount);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ResetDoorHitCount, GetZombieDoorsConfig().DoorHitCounterTimerSecs * 1000, false);
	}

	// Resets the hit counter
	void ResetDoorHitCount()
	{
		hitCount = 0;
	}
};