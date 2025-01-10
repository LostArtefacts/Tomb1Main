#include "game/demo.h"

#include "game/input.h"
#include "global/vars.h"

static int32_t m_DemoIdx = 0;
static INPUT_STATE m_OldDemoInputDB = {};

static int32_t M_GetNextLevel(void);

static int32_t M_GetNextLevel(void)
{
    if (g_GameFlow.num_demos <= 0) {
        return -1;
    }
    const int32_t level_num = g_GF_ValidDemos[m_DemoIdx];
    m_DemoIdx++;
    m_DemoIdx %= g_GameFlow.num_demos;
    return level_num;
}

int32_t Demo_ChooseLevel(int32_t demo_num)
{
    if (demo_num < 0) {
        return M_GetNextLevel();
    } else if (g_GameFlow.num_demos <= 0) {
        return -1;
    } else if (demo_num < 0 || demo_num >= g_GameFlow.num_demos) {
        return -1;
    } else {
        return g_GF_ValidDemos[demo_num];
    }
}

bool Demo_GetInput(void)
{
    if (g_DemoCount == 0) {
        m_OldDemoInputDB = (INPUT_STATE) {};
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
