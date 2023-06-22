



#ifndef USER_APP_WM_H_
#define USER_APP_WM_H_


#include "user_util.h"
#include "event_driven.h"

#include "user_adc.h"


#define USING_APP_TEMH

#define MAX_SLAVE               5             

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
#define NUM_SLAVE_DEFAULT       2

#define MAX_POINT_CALIB         14
#define DEFAULT_POINT_CALIB     11
#define REAL                    0
#define SAMP                    1 

#define UART_485	            huart1

#define MAX_GET_TEMH_FAIL       10                

#define TIME_READ_FREQUENCY     5000
#define TIME_RESEND_IVT         1000

#define TIME_SEND_OFF_IVT       10000

#define NUMBER_CONFIG           10
#define NUMBER_BUFFER_CONFIG    NUMBER_CONFIG*4 + 2


/*===========Andress Register KDE===============*/


/*================ var struct =================*/
typedef enum
{
    _EVENT_ENTRY_IVT,
    _EVENT_ONCE_HANDLE,
    _EVENT_WRITE_FLASH,
    _EVENT_CONFIG_IVT,
    _EVENT_WRITE_READ_IVT,
  
    _EVENT_TEMH_LOG_TSVH,

    _EVENT_END_TEMP_HUMI,
}eKindEventWm;

typedef enum
{
    _LED_STATUS,
    _LED_GPS,
    _LED_TEMH,
} Led_TypeDef;


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

typedef enum
{
    _ENUM_FREQUENCY_SETTING,
    _ENUM_START_STOP_SETTING,
    _ENUM_ACCELERATION_TIME,
    _ENUM_DECELERATION_TIME,
    
    _ENUM_FINISH_CONFIG,
}Enum_Config_IVT;

typedef struct
{
    uint8_t name;
    uint8_t status;
    uint16_t address_register;
    uint16_t infor_register;
}Struct_Config_IVT;

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
    int16_t     Val_i16;
    uint16_t    Scale_u8; 
    uint16_t    Unit_u16;
}structFloatValue;


typedef struct
{
    uint8_t             ModBusStatus_u8;
    
    uint8_t             Status_u8[MAX_SLAVE]; 
    uint8_t             aSlaveID[MAX_SLAVE];
    structFloatValue    sTemperature[MAX_SLAVE];
    structFloatValue    sHumidity[MAX_SLAVE]; 
    
    structFloatValue    sCurrent;
    uint8_t             NumSlave_u8;
    uint8_t             PowerStatus_u8;
    uint8_t             CountFailGetTemh_u8;
}STempHumiVariable;

typedef struct
{
    uint8_t             ModBusStatus_u8;
    
    uint8_t             Status_u8[MAX_SLAVE]; 
    uint8_t             aSlaveID[MAX_SLAVE];
    structFloatValue    sTemperature[MAX_SLAVE];
    structFloatValue    sHumidity[MAX_SLAVE]; 
    
    structFloatValue    sCurrent;
    uint8_t             NumSlave_u8;
    uint8_t             PowerStatus_u8;
    uint8_t             CountFailGetTemh_u8;
}RS485_Control_IVT;

typedef struct
{
	uint8_t		Status_Receive_u8;
	uint16_t	Pre_IVT;
    uint8_t     Scale_IVT;
}IVT_Receive;

typedef struct
{
    uint8_t     Status_Receive_u8;
    uint8_t    *aData;
}ADDR_485_Receive;


typedef struct
{
    uint8_t  ID_Iverter;
    uint8_t  Scale_Inverter;
    uint16_t Set_Freq_IVT;
    uint16_t Out_Freq_IVT;
}Param_Read_485_Inverter;

typedef struct
{
    uint16_t Addr_IVT;
    uint16_t Stop_IVT;
    uint16_t Run_IVT;
}Param_Run_Stop_485_Inverter;

typedef struct 
{
    uint16_t Addr_IVT;
    uint16_t Infor_IVT;
}Param_Check_Config_485_Inverter;

extern Struct_Battery_Status    sBattery;
extern Struct_Battery_Status    sVout;

extern struct_ThresholdConfig   sMeterThreshold;

extern uint8_t aMARK_ALARM[10];
extern uint8_t aMARK_ALARM_PENDING[10];

extern sEvent_struct sEventAppIVT[];

extern STempHumiVariable  sTempHumi;
extern sData   sUart485;
extern uint8_t aUART_485_DATA [64];

extern uint32_t ID_server_u32;
extern uint8_t connect_IVT_u8;
extern uint8_t check_irq;
extern uint8_t config_IVT_u8;
extern uint8_t UnLock_run_stop_485_IVT;
extern uint8_t run_stop_485_IVT;

extern IVT_Receive       sFre_IVT_Receive;
extern ADDR_485_Receive  sRead_485_IVT;
extern ADDR_485_Receive  sConfig_485_IVT;
extern ADDR_485_Receive  sRun_485_IVT;
/*================ Function =================*/

//Function handler
uint8_t     AppIVT_Task(void);
void        AppIVT_Init (void);

void        AppIVT_Init_Slave_ID (void);
void        AppIVT_Save_Slave_ID (void);

void        AppIVT_Init_Thresh_Measure (void);
void        AppIVT_Save_Thresh_Measure (void);

void        AppIVT_Log_Data_TSVH (void);
uint8_t     AppIVT_Packet_TSVH (uint8_t *pData);
void        Send_Command_IVT (uint8_t SlaveID, uint8_t Func_Code, uint16_t Addr_Register, uint16_t Infor_Register, void (*pFuncResetRecvData) (void)) ;

void        AppIVT_Clear_Before_Recv (void);

uint16_t    AppTemH_Get_VBAT_mV(void);
uint16_t    AppTemH_Get_Vout_mV(void);

void        LED_Toggle (Led_TypeDef Led);
void        LED_On (Led_TypeDef Led);
void        LED_Off (Led_TypeDef Led);


uint8_t     Check_CountBuffer_Complete_Uart(void);
uint8_t     Check_Complete_Command_IVT(uint8_t Addr_Slave, uint16_t Addr_Register, uint16_t Infor_Register); 
uint16_t    Calculator_Scale_IVT(IVT_Receive  Fre_IVT_Receive);

uint8_t     Task_ConfigIVT(uint8_t ID_Slave);
uint8_t     Setup_Addr_Register_Read_IVT(void);
uint8_t     Setup_Addr_Register_Write_IVT(void);
uint8_t     Setup_Addr_Register_Run_Stop_IVT(void);

uint32_t    Calculator_Scale(uint8_t Scale);
uint16_t    Read_Flash_U16(uint32_t address);
#endif

