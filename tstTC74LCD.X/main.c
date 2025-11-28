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

#include "mcc_generated_files/mcc.h"
#include "I2C/i2c.h"
#include "LCD/lcd.h"
#include "stdio.h"
#include "TC74/tc74.h"
#include "EEPROM/eeprom.h"

#define PMON_DEFAULT 5
#define TALA_DEFAULT 3
#define TINA_DEFAULT 10
#define ALAF_DEFAULT 0
#define ALAH_DEFAULT 12
#define ALAM_DEFAULT 0
#define ALAS_DEFAULT 0
#define ALAT_DEFAULT 20
#define ALAL_DEFAULT 2
#define CLKH_DEFAULT 0
#define CLKM_DEFAULT 0

#define MAX_TEMP_ADDR   0x00
#define MIN_TEMP_ADDR   0x05
#define MAX_LUMIN_ADDR  0x0A
#define MIN_LUMIN_ADDR  0x0F
#define PARAM_ADDR      0x14

/*
                         Main application
 */

uint8_t timer_1s_flag = 0;
uint8_t timer_PMON_flag = 0;
uint8_t timer3_100ms_flag = 1;
uint8_t timer5_100ms_flag = 1;

void timer_1s(void) {
    timer_1s_flag = 1;
    timer_PMON_flag += 1;
}

void timer3_100ms(void) {
    timer3_100ms_flag = 1;
    TMR3_StopTimer();
    TMR3_Reload();
}

void timer5_100ms(void) {
    timer5_100ms_flag = 1;
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
    /* CLOCK */
    uint8_t sec = 0;
    uint8_t min;
    uint8_t hour;
    /* LCD BUFFER */
    char buf[17];
    /* BUTTONS */
    uint8_t sw1 = HIGH;
    uint8_t sw2 = HIGH;

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
    //PWM6_LoadDutyValue();
    
    /* INIT PARAMETERS */
    config = read_EEPROM_config(PARAM_ADDR);
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
    
    /* WRITE CURRENT RECORDS */
    write_EEPROM_record(max_temp, MAX_TEMP_ADDR);
    write_EEPROM_record(min_temp, MIN_TEMP_ADDR);
    write_EEPROM_record(max_lumin, MAX_LUMIN_ADDR);
    write_EEPROM_record(min_lumin, MIN_LUMIN_ADDR);
    

    while (1)
    {
        // Add your application code
        
        /* Debounce buttons */
        if (SW1_GetValue() == LOW && timer3_100ms_flag) {
            timer3_100ms_flag = 0;
            TMR3_StartTimer();
            
            sw1 = LOW;
        }
        if (SW2_GetValue() == LOW && timer5_100ms_flag) {
            timer5_100ms_flag = 0;
            TMR5_StartTimer();
            
            sw2 = LOW;
        }
        /* Buttons tasks */
        if (sw1 == LOW) {
            sw1 = HIGH;
            // ...
        }
        if (sw2 == LOW) {
            sw2 = HIGH;
            // ...
        }
        
        /* Five seconds elapsed */
        if (timer_PMON_flag == config.pmon) {
            timer_PMON_flag = 0;
            
            temp = readTC74();
            potentiometer = ADCC_GetSingleConversion(adc_potent);
            lumin = (potentiometer >> 8); // Get only the 2 MSbits (10 bits - 8 bits = 2 bits)
            
            if (temp > max_temp.temp) {
                max_temp.temp = temp;
                max_temp.lumin = lumin;
                max_temp.hour = hour;
                max_temp.min = min;
                max_temp.sec = sec;
            }
            if (temp < min_temp.temp) {
                min_temp.temp = temp;
                min_temp.lumin = lumin;
                min_temp.hour = hour;
                min_temp.min = min;
                min_temp.sec = sec;
            }
            
            if (lumin > max_lumin.lumin) {
                max_lumin.temp = temp;
                max_lumin.lumin = lumin;
                max_lumin.hour = hour;
                max_lumin.min = min;
                max_lumin.sec = sec;
            }
            if (lumin < min_lumin.lumin) {
                min_lumin.temp = temp;
                min_lumin.lumin = lumin;
                min_lumin.hour = hour;
                min_lumin.min = min;
                min_lumin.sec = sec;
            }
        }
        
        /* One second elapsed */
        if (timer_1s_flag == 1) {
            timer_1s_flag = 0;
            
            sec += 1;
            if (sec >= 60) {
                sec = 0;
                min += 1;
                if (min >= 60) {
                    min = 0;
                    hour = (hour + 1) % 24;
                }
            }
            
            LCDpos(0, 0);
            sprintf(buf, "%02d:%02d:%02d  CTL AR", hour, min, sec);
            while (LCDbusy());
            LCDstr(buf);

            LCDpos(1, 0);;
            sprintf(buf, "%02d oC       L %1d", temp, lumin);
            while (LCDbusy());
            LCDstr(buf);
        }
        
    }
}
/**
 End of File
*/