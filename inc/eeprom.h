/*
    Eeprom Driver

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/

#ifndef __EEPROM_DRIVER_H
#define __EEPROM_DRIVER_H

#include "RTE_Components.h"
#include  CMSIS_device_header

const uint32_t DATA_EEPROM_START_ADDR = 0x08080000;
const uint32_t DATA_EEPROM_END_ADDR   = 0x080803FF;

class eeprom_driver
{
public:
    void read(uint32_t Addr, uint8_t *buff_data, uint8_t len)
    {
        uint32_t  Addr_End = Addr + len + DATA_EEPROM_START_ADDR;
        Addr += DATA_EEPROM_START_ADDR;
        while(Addr < Addr_End)
        {
            *buff_data = *(__IO uint8_t *)Addr;
            buff_data++;
            Addr = Addr + 1;
        }
    }
    void write(uint32_t Addr, uint8_t *buff_data, uint8_t len)
    {
        __IO FLASH_Status FLASHStatus = FLASH_COMPLETE;
        uint32_t  Addr_End = Addr + len + DATA_EEPROM_START_ADDR;
        Addr += DATA_EEPROM_START_ADDR;
        __set_PRIMASK(1);
        DATA_EEPROM_Unlock();
        while(Addr < Addr_End)
        {
            FLASHStatus = DATA_EEPROM_ProgramByte(Addr, *buff_data);
            if(FLASHStatus == FLASH_COMPLETE)
            {
                Addr = Addr + 1;
                buff_data++;
            }
            else
            {
                FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                                | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR);
            }
        }
        DATA_EEPROM_Lock();
        __set_PRIMASK(0);
    }
private:

};

#endif //__EEPROM_DRIVER_H