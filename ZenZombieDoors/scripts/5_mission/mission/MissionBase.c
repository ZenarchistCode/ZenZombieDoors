modded class MissionBase extends MissionBaseWorld
{
	void MissionBase()
	{
		// Register server->client RPCs
		if (!GetGame().IsDedicatedServer())
			GetRPCManager().AddRPC("ZZ_RPC", "RPC_ZZ_SendBreakDoorData", this, SingeplayerExecutionType.Server);
	}

	// SERVER->CLIENT RPCS /////////////////////////////////////////////////////////////////////////////
	void RPC_ZZ_SendBreakDoorData(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)
	{
		if (type == CallType.Client && !GetGame().IsDedicatedServer())
		{
			Param4< float, float, float, bool > data;
			if (!ctx.Read(data))
			{
				Print("[ZenZombieDoors] RPC_ZZ_SendBreakDoorData - sound sync error");
				return;
			}
			
			// Get door pos
			vector theSoundPosition = vector.Zero;
			theSoundPosition[0] = data.param1;
			theSoundPosition[1] = data.param2;
			theSoundPosition[2] = data.param3;

			// Delay sound to match animation
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PlayDelayedDoorHitSound, Math.RandomFloatInclusive(300, 500), false, theSoundPosition);

			// If attackType == 12 then the zed does a double-hit, so play sound twice
			if (data.param4)
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PlayDelayedDoorHitSound, Math.RandomFloatInclusive(300, 500) * 3, false, theSoundPosition);
		}
	}

	// Delay playing of sound based on animation
	private void PlayDelayedDoorHitSound(vector pos)
	{
		pos[0] = pos[0] + Math.RandomFloatInclusive(-0.1, 0.1);
		pos[1] = pos[1] + Math.RandomFloatInclusive(-0.1, 0.1);
		pos[2] = pos[2] + Math.RandomFloatInclusive(-0.1, 0.1);
		SEffectManager.PlaySound("zombie_Hit_Door_SoundSet", pos);
	}
	// END CLIENT RPCS /////////////////////////////////////////////////////////////////////////////
}