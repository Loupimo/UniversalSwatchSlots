# Overview
Tired of re-doing your color preset between each game or being limited by the low number of swatch slots? Well, this mod is made for you!

This mod allows you to create your own palette of swatches and apply it to any session game you want. Meaning that you can share swatches between different session or even with other players by simply copying their JSON configuration!
This JSON configuration gives you a lot of options to customize your palette as you wish:
- You can create groups to organize your swatches as you want.
- You can name each of your swatch as you want (duplicates are allowed and will be treated as distinct slot).
- You can define priority between groups and swatches in order to make them appear the way you want.
- You can tell the mod to add the primary and / or the secondary colors of your swatches to the player color preset list. If you think later on that was a bad idea there is an option to only remove swatches that are prensents in the palette you activate.

A default palette comes with this mod and is activated by default. It contains the dinstinct swatches for each craftable item in the game. All the names (groups and swatches) are based on your game language. It is currently availble for english, french, spanish and german (english will be chosen by default if your language is not supported).

# How to Use
In order to make the swatch slots appear in your game you need to configure the mod directly via the ingame menu or by editing the JSON file located at "YourStatifactoryGameFolder/FactoryGame/Configs/UniversalSwatchSlots/YourPaletteName.json"
- **Step 1:** If you don't want to use the default palette your can create as many palettes as you want using the mod configueration menu.  
You can add / edit / delete up to <span style="color:red">226 slots (default SF slots goes from 1 to 28 and slot 255 is for custom swatch). If you have More Swatch Slots or Extra Finishes installed it reduce the possibilities (MSS has 20 slots, EF has 15 but start at index 32 so consider 19)</span>. The fields are self explanatory and have tooltip to help you understand their purpose.  
Once your are happy with your palette don't forget to tick the checkbox to activate it.
- **Step 3:** Start a game.
- **Step 4:** Go to your nearest Awesome Shop, customization tab, and buy the Universal Swatch Slots upgrade for only 1 coupon !!!

<span style="color:red">Important note: If you change a swatch color in game it will not be saved in your configuration and will be lost once you reload your save !</span>

# Multiplayer

This mod has been tested on windows dedicated server but should also work with linux server. You don't need to activate anything: the server will use the default mod's palette. If you wish to use a custom palette simply copy the "UniversalSwatchSlots" folder located at "YourStatifactoryGameFolder/FactoryGame/Configs/" to your server configs folder. Don't forget to activate the palette you want first ! 
Joining players need to use the same configuration as the server's one. They may have a different naming as long as the number of swatches and order remain the same.

Note: The swatch color are copied from the server to the player so changing a color swatch in your mod configuration will not affect the server. If your server doesn't load the correct palette it means that you have another activated palette that has a higher priority (file name based).

# New Feature

Since version 1.3.0 this mod adds a new feature ! You can now press "R" when holding the customizer in hand to switch to blueprint mod painting. This allows you to paint all buildings of a same blueprint at once ! It works with USS swatches, vanilla swatches, patterns and materials.

Normally when using the material customizer, the targeted building will no longer be part of the blueprint it used to belong to. Here it will still be part of it after it changed. Currently, you will not be charged for modifying materials however you will not get refunded by the previous needed materials

Also you can preview the changes before applying them. For performance reason, previewing material changes is not possible.

If you are playing on a dedicated server or joining someone else you can activate the preview by tick the corresponding option in the mod configuration (<span style="color:yellow">Caution: Activating it may, on very rare cases, causes a crash client side!</span>)

Note: Changing color or material of a blueprint placed in game does not affect the original one.

![Awesome Shop](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/AS_Buy.png?raw=true)
![Swatches](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/Swatches.png?raw=true)
![Color preset](https://github.com/Loupimo/UniversalSwatchSlots/blob/main/Resources/Color_preset.png?raw=true)

# Q/A

**Is this mod compatible with More Swatch Slots ?**

Now it should be fully compatible with MSS and not overwrite slots anymore and this regardless when you install / uninstall MSS

**I have deleted some swatches in the default configuration and want them back, how do I do ?**

Simply rename / delete the palette or move its json file to another location and launch the game again. The palette will be back as normal.

**I want to copy the default palette, how do I do ?**

Option 1: using the mod configuration menu simply rename the default palette's name. Do your changes, quit and relaunch the game. The default palette will reappear as well as your new palette.  
Option 2: go to "YourStatifactoryGameFolder/FactoryGame/Configs/UniversalSwatchSlots/" folder and copy paste the palette file you want to use as base.

**What happen if I have applied a swatch to a building and remove it from the palette ?**

It will take the color of the next swatch in your palette.
If your palette has no swatch or the swatch you removed was the last one, it will switch back to default.

**What happen if I have applied swatches of a palette and want to switch to another ?**

If your palette has the same swatch names as the one you applied nothing wrong should happend (you will see warning in log for all other missing swatch but as long as they are not used you are good to go).
Otherwise: 
- If your new palette has less swatches than your previous one only the buildings that uses the first swatches (as many as your new palette has) will be changed, all other extra slots of your old palette should turned to black or default.
- If your new palette has more or equal number of swatches than your previous one all the swatches will be modified.

**What happen if I have applied swatches of this mod to my building and uninstall the mod ?**

They will turn black or back to their default color.

Note: If you have selected the "add color to player preset" these colors will remain in your game. You can delete them by clicking on the trash icon.

 <span style="color:yellow">Caution: If you save your game and reinstall the mod your building colors will be not restored!</span>


 <span style="color:red">Big warning here: if you have selected one or more of the swatches as a default color group be sure to uncheck them before uninstalling otherwise you will not be able to load your save without the mod !</span>

# Support & Translations
For questions or support, contact:
- **Issue Tracker:** [GitHub Issues](https://github.com/Loupimo/UniversalSwatchSlots/issues)

If you wish to propose a translation of the default palette, please use this link:
- **Tanslations:** [GitHub Translations](https://github.com/Loupimo/UniversalSwatchSlots/discussions/7)

If you wish to share your own beautiful palette with other you can do it here:
- **Share Your Palette**:  [GitHub Palettes](https://github.com/Loupimo/UniversalSwatchSlots/discussions/8)

❤️ Support This Mod

This mod will always remain free and open source.

If you enjoy using it and would like to support future updates, bug fixes, and new projects, consider leaving a tip. Support is completely optional, but always appreciated.

☕ [Buy Loupimo a Coffee](https://ko-fi.com/loupimo)

Thank you for helping keep these projects alive!