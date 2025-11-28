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

/*
                         Main application
 */

uint8_t timer_1s_flag = 0;
uint8_t timer_5s_flag = 0;

void timer_1s(void) {
    timer_1s_flag = 1;
    timer_5s_flag += 1;
}

void main(void)
{   
    /* POTENTIOMETER & LUMINOSITY */
    adc_result_t potentiometer;
    uint8_t lumin;
    /* TEMPERATURE MEASUREMENT */
    uint8_t temp;
    /* CLOCK */
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    /* LCD BUFFER */
    char buf[17];

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
    /* TIMER INIT */
    TMR1_Initialize();
    TMR1_SetInterruptHandler( timer_1s );
    TMR1_StartTimer();
    /* ADC INIT */
    ADCC_Initialize();
    ADCC_DisableContinuousConversion();

    while (1)
    {
        // Add your application code
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
        
        if (timer_5s_flag == 5) {
            timer_5s_flag = 0;
            
            temp = readTC74();
            potentiometer = ADCC_GetSingleConversion(adc_potent);
            lumin = (potentiometer >> 8); // Get only the 2 MSbits (10 bits - 8 bits = 2 bits)
        }
        
    }
}
/**
 End of File
*/