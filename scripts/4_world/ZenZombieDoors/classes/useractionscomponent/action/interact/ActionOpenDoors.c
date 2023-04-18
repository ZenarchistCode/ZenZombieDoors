modded class ActionOpenDoors
{
	// When a door is opened, remove it from the saved doors list
	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		// Door is no longer shut, no need to keep it stored as a reference
		Building building;
		if (Class.CastTo(building, action_data.m_Target.GetObject()))
		{
			int doorIndex = building.GetDoorIndex(action_data.m_Target.GetComponentIndex());
			if (doorIndex != -1 && !building.GetType().Contains("Wreck") && !building.GetType().Contains("wreck"))
			{
				ZedDoorsSaved.RemoveDoor(building, building.GetDoorSoundPos(doorIndex), doorIndex);
			}
		}
	}
};