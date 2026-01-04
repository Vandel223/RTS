//#ifdef notdef

/***************************************************************************
| File: comando.c  -  Concretizacao de comandos (exemplo)
|
| Autor: Carlos Almeida (IST)
| Data:  Nov 2002
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "msg.h"

extern QueueHandle_t xQueueCMD;
extern QueueHandle_t xQueueRTC;
extern QueueHandle_t xQueueTemp;

/*------------------------------------------------------------------------+
| Function: cmd_sair - termina a aplicacao
+--------------------------------------------------------------------------*/ 
void cmd_sair (int argc, char **argv)
{
//  exit(0);
}

/*-------------------------------------------------------------------------+
| Function: cmd_test - apenas como exemplo
+--------------------------------------------------------------------------*/ 
void cmd_test (int argc, char** argv)
{
  int i;

  /* exemplo -- escreve argumentos */
  for (i=0; i<argc; i++)
    printf ("\nargv[%d] = %s", i, argv[i]);
}

/*-------------------------------------------------------------------------+
| Function: cmd_send - send message
+--------------------------------------------------------------------------*/ 
void cmd_send (int argc, char** argv)
{
int32_t lValueToSend;
BaseType_t xStatus;

    if (argc == 2) {
        printf ("msg: %s\n", argv[1]);
        lValueToSend = atoi(argv[1]);
        xStatus = xQueueSend(xQueueCMD, &lValueToSend, 0);
    }
    else {
        printf ("wrong number of arguments!\n");
    }
}



void cmd_rdt(int argc, char **argv)
{
    Msg request;
    request.kind = MSG_CMD;
    request.cmdID = 1;
    xQueueSend(xQueueRTC, &request, 0); 

}

void cmd_sd(int argc, char **argv)
{
    Msg msg;
    msg.kind = MSG_CMD;
    msg.cmdID = 2;
    if (argc == 3) {
        //fazer verificação dos inputs
        msg.v.date.d = atoi(argv[1]);
        msg.v.date.M = atoi(argv[2]);
        msg.v.date.Y = atoi(argv[3]);
        xQueueSend(xQueueRTC, &msg, 0); 
    }
    else {
        printf ("wrong number of arguments!\n");
    }
}

void cmd_rc (int argc, char **argv)
{
    Msg msg;
    msg.kind = MSG_CMD;
    xQueueSend(xQueueRTC, &msg, 0); 

}

/*por fazer*/
void cmd_sc (int argc, char **argv)
{
    Msg msg;
    msg.kind = MSG_CMD;
    msg.cmdID = 4;
    if (argc == 3) {
        //fazer verificação dos inputs
        msg.v.clock.h = atoi(argv[1]);
        msg.v.clock.m = atoi(argv[2]);
        msg.v.clock.s = atoi(argv[3]);
        xQueueSend(xQueueRTC, &msg, 0); 
    }
    else {
        printf ("wrong number of arguments!\n");
    }
}

void cmd_rt (int argc, char **argv)
{
    Msg msg;
    msg.kind = MSG_CMD;
    msg.v.can_i_send = true; //signal to the RTC task send the time to cmd mail box
    xQueueSend(xQueueTemp, &msg, 0); 

    msg.v.can_i_send = false; //i'm not sure if it's necessary 
}
void cmd_rmm (int argc, char **argv)
{
    Msg msg;
    msg.kind = MSG_CMD;
    msg.cmdID = 6;
    xQueueSend(xQueueTemp, &msg, 0); 

}
void cmd_cmm (int argc, char **argv)
{
    Msg msg;
    msg.kind = MSG_CMD;
    msg.cmdID = 7;
    xQueueSend(xQueueTemp, &msg, 0); 

}

void cmd_rp(int argc, char **argv)
{
    printf("\npmon: %d \ntala: %d", pmon, tala);
}

void cmd_mmp(int argc, char **argv)
{
    if(argc==1){
        pmon = atoi(argv[1]);
    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_mta(int argc, char **argv)
{
    if(argc==1){
        tala = atoi(argv[1]);
    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_rai(int argc, char **argv)
{
    if (alarmClockEnabled) {
        printf("\nThe alarm clock is enabled (%02d:%02d:%02d)",
            alarm_hour, alarm_minute, alarm_second);
    } else {
        printf("\nThe alarm clock is disabled");
    }    
    if (alarmTempEnabled) {
        printf("\nThe temperature alarm is enabled (tlow = %.3f, thigh = %.3f)",
            tlow, thigh);
    } else {
        printf("\nThe temperature alarm is disabled");
    }

}

void cmd_sac(int argc, char **argv)
{
    if(argc==3){
        alarm_hour = atoi(argv[1]);
        alarm_minute = atoi(argv[2]);
        alarm_second = atoi(argv[3]);
    }else{
        printf ("wrong number of arguments! Expecting 3 arguments.\n");
    }
}

void cmd_sat(int argc, char **argv)
{
    if(argc==2){
        tlow = atoi(argv[1]);
        thigh = atoi(argv[2]);
    }else{
        printf ("wrong number of arguments! Expecting 2 arguments.\n");
    }
}

void cmd_adac(int argc, char **argv)
{
    if(argc==1){
        if(atoi(argv[1])==1){
            alarmClockEnabled=true;
        }else if(atoi(argv[1])==0){
            alarmClockEnabled=false;
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }

    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}

void cmd_adat(int argc, char **argv)
{
    if(argc==1){
        if(atoi(argv[1])==1){
            alarmTempEnabled=true;
        }else if(atoi(argv[1])==0){
            alarmTempEnabled=false;
        }else{
            printf("wrong argument! Expecting 1(activate) or 0(deactivate)");
        }

    }else{
        printf ("wrong number of arguments! Expecting 1 argument.\n");
    }
}
//#endif //notdef
