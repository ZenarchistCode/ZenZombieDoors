modded class ZombieBase
{
	static const float ZENMOD_BUILDING_SEARCH_RADIUS = 6.0;

	protected static ref ZenZombieDoorsPlugin m_ZenDoorPlugin;
	protected ref NoiseParams m_ZenDoorNoiseParams;
	protected ref ZenZombieData m_ZenZombieData;
	protected float m_ZenDoorsCheckTimer;
	protected bool m_ZenDoorHittingPaused;
	protected bool m_PlayZenZombieDoorHitSound;

	override void Init()
	{
		super.Init();

		RegisterNetSyncVariableBoolSignal("m_PlayZenZombieDoorHitSound");
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		CheckZenDoorSound();
	}

	void CheckZenDoorSound()
	{
		if (!m_PlayZenZombieDoorHitSound)
			return;

		vector pos = GetPosition();
		pos[0] = pos[0] + Math.RandomFloatInclusive(-0.1, 0.1);
		pos[1] = pos[1] + 1 + Math.RandomFloatInclusive(-0.1, 0.1);
		pos[2] = pos[2] + Math.RandomFloatInclusive(-0.1, 0.1);
		SEffectManager.PlaySound("Zombie_Hit_Door_SoundSet", pos);
	}

	override void EOnInit(IEntity other, int extra)
	{
		super.EOnInit(other, extra);

		#ifdef ZENMODPACK
		if (!ZenModEnabled("ZenZombieDoors"))
			return;
		#endif

		SetupZombieDoors();
	}

	override bool ModCommandHandlerInside(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		bool superman = super.ModCommandHandlerInside(pDt, pCurrentCommandID, pCurrentCommandFinished);

		#ifdef ZENMODPACK
		if (!ZenModEnabled("ZenZombieDoors"))
			return superman;
		#endif

		if (GetMindStateSynced() <= DayZInfectedConstants.MINDSTATE_DISTURBED || !IsAlive())
			return superman;

		m_ZenDoorsCheckTimer += pDt;

		if (m_ZenDoorsCheckTimer > GetZombieDoorsConfig().HitDoorDelaySecs)
		{
			m_ZenDoorsCheckTimer = 0;
			CheckBuildingDoors();
		}

		return superman;
	}

	void SetupZombieDoors()
	{
		m_ZenDoorsCheckTimer = 0;
		m_ZenDoorHittingPaused = false;

		m_ZenZombieData = GetZombieDoorsConfig().GetZombieData(GetType());

		bool canHitDoor = GetZombieDoorsConfig().KnockDownDoors && GetZombieDoorsConfig().DoorHitCount > 0;

		if (m_ZenZombieData)
			canHitDoor = m_ZenZombieData.KnockDownDoors && m_ZenZombieData.DoorHitCount > 0;

		if (canHitDoor && !m_ZenDoorNoiseParams)
		{
			m_ZenDoorNoiseParams = new NoiseParams();
			m_ZenDoorNoiseParams.LoadFromPath("CfgSoundSets Zombie_Hit_Door_SoundSet Noise");
		}
	}

	ZenZombieDoorsPlugin GetZenDoorsPlugin()
	{
		if (!m_ZenDoorPlugin)
		{
			m_ZenDoorPlugin = ZenZombieDoorsPlugin.Cast(GetPlugin(ZenZombieDoorsPlugin));
		}

		if (!m_ZenDoorPlugin)
		{
			Error("BAD ERROR: Couldn't retrieve ZenZombieDoorsPlugin!");
		}

		return m_ZenDoorPlugin;
	}

	void CheckBuildingDoors()
	{
		if (m_ZenDoorHittingPaused)
			return;

		vector myPos = GetPosition();
		float rangeSq = ZENMOD_BUILDING_SEARCH_RADIUS * ZENMOD_BUILDING_SEARCH_RADIUS;

		

		foreach (ZenBuildingData b : GetZenDoorsPlugin().GetAllBuildings())
		{
			if (!b.m_Building)
			{
				continue;
			}

			if (vector.DistanceSq(myPos, b.m_Building.GetPosition()) <= rangeSq)
			{
				HandleBuildingDoors(b);
				break;
			}
		}
	}

	void HandleBuildingDoors(notnull ZenBuildingData bData)
	{
		if (!bData.m_Building)
			return;

		vector myPos = GetPosition();
		vector doorWorldPos;
		vector bestDoorWorldPos = vector.Zero;
		vector bestDoorLocalPos = vector.Zero;
		int bestDoorIndex = -1;
		float bestDist = float.MAX;

		array<Man> players = new array<Man>;
		array<Man> nearbyPlayers = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (Man p : players)
		{
			if (vector.DistanceSq(myPos, p.GetPosition()) <= 25.0)
				nearbyPlayers.Insert(p);
		}

		if (nearbyPlayers.Count() == 0)
			return;

		for (int i = 0; i < bData.m_Doors.Count(); i++)
		{
			ZenZombieDoorData door = bData.m_Doors[i];

			if (bData.m_Building.IsDoorOpen(door.m_DoorIndex))
				continue;

			doorWorldPos = GetGame().ObjectModelToWorld(bData.m_Building, door.m_LocalPos);

			if (vector.Distance(myPos, doorWorldPos) > 2)
				continue;

			foreach (Man player : nearbyPlayers)
			{
				if (vector.Distance(myPos, player.GetPosition()) < 0.4)
					return; // player too close, ignore the door and focus on player

				float dist = vector.Distance(GetPosition(), doorWorldPos);
				if (dist < bestDist)
				{
					bestDoorLocalPos =  door.m_LocalPos;
					bestDoorWorldPos = doorWorldPos;
					bestDoorIndex = door.m_DoorIndex;
					bestDist = dist;
				}
			}
		}

		if (bestDoorWorldPos == vector.Zero)
			return;

		int attackType = Math.RandomIntInclusive(9, 13);
		m_ActualAttackType = GetDayZInfectedType().ChooseAttack(DayZInfectedAttackGroupType.FIGHT, 1, 0);
		StartCommand_Attack(NULL, attackType, 1);

		int hitCount = GetZenDoorsPlugin().GetDoorHitCount(bData.m_Building, bestDoorWorldPos);
		bool openedDoor = false;

		if (m_ZenZombieData)
			openedDoor = m_ZenZombieData.KnockDownDoors && hitCount >= m_ZenZombieData.DoorHitCount;
		else
			openedDoor = GetZombieDoorsConfig().KnockDownDoors && hitCount >= GetZombieDoorsConfig().DoorHitCount;

		TriggerZombieDoorSoundFX(bData, bestDoorWorldPos, bestDoorLocalPos, bestDoorIndex, attackType == 12, openedDoor);
	}

	void TriggerZombieDoorSoundFX(notnull ZenBuildingData bData, vector doorWorldPos, vector doorLocalPos, int doorIndex, bool doubleHit, bool openedDoor)
	{
		if (!bData.m_Building)
			return;

		OrientateZombieToDoor(doorWorldPos);

		float delay = Math.RandomFloatInclusive(300, 500);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(SpawnZombieDoorSoundFX, delay, false, doorWorldPos, doorLocalPos);

		if (openedDoor)
		{
			if (!doubleHit)
			{
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ZombieOpenedDoor, delay + 100, false, bData, doorIndex, doorLocalPos);
			} else
			{
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ZombieOpenedDoor, (delay * 3) + 100, false, bData, doorIndex, doorLocalPos);
			}
		}

		if (doubleHit)
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(SpawnZombieDoorSoundFX, delay * 3, false, doorWorldPos, doorLocalPos);

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(ResetDoorHitCounter);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ResetDoorHitCounter, GetZombieDoorsConfig().DoorHitCounterTimerSecs * 1000, false, bData, doorLocalPos);
	}

	void ZombieOpenedDoor(notnull ZenBuildingData bData, int doorIdx, vector doorLocalPos)
	{
		if (!bData.m_Building)
			return;

		if (bData.m_Building.IsDoorLocked(doorIdx))
			bData.m_Building.UnlockDoor(doorIdx);

		bData.m_Building.OpenDoor(doorIdx);
		GetZenDoorsPlugin().ResetDoorHitCount(bData, doorLocalPos);
	}

	void ResetDoorHitCounter(notnull ZenBuildingData bData, vector doorLocalPos)
	{
		GetZenDoorsPlugin().ResetDoorHitCount(bData, doorLocalPos);
	}

	void SpawnZombieDoorSoundFX(vector doorWorldPos)
	{
		GetGame().GetNoiseSystem().AddNoiseTarget(doorWorldPos, 10, m_ZenDoorNoiseParams, 1.0);
		m_PlayZenZombieDoorHitSound = true;
		SetSynchDirty();
	}

	void OrientateZombieToDoor(vector targetPos)
	{
		if (!GetZombieDoorsConfig().OrientateToDoor)
			return;

		vector direction = targetPos - GetPosition();
		float yawRadians = Math.Atan2(direction[0], direction[2]);
		float yawDegrees = yawRadians * Math.RAD2DEG;
		if (yawDegrees < 0)
			yawDegrees += 360;

		vector ori = GetOrientation();
		ori[0] = yawDegrees;
		SetOrientation(ori);
		SetSynchDirty();
	}

	override bool FightLogic(int pCurrentCommandID, DayZInfectedInputController pInputController, float pDt)
	{
		if (pCurrentCommandID == DayZInfectedConstants.COMMANDID_ATTACK)
		{
			if (!m_ActualAttackType)
				return false;

			if (m_ActualAttackType.m_Subtype != 1)
			{
				m_ZenDoorHittingPaused = true;
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(ReactivateDoorHitting);
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ReactivateDoorHitting, GetZombieDoorsConfig().HitDoorDelaySecs + Math.RandomFloatInclusive(3000, 6000), false);
			}
		}

		return super.FightLogic(pCurrentCommandID, pInputController, pDt);
	}

	void ReactivateDoorHitting()
	{
		m_ZenDoorHittingPaused = false;
	}

	void ~ZombieBase()
	{
		if (GetGame() && GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM))
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(SpawnZombieDoorSoundFX);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ZombieOpenedDoor);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ResetDoorHitCounter);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ReactivateDoorHitting);
		}
	}
}
