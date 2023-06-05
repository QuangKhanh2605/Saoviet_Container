/*
 * S25FL.h
 *
 *  Created on: Dec 15, 2021
 *      Author: lenovo
 */

#ifndef FLASH_EXT_S25FL_H_
#define FLASH_EXT_S25FL_H_

#include <stdint.h>
#include "spi.h"

#define SKIP				0x00

#define LOW    	 			0x00  /* Chip Select line low */
#define HIGH    			0x01  /* Chip Select line high */

#define S25FL_READ          0x03
#define S25FL_WRITE         0x02
#define S25FL_READ_STATUS   0x05
#define S25FL_WREN          0x06
#define S25FL_SECTOR_ERASE  0x20

#define S25FL_BLOCK_ERASE   0xD8
#define S25FL_CHIP_ERASE    0xC7
#define S25FL_JEDEC         0x9F  // 0x90
#define S25FL_WRITE_DISABLE 0x04

#define S25FL_VOLATILE_WREN 0x50
#define S25FL_RSTEN         0x66
#define S25FL_RST           0x99

#define WIP_Flag   		    0x03 // 0x01 vs 0x02 /* Write In Progress (WIP) flag */
#define WEL_Flag   		    0x02 /* Chua co check Flag WEL */
#define DUMMY_BYTE 		    0xA5

/* Define Size */
#define S25FL_PAGE_SIZE 	256
#define S25FL_SECTOR_SIZE 	4096

#define FLASH_S25FL_BASE 	0x0
#define FLASH_S25FL_END 	0x800000    //2048 sector

#define SPI_NSS_GPIO_Port   FLASH_CS_GPIO_Port
#define SPI_NSS_Pin         FLASH_CS_Pin  

#define hSPI                hspi2

/*=========== Function =============*/
void    S25FL_ChipSelect(uint8_t State);
uint8_t S25FL_Send_Byte(uint8_t byte);

void    S25FL_Send_Address (uint32_t address);


#endif /* FLASH_EXT_S25FL_H_ */






