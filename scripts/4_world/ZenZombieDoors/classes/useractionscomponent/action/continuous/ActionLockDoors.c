modded class ActionLockDoors
{
	// When a door is locked, add it to the saved doors list
	override void LockDoor(ActionTarget target)
	{
		super.LockDoor(target);

		// Door has been locked, store reference so zombies can hit it
		Building building;
		if (Class.CastTo(building, target.GetObject()))
		{
			int doorIndex = building.GetDoorIndex(target.GetComponentIndex());
			if (doorIndex != -1)
			{
				ZedDoorsSaved.AddDoor(building, building.GetDoorSoundPos(doorIndex), doorIndex);
			}
		}
	}
}