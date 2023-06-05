
/*
    8/2021
    Thu vien Quan ly extern flash
*/

#ifndef _USER_EXTERNAL_FLASH_H
#define _USER_EXTERNAL_FLASH_H

#include "user_util.h"
#include "S25FL216K.h"


/*===================Define======================================*/
#define METER_LOG_MESSAGE_SIZE			1280 	// byte 
#define LENGTH_FIRM_LOG			        1024 	// byte 
#define DCU_LOG_MESSAGE_SIZE			256 	// byte 
#define FLASH_BYTE_FDEFAULT             0xFF    

/*==== Add App DK relay=========*/
#define ADD_NUM_RELAY_USING             1 	            //Num Sector
#define ADD_CONFIG_RELAY_1              2
/*==== Add App DCU ======*/


#define ADDR_METER_A_START 			 		0x00A000		//Sector 11
#define ADDR_METER_A_STOP					0x0F9FFF		//Sector 250 (bao gom ca sector nay)

#define ADDR_METER_B_START 			 		0x104000		//Sector 261
#define ADDR_METER_B_STOP					0x1F3FFF		//Sector 500 (bao gom ca sector nay)

#define ADDR_OFFSET_METER_INFO				0x0FA000		//ADDR_METER_B_START - ADDR_METER_A_START

#define ADDR_LPF_A_START 			 		0x1FE000		//Sector 511
#define ADDR_LPF_A_STOP					    0x2EDFFF		//Sector 750

#define ADDR_LPF_B_START 			 		0x2F8000		//Sector 761
#define ADDR_LPF_B_STOP					    0x3E7FFF		//Sector 1000

#define ADDR_OFFSET_METER_LPF				0x0FA000		//ADDR_LPF_B_START - ADDR_LPF_A_START

#define ADDR_BILL_A_START		 		    0x3F2000		//Sector 1011
#define ADDR_BILL_A_STOP				    0x455FFF		//Sector 1110       //moi cai 100 sector

#define ADDR_BILL_B_START		 		    0x46A000		//Sector 1131
#define ADDR_BILL_B_STOP				    0x4CDFFF		//Sector 1230

#define ADDR_OFFSET_METER_BILL				0x078000

#define ADDR_SENT_BILLING_MARK				0x4E6000		//Sector 1255

#define ADDR_BASE_LOG_EVEN 			 		0x4EC000		//Sector 1261
#define ADDR_TOP_LOG_EVEN					0x5DBFFF		//Sector 1500		

#define ADDR_BASE_LOG_DCU 			 	    0x5E6000		//Sector 1511		
#define ADDR_TOP_LOG_DCU					0x6246FF		//Sector 1750	0x624800  la 1000 page	. chi ghi den 999 Page thoi tu 1-999

#define ADDR_BASE_FIRM_DCU 			 		0x6F0000		//Sector 1761		
#define ADDR_CRC_FIRM   					0x7D0000	

#define ADDR_MESS_FAIL_TSVH        			0x7D1000      
#define ADDR_MESS_FAIL_BILL   			    0x7D2000     
#define ADDR_MESS_FAIL_LPF   			    0x7D3000        




/*===================Var struct======================================*/
typedef enum
{
    _PART_A = 0,
    _PART_B = 1,
}Struct_Type_Save_Flash;

typedef struct Manage_Flash_Struct
{
	uint8_t			Step_ui8;
	uint16_t		BuffA_Writting_S_ui16;				// Meter -> Flash  : Doc meter thanh cong
	uint16_t		BuffB_Writting_S_ui16;				// Sim900 -> Flash : da day len Server thanh cong
	uint8_t			BuffA_Change_Sector_ui8;            // Phia log vao flash da lam thay doi vi tri Pos PartB. thi se k cho ghi vao
	uint32_t 		LandMarkCheck_u32;
    
    uint32_t        AddStartA_u32;
    uint32_t        AddStopA_u32;
    uint32_t        AddStartB_u32;
    uint32_t        AddStopB_u32;
    uint32_t        AddOffsetAB_u32;
    
    uint32_t        PosMessPartA_u32;
    uint32_t        PosMessPartB_u32;
}Manage_Flash_Struct;

typedef struct
{
    uint8_t			Error_ui8;
    uint8_t         CountRetryInit_u8;
    uint32_t        LandMarkInitFlash_u32;
    uint8_t         rEraseFlash_u8;
}Struct_Manage_External_Flash;


typedef struct 
{
 	uint8_t			MessType_u8;			// 0 - Operation; 1 - Alarm; 2 - Billing; 3 - Even
 	uint8_t			MessDirect_u8;		    // 0-Read	1-Write
 	sData 	        strData;
 	uint8_t			MessStatus_u8;		    // 0 : don't RUN ; 1 : Success; 2 : Error
    uint8_t         NumRetry;              // 5/7/21
}Meter_Flash_Queue_Struct;


typedef enum
{
    _DIRECT_BACK,
    _DIRECT_FORWARD,
}sType_Direct;


typedef enum
{
    _OLD_TSVH,
    _OLD_BILLING,
    _OLD_LPF,
}struct_Type_Old_Data;

typedef struct
{
    uint32_t        AddMax_u32;                 //Addmax cua vung luu data
    uint32_t        AddMin_u32;                 //Add min cua vung luu data
    
    uint8_t         IsRequest_u8;                //1: co request doc lai old data
    ST_TIME_FORMAT  sTimeStart;                 //thoi gian start req
    ST_TIME_FORMAT  sTimeStop;                  //thoi gian stop req
    uint32_t        PosCurrentRead_u32;         //Vi tri dang doc old data
    uint8_t         Step_u8;                    //Step read old data    
    uint8_t         TypeReqAT_u8;               //Req bang AT hay Server
    uint32_t        LandMarkRead_u32;           //Moc thoi gian doc xong
}struct_Manage_Read_Old_Packet;


/*=================== Extern Var struct======================================*/
extern Manage_Flash_Struct				Manage_Flash_TSVH;
extern Manage_Flash_Struct				Manage_Flash_Lpf;
extern Manage_Flash_Struct				Manage_Flash_Bill; 

extern Struct_Manage_External_Flash     sExFlash;

extern uint32_t         pos_DCU_u32;           //Vi tri luu log
extern uint32_t         pos_FirmWrite_u32;
extern uint32_t         pos_FirmRead_u32;
extern uint8_t          ReadBackBuff[METER_LOG_MESSAGE_SIZE];

extern struct_Manage_Read_Old_Packet    sOldPacket[3];


//Khi bao cac queue
extern Meter_Flash_Queue_Struct		sQMeterFlashInfo, *ptrsQMeterFlashInfo;
extern Meter_Flash_Queue_Struct		sQMeterSIM_Data, *ptrsQMeterSIM_Data;

extern Meter_Flash_Queue_Struct		sQFlashSIM_Info1, *ptrsQFlashSIM_Info1;
extern Meter_Flash_Queue_Struct		sQFlashSIM_Info2, *ptrsQFlashSIM_Info2;




/*===================Function======================================*/
uint8_t     Init_External_Flash(void);
uint8_t     Check_Flash_Pos(Manage_Flash_Struct *ManageType);
uint8_t     Check_Flash_1Part (uint32_t *PosResult, uint32_t ADD_START, uint32_t ADD_STOP);
uint8_t     Flash_Log_Message(Manage_Flash_Struct *ManageType, uint8_t *Buff, uint16_t length, uint16_t TypePartAB);
uint8_t     Flash_Log_Message_1Part (uint32_t *PosTaget, uint32_t ADD_START, uint32_t ADD_STOP, uint16_t SIZE_LOG, uint8_t *Buff, uint16_t length);
uint8_t     Flash_Log_Firm (uint8_t *Buff, uint16_t length, uint8_t type, uint8_t *CrcResult) ;


uint8_t     ExFlash_Save_Data_To_Sector (uint32_t Add, uint8_t *pData, uint16_t Length);
void        ExFlash_Erase (void);

#endif /*  */