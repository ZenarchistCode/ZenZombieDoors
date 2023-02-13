modded class ZombieBase extends DayZInfected
{
	// All of this is server-side only
	#ifdef SERVER

	private float m_MovementSpeed;
	private int m_MindState;
	private ref autoptr ZenZombieData m_ZedData;

	// Called when zombie is created
	override void EOnInit(IEntity other, int extra)
	{
		super.EOnInit(other, extra);

		// Get zombie data
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitZedData, Math.RandomFloatInclusive(1000, 1500), false);
	}

	// Init zombie manager
	private void InitZedData()
	{
		if (!IsAlive())
			return;

		m_ZedData = GetZombieDoorsConfig().GetZombieData(GetType());
		bool canHitDoor = GetZombieDoorsConfig().KnockDownDoors && GetZombieDoorsConfig().DoorHitCount > 0;

		if (m_ZedData)
			canHitDoor = m_ZedData.KnockDownDoors && m_ZedData.DoorHitCount > 0;

		// If door hitting is disabled globally or for this zed type, stop here & do not start door search loop.
		if (!canHitDoor)
			return;
	}

	// START DOORS /////////////////////////////////////////////////////////////////////////////
	const static float DECTECT_DOORS_TIMER = 1000;
	const static float DOOR_HIT_TIMER = 2000;
	private bool m_DoorHitPaused;

	// Detect mind state change. If zed is aggro'd, start door checker
	override bool HandleMindStateChange(int pCurrentCommandID, DayZInfectedInputController pInputController, float pDt)
	{
		m_MindState = pInputController.GetMindState();
		if (m_LastMindState != m_MindState && m_MindState >= 2)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(DetectDoors);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DetectDoors, DECTECT_DOORS_TIMER, false);
		}

		return super.HandleMindStateChange(pCurrentCommandID, pInputController, pDt);
	}

	// Get zombie's movement speed (meters per second) - only attack doors when zed is not sprinting around
	private float GetMovementSpeed()
	{
		if (GetInputController())
			return GetInputController().GetMovementSpeed();

		return -1;
	}

	// Loop: Detect nearby doors and attack them if aggro'd
	private void DetectDoors()
	{
		// If dead or crawling, don't hit doors
		if (!IsAlive() || m_IsCrawling)
			return;

		// Check for nearby doors
		if (m_MindState >= 2 && !m_DoorHitPaused)
		{
			// GetDoor() only returns doors within 2.5m of zombie
			ZenDoorData doorData = ZedDoorsSaved.GetDoor(GetPosition());
			if (doorData)
			{
				AttackDoor(doorData);
			}
		}

		// If zombie is alive and not a crawler, loop this door check
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DetectDoors, DECTECT_DOORS_TIMER + Math.RandomIntInclusive(0, DECTECT_DOORS_TIMER), false);
	}

	// Attack the given door and once it has been hit X number of times, open it
	private void AttackDoor(ZenDoorData doorData)
	{
		// After a short delay, re-enable door hitting (call this here so that the return; calls don't break the door checks)
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ReactivateDoorHitting, DOOR_HIT_TIMER + Math.RandomFloatInclusive(0, 1000), false);

		// If door data doesn't exist, zombie is running, or we're not close enough, stop here.
		if (!doorData || GetMovementSpeed() > 2)
			return;

		// If door is already opened, or locked and cannot be bashed open, stop here
		Building building = doorData.building;

		int doorIndex = doorData.index;
		if (!building || building.IsDoorOpen(doorIndex))
		{
			doorData.ResetDoorHitCount();
			return;
		}

		// Send attack animation sync
		int attackType = Math.RandomIntInclusive(9, 13);
		m_ActualAttackType = GetDayZInfectedType().ChooseAttack(DayZInfectedAttackGroupType.FIGHT, 1, 0);
		DayZInfectedCommandAttack dayZInfectedCommandAttackTest = StartCommand_Attack(NULL, attackType, 1);

		// Check max hit config
		int maxDoorCount = 99;
		if (m_ZedData)
		{
			if (building.IsDoorLocked(doorIndex))
				maxDoorCount = m_ZedData.DoorHitLockedCount;
			else
				maxDoorCount = m_ZedData.DoorHitCount;
		}
		else
		{
			if (building.IsDoorLocked(doorIndex))
				maxDoorCount = GetZombieDoorsConfig().DoorHitLockedCount;
			else
				maxDoorCount = GetZombieDoorsConfig().DoorHitCount;
		}

		// Check if this zombie has custom config or if we should use global config
		bool canHitLockedDoor = false;
		if (m_ZedData)
			canHitLockedDoor = !building.IsDoorLocked(doorIndex) || m_ZedData.KnockDownLockedDoors;
		else
			canHitLockedDoor = !building.IsDoorLocked(doorIndex) || GetZombieDoorsConfig().KnockDownLockedDoors;

		// Only increase door hit count if we can bash open locked doors
		if (canHitLockedDoor)
		{
			// Hit the door
			bool openedDoor = false;
			doorData.HitDoor();

			// Check if it should open
			if (doorData.hitCount >= maxDoorCount)
			{
				// If we're allowed to hit unlocked doors, unlock it now
				if (building.IsDoorLocked(doorIndex))
					building.UnlockDoor(doorIndex);

				// Force open door
				building.OpenDoor(doorIndex);
				openedDoor = true;

				// Remove door from list
				ZedDoorsSaved.RemoveDoor(doorData.building, doorData.pos, doorData.index);
			}
		}

		// Trigger server-side noise (not a sound, attracts AI like other zombies)
		ref NoiseParams noiseParams();
		noiseParams.LoadFromPath("CfgSoundSets zombie_Hit_Door_SoundSet Noise");
		GetGame().GetNoiseSystem().AddNoiseTarget(doorData.pos, 1.0, noiseParams, 1.0);

		// Send door noise to nearby clients
		array<Man> players = new array<Man>;
		g_Game.GetWorld().GetPlayerList(players);
		for (int x = 0; x < players.Count(); x++)
		{
			// If player is over 50m away they're not going to hear the sound so don't send RPC.
			if (vector.Distance(players.Get(x).GetPosition(), doorData.pos) < 50)
			{
				// Send actual sound trigger to client
				GetRPCManager().SendRPC("ZZ_RPC", "RPC_ZZ_SendBreakDoorData", new Param4< float, float, float, bool >(doorData.pos[0], doorData.pos[1], doorData.pos[2], attackType == 12), true, players.Get(x).GetIdentity());
			}
		}

		// Pause hitting the door temporarily, then reset after 1-2 secs
		m_DoorHitPaused = true;
	}

	// Enable hitting doors
	private void ReactivateDoorHitting()
	{
		m_DoorHitPaused = false;
	}
	// END DOORS /////////////////////////////////////////////////////////////////////////////
	#endif
}