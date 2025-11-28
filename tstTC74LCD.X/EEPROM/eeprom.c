/*
 * File:   eeprom.c
 * Author: dias
 *
 * Created on November 24, 2025, 4:27 PM
 * 
 * Main functions used to integrate the TC74 temperature sensor
 */

#include <xc.h>
#include "eeprom.h"
#include "../mcc_generated_files/memory.h"

void write_EEPROM_record(EEPROM_record record, uint16_t addr) {
    DATAEE_WriteByte(addr, record.hour);
    DATAEE_WriteByte(addr + 1, record.min);
    DATAEE_WriteByte(addr + 2, record.sec);
    DATAEE_WriteByte(addr + 3, record.temp);
    DATAEE_WriteByte(addr + 4, record.lumin);
}

EEPROM_record read_EEPROM_record(uint16_t addr) {
    EEPROM_record record;
    record.hour = DATAEE_ReadByte(addr);
    record.min = DATAEE_ReadByte(addr + 1);
    record.sec = DATAEE_ReadByte(addr + 2);
    record.temp = DATAEE_ReadByte(addr + 3);
    record.lumin = DATAEE_ReadByte(addr + 4);
    return record;
}

uint8_t EEPROM_checksum(EEPROM_config config)
{
    uint8_t sum = 0;
    sum +=  config.pmon +
            config.tala +
            config.tina +
            config.alaf +
            config.alah +
            config.alam +
            config.alas +
            config.alat +
            config.alal +
            config.clkh +
            config.clkm;
    return sum;
}

void write_EEPROM_config(EEPROM_config config, uint16_t addr) {
    DATAEE_WriteByte(addr, MAGIC_WORD);
    DATAEE_WriteByte(addr + 1, config.pmon);
    DATAEE_WriteByte(addr + 2, config.tala);
    DATAEE_WriteByte(addr + 3, config.tina);
    DATAEE_WriteByte(addr + 4, config.alaf);
    DATAEE_WriteByte(addr + 5, config.alah);
    DATAEE_WriteByte(addr + 6, config.alam);
    DATAEE_WriteByte(addr + 7, config.alas);
    DATAEE_WriteByte(addr + 8, config.alat);
    DATAEE_WriteByte(addr + 9, config.alal);
    DATAEE_WriteByte(addr + 10, config.clkh);
    DATAEE_WriteByte(addr + 11, config.clkm);
    DATAEE_WriteByte(addr + 12, EEPROM_checksum(config));
}

EEPROM_config read_EEPROM_config(uint16_t addr) {
    EEPROM_config config;
    uint8_t magic_word, checksum;
    
    magic_word = DATAEE_ReadByte(addr);
    config.pmon = DATAEE_ReadByte(addr + 1);
    config.tala = DATAEE_ReadByte(addr + 2);
    config.tina = DATAEE_ReadByte(addr + 3);
    config.alaf = DATAEE_ReadByte(addr + 4);
    config.alah = DATAEE_ReadByte(addr + 5);
    config.alam = DATAEE_ReadByte(addr + 6);
    config.alas = DATAEE_ReadByte(addr + 7);
    config.alat = DATAEE_ReadByte(addr + 8);
    config.alal = DATAEE_ReadByte(addr + 9);
    config.clkh = DATAEE_ReadByte(addr + 10);
    config.clkm = DATAEE_ReadByte(addr + 11);
    checksum = DATAEE_ReadByte(addr + 12);
    
    if (magic_word == MAGIC_WORD && checksum == EEPROM_checksum(config))
        return config;
    else {
        config.pmon = 0;
        return config;
    }
        
}