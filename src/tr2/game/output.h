#pragma once

#include "global/types.h"

#include <libtrx/game/output.h>

#include <stdint.h>

typedef struct {
    float x;
    float y;
    float z;
    float rhw;
    float u;
    float v;
    float g;
} VERTEX_INFO;

// TODO: eliminate
void Output_InsertPolygons(const int16_t *obj_ptr, int32_t clip);
void Output_InsertPolygons_I(const int16_t *ptr, int32_t clip);
void Output_DrawObjectMesh(const OBJECT_MESH *mesh, int32_t clip);
void Output_DrawObjectMesh_I(const OBJECT_MESH *mesh, int32_t clip);
void Output_InsertRoom(const ROOM_MESH *mesh, bool is_outside);
void Output_InsertSkybox(const int16_t *obj_ptr);
const int16_t *Output_CalcObjectVertices(const int16_t *obj_ptr);
const int16_t *Output_CalcSkyboxLight(const int16_t *obj_ptr);
const int16_t *Output_CalcVerticeLight(const int16_t *obj_ptr);
void Output_CalcRoomVertices(const ROOM_MESH *mesh, int32_t far_clip);

void Output_InsertRoomSprite(const ROOM_MESH *mesh);

void Output_InsertClippedPoly_Textured(
    int32_t vtx_count, float z, int16_t poly_type, int16_t tex_page);

void Output_InsertPoly_Gouraud(
    int32_t vtx_count, float z, int32_t red, int32_t green, int32_t blue,
    int16_t poly_type);

void Output_DrawClippedPoly_Textured(int32_t vtx_count);

void Output_DrawPoly_Gouraud(
    int32_t vtx_count, int32_t red, int32_t green, int32_t blue);

void Output_DrawSprite(
    uint32_t flags, int32_t x, int32_t y, int32_t z, int16_t sprite_idx,
    int16_t shade, int16_t scale);

void Output_DrawPickup(
    int32_t sx, int32_t sy, int32_t scale, int16_t sprite_idx, int16_t shade);

void Output_DrawScreenSprite2D(
    int32_t sx, int32_t sy, int32_t sz, int32_t scale_h, int32_t scale_v,
    int16_t sprite_idx, int16_t shade, uint16_t flags);

void Output_DrawScreenSprite(
    int32_t sx, int32_t sy, int32_t sz, int32_t scale_h, int32_t scale_v,
    int16_t sprite_idx, int16_t shade, uint16_t flags);

void Output_DrawScaledSpriteC(const int16_t *obj_ptr);

void Output_ClearDepthBuffer(void);

bool Output_MakeScreenshot(const char *path);

void Output_InsertBackPolygon(int32_t x0, int32_t y0, int32_t x1, int32_t y1);

void Output_DrawScreenLine(
    int32_t x, int32_t y, int32_t z, int32_t x_len, int32_t y_len,
    uint8_t color_idx, const void *gour, uint16_t flags);

void Output_DrawScreenBox(
    int32_t sx, int32_t sy, int32_t z, int32_t width, int32_t height,
    uint8_t color_idx, const void *gour, uint16_t flags);

void Output_DrawScreenFBox(
    int32_t sx, int32_t sy, int32_t z, int32_t width, int32_t height,
    uint8_t color_idx, const void *gour, uint16_t flags);

void Output_DrawHealthBar(int32_t percent);
void Output_DrawAirBar(int32_t percent);

void Output_LoadBackgroundFromObject(void);

int16_t Output_FindColor(int32_t red, int32_t green, int32_t blue);
void Output_DoAnimateTextures(int32_t ticks);
void Output_InsertShadow(
    int16_t radius, const BOUNDS_16 *bounds, const ITEM *item);

void Output_CalculateWibbleTable(void);
int32_t Output_GetObjectBounds(const BOUNDS_16 *bounds);
void Output_CalculateLight(int32_t x, int32_t y, int32_t z, int16_t room_num);
void Output_CalculateStaticLight(int16_t adder);
void Output_CalculateStaticMeshLight(
    int32_t x, int32_t y, int32_t z, int32_t shade_1, int32_t shade_2,
    const ROOM *room);
void Output_CalculateObjectLighting(const ITEM *item, const BOUNDS_16 *bounds);
void Output_LightRoom(ROOM *room);
void Output_SetupBelowWater(bool is_underwater);
void Output_SetupAboveWater(bool is_underwater);
void Output_AnimateTextures(int32_t ticks);

void Output_AddDynamicLight(
    int32_t x, int32_t y, int32_t z, int32_t intensity, int32_t falloff);
