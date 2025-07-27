What Is This?

This mod adds the ability for zombies/infected to bash open doors.

It's inspired by Liven's PvZ Mod, but uses a slightly different method for detecting door locations based on their open/close sound position.

This mod does not touch anything else to do with zombies. It just makes aggro'd zombies hit any doors they are standing near until they open.

There is a JSON config with the mod that allows you to tweak how many hits it takes to open a door, whether or not locked doors can be opened, and you can also adjust how many door hits are required to open a door based on the zombie type.

Only doors that have been closed or locked with a Lockpick by players are detectable by zombies in my version of the mod. In my experience this is usually fine, as most players are only hunted by zombies into a building when they've shut a door behind them anyway.

Each door has a unique hit counter associated with it, so if you get swarmed by zeds they can bash open the door pretty quickly.

Known Issues:

Sometimes they will wander off before the door opens, but I think this is a good thing as it adds an element of randomness to them. Sometimes they will bust open your house, sometimes they will lose interest and wander off. To decrease the chances of this happening just set the door hit count to a lower number than the default.

There may be other bugs, but after running this on my servers for several months I haven't noticed any.

Installation Instructions:

Install this mod like any other mod - copy it into your server folder and add it to your mods list. It must be run on both server and client. Make sure to copy the .bikey into your server keys if you're not using a server management tool like OmegaManager which does that automatically.

When you run the mod for the first time a default JSON config will be created in your server profile: %server_root%/profiles/Zenarchist/ZenZombieDoors.json

This is where you can tweak the zombie door banger config values such as how many hits a door takes to open.

Repack & Source Code:

You can repack this mod if you like, and do anything else you want with it for that matter. Just keep in mind my future updates and improvements won't be applied so make sure to check back for new versions if you notice any bugs. The source code is on my GitHub at www.zenarchist.io

Enjoy!