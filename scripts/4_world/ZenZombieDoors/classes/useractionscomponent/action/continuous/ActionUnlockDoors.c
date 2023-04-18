modded class ActionUnlockDoors
{
	// When a door is unlocked, add it to the saved doors list
	override void UnlockDoor(ActionTarget target)
	{
		super.UnlockDoor(target);

		// Door is unlocked, but it's still closed, so add reference so zombies can hit it
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