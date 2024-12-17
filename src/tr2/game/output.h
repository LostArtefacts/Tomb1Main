#pragma once

#include "global/types.h"

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

void __cdecl Output_InsertPolygons(const int16_t *obj_ptr, int32_t clip);
void __cdecl Output_InsertPolygons_I(const int16_t *ptr, int32_t clip);
void __cdecl Output_InsertRoom(const int16_t *obj_ptr, int32_t is_outside);
void __cdecl Output_InsertSkybox(const int16_t *obj_ptr);
const int16_t *__cdecl Output_CalcObjectVertices(const int16_t *obj_ptr);
const int16_t *__cdecl Output_CalcSkyboxLight(const int16_t *obj_ptr);
const int16_t *__cdecl Output_CalcVerticeLight(const int16_t *obj_ptr);
const int16_t *__cdecl Output_CalcRoomVertices(
    const int16_t *obj_ptr, int32_t far_clip);
void __cdecl Output_RotateLight(int16_t pitch, int16_t yaw);
void __cdecl Output_AlterFOV(int16_t fov);

const int16_t *__cdecl Output_InsertRoomSprite(
    const int16_t *obj_ptr, int32_t vtx_count);

void __cdecl Output_InsertClippedPoly_Textured(
    int32_t vtx_count, float z, int16_t poly_type, int16_t tex_page);

void __cdecl Output_InsertPoly_Gouraud(
    int32_t vtx_count, float z, int32_t red, int32_t green, int32_t blue,
    int16_t poly_type);

void __cdecl Output_DrawClippedPoly_Textured(int32_t vtx_count);

void __cdecl Output_DrawPoly_Gouraud(
    int32_t vtx_count, int32_t red, int32_t green, int32_t blue);

void __cdecl Output_DrawSprite(
    uint32_t flags, int32_t x, int32_t y, int32_t z, int16_t sprite_idx,
    int16_t shade, int16_t scale);

void __cdecl Output_DrawPickup(
    int32_t sx, int32_t sy, int32_t scale, int16_t sprite_idx, int16_t shade);

void __cdecl Output_DrawScreenSprite2D(
    int32_t sx, int32_t sy, int32_t sz, int32_t scale_h, int32_t scale_v,
    int16_t sprite_idx, int16_t shade, uint16_t flags);

void __cdecl Output_DrawScreenSprite(
    int32_t sx, int32_t sy, int32_t sz, int32_t scale_h, int32_t scale_v,
    int16_t sprite_idx, int16_t shade, uint16_t flags);

void __cdecl Output_DrawScaledSpriteC(const int16_t *obj_ptr);

void Output_ClearDepthBuffer(void);

bool __cdecl Output_MakeScreenshot(const char *path);

void Output_InsertBackPolygon(int32_t x0, int32_t y0, int32_t x1, int32_t y1);

void Output_DrawBlackRectangle(int32_t opacity);
void Output_DrawBackground(void);
void Output_DrawPolyList(void);

void Output_DrawScreenLine(
    int32_t x, int32_t y, int32_t z, int32_t x_len, int32_t y_len,
    uint8_t color_idx, const void *gour, uint16_t flags);

void Output_DrawScreenBox(
    int32_t sx, int32_t sy, int32_t z, int32_t width, int32_t height,
    uint8_t color_idx, const void *gour, uint16_t flags);

void Output_DrawScreenFBox(
    int32_t sx, int32_t sy, int32_t z, int32_t width, int32_t height,
    uint8_t color_idx, const void *gour, uint16_t flags);

void __cdecl Output_DrawHealthBar(int32_t percent);
void __cdecl Output_DrawAirBar(int32_t percent);

void Output_LoadBackgroundFromFile(const char *file_name);
void Output_LoadBackgroundFromObject(void);
void Output_UnloadBackground(void);

void Output_BeginScene(void);
int32_t Output_EndScene(void);

int16_t Output_FindColor(int32_t red, int32_t green, int32_t blue);
void __cdecl Output_AnimateTextures(int32_t ticks);
void __cdecl Output_InsertShadow(
    int16_t radius, const BOUNDS_16 *bounds, const ITEM *item);

void __cdecl Output_CalculateWibbleTable(void);
int32_t __cdecl Output_GetObjectBounds(const BOUNDS_16 *bounds);
void __cdecl Output_CalculateLight(
    int32_t x, int32_t y, int32_t z, int16_t room_num);
void __cdecl Output_CalculateStaticLight(int16_t adder);
void __cdecl Output_CalculateStaticMeshLight(
    int32_t x, int32_t y, int32_t z, int32_t shade_1, int32_t shade_2,
    const ROOM *room);
void __cdecl Output_LightRoom(ROOM *room);
