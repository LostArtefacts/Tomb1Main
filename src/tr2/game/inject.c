#include "game/inject.h"

#include <libtrx/benchmark.h>
#include <libtrx/log.h>
#include <libtrx/memory.h>
#include <libtrx/utils.h>
#include <libtrx/virtual_file.h>

#define INJECTION_MAGIC MKTAG('T', '2', 'X', 'J')
#define INJECTION_CURRENT_VERSION 1
#define NULL_FD_INDEX ((uint16_t)(-1))

typedef enum {
    INJ_VERSION_1 = 1,
} INJECTION_VERSION;

typedef enum {
    INJ_GENERAL = 0,
} INJECTION_TYPE;

typedef struct {
    VFILE *fp;
    INJECTION_VERSION version;
    INJECTION_TYPE type;
    INJECTION_INFO *info;
    bool relevant;
} INJECTION;

static int32_t m_NumInjections = 0;
static INJECTION *m_Injections = NULL;

static void M_LoadFromFile(INJECTION *injection, const char *filename);

static void M_LoadFromFile(INJECTION *const injection, const char *filename)
{
    injection->relevant = false;
    injection->info = NULL;

    VFILE *const fp = VFile_CreateFromPath(filename);
    injection->fp = fp;
    if (fp == NULL) {
        LOG_WARNING("Could not open %s", filename);
        return;
    }

    const uint32_t magic = VFile_ReadU32(fp);
    if (magic != INJECTION_MAGIC) {
        LOG_WARNING("Invalid injection magic in %s", filename);
        return;
    }

    injection->version = VFile_ReadS32(fp);
    if (injection->version < INJ_VERSION_1
        || injection->version > INJECTION_CURRENT_VERSION) {
        LOG_WARNING(
            "%s uses unsupported version %d", filename, injection->version);
        return;
    }

    injection->type = VFile_ReadS32(fp);

    switch (injection->type) {
    case INJ_GENERAL:
        injection->relevant = true;
        break;
    default:
        LOG_WARNING("%s is of unknown type %d", filename, injection->type);
        break;
    }

    if (!injection->relevant) {
        return;
    }

    injection->info = Memory_Alloc(sizeof(INJECTION_INFO));
    INJECTION_INFO *const info = injection->info;

    // TODO: read header

    LOG_INFO("%s queued for injection", filename);
}

void Inject_Init(const int injection_count, char *filenames[])
{
    m_NumInjections = injection_count;
    if (m_NumInjections == 0) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();

    m_Injections = Memory_Alloc(sizeof(INJECTION) * m_NumInjections);
    for (int32_t i = 0; i < m_NumInjections; i++) {
        M_LoadFromFile(&m_Injections[i], filenames[i]);
    }

    Benchmark_End(benchmark, NULL);
}

void Inject_AllInjections(void)
{
    if (m_NumInjections == 0) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();

    for (int32_t i = 0; i < m_NumInjections; i++) {
        const INJECTION *const injection = &m_Injections[i];
        if (!injection->relevant) {
            continue;
        }

        // TODO: process edits
    }

    Benchmark_End(benchmark, NULL);
}

void Inject_Cleanup(void)
{
    if (m_NumInjections == 0) {
        return;
    }

    BENCHMARK *const benchmark = Benchmark_Start();

    for (int32_t i = 0; i < m_NumInjections; i++) {
        INJECTION *const injection = &m_Injections[i];
        VFile_Close(injection->fp);
        Memory_FreePointer(&injection->info);
    }

    Memory_FreePointer(&m_Injections);
    Benchmark_End(benchmark, NULL);
}
