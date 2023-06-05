


#ifndef USER_APP_SIM_H_
#define USER_APP_SIM_H_


#include "user_util.h"
#include "user_sim.h"
#include "user_mqtt.h"


#define USING_APP_SIM   

/*================ Var struct =================*/
typedef enum
{
    _EVENT_SIM_SEND_MESS,      
    _EVENT_SIM_SEND_PING,      
    
	_EVENT_END_SIM,
}eKindsEventSim;


extern sFuncCallBackModem  sModemCallBackToSimHandler;
extern sEvent_struct sEventAppSim []; 


/*================ Func =================*/
uint8_t     _Cb_Event_Sim_Send_Ping (uint8_t event);
uint8_t     _Cb_Event_Sim_Send_Mess (uint8_t event);
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


#endif

