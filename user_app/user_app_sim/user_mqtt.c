/*
    8/2021
    Thu vien xu ly Uart
*/
#include "MQTTConnect.h"
#include "MQTTFormat.h"
#include "MQTTPacket.h"
#include "MQTTPublish.h"
#include "MQTTSubscribe.h"
#include "MQTTUnsubscribe.h"
#include "StackTrace.h"

#include "user_define.h"

#include "user_mqtt.h"
#include "user_sim.h"

#include "user_string.h"
#include "cat24mxx.h"
#include "user_modem.h"


/*======================== Structs var======================*/
const struct_MARK_TYPE_Message_SEND     sMark_MessageSend_Type[] =
{
    //Mess wait ACK
    {TOPIC_NOTIF,						NULL,								{(uint8_t*)"Notif",5}},

    {DATA_HANDSHAKE,					_mDATA_HANDSHAKE,				    {(uint8_t*)"MePDV",5}}, //

    {DATA_TSVH,				            _mDATA_TSVH_PACKET,			        {(uint8_t*)"Flash",5}}, //
    {DATA_TSVH_OPERA,				    _mDATA_TSVH_PACKET_OPERA,			{(uint8_t*)"Opera",5}}, //
    {DATA_INTAN_TSVH,				    _mDATA_INTAN_TSVH,				    {(uint8_t*)"Intan",5}}, //

    {DATA_STATUS,				        _mDATA_STATUS,				        {(uint8_t*)"Statu",5}}, //
    {DATA_FLASH_MEM,					_mDATA_ON_FLASH,				    {(uint8_t*)"Debug",5}}, //
    {DATA_PING,							_mDATA_PING,						{(uint8_t*)"mPing",5}}, //

    {DATA_EVENT,					    _mDATA_EVENT,				        {(uint8_t*)"Event",5}}, //
    {DATA_GPS,					        _mDATA_GPS,				            {(uint8_t*)"Gpsda",5}}, //
    
    {SEND_SIM_ID,					    _mSEND_SIM_ID,					    {(uint8_t*)"SimID",5}},
    {SEND_RESPOND_SERVER_ACK,	    	_mSEND_RESPOND_SERVER_ACK,			{(uint8_t*)"sCoFi",5}},
    {SEND_SAVE_BOX_FAIL,				_mSEND_SAVE_BOX_FAIL,				{(uint8_t*)"sCoFi",5}},
    {SEND_SAVE_BOX_OK,				    _mSEND_SAVE_BOK_OK,				    {(uint8_t*)"sCoFi",5}},
    //Mess No wait ACK
    {SEND_RESPOND_SERVER,				_mSEND_RESPOND_SERVER,				{(uint8_t*)"sCoFi",5}},
    {SEND_RESPOND_FROM_RF,				_mSEND_RESPOND_FROM_RF,				{(uint8_t*)"ReqRF",5}},
    {SEND_SHUTTING_DOWN,				_mSEND_SHUTTING_DOWN,				{(uint8_t*)"Reset",5}},
    {SEND_RECONNECT_SERVER,				_mSEND_RECONNECT_SERVER,			{(uint8_t*)"ReCon",5}},
    {SEND_HARDTS_MCU,				    _mSEND_HARDRS_MCU,			        {(uint8_t*)"Alarm",5}},
    {SEND_ALARM_EMERGENCY,				_mSEND_ALARM_EMERGENCY,				{(uint8_t*)"Alarm",5}},
    //
    {SEND_SERVER_TIME_PENDING,			_mSEND_SERVER_TIME_PENDING,			{(uint8_t*)"sTime",5}},    // {(uint8_t*)"Realtime pending",16}},
    {SEND_SERVER_TIME_OK,				_mSEND_SERVER_TIME_OK,				{(uint8_t*)"Notif",5}},    // {(uint8_t*)"Realtime OK",11}},
    {SEND_SERVER_TIME_FAIL,				_mSEND_SERVER_TIME_FAIL,			{(uint8_t*)"Notif",5}},    // {(uint8_t*)"Realtime fail",13}},
    {SEND_EMPTY_MESS,				    _mSEND_EMPTY_MESS,				    {(uint8_t*)"Notif",5}},    // {(uint8_t*)"Empty Mess FLash!",17}},

    {SEND_UPDATE_FIRM_OK,				_mSEND_UPDATE_FIRM_OK,				{(uint8_t*)"Notif",5}},    // {(uint8_t*)"Realtime OK",11}},
    {SEND_UPDATE_FIRM_FAIL,				_mSEND_UPDATE_FIRM_FAIL,			{(uint8_t*)"Notif",5}},    // {(uint8_t*)"Realtime fail",13}}

    {END_MQTT_SEND,						NULL,								{NULL}},
};

/* */
const struct_MARK_TYPE_Message_Receiv sMark_MessageRecei_Type[] =
{
    // ban tin danh rieng cho SIM
    {REQUEST_RESET,				_rREQUEST_RESET,				{(uint8_t*)"Reset",5}},
    {INTAN_DATA,                _rINTAN_DATA,                   {(uint8_t*)"Intan",5}},
    {REQUEST_SETTING,			_rREQUEST_SETTING,				{(uint8_t*)"sCoFi",5}},
    {UPDATE_TIME_SERVER,		_rUPDATE_TIME_SERVER,			{(uint8_t*)"sTime",5}},
    {RESET_DATA,				_rRESET_DATA,	    			{(uint8_t*)"Clear",5}},
    {REQUEST_AT,				_rREQ_AT_CMD,	    			{(uint8_t*)"ReqAT",5}},
    {REQUEST_RF,				_rREQ_AT_TO_RF,	    			{(uint8_t*)"ReqRF",5}},
    
    {REQUEST_UPDATE_FIRM,	    _rUPDATE_FIRMWARE,		        {(uint8_t*)"rFirm",5}},

    {END_MQTT_RECEI,			NULL,							{NULL}},
};



const MARK_TYPE_DCU sMark_DCU_Type[] =
{
	{ _DHN_NBIOT_1,                     {(uint8_t*)"DCU/WMSV/WME1/",14}}, // 15 - fix 26_03_18  DCU/EMSV/SVE1/
	{ _WM_GSM_HOABINH,	                {(uint8_t*)"DCU/WMSV/WMP2/",14}},
	{ _METER_COSPHI,		            {(uint8_t*)"DCU/EMET/COSP/",14}},
	{ _EMET_NB_IOT,		           	    {(uint8_t*)"DCU/EMSV/SVE1/",14}},
    { _WM_GSM_LEVEL,		           	{(uint8_t*)"DCU/WMSV/WML1/",14}},
    { _WM_GSM_LORA,		           	    {(uint8_t*)"DCU/WMSV/WMLG/",14}},
    { _TEM_HUMI_GSM,		           	{(uint8_t*)"DCU/THSV/THGS/",14}},
    { _WM_GSM_CONV,		           	    {(uint8_t*)"DCU/WMSV/WMPI/",14}},
    { _CONTROL_INVERTER,		        {(uint8_t*)"DCU/WMSV/WMCI/",14}},
};

struct_MQTT				sMQTT;
MQTTPacket_connectData 	Connect_Packet = MQTTPacket_connectData_initializer;
MQTTString 				topicString = MQTTString_initializer;


uint8_t	aSubcribe_Header[HEADER_LENGTH];
uint8_t	aPublish_Header[HEADER_LENGTH];		//Header

uint8_t	aDATA_MQTT[MAX_LENGTH_MQTT];        //Buffer chưa data da duoc dong goi MQTT: Bao gom Payload + Header MQTT
uint8_t aPAYLOAD_MQTT[MAX_LENGTH_MQTT];		//Buffer chưa data chua duoc dong goi MQTT: Bao gom Payload

/*======================== Function ======================*/

void mSet_default_MQTT(void)
{
	sMQTT.str.Data_a8 		= aDATA_MQTT;
	sMQTT.str.Length_u16 	= 0;

    sMQTT.Status_u8         = FALSE;
}



/*
 * 		FUNC : ghi ra chuoi Header Publish message day du
 * Param :
 * 		- sData *sTarget : Chuoi su dung *
 * 		- Step_Data				: Dang Header
 * 		- Kind 					: 0 Type Message in table  sMark_MessageSend_Type
 * 								: 1 Type Message in table  sMark_MessageRecei_Type
 * Return :
 * 		0 - FALSE 	: 	ERROR :tran chuoi copy vao,
 * 		1 - True 	:	Tao Thanh Cong
 */

uint8_t Write_publish_header(sData *sTarget, int Step_Data, uint8_t Kind)
{
    //Set up Header Pub again
    UTIL_MEM_set(aPublish_Header, 0, sizeof (aPublish_Header));

    sTarget->Data_a8 = &aPublish_Header[0];
    sTarget->Length_u16 = 0;

    Copy_String(sTarget, (sData *)&sMark_DCU_Type[sModem.TypeModem_u8].sKind, 255, HEADER_LENGTH);

    if (Kind == 1)
		Copy_String(sTarget, (sData *)&sMark_MessageRecei_Type[Step_Data].sKind, 255, HEADER_LENGTH);
	else
        Copy_String(sTarget, (sData *)&sMark_MessageSend_Type[Step_Data].sKind, 255, HEADER_LENGTH);

	*(sTarget->Data_a8 + sTarget->Length_u16++) = '/';

	Copy_String(sTarget, &sModem.sDCU_id, 255, HEADER_LENGTH);

	return 1;
}


/*
 * Ham gui ban tin MQTT.
 * return 	TRUE : neu gui thanh cong va nhan lai FEEDBACK
 * 			FALSE : Neu gui > MAX_Resend ma ko co FB lai tu AMI
 * 	neu Max_Resend = 0xFF -> Ham luon tra ve TRUE */



void mSend_Packet_MQTT(struct_MQTT *mqtt)
{
    //Dong goi Ban tin connect
#ifdef USING_TRANSPARENT_MODE
    UTIL_Printf_Str( DBLEVEL_M, "\r\n Message transfer: \r\n" );
    Sim_Common_Send_AT_Cmd(&uart_sim, mqtt->str.Data_a8, mqtt->str.Length_u16, 1000);
#endif
}


void mData_MQTT (uint8_t MessType, uint8_t Qos)
{
    mSet_default_MQTT();        //dua con tro MQTT.str.data_u8 tro dau mang aDATA_MQTT

    sMQTT.PulishQos = Qos;   // MY_QOS;
	sMQTT.PulishRetained = 0;
	sMQTT.PulishPacketId = 1;

    Write_publish_header(&sMQTT.sPublishHeader, MessType, 0);
	topicString.cstring = (char*) sMQTT.sPublishHeader.Data_a8;

    sMQTT.str.Length_u16 = MQTTSerialize_publish(sMQTT.str.Data_a8, sizeof(aDATA_MQTT), 0, sMQTT.PulishQos, \
            sMQTT.PulishRetained, sMQTT.PulishPacketId, topicString, sMQTT.sPayload.Data_a8, sMQTT.sPayload.Length_u16);
}

/*============= Func Callback ============================*/

uint8_t _mDATA_HANDSHAKE(int Kind_Send)
{
    uint8_t	    i = 0;
    uint8_t     var = 0;
    uint8_t     TempCrc = 0;
    uint32_t    FreqSend = 0;

    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
    sMQTT.sPayload.Length_u16 = 0;

    //Thoi gian thu thap
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x01;   // sTime
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x06;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sRTC.year;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sRTC.month;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sRTC.date;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sRTC.hour;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sRTC.min;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sRTC.sec;

    //Cuong do song
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x06;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x01;

    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sSimInfor.RSSI_u8;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x00;

    //Thoi gian thu thap
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x08;
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x02;

    if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
    {
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = (sFreqInfor.FreqSendUnitMin_u32 >> 8) & 0xFF;
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sFreqInfor.FreqSendUnitMin_u32 & 0xFF;
    } else
    {
        FreqSend = sFreqInfor.NumWakeup_u8 * sFreqInfor.FreqWakeup_u32;
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = (FreqSend >> 8) & 0xFF;
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = FreqSend & 0xFF;
    }
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x00;

    //Seri SIM
	*(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x0B; // obit
	*(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sSimInfor.sSIMid.Length_u16; // length

	for (var = 0; var < sSimInfor.sSIMid.Length_u16; ++var)
		*(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++)  = sSimInfor.sSIMid.Data_a8[var];

    //Dia chi server dang connect
    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = 0x0C; // obit
    if (sSimVar.ModeConnectNow_u8 == MODE_CONNECT_DATA_MAIN)
    {
        //IP
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sSimInfor.sServer.sIPMain.Length_u16 + sSimInfor.sServer.sPortMain.Length_u16 + 1; // length
        for (var = 0; var < sSimInfor.sServer.sIPMain.Length_u16; ++var)
            *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++)  = sSimInfor.sServer.sIPMain.Data_a8[var];
        //Port
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = ':'; // length
        for (var = 0; var < sSimInfor.sServer.sPortMain.Length_u16; ++var)
            *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++)  = sSimInfor.sServer.sPortMain.Data_a8[var];
    } else
    {
        //IP
         *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = sSimInfor.sServer.sIPBackUp.Length_u16 + sSimInfor.sServer.sPortBackUp.Length_u16 + 1;
        for (var = 0; var < sSimInfor.sServer.sIPBackUp.Length_u16; ++var)
            *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++)  = sSimInfor.sServer.sIPBackUp.Data_a8[var];
        //Port
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = ':';
        for (var = 0; var < sSimInfor.sServer.sPortBackUp.Length_u16; ++var)
            *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++)  = sSimInfor.sServer.sPortBackUp.Data_a8[var];
    }

    // caculator crc
    sMQTT.sPayload.Length_u16++;
	for (i = 0; i < (sMQTT.sPayload.Length_u16 - 1); i++)
		TempCrc ^= *(sMQTT.sPayload.Data_a8 + i);

    *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16 - 1) = TempCrc;

    //Send MQTT
    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}


uint8_t _mDATA_TSVH_PACKET(int Kind_Send)
{
    //Get Data Payload
    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
    sMQTT.sPayload.Length_u16 = 0;

    for (uint16_t i = 0; i < sAppSimVar.sDataFlashSim.Length_u16; i++)
    {
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(sAppSimVar.sDataFlashSim.Data_a8 + i);
    }
    
    mData_MQTT(Kind_Send, MY_QOS);
    
    return 1;
}



uint8_t _mDATA_TSVH_PACKET_OPERA (int Kind_Send)
{   
    //Get Data Payload
    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
    sMQTT.sPayload.Length_u16 = 0;

    for (uint16_t i = 0; i < sAppSimVar.sDataFlashSim.Length_u16; i++)
    {
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(sAppSimVar.sDataFlashSim.Data_a8 + i);
    }
    
    mData_MQTT(Kind_Send, MY_QOS);
    
    return 1;
}


uint8_t _mDATA_INTAN_TSVH(int Kind_Send)
{
    //Ðoc record tu Flash ra-> Dong goi du lieu vao sMQTT.sPayload
    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
    sMQTT.sPayload.Length_u16 = 0;

#if defined(USING_APP_WM) || defined (USING_APP_EMET)
    sMQTT.sPayload.Length_u16 = AppWm_Packet_TSVH (sMQTT.sPayload.Data_a8);
#endif
   
#if defined(USING_APP_TEMH) 
    sMQTT.sPayload.Length_u16 = AppIVT_Packet_TSVH (sMQTT.sPayload.Data_a8);
#endif
    
#if defined(USING_APP_LORA)  
    for (uint16_t i = 0; i < sLoraVar.sIntanData.Length_u16; i++)
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(sLoraVar.sIntanData.Data_a8 + i); 
#endif

    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}


uint8_t _mDATA_ON_FLASH(int Kind_Send)
{
    
    
    return 1;
}



uint8_t _mSEND_SHUTTING_DOWN(int Kind_Send)
{
    sData sPayLoadShutdown = {(uint8_t*)"MCU shutting down", 17};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, SEND_SHUTTING_DOWN);
	mPayload_Load_MesErr(&sPayLoadShutdown);

    mData_MQTT(Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_RECONNECT_SERVER(int Kind_Send)
{
    sData sPayLoadReConn = {(uint8_t*)"MQTT ReConnect!", 15};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, SEND_SHUTTING_DOWN);
	mPayload_Load_MesErr(&sPayLoadReConn);

    mData_MQTT(Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_HARDRS_MCU(int Kind_Send)
{
    sData sPayLoadReConn = {(uint8_t*)"Alarm: Hard Reset MCU!", 23};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, SEND_SHUTTING_DOWN);
	mPayload_Load_MesErr(&sPayLoadReConn);

    mData_MQTT(Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_ALARM_EMERGENCY(int Kind_Send)
{
    uint16_t i = 0;

    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
	sMQTT.sPayload.Length_u16 = MAX_LENGTH_MQTT;

	Reset_Buff(&sMQTT.sPayload);

    for (i = 0; i < sModem.strAlarmEmer.Length_u16; i++)
      *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(sModem.strAlarmEmer.Data_a8 + i);

    Reset_Buff(&sModem.strAlarmEmer);
    
    mData_MQTT(Kind_Send, 0);
        
    return 1;
}

/*
 * 		PUBLISH MESSAGE
 */
uint8_t _mDATA_PING(int Kind_Send)
{
	_mInsert_Ping_Payload(&sMQTT.sPayload);

    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}


uint8_t _mDATA_STATUS(int Kind_Send)
{   
    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
	sMQTT.sPayload.Length_u16 = MAX_LENGTH_MQTT;
    
    Reset_Buff(&sMQTT.sPayload);
       
    return 0;
}



uint8_t _mDATA_EVENT(int Kind_Send)
{        
    //Get Data Payload
    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
    sMQTT.sPayload.Length_u16 = 0;

    for (uint16_t i = 0; i < sAppSimVar.sDataFlashSim.Length_u16; i++)
    {
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(sAppSimVar.sDataFlashSim.Data_a8 + i);
    }
    
    mData_MQTT(Kind_Send, MY_QOS);
    
    return 1;
}


uint8_t _mDATA_GPS(int Kind_Send)
{        
    //Get Data Payload
    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
    sMQTT.sPayload.Length_u16 = 0;

    for (uint16_t i = 0; i < sAppSimVar.sDataFlashSim.Length_u16; i++)
    {
        *(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(sAppSimVar.sDataFlashSim.Data_a8 + i);
    }
    
    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}






void _mInsert_Ping_Payload (sData *Payload)
{
	uint8_t	    i = 0;
	uint8_t     var = 0;
	uint8_t     TempCrc = 0;

	Payload->Data_a8 		= aPAYLOAD_MQTT;
	Payload->Length_u16 	= MAX_LENGTH_MQTT;

	Reset_Buff(Payload);
    *(Payload->Data_a8 + Payload->Length_u16++)  = '#';
	// SIM_ID
	for (var = 0; var < sSimInfor.sSIMid.Length_u16; ++var)
		*(Payload->Data_a8 + Payload->Length_u16++)  = sSimInfor.sSIMid.Data_a8[var];
    //Insert Version
    *(Payload->Data_a8 + Payload->Length_u16++)  = '#';
    for (var = 0; var < sFirmVersion.Length_u16; ++var)
		*(Payload->Data_a8 + Payload->Length_u16++)  = sFirmVersion.Data_a8[var];

    *(Payload->Data_a8 + Payload->Length_u16++)  = '#';

    //Test them GPS
    if (sSimCommon.sGPS.Status_u8 == true)
    {
        for (var = 0; var < sSimCommon.sGPS.LengData_u8; ++var)
            *(Payload->Data_a8 + Payload->Length_u16++)  = sSimCommon.sGPS.aPOS_INFOR[var];
        
        *(Payload->Data_a8 + Payload->Length_u16++)  = '#';
    }
#ifdef USING_APP_SIM
    *(Payload->Data_a8 + Payload->Length_u16++)  = 'C';
    *(Payload->Data_a8 + Payload->Length_u16++)  = 'S';
    *(Payload->Data_a8 + Payload->Length_u16++)  = 'Q';
    *(Payload->Data_a8 + Payload->Length_u16++)  = '-';
    
    Convert_Uint64_To_StringDec(Payload, sSimInfor.RSSI_u8, 0);
    
    *(Payload->Data_a8 + Payload->Length_u16++)  = '#';
#endif
    //End test
    
	// caculator crc
	Payload->Length_u16++;
	for (i = 0; i < (Payload->Length_u16 - 1); i++)
		TempCrc ^= *(Payload->Data_a8 + i);

	*(Payload->Data_a8 + Payload->Length_u16 - 1) = TempCrc;
}




uint8_t _mSEND_SIM_ID(int Kind_Send)
{
	_mPayload_Sim_ID(&sMQTT.sPayload);  // Load DATA for Payload

    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}



void _mPayload_Sim_ID(sData *Payload)
{
	uint8_t	    i = 0;
    uint8_t     var = 0;
    uint8_t     TempCrc = 0;

	Payload->Data_a8 		= aPAYLOAD_MQTT;
	Payload->Length_u16 	= MAX_LENGTH_MQTT;

	Reset_Buff(Payload);

    // SIM_ID
	*(Payload->Data_a8 + Payload->Length_u16++) = 0x0B; // obit
	*(Payload->Data_a8 + Payload->Length_u16++) = sSimInfor.sSIMid.Length_u16; // length
    //
	for (var = 0; var < sSimInfor.sSIMid.Length_u16; ++var)
		*(Payload->Data_a8 + Payload->Length_u16++)  = sSimInfor.sSIMid.Data_a8[var];

    // caculator crc
    Payload->Length_u16++;
	for (i = 0; i < (Payload->Length_u16 - 1); i++)
		TempCrc ^= *(Payload->Data_a8 + i);

    *(Payload->Data_a8 + Payload->Length_u16 - 1) = TempCrc;
}



uint8_t _mSEND_RESPOND_SERVER(int Kind_Send)
{
    uint16_t i = 0;

	sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
	sMQTT.sPayload.Length_u16 = MAX_LENGTH_MQTT;

	Reset_Buff(&sMQTT.sPayload);

    for (i = 0; i < sModem.strATResp.Length_u16; i++)
      *(sMQTT.sPayload.Data_a8 +  sMQTT.sPayload.Length_u16++) = *(sModem.strATResp.Data_a8 + i);

    mData_MQTT(Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_RESPOND_FROM_RF(int Kind_Send)
{
    uint16_t i = 0;

	sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
	sMQTT.sPayload.Length_u16 = MAX_LENGTH_MQTT;

	Reset_Buff(&sMQTT.sPayload);

    for (i = 0; i < sModem.strATResp.Length_u16; i++)
      *(sMQTT.sPayload.Data_a8 +  sMQTT.sPayload.Length_u16++) = *(sModem.strATResp.Data_a8 + i);

    mData_MQTT(Kind_Send, 0);

    return 1;
}




uint8_t _mSEND_RESPOND_SERVER_ACK(int Kind_Send)
{
    uint16_t i = 0;

    sMQTT.sPayload.Data_a8 = aPAYLOAD_MQTT;
	sMQTT.sPayload.Length_u16 = MAX_LENGTH_MQTT;

	Reset_Buff(&sMQTT.sPayload);

    for (i = 0; i < sModem.strATResp.Length_u16; i++)
      *(sMQTT.sPayload.Data_a8 +  sMQTT.sPayload.Length_u16++) = *(sModem.strATResp.Data_a8 + i);

    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}


void mPayload_Update_Add(uint8_t *Add_array, uint8_t Code, uint8_t Err)
{
	sMQTT.sPayload.Data_a8 		= Add_array;
	*sMQTT.sPayload.Data_a8		= Code;
	sMQTT.sPayload.Length_u16	= 1;
}
//
void mPayload_Load_MesErr(sData *Payload)
{
 	uint8_t var;

	for (var = 0; var < Payload->Length_u16; ++var)
		*(sMQTT.sPayload.Data_a8 + sMQTT.sPayload.Length_u16++) = *(Payload->Data_a8 + var);
}

uint8_t _mSEND_SERVER_TIME_PENDING(int Kind_Send)
{
    sData sPayTimePending = {(uint8_t *) "Realtime pending",16};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, Kind_Send);
	mPayload_Load_MesErr(&sPayTimePending);

    mData_MQTT(Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_SERVER_TIME_OK(int Kind_Send)
{
    sData sPayTimeOK = {(uint8_t *) "Realtime OK", 11};

	mPayload_Update_Add(aPAYLOAD_MQTT,MS_CORRECT,Kind_Send);
	mPayload_Load_MesErr(&sPayTimeOK);

    mData_MQTT(Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_SERVER_TIME_FAIL(int Kind_Send)
{
    sData sPayTimeFail = {(uint8_t *) "Realtime fail", 13};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_FAIL, Kind_Send);
	mPayload_Load_MesErr(&sPayTimeFail);

    mData_MQTT(Kind_Send, 0);

    return 1;
}

uint8_t _mSEND_SAVE_BOX_FAIL(int Kind_Send)
{
    sData sPaySaveBoxFail = {(uint8_t*)"Luu Kho Fail", 12};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, Kind_Send);
	mPayload_Load_MesErr(&sPaySaveBoxFail);

    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}

uint8_t _mSEND_SAVE_BOK_OK(int Kind_Send)
{
    sData sPaySaveBoxOK = {(uint8_t*)"\r\nOK", 4};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, Kind_Send);
	mPayload_Load_MesErr(&sPaySaveBoxOK);

    mData_MQTT(Kind_Send, MY_QOS);

    return 1;
}




uint8_t _mSEND_EMPTY_MESS(int Kind_Send)
{
    sData sPayEmpFlashFail = {(uint8_t*)"Empty Mess FLash!", 17};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, Kind_Send);
	mPayload_Load_MesErr(&sPayEmpFlashFail);

    mData_MQTT(Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_UPDATE_FIRM_OK(int Kind_Send)
{
    sData strFirmSuccess = {(uint8_t*)"Update Firmware Successfully!", 29};
//    sData strWaitPulse = {(uint8_t*)" - Wait increase 5 pulse", 24};

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, Kind_Send);
	mPayload_Load_MesErr(&strFirmSuccess);

    UTIL_Log(DBLEVEL_M, strFirmSuccess.Data_a8, strFirmSuccess.Length_u16);

#ifdef USING_APP_WM
    sData strWaitPulse = {(uint8_t*)" - Wait increase 5 pulse", 24};

    if (sModem.IsOverFivePulse_u8 == 0)
    {
        mPayload_Load_MesErr(&strWaitPulse);
        UTIL_Log(DBLEVEL_M, strWaitPulse.Data_a8, strWaitPulse.Length_u16);
    }
#endif

    mData_MQTT(Kind_Send, 0);

    return 1;
}


uint8_t _mSEND_UPDATE_FIRM_FAIL(int Kind_Send)
{
    sData strFirmFail;

    switch (sSimFwUpdate.UpdateFailStatus_u8)
    {
        case SEND_UPDATE_FIRMWARE_FAIL_LENGTH:
            strFirmFail.Data_a8 = (uint8_t*) "Update Fail Length!";
            strFirmFail.Length_u16 = 19;
            break;
        case SEND_UPDATE_FIRMWARE_FAIL_FLASH:
            strFirmFail.Data_a8 = (uint8_t*) "Update Fail Flash!";
            strFirmFail.Length_u16 = 18;
            break;
        case SEND_UPDATE_FIRMWARE_FAIL_TIME:
            strFirmFail.Data_a8 = (uint8_t*) "Update Fail Time!";
            strFirmFail.Length_u16 = 17;
            break;
        case SEND_UPDATE_FIRMWARE_FAIL_SETUP:
            strFirmFail.Data_a8 = (uint8_t*) "Update Fail Setup!";
            strFirmFail.Length_u16 = 18;
            break;
        default:
            strFirmFail.Data_a8 = (uint8_t*) "Update Fail!";
            strFirmFail.Length_u16 = 12;
            break;
    }

	mPayload_Update_Add(aPAYLOAD_MQTT, MS_CORRECT, Kind_Send);
	mPayload_Load_MesErr(&strFirmFail);

    UTIL_Log(DBLEVEL_M, strFirmFail.Data_a8, strFirmFail.Length_u16);

#ifdef USING_APP_WM
    sData strWaitPulse = {(uint8_t*)" - Wait increase 5 pulse", 24};

    if (sModem.IsOverFivePulse_u8 == 0)
    {
        mPayload_Load_MesErr(&strWaitPulse);
        UTIL_Log(DBLEVEL_M, strWaitPulse.Data_a8, strWaitPulse.Length_u16);
    }
#endif

    mData_MQTT(Kind_Send, 0);

    return 1;
}




/*---------------Function Callbacck recei-----------------------*/

void _rREQUEST_RESET(sData *str_Receiv, int16_t Pos)
{
    sMQTT.aMARK_MESS_PENDING[SEND_SHUTTING_DOWN] = TRUE;
}

void _rINTAN_DATA (sData *str_Receiv,int16_t Pos)
{
#ifdef USING_APP_WM
	//31/8/2022: intan -> measure ap suat -> send Intan
	sModem.IsReqIntanMess_u8 = TRUE;
	//Active event Measure Press
	fevent_active(sEventAppWM, _EVENT_ENTRY_WM);
#endif
}


__weak void _rREQUEST_SETTING(sData *str_Receiv, int16_t Pos)
{
    uint8_t   length = sMark_MessageRecei_Type[REQUEST_SETTING].sKind.Length_u16;
    uint16_t  PosFix = length + Pos;
    uint8_t   TempLeng;
    uint8_t   ObisConfig = 0;
    uint8_t   TempU8 = 0, i = 0;
    uint16_t  TempU16 = 0;
    uint32_t  TempU32 = 0; 
    
    uint8_t     TempCrc = 0;   
    
    //chay tu tren xuong duoi de check tat ca cac obis cau hinh
    while ((PosFix + 4) <= str_Receiv->Length_u16)   //vi byte cuoi la crc
    {
        TempLeng = 0;
        ObisConfig = *(str_Receiv->Data_a8 + PosFix++);
        switch(ObisConfig)   //luon luon co 1 byte crc phía sau
        {
            case 0x01:  //Lenh set duty cycle: Ex: 0102000A09.
                if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
                {
                    TempU16 = 0;
                    TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                    for (i = 0; i < TempLeng; i++)
                        TempU16 = (TempU16 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);

                    sFreqInfor.FreqSendOnline_u32 = TempU16;
                    
                } else
                {
                    TempU16 = 0;
                    TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                    for (i = 0; i < TempLeng; i++)
                        TempU16 = (TempU16 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);

                    sFreqInfor.FreqWakeup_u32 = TempU16;
                }
                
                //Luu lai
                Save_Freq_Send_Data ();
                DCU_Respond (_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 4, 1);
                AppComm_Set_Next_TxTimer();
                break;
            case 0x02:   //Lenh Reset so xung ve 0
                PosFix++;
            #ifdef USING_APP_WM
                sPulse.Number_u32 = 0;
            #endif

                DCU_Respond (_AT_REQUEST_SERVER, (uint8_t *)"\r\nRESET SO NUOC OK!", 2, 1);
                break;
                
            case 0x03:  //set ve che do luu kho
                PosFix += 2;

                DCU_Respond (_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 12, 1);
                sModem.IsSaveBoxMode_u8 = _AT_REQUEST_SERVER;
                break;
                
            case 0x06:  //Set so lan thuc day. Cho dong bo voi Nuoc hoa binh cua tiep de thong nhat
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);   //length
                TempU8 = *(str_Receiv->Data_a8 + PosFix++);

                if ((TempLeng == 1) && (TempU8 <= MAX_NUMBER_WAKE_UP))
                {
                    sFreqInfor.NumWakeup_u8 = TempU8;

                    Save_Freq_Send_Data ();
                    DCU_Respond (_AT_REQUEST_SERVER, (uint8_t *)"OK", 2, 1);
                } else
                    DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"FAIL", 4, 0);
                break;
                
            case 0x28:
                TempU16 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                    TempU16 = (TempU16 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);

                sFre_IVT_Receive.Pre_IVT = TempU16;
                sFre_IVT_Receive.Status_Receive_u8 = 1;
                sFre_IVT_Receive.Scale_IVT = *(str_Receiv->Data_a8 + PosFix++);
                //Luu lai
//                Save_Freq_Send_Data ();
                break;
                
            case OBIS_ENVI_CONFIG_IVT:
                TempU16 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                TempU16 = (TempU16 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                config_IVT_u8 = TempU16;
                break;
                
            case OBIS_ENVI_RUN_STOP_IVT:
                TempU16 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                TempU16 = (TempU16 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                run_stop_485_IVT = TempU16;
                UnLock_run_stop_485_IVT = run_stop_485_IVT;
                break;
                
            case OBIS_ENVI_ADDR_READ_IVT:
                TempU16 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                if(TempLeng == 0x06)
                {
                    UTIL_MEM_set(sRead_485_IVT.aData , 0x00, TempLeng + 2);
                    sRead_485_IVT.Status_Receive_u8 = 1;
                    for (i = 0; i < TempLeng; i++)
                    {
                        sRead_485_IVT.aData[i] = *(str_Receiv->Data_a8 + PosFix++);
                    }
                }
                break;
                
            case OBIS_ENVI_ADDR_CONFIG_IVT:
                TempU16 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                if(TempLeng == 0x06)
                {
                    UTIL_MEM_set(sConfig_485_IVT.aData , 0x00, TempLeng);
                    sConfig_485_IVT.Status_Receive_u8 = 1;
                    for (i = 0; i < TempLeng; i++)
                    {
                        sConfig_485_IVT.aData[i] = *(str_Receiv->Data_a8 + PosFix++);
                    }
                }
                break;
                
            case OBIS_ENVI_ADDR_RUN_IVT:
                TempU16 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                if(TempLeng == 0x06)
                {
                    UTIL_MEM_set(sRun_485_IVT.aData, 0x00, TempLeng);
                    sRun_485_IVT.Status_Receive_u8 = 1;
                    for (i = 0; i < TempLeng; i++)
                    {
                        sRun_485_IVT.aData[i] = *(str_Receiv->Data_a8 + PosFix++);
                    }
                }
                break;
          
            case OBIS_ENVI_ID_SERVER:
                TempU32 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                TempU32 = (TempU32 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);

                ID_server_u32 = TempU32;
//                Scale = *(str_Receiv->Data_a8 + PosFix++);

                UTIL_MEM_set (aRESPONDSE_AT, 0, sizeof ( aRESPONDSE_AT ));
                sModem.strATResp.Length_u16 = 0;
                
                //Dong goi phan hoi
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = OBIS_TIME_DEVICE;   // sTime
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = 0x06;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sRTC.year;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sRTC.month;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sRTC.date;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sRTC.hour;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sRTC.min;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sRTC.sec;
                
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = OBIS_EMET_FREQ; 
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = 0x02;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = (sFre_IVT_Receive.Pre_IVT >> 8) & 0xFF;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sFre_IVT_Receive.Pre_IVT & 0xFF;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = sFre_IVT_Receive.Scale_IVT;
                
                
                    //----------trang thai ket noi --------------------
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = OBIS_ENVI_CONNECT_STATUS; 
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = 0x01;
                //aRESPONDSE_AT[sModem.strATResp.Length_u16++] = (connect_KDE >> 8) & 0xFF;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = connect_IVT_u8 & 0xFF;
                //aRESPONDSE_AT[sModem.strATResp.Length_u16++] = 0x00;
                
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = OBIS_ENVI_ID_SERVER; 
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = 0x04;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = (ID_server_u32 >> 24) & 0xFF;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = (ID_server_u32 >> 16) & 0xFF;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = (ID_server_u32 >> 8) & 0xFF;
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = ID_server_u32 & 0xFF;
                
                // caculator crc
                sModem.strATResp.Length_u16++;
                for (i = 0; i < (sModem.strATResp.Length_u16 - 1); i++)
                    TempCrc ^= aRESPONDSE_AT[i];

                aRESPONDSE_AT[sModem.strATResp.Length_u16-1] = TempCrc;
                
            #ifdef USING_APP_SIM  
                sMQTT.aMARK_MESS_PENDING[SEND_RESPOND_SERVER] = TRUE;
            #endif
                break;
              
            default:
                return;
        }
    }
}



void _rUPDATE_TIME_SERVER(sData *str_Receiv,int16_t Pos)
{
	ST_TIME_FORMAT 	sRTC_temp = {0};
	uint8_t length = sMark_MessageRecei_Type[UPDATE_TIME_SERVER].sKind.Length_u16;

	if (Pos > 0)
    { // nhan duoc ban tin Realtime
		sRTC_temp.year 		= *(str_Receiv->Data_a8 + Pos + length);
		sRTC_temp.month 	= *(str_Receiv->Data_a8 + Pos + length + 1);
		sRTC_temp.date 		= *(str_Receiv->Data_a8 + Pos + length + 2);
		sRTC_temp.day 		= *(str_Receiv->Data_a8 + Pos + length + 3);
		sRTC_temp.hour 		= *(str_Receiv->Data_a8 + Pos + length + 4);
		sRTC_temp.min 		= *(str_Receiv->Data_a8 + Pos + length + 5);
		sRTC_temp.sec 		= *(str_Receiv->Data_a8 + Pos + length + 6);
	}
	if (Check_update_Time(&sRTC_temp) == TRUE)
    {
	    sRTCSet.year 		= sRTC_temp.year;
	    sRTCSet.month 		= sRTC_temp.month;
	    sRTCSet.date 		= sRTC_temp.date;
	    sRTCSet.day 		= sRTC_temp.day;
	    sRTCSet.hour 		= sRTC_temp.hour;
	    sRTCSet.min 		= sRTC_temp.min;
	    sRTCSet.sec 		= sRTC_temp.sec;

		fevent_active(sEventAppComm, _EVENT_SET_RTC);

		sMQTT.aMARK_MESS_PENDING[SEND_SERVER_TIME_OK] = 1;
	} else
    {
		if ((*(str_Receiv->Data_a8 + Pos + length + 2) == 'U') && (*(str_Receiv->Data_a8 + Pos + length + 3) == 'd'))
		{ // Cap nhat thoi gian
			sMQTT.aMARK_MESS_PENDING[SEND_SERVER_TIME_PENDING] = 1;
		} else
			sMQTT.aMARK_MESS_PENDING[SEND_SERVER_TIME_FAIL] = 1;
	}
}

void _rRESET_DATA(sData *str_Receiv,int16_t Pos)
{
    _rClear_Data_InFlash();
    DCU_Respond(2, (uint8_t *)"\r\nXoa Flash OK", 14, 0);
    Reset_Chip();
}

void _rREQ_AT_CMD(sData *str_Receiv,int16_t Pos)
{    
    UTIL_Printf_Str( DBLEVEL_M, "u_mqtt: request at from server\r\n" );
    
    Check_AT_User(str_Receiv, _AT_REQUEST_SERVER);  
}

void _rREQ_AT_TO_RF(sData *str_Receiv,int16_t Pos)
{
    UTIL_Printf_Str( DBLEVEL_M, "u_mqtt: request at to rf\r\n" );
    
    Check_AT_User(str_Receiv, _AT_REQUEST_LORA);  
}




void _rUPDATE_FIRMWARE(sData *str_Receiv,int16_t Pos)
{
    if (sModem.ModeSimPower_u8 == _POWER_MODE_ONLINE)
    {
        // Split FTP
        if (mUpdate_Split_FTP(str_Receiv) == TRUE)
        {

        } else
            DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nERROR", 7, 0);
    } else
    {
        DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nERROR", 7, 0);
    }
}



/*
 *  Return 	1 : Neu chia cac chuoi ten ra thanh cong
 *  		0 : ko thanh cong
 */

uint8_t	 mUpdate_Split_FTP(sData *str_Receiv)
{
	int pos;

	pos = Find_String_V2((sData *)&sMark_MessageRecei_Type[REQUEST_UPDATE_FIRM].sKind, str_Receiv);
	pos += sMark_MessageRecei_Type[REQUEST_UPDATE_FIRM].sKind.Length_u16;
    //Start path
    Reset_Buff(&sSimFwUpdate.StrURL_FirmPath);
	if (Cut_String(str_Receiv,&pos, &sSimFwUpdate.StrURL_FirmPath) == FALSE)
        return 0;

    Reset_Buff(&sSimFwUpdate.StrURL_FirmName);
	if (Cut_String(str_Receiv, &pos, &sSimFwUpdate.StrURL_FirmName) == FALSE)
        return 0;

    //IP port user pass de mac dinh
	return 1;
}





void _rClear_Data_InFlash (void)
{

}



/*---------------Function handle-----------------------*/
void mInit_Header_Subcribe (void)
{
	sData           Temp 			= {(uint8_t*)"/#",2};
	MQTTString      topicSubcri[] 	= {MQTTString_initializer,MQTTString_initializer};
	int			    MsgId, ReqQos[] = {SUB_QOS,SUB_QOS};
	sData 	        cHeaderSUB_8u   = {(uint8_t*)"AMI/",4} ;
    uint8_t		    aHeader1[HEADER_LENGTH] = {0};
    uint8_t		    aHeader2[HEADER_LENGTH] = {0};
    sData sHeader[] = { {aHeader1, 0}, {aHeader2, 0} };

    // init
    UTIL_MEM_set(aSubcribe_Header, 0, sizeof(aSubcribe_Header));
	sMQTT.sSubcribe.Data_a8				= aSubcribe_Header;
	sMQTT.sSubcribe.Length_u16 			= 0;
    // Subcriber
    // Topic 1
	Copy_String(&sHeader[0], (sData *) &cHeaderSUB_8u, 255, HEADER_LENGTH);
	Copy_String(&sHeader[0], &sModem.sDCU_id, 255, HEADER_LENGTH);
	Copy_String(&sHeader[0], &Temp, 255, HEADER_LENGTH);

	topicSubcri[0].cstring = (char*)sHeader[0].Data_a8;
	MsgId = MY_ID1 | (MY_ID2 << 8);
    // Topic 2
	Copy_String(&sHeader[1],(sData *)&cHeaderSUB_8u,255,HEADER_LENGTH);
	Copy_String(&sHeader[1],(sData *)&sMark_MessageRecei_Type[UPDATE_TIME_SERVER].sKind,255,HEADER_LENGTH);

	topicSubcri[1].cstring = (char*)sHeader[1].Data_a8;

	sMQTT.sSubcribe.Length_u16 = MQTTSerialize_subscribe(sMQTT.sSubcribe.Data_a8, HEADER_LENGTH, 0, MsgId, 2, topicSubcri, ReqQos);
}



void mConnect_MQTT (void)
{
    sMQTT.KeepAliveTime_u8 = sSimInfor.sServer.KeepAlive_u32;

	mSet_default_MQTT();
	Connect_Packet.clientID.cstring = (char*)sModem.sDCU_id.Data_a8;
	Connect_Packet.keepAliveInterval = sMQTT.KeepAliveTime_u8;
	Connect_Packet.cleansession = 1;
	Connect_Packet.username.cstring = (char*)sSimInfor.sServer.sUserNameMQTTMain.Data_a8;
	Connect_Packet.password.cstring = (char*)sSimInfor.sServer.sPasswordMQTTMain.Data_a8;

	//dua ban tin chua id username pass vao buff
	sMQTT.str.Length_u16 = MQTTSerialize_connect(sMQTT.str.Data_a8, sizeof(aDATA_MQTT), &Connect_Packet);
	//gui ban tin chua id va username pass lên uart sim va uart debug
	sMQTT.LandmarkSendPing_u32 = RtCountSystick_u32;

	mSend_Packet_MQTT(&sMQTT);
}



void mSubcribe_MQTT (void)
{
    mInit_Header_Subcribe();

	mSet_default_MQTT();
	sMQTT.str.Data_a8 		= sMQTT.sSubcribe.Data_a8;
	sMQTT.str.Length_u16 	= sMQTT.sSubcribe.Length_u16;
	// Send sub trong 2 truong hop : DATA va Time
	mSend_Packet_MQTT(&sMQTT);
}


void mPublish_MQTT (void)
{
    mSend_Packet_MQTT(&sMQTT);
}

/*
    Func: Check empty mess
*/
uint8_t mCheck_Empty_Mess (void)
{
    uint16_t i = 0;

    for (i = TOPIC_NOTIF; i < END_MQTT_SEND; i++)
        if (sMQTT.aMARK_MESS_PENDING[i] == 1)
            return 0;

    return 1;
}



