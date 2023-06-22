


#ifndef USER_APP_SIM_H_
#define USER_APP_SIM_H_


#include "user_util.h"

#include "user_define.h"

#include "user_sim.h"
#include "user_mqtt.h"


#define USING_APP_SIM   


/*GPS: So vi tri lay mau kiem tra di chuyen*/
#define MAX_SAMPLE_GPS      12

/*GPS: Khoang cach so sanh di chuyen*/
#define DISTANCE_GPS_MOVE   50


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
    uint8_t     aData[30];
    uint8_t     Length_u8;
    double      Lat;
    double      Long;
}sGpsData;

typedef struct
{
    sGpsData    sLocation[MAX_SAMPLE_GPS];
    sGpsData    sLocaOrigin;
    
    uint8_t     Status_u8;
    uint8_t     Index_u8;
    uint8_t     IsInit_u8;
    uint8_t     MarkFirstError_u8;
    uint16_t    CountError_u16;
}sGpsInformation;


typedef struct
{
    sData       sDataFlashSim;
    sData       sDataGPS;
    
    sGpsInformation sGPS;
    uint8_t     IsFinishHandleSim_u8;
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

void        AppSim_GPS_OK (void);
uint8_t     AppSim_GPS_Extract_Lat_Long (uint8_t *pData, uint16_t Length, double *Lat, double *Long);

uint8_t     AppSim_GPS_Check_Moving (void);
void        AppSim_GPS_Packet_Record (uint8_t CheckResult);
void        AppSim_GPS_Error (void);

void        AppSim_Start_Module_Sim (void);




#endif

