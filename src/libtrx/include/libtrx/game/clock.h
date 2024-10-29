#pragma once

#include <stddef.h>

size_t Clock_GetDateTime(char *buffer, size_t size);
extern double Clock_GetHighPrecisionCounter(void);
