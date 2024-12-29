#include "game/overlay.h"

#include "config.h"
#include "decomp/decomp.h"
#include "game/clock.h"
#include "game/console/common.h"
#include "game/inventory.h"
#include "game/matrix.h"
#include "game/music.h"
#include "game/objects/common.h"
#include "game/output.h"
#include "game/viewport.h"
#include "global/vars.h"

#include <libtrx/utils.h>

#include <stdio.h>

#define MAX_PICKUP_COLUMNS 4
#define MAX_PICKUP_ROWS 3
#define MAX_PICKUPS (MAX_PICKUP_COLUMNS * MAX_PICKUP_ROWS)
#define MAX_PICKUP_DURATION_DISPLAY (LOGIC_FPS * 2)
#define MAX_PICKUP_DURATION_EASE_IN (LOGIC_FPS / 2)
#define MAX_PICKUP_DURATION_EASE_OUT LOGIC_FPS
#define PICKUPS_FOV 65

#define FLASH_FRAMES 5
#define AMMO_X (-10)
#define AMMO_Y 35
#define MODE_INFO_X (-16)
#define MODE_INFO_Y (-16)

typedef enum {
    DPP_EASE_IN,
    DPP_DISPLAY,
    DPP_EASE_OUT,
    DPP_DEAD,
} DISPLAY_PICKUP_PHASE;

typedef struct {
    OBJECT *object;
    OBJECT *inv_object;
    double duration;
    int32_t grid_x;
    int32_t grid_y;
    int32_t rot_y;
    DISPLAY_PICKUP_PHASE phase;
} DISPLAY_PICKUP;

static DISPLAY_PICKUP m_Pickups[MAX_PICKUPS] = { 0 };
static int32_t m_OldHitPoints = -1;
static bool m_FlashState = false;
static int32_t m_FlashCounter = 0;
static int32_t m_DisplayModeInfoTimer = 0;
static TEXTSTRING *m_DisplayModeTextInfo = NULL;
static TEXTSTRING *m_AmmoTextInfo = NULL;

static float M_Ease(int32_t cur_frame, int32_t max_frames);
static BOUNDS_16 M_GetBounds(const OBJECT *obj, const FRAME_INFO *frame);
static void M_DrawPickup3D(const DISPLAY_PICKUP *pickup);
static void M_DrawPickupSprite(const DISPLAY_PICKUP *pickup);

static float M_Ease(const int32_t cur_frame, const int32_t max_frames)
{
    const float ratio = cur_frame / (float)max_frames;
    float result;
    if (ratio < 0.5f) {
        result = 2.0f * ratio * ratio;
    } else {
        const float new_ratio = ratio - 1.0f;
        result = 1.0f - 2.0f * new_ratio * new_ratio;
    }
    return result;
}

bool Overlay_FlashCounter(const int32_t ticks)
{
    if (m_FlashCounter > 0) {
        m_FlashCounter -= ticks;
        return m_FlashState;
    } else {
        m_FlashCounter = FLASH_FRAMES;
        m_FlashState = !m_FlashState;
    }
    return m_FlashState;
}

void Overlay_DrawAssaultTimer(void)
{
    if (g_CurrentLevel != 0 || !g_IsAssaultTimerDisplay) {
        return;
    }

    static char buffer[8];
    const int32_t total_sec = g_SaveGame.statistics.timer / FRAMES_PER_SECOND;
    const int32_t frame = g_SaveGame.statistics.timer % FRAMES_PER_SECOND;
    sprintf(
        buffer, "%d:%02d.%d", total_sec / 60, total_sec % 60,
        frame * 10 / FRAMES_PER_SECOND);

    const int32_t scale_h = PHD_ONE;
    const int32_t scale_v = PHD_ONE;

    const int32_t dot_offset = -6;
    const int32_t dot_width = 14;
    const int32_t colon_offset = -6;
    const int32_t colon_width = 14;
    const int32_t digit_offset = 0;
    const int32_t digit_width = 20;

    const int32_t y = 36;
    int32_t x = (g_PhdWinMaxX / 2) - 50;

    for (char *c = buffer; *c != '\0'; c++) {
        if (*c == ':') {
            x += colon_offset;
            Output_DrawScreenSprite2D(
                x, y, 0, scale_h, scale_v,
                g_Objects[O_ASSAULT_DIGITS].mesh_idx + 10, 0x1000, 0);
            x += colon_width;
        } else if (*c == '.') {
            x += dot_offset;
            Output_DrawScreenSprite2D(
                x, y, 0, scale_h, scale_v,
                g_Objects[O_ASSAULT_DIGITS].mesh_idx + 11, 0x1000, 0);
            x += dot_width;
        } else {
            x += digit_offset;
            Output_DrawScreenSprite2D(
                x, y, 0, scale_h, scale_v,
                *c + g_Objects[O_ASSAULT_DIGITS].mesh_idx - '0', 0x1000, 0);
            x += digit_width;
        }
    }
}

void Overlay_DrawGameInfo(const bool pickup_state)
{
    Overlay_DrawAmmoInfo();
    Overlay_DrawModeInfo();
    if (g_OverlayStatus > 0) {
        Overlay_DrawHealthBar();
        Overlay_DrawAirBar();
        Overlay_DrawPickups(pickup_state);
        Overlay_DrawAssaultTimer();
    }
    Console_Draw();
    Text_Draw();
}

void Overlay_Animate(int32_t ticks)
{
    Overlay_FlashCounter(ticks);

    for (int i = 0; i < MAX_PICKUPS; i++) {
        DISPLAY_PICKUP *const pickup = &m_Pickups[i];

        if (g_Config.visuals.enable_3d_pickups) {
            pickup->rot_y += 4 * PHD_DEGREE * ticks;
        } else {
            // Stop existing animations
            switch (pickup->phase) {
            case DPP_EASE_IN:
                pickup->phase = DPP_DISPLAY;
                pickup->duration = 0;
                break;

            case DPP_EASE_OUT:
                pickup->phase = DPP_DEAD;
                break;

            default:
                break;
            }
        }

        switch (pickup->phase) {
        case DPP_DEAD:
            continue;

        case DPP_EASE_IN:
            pickup->duration += ticks;
            if (pickup->duration >= MAX_PICKUP_DURATION_EASE_IN) {
                pickup->phase = DPP_DISPLAY;
                pickup->duration = 0;
            }
            break;

        case DPP_DISPLAY:
            pickup->duration += ticks;
            if (pickup->duration >= MAX_PICKUP_DURATION_DISPLAY) {
                pickup->phase = g_Config.visuals.enable_3d_pickups
                    ? DPP_EASE_OUT
                    : DPP_DEAD;
                pickup->duration = 0;
            }
            break;

        case DPP_EASE_OUT:
            pickup->duration += ticks;
            if (pickup->duration >= MAX_PICKUP_DURATION_EASE_OUT) {
                pickup->phase = DPP_DEAD;
                pickup->duration = 0;
            }
            break;
        }
    }
}

void Overlay_DrawHealthBar(void)
{
    int32_t hit_points = g_LaraItem->hit_points;
    CLAMP(hit_points, 0, LARA_MAX_HITPOINTS);

    if (m_OldHitPoints != hit_points) {
        m_OldHitPoints = hit_points;
        g_HealthBarTimer = 40;
    }
    CLAMPL(g_HealthBarTimer, 0);

    const int32_t percent = hit_points * 100 / LARA_MAX_HITPOINTS;
    if (hit_points <= LARA_MAX_HITPOINTS / 4) {
        Output_DrawHealthBar(m_FlashState ? percent : 0);
    } else if (g_HealthBarTimer > 0 || g_Lara.gun_status == LGS_READY) {
        Output_DrawHealthBar(percent);
    }
}

void Overlay_DrawAirBar(void)
{
    if (g_Lara.water_status != LWS_UNDERWATER
        && g_Lara.water_status != LWS_SURFACE) {
        return;
    }

    int32_t air = g_Lara.air;
    CLAMP(air, 0, LARA_MAX_AIR);
    const int32_t percent = air * 100 / LARA_MAX_AIR;
    if (air <= 450) {
        Output_DrawAirBar(m_FlashState ? percent : 0);
    } else {
        Output_DrawAirBar(percent);
    }
}

void Overlay_HideGameInfo(void)
{
    Text_Remove(m_AmmoTextInfo);
    m_AmmoTextInfo = NULL;

    Text_Remove(m_DisplayModeTextInfo);
    m_DisplayModeTextInfo = NULL;
}

void Overlay_MakeAmmoString(char *const string)
{
    char result[128] = "";

    char *ptr = string;
    while (*ptr != '\0') {
        if (*ptr == ' ') {
            strcat(result, " ");
        } else if (*ptr == 'A') {
            strcat(result, "\\{ammo shotgun}");
        } else if (*ptr == 'B') {
            strcat(result, "\\{ammo magnums}");
        } else if (*ptr == 'C') {
            strcat(result, "\\{ammo uzis}");
        } else if (*ptr >= '0' && *ptr <= '9') {
            strcat(result, "\\{small digit ");
            char tmp[2] = { *ptr, '\0' };
            strcat(result, tmp);
            strcat(result, "}");
        }
        ptr++;
    }

    strcpy(string, result);
}

void Overlay_DrawAmmoInfo(void)
{
    if (g_Lara.gun_status != LGS_READY || g_OverlayStatus <= 0
        || g_SaveGame.bonus_flag) {
        if (m_AmmoTextInfo != NULL) {
            Text_Remove(m_AmmoTextInfo);
            m_AmmoTextInfo = NULL;
        }
        return;
    }

    char buffer[128] = "";
    switch (g_Lara.gun_type) {
    case LGT_MAGNUMS:
        sprintf(buffer, "%5d", g_Lara.magnum_ammo.ammo);
        break;

    case LGT_UZIS:
        sprintf(buffer, "%5d", g_Lara.uzi_ammo.ammo);
        break;

    case LGT_SHOTGUN:
        sprintf(buffer, "%5d", g_Lara.shotgun_ammo.ammo / 6);
        break;

    case LGT_M16:
        sprintf(buffer, "%5d", g_Lara.m16_ammo.ammo);
        break;

    case LGT_GRENADE:
        sprintf(buffer, "%5d", g_Lara.grenade_ammo.ammo);
        break;

    case LGT_HARPOON:
        sprintf(buffer, "%5d", g_Lara.harpoon_ammo.ammo);
        break;

    default:
        return;
    }

    Overlay_MakeAmmoString(buffer);
    if (m_AmmoTextInfo != NULL) {
        Text_ChangeText(m_AmmoTextInfo, buffer);
    } else {
        m_AmmoTextInfo = Text_Create(AMMO_X, AMMO_Y, buffer);
        Text_AlignRight(m_AmmoTextInfo, true);
    }
}

void Overlay_InitialisePickUpDisplay(void)
{
    for (int32_t i = 0; i < MAX_PICKUPS; i++) {
        m_Pickups[i].phase = DPP_DEAD;
    }
}

static void M_DrawPickup3D(const DISPLAY_PICKUP *const pickup)
{
    const OBJECT *const obj = pickup->inv_object;
    const FRAME_INFO *const frame =
        (FRAME_INFO *)g_Anims[obj->anim_idx].frame_ptr;

    float ease = 1.0f;
    switch (pickup->phase) {
    case DPP_EASE_IN:
        ease = M_Ease(pickup->duration, MAX_PICKUP_DURATION_EASE_IN);
        break;

    case DPP_EASE_OUT:
        ease = M_Ease(
            MAX_PICKUP_DURATION_EASE_OUT - pickup->duration,
            MAX_PICKUP_DURATION_EASE_OUT);
        break;

    case DPP_DISPLAY:
        ease = 1.0f;
        break;

    case DPP_DEAD:
        return;
    }

    const VIEWPORT old_vp = *Viewport_Get();

    BOUNDS_16 bounds = frame->bounds;
    if (frame->bounds.min_x == frame->bounds.max_x
        && frame->bounds.min_y == frame->bounds.max_y) {
        // fix broken collision box for the prayer wheel
        bounds = Object_GetBoundingBox(obj, frame, -1);
    }

    const int32_t scale = 1280;
    const int32_t padding_right = MIN(old_vp.width, old_vp.height) / 10;
    const int32_t padding_bottom = padding_right;

    // Try to fit in a quarter of the screen
    const int32_t available_width = old_vp.width * 0.4 - padding_right;
    const int32_t available_height = old_vp.height / 2 - padding_bottom;

    // maintain aspect ratio
    const int32_t cell_width = available_width / MAX_PICKUP_COLUMNS;
    const int32_t cell_height = available_height / MAX_PICKUP_ROWS;
    const int32_t offscreen_offset = cell_width;

    const int32_t vp_width = cell_width;
    const int32_t vp_height = cell_height;
    const int32_t vp_src_x = old_vp.width + offscreen_offset;
    const int32_t vp_dst_x = old_vp.width - (cell_width / 2 + padding_right)
        - pickup->grid_x * cell_width;
    const int32_t vp_src_y = old_vp.height - (cell_height / 2 + padding_bottom);
    const int32_t vp_dst_y = vp_src_y - pickup->grid_y * cell_height;
    const int32_t vp_x = vp_src_x + (vp_dst_x - vp_src_x) * ease;
    const int32_t vp_y = vp_src_y + (vp_dst_y - vp_src_y) * ease;

    VIEWPORT new_vp = old_vp;
    new_vp.game_vars.win_center_x = vp_x;
    new_vp.game_vars.win_center_y = vp_y;
    Viewport_Restore(&new_vp);

    Matrix_PushUnit();
    Matrix_TranslateRel(0, 0, scale);
    Matrix_RotYXZ(0, PHD_DEGREE * 15, 0);
    Matrix_RotYXZ(pickup->rot_y, 0, 0);

    g_LsDivider = 0x6000;
    g_LsAdder = LOW_LIGHT;
    Output_RotateLight(0, 0);
    Output_SetupAboveWater(false);

    Matrix_Push();
    Matrix_TranslateRel(frame->offset.x, frame->offset.y, frame->offset.z);
    Matrix_TranslateRel(
        -(bounds.min_x + bounds.max_x) / 2, -(bounds.min_y + bounds.max_y) / 2,
        -(bounds.min_z + bounds.max_z) / 2);

    int16_t **mesh_ptrs = &g_Meshes[obj->mesh_idx];
    int32_t *bone = &g_AnimBones[obj->bone_idx];
    const int16_t *mesh_rots = frame->mesh_rots;
    Matrix_RotYXZsuperpack(&mesh_rots, 0);

    Output_InsertPolygons(mesh_ptrs[0], 0);
    for (int32_t mesh_idx = 1; mesh_idx < obj->mesh_count; mesh_idx++) {
        int32_t bone_extra_flags = *bone;
        if (bone_extra_flags & BF_MATRIX_POP) {
            Matrix_Pop();
        }

        if (bone_extra_flags & BF_MATRIX_PUSH) {
            Matrix_Push();
        }

        Matrix_TranslateRel(bone[1], bone[2], bone[3]);
        Matrix_RotYXZsuperpack(&mesh_rots, 0);
        bone += 4;

        Output_InsertPolygons(mesh_ptrs[mesh_idx], 0);
    }
    Matrix_Pop();

    Viewport_Restore(&old_vp);
}

static void M_DrawPickupSprite(const DISPLAY_PICKUP *const pickup)
{
    const int32_t sprite_height =
        MIN(g_PhdWinWidth, g_PhdWinHeight * 640 / 480) / 10;
    const int32_t sprite_width = sprite_height * 4 / 3;

    const int32_t x =
        g_PhdWinWidth - sprite_height - sprite_width * pickup->grid_x;
    const int32_t y =
        g_PhdWinHeight - sprite_height - sprite_height * pickup->grid_y;

    // TODO: use proper scaling
    const int32_t scale = 12288 * g_PhdWinWidth / 640;
    const int16_t sprite_num = pickup->object->mesh_idx;
    Output_DrawPickup(x, y, scale, sprite_num, 4096);
}

void Overlay_DrawPickups(const bool timed)
{
    for (int i = 0; i < MAX_PICKUPS; i++) {
        const DISPLAY_PICKUP *const pickup = &m_Pickups[i];
        if (pickup->phase == DPP_DEAD) {
            continue;
        }

        if (g_Config.visuals.enable_3d_pickups && pickup->inv_object != NULL) {
            M_DrawPickup3D(pickup);
        } else {
            M_DrawPickupSprite(pickup);
        }
    }
}

void Overlay_AddDisplayPickup(const int16_t object_id)
{
    if (object_id == O_SECRET_1 || object_id == O_SECRET_2
        || object_id == O_SECRET_3) {
        Music_Play(g_GameFlow.secret_track, MPM_ALWAYS);
    }

    int32_t grid_x = -1;
    int32_t grid_y = -1;
    for (int i = 0; i < MAX_PICKUPS; i++) {
        int x = i % MAX_PICKUP_COLUMNS;
        int y = i / MAX_PICKUP_COLUMNS;
        bool is_occupied = false;
        for (int j = 0; j < MAX_PICKUPS; j++) {
            bool is_dead_or_dying = m_Pickups[j].phase == DPP_DEAD
                || m_Pickups[j].phase == DPP_EASE_OUT;
            if (m_Pickups[j].grid_x == x && m_Pickups[j].grid_y == y
                && !is_dead_or_dying) {
                is_occupied = true;
                break;
            }
        }

        if (!is_occupied) {
            grid_x = x;
            grid_y = y;
            break;
        }
    }

    const GAME_OBJECT_ID inv_object_id = Inv_GetItemOption(object_id);
    for (int32_t i = 0; i < MAX_PICKUPS; i++) {
        DISPLAY_PICKUP *const pickup = &m_Pickups[i];
        if (pickup->phase == DPP_DEAD) {
            pickup->object = Object_GetObject(object_id);
            pickup->inv_object = inv_object_id != NO_OBJECT
                ? Object_GetObject(inv_object_id)
                : NULL;
            pickup->duration = 0;
            pickup->grid_x = grid_x;
            pickup->grid_y = grid_y;
            pickup->rot_y = 0;
            pickup->phase =
                g_Config.visuals.enable_3d_pickups ? DPP_EASE_IN : DPP_DISPLAY;
            break;
        }
    }
}

void Overlay_DisplayModeInfo(const char *const string)
{
    if (string == NULL) {
        Text_Remove(m_DisplayModeTextInfo);
        m_DisplayModeTextInfo = NULL;
        return;
    }

    if (m_DisplayModeTextInfo != NULL) {
        Text_ChangeText(m_DisplayModeTextInfo, string);
    } else {
        m_DisplayModeTextInfo = Text_Create(MODE_INFO_X, MODE_INFO_Y, string);
        Text_AlignRight(m_DisplayModeTextInfo, 1);
        Text_AlignBottom(m_DisplayModeTextInfo, 1);
    }
    m_DisplayModeInfoTimer = 2.5 * FRAMES_PER_SECOND;
}

void Overlay_DrawModeInfo(void)
{
    if (m_DisplayModeTextInfo == NULL) {
        return;
    }

    m_DisplayModeInfoTimer--;
    if (m_DisplayModeInfoTimer <= 0) {
        Text_Remove(m_DisplayModeTextInfo);
        m_DisplayModeTextInfo = NULL;
    }
}
