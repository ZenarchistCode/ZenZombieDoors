modded class ActionCloseDoors
{
	// When a door is closed, add it to the saved doors list
	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		// Door is shut, store reference so zombies can hit it
		Building building;
		if (Class.CastTo(building, action_data.m_Target.GetObject()))
		{
			int doorIndex = building.GetDoorIndex(action_data.m_Target.GetComponentIndex());
			if (doorIndex != -1 && !building.GetType().Contains("Wreck") && !building.GetType().Contains("wreck"))
			{
				ZedDoorsSaved.AddDoor(building, building.GetDoorSoundPos(doorIndex), doorIndex);
			}
		}
	}
};