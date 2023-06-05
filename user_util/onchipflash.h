
/*
    8/2021
    Thu vien Quan ly onchip Flash
*/
#ifndef ONCHIPFLASH_H
#define ONCHIPFLASH_H

#include "user_util.h"


extern HAL_StatusTypeDef    FLASH_WaitForLastOperation(uint32_t Timeout);

#if defined (STM32L433xx) 
    extern void  FLASH_PageErase(uint32_t Page, uint32_t Banks);
#endif
    
#if defined (STM32L151xC) || defined (STM32L072xx) || defined (STM32L082xx) 
    extern void  FLASH_PageErase(uint32_t Page);
#endif

#define BYTE_TEMP_FIRST    0xAA 
    
/*------------------------------Function-----------------------------*/
HAL_StatusTypeDef	OnchipFlashWriteData (uint32_t Addr, uint8_t *dataAddr, uint32_t dataLen);

HAL_StatusTypeDef	OnchipFlashCopy (uint32_t sourceandress, uint32_t destinationandress, uint32_t data_length);
HAL_StatusTypeDef 	OnchipFlashPageErase (uint32_t pageaddress);
void                Erase_Firmware(uint32_t Add_Flash_update,uint32_t Total_page);
void                OnchipFlashReadData (uint32_t address,uint8_t *destination, uint16_t length_inByte);

/*=========================Function xu ly onchip flash DCU===========*/
uint8_t             Save_Array(uint32_t ADD, uint8_t* Buff, uint16_t length);
uint8_t             Save_Array_without_erase (uint32_t ADD, uint8_t* Buff, uint16_t length);
uint8_t             OnchipFlash_Write_Buff (uint32_t Addr, uint8_t *pData, uint16_t Length);


#endif /*  */