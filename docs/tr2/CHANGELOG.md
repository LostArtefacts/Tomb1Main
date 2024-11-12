## [Unreleased](https://github.com/LostArtefacts/TRX/compare/tr2-0.6...develop) - ××××-××-××
- added support for custom levels to enforce values for any config setting (#1846)
- added an option to fix inventory item usage duplication (#1586)
- fixed depth problems when drawing certain rooms (#1853, regression from 0.6)
- fixed Lara getting stuck in her hit animation if she is hit while mounting the boat or skidoo (#1606)
- fixed being unable to go from surface swimming to underwater swimming without first stopping (#1863, regression from 0.6)
- fixed pistols appearing in Lara's hands when entering the fly cheat during certain animations (#1874)
- fixed Lara continuing to walk after being killed if in that animation (#1880, regression from 0.1)

## [0.6](https://github.com/LostArtefacts/TRX/compare/tr2-0.5...tr2-0.6) - 2024-11-06
- added a fly cheat key (#1642)
- added an items cheat key (#1641)
- added a level skip cheat key (#1640)
- added a turbo cheat (#1639)
- added the ability to skip end credits with the action and escape keys (#1800)
- added the ability to skip FMVs with the action key (#1650)
- added the ability to hold forward/back to move through menus more quickly (#1644)
- added optional rendering of pickups in the UI as 3D meshes (#1633)
- added optional rendering of pickups on the ground as 3D meshes (#1634)
- added a special target, "pickup", to item-based console commands
- changed the inputs backend from DirectX to SDL (#1695)
    - improved controller support to match TR1X
    - changed the number of custom layouts to 3
    - changed default key bindings according to the following table:
        | Key                           | Old binding | New binding  | Reason
        | ----------------------------- | ----------- | ------------ | -----
        | Flare                         | Comma (,)   | Period (.)   | To maintain forward compatibility with TR3
        | Screenshot                    | S           | Print Screen | To maintain compatibility with TR1X
        | Toggle bilinear filter        | F8          | F3           | To maintain compatibility with TR1X
        | Toggle perspective filter     | Shift+F8    | F4           | To maintain compatibility with TR1X
        | Toggle z-buffer               | F7          | F7           | Likely to be permanently enabled in the future
        | Toggle triple buffering       | Shift+F7    | **Removed**  | Obscure setting, will be either removed or available via the ingame UI at some point
        | Toggle dither                 | F11         | **Removed**  | Obscure setting, will be either removed or available via the ingame UI at some point
        | Toggle fullscreen             | F12         | Alt-Enter    | To maintain compatibility with TR1X
        | Toggle rendering mode         | Shift+F12   | F12          | No more conflict to require Shift
        | Decrease resolution           | F1          | Shift+F1     | F3 and F4 are already taken
        | Increase resolution           | F2          | F1           | F3 and F4 are already taken
        | Decrease internal screen size | F3          | Shift+F2     | F3 and F4 are already taken
        | Increase internal screen size | F4          | F2           | F3 and F4 are already taken
    - removed "falling through" to the default layout, with the exception of keyboard arrows (matching TR1X behavior)
    - removed hardcoded Shift+F7 key binding for toggling triple buffering
    - removed hardcoded `0` key binding for flares
    - removed hardcoded cooldown of 15 frames for medipacks
- changed text backend to accept named sequences (eg. "\{arrow up}" and similar)
- changed inventory to pause the music rather than muting it (#1707)
- changed the `/pos` command to include the level number and title
- changed the `/tp` command to teleport to items in a round-robin fashion
  The first call will teleport Lara to the object that's the closest to her; repeated calls will cycle through all matching objects in the object placement order.
- improved FMV mode appearance - removed black scanlines (#1729)
- improved FMV mode behavior - stopped switching screen resolutions (#1729)
- improved screenshots: now saved in the screenshots/ directory with level titles and timestamps as JPG or PNG, similar to TR1X (#1773)
- improved switch object names
    - Switch Type 1 renamed to "Airlock Switch"
    - Switch Type 2 renamed to "Small Switch"
    - Switch Type 3 renamed to "Switch Button"
    - Switch Type 4 renamed to "Lever/Switch"
    - Switch Type 5 renamed to "Underwater Lever/Switch"
- fixed screenshots not working in windowed mode (#1766)
- fixed screenshots key not getting debounced (#1773)
- fixed `/give` not working with weapons (regression from 0.5)
- fixed the camera being cut off after using the gong hammer in Ice Palace (#1580)
- fixed the audio not being in sync when Lara strikes the gong in Ice Palace (#1725)
- fixed door cheat not working with drawbridges (#1748)
- fixed certain audio samples continuing to play after finishing the level (#1770, regression from 0.2)
- fixed Lara's underwater hue being retained when re-entering a boat (#1596)
- fixed Lara reloading the harpoon gun after every shot in NG+ (#1575)
- fixed the dragon reviving itself after Lara removes the dagger in rare circumstances (#1572)
- fixed grenades counting as double kills in the game statistics (#1560)
- fixed the ammo counter being hidden while a demo plays in NG+ (#1559)
- fixed the game crashing in large rooms with z-buffer disabled (#1761, regression from 0.2)
- fixed the game hanging if exited during the level stats, credits, or final stats (#1585)
- fixed the console not being drawn during credits (#1802)
- fixed grenades launched at too slow speeds (#1760, regression from 0.3)
- fixed the dragon counting as more than one kill if allowed to revive (#1771)
- fixed a crash when firing grenades at Xian guards in statue form (#1561)
- fixed harpoon bolts damaging inactive enemies (#1804)
- fixed enemies that are run over by the skidoo not being counted in the statistics (#1772)
- fixed sound settings resuming the music (#1707)
- fixed the inventory ring spinout animation sometimes running too fast (#1704, regression from 0.3)
- fixed new saves not displaying the save count in the passport (#1591)
- fixed certain erroneous `/play` invocations resulting in duplicated error messages

## [0.5](https://github.com/LostArtefacts/TRX/compare/afaf12a...tr2-0.5) - 2024-10-08
- added `/sfx` command
- added `/nextlevel` alias to `/endlevel` console command
- added `/quit` alias to `/exit` console command
- added the ability to cycle through console prompt history (#1571)
- changed `/set` console command to do fuzzy matching (LostArtefacts/libtrx#38)
- fixed crash in the `/set` console command (regression from 0.3)
- fixed using console in cutscenes immediately exiting the game (regression from 0.3)
- fixed Lara remaining tilted when teleporting off a vehicle while on a slope (LostArtefacts/TR2X#275, regression from 0.3)
- fixed `/endlevel` displaying a success message in the title screen
- fixed very loud music volume set by default (#1614)
- improved vertex movement when looking through water portals (#1493)
- improved console commands targeting creatures and pickups (#1667)

## [0.4]
Version 0.4 was skipped because of a major repository merge with TR1X into TRX.

## [0.3](https://github.com/LostArtefacts/TR2X/compare/0.2...0.3) - 2024-09-20
- added new console commands:
    - `/endlevel`
    - `/demo`
    - `/title`
    - `/play [level]`
    - `/load [slot]`
    - `/save [slot]`
    - `/exit`
    - `/fly`
    - `/give`
    - `/kill`
    - `/flip`
    - `/set`
- added an ability to remap the console key (LostArtefacts/TR2X#163)
- added `/tp` console command's ability to teleport to specific items
- added `/fly` console command's ability to open nearest doors
- added an option to fix M16 accuracy while running (LostArtefacts/TR2X#45)
- added a .NET-based configuration tool (LostArtefacts/TR2X#197)
- changed the default flare key from `/` to `.` to avoid conflicts with the console (LostArtefacts/TR2X#163)
- fixed numeric keys interfering with the demos (LostArtefacts/TR2X#172)
- fixed explosions sometimes being drawn too dark (LostArtefacts/TR2X#187)
- fixed killing the T-Rex with a grenade launcher crashing the game (LostArtefacts/TR2X#168)
- fixed secret rewards not displaying shotgun ammo (LostArtefacts/TR2X#159)
- fixed controls dialog remapping being too sensitive (LostArtefacts/TR2X#5)
- fixed `/tp` console command during special animations in HSH and Offshore Rig (LostArtefacts/TR2X#178, regression from 0.2)
- fixed `/hp` console command taking arbitrary integers
- fixed console commands being able to interfere with demos, cutscenes and the title screen (LostArtefacts/TR2X#182, #179, regression from 0.2)
- fixed console registering key inputs too eagerly (regression from 0.2)
- fixed console not being drawn in cutscenes (LostArtefacts/TR2X#180, regression from 0.2)
- fixed sounds not playing under certain circumstances (LostArtefacts/TR2X#113, regression from 0.2)
- fixed the excessive pitch and playback speed correction for music files with sampling rate other than 44100 Hz (LostArtefacts/TR1X#1417, regression from 0.2)
- fixed a crash potential with certain music files (regression from 0.2)
- fixed enemy movement patterns in demo 1 and demo 3 (LostArtefacts/TR2X#98, regression from 0.1)
- fixed underwater creatures dying (LostArtefacts/TR2X#98, regression from 0.1)
- fixed a crash when spawning enemy drops (LostArtefacts/TR2X#125, regression from 0.1)
- fixed how sprites are shaded (LostArtefacts/TR2X#134, regression from 0.1.1)
- fixed enemies unable to climb (LostArtefacts/TR2X#138, regression from 0.1)
- fixed items not being reset between level loads (LostArtefacts/TR2X#142, regression from 0.1)
- fixed pulling the dagger from the dragon not activating triggers (LostArtefacts/TR2X#148, regression from 0.1)
- fixed the music at the beginning of Offshore Rig not playing (LostArtefacts/TR2X#150, regression from 0.1)
- fixed wade animation when moving from deep to shallow water (LostArtefacts/TR2X#231, regression from 0.1)
- fixed the distorted skybox in room 5 of Barkhang Monastery (LostArtefacts/TR2X#196)
- improved initial level load time by lazy-loading audio samples (LostArtefacts/TR2X#114)
- improved crash debug information (LostArtefacts/TR2X#137)
- improved the console caret sprite (LostArtefacts/TR2X#91)

## [0.2](https://github.com/LostArtefacts/TR2X/compare/0.1.1...0.2) - 2024-05-07
- added dev console with the following commands:
    - `/pos`
    - `/tp [room_num]`
    - `/tp [x] [y] [z]`
    - `/hp`
    - `/hp [num]`
    - `/heal`
- changed the music backend from WinMM to libtrx (SDL + libav)
- changed the sound backend from DirectX to libtrx (SDL + libav)
- fixed seams around underwater portals (LostArtefacts/TR2X#76, regression from 0.1)
- fixed Lara's climb down camera angle (LostArtefacts/TR2X#78, regression from 0.1)
- fixed healthbar and airbar flashing the wrong way when at low values (LostArtefacts/TR2X#82, regression from 0.1)

## [0.1.1](https://github.com/LostArtefacts/TR2X/compare/0.1...0.1.1) - 2024-04-27
- fixed Lara's shadow with z-buffer option on (LostArtefacts/TR2X#64, regression from 0.1)
- fixed rare camera issues (LostArtefacts/TR2X#65, regression from 0.1)
- fixed flat rectangle colors (LostArtefacts/TR2X#70, regression from 0.1)
- fixed medpacks staying open after use in Lara's inventory (LostArtefacts/TR2X#69, regression from 0.1)
- fixed pickup sprites UI drawn forever in Lara's Home (LostArtefacts/TR2X#68, regression from 0.1)

## [0.1](https://github.com/rr-/TR2X/compare/...0.1) - 2024-04-26
- added version string to the inventory
- fixed CDAudio not playing on certain versions (uses PaulD patch)
- fixed TGA screenshots crashing the game
