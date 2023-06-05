


#ifndef INC_USER_APP_WM_H_
#define INC_USER_APP_WM_H_


#include "user_util.h"
#include "event_driven.h"

#include "user_lptim.h"
#include "user_internal_mem.h"


/*================ Define ===================*/
#define  USING_APP_WM


/*================ Var struct ===================*/
typedef enum
{
    _EVENT_ENTRY_WM,
	_EVENT_LOG_TSVH,
    _EVENT_MEAS_PRESSURE,
    
    _EVENT_INTAN_IRQ,
    _EVENT_WATING_CONFIG,
    
	_EVENT_END_WM,
}sTypeEventWM;


typedef struct
{
    ST_TIME_FORMAT  sTime;
    uint32_t        Pulse_u32; 
}sLastRecordInfor;

typedef enum
{
	_ALARM_FLOW_LOW = 2,
	_ALARM_FLOW_HIGH,
	_ALARM_PEAK_LOW,
	_ALARM_PEAK_HIGH,
    _ALARM_VBAT_LOW,
    _ALARM_DIR_CHANGE,
    _ALARM_DETECT_CUT,
    
	_ALARM_END,
}Struct_Type_Alarm;


typedef struct
{
    uint8_t             ModBusStatus_u8;   //1: Pending  1: Success
    uint8_t             LevelSlaveID_u8;
    
    int16_t             ZeroPointCalib_i16;
    int16_t             LevelRealValue_i16;
    uint8_t             IsRqIntan_u8;
}SWaterMeterVariable;



extern sEvent_struct sEventAppWM[];

//extern Struct_Battery_Status    sBattery;
//extern Struct_Battery_Status    sVout;
//extern Struct_Battery_Status    sPressure;

extern struct_ThresholdConfig   sMeterThreshold;
extern Struct_Pulse			    sPulse;
extern sLastRecordInfor         sLastPackInfor;    
extern SWaterMeterVariable      sWmVar;
extern uint8_t aMARK_ALARM[10];
extern uint8_t aMARK_ALARM_PENDING[10];


/*=============Function ===========*/
void        Modem_Get_Last_Packet_Infor (void);

void        Modem_Check_Add_5Pulse (Struct_Pulse *sPulse);
void        Modem_Log_Data_TSVH (StructManageRecordFlash *sRecordTSVH);
void        Modem_Log_Data_Event (StructManageRecordFlash *sRecordEvent);

uint8_t     Modem_Check_Alarm (void);
uint8_t     Modem_Packet_TSVH (uint8_t *pData);


uint16_t    AppWm_Get_VBat(void);
uint16_t    AppWm_Get_VOut(void);

uint8_t     AppWm_Task(void);
void        AppWm_Init (void);
void        AppWm_Init_Pulse_Infor (void);
void        AppWm_Init_Thresh_Measure (void);
void        AppWm_Save_Thresh_Measure (void);

int32_t     Modem_Cacul_Quantity (uint32_t PulseCur, uint32_t PulseOld);
uint8_t     Modem_Handler_Alarm (uint8_t Index);

#endif