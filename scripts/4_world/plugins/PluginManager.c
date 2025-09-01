modded class PluginManager
{
	override void Init()
	{
		super.Init();

		#ifdef SERVER
		#ifdef ZENMODPACK
		if (ZenModEnabled("ZenZombieDoors"))
		{
			// Server-side plugin
			RegisterPlugin("ZenZombieDoorsPlugin", false, true);
		}
		#else
		RegisterPlugin("ZenZombieDoorsPlugin", false, true);
		#endif
		#endif
	}
}