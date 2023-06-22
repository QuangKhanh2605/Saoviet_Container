


#ifndef USER_SIM_COMMON_H_
#define USER_SIM_COMMON_H_

#include "user_util.h"
#include "main.h"
#include "onchipflash.h"

/*=========== Define =====================*/
#define SIM_ID_LENGTH			21

#define MAX_IP_LENGTH		    40
#define MAX_PORT_LENGTH		    10
#define MAX_USER_PASS_LENGTH	40

#define BUFF_LENGTH_SETUP	    30
#define LENGTH_FTP				30
#define LENGTH_FIRM_NAME        13

#define LENGTH_HTTP_URL         80

#define LENGTH_HEADER_FW        12
#define MAX_LENGTH_GPS          100

/*===== Char MQTT =======*/
#define MY_CONNACK              0x20
#define MY_PUBACK               0x40
#define MY_SUBACK               0x90

/*=========== Struct =====================*/

typedef uint8_t (*CB_AT_Success) (sData *str_Receive);
typedef uint8_t (*CB_AT_Failure) (uint8_t Type);
typedef struct
{
	uint8_t at_name;
	CB_AT_Success	callback_success;
	CB_AT_Failure	callback_failure;
    char 	*at_response;
    char	*at_string;
} sCommand_Sim_Struct; // it is not a declaration, it is a new type of data



typedef struct
{
	sData  	sIPMain;
	sData  	sPortMain;
	sData  	sUserNameMQTTMain;
	sData  	sPasswordMQTTMain;
    
    sData  	sIPBackUp;
	sData  	sPortBackUp;
	sData  	sUserNameMQTTBackUp;
	sData  	sPasswordMQTTBackUp;
    
	uint32_t		KeepAlive_u32;

}Struct_Server_Infor;



typedef struct
{
	sData		sSIMid;      //String Sim ID
	uint8_t		RSSI_u8;     //RSSI module SIM
	uint8_t		Ber_u8;
    uint8_t		RSSILevel_u8;  
    sData       sImei;
    
    sData       sAPN;
    sData	    sAPNDialString;
	sData	    sAPNUsername;
	sData	    sAPNPassword;
    
    //Server Infor
    Struct_Server_Infor   sServer;

    ST_TIME_FORMAT  sTime; 
} Struct_Sim_Information;


typedef struct
{
    uint8_t     Status_u8;          //Get Pos OK: 1   | Fail 0
    uint8_t     aPOS_INFOR[MAX_LENGTH_GPS];    //string location
    uint8_t     LengData_u8;
}SGPSPositionInformation;


typedef struct
{
	uint8_t 	CallReady_u8;		        // Bao có card sim
	uint8_t		GroupStepID_u8;		        // Nhóm step: Pre Init | Check Attach | Setting |Open TCP...
    uint8_t     DataMode_u8;  
    
    uint8_t     PowerStatus_u8;
    
    //status sim
    uint8_t     RxPinReady_u8;          // Chan Rx OK: Khi nhan dc ki tu tu uart rx
    uint8_t     TxPinReady_u8;          // Truyen 1 lenh vao co phan hoi lai -> Tx Pin OK
    uint8_t     NetReady_u8;            // CGATT = 1
    uint8_t     ServerReady_u8;         // Connect Server
    
    SGPSPositionInformation sGPS;
}Struct_Sim_Variable;



typedef struct
{
    sData	    StrURL_FirmIP;
    sData	    StrURL_FirmPort;
    sData	    StrURL_FirmPath;
    sData	    StrURL_FirmName;
    sData	    StrURL_HTTP;
    
    uint32_t	FirmLength_u32;
	sData		strFirmLength;			        // string Length receive from SIM module

	uint8_t		DataTemp_u8;				     // Bo dem Uart FTP
    
	uint32_t	AddSave_u32;                    //Add Save Fw
    uint16_t    MaxPage_u16;                    //Max Page
        
	uint8_t		Pending_u8;				        // Doi tinh toan uart FTP
	uint32_t	CountByteTotal_u32;
    //
    uint32_t    AddOffset_u32;
    uint16_t    CountPacket_u8;
    uint8_t     LastCrcFile_u8;
    uint8_t     LasCrcCal_u8;
    uint8_t     IsFirmSaoViet_u8;
    
    uint8_t     UpdateFailStatus_u8;
}StructSimUpdateFirmware;


typedef enum
{
	_GR_PREE_INIT,
	_GR_CHECK_ATTACH,
	_GR_SETTING,
	_GR_INIT_TCP,
	_GR_SEND_MQTT,
	_GR_CLOSE_TCP,
    _GR_READ_HTTP,
    
}Group_Step_Control;


typedef enum
{
    _SIM_COMM_EVENT_AT_FAILURE,
    _SIM_COMM_EVENT_GET_CONN, 
    _SIM_COMM_EVENT_HTTP_READ_OK,
    
	_SIM_COMM_EVENT_SIM_CARD,
    _SIM_COMM_EVENT_GET_STIME, 
    _SIM_COMM_EVENT_GPS_OK, 
    _SIM_COMM_EVENT_GPS_ERROR,
    
    _SIM_COMM_EVENT_TCP_SEND_1,
    _SIM_COMM_EVENT_TCP_SEND_2,
    
    _SIM_COMM_EVENT_CONN_MQTT_1,
    _SIM_COMM_EVENT_CONN_MQTT_2,
    
    _SIM_COMM_EVENT_SUB_MQTT_1,
    _SIM_COMM_EVENT_SUB_MQTT_2,
    
    _SIM_COMM_EVENT_PUB_MQTT_1,
    _SIM_COMM_EVENT_PUB_MQTT_2,
    
    _SIM_COMM_EVENT_HTTP_UPDATE_OK,
    _SIM_COMM_EVENT_HTTP_UPDATE_FAIL,
    
}sKindHandlerSIM;

typedef enum
{
	_SIM_COMM_URC_RESET_SIM = 0,
	_SIM_COMM_URC_SIM_LOST,
	_SIM_COMM_URC_SIM_REMOVE,
	_SIM_COMM_URC_CLOSED,
	_SIM_COMM_URC_PDP_DEACT,
	_SIM_COMM_URC_CALL_READY,
    _SIM_COMM_URC_CALLING,
    _SIM_COMM_URC_SMS_CONTENT,
    _SIM_COMM_URC_SMS_IND,
	_SIM_COMM_URC_ERROR,
    _SIM_COMM_URC_RECEIV_SERVER,
	_SIM_COMM_URC_END,
}KindSimCommonURC;


typedef enum
{
    MODE_CONNECT_DATA_MAIN = 1,		
    MODE_CONNECT_DATA_BACKUP,		
    MODE_CONNECT_FTP,		       
    MODE_CONNECT_HTTP ,
    
}sKindSimConnect;

typedef enum 
{
    // Don't need feedback
	SEND_UPDATE_FIRMWARE,
	SEND_UPDATE_FIRMWARE_PENDING,
	SEND_UPDATE_FIRMWARE_OK,
	SEND_UPDATE_FIRMWARE_FAIL_SETUP,
	SEND_UPDATE_FIRMWARE_FAIL_TIME,
	SEND_UPDATE_FIRMWARE_FAIL_FLASH,
	SEND_UPDATE_FIRMWARE_FAIL_LENGTH,
	SEND_ERASE_FLASH_SUCCESS,
	SEND_ERASE_FLASH_FAIL,
}Struct_Step_UpdateFirm;
    

typedef struct
{
    uint8_t     aNAME[50];
    uint8_t     LengthName_u8;
    
    uint8_t     aTEMP_STR_HANDLE[5];
    sData       strHandle;
    
    uint32_t    Handle_u32;        
}SFileSystemInfor;

typedef enum
{
    _POWER_START= 0,
    _POWER_INIT,
    _POWER_CONN_MQTT,		
    _POWER_PSM,		
    _POWER_POWER_OFF,	
}sKindSimPower;



typedef struct
{
    uint8_t     ( *pSim_Common_Handler_AT ) (uint8_t Type);
    void        ( *pSim_Common_Handler_URC ) (uint8_t Type);

    void        ( *pSim_Common_Config_Update ) (void);
    sData*      ( *pSim_Common_Get_Fw_Version ) (void);
}sFuncCallbackHandlerSim;

/*===================== extern ====================*/
extern sFuncCallbackHandlerSim *sSimCommFuncCallBack;


extern Struct_Sim_Information	sSimInfor;
extern Struct_Sim_Variable		sSimCommon;
extern StructSimUpdateFirmware  sSimFwUpdate;
extern SFileSystemInfor         sFileSys;

extern uint8_t aIP_SERVER_MAIN[MAX_IP_LENGTH];
extern uint8_t aPORT_SERVER_MAIN[MAX_PORT_LENGTH];
extern uint8_t aUSER_MQTT_MAIN[MAX_USER_PASS_LENGTH];
extern uint8_t aPASS_MQTT_MAIN[MAX_USER_PASS_LENGTH];

extern uint8_t aIP_SERVER_BACKUP[MAX_IP_LENGTH];
extern uint8_t aPORT_SERVER_BACKUP[MAX_PORT_LENGTH];
extern uint8_t aUSER_MQTT_BACKUP[MAX_USER_PASS_LENGTH];
extern uint8_t aPASS_MQTT_BACKUP[MAX_USER_PASS_LENGTH];

/*================= Struct =================*/

void    Sim_Common_Get_Sim_ID (sData *uart_string);
void    Sim_Common_Get_CSQ (sData *strCheck, sData *strSoure);
void    Sim_Common_Get_Imei(sData *strSoure);
void    Sim_Common_Get_Stime_BTS (sData *strCheck, sData *strSoure);

//Http
uint8_t Sim_Common_Http_Read_Data (sData *uart_string);
//Call func URC
void    Sim_Common_URC_Reset (void);
void    Sim_Common_URC_Lost_Card (void);
void    Sim_Common_URC_Closed (void);
void    Sim_Common_URC_Error (void);
void    Sim_Common_URC_Recv_Server (void);
void    Sim_Common_URC_Calling (void);
void    Sim_Common_URC_Sms_Content (void);
void    Sim_Common_URC_Sms_Indication (void);
//
void    Sim_Common_Config_UpdateFw (uint32_t AddNewFw, uint16_t MAX_SIZE);
void    Sim_Common_Init_UpdateFw (void);
void    Sim_Common_Init_Struct_UpdateFw (void);
//
void    Sim_Common_Init_Var_Default (void);
void    Sim_Common_Send_AT_Cmd (UART_HandleTypeDef *huart, uint8_t *string, uint16_t length, uint32_t TimeOut); 
void    Sim_Common_Init_Struct_FileSys (void);

void    Sim_Common_Default_Struct_GPS (void);


#endif
