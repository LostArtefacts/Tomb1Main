## Migration guide for level builders

## TR1X

### Version 4.7 to TR1X 4.8

1. **Rename basic keys**  
   - Replace `file` key with `path` for every level.
   - Replace `music` key with `music_track` for every level.

2. **Update level enumeration structure**:
   - The `"type": "title"` property is no longer supported. Instead, the title
     level needs to be placed in the top-level `"title"` key.
   - The `"type": "cutscene"` property is no longer supported. Instead, the
     cutscenes need to be placed in the top-level `"cutscenes"` array.
   - All FMVs need to be placed in its own top-level `"fmvs"` array.

3. **Update individual level sequences**  
   - `start_game` should be removed.
   - `exit_to_cine` should be removed.
   - `exit_to_level` should be replaced with `level_complete`. No parameter needed.
   - `display_picture` no longer takes a `picture_path` argument and instead just takes a `path`.
   - `loading_screen` no longer takes a `picture_path` argument and instead just takes a `path`.
   - `level_stats` no longer takes a `level_id` argument.
   - `total_stats` no longer takes a `picture_path` argument and instead takes a `background_path`.
   - `play_fmv` no longer takes a `fmv_path` argument and instead takes a `fmv_id`.

4. **Update strings**  
   The game strings are now placed in a separate file, `TR1X_strings.json5` in
   preparation to eventually support internationalization. Elements such as
   item titles or item names need to be configured entirely in the new file, so
   all `"strings"` keys can be safely removed from the game flow. Refer to
   [GAME_STRINGS.md](GAME_STRINGS.md) for more details.
