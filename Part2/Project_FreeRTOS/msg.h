#ifndef MSG_H
#define MSG_H

/* Standard includes */
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef enum {
    MSG_TEMPERATURE = 1,
    MSG_BUBBLE_POS,
    MSG_1000MS,
    MSG_ALARM_ON,
    MSG_ALARM_CLK,
    MSG_ALARM_TEMP,
    MSG_TEMP_MIN,
    MSG_TEMP_MAX,

    CMD_MMP = 200,
    CMD_RT,
    CMD_RMM,
    CMD_CMM,
    CMD_SAT,
    CMD_ADAT,
    CMD_MTA,
    CMD_ADCS,
    CMD_ADBL_0
} MsgKind;


/* MESSAGES
 * The following structure is a general message 
 * 
 * kind (MsgKind)   - enum with all possible message kinds
 * v (union)        - union with types of payloads
 *              f32 (float)
 *              u32 (uint32_t)
 *              i16x2 (x, y - 2 x int16_t)
 *              cmdID (enum) - enum with posible commands
 */
typedef struct {
    MsgKind         kind;
    union {
        float       f32;
        struct {
            int16_t x;
            int16_t y;
        } i16x2;
        uint32_t    u32;
        bool        b;
        struct {
            float   temp;
            time_t  tstamp;
        } temp_rep;
        struct {
            float tlow;
            float thigh;
        } temp_lim;
    } v;
} Msg;



#endif /* MSG_H */