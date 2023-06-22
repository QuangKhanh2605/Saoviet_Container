/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_MQTT_H
#define USER_MQTT_H

#include "user_util.h"
/*======================== Define ======================*/
#define MY_CONNECT             0x10
#define MY_CONNACK             0x20
#define MY_PUBLISH             0x32
#define MY_PUBACK              0x40
#define MY_SUBSCRIBE           0x82
#define MY_SUBACK              0x90
#define MY_PINGREQ             0xC0
#define MY_PINGRESP            0xD0
#define MY_DISCONNECT          0xE0
#define SUB_QOS                0x00
#define MY_QOS                 0x01
#define MY_CONNECTIONFLASG     0XC2
#define MY_MQTTVERSION         0x03

#define MY_ID1 					0x0A
#define MY_ID2 					0x00


#define HEADER_LENGTH 			50
#define MAX_LENGTH_MQTT  		1200

#define TOTAL_SLOT				30

#define NONE					0
#define SUCCESS					1

#define MS_FAIL 				0xE2
#define MS_CORRECT 				0xE1

#define TIME_DELAY_MQTT			60000
#define	TIME_RETRY				3
/*======================== Structs var======================*/
typedef enum {

    //Mess wait ACK
	TOPIC_NOTIF,

    DATA_HANDSHAKE,

	DATA_TSVH,
    DATA_TSVH_OPERA,
    DATA_INTAN_TSVH,
    
    DATA_STATUS,
    DATA_FLASH_MEM,
	DATA_PING,

    DATA_EVENT,
    DATA_GPS,

	SEND_SIM_ID,
    SEND_RESPOND_SERVER_ACK,
    SEND_SAVE_BOX_FAIL,
    SEND_SAVE_BOX_OK,
    //Mess no wait ACK
    SEND_RESPOND_SERVER,
    SEND_RESPOND_FROM_RF,
	SEND_SHUTTING_DOWN,
    SEND_RECONNECT_SERVER,
    SEND_HARDTS_MCU,
    SEND_ALARM_EMERGENCY,

	SEND_SERVER_TIME_PENDING,   //18
	SEND_SERVER_TIME_OK,
	SEND_SERVER_TIME_FAIL,
    SEND_EMPTY_MESS,

    SEND_UPDATE_FIRM_OK,
    SEND_UPDATE_FIRM_FAIL,

	END_MQTT_SEND,
} Type_Message_MQTT_Send;

typedef enum {
	REQUEST_RESET,
    INTAN_DATA,
	REQUEST_SETTING,
	UPDATE_TIME_SERVER,
    RESET_DATA,
    REQUEST_AT,
    REQUEST_RF,
    
    REQUEST_UPDATE_FIRM,

	END_MQTT_RECEI,
} Type_Message_MQTT_Receiv;
//

typedef uint8_t (*task_callback_Send)(int Kind_Send);
typedef struct {
	int 				idMark;
	task_callback_Send 	CallBack;
	sData		sKind;
} struct_MARK_TYPE_Message_SEND;


//
typedef void (*task_callback_r)(sData *str_Receiv, int16_t Pos);
typedef struct {
	int 			idMark;
	task_callback_r	CallBack;
	sData	        sKind;
} struct_MARK_TYPE_Message_Receiv;

typedef struct {
	int 	idMark;
	sData	sKind;					// Send -> Sim900
} MARK_TYPE_DCU;



typedef struct {
	int				aMARK_MESS_PENDING[TOTAL_SLOT];	// Danh dau ban tin can SEND di (0-2 : ban tin quan trong; 3-12 : ban tin data; 13 - log -> clear khi reset)
// Cal Data
	uint32_t		LandmarkSendPing_u32;
	sData 	        str;
	sData 	        strHexData;
// 	publish
    sData 	        sPublishHeader;
	int 			PulishQos;
	int				SubcribeQos;
	unsigned char	PulishRetained;
	unsigned short	PulishPacketId;
//  Subcribe
	sData 	        sSubcribe;
	int 			MsgId;
	int				ReqQos;
	uint8_t 		KeepAliveTime_u8;
    //
	sData 	        sPayload;
	uint32_t		LandMarkLastSend_u32;   //Thoi gian cuoi cung gui len server. De check trong truong hop Danh thuc bang PIN
    uint8_t         MessType_u8;            //Danh dau ban tin dang chuan bi gui di
    uint8_t         Status_u8;              //Flag dang xu ly Send:   2: Pending, 1: Success 0: False
    uint8_t         TypeConfigServer_u8;    //Type Config Server: Wait ACK:  1: Config FreqSend;  3: ConfigSaveBox.
} struct_MQTT;


/*======================== External Var struct ======================*/

extern const MARK_TYPE_DCU sMark_DCU_Type[];
extern const struct_MARK_TYPE_Message_SEND sMark_MessageSend_Type[];
extern const struct_MARK_TYPE_Message_Receiv sMark_MessageRecei_Type[];

extern struct_MQTT					sMQTT;

/*======================== Function ======================*/

/*---------------Function Callback send-----------------------*/
uint8_t     _mDATA_HANDSHAKE(int Kind_Send);

uint8_t 	_mDATA_TSVH_PACKET(int Kind_Send);
uint8_t     _mDATA_TSVH_PACKET_OPERA (int Kind_Send);
uint8_t 	_mDATA_ON_FLASH(int Kind_Send);
uint8_t     _mDATA_INTAN_TSVH(int Kind_Send);
uint8_t 	_mSEND_SHUTTING_DOWN(int Kind_Send);
uint8_t 	_mDATA_PING(int Kind_Send);
void 		_mInsert_Ping_Payload (sData *Payload);

uint8_t 	_mSEND_SIM_ID(int Kind_Send);
void 		_mPayload_Sim_ID(sData *Payload);

uint8_t     _mDATA_EVENT(int Kind_Send);
uint8_t     _mDATA_GPS(int Kind_Send);

uint8_t		_mSEND_RESPOND_SERVER(int Kind_Send);
uint8_t     _mSEND_RESPOND_FROM_RF(int Kind_Send);
uint8_t		_mSEND_RESPOND_SERVER_ACK(int Kind_Send);

uint8_t		_mSEND_SERVER_TIME_PENDING(int Kind_Send);
uint8_t		_mSEND_SERVER_TIME_OK(int Kind_Send);
uint8_t		_mSEND_SERVER_TIME_FAIL(int Kind_Send);
uint8_t		_mSEND_SAVE_BOX_FAIL(int Kind_Send);
uint8_t     _mSEND_SAVE_BOK_OK(int Kind_Send);
uint8_t		_mSEND_EMPTY_MESS(int Kind_Send);

uint8_t     _mSEND_UPDATE_FIRM_OK(int Kind_Send);
uint8_t     _mSEND_UPDATE_FIRM_FAIL(int Kind_Send);
uint8_t     _mSEND_RECONNECT_SERVER(int Kind_Send);

//handle Send
void        mSend_Packet_MQTT(struct_MQTT *mqtt);

void 		mSet_default_MQTT(void);
uint8_t 	mWrite_publish_header(sData *sTarget,int Step_Data, uint8_t Kind);
void 		mPayload_Update_Add(uint8_t *Add_array, uint8_t Code, uint8_t Err);
void 		mPayload_Load_MesErr(sData *Payload);

/*---------------Function Calback receive-----------------------*/
void 		_rREQUEST_RESET(sData *str_Receiv,int16_t Pos);
void 		_rINTAN_DATA (sData *str_Receiv,int16_t Pos);
__weak void _rREQUEST_SETTING(sData *str_Receiv,int16_t Pos);
void 		_rUPDATE_TIME_SERVER(sData *str_Receiv,int16_t Pos);
void 		_rRESET_DATA(sData *str_Receiv,int16_t Pos);

void 		_rClear_Data_InFlash (void);
void        _rREQ_AT_CMD(sData *str_Receiv,int16_t Pos);
void        _rREQ_AT_TO_RF(sData *str_Receiv,int16_t Pos);
void        _rUPDATE_FIRMWARE(sData *str_Receiv,int16_t Pos);

/*---------------Function handle-----------------------*/
void 		mInit_Header_MQTT(void);
void     	mConnect_MQTT (void);
void     	mSubcribe_MQTT (void);
void        mPublish_MQTT (void);

void        mData_MQTT (uint8_t MessType, uint8_t Qos);
uint8_t     mCheck_Empty_Mess (void);
void        mRespond_Config_Server (uint8_t portNo, uint8_t *data, uint16_t length);

uint8_t	    mUpdate_Split_FTP(sData *str_Receiv);
uint8_t     _mSEND_HARDRS_MCU(int Kind_Send);
uint8_t     _mSEND_ALARM_EMERGENCY(int Kind_Send);

uint8_t     _mDATA_STATUS(int Kind_Send);


#endif
