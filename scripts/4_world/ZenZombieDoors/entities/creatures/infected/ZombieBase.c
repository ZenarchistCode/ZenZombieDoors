modded class ZombieBase extends DayZInfected
{
	// All of this is server-side only
	#ifdef SERVER

	private float m_MovementSpeed;
	private int m_MindState;
	private ref autoptr ZenZombieData m_ZedData;
	private ref autoptr NoiseParams m_NoiseParams;

	// Called when zombie is created
	override void EOnInit(IEntity other, int extra)
	{
		super.EOnInit(other, extra);

		// Get zombie data
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitZedData, Math.RandomFloatInclusive(1000, 1500), false);
	}

	// Called when zombie is deleted
	void ~ZombieBase()
	{
		delete m_ZedData;
		delete m_NoiseParams;
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

		// Prepare noise parameters
		m_NoiseParams = new NoiseParams();
	}

	// START DOOR HANDLING /////////////////////////////////////////////////////////////////////////////
	private bool m_DoorHitPaused;

	// Detect mind state change. If zed is aggro'd, start door checker
	override bool HandleMindStateChange(int pCurrentCommandID, DayZInfectedInputController pInputController, float pDt)
	{
		m_MindState = pInputController.GetMindState();
		if (m_LastMindState != m_MindState && m_MindState >= 2)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(DetectDoors);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DetectDoors, GetZombieDoorsConfig().DetectDoorTimer, false);
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
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DetectDoors, GetZombieDoorsConfig().DetectDoorTimer + Math.RandomIntInclusive(0, GetZombieDoorsConfig().DetectDoorTimer), false);
	}

	// Attack the given door and once it has been hit X number of times, open it
	private void AttackDoor(ZenDoorData doorData)
	{
		// After a short delay, re-enable door hitting (call this here so that the return; calls don't break the door checks)
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ReactivateDoorHitting, GetZombieDoorsConfig().HitDoorDelay + Math.RandomFloatInclusive(0, 1000), false);

		// If door data doesn't exist or zombie is running, stop here.
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
				openedDoor = true;
		}

		TriggerZombieDoorSoundFX(doorData, attackType == 12, openedDoor);

		// Pause hitting the door temporarily, then reset after 1-2 secs
		m_DoorHitPaused = true;
	}

	// Disable hitting doors if zombie recently hit a player
	override bool FightLogic(int pCurrentCommandID, DayZInfectedInputController pInputController, float pDt)
	{
		if (pCurrentCommandID == DayZInfectedConstants.COMMANDID_ATTACK)
		{
			if (m_ActualAttackType.m_Subtype != 1) // Subtype 1 = door attack animation
			{
				m_DoorHitPaused = true;
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReactivateDoorHitting);
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ReactivateDoorHitting, GetZombieDoorsConfig().HitDoorDelay + Math.RandomFloatInclusive(0, 1000), false);
			}
		}

		return super.FightLogic(pCurrentCommandID, pInputController, pDt);
	};

	// Enable hitting doors
	private void ReactivateDoorHitting()
	{
		m_DoorHitPaused = false;
	}

	// Trigger sound effect & open door
	private void TriggerZombieDoorSoundFX(ZenDoorData doorData, bool doubleHit, bool openedDoor)
	{
		// Orientate zombie towards door
		OrientateZombieToDoor(doorData.pos);

		// Delay sound to match animation
		float delay = Math.RandomFloatInclusive(300, 500);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnZombieDoorSoundFX, delay, false, doorData.pos);

		if (!doubleHit)
		{
			// If door was opened, open it after sound delay
			if (openedDoor)
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZombieOpenedDoor, delay + 10, false, doorData);
		}
		else
		{
			// If attackType == 12 then the zed does a double-hit, so play sound twice
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnZombieDoorSoundFX, delay * 3, false, doorData.pos);

			if (openedDoor)
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ZombieOpenedDoor, (delay * 3) + 10, false, doorData);
		}
		
	}

	// Trigger sound effect
	private void SpawnZombieDoorSoundFX(vector pos)
	{
		// Trigger server-side noise (not a sound, attracts AI like other zombies)
		m_NoiseParams.LoadFromPath("CfgSoundSets zombie_Hit_Door_SoundSet Noise");
		GetGame().GetNoiseSystem().AddNoiseTarget(pos, 1.0, m_NoiseParams, 1.0);

		// Spawn sound effect
		GetGame().CreateObject("ZenZombieDoorBangerBang", pos);

		// Orientate zombie towards door
		OrientateZombieToDoor(pos);
	}

	// Open the door
	private void ZombieOpenedDoor(ZenDoorData doorData)
	{
		// If we're allowed to hit unlocked doors, unlock it now
		if (doorData.building.IsDoorLocked(doorData.index))
			doorData.building.UnlockDoor(doorData.index);

		// Force open door
		doorData.building.OpenDoor(doorData.index);

		// Remove door from list
		ZedDoorsSaved.RemoveDoor(doorData.building, doorData.pos, doorData.index);
	}

	// Orientate zombie towards position
	private void OrientateZombieToDoor(vector pos)
	{
		// Check server config
		if (!GetZombieDoorsConfig().OrientateToDoor)
			return;

		vector angles = vector.Direction(pos, GetPosition()).Normalized().VectorToAngles();
		SetDirection(GetPosition() - pos);
		SetOrientation(Vector(angles[0] - 180, 0, 0));
		SetPosition(GetPosition());
	}

	// END DOOR HANDLING /////////////////////////////////////////////////////////////////////////////
	#endif
}