# Overview
Tired of re-doing your color preset between each game or being limited by the low number of swatch slots? Well, this mod is made for you!

This mod allows you to create your own palette of swatches and apply it to any session game you want. Meaning that you can share swatches between different session or even with other players by simply copying their JSON configuration!
This JSON configuration gives you a lot of options to customize your palette as you wish:
- You can create groups to oragnize your swatches as you want.
- You can name each of your swatch as you want (duplicates are allowed and will be treated as distinct slot).
- You can define priority between groups and swatches in order to make them appear the way you want.
- You can tell the mod to add the primary and / or the secondary colors of your swatches to the player color preset list.

Two default palettes come with this mod. Each contains the same dinstinct swatches for each craftable item in the game, only the language change (one in english, one in french).

# How to Use
In order to make the swatch slots appear in your game you need to configure the mod directly via the ingame menu or by editing the JSON file located at "YourStatifactoryGameFolder/FactoryGame/Configs/UniversalSwatchSlots.cfg"
- **Step 1:** The first array represent the palette / session associations. This is where you tell the mod which palette to apply to which session game (be sure to use the session name and not the save game name). You can also configure the "add color swatches to player preset" behavior. 
- **Step 2:** The second array represent your available palettes. You can add / edit / delete up to <span style="color:red">255 slots (including default + custom : 29, More Swatch Slots : 20 and other mod that could add more swatches)</span> but keep in mind that the more you have the longer it takes for the ingame menu to display. The fields are self explenatory and have tooltip to help you understand their purpose.
- **Step 3:** Launch a save game that belongs to session your entered.
- **Step 4:** Go to your nearest Awesome Shop, customization tab, and buy the Universal Swatch Slots upgrade for only 1 coupon !!!

Note: I recommand to directly edit the JSON configuration as the ingame menu can take a lot of time to display everything. Be sure to have launched the game at least one time with this mod in order to make the default configuration appears.

# Multiplayer

This mod has been tested on windows dedicated server but should also work with linux server. To make it work, simply edit the UniversalSwatchSlots.cfg server file to match the session name you want to apply the mod to. Normally, no joining player doesn't need to set up any configuration on their side but if you're experiencing issue (like swatch slots don't appear) make sure to have to have the same configuration file on both sides.

When playing in host / client mod make sure that all players share the same configuration file (or at least the same palette and palette association than the host has).

# Images

![Configuration](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/Config.png?raw=true)
![Awesome Shop](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/AS_Buy.png?raw=true)
![Swatches](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/Swatches.png?raw=true)
![Color preset](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/Color_preset.png?raw=true)

# Q/A

**Is this mod compatible with More Swatch Slots ?**

More or less:
- If More Swatch Slots was installed before then nothing terrible should happen. <span style="color:red">Please note that your MSS slots may turn black the first time but they will keep their color slot distinct from the newly added ones.</span>
- If you've installed this mod first and decided to add More Swatch Slot (or other mods that add swatches at pre-defined index) then all your swatches will share colors slots (e.g. MSS slots will share the same 20 first slots of your palette meaning that one you change one you change the other).
Custom slots added by More Swatch Slots are not concerned.

**I have deleted some swatches in the default configuration and want them back, how do I do ?**

Simply delete or move your configuration file to another location and launch the game again. You can then edit the JSON to put your own palette back.

**What happen if I have applied a swatch to a building and remove it from the palette ?**

It depends:
- If you remove it without adding another swatch it will turn black or default.
- If your remove it and add one or more swatches it will take the color of the first one.

To revert it back, just re-add a new swatch with the same name anywhere in your palette.

 <span style="color:yellow">Note: if you removed one swatch without adding one, I strongly advise you to reaload your game as just reloading your save could let traces of previous swatch and lead to undifined behavior !</span>

**What happen if I have applied swatches of a palette and want to switch to another ?**

If your palette has the same swatch names as the one you applied nothing wrong should happend (you will see warning in log for all other missing swatch but as long as they are not used you are good to go).
Otherwise: 
- If your new palette has less swatches than your previous one only the buildings that uses the first swatches (as many as your new palette has) will be changed, all other extra slots of your old palette should turned to black or default.
- If your new palette has more or equal number of swatches than your previous one all the swatches will be modified. 

**What happen if I have applied swatches of this mod to my building and uninstall the mod ?**

They will turn black or back to their default color.
Caution: If you save your game and reinstall the mod your building colors will be not restored!
Note: If you have selected the "add color to player preset" these colors will remain in your game. You can delete them by clicking on the trash icon

# Support
For questions or support, contact:
- **Issue Tracker:** [GitHub Issues](https://github.com/Loupimo/UniversalSwatchSlots/issues)

PS : This is my first mod so don't hesitate to report any issue
