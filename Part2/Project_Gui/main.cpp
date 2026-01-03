#include "FreeRTOSConfig.h"
#include "mbed.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LM75B.h"
#include "C12832.h"
#include "MMA7660.h"
#include "RTC.h"



typedef enum {
    MSG_TEMPERATURE     = 1,
    MSG_BUBBLE_POS      = 2,
    MSG_RTC             = 3,
    MSG_HIT_BIT_        = 4,
    MSG_ALARM_FLAGS     = 5
} MsgKind;

typedef struct {
    MsgKind         kind;
    union {
        float       f32;
        time_t      t32;
        struct {
            int16_t x;
            int16_t y;
        } coords;
        uint32_t    u32;
    } v;
} LcdMsg;


C12832 lcd(p5, p7, p6, p8, p11);

I2C i2c(p28, p27);
MMA7660 MMA(i2c);
LM75B tempSensor(i2c);

Serial pc(USBTX,USBRX);

QueueHandle_t xQueueLCD;
static SemaphoreHandle_t xMutexI2C;
static SemaphoreHandle_t xSemphrRTC;

//function to read the command line
extern void monitor(void);

/* GLOBAL ALARM SYSTEM VARIABLES */

//Alarm clock

static int alarm_hour   = 12;
static int alarm_minute = 0;
static int alarm_second = 0;
static bool alarmClockEnabled = false;
static SemaphoreHandle_t xSemphrAlarm;     // Alarm Task

//Temperature Alarm

static int pmon = 5;              // monitoring period
static float tlow = 10.0f;
static float thigh = 25.0f;
static bool alarmTempEnabled = false;

static float tmin = 1000.0f;
static float tmax = -1000.0f;
static time_t tmin_timestamp = 0;
static time_t tmax_timestamp = 0;


//configuration for sound status
static bool configSoundEnabled = false;

//rgb led
PwmOut led_r(p24);   // not sure about the led positions, so we should check and update.
PwmOut led_g(p23);
PwmOut led_b(p25);

//Buzzer queue
QueueHandle_t xQueueBuzzer; // buzzer events

//cmd queue
QueueHandle_t xQueueCMD; //monitor -> function

/*Temperature rgb led and alarm helpers */

void setTempColor(float temp) {
    if (temp <= tlow) {
        led_r = 1.0f; led_g = 1.0f; led_b = 0.0f;  // BLUE
        return;
    }
    if (temp >= thigh) {
        led_r = 0.0f; led_g = 1.0f; led_b = 1.0f;  // RED
        return;
    }

    float ratio = (temp - tlow) / (thigh - tlow);

    led_r = 1.0f - ratio; 
    led_b = ratio;
    led_g = 1.0f - fabsf(0.5f - ratio) * 2.0f;
}

void updateMinMax(float temp) {
    time_t now = time(NULL);
    if (temp > tmax) { tmax = temp; tmax_timestamp = now; }
    if (temp < tmin) { tmin = temp; tmin_timestamp = now; }
}

void checkTemperatureAlarm(float temp) {
    if (!alarmTempEnabled) return;
    if (temp < tlow || temp > thigh) {
        uint32_t event = 1;
        xQueueSend(xQueueBuzzer, &event, 0);
    }
}

void sendAlarmFlagsToLCD() {
    LcdMsg msg;
    msg.kind = MSG_ALARM_FLAGS;

    msg.v.u32 =
        (alarmClockEnabled   ? 0x01 : 0x00) |
        (alarmTempEnabled    ? 0x02 : 0x00) |
        (configSoundEnabled  ? 0x04 : 0x00);

    xQueueSend(xQueueLCD, &msg, 0);
}



/*---------------------------created by professor----------------------------------------------*/

char* my_fgets (char* ln, int sz, FILE* f)
{
//  fgets(line, MAX_LINE, stdin);
//  pc.gets(line, MAX_LINE);
  int i; char c;
  for(i=0; i<sz-1; i++) {
      c = pc.getc();
      ln[i] = c;
      if ((c == '\n') || (c == '\r')) break;
  }
  ln[i] = '\0';

  return ln;
}



/* ISR for the RTC */
void vISRRTC(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemphrRTC, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* RTC alarm clock interrupt handler*/

void vISRAlarmClock(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemphrAlarm, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


/* Alarm clock Task */

void vTaskAlarmClock(void *pvParameters) {
    while (1) {
        xSemaphoreTake(xSemphrAlarm, portMAX_DELAY);
        if (alarmClockEnabled) {
            uint32_t event = 1;
            xQueueSend(xQueueBuzzer, &event, 0);
        }
    }
}


/* RTC Task */
void vTaskRTC(void *pvParameters) {
    LcdMsg txMsg;
    txMsg.kind = MSG_RTC;

    while (1) {
        xSemaphoreTake(xSemphrRTC, portMAX_DELAY);
        txMsg.v.t32 = time(NULL);
        xQueueSend(xQueueLCD, &txMsg, 0);
        sendAlarmFlagsToLCD();
    }
}


/*Buzzer task (It will be used by the two alarms)*/

PwmOut buzzer(p21);   // adjust to correct pin
static int tala = 3;  // duration in seconds

void vTaskBuzzer(void *pvParameters) {
    uint32_t event;

    while (1) {
        if (xQueueReceive(xQueueBuzzer, &event, portMAX_DELAY) == pdPASS) {
            buzzer.period(0.001f);   // 1 kHz
            buzzer = 0.5f;

            vTaskDelay(pdMS_TO_TICKS(tala * 1000));

            buzzer = 0.0f;
            xQueueReset(xQueueBuzzer);
        }
    }
}



/* LM75B Temperature Sensor Task */
void vTaskTemperature(void *pvParameters) {
    TickType_t xLastTime;
    LcdMsg txMsg;
    txMsg.kind = MSG_TEMPERATURE;

    //Initialize xLastTime
    xLastTime = xTaskGetTickCount();
    //Try to open the LM75B
    if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
        bool ok = tempSensor.open();      // I2C transaction [web:100]
        xSemaphoreGive(xMutexI2C);

        if (!ok) error("[Temperature Task] LM75B Device not detected\n");
        else printf("[Temperature Task] LM75B Device detected!\n");
    }

    while (1) {

        if (pmon > 0) {

            if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
                txMsg.v.f32 = (float)tempSensor.temp();
                xSemaphoreGive(xMutexI2C);
            }

            float temp = txMsg.v.f32;

            setTempColor(temp);
            updateMinMax(temp);
            checkTemperatureAlarm(temp);

            xQueueSend(xQueueLCD, &txMsg, 0);
        }

        if (pmon == 0)
            vTaskDelay(pdMS_TO_TICKS(200));
        else
            vTaskDelayUntil(&xLastTime, pdMS_TO_TICKS(pmon * 1000));
    }
    
}

/* Bubble Leveler Task */
void vTaskBubble(void *pvParameters) {
    TickType_t xLastTime;
    LcdMsg txMsg;
    txMsg.kind = MSG_BUBBLE_POS;

    //Initialize xLastTime
    xLastTime = xTaskGetTickCount();

    if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
        bool ok = MMA.testConnection();      // I2C transaction [web:100]
        xSemaphoreGive(xMutexI2C);

        if (!ok) error("[Bubble Task] MMA7660 Device not detected\n");
        else printf("[Bubble Task] MMA7660 Device detected!\n");
    }

    int16_t x = 0, y = 0;
    while(1) {
        if (xSemaphoreTake(xMutexI2C, 50) == pdTRUE) {
            //read X,Y +/-Gs and scale for #display pixels
            x = (x + MMA.x() * 32.0)/2.0;
            y = (y -(MMA.y() * 32.0))/2.0;
            xSemaphoreGive(xMutexI2C);
        }

        // bound output to box
        if (x < -13) x = -13;
        else if (x > 13) x = 13;
        if (y < -13) y = -13;
        else if (y > 13) y = 13;

        txMsg.v.coords.x = x;
        txMsg.v.coords.y = y;
        xQueueSend(xQueueLCD, &txMsg, 0);
        vTaskDelayUntil(&xLastTime, pdMS_TO_TICKS(100));
    }
}


/* LCD Resource Manager */
void vTaskLCD(void *pvParameters) {
    LcdMsg rxMsg;

    while (1) {
        /* Block until there is something to update */
        if ((xQueueReceive(xQueueLCD, &rxMsg, portMAX_DELAY) == pdPASS)) {
            switch (rxMsg.kind) {
                case MSG_TEMPERATURE:
                    lcd.locate(2,20);
                    lcd.printf("T(C) = %.3f\n", rxMsg.v.f32);
                    break;

                case MSG_BUBBLE_POS:
                    lcd.fillrect(95, 0, 127, 31, 0); // clear
                    lcd.rect(95, 0, 127, 31, 1);
                    lcd.fillcircle(rxMsg.v.coords.x+111, rxMsg.v.coords.y+15, 3, 1); //draw bubble
                    lcd.circle(111, 15, 5, 1);
                    break;

                case MSG_HIT_BIT_:
                    break;

                case MSG_RTC:
                    time_t t = rxMsg.v.t32;
                    // conversion to secs, mins and hours
                    uint32_t s = (uint32_t)(t % 86400);
                    uint32_t hh = (s / 3600) % 24;
                    uint32_t mm = (s / 60) % 60;
                    uint32_t ss = s % 60;
                    lcd.locate(2, 2);
                    lcd.printf("%02lu:%02lu:%02lu", hh, mm, ss);
                    break;

                case MSG_ALARM_FLAGS: {
                    uint32_t f = rxMsg.v.u32;

                    bool C = f & 0x01;
                    bool T = f & 0x02;
                    bool O = f & 0x04;

                    lcd.locate(2, 12);
                    lcd.printf("A: %c %c %c", 
                        C ? 'C' : '-', 
                        T ? 'T' : '-', 
                        O ? 'O' : '-'
                    );
                    break;
                }    

                default:
                    break;

            }
        }
    }
}

/*CMD task sender*/
void vTaskCMDSend( void *pvParameters ) {
int32_t lValueToSend;
BaseType_t xStatus;
    for( ;; ) {
        lValueToSend = 201;
        xStatus = xQueueSend( xQueue, &lValueToSend, 0 );
        monitor(); //does not return
    }
}

/*CMD task receiver*/
void vTaskCMDReceive( void *pvParameters ) {
int32_t lReceivedValue;
BaseType_t xStatus;

    printf("Hello from mbed -- FreeRTOS / cmd\n");
    for( ;; ) {
//        vTaskDelay( 1000 );
        xStatus = xQueueReceive( xQueue, &lReceivedValue, 1000 );
        if( xStatus == pdPASS ) {
            printf( "Received = %d", lReceivedValue );
        }
    }
}

int main( void ) {
    /* Perform any hardware setup necessary. */
    BaseType_t xStatus;

    pc.baud(115200);
    set_time(1256729737); // Set time to Wed, 28 Oct 2009 11:35:37
    tm alarm_tm = RTC::getDefaultTM();
    alarm_tm.tm_hour = alarm_hour;
    alarm_tm.tm_min  = alarm_minute;
    alarm_tm.tm_sec  = alarm_second;
    RTC::alarm(&vISRAlarmClock, alarm_tm);

//    prvSetupHardware();

//    printf("Hello from mbed -- FreeRTOS / cmd\n");

    /* Queue/Semaphore init */
    printf("Initializing Queues...\n");
    xQueueLCD = xQueueCreate(10, sizeof(LcdMsg));
    configASSERT(xQueueLCD != NULL);
    xQueueCMD = xQueueCreate( 19, sizeof( int32_t ) ); //19 cmd commands
    configASSERT(xQueueCMD != NULL);
    printf("Initializing Semaphores...\n");
    xMutexI2C = xSemaphoreCreateMutex();
    configASSERT(xMutexI2C != NULL);
    xSemphrRTC = xSemaphoreCreateBinary();
    configASSERT(xSemphrRTC != NULL);
    xSemaphoreGive(xSemphrRTC); // prime the RTC sempahore to print the time on start
    xSemphrAlarm = xSemaphoreCreateBinary();
    configASSERT(xSemphrAlarm != NULL);
    xQueueBuzzer = xQueueCreate(2, sizeof(uint32_t));
    configASSERT(xQueueBuzzer != NULL);
    /* Task init */
    printf("Initializing Tasks...\n");
    xStatus = xTaskCreate(vTaskRTC, "Task RTC", 2*configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate( vTaskTemperature, "Task Temp", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate( vTaskBubble, "Task Bubb", 2*configMINIMAL_STACK_SIZE, NULL, 2, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate( vTaskLCD, "Task LCD", 2*configMINIMAL_STACK_SIZE, NULL, 5, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate(vTaskAlarmClock, "Task Alarm", 2*configMINIMAL_STACK_SIZE, NULL, 4, NULL);
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate(vTaskBuzzer,    "Task Buzzer", 2*configMINIMAL_STACK_SIZE, NULL, 4, NULL);  
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate(vTaskCMDSend, "Task command line write", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL ); //check priority 
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate(vTaskCMDReceive,"Task command line read", 2*configMINIMAL_STACK_SIZE, NULL, 2, NULL ); //check priority 
    configASSERT(xStatus == pdPASS);
    /* Attach interruption to RTC every second */
    NVIC_SetPriority(RTC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY); // make sure RTC_IRQn can be masked by FreeRTOS
    RTC::attach(&vISRRTC, RTC::Second);
    /* Start the created tasks running. */
    printf("Intializing Scheduler...\n");
    vTaskStartScheduler();
    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
    for ( ;; );
    return 0;
}
