/* Includes ------------------------------------------------------------------*/

#ifndef _EXTERNAL_FLASH_S25FL216K_H
#define _EXTERNAL_FLASH_S25FL216K_H


#include "user_util.h"


/* ===================================================== */
/****************			FLASH – S25FL				****************/
#define S25FL_FAILED        0      /* STATUS ERROR */
#define S25FL_SUCCESS       1	  /* STATUS SUCCESS */

#define LOW    	 			0x00  /* Chip Select line low */
#define HIGH    			0x01  /* Chip Select line high */

#define WIP_Flag   		    0x03 // 0x01 vs 0x02 /* Write In Progress (WIP) flag */  
#define WEL_Flag   		    0x02							/* Chua co check Flag WEL */
#define DUMMY_BYTE 		    0xA5  /* Byte temp */

/* ID command S25FL */
#define S25FL_W_STATUS      0x01
#define S25FL_WRITE         0x02
#define S25FL_READ          0x03
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

/* Define Size */
#define S25FL_PAGE_SIZE 		256		// byte
#define S25FL_SECTOR_SIZE 		4096    // byte  
#define S25FL_NUM_PAGE_MAX 		32768	// page
#define S25FL_NOPAGE_OFSECTOR	16	    // page

#define S25FL_SC_PORT 						GPIOC   			// PORT 
#define S25FL_SC_PIN 						GPIO_PIN_4   		// PIN Select chip

/*============== Dia chi ban tin data==============*/
#define FLASH_S25FL_BASE 					0x000000

#define TIMEOUT_ER_CHIP							300000
#define TIMEOUT_ER_SECTOR						1000
#define TIMEOUT_WR_PAGE							100
#define TIMEOUT_WR_ENABLE						1000

#define	BLANK_SECTOR							0
#define	WRITTING_SECTOR							1
#define	FULL_SECTOR								2
/* 
Chuyen tu dia chi -> page/sector va nguoc lai page/sector -> dia chi 
*/
/*
	Gioi han tren cua dia chi Flash ngoai
 */				
#define deFLASH_S25FL_TOP       (ADDR_BASE_A_LOG_METER + S25FL_PAGE_SIZE*S25FL_NUM_PAGE_MAX - 1)  // 1FFFFF

/*
  Xac dinh so Page tu dia chi Flash ngoai  ( chua check truong hop address < 0xFF)
 */
#define deFLASH_S25FL_PAGE_OF_ADDRESS(address)                                       \
                              ((address - FLASH_S25FL_BASE) / S25FL_PAGE_SIZE + 1) 
															
/*
  Xac dinh so Sector tu dia chi Flash ngoai ( chua check truong hop address < 0xFF)
 */
#define deFLASH_S25FL_SECTOR_OF_ADDRESS(address)                                       \
                              ((address - FLASH_S25FL_BASE) / S25FL_SECTOR_SIZE + 1) 															

/*
	Xac dinh dia chi bat dau cua Page tu so Page (so page bat dau tu 1)
 */
#define deFLASH_S25FL_ADDRESS_OF_PAGE(page)                                       \
                              (FLASH_S25FL_BASE + (page-1) * S25FL_PAGE_SIZE)			
										
/*
  Xac dinh dia chi bat dau cua Sector tu so Sector (so Sector bat dau tu 1)
 */
#define deFLASH_S25FL_ADDRESS_OF_SECTOR(Sector)                                       \
                              (FLASH_S25FL_BASE + (Sector-1) * S25FL_NOPAGE_OFSECTOR * S25FL_PAGE_SIZE)			

/*
  Kiem tra dia chi ghi Flash ben trong vung dc su dung
 */
#define deFLASH_S25FL_IS_ADDRESS_USERSPACE(address)                                  \
                        (address >= ADDR_BASE_A_LOG_METER && address <= FLASH_S25FL_TOP)
/*
    Kiem tra dia chi co phai dia chi bat dau sector khong?
*/
#define deFLASH_S25FL_IS_NEW_SECTOR(address)                                       \
                              (((address - FLASH_S25FL_BASE) / S25FL_PAGE_SIZE)%16)


/*=============== Struct var =======================*/
extern SPI_HandleTypeDef hspi1;
                                

/*========================= Functions  ==============================*/
void        Flash_S25FL_ChipSelect(uint8_t State);
/* Xac dinh loai Chip Flash */
uint32_t    Flash_S25FL_Connect(void);
// Ham doc toan bo du lieu Flash
void        Flash_S25FL_StartReadSequence(uint32_t ReadAddr);
// Ham doc 1 byte tu Flash, ham nay chi su dung khi dung ham Flash_S25FL_StartReadSequence
uint8_t     Flash_S25FL_ReadByte(void);
// Ham xoa Sector
uint8_t     Flash_S25FL_Erase_Sector(uint32_t SectorAddr);
// Ham xoa chip
uint8_t     Flash_S25FL_Erase_Chip(void);
// Truyen 1 byte qua SPI
uint8_t     Flash_S25FL_Send_Byte(uint8_t byte);
// kiem tra xem co the thuc hien lap trinh khong (ghi hoac xoa)
void        Flash_S25FL_Enable(void);
/* kiem tra co Flash co dang ban khong */
uint8_t     Flash_S25FL_Busy(void);  
/* Function Name  : Flash_S25FL_WaitForWriteEnd */
uint8_t     Flash_S25FL_WaitForWriteEnd(uint32_t timeout);
/* Kiem tra Write Enable */
uint8_t     Flash_S25FL_CheckWriteEnable(uint32_t timeout);
/* Doc thanh ghi trang thai cua Chip */
uint8_t     Flash_S25FL_Read_Status(void);
/* Kiem tra de xoa cac Sector truoc khi ghi moi */
void        Flash_S25FL_Check_Erase_Sector(uint16_t No_Page);
void        S25FL_Write_Disable(void);
// Ham ghi 1 page (do dai byte phai be hon hoac bang kich thuoc 1 page
uint8_t     Flash_S25FL_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
// Ham ghi chuoi du lieu co chieu dai bat ki
uint8_t     Flash_S25FL_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
uint8_t     Flash_S25FL_Read_StatusRegis (void);
uint8_t     Flash_S25FL_SoftReset(void);
uint8_t     Flash_S25FL_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);


#endif /* _START_INC_s25FL216K_H_ */
