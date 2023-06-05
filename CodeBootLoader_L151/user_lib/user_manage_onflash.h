/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_MANAGE_ONFLASH_H
#define USER_MANAGE_ONFLASH_H

#include "user_util.h"
#include "onchipflash.h"

/*======================== Define ======================*/
#define ON_FLASH_BASE               0x08000000
#define	ADDR_FLASH_END			    0x0803F800

//===============Define address=================
#if defined (STM32L433xx) 
    #define	ADDR_MAIN_PROGRAM			0x08004000	        //43 Page cho chuong trinh chinh
    #define	ADDR_UPDATE_PROGRAM			0x08019800	        //43 page -> End Program = 0x0802F000

    #define ADDR_FLAG_HAVE_NEW_FW       0x08030000

    #define	ADDR_INDEX_LOG_SEND 		0x08031000
    #define	ADDR_INDEX_LOG_SAVE 	    0x08031800

    #define	ADDR_SERVER_BACKUP			0x08032000

    #define	ADDR_MESS_A_START   		0x08032800	         //
    #define	ADDR_MESS_A_STOP			0x08033000

    #define	ADDR_EVENT_START   		    0x08032800	         //
    #define	ADDR_EVENT_STOP			    0x08033000

    #define	ADDR_LOG_START   		    0x08034000	         //8 page * 2K = 16 K
    #define	ADDR_LOG_STOP			    0x08038000           //64*200 = 12800 = hơn 12k . 

    //Config threshold tu dia chi C380
    #define	ADDR_INDEX_EVENT_SEND 		0x08038800
    #define	ADDR_INDEX_EVENT_SAVE 	    0x08039000

    #define	ADDR_COFI_LOW_BATERY		0x08039800	        //  San luong tren
    #define	ADDR_COFI_HIGH_PEAK			0x0803A000	        //  San luong tren
    #define	ADDR_COFI_LOW_PEAK			0x0803A800	        // San luong duoi

    #define	ADDR_COFI_HIGH_FLOW			0x0803B000	        // Luu luong tren     Pulse per hour
    #define	ADDR_COFI_LOW_FLOW			0x0803B800	        //  luu luong duoi    Pulse per hour
    /*======= ADD Server infor ===========*/

    #define	ADDR_SERVER_INFOR			0x0803C000

    #define	ADDR_INDEX_TSVH_SEND 		0x0803C800
    #define	ADDR_INDEX_TSVH_SAVE 		0x0803D000

    #define	ADDR_METER_NUMBER			0x0803D800
    #define	ADDR_CONFIG       	        0x0803E000
    #define	ADDR_INIT_BC66       	    0x0803E800
    #define	ADDR_FLAG_RESET		        0x0803F000 
    #define ADDR_DCUID         			0x0803F800        // sDCU.sDCU_id

    #define	FLASH_MAX_MESS_TSVH_SAVE	2                 //100
    #define	FLASH_MAX_MESS_EVENT_SAVE   2                 //100
    #define FLASH_MAX_MESS_LOG_SAVE     200  

    #define MAX_SIZE_FIRMWARE           84     //Kb
    #define FIRMWARE_SIZE_PAGE	        MAX_SIZE_FIRMWARE*1024/FLASH_PAGE_SIZE
    #define SIZE_DATA_RECORD	        64          //64 byte: 4 record 1 page
    #define SIZE_DATA_LOG	            64          //64 byte
#endif


/*=========== define cho chip L151 ==================*/

#if defined (STM32L151xC) 
    #define	ADDR_MAIN_PROGRAM			0x08004000	        //43 Page cho chuong trinh chinh
    #define	ADDR_UPDATE_PROGRAM			0x08019800	        //43 page -> End Program = 0x0802F000

    #define ADDR_FLAG_HAVE_NEW_FW       0x08030000

    /*======= ADD Record ===========*/
    #define	ADDR_MESS_A_START   		0x08032000	         //8 K
    #define	ADDR_MESS_A_STOP			0x08034100

    #define	ADDR_EVENT_START   		    0x08035000	         //8 K
    #define	ADDR_EVENT_STOP			    0x08037100

    #define	ADDR_LOG_START   		    0x08038000	         //8 K
    #define	ADDR_LOG_STOP			    0x0803A100           //64*128 = 8192 = 8k . 

    /*======= ADD Index Record ===========*/
    #define	ADDR_INDEX_LOG_SEND 		0x0803CA00
    #define	ADDR_INDEX_LOG_SAVE 	    0x0803CB00

    #define	ADDR_INDEX_EVENT_SEND 		0x0803CC00
    #define	ADDR_INDEX_EVENT_SAVE 	    0x0803CD00

    #define	ADDR_INDEX_TSVH_SEND 		0x0803CE00
    #define	ADDR_INDEX_TSVH_SAVE 		0x0803CF00

    //Config threshold tu dia chi C380
    #define	ADDR_COFI_LOW_BATERY		0x0803DB00	    //  San luong tren
    #define	ADDR_COFI_HIGH_PEAK			0x0803DC00	    //  San luong tren
    #define	ADDR_COFI_LOW_PEAK			0x0803DD00	    // San luong duoi

    #define	ADDR_COFI_HIGH_FLOW			0x0803DE00	    // Luu luong tren     Pulse per hour
    #define	ADDR_COFI_LOW_FLOW			0x0803DF00	    //  luu luong duoi    Pulse per hour

    /*======= ADD Information ===========*/
    #define	ADDR_FREQ_ONLINE			0x0803F300
    #define	ADDR_SERVER_BACKUP			0x0803F400
    #define	ADDR_SERVER_INFOR			0x0803F500
    #define	ADDR_METER_NUMBER			0x0803F600
    #define	ADDR_CONFIG       	        0x0803F700
    #define ADDR_DCUID         			0x0803F800        // sDCU.sDCU_id

    #define	FLASH_MAX_MESS_TSVH_SAVE	60   
    #define	FLASH_MAX_MESS_EVENT_SAVE   60   
    #define FLASH_MAX_MESS_LOG_SAVE     128  

    #define MAX_SIZE_FIRMWARE           84     //Kb
    #define FIRMWARE_SIZE_PAGE	        MAX_SIZE_FIRMWARE*1024/FLASH_PAGE_SIZE
    #define SIZE_DATA_RECORD	        128         //128 byte: 2 record 1 page
    #define SIZE_DATA_LOG	            64          //64 byte   

#endif


/*============== Inline Flash ===================*/
#define _ON_FLASH_IS_NEW_PAGE(address)     \
                    ((address - ON_FLASH_BASE) % FLASH_PAGE_SIZE)
                                  
/*======================== Structs var======================*/
typedef struct
{
	uint16_t        IndexSend_u16;                  //vi tri doc ra va gui
    uint16_t        IndexSave_u16;                  //vi tri luu tiep theo
    
    uint32_t        AddStart_u32;
    uint32_t        AddStop_u32;
    
    uint16_t        MaxRecord_u16;                  //Max record Save in Flash
    uint16_t        SizeRecord_u16;                 //Size record
    
    uint32_t        AddIndexSend_u32;               //Add luu index send
    uint32_t        AddIndexSave_u32;
        
    uint16_t        CountMessReaded_u16;
    uint16_t        CountMessPacket_u16;            //So ban tin doc ra de dong goi    

} StructManageRecordFlash;



/*======================== Function ======================*/
uint8_t     Flash_Check_Have_Packet (uint8_t IndexSend, uint8_t IndexSave);
void        Flash_Read_Old_Packet (uint8_t Type);
void        Flash_Save_U16 (uint32_t andress, uint16_t Num);

uint8_t     Flash_Read_Record (uint32_t andress, sData* str, uint8_t IndexMess);
uint8_t     Flash_Read_Record_Without_Index (uint32_t andress, sData* str);

uint8_t     Flash_Read_Last_Record (StructManageRecordFlash sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime);
uint8_t     Flash_Save_Record (StructManageRecordFlash *sRecord, uint8_t *pData, uint8_t Length);

void        Flash_Save_Index (uint32_t Add, uint16_t Value);
//Function Init
void        Flash_Init_Record_Index (StructManageRecordFlash *sRecord);
void        Flash_Get_Infor (uint32_t AddStart, uint8_t *pData, uint16_t *Length, uint8_t MAX_LEGNTH_INFOR);



#endif
