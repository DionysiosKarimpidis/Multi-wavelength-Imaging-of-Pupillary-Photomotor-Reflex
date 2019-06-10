#ifndef DEFINITIONS_H
#define DEFINITIONS_H

/* * * * * * * * * * * * * * *
 * H.264 Control Definitions *
 * * * * * * * * * * * * * * */

// Profile //
#define PROFILE 100

#define PROFILE_BASELINE 101
#define PROFILE_MAIN     102
#define PROFILE_EXTENDED 103
#define PROFILE_HIGH     104
#define PROFILE_HIGH10   105
#define PROFILE_HIGH422  106
#define PROFILE_HIGH444  107
#define PROFILE_CONTRAINED_BASELINE 108

// LEVEL //
#define LEVEL 120

#define LEVEL_1   121
#define LEVEL_1b  122
#define LEVEL_11  123
#define LEVEL_12  124
#define LEVEL_13  125
#define LEVEL_2   126
#define LEVEL_21  127
#define LEVEL_22  128
#define LEVEL_23  129
#define LEVEL_3   130
#define LEVEL_31  131
#define LEVEL_32  132
#define LEVEL_4   133
#define LEVEL_41  134
#define LEVEL_42  135
#define LEVEL_5   136
#define LEVEL_51  137

// BITRATE //
#define BITRATE 140

// NAL UNIT FORMAT //
#define NAL_FORMAT 160

#define NAL_STARTCODES 161
#define NAL_PERBUFFER 162
#define NAL_1BYTE 163
#define NAL_2BYTE 164
#define NAL_4BYTE 165
#define NAL_DUMMY 166

// INTRA MODE //
#define INTRA_MODE 180

#define INTRA_4X4 181
#define INTRA_8X8 182
#define INTRA_16X16 183
#define INTRA_DUMMY 184

// INTRA PERIOD //
#define INTRA_PERIOD 200

// RATE CONTROL //
#define RATE 220

#define RATE_DEFAULT 221
#define RATE_VARIABLE 222
#define RATE_CONSTANT 223
#define RATE_VARIABLE_SKIP_FRAMES 224
#define RATE_CONSTANT_SKIP_FRAMES 225

// ENCODE QP //
#define ENCODE_QP 240

// MIN - MAX QUANT //
#define MIN_QUANT 260
#define MAX_QUANT 280

#define PHOT_BOX_WIDTH 10
#define PHOT_BOX_HEIGH 10

typedef enum
{
    PHOTOMETRY
}processRoutines;

static const int EYE_WAIT_TIME = 2;
static const int EYE_STIMULATION_TIME = 10;
static const int EYE_RECOVERY_TIME = 2;
static const int EYE_THRES_MIN = 250;
static const int EYE_THRES_MAX = 255;

typedef enum{
    LEFT,
    RIGHT
}eyePos;

struct EyeParameters{
    int waitTime = EYE_WAIT_TIME;
    int stimulationTime = EYE_STIMULATION_TIME;
    int recoveryTime = EYE_RECOVERY_TIME;
    int minThresValue = EYE_THRES_MIN;
    int maxThresValue = EYE_THRES_MAX;
    int areaValue = 30;
    int ledValue = 0;
    int ledNum = 0;
    int eyeNum = LEFT;
};

#endif // DEFINITIONS_H
