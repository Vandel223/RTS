/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.6
        Device            :  PIC16F18875
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/
#include "stdio.h"
#include "stdbool.h"

#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"
#include "LCD/lcd.h"
#include "TC74/tc74.h"
#include "EEPROM/eeprom.h"

/* Default Parameters */
#define PMON_DEFAULT 5
#define TALA_DEFAULT 3
#define TINA_DEFAULT 10
#define ALAF_DEFAULT 1
#define ALAH_DEFAULT 12
#define ALAM_DEFAULT 0
#define ALAS_DEFAULT 0
#define ALAT_DEFAULT 20
#define ALAL_DEFAULT 2
#define CLKH_DEFAULT 0
#define CLKM_DEFAULT 0

/* Addresses of the different Parameters and Records saved in EEPROM*/
#define MAX_TEMP_ADDR   0xF000
#define MIN_TEMP_ADDR   0xF005
#define MAX_LUMIN_ADDR  0xF00A
#define MIN_LUMIN_ADDR  0xF00F
#define PARAM_ADDR      0xF014

/* Duty of each type of alarm */
#define CLK_ALARM_DUTY      99
#define TEMP_ALARM_DUTY     66
#define LUMIN_ALARM_DUTY    32
#define OFF_DUTY            0

/* Records */
#define TEMP_RECORD     1
#define LUMIN_RECORD    2

/* Possible states of the clock */
#define NORMAL_MODE 0
#define CONFIG_MODE 1
typedef uint8_t mode_t;

/*
                         Main application
 */

bool timer_1s_flag = false;
uint8_t timer_PMON_cnt = 0; // counter for PMON * 1s iterations. compared against config.pmon
uint8_t timer_TINA_cnt = 0;
uint8_t timer_TALA_cnt = 0;
bool timer_TALA_on = false;
bool timer_TINA_on = false;
bool timer3_100ms_flag = true;
bool timer5_100ms_flag = true;

void timer_1s(void) {
    timer_1s_flag = true;
    timer_PMON_cnt += 1;
    timer_TINA_cnt += 1;
    timer_TALA_cnt += 1;
}

void timer3_100ms(void) {
    timer3_100ms_flag = true;
    TMR3_StopTimer();
    TMR3_Reload();
}

void timer5_100ms(void) {
    timer5_100ms_flag = true;
    TMR5_StopTimer();
    TMR5_Reload();
}

void main(void)
{   
    /* CONFIG */
    EEPROM_config config;
    /* POTENTIOMETER & LUMINOSITY */
    adc_result_t potentiometer;
    uint8_t lumin;
    EEPROM_record max_lumin = {0, 0, 0, 0, 0};
    EEPROM_record min_lumin = {0, 3, 0, 0, 0};
    /* TEMPERATURE */
    uint8_t temp;
    EEPROM_record max_temp = {0, 0, 0, 0, 0};
    EEPROM_record min_temp = {255, 0, 0, 0, 0};
    /* CLOCK SECONDS */
    uint8_t sec = 0;
    /* LCD BUFFER */
    char buf[17];
    /* BUTTONS */
    uint8_t sw1 = HIGH;
    uint8_t sw2 = HIGH;
    /* MODE STATE */
    mode_t mode = NORMAL_MODE; // initially NORMAL mode
    /* ALARMS */
    bool temp_alarm = false;
    bool lumin_alarm = false;
    bool clock_alarm = false;
    /* CURSOR */
    uint8_t cursor;
    /* SHOW RECORDS */
    uint8_t curr_record = 0; // default, no record shown

    // initialize the device
    SYSTEM_Initialize();

    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    /* I2C INIT */
    OpenI2C();
    /* LCD INIT */
    LCDinit();
    /* TIMERS INIT */
    // TMR1
    TMR1_Initialize();
    TMR1_SetInterruptHandler( timer_1s );
    TMR1_StartTimer();
    // TMR3
    TMR3_Initialize();
    TMR3_SetInterruptHandler( timer3_100ms );
    // TMR5
    TMR5_Initialize();
    TMR5_SetInterruptHandler( timer5_100ms );
    /* ADC INIT */
    ADCC_Initialize();
    ADCC_DisableContinuousConversion();
    /* PWM INIT */
    PWM6_Initialize();
    
    /* INIT PARAMETERS */
    read_EEPROM_config(&config, PARAM_ADDR);
    if (config.pmon == 0) {
        config.pmon = PMON_DEFAULT;
        config.tala = TALA_DEFAULT;
        config.tina = TINA_DEFAULT;
        config.alaf = ALAF_DEFAULT;
        config.alah = ALAH_DEFAULT;
        config.alam = ALAM_DEFAULT;
        config.alas = ALAS_DEFAULT;
        config.alat = ALAT_DEFAULT;
        config.alal = ALAL_DEFAULT;
        config.clkh = CLKH_DEFAULT;
        config.clkm = CLKM_DEFAULT;
    }
    
    /* READ CURRENT RECORDS */
    read_EEPROM_record(&max_temp, MAX_TEMP_ADDR);
    read_EEPROM_record(&min_temp, MIN_TEMP_ADDR);
    read_EEPROM_record(&max_lumin, MAX_LUMIN_ADDR);
    read_EEPROM_record(&min_lumin, MIN_LUMIN_ADDR);
    

    while (1)
    {
        // Add your application code
        if (timer_TINA_cnt == config.tina) {
            curr_record = 0;
        }
        /* Debounce buttons */
        if (SW1_GetValue() == LOW && timer3_100ms_flag) {
            timer3_100ms_flag = false;
            TMR3_Reload();
            TMR3_StartTimer();
            
            sw1 = LOW;
        }
        if (SW2_GetValue() == LOW && timer5_100ms_flag) {
            timer5_100ms_flag = false;
            TMR5_Reload();
            TMR5_StartTimer();
            
            sw2 = LOW;
        }
        
        if (sw1 == LOW) {
            sw1 = HIGH;
            if (mode == NORMAL_MODE) {
                clock_alarm = false;
                temp_alarm = false;
                lumin_alarm = false;
                //mode = CONFIG_MODE;
            }
        }
        if (sw2 == LOW) {
            sw2 = HIGH;
            if (mode == NORMAL_MODE) {
                timer_TINA_cnt = 0;
                timer_TINA_on = true;
                curr_record = (curr_record + 1) % 3;
            }
        }

        uint8_t any_alarm = false;
        if (config.alaf) {
            if (lumin_alarm) {
                LED_L_SetHigh();
                if (!timer_TALA_on) {
                    PWM6_LoadDutyValue(LUMIN_ALARM_DUTY);
                    timer_TALA_cnt = 0;
                    timer_TALA_on = true;
                }
                any_alarm = true;
            }
            else
                LED_L_SetLow();
            
            if (temp_alarm) {
                LED_T_SetHigh();
                if (!timer_TALA_on) {
                    PWM6_LoadDutyValue(TEMP_ALARM_DUTY);
                    timer_TALA_cnt = 0;
                    timer_TALA_on = true;
                }
                any_alarm = true;
            }
            else
                LED_T_SetLow();
            
            if (clock_alarm) {
                if (!timer_TALA_on) {
                    PWM6_LoadDutyValue(CLK_ALARM_DUTY);
                    timer_TALA_cnt = 0;
                    timer_TALA_on = true;
                }
                any_alarm = true;
            }
            
            if (!any_alarm)
                PWM6_LoadDutyValue(OFF_DUTY);
            
            if (timer_TALA_cnt == config.tala) {
                PWM6_LoadDutyValue(OFF_DUTY);
                timer_TALA_on = false;
            }
        }
        
        /* Five seconds elapsed */
        if (timer_PMON_cnt == config.pmon) { // Compare to PMON
            timer_PMON_cnt = 0; // Reset counter
            
            temp = readTC74();
            potentiometer = ADCC_GetSingleConversion(adc_potent);
            lumin = (potentiometer >> 8); // Get only the 2 MSbits (10 bits - 8 bits = 2 bits)
            
            /* Check if max/min temperature/luminosity was reached */
            if (temp > max_temp.temp) {
                max_temp.temp = temp;
                max_temp.lumin = lumin;
                max_temp.hour = config.clkh;
                max_temp.min = config.clkm;
                max_temp.sec = sec;
                write_EEPROM_record(max_temp, MAX_TEMP_ADDR);
            }
            if (temp < min_temp.temp) {
                min_temp.temp = temp;
                min_temp.lumin = lumin;
                min_temp.hour = config.clkh;
                min_temp.min = config.clkm;
                min_temp.sec = sec;
                write_EEPROM_record(min_temp, MIN_TEMP_ADDR);
            }
            
            if (lumin > max_lumin.lumin) {
                max_lumin.temp = temp;
                max_lumin.lumin = lumin;
                max_lumin.hour = config.clkh;
                max_lumin.min = config.clkm;
                max_lumin.sec = sec;
                write_EEPROM_record(max_lumin, MAX_LUMIN_ADDR);
            }
            if (lumin < min_lumin.lumin) {
                min_lumin.temp = temp;
                min_lumin.lumin = lumin;
                min_lumin.hour = config.clkh;
                min_lumin.min = config.clkm;
                min_lumin.sec = sec;
                write_EEPROM_record(min_lumin, MIN_LUMIN_ADDR);
            }
            
            /* Check thresholds */
            if (temp >= config.alat)
                temp_alarm = true;
            if (lumin <= config.alal)
                lumin_alarm = true;
        }
        
        /* One second elapsed */
        if (timer_1s_flag) {
            timer_1s_flag = false;
            
            /* Update Time */
            sec += 1;
            if (sec == config.alas && config.clkm == config.alam && config.clkh == config.alah)
                clock_alarm = true;
            if (sec >= 60) {
                sec = 0;
                config.clkm += 1;
                if (config.clkm >= 60) {
                    config.clkm = 0;
                    config.clkh = (config.clkh + 1) % 24;
                    
                }
                /* Save config on EEPROM */
                write_EEPROM_config(config, PARAM_ADDR);
            }
            
            
            /* Toggle C LED*/
            LED_C_Toggle();
            
            /* Update display */
            if (mode == NORMAL_MODE) {
                /* First Line */
                while (LCDbusy());
                LCDpos(0, 0);
                switch (curr_record) {
                    case 0:
                        sprintf(buf, "%02d:%02d:%02d  %c%c%c %c ",
                            config.clkh, config.clkm, sec,
                            clock_alarm ? 'C' : ' ', temp_alarm ? 'T' : ' ', lumin_alarm ? 'L' : ' ',
                            config.alaf ? 'A' : 'a'
                        );
                        break;
                        
                    case TEMP_RECORD:
                        sprintf(buf, "%02d:%02d:%02d %02dC   ",
                            max_temp.hour, max_temp.min, max_temp.sec,
                                max_temp.temp
                        );
                        sprintf(buf + 14, "L%1d", max_temp.lumin);
                        break;
                                            
                    case LUMIN_RECORD:
                        sprintf(buf, "%02d:%02d:%02d %02dC   ",
                            max_lumin.hour, max_lumin.min, max_lumin.sec,
                                max_lumin.temp
                        );
                        sprintf(buf + 14, "L%1d", max_lumin.lumin);
                        break;
                        
                    default:
                        break;
                }
                while (LCDbusy());
                LCDstr(buf);
                /* Second Line */
                while (LCDbusy());
                LCDpos(1, 0);
                switch (curr_record) {
                    case 0:
                        sprintf(buf, "%02d oC           ", temp);
                        sprintf(buf + 13, "L %1d", lumin);
                        break;
                     
                    case TEMP_RECORD:
                        sprintf(buf, "%02d:%02d:%02d %02dC   ",
                            min_temp.hour, min_temp.min, min_temp.sec,
                                min_temp.temp
                        );
                        sprintf(buf + 14, "L%1d", min_temp.lumin);
                        break;
                        
                    case LUMIN_RECORD:
                        sprintf(buf, "%02d:%02d:%02d %02dC   ",
                            min_lumin.hour, min_lumin.min, min_lumin.sec,
                                min_lumin.temp
                        );
                        sprintf(buf + 14, "L%1d", min_lumin.lumin);
                        break;
                        
                    default:
                        break;
                        
                }
                while (LCDbusy());
                LCDstr(buf);
            }
            
            if (mode == CONFIG_MODE) {
                
            }
        }
        
    }
}
/**
 End of File
*/