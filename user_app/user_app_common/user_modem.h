
/*
    8/2021
    Thu vien quan ly modem
*/

#ifndef USER_MODEM_H
#define USER_MODEM_H

#include "user_util.h"

#include "user_define.h"

#include "user_log.h"
#include "user_lpm.h"

#include "user_app_mem.h"
#include "user_mem_comm.h"

/*===================Define=========================*/
#define MAX_DCU_ID_LENGTH	    20
#define MAX_ALARM_CONTINUOUS    30

#define VDD_INTERNAL_MAX        3600     
#define VDD_INTERNAL_MIN        2800

#define LEGNTH_AT_SERIAL        256
#define MAX_LENGTH_ALAMR_EMER   1024

/*===================Struct, Var=========================*/
/*
 * 		KIND DCU
 */
typedef enum
{
	_DHN_NBIOT_1,
	_WM_GSM_HOABINH,
	_METER_COSPHI,
	_EMET_NB_IOT,
    _WM_GSM_LEVEL,
    _WM_GSM_LORA,
    _TEM_HUMI_GSM,
    _WM_GSM_CONV,
    _CONTROL_INVERTER,
    
} Type_Mark_DCU;

typedef enum
{
    _AT_REQUEST_SERIAL = 1,
    _AT_REQUEST_SERVER = 2,
    _AT_REQUEST_LORA = 3,
}Kind_Request_AT_Cmd;



typedef struct
{
	//Information
	uint32_t 		MarkResetChip_u8;
    
	sData  			sDCU_id;
	//Variable
	sData 			sPayload;
	uint8_t			TypeModem_u8;		            //loai DCU

	uint8_t			IsSaveBoxMode_u8;		        //Flag danh dau vao che do Save box
	uint8_t			IsSendIntanPacket_u8;	        //Danh dau gui tuc thoi bang PIN

    uint8_t	        CountSleepToSend_u8;
    uint8_t         rExternIrq_u8;
    //
    uint8_t         ModeSimPower_u8;               //0: Connecting Alway;  1:Save Power Mode
    uint32_t        PressureValue_u32; 
    uint8_t         rTestPulse_u8; 
    uint8_t         DetectCutStatus_u8;
    uint8_t         rGetCSQagain_u8;             //Lay csq truoc khi Send Data
    uint8_t         IRQPowerOutAgain_u8;

    uint8_t 		IsReqIntanMess_u8;
    uint8_t 		WaitingResetMCU_u8;
    
    sData           strATResp;
    sData           strATCmd;
    uint8_t         IsOverFivePulse_u8;           //Pulse Init
    sData           strAlarmEmer;
    uint8_t 		IsDutyCycle_u8;
    uint8_t         CountEventWaitConfg_u8;
    
    uint32_t        TimeDelayTx_u32;
}Struct_Modem_Variable;



typedef struct
{
    uint8_t			NumWakeup_u8;                //So lan lay mau roi gui ban tin: Che do SAVE_MODE
    uint16_t        FreqWakeup_u32;              //Chu ki thuc day
    
    uint32_t		FreqSendOnline_u32;            //Tan suat gui ban tin: che do ONLINE
	uint32_t		FreqSendUnitMin_u32;         //Gia tri dong goi vao ban tin
}SModemFreqActionInformation;



/*===================External Var, Struct=========================*/
extern Struct_Modem_Variable	sModem;
extern ST_TIME_FORMAT           sRTCSet;

extern sRecordMemoryManager     sRecTSVH;
extern sRecordMemoryManager     sRecEvent;
extern sRecordMemoryManager     sRecLog;
extern sRecordMemoryManager     sRecGPS;

extern SModemFreqActionInformation     sFreqInfor; 
extern StructpFuncLPM           sModemLPMHandler;
//Extern Buffer
extern uint8_t aDCU_ID[MAX_DCU_ID_LENGTH];

extern uint8_t aRESPONDSE_AT [LEGNTH_AT_SERIAL];
/*================Extern var struct=====================*/


/*===================Function=========================*/
/*------ Cac function xu ly cac task----------------*/
uint8_t  	Reset_Chip (void);
void 		Reset_Chip_Immediately (void);
void        Modem_Log_And_Send_Emergency (uint8_t *pData, uint16_t length);

void        Init_Server_BackUp_Infor(void);
void        Save_Server_BackUp_Infor (void);

//Func Modem App
void        Modem_Get_Config_UpdateFw (uint32_t *AddNewFw, uint16_t *MAX_PAGE);;
void        DCU_Respond(uint8_t portNo, uint8_t *data, uint16_t length, uint8_t Type);

void        Modem_Deinit_Peripheral (void);
void        Modem_Init_Peripheral (void);
void        Modem_Init_Before_IRQ_Handle (void);
void        Modem_Deinit_Before_IRQ_Handle (void);

void        MX_GPIO_DeInit(void);
void        Modem_Uart_DeInit(void);
void        Modem_Init_Gpio_Again(void);




#endif /*  */






