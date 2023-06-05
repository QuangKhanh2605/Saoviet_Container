


#ifndef USER_APP_SIM_H_
#define USER_APP_SIM_H_


#include "user_util.h"

#include "user_define.h"

#include "user_sim.h"
#include "user_mqtt.h"


#define USING_APP_SIM   

/*================ Var struct =================*/
typedef enum
{
    _EVENT_SIM_SEND_MESS,      
    _EVENT_SIM_SEND_PING,  
    _EVENT_SIM_REQ_GPS,
    _EVENT_SIM_GET_GPS,
    
	_EVENT_END_SIM,
}eKindsEventSim;


typedef struct
{
    sData       sDataFlashSim;
    sData       sDataGPS;
    uint8_t     IsGetGPS_u8;           
}sAppSimVariable;



extern sFuncCallBackModem  sModemCallBackToSimHandler;
extern sEvent_struct sEventAppSim []; 
extern sAppSimVariable sAppSimVar;

/*================ Func =================*/
uint8_t     _Cb_Event_Sim_Send_Ping (uint8_t event);
uint8_t     _Cb_Event_Sim_Send_Mess (uint8_t event);
uint8_t     _Cb_Event_Sim_Req_GPS (uint8_t event);
uint8_t     _Cb_Event_Sim_Get_GPS (uint8_t event);
//
void        AppSim_Init (void);
uint8_t     AppSim_Task(void);


uint8_t     AppSim_Send_Mess (void);

void        _CbAppSim_Recv_sTime (ST_TIME_FORMAT sTimeSet);
void        _CbAppSim_TCP_Send_1 (sData *pData);
void        _CbAppSim_TCP_Send_2 (sData *pData);
void        _CbAppSim_Recv_PUBACK (void);

uint8_t     AppSim_Process_AT_Event (uint8_t Type);
void        AppSim_Process_Downl_Mess (sData *sUartSim);
void        AppSim_Process_Sms (sData *sUartSim);
sData       * AppSim_Get_Firmware_Version (void);


void        AppSim_Get_Data_From_Flash (uint8_t MessType, uint8_t *pData, uint16_t Length);
void        AppSim_Push_Mess_To_Flash (uint8_t TypeData, uint8_t PartAorB, uint8_t *pData, uint16_t Length);

void        AppSim_Unmark_Mess (uint8_t TypeMess);

#endif

