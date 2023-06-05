


#ifndef USER_EXTERNAL_MEM_H
#define USER_EXTERNAL_MEM_H


#include "user_util.h"
#include "user_internal_mem.h"
#include "cat24mxx.h"



/*=========== Define =================*/
#define CAT_MAX_SIZE_RECORD             128   //Lon nhat cua cac mess	

#define CAT_SIZE_DATA_TSVH	            128        
#define CAT_SIZE_DATA_EVENT	            64         
#define CAT_SIZE_DATA_LOG	            64   

#define	CAT_MAX_MESS_TSVH_SAVE			50
#define	CAT_MAX_MESS_EVENT_SAVE			10
#define CAT_MAX_MESS_LOG_SAVE           130  

#define CAT_BYTE_EMPTY                  0xFF


/* ================================ Address config DCU ==================================== */

#define	CAT_ADDR_THRESH_MEAS		    576	        //  24 byte thresh
#define	CAT_ADDR_LEVEL_CALIB            640 

#define CAT_ADDR_INDEX_TSVH_SEND        710        /* Inden TSVH Send */ 
#define CAT_ADDR_INDEX_TSVH_SAVE        714        /* Inden TSVH Save */ 

#define	CAT_ADDR_INDEX_EVENT_SEND 		718
#define	CAT_ADDR_INDEX_EVENT_SAVE 	    722

#define	CAT_ADDR_INDEX_LOG_SEND 		746
#define	CAT_ADDR_INDEX_LOG_SAVE 	    750

/*=========Pos Record ================*/
//LOG Mess TSVH (16384 - 800) / 64 = 243: 100pack + 10Pack + 120pack
#define	CAT_ADDR_MESS_A_START   		832	            //50: 128 byte    
#define	CAT_ADDR_MESS_A_STOP			7232
//LOG Mess EVENT
#define	CAT_ADDR_EVENT_START   		    7296            //10 packet + 1 du
#define	CAT_ADDR_EVENT_STOP			    7936
//LOG Mess LOG
#define	CAT_ADDR_LOG_START   		    8000            //130 packet + 1 du
#define	CAT_ADDR_LOG_STOP			    16384




/*================ Func ============================*/
void        ExMem_Save_Index (uint32_t Add, uint16_t Value);
void        ExMem_Init_Record_Index (StructManageRecordFlash *sRecord);

uint8_t     ExMem_Read_Last_Record (StructManageRecordFlash sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime);
uint8_t     ExMem_Save_Record (StructManageRecordFlash *sRecord, uint8_t *pData, uint8_t Length);

uint8_t     ExMem_Read_Record (uint32_t andress, sData *str, uint8_t IndexMess);
uint8_t     ExMem_Read_Record_Without_Index (uint32_t andress, sData *str);

void        Exmem_Get_Infor (uint8_t *pSource, uint8_t *pData, uint16_t *Length, uint8_t MAX_LEGNTH_INFOR);
void        Exmem_Read_Array (uint32_t Addr, uint8_t *pData, uint16_t length);

#endif

