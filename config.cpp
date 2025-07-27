class CfgPatches
{
	class ZenZombieDoors
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[] = 
		{ 
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Sounds_Effects"
		};
	};
};

class CfgMods
{
	class ZenZombieDoors
	{
		dir="ZenZombieDoors";
		picture=""; 
		action="";
		hideName=1;
		hidePicture=1;
		name="ZenZombieDoors";
		credits="Liven"; 
		author="Zenarchist";
		authorID="0";  
		version="1.0";
		extra=0;
		type="mod";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[] = { "ZenZombieDoors/scripts/3_game"; };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "ZenZombieDoors/scripts/4_world" };
			};
			class missionScriptModule
			{
				value="";
				files[] = { "ZenZombieDoors/scripts/5_mission" };
			};
		}
	};
};

class CfgSoundShaders
{
	class zombie_Hit_Door_SoundShader
	{
		samples[] =
		{
			{
				"\DZ\sounds\vehicles\shared\collisions\offroad_hit_character_1",
				1
			},
			{
				"\DZ\sounds\vehicles\shared\collisions\offroad_hit_character_2",
				1
			},
			{
				"\DZ\sounds\vehicles\shared\collisions\offroad_hit_character_3",
				1
			},
			{
				"\DZ\sounds\vehicles\shared\collisions\offroad_hit_character_4",
				1
			},
			{
				"\DZ\sounds\vehicles\shared\collisions\offroad_hit_character_5",
				1
			},
			{
				"\DZ\sounds\vehicles\shared\collisions\offroad_hit_character_6",
				1
			}
		};
		volume = 1;
		range = 40;
	};
};

class CfgSoundSets
{
	class baseVehicles_SoundSet;
	class zombie_Hit_Door_SoundSet : baseVehicles_SoundSet
	{
		soundShaders[] =
		{
			"zombie_Hit_Door_SoundShader"
		};
		volumefactor = 1;
		frequencyrandomizer = 1;
		volumerandomizer = 1;
		class Noise
		{
			strength = 20;
			type = "sound";
		};
	};
};
