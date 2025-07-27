modded class PluginManager
{
	override void Init()
	{
		super.Init();

		#ifdef ZENMODPACK
		if (ZenModEnabled("ZenZombieDoors"))
		{
			if (ZenModEnabled("ZenZombieDoors"))
			{
				// Server-side plugin
				RegisterPlugin("ZenZombieDoorsPlugin", false, true);
			}
		}
		#endif
	}
}