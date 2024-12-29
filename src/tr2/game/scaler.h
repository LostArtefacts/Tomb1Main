#pragma once

#include <stdint.h>

typedef enum {
    SCALER_TARGET_BAR,
    SCALER_TARGET_TEXT,
} SCALER_TARGET;

int32_t Scaler_Calc(int32_t unit, SCALER_TARGET target);
int32_t Scaler_CalcInverse(int32_t unit, SCALER_TARGET target);
