



#ifndef USER_APP_WM_H_
#define USER_APP_WM_H_


#include "user_util.h"
#include "event_driven.h"

#include "user_adc.h"

#include "user_external_mem.h"
#include "user_internal_mem.h"


#define  USING_APP_TEMH

/*============= Define =====================*/
#define VDD_OUT_MAX             12000     
#define VDD_OUT_MIN             9000

#define VDD_BAT                 3600     
#define VDD_MIN                 2800

#define V12_IN_ON               HAL_GPIO_WritePin(ON_OFF_12V_GPIO_Port, ON_OFF_12V_Pin, GPIO_PIN_SET)
#define V12_IN_OFF              HAL_GPIO_WritePin(ON_OFF_12V_GPIO_Port, ON_OFF_12V_Pin, GPIO_PIN_RESET)

#define V_PIN_ON                HAL_GPIO_WritePin(ON_OFF_BOOT_GPIO_Port, ON_OFF_BOOT_Pin, GPIO_PIN_SET)
#define V_PIN_OFF               HAL_GPIO_WritePin(ON_OFF_BOOT_GPIO_Port, ON_OFF_BOOT_Pin, GPIO_PIN_RESET)


#define SLAVE_ID_DEFAULT        0x1A

#define MAX_POINT_CALIB         14
#define DEFAULT_POINT_CALIB     11
#define REAL                    0
#define SAMP                    1

#define LED_SIM_Pin             LED_3_Pin
#define LED_SIM_GPIO_Port       LED_3_GPIO_Port   
  
#define UART_485	            huart1


/*================ var struct =================*/
typedef enum
{
    _EVENT_ENTRY_TEMH,
    _EVENT_LOG_TSVH,             //0
    _EVENT_CONTROL_LED1,         //1
    
    _EVENT_TEST_RS485,
    
	_EVENT_END_TEMP_HUMI,
}eKindEventWm;


typedef enum
{
    _LED_MODE_ONLINE_INIT,
    _LED_MODE_CONNECT_SERVER,
    _LED_MODE_UPDATE_FW,
    _LED_MODE_POWER_SAVE,
    _LED_MODE_TEST_PULSE,
}sKindModeLed;

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
    uint16_t    FlowHigh;
    uint16_t    FlowLow;
    
    uint16_t    PeakHigh;
    uint16_t    PeakLow;
    
    uint8_t     LowBatery;
    
    int16_t     LevelHigh;
    int16_t     LevelLow;
}struct_ThresholdConfig;



typedef struct
{
    uint8_t SlaveID_u8;
}STempHumiVariable;


extern Struct_Battery_Status    sBattery;
extern Struct_Battery_Status    sVout;

extern struct_ThresholdConfig   sMeterThreshold;

extern uint8_t aMARK_ALARM[10];
extern uint8_t aMARK_ALARM_PENDING[10];

extern sEvent_struct sEventAppTempH[];

extern STempHumiVariable  sTempHumi;


/*================ Function =================*/
uint8_t     _Cb_Log_TSVH (uint8_t event);
uint8_t     _Cb_Control_Led1 (uint8_t event);
uint8_t     _Cb_Entry_TemH (uint8_t event);
uint8_t     _Cb_Test_RS485 (uint8_t event);

//Function handler

uint8_t     AppTemH_Task(void);
void        AppTemH_Init (void);
void        AppTemH_Save_Thresh_Measure (void);

void        AppTemH_Log_Data_TSVH (StructManageRecordFlash *sRecordTSVH);
uint8_t     AppTemH_Packet_TSVH (uint8_t *pData);

void        AppTemH_485_Read_Value (uint8_t SlaveID, void (*pFuncResetRecvData) (void)) ;


#endif