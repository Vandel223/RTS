/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   eeprom.h
 * Author: Guilherme Dias
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __EEPROM__H
#define	__EEPROM__H

#include <xc.h> // include processor files - each processor file is guarded.  

// TODO Insert appropriate #include <>

// TODO Insert declarations
#define MAGIC_WORD 0xAB

typedef struct __EEPROM_record {
    uint8_t temp;
    uint8_t lumin;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} EEPROM_record;

typedef struct __EEPROM_config {
    uint8_t pmon;
    uint8_t tala;
    uint8_t tina;
    uint8_t alaf;
    uint8_t alah;
    uint8_t alam;
    uint8_t alas;
    uint8_t alat;
    uint8_t alal;
    uint8_t clkh;
    uint8_t clkm;
} EEPROM_config;

void write_EEPROM_record(EEPROM_record record, uint16_t addr);
EEPROM_record read_EEPROM_record(uint16_t addr);
void write_EEPROM_config(EEPROM_config config, uint16_t addr);
EEPROM_config read_EEPROM_config(uint16_t addr);

#endif	/* __EEPROM__H */