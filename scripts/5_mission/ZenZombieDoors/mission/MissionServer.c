modded class MissionServer
{
	// Load config on server startup
	override void OnInit()
	{
		super.OnInit();
		GetZombieDoorsConfig();
	}
}