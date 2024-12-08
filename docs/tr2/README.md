<p align="center">
<img alt="TR2X logo" src="/data/tr2/logo-light-theme.png#gh-light-mode-only" width="400"/>
<img alt="TR2X logo" src="/data/tr2/logo-dark-theme.png#gh-dark-mode-only" width="400"/>
</p>

TR2X is currently in the early stages of development, focusing on the
decompilation process. We recognize that there is much work to be done.

## Windows

### Installing (manual)

1. Head over to GitHub releases: https://github.com/LostArtefacts/TRX/releases
2. Download the TR2X zip file.
3. Extract the TR2X zip file into a directory of your choice.  
   Make sure you choose to overwrite existing directories and files.
4. (First time installation) Put your original game files into the target directory.
5. To play the game, run `TR2X.exe`.

## Improvements over original game

#### Gameplay
- added an option to fix M16 accuracy while running
- added optional rendering of pickups in the UI as 3D meshes
- added optional automatic key/puzzle inventory item pre-selection
- changed inventory to pause the music rather than muting it
- fixed killing the T-Rex with a grenade launcher crashing the game
- fixed secret rewards not displaying shotgun ammo
- fixed numeric keys interfering with the demos
- fixed the ammo counter being hidden while a demo plays in NG+
- fixed explosions sometimes being drawn too dark
- fixed controls dialog remapping being too sensitive
- fixed Lara reloading the harpoon gun after every shot in NG+
- fixed the dragon reviving itself after Lara removes the dagger in rare circumstances
- fixed grenades counting as double kills in the game statistics
- fixed the game hanging if exited during the level stats, credits, or final stats
- fixed a crash when firing grenades at Xian guards in statue form
- fixed harpoon bolts damaging inactive enemies
- fixed the distorted skybox in room 5 of Barkhang Monastery
- fixed new saves not displaying the save count in the passport
- fixed Lara getting stuck in her hit animation if she is hit while mounting the boat or skidoo
- fixed the detonator key and gong hammer not activating their target items when manually selected from the inventory
- fixed a potential crash if Lara is on the skidoo in a room with many other adjoining rooms
- fixed a softlock in Home Sweet Home if the final cutscene is triggered while Lara is on water surface
- fixed the following floor data issues:
    - **Opera House**: fixed the trigger under item 203 to trigger it rather than item 204
    - **Wreck of the Maria Doria**: fixed room 98 not having water
    - **Tibetan Foothills**: added missing triggers for the drawbridge in room 96 (after the flipmap)
    - **Catacombs of the Talion**: changed some music triggers to pads near the first yeti, and added missing triggers and ladder in room 116 (after the flipmap)
    - **Ice Palace**: fixed door 143's position to resolve the invisible wall in front of it, and added an extra pickup trigger beside the Gong Hammer in room 29
    - **Temple of Xian**: fixed missing death tiles in room 91
    - **Floating Islands**: fixed door 72's position to resolve the invisible wall in front of it

#### Cheats
- added a fly cheat
- added a level skip cheat
- added a door open cheat (while in fly mode)
- added a cheat to increase the game speed
- added a cheat to explode Lara like in TR2 and TR3

#### Input
- added additional custom control schemes
- added customizable controller support
- fixed setting user keys being very difficult
- fixed skipping FMVs triggering inventory
- fixed skipping credits working too fast

#### Statistics
- fixed the dragon counting as more than one kill if allowed to revive
- fixed enemies that are run over by the skidoo not being counted in the statistics

#### Input
- added ability to hold forward/back to move through menus more quickly

#### Visuals
- added support for HD FMVs
- fixed TGA screenshots crashing the game
- fixed the camera being cut off after using the gong hammer in Ice Palace
- fixed Lara's underwater hue being retained when re-entering a boat
- improved FMV mode behavior - stopped switching screen resolutions
- improved vertex movement when looking through water portals

#### Audio
- fixed music not playing with certain game versions
- fixed the audio not being in sync when Lara strikes the gong in Ice Palace
- fixed sound settings resuming the music
- fixed wrong default music volume (being very loud on some setups)

#### Mods
- added developer console (accessible with `/`, see [COMMANDS.md](COMMANDS.md) for details)

#### Miscellaneous
- added .jpeg/.png screenshots
- added ability to skip FMVs with both the Action key
- added ability to skip end credits with the Action and Escape keys
- ported video decoding library to ffmpeg
- ported audio output library to SDL
- ported input method to SDL
- fixed screenshots not working in windowed mode
- fixed screenshots key not getting debounced
- changed screenshots to be put in the screenshots/ directory
- changed saves to be put in the saves/ directory
