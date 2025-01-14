#include "benchmark.h"
#include "game/anims.h"
#include "game/gamebuf.h"

static void M_ParseCommand(ANIM_COMMAND *command, const int16_t **data);

static void M_ParseCommand(ANIM_COMMAND *const command, const int16_t **data)
{
    const int16_t *data_ptr = *data;
    command->type = *data_ptr++;

    switch (command->type) {
    case AC_MOVE_ORIGIN: {
        XYZ_16 *const pos = GameBuf_Alloc(sizeof(XYZ_16), GBUF_ANIM_COMMANDS);
        pos->x = *data_ptr++;
        pos->y = *data_ptr++;
        pos->z = *data_ptr++;
        command->data = (void *)pos;
        break;
    }

    case AC_JUMP_VELOCITY: {
        ANIM_COMMAND_VELOCITY_DATA *const data = GameBuf_Alloc(
            sizeof(ANIM_COMMAND_VELOCITY_DATA), GBUF_ANIM_COMMANDS);
        data->fall_speed = *data_ptr++;
        data->speed = *data_ptr++;
        command->data = (void *)data;
        break;
    }

    case AC_SOUND_FX:
    case AC_EFFECT: {
        ANIM_COMMAND_EFFECT_DATA *const data =
            GameBuf_Alloc(sizeof(ANIM_COMMAND_EFFECT_DATA), GBUF_ANIM_COMMANDS);
        data->frame_num = *data_ptr++;
        const int16_t effect_data = *data_ptr++;
        data->effect_num = effect_data & 0x3FFF;
        data->environment = (effect_data & 0xC000) >> 14;
        command->data = (void *)data;
        break;
    }

    default:
        command->data = NULL;
        break;
    }

    *data = data_ptr;
}

void Anim_LoadCommands(const int16_t *data)
{
    BENCHMARK *const benchmark = Benchmark_Start();

    const int32_t anim_count = Anim_GetTotalCount();
    for (int32_t i = 0; i < anim_count; i++) {
        ANIM *const anim = Anim_GetAnim(i);
        if (anim->num_commands == 0) {
            continue;
        }

        anim->commands = GameBuf_Alloc(
            sizeof(ANIM_COMMAND) * anim->num_commands, GBUF_ANIM_COMMANDS);
        const int16_t *data_ptr = &data[anim->command_idx];
        for (int32_t j = 0; j < anim->num_commands; j++) {
            ANIM_COMMAND *const command = &anim->commands[j];
            M_ParseCommand(command, &data_ptr);
        }
    }

    Benchmark_End(benchmark, NULL);
}
