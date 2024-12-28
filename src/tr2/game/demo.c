#include "game/demo.h"

#include "game/input.h"
#include "global/vars.h"

#include <libtrx/log.h>

static int32_t m_DemoLevel = 0;
static INPUT_STATE m_OldDemoInputDB = { 0 };

static int32_t M_GetNextLevel(void);

static int32_t M_GetNextLevel(void)
{
    if (g_GameFlow.num_demos <= 0) {
        return -1;
    }
    if (m_DemoLevel >= g_GameFlow.num_demos) {
        m_DemoLevel = 0;
    }
    const int32_t level_num = g_GF_ValidDemos[m_DemoLevel];
    m_DemoLevel++;
    return level_num;
}

int32_t Demo_ChooseLevel(int32_t level_num)
{
    if (level_num < 0) {
        level_num = M_GetNextLevel();
    }
    return level_num;
}

bool Demo_GetInput(void)
{
    if (g_DemoCount == 0) {
        m_OldDemoInputDB = (INPUT_STATE) { 0 };
    }
    if (g_DemoCount >= MAX_DEMO_SIZE) {
        return false;
    }

    union {
        uint32_t any;
        struct {
            // clang-format off
            uint32_t forward:      1;
            uint32_t back:         1;
            uint32_t left:         1;
            uint32_t right:        1;
            uint32_t jump:         1;
            uint32_t draw:         1;
            uint32_t action:       1;
            uint32_t slow:         1;
            uint32_t option:       1;
            uint32_t look:         1;
            uint32_t step_left:    1;
            uint32_t step_right:   1;
            uint32_t roll:         1;
            uint32_t _pad:         6;
            uint32_t use_flare:    1;
            uint32_t menu_confirm: 1;
            uint32_t menu_back:    1;
            uint32_t save:         1;
            uint32_t load:         1;
            // clang-format on
        };
    } demo_input = { .any = g_DemoPtr[g_DemoCount] };

    if ((int32_t)demo_input.any == -1) {
        return false;
    }

    g_Input = (INPUT_STATE) {
        0,
        // clang-format off
        .forward      = demo_input.forward,
        .back         = demo_input.back,
        .left         = demo_input.left,
        .right        = demo_input.right,
        .jump         = demo_input.jump,
        .draw         = demo_input.draw,
        .action       = demo_input.action,
        .slow         = demo_input.slow,
        .option       = demo_input.option,
        .look         = demo_input.look,
        .step_left    = demo_input.step_left,
        .step_right   = demo_input.step_right,
        .roll         = demo_input.roll,
        .use_flare        = demo_input.use_flare,
        .menu_confirm = demo_input.menu_confirm,
        .menu_back    = demo_input.menu_back,
        .save         = demo_input.save,
        .load         = demo_input.load,
        // clang-format on
    };

    g_InputDB.any = g_Input.any & ~m_OldDemoInputDB.any;
    m_OldDemoInputDB = g_Input;
    g_DemoCount++;
    return true;
}
