/*
 * user_sim.h
 *
 *  Created on: Dec 31, 2021
 *      Author: Chien
 */

#ifndef USER_SIM_H_
#define USER_SIM_H_

#include "user_util.h"

#include "usart.h"
#include "queue_p.h"

#include "event_driven.h"
#include "user_sim_common.h"


/*======= Define Type SIM: SIM_L506_LIB =============*/  

//#define SIM_L506_LIB
#define SIM_EC200U_LIB  
     
#ifdef SIM_L506_LIB
    #include "user_l506.h"
    #define SIM_STEP_END                                _SIM_END
    #define SIM_STEP_OUT_DATA_MODE                      _SIM_TCP_OUT_DATAMODE
    #define SIM_STEP_ATTACH                             _SIM_NET_CHECK_ATTACH
     
    #define SIM_GET_AT_CMD(sim_step)                    L506_Get_AT_Cmd(sim_step) 
     
    #define SIM_CHECK_STEP_LONG_TIMEOUT(sim_step)       L506_Check_Step_Long_Timeout(sim_step)
    #define SIM_TYPE_DELAY_2_STEP(sim_step)             L506_Get_Delay_2_AT(sim_step)    
    #define SIM_IS_STEP_CHECK_URC(sim_step)             L506_Is_Step_Check_URC(sim_step)  
    #define SIM_IS_STEP_SKIP_ERROR(sim_step)            L506_Check_Step_Skip_Error(sim_step) 
#endif


#ifdef SIM_EC200U_LIB
    #include "user_ec200u.h"
    #define SIM_STEP_END                                _SIM_END
    #define SIM_STEP_OUT_DATA_MODE                      _SIM_TCP_OUT_DATAMODE
    #define SIM_STEP_ATTACH                             _SIM_NET_CHECK_ATTACH
     
    #define SIM_GET_AT_CMD(sim_step)                    EC200U_Get_AT_Cmd(sim_step) 
     
    #define SIM_CHECK_STEP_LONG_TIMEOUT(sim_step)       EC200U_Check_Step_Long_Timeout(sim_step)    
    #define SIM_TYPE_DELAY_2_STEP(sim_step)             EC200U_Get_Delay_2_AT(sim_step)  
    #define SIM_IS_STEP_CHECK_URC(sim_step)             EC200U_Is_Step_Check_URC(sim_step)
    #define SIM_IS_STEP_SKIP_ERROR(sim_step)            EC200U_Check_Step_Skip_Error(sim_step)
#endif



/*================Define================*/
#define SIM_MAX_ITEM_QUEUE  50

#define SIM_CMD_RETRY		2
#define SIM_CMD_FREQ		50
#define SIM_CMD_TIMEOUT		10000
#define SIM_CMD_TIMEOUT2 	5000
#define SIM_TCP_TIMEOUT		60000
#define SIM_MAX_CGATT  	    10
     
#define MAX_HARD_RESET		20
#define MAX_RETRY_CHANGE_SERVER     5
#define MAX_SOFT_RESET		2
     
#define MAX_PING_TO_CONFIG  1
#define MAX_LENGTH_AT_SERVER    40

#define MAX_LEVEL_CSQ     	100
#define MAX_CSQ     	    0x20
#define MIN_CSQ      	    0

#define STEP_INVALID  	    0xFF
     
// ********** ON - OFF POWER
#define SIM_PW_OFF1         HAL_GPIO_WritePin(SIM_ON_OFF_GPIO_Port, SIM_ON_OFF_Pin, GPIO_PIN_RESET)
#define SIM_PW_ON1          HAL_GPIO_WritePin(SIM_ON_OFF_GPIO_Port, SIM_ON_OFF_Pin, GPIO_PIN_SET)

// ********** PWKEY
#define SIM_PWKEY_ON1       HAL_GPIO_WritePin(SIM_PWR_KEY_GPIO_Port, SIM_PWR_KEY_Pin, GPIO_PIN_SET)
#define SIM_PWKEY_OFF1      HAL_GPIO_WritePin(SIM_PWR_KEY_GPIO_Port, SIM_PWR_KEY_Pin, GPIO_PIN_RESET)
//============Sim Reset
#define SIM_RESET_ON1       HAL_GPIO_WritePin(SIM_RESET_GPIO_Port, SIM_RESET_Pin, GPIO_PIN_SET)  
#define SIM_RESET_OFF1      HAL_GPIO_WritePin(SIM_RESET_GPIO_Port, SIM_RESET_Pin, GPIO_PIN_RESET)

// ********** ON - OFF DTR  
#define SIM_DTR_OFF1        //HAL_GPIO_WritePin(SIM_DTR_GPIO_Port, SIM_DTR_Pin, GPIO_PIN_RESET)  
#define SIM_DTR_ON1         //HAL_GPIO_WritePin(SIM_DTR_GPIO_Port, SIM_DTR_Pin, GPIO_PIN_SET)
         
/*===============struct, var================*/
typedef enum
{
	_EVENT_SIM_AT_SEND = 0,
    _EVENT_SIM_UART_RECEIVE,
	_EVENT_SIM_AT_SEND_OK,
	_EVENT_SIM_AT_SEND_TIMEOUT,
	_EVENT_SIM_HARD_RESET,
	_EVENT_SIM_TURN_ON,
	_EVENT_SIM_WAKEUP,
    _EVENT_SIM_DTR_PIN,
    _EVENT_SIM_POWER_OFF,
    
	_EVENT_SIM_END, // don't use
}Event_sim_TypeDef;


typedef struct 
{
    uint8_t     NumRetry_u8;                //So lan retry 1 lenh AT
    uint8_t     NumATProcess_u8;            //So step AT thuc hien 1 lenh
    uint8_t     IsAllowSendAT_u8;
    uint8_t     RespStatus_u8;              //1: Resp TRUE  ; 0: Resp Fail
    
    uint8_t		CountStepCGATT_u8;          //Dem so lan gui lenh Attach: wait attach BTS
    uint8_t		CountHardReset_u8;          //Dem so lan softreset
    uint8_t     CountSoftReset_u8;
    
    uint8_t     StepPowerOn_u8;              //Step Power On
    uint32_t    LastTimeExcuteEvent_u32;     //Last time Excute Event SIM
    uint8_t     ConnSerStatus_u8;      //Trang thai ket noi Server cuoi cung: 1:OK;  0: False
    
    uint8_t     ModeConnectNow_u8;
    uint8_t     ModeConnectFuture_u8;
    uint8_t     ModeConnectLast_u8;         
    
    uint8_t     *ModePower_u8;              //SaveMode;  Connect Continue
    uint8_t     IsUpdateFinish_u8;
    uint8_t     IsRunningATcmd_u8;
    uint32_t    LandMarkSendAT_u32;
}StructSimVariable;


/*====== Ma loi module sim======*/
typedef enum
{
    _SIM_ERROR_PIN_RX,          // k nhan dc rx uart: do chan rx hoac power on fail
    _SIM_ERROR_PIN_TX,          // Truyen Tx vao va k nha dc gi
    _SIM_ERROR_CARD,            //Loi CPIN not ready: k có card sim
    _SIM_ERROR_ATTACH,          //Loi CGATT chua OK
    _SIM_ERROR_TCP,             //Chua ket noi TCP
    _SIM_ERROR_NONE,
}eSimErrorCode;





typedef struct 
{
    void    ( *pSim_Get_Config_UpdateFw ) (uint32_t *AddNewFw, uint16_t *MAX_PAGE);
    uint8_t ( *pSim_Reset_MCU ) (void);
    
    uint8_t ( *pSim_Handler_AT_Cmd ) (uint8_t Type);
    void    ( *pSim_Handler_Downlink_Server ) (sData *sUartSim);
    void    ( *pSim_Process_Sms ) (sData *sUartSim);
    sData*  ( *pSim_Get_Fw_Version) (void);
}sFuncCallBackModem;


/*============ extern Struct var ==================*/
extern uint8_t uartSimBuffReceive[1200];
extern sData sUartSim;

extern StructSimVariable    sSimVar;
extern sEvent_struct        sEventSim[];
extern sFuncCallBackModem   *sFuncCBModem;
extern Struct_Queue_Type    qSimStep;
/*==============Function==================*/
uint8_t 	fPushSimStepToQueue(uint8_t sim_step);
uint8_t 	fPushBlockSimStepToQueue(uint8_t *block_sim_step, uint8_t size);
uint8_t     Sim_Back_Current_AT (uint8_t NumAT);

uint8_t 	Sim_Check_Response(uint8_t sim_step);
uint8_t		Is_SIM_Waitting_Response (void);

uint8_t     Sim_event_active(uint8_t event_sim);
uint8_t     Sim_event_enable(uint8_t event_sim);
uint8_t     Sim_event_disable(uint8_t event_sim);

uint8_t     Sim_Get_Step_From_Queue(uint8_t Type);
void        Sim_Disable_All_Event (void);

void        Sim_Init_Timer_PowerOn (void);
void        Sim_Init_Handler_Process (void);

uint8_t     SIM_POWER_ON(void);
void        Sim_Set_Config_UpdateFw (void);
sData *     Sim_Get_Fw_Version (void);

void        Sim_Push_AT_Publish (void);
void        Sim_Push_AT_Publish_Fb (void);

void        Sim_Set_Server_Infor_Main (sData sIP, sData Port, sData sUserName, sData sPass);
uint8_t     Sim_Get_Server_Infor_Main (uint8_t *pData);
void        Sim_Set_Server_Infor_Backup (sData sIP, sData Port, sData sUserName, sData sPass);
uint8_t     Sim_Get_Server_Infor_Backup (uint8_t *pData);

Struct_Sim_Information *pSim_Get_Infor (void);

void 		Init_IP_Port_Server(void);
void 		Save_Server_Infor (void);
void        Init_Server_BackUp_Infor(void);
void        Save_Server_BackUp_Infor (void);

void 		Sim_Init(void);
uint8_t 	Sim_Task(void);

uint8_t     Sim_Analys_Error (void);
void        Sim_Defaul_Struct_GPS (void);
void        Sim_Clear_Queue_Step (void);



#endif /* USER_SIM_H_ */
