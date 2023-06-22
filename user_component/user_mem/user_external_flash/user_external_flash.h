
/*
    8/2021
    Thu vien Quan ly extern flash
*/

#ifndef _USER_EXTERNAL_FLASH_H
#define _USER_EXTERNAL_FLASH_H

#include "user_util.h"
#include "S25FL216K.h"

#include "event_driven.h"
#include "queue_p.h"

#include "user_mem_comm.h"

/*
    0: Luu Index send save Onchipflash: 
    1: Luu Part B
*/
#define TYPE_PART_AB                0
#define TYPE_INDEX                  1

#define FLASH_MANAGE_TYPE           TYPE_PART_AB

/*===================Define======================================*/

#define TIMEOUT_ER_SECTOR				1000
#define TIMEOUT_WR_PAGE					100
#define TIMEOUT_WR_ENABLE				1000


#define FLASH_CMD_TIMEOUT               (1000)
#define FLASH_CMD_TIME 	                (1)
#define FLASH_ERASE_TIME                50

#define FLASH_ADDR_TEST 		        0x000000		//Sector 0

#define FLASH_ADDR_TSVH_A_START 		0x00A000		//Sector 10
#define FLASH_ADDR_TSVH_A_STOP			0x0F9FFF		//Sector 250 (bao gom ca sector nay)

#define FLASH_ADDR_TSVH_B_START 		0x104000		//Sector 260
#define FLASH_ADDR_TSVH_B_STOP			0x1F3FFF		//Sector 500 (bao gom ca sector nay)

#define FLASH_ADDR_TSVH_OFFSET			0x0FA000		//FLASH_ADDR_TSVH_B_START - FLASH_ADDR_TSVH_A_START = 240 sector

#define FLASH_ADDR_EVENT_A_START 		0x1F5000		//Sector 501
#define FLASH_ADDR_EVENT_A_STOP			0x208FFF		//Sector 521

#define FLASH_ADDR_EVENT_B_START 		0x20B000		//Sector 523
#define FLASH_ADDR_EVENT_B_STOP			0x21EFFF		//Sector 543

#define FLASH_ADDR_EVENT_OFFSET   		0x016000		//FLASH_ADDR_EVENT_B_START - FLASH_ADDR_EVENT_A_START = 20 sector

#define FLASH_ADDR_LOG_START 			0x221000		//Sector 545		
#define FLASH_ADDR_LOG_STOP				0x2346FF		//Sector 565	20 sector * 4096 / (64 byte 1 record log) = 1280 rec

#define FLASH_ADDR_GPS_A_START 			0x237000		//Sector 567		
#define FLASH_ADDR_GPS_A_STOP			0x7AE6FF		//Sector 1266	699 sector = 2796 record (x 1024 byte) = 11184 record (x256 byte)

#define FLASH_ADDR_GPS_B_START 			0x237000		//Sector 1268		
#define FLASH_ADDR_GPS_B_STOP			0x7AE6FF		//Sector 1967	699 sector = 2796 record (x 1024 byte) = 11184 record (x256 byte)

#define FLASH_ADDR_GPS_OFFSET   		0x0FA000        // FLASH_ADDR_GPS_B_START -  FLASH_ADDR_GPS_A_START = 

/*--------- Max size record ----------*/
#define FLASH_MAX_RECORD_TSVH           500
#define FLASH_MAX_RECORD_EVENT          500
#define FLASH_MAX_RECORD_LOG            1000
#define FLASH_MAX_RECORD_GPS            10000

#define FLASH_SIZE_DATA_TSVH            128
#define FLASH_SIZE_DATA_EVENT           64
#define FLASH_SIZE_DATA_LOG             64
#define FLASH_SIZE_DATA_GPS             256

#define FLASH_MAX_DATA_READ             1024

#define FLASH_MAX_RETRY_ITEM            3
#define FLASH_MAX_RETRY_SKIP            6


#define FLASH_MAX_RETRY_TEST            3    

/*===================Var struct======================================*/

#define FLASH_CACUL_INDEX(Addr, AddrStart, SizeRec)   (Addr - AddrStart) / SizeRec 
                

typedef enum
{
	_EVENT_FLASH_SEND_BYTE = 0,
	_EVENT_FLASH_WRITE_BUFF,
	_EVENT_FLASH_READ_BUFF,
    _EVENT_CHECK_QUEUE,

	_EVENT_FLASH_END, // don't use
}Event_Flash_TypeDef;

typedef enum
{

	_FLASH_CMD_READ_STATUS = 0,
	_FLASH_CMD_CHECK_WRITE_ENABLE,
	_FLASH_CMD_CHECK_ERASE_ENABLE,

	_FLASH_CMD_CHECK_WRITE_END,
	_FLASH_CMD_CHECK_ERASE_END,

	_FLASH_CMD_ERASE_SECTOR,
	_FLASH_CMD_WRITE_DATA,
	_FLASH_CMD_READ_DATA,

	_FLASH_CMD_ACTIVE_CS,
	_FLASH_CMD_DEACTIVE_CS,

	_FLASH_CMD_ENABLE,
    
    _FLASH_CMD_ERASE_DELAY,
    _FLASH_CMD_READ_CHECK_WRITE,
    
    _FLASH_CMD_ERASE_CHIP,
    _FLASH_CMD_ERASE_CHIP_END,
    
	_FLASH_CMD_END, // don't use
}Cmd_Flash_TypeDef;


typedef uint8_t (*fp_flash_cmd_callback) (void);

typedef struct
{
	uint8_t cmd_name;
	uint8_t cmd_byte;
	uint8_t cmd_check;
	fp_flash_cmd_callback callback_success;
	fp_flash_cmd_callback callback_failure;
}sFlash_CMD_struct;


typedef struct
{
    uint8_t         StepInit_u8;
    uint8_t         Pending_u8;
    uint8_t         Status_u8;
    
    uint32_t        PosFindData_u32;
    uint8_t         FlagMark_u8;
    uint8_t         IsFullArea_u8;
    uint8_t         IsEmptyArea_u8;
}sFlashInitVar;


extern sEvent_struct        sEventExFlash[];
extern sFlash_CMD_struct	aFlashCmd[];

extern Struct_Queue_Type    qFlashStep;
extern Struct_Queue_Type    qFlashRead;
extern Struct_Queue_Type    qFlashWrite;

extern sMemoryManager       sExFlash;
extern sFlashInitVar        sExFlashInit;

/*===================Function======================================*/
uint8_t eFlash_Init(void);
uint8_t eFlash_Task (void);


void    eFlash_Send_Address(uint32_t address);

uint8_t eFlash_Push_Step_To_Queue(uint8_t step);
uint8_t eFlash_Push_Block_To_Queue(uint8_t *block_flash_step, uint8_t size);
uint8_t eFlash_Get_Step_From_Queue(uint8_t Type);

/*========== Func test ===========*/

void    eFlash_S25FL_Enable(void);
uint8_t eFlash_S25FL_Erase_Sector(uint32_t SectorAddr);
uint8_t eFlash_S25FL_CheckWriteEnable(uint32_t timeout);
uint8_t eFlash_S25FL_WaitForWriteEnd(uint32_t timeout);
uint8_t eFlash_S25FL_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
uint8_t eFlash_S25FL_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

uint8_t eFlash_S25FL_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
/*====== Func Handle record =========*/
void    eFlash_Default_Struct_Init_Pos (void);
void    eFlash_Push_Step_Erase_Sector (void);
void    eFlash_Push_Step_Erase_Chip (void);

uint8_t eFlash_Test_Write (void);
uint8_t eFlash_Status (void);
void    eFlash_Setup_Init (void);
uint8_t eFlash_Is_Queue_Read_Empty (void);
uint8_t eFlash_Is_Queue_Write_Empty (void);
void    eFlash_Check_Data_Init_Pos (uint32_t Addr, uint8_t *pData, uint16_t Length, uint8_t PartAorB);

void    eFlash_Send_To_Queue_Read (sQueueMemReadManager *qRead);
uint8_t eFlash_Check_Write_To_PartAB (void);
void    eFlash_Send_To_Queue_Write (sQueueMemWriteManager *qWrite);

uint8_t eFlash_Check_Pos_PartA (sRecordMemoryManager *sRec);
uint8_t eFlash_Check_Pos_PartB (sRecordMemoryManager *sRec);


#endif /*  */