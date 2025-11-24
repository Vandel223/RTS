/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for .
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.6
        Device            :  PIC16F18875
        Driver Version    :  2.11
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.30 and above
        MPLAB 	          :  MPLAB X 5.40	
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

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

/**
  Section: Included Files
*/

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set LED_L aliases
#define LED_L_TRIS                 TRISAbits.TRISA4
#define LED_L_LAT                  LATAbits.LATA4
#define LED_L_PORT                 PORTAbits.RA4
#define LED_L_WPU                  WPUAbits.WPUA4
#define LED_L_OD                   ODCONAbits.ODCA4
#define LED_L_ANS                  ANSELAbits.ANSA4
#define LED_L_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define LED_L_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define LED_L_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define LED_L_GetValue()           PORTAbits.RA4
#define LED_L_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define LED_L_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define LED_L_SetPullup()          do { WPUAbits.WPUA4 = 1; } while(0)
#define LED_L_ResetPullup()        do { WPUAbits.WPUA4 = 0; } while(0)
#define LED_L_SetPushPull()        do { ODCONAbits.ODCA4 = 0; } while(0)
#define LED_L_SetOpenDrain()       do { ODCONAbits.ODCA4 = 1; } while(0)
#define LED_L_SetAnalogMode()      do { ANSELAbits.ANSA4 = 1; } while(0)
#define LED_L_SetDigitalMode()     do { ANSELAbits.ANSA4 = 0; } while(0)

// get/set LED_T aliases
#define LED_T_TRIS                 TRISAbits.TRISA5
#define LED_T_LAT                  LATAbits.LATA5
#define LED_T_PORT                 PORTAbits.RA5
#define LED_T_WPU                  WPUAbits.WPUA5
#define LED_T_OD                   ODCONAbits.ODCA5
#define LED_T_ANS                  ANSELAbits.ANSA5
#define LED_T_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define LED_T_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define LED_T_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define LED_T_GetValue()           PORTAbits.RA5
#define LED_T_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define LED_T_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define LED_T_SetPullup()          do { WPUAbits.WPUA5 = 1; } while(0)
#define LED_T_ResetPullup()        do { WPUAbits.WPUA5 = 0; } while(0)
#define LED_T_SetPushPull()        do { ODCONAbits.ODCA5 = 0; } while(0)
#define LED_T_SetOpenDrain()       do { ODCONAbits.ODCA5 = 1; } while(0)
#define LED_T_SetAnalogMode()      do { ANSELAbits.ANSA5 = 1; } while(0)
#define LED_T_SetDigitalMode()     do { ANSELAbits.ANSA5 = 0; } while(0)

// get/set RA6 procedures
#define RA6_SetHigh()            do { LATAbits.LATA6 = 1; } while(0)
#define RA6_SetLow()             do { LATAbits.LATA6 = 0; } while(0)
#define RA6_Toggle()             do { LATAbits.LATA6 = ~LATAbits.LATA6; } while(0)
#define RA6_GetValue()              PORTAbits.RA6
#define RA6_SetDigitalInput()    do { TRISAbits.TRISA6 = 1; } while(0)
#define RA6_SetDigitalOutput()   do { TRISAbits.TRISA6 = 0; } while(0)
#define RA6_SetPullup()             do { WPUAbits.WPUA6 = 1; } while(0)
#define RA6_ResetPullup()           do { WPUAbits.WPUA6 = 0; } while(0)
#define RA6_SetAnalogMode()         do { ANSELAbits.ANSA6 = 1; } while(0)
#define RA6_SetDigitalMode()        do { ANSELAbits.ANSA6 = 0; } while(0)

// get/set LED_C aliases
#define LED_C_TRIS                 TRISAbits.TRISA7
#define LED_C_LAT                  LATAbits.LATA7
#define LED_C_PORT                 PORTAbits.RA7
#define LED_C_WPU                  WPUAbits.WPUA7
#define LED_C_OD                   ODCONAbits.ODCA7
#define LED_C_ANS                  ANSELAbits.ANSA7
#define LED_C_SetHigh()            do { LATAbits.LATA7 = 1; } while(0)
#define LED_C_SetLow()             do { LATAbits.LATA7 = 0; } while(0)
#define LED_C_Toggle()             do { LATAbits.LATA7 = ~LATAbits.LATA7; } while(0)
#define LED_C_GetValue()           PORTAbits.RA7
#define LED_C_SetDigitalInput()    do { TRISAbits.TRISA7 = 1; } while(0)
#define LED_C_SetDigitalOutput()   do { TRISAbits.TRISA7 = 0; } while(0)
#define LED_C_SetPullup()          do { WPUAbits.WPUA7 = 1; } while(0)
#define LED_C_ResetPullup()        do { WPUAbits.WPUA7 = 0; } while(0)
#define LED_C_SetPushPull()        do { ODCONAbits.ODCA7 = 0; } while(0)
#define LED_C_SetOpenDrain()       do { ODCONAbits.ODCA7 = 1; } while(0)
#define LED_C_SetAnalogMode()      do { ANSELAbits.ANSA7 = 1; } while(0)
#define LED_C_SetDigitalMode()     do { ANSELAbits.ANSA7 = 0; } while(0)

// get/set RC3 procedures
#define RC3_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define RC3_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define RC3_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define RC3_GetValue()              PORTCbits.RC3
#define RC3_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define RC3_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define RC3_SetPullup()             do { WPUCbits.WPUC3 = 1; } while(0)
#define RC3_ResetPullup()           do { WPUCbits.WPUC3 = 0; } while(0)
#define RC3_SetAnalogMode()         do { ANSELCbits.ANSC3 = 1; } while(0)
#define RC3_SetDigitalMode()        do { ANSELCbits.ANSC3 = 0; } while(0)

// get/set RC4 procedures
#define RC4_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define RC4_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define RC4_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define RC4_GetValue()              PORTCbits.RC4
#define RC4_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define RC4_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define RC4_SetPullup()             do { WPUCbits.WPUC4 = 1; } while(0)
#define RC4_ResetPullup()           do { WPUCbits.WPUC4 = 0; } while(0)
#define RC4_SetAnalogMode()         do { ANSELCbits.ANSC4 = 1; } while(0)
#define RC4_SetDigitalMode()        do { ANSELCbits.ANSC4 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/