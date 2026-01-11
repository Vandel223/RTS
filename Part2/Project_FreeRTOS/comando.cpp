//#ifdef notdef

/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Nov 2002
***************************************************************************/
#include <cstdint>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "queue.h"
#include "semphr.h"
#include "msg.h"
#include "mbed.h"
#include "RTC.h"

extern void vISRAlarmClock(void);

extern QueueHandle_t xQueueLCD;
extern QueueHandle_t xQueueCMD;
extern QueueHandle_t xQueueTemp;
extern QueueHandle_t xQueueBuzzer;

extern SemaphoreHandle_t xSmphrBubble;
extern SemaphoreHandle_t xSmphrHitBit;

//Alarm clock

uint8_t alarm_hour = 12;
uint8_t alarm_minute = 0;
uint8_t alarm_second = 0;
bool alarmClockEnabled = false;

//Temperature Alarm

uint32_t pmon = 5;              // monitoring period
float tlow = 10.0f;
float thigh = 25.0f;
bool alarmTempEnabled = false;

float tmin = 1000.0f;
float tmax = -1000.0f;

uint32_t tala = 3;  // duration in seconds
bool configSound = false;
bool bubbleEnable = true;
bool hitBitEnable = false;


void cmd_rdt(int argc, char **argv)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[32];

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", t);
    printf("%s\n", buffer);

}
void cmd_sd(int argc, char **argv)
{
    if (argc != 4) {
        printf("Wrong number of arguments. Usage: sd <day> <month> <year>\n");
        return;
    }

    time_t now = time(NULL);
    if (now <= 0) {
        printf("RTC not initialized\n");
        return;
    }

    struct tm *tmp = localtime(&now);
    if (!tmp) {
        printf("Time error\n");
        return;
    }

    struct tm t = *tmp;   // copy current date + clock

    /* Update ONLY the date */
    t.tm_mday = atoi(argv[1]);
    t.tm_mon  = atoi(argv[2]) - 1;
    t.tm_year = atoi(argv[3]) - 1900;

    time_t epoch = mktime(&t);
    if (epoch == (time_t)-1) {
        printf("Invalid date\n");
        return;
    }

    set_time(epoch);
    printf("Date updated\n");
}


void cmd_rc (int argc, char **argv)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[16];

    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);
    printf("%s\n", buffer);

}

void cmd_sc(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage: sc <hour> <minute> <second>\n");
        return;
    }

    time_t now = time(NULL);
    if (now <= 0) {
        printf("RTC not initialized\n");
        return;
    }

    struct tm *tmp = localtime(&now);
    if (!tmp) {
        printf("Time error\n");
        return;
    }

    struct tm t = *tmp;   // copy valid date + time

    t.tm_hour = atoi(argv[1]);
    t.tm_min  = atoi(argv[2]);
    t.tm_sec  = atoi(argv[3]);


    time_t epoch = mktime(&t);
    if (epoch == (time_t)-1) {
        printf("Invalid time\n");
        return;
    }

    set_time(epoch);
    printf("Time updated\n");
}
void cmd_rt (int argc, char **argv)
{
    Msg msg;
    msg.kind = CMD_RT;
    xQueueSend(xQueueTemp, &msg, 0); 

}
void cmd_rmm (int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = CMD_RMM;
    xQueueSend(xQueueTemp, &txMsg, 0);
}
void cmd_cmm (int argc, char **argv)
{
    Msg msg;
    msg.kind = CMD_CMM;
    tmin = 1000.0f;
    tmax = -1000.0f;
    xQueueSend(xQueueTemp, &msg, 0); 
}

void cmd_rp(int argc, char **argv)
{
    printf("\npmon: %d \ntala: %d", pmon, tala);
}

void cmd_mmp(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = CMD_MMP;
    if(argc==2){
        pmon = atoi(argv[1]);
        txMsg.v.u32 = pmon;
        xQueueSend(xQueueTemp, &txMsg, 0);
    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_mta(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = CMD_MTA;
    if(argc==2){
        tala = atoi(argv[1]);
        txMsg.v.u32 = tala;
        xQueueSend(xQueueBuzzer, &txMsg, 0);
    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_rai(int argc, char **argv)
{
    printf("\nAlarm Clock\n\t%02d:%02d:%02d",
            alarm_hour, alarm_minute, alarm_second);
    if (alarmClockEnabled) {
        printf(" (Enabled)");
    } else {
        printf(" (Disabled)");
    }    
    printf("\nAlarm Temperature\n\ttlow = %.3f, thigh = %.3f",
                                        tlow, thigh);
    if (alarmTempEnabled) {
        printf(" (Enabled)");
    } else {
        printf(" (Disabled)");
    }

}

void cmd_sac(int argc, char **argv)
{
    if(argc==4){
        alarm_hour = atoi(argv[1]);
        alarm_minute = atoi(argv[2]);
        alarm_second = atoi(argv[3]);

        if (alarmClockEnabled) {
            tm t = RTC::getDefaultTM();

            t.tm_hour = alarm_hour;
            t.tm_min = alarm_minute;
            t.tm_sec = alarm_second;

            RTC::alarmOff();
            RTC::alarm(&vISRAlarmClock, t);
        }
    }else{
        printf ("wrong number of arguments! Expecting 3 arguments.\n");
    }
}

void cmd_sat(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = CMD_SAT;
    if(argc==3){
        tlow = atof(argv[1]);
        thigh = atof(argv[2]);
        txMsg.v.temp_lim.tlow = tlow;
        txMsg.v.temp_lim.thigh = thigh;
        xQueueSend(xQueueTemp, &txMsg, 0);
    }else{
        printf ("wrong number of arguments! Expecting 2 arguments.\n");
    }
}

void cmd_adac(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = MSG_ALARM_CLK;
    if(argc==2){
        if(atoi(argv[1])==1){
            tm t = RTC::getDefaultTM();

            t.tm_hour = alarm_hour;
            t.tm_min = alarm_minute;
            t.tm_sec = alarm_second;

            alarmClockEnabled=true;

            RTC::alarm(&vISRAlarmClock, t);

            txMsg.v.b = true;
            xQueueSend(xQueueLCD, &txMsg, 0);
        }else if(atoi(argv[1])==0){
            alarmClockEnabled=false;
            RTC::alarmOff();

            txMsg.v.b = false;
            xQueueSend(xQueueLCD, &txMsg, 0);
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }
    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_adat(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = MSG_ALARM_TEMP;
    if(argc==2){
        if(atoi(argv[1])==1){
            alarmTempEnabled=true;

            txMsg.v.b = true;
            xQueueSend(xQueueLCD, &txMsg, 0);

            txMsg.kind = CMD_ADAT;
            xQueueSend(xQueueTemp, &txMsg, 0);
        }else if(atoi(argv[1])==0){
            alarmTempEnabled=false;

            txMsg.v.b = false;
            xQueueSend(xQueueLCD, &txMsg, 0);

            txMsg.kind = CMD_ADAT;
            xQueueSend(xQueueTemp, &txMsg, 0);
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }

    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_rts(int argc, char **argv)
{
    printf("\nBubble Level:");
    if (bubbleEnable) {
        printf("\n\tEnabled");
    } else {
        printf("\n\tDisabled");
    }    

    printf("\nHit Bit Game:");
    if (hitBitEnable) {
        printf("\n\tEnabled");
    } else {
        printf("\n\tDisabled");
    }

    printf("\nConfig Sound:");
    if (configSound) {
        printf("\n\tEnabled");
    } else {
        printf("\n\tDisabled");
    }

}

void cmd_adcs(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = CMD_ADCS;
    if(argc==2){
        if(atoi(argv[1])==1){
            configSound=true;

            txMsg.v.b = configSound;
            xQueueSend(xQueueBuzzer, &txMsg, 0);
        }else if(atoi(argv[1])==0){
            configSound=false;

            txMsg.v.b = configSound;
            xQueueSend(xQueueBuzzer, &txMsg, 0);
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }

    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_adbl(int argc, char **argv)
{
    Msg txMsg;
    txMsg.kind = CMD_ADBL_0;
    if(argc==2){
        if(atoi(argv[1])==1){
            bubbleEnable=true;
            xSemaphoreGive(xSmphrBubble);
        }else if(atoi(argv[1])==0){
            bubbleEnable=false;
            xSemaphoreTake(xSmphrBubble, pdMS_TO_TICKS(100));
            xQueueSend(xQueueLCD, &txMsg, 0);
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }

    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_adhb(int argc, char **argv)
{
    if(argc==2){
        if(atoi(argv[1])==1){
            hitBitEnable=true;
            xSemaphoreGive(xSmphrHitBit);
        }else if(atoi(argv[1])==0){
            hitBitEnable=false;
            xSemaphoreTake(xSmphrHitBit, pdMS_TO_TICKS(100));
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }

    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

//#endif //notdef