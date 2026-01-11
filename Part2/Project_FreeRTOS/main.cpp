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
#include "msg.h"


C12832 lcd(p5, p7, p6, p8, p11);

PwmOut buzzer(p26);   // adjust to correct pin

I2C i2c(p28, p27);
MMA7660 MMA(i2c);
LM75B tempSensor(i2c);

Serial pc(USBTX,USBRX);

QueueHandle_t xQueueLCD;
QueueHandle_t xQueueCMD;
QueueHandle_t xQueueBuzzer;
QueueHandle_t xQueueTemp;

static SemaphoreHandle_t xMutexI2C;
SemaphoreHandle_t        xSmphrBubble;
SemaphoreHandle_t        xSmphrHitBit;

//function to read the command line
extern void monitor(void);

//rgb led
PwmOut led_r(p23);
PwmOut led_g(p24);
PwmOut led_b(p25);

// Potentiometers
AnalogIn pot1(p19);
AnalogIn pot2(p20);

// Hit Bit
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
DigitalIn joystick(p14);

const char TitleMsg[] = "\n Application Control Monitor\n";

/*------------------------------------------------+
| Function: my_fgets - called from my_getline
+-------------------------------------------------*/
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
    Msg txMsg;
    txMsg.kind = MSG_1000MS;
    xQueueSendFromISR(xQueueLCD, &txMsg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* RTC alarm clock interrupt handler*/

void vISRAlarmClock(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    Msg txMsg;
    txMsg.kind = MSG_ALARM_ON;
    xQueueSendFromISR(xQueueBuzzer, &txMsg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*Buzzer task (It will be used by the two alarms)*/

void vTaskBuzzer(void *pvParameters) {
    Msg rxMsg;
    float buzzer_duty = 0.2f;
    float buzzer_period = 0.001f; // 1 kHz


    uint32_t tala = 3;
    bool configSound = false;

    while (1) {
        if (xQueueReceive(xQueueBuzzer, &rxMsg, portMAX_DELAY) == pdPASS) {
            switch (rxMsg.kind) {
                case CMD_MTA:
                    tala = rxMsg.v.u32;
                    break;

                case CMD_ADCS:
                    configSound = rxMsg.v.b;
                    break;

                case MSG_ALARM_ON:
                    if (configSound) {
                        buzzer_duty = pot1.read() * 0.5f;
                        buzzer_period = 1/( 4500.0f * pot2.read() + 500.0f);
                    }
                    else {
                        buzzer_duty = 0.2f;
                        buzzer_period = 0.001f; // 1 kHz
                    }

                    buzzer.period(buzzer_period);
                    buzzer = buzzer_duty;

                    vTaskDelay(pdMS_TO_TICKS(tala * 1000));

                    buzzer = 0.0f;
                    break;

                default:
                    break;
            }
        }
    }
}



/* LM75B Temperature Sensor Task */
void vTaskTemperature(void *pvParameters) {
    Msg txMsg;
    Msg rxMsg;
    txMsg.kind = MSG_TEMPERATURE;
    float temp;
    time_t tnow;

    // Temp system variables
    uint8_t pmon = 5;
    float tlow = 10.0f;
    float thigh = 25.0f;
    float tmin = 1000.0f;
    float tmax = -1000.0f;
    time_t tmin_timestamp = 0;
    time_t tmax_timestamp = 0;
    bool alarmTempEnabled = false;

    //Try to open the LM75B
    if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
        bool ok = tempSensor.open();      // I2C transaction [web:100]
        xSemaphoreGive(xMutexI2C);

        if (!ok) error("[Temperature Task] LM75B Device not detected\n");
        //else printf("[Temperature Task] LM75B Device detected!\n");
    }

    TickType_t nextSample = xTaskGetTickCount(); 

    while (1) {

        TickType_t waitTicks;
        if (pmon == 0) {
            waitTicks = portMAX_DELAY;   // wait only for commands [web:66]
        } else {
            TickType_t now = xTaskGetTickCount();
            waitTicks = (now < nextSample) ? (nextSample - now) : 0;
        }

        if (xQueueReceive(xQueueTemp, &rxMsg, waitTicks) == pdTRUE) {  // got command [web:46]
                switch(rxMsg.kind){
                    case CMD_ADAT:
                        alarmTempEnabled = rxMsg.v.b;
                        break;

                    case CMD_SAT:
                        thigh = rxMsg.v.temp_lim.thigh;
                        tlow = rxMsg.v.temp_lim.tlow;
                        break;

                    case CMD_RMM:
                        txMsg.kind = MSG_TEMP_MIN;
                        txMsg.v.temp_rep.temp = tmin;
                        txMsg.v.temp_rep.tstamp = tmin_timestamp;
                        xQueueSend(xQueueCMD, &txMsg, 0);

                        txMsg.kind = MSG_TEMP_MAX;
                        txMsg.v.temp_rep.temp = tmax;
                        txMsg.v.temp_rep.tstamp = tmax_timestamp;
                        xQueueSend(xQueueCMD, &txMsg, 0);

                        txMsg.kind = MSG_TEMPERATURE;
                        break;

                    case CMD_MMP:
                        pmon = rxMsg.v.u32;
                        break;

                    case CMD_RT:
                        // Timeout happened => only possible when pmon > 0 => do the periodic sample [web:46]
                        if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
                            temp = (float)tempSensor.temp();
                            xSemaphoreGive(xMutexI2C);
                        }
                        if (temp <= tlow) {
                            led_r = 1.0f; led_g = 1.0f; led_b = 0.0f;  // BLUE
                        }
                        else if (temp >= thigh) {
                            led_r = 0.0f; led_g = 1.0f; led_b = 1.0f;  // RED
                        }
                        else {
                            float ratio = (temp - tlow) / (thigh - tlow);

                            led_r = 1.0f - ratio; 
                            led_b = ratio;
                            led_g = 1.0f;
                        }

                        tnow = time(NULL);
                        if (temp > tmax) { tmax = temp; tmax_timestamp = tnow; }
                        if (temp < tmin) { tmin = temp; tmin_timestamp = tnow; }

                        if (alarmTempEnabled) {
                            txMsg.kind = MSG_ALARM_ON;
                            if (temp < tlow || temp > thigh) {
                                xQueueSend(xQueueBuzzer, &txMsg, 0);
                            }
                            txMsg.kind = MSG_TEMPERATURE;
                        }

                        txMsg.v.f32 = temp;
                        xQueueSend(xQueueCMD, &txMsg, 0); //send the temp
                        break;

                    case CMD_CMM:          
                        tmin = 1000.0f;
                        tmax = -1000.0f;
                        break;

                    default:
                        break;
                }
                
            continue; // then go back to waiting (remaining time is naturally preserved)
        }

        // Timeout happened => only possible when pmon > 0 => do the periodic sample [web:46]
        if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
            temp = (float)tempSensor.temp();
            xSemaphoreGive(xMutexI2C);
        }

        if (temp <= tlow) {
            led_r = 1.0f; led_g = 1.0f; led_b = 0.0f;  // BLUE
        }
        else if (temp >= thigh) {
            led_r = 0.0f; led_g = 1.0f; led_b = 1.0f;  // RED
        }
        else {
            float ratio = (temp - tlow) / (thigh - tlow);

            led_r = 1.0f - ratio; 
            led_b = ratio;
            led_g = 1.0f;
        }

        tnow = time(NULL);
        if (temp > tmax) { tmax = temp; tmax_timestamp = tnow; }
        if (temp < tmin) { tmin = temp; tmin_timestamp = tnow; }

        if (alarmTempEnabled) {
            txMsg.kind = MSG_ALARM_ON;
            if (temp < tlow || temp > thigh) {
                xQueueSend(xQueueBuzzer, &txMsg, 0);
            }
            txMsg.kind = MSG_TEMPERATURE;
        }

        txMsg.v.f32 = temp;
        xQueueSend(xQueueLCD, &txMsg, 0);
        nextSample += pdMS_TO_TICKS(pmon * 1000);
    }
    
}

/* Bubble Leveler Task */
void vTaskBubble(void *pvParameters) {
    TickType_t xLastTime;
    Msg txMsg;
    txMsg.kind = MSG_BUBBLE_POS;

    //Initialize xLastTime
    xLastTime = xTaskGetTickCount();

    if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
        bool ok = MMA.testConnection();      // I2C transaction [web:100]
        xSemaphoreGive(xMutexI2C);

        if (!ok) error("[Bubble Task] MMA7660 Device not detected\n");
        //else printf("[Bubble Task] MMA7660 Device detected!\n");
    }

    int16_t x = 0, y = 0;
    while(1) {
        if (xSemaphoreTake(xSmphrBubble, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(xMutexI2C, pdMS_TO_TICKS(50)) == pdTRUE) {
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

            txMsg.v.i16x2.x = x;
            txMsg.v.i16x2.y = y;
            xQueueSend(xQueueLCD, &txMsg, 0);

            xSemaphoreGive(xSmphrBubble);
            vTaskDelayUntil(&xLastTime, pdMS_TO_TICKS(100));
        }
    }
}

void vTaskHitBit(void *pvParameters) {
    uint8_t value = 0x12; // this is 0b1100
    TickType_t xLastTime;

    //Initialize xLastTime
    xLastTime = xTaskGetTickCount();

    while(1) {

        led1 = 0x00;
        led2 = 0x00;
        led3 = 0x00;
        led4 = 0x00;

        if (xSemaphoreTake(xSmphrHitBit, portMAX_DELAY) == pdTRUE) {
            value = value ^ joystick;
            if (value == 0) {                    
                led1 = 0x01;
                led2 = 0x01;
                led3 = 0x01;
                led4 = 0x01;

                vTaskDelay(pdMS_TO_TICKS(250));

                led1 = 0x00;
                led2 = 0x00;
                led3 = 0x00;
                led4 = 0x00;

                vTaskDelay(pdMS_TO_TICKS(250));

                led1 = 0x01;
                led2 = 0x01;
                led3 = 0x01;
                led4 = 0x01;

                vTaskDelay(pdMS_TO_TICKS(250));

                led1 = 0x00;
                led2 = 0x00;
                led3 = 0x00;
                led4 = 0x00;

                vTaskDelay(pdMS_TO_TICKS(250));

                led1 = 0x01;
                led2 = 0x01;
                led3 = 0x01;
                led4 = 0x01;

                vTaskDelay(pdMS_TO_TICKS(500));

                led1 = 0x00;
                led2 = 0x00;
                led3 = 0x00;
                led4 = 0x00;

                value = 0x12;
            }
            value = ((value & 0x01)<<3) | value >> 1;
            
            led1 = (value >> 3) & 0x01;
            led2 = (value >> 2) & 0x01;
            led3 = (value >> 1) & 0x01;
            led4 = (value) & 0x01;

            xSemaphoreGive(xSmphrHitBit);
            vTaskDelayUntil(&xLastTime, pdMS_TO_TICKS(250));
        }
    }
}

/* LCD Resource Manager */
void vTaskLCD(void *pvParameters) {
    Msg rxMsg;
    time_t t;
    struct tm *tmp;

    bool alarmClockEnabled = false;
    bool alarmTempEnabled = false;

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
                    lcd.fillcircle(rxMsg.v.i16x2.x+111, rxMsg.v.i16x2.y+15, 3, 1); //draw bubble
                    lcd.circle(111, 15, 5, 1);
                    break;

                case MSG_1000MS:
                    t = time(NULL);
                    tmp = localtime(&t);

                    if (tmp) {
                        lcd.locate(2, 2);
                        lcd.printf("%02d:%02d:%02d", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
                    }
                    break;

                case MSG_ALARM_CLK:
                    alarmClockEnabled = rxMsg.v.b;
                    lcd.locate(2, 11);
                    lcd.printf("A: %c %c", 
                        alarmClockEnabled ? 'C' : ' ',
                        alarmTempEnabled ? 'T': ' '
                    );
                    break;

                case MSG_ALARM_TEMP:
                    alarmTempEnabled = rxMsg.v.b;
                    lcd.locate(2, 11);
                    lcd.printf("A: %c %c", 
                        alarmClockEnabled ? 'C' : ' ',
                        alarmTempEnabled ? 'T': ' '
                    );
                    break;

                case CMD_ADBL_0:
                    lcd.fillrect(95, 0, 127, 31, 0); // clear
                    break;

                default:
                    break;

            }
        }
    }
}

/*CMD task sender*/
void vTaskCMD( void *pvParameters ) {
    Msg rxMsg;
    struct tm *t;
    char buffer[32];

    printf("%s Type sos for help\n", TitleMsg);
    for( ;; ) {
        printf("\nCmd> "); fflush(stdout);
        while (!pc.readable())
            vTaskDelay(pdMS_TO_TICKS(100));
        
        monitor(); //does not return

        while ((xQueueReceive(xQueueCMD, &rxMsg, pdMS_TO_TICKS(100)) == pdPASS)) {
            switch (rxMsg.kind) {
                case MSG_TEMPERATURE:
                    printf("T(C) = %.3f\n", rxMsg.v.f32);
                    break;

                case MSG_TEMP_MAX:
                    t = localtime(&rxMsg.v.temp_rep.tstamp);
                    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", t);
                    printf("Tmax(C) = %.3f @ %s\n", rxMsg.v.temp_rep.temp, buffer);
                    break;

                case MSG_TEMP_MIN:
                    t = localtime(&rxMsg.v.temp_rep.tstamp);
                    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", t);
                    printf("Tmin(C) = %.3f @ %s\n", rxMsg.v.temp_rep.temp, buffer);
                    break;

                default:
                    break;

            }
        }
    }
}

int main( void ) {
    /* Perform any hardware setup necessary. */
    BaseType_t xStatus;

    pc.baud(115200);

    /* Queue/Semaphore init */
    printf("Initializing Semaphores...\n");
    xMutexI2C = xSemaphoreCreateMutex();
    configASSERT(xMutexI2C != NULL);
    xSmphrHitBit = xSemaphoreCreateBinary();
    configASSERT(xSmphrHitBit != NULL);
    xSmphrBubble = xSemaphoreCreateBinary();
    configASSERT(xSmphrBubble != NULL);
    printf("Initializing Queues...\n");
    xQueueLCD = xQueueCreate(15, sizeof(Msg));
    configASSERT(xQueueLCD != NULL);
    xQueueTemp = xQueueCreate(5, sizeof(Msg));
    configASSERT(xQueueTemp != NULL);
    xQueueBuzzer = xQueueCreate(5, sizeof(Msg));
    configASSERT(xQueueBuzzer != NULL);
    xQueueCMD = xQueueCreate(10, sizeof(Msg));
    configASSERT(xQueueCMD != NULL);

    /* Task init */
    printf("Initializing Tasks...\n");
    xStatus = xTaskCreate( vTaskTemperature, "Task Temp", 4*configMINIMAL_STACK_SIZE, NULL, 3, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate( vTaskBubble, "Task Bubb", 2*configMINIMAL_STACK_SIZE, NULL, 3, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate( vTaskHitBit, "Task HitBit", 2*configMINIMAL_STACK_SIZE, NULL, 2, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate( vTaskLCD, "Task LCD", 2*configMINIMAL_STACK_SIZE, NULL, 5, NULL );
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate(vTaskBuzzer, "Task Buzzer", 2*configMINIMAL_STACK_SIZE, NULL, 4, NULL);  
    configASSERT(xStatus == pdPASS);
    xStatus = xTaskCreate(vTaskCMD, "Task CMD", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL ); //check priority 
    configASSERT(xStatus == pdPASS);

    // RTC
    set_time(1256729737); // Set time to Wed, 28 Oct 2009 11:35:37
    NVIC_SetPriority(RTC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY); // make sure RTC_IRQn can be masked by FreeRTOS
    RTC::attach(&vISRRTC, RTC::Second);

    // Prime the LCD
    Msg primeLCD = {MSG_1000MS, 0};
    xQueueSend(xQueueLCD, &primeLCD, 0); // to print the clock time
    primeLCD.kind = MSG_ALARM_CLK;
    xQueueSend(xQueueLCD, &primeLCD, 0); // to print the alarms

    // Prime the Bubble Semaphore
    xSemaphoreGive(xSmphrBubble);

    /* Start the created tasks running. */
    printf("Intializing Scheduler...\n");
    vTaskStartScheduler();
    /* Execution will only reach here if there was insufficient heap to
    start the scheduler. */
    for ( ;; );
    return 0;
}
