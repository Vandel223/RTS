#ifndef MSG_H
#define MSG_H

/* Standard includes */
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef enum {
    MSG_TEMPERATURE     = 1,
    MSG_BUBBLE_POS      = 2,
    MSG_RTC             = 3,
    MSG_HIT_BIT_        = 4,
    MSG_ALARM_FLAGS     = 5,
    MSG_CMD             = 6
} MsgKind;


typedef struct {
    MsgKind         kind;
    uint8_t         cmdID;
    union {
        float       f32;
        time_t      t32;
        struct {
            int16_t x;
            int16_t y;
        } i16x2;
        struct{
            uint8_t d;
            uint8_t M;
            uint16_t Y;
        }date;
        struct{
            uint8_t h;
            uint8_t m;
            uint8_t s;
        }clock;
        uint32_t    u32;
        bool        can_i_send;
    } v;
}Msg;

/* GLOBAL ALARM SYSTEM VARIABLES */

//Alarm clock

static int alarm_hour   = 12;
static int alarm_minute = 0;
static int alarm_second = 0;
static bool alarmClockEnabled = false;

//Temperature Alarm

static int pmon = 5;              // monitoring period
static float tlow = 10.0f;
static float thigh = 25.0f;
static bool alarmTempEnabled = false;

static float tmin = 1000.0f;
static float tmax = -1000.0f;
static time_t tmin_timestamp = 0;
static time_t tmax_timestamp = 0;


static int tala = 3;  // duration in seconds



#endif /* MSG_H */