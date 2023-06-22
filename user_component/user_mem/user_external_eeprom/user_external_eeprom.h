


#ifndef USER_EXTERNAL_EEPROM_H
#define USER_EXTERNAL_EEPROM_H


#include "user_util.h"
#include "cat24mxx.h"

#include "event_driven.h"
#include "queue_p.h"
#include "user_mem_comm.h"

/*=========== Define =================*/
#define CAT_MAX_SIZE_RECORD             128   //Lon nhat cua cac mess	

#define CAT_SIZE_DATA_TSVH	            128        
#define CAT_SIZE_DATA_EVENT	            64         
#define CAT_SIZE_DATA_LOG	            64   
#define CAT_SIZE_DATA_GPS	            256   

#define	CAT_MAX_MESS_TSVH_SAVE			50
#define	CAT_MAX_MESS_EVENT_SAVE			10
#define CAT_MAX_MESS_LOG_SAVE           130 
#define CAT_MAX_MESS_GPS_SAVE           2  

#define CAT_BYTE_EMPTY                  0xFF


/* ================================ Address config DCU ==================================== */

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

#define	CAT_ADDR_GPS_START   		    0            //130 packet + 1 du
#define	CAT_ADDR_GPS_STOP			    768


//Define Variable
#define	EEPROM_MAX_RETRY_WRITE          10

/*========== Var struct ===============*/

typedef enum
{
	_EVENT_EX_EEPROM_WRITE_BUFF = 0,
	_EVENT_EX_EEPROM_READ_BUFF,
    _EVENT_EX_EEPROM_CHECK_QUEUE,

	_EVENT_EX_EEPROM_END, // don't use
}eEVENT_EXTERNL_EEPROM;



extern sEvent_struct        sEventExEEPROM[];
extern sMemoryManager       sExEEPROM;

extern Struct_Queue_Type    qExEEPROMRead;
extern Struct_Queue_Type    qExEEPROMWrite;

/*================ Func ============================*/
void        ExEEPROM_Init (void);
uint8_t     ExEEPROM_Task (void);    

uint8_t     ExEEPROM_Read_Last_Record (sRecordMemoryManager sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime);

uint8_t     ExEEPROM_Status (void);
void        ExEEPROM_Setup_Init (void);
uint8_t     ExEEPROM_Test_Write (void);

uint8_t     ExEEPROM_Is_Queue_Read_Empty (void);
uint8_t     ExEEPROM_Is_Queue_Write_Empty (void);

void        ExEEPROM_Send_To_Queue_Read (sQueueMemReadManager *qRead);
void        ExEEPROM_Send_To_Queue_Write (sQueueMemWriteManager *qWrite);

#endif

