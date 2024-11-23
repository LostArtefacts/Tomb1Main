#pragma once

#include <stdint.h>

typedef struct {
    int16_t room_index;
    int16_t num_vertices;
    int16_t num_quads;
    int16_t num_triangles;
    int16_t num_sprites;
} INJECTION_MESH_META;
