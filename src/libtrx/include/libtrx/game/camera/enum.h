#pragma once

typedef enum {
    CAM_CHASE = 0,
    CAM_FIXED = 1,
    CAM_LOOK = 2,
    CAM_COMBAT = 3,
    CAM_CINEMATIC = 4,
    CAM_HEAVY = 5,
#if TR_VERSION == 1
    CAM_PHOTO_MODE = 6,
#endif
} CAMERA_TYPE;
