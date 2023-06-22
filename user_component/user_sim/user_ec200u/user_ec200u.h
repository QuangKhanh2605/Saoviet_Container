/*
 * siml506.h
 *
 *  Created on: Feb 16, 2022
 *      Author: chien
 */

#ifndef USER_EC200U_H_
#define USER_EC200U_H_

#include "user_util.h"
#include "main.h"
#include "user_sim_common.h"


//#define USING_TRANSPARENT_MODE     

#define CID_SERVER              0x32

/*================== Var struct =====================*/
typedef enum
{
    //Cmd Pre Init
	_SIM_AT_AT,
	_SIM_AT_ECHO,
    //Cmd Init
    _SIM_AT_BAUD_RATE,
    _SIM_AT_GET_ID,
	_SIM_AT_CHECK_RSSI,
	_SIM_AT_GET_IMEI,
	_SIM_AT_CHECK_SIM,
    _SIM_AT_SET_APN_1,
    _SIM_AT_SET_APN_2,
    _SIM_AT_APN_AUTH_1,
    _SIM_AT_APN_AUTH_2,
    
    _SIM_AT_DTR_MODE,
    _SIM_AT_TEST_0,
    _SIM_AT_GET_IP_APN,
    _SIM_AT_ACTIVE_APN,
    //SMS
    _SIM_SMS_FORMAT_TEXT,
    _SIM_SMS_NEW_MESS_IND,
    _SIM_SMS_READ_UNREAD,
    _SIM_SMS_READ_ALL,
    _SIM_SMS_DEL_ALL,
    //Calling
    _SIM_CALLING_LINE,
    //Cmd Check Network
	_SIM_NET_CEREG,
	_SIM_NET_CHECK_ATTACH,
    //Cmd clock
	_SIM_NET_GET_RTC,
    //Cmd Data mode and command mode
    _SIM_TCP_OUT_DATAMODE,
    _SIM_TCP_IN_DATAMODE,
    //Cmd TCP
    _SIM_TCP_IP_SIM,
    
    _SIM_TCP_CLOSE_1,
    _SIM_TCP_CLOSE_2,
    
	_SIM_TCP_CONNECT_1,
    _SIM_TCP_CONNECT_2,
    
    _SIM_TCP_SEND_CMD_1,
    _SIM_TCP_SEND_CMD_2,
    _SIM_TCP_SEND_CMD_3,
        
    _SIM_MQTT_CONN_1,
    _SIM_MQTT_CONN_2,
    _SIM_MQTT_SUB_1,
    _SIM_MQTT_SUB_2,
    _SIM_MQTT_PUB_1,
    _SIM_MQTT_PUB_2,
    _SIM_MQTT_PUB_FB_1,
    _SIM_MQTT_PUB_FB_2,
    
    _SIM_TCP_PING_TEST1,
    _SIM_TCP_PING_TEST2,
    //File
    _SIM_SYS_FILE_DEL1,  
    _SIM_SYS_FILE_DEL2,
    _SIM_SYS_FILE_DEL_ALL,
    _SIM_SYS_FILE_LIST,
    _SIM_SYS_FILE_OPEN1,
    _SIM_SYS_FILE_OPEN2,
    _SIM_SYS_FILE_CLOSE1,
    _SIM_SYS_FILE_CLOSE2,
    _SIM_SYS_FILE_SEEK1,
    _SIM_SYS_FILE_SEEK2,
    _SIM_SYS_FILE_POS1,
    _SIM_SYS_FILE_POS2,
    _SIM_SYS_FILE_READ1,
    _SIM_SYS_FILE_READ2,
    //FTP
    _SIM_FTP_SERVER_1,
    _SIM_FTP_SERVER_2,
    _SIM_FTP_PORT,
    
    _SIM_FTP_USERNAME,
    _SIM_FTP_PASSWORD,
    _SIM_FTP_TYPE,
    _SIM_FTP_LIST_FILE,
    _SIM_FTP_GET_FILE_1,
    _SIM_FTP_GET_FILE1,
    _SIM_FTP_GET_FILE2,
    _SIM_FTP_GET_FILE3,
    _SIM_FTP_READ_1,
    _SIM_FTP_READ_2,
    //Http
    _SIM_HTTP_CFG_CONTEXT,
    _SIM_HTTP_CFG_HEADER1,
    _SIM_HTTP_CFG_HEADER2,
    _SIM_HTTP_CFG_SSL,
    
    _SIM_HTTP_SET_URL_1,
    _SIM_HTTP_SET_URL_2,
    _SIM_HTTP_SET_URL_3,
    
    _SIM_HTTP_REQUEST_GET,
    _SIM_HTTP_STORE_FILE1,
    _SIM_HTTP_STORE_FILE2,

    _SIM_HTTP_READ_1,
    _SIM_HTTP_READ_2,
    
    _SIM_GPS_CFG_TOKEN,
    _SIM_GPS_CFG_FORMAT,
    _SIM_GPS_QUERY_TOKEN,
    _SIM_GPS_GET_LOCATION,
    
    _SIM_GPS_CFG_OUT_PORT,
    _SIM_GPS_CFG_AUTO_RUN,
    _SIM_GPS_DEL_CUR_DATA,
    _SIM_GPS_TURN_ON,
    _SIM_GPS_POSTION_INF,
    _SIM_GPS_TURN_OFF,
        
	_SIM_END,                // don't use
}AT_SIM_TypeDef;

typedef enum
{
	_SIM_URC_RESET_SIM900 = 0,
	_SIM_URC_SIM_LOST,
	_SIM_URC_SIM_REMOVE,
	_SIM_URC_CLOSED,
	_SIM_URC_PDP_DEACT,
	_SIM_URC_CALL_READY,
    _SIM_URC_CALLING,
    _SIM_URC_SMS_CONTENT,
    _SIM_URC_SMS_INDICATION1,
    _SIM_URC_SMS_INDICATION2,
	_SIM_URC_ERROR,
    _SIM_URC_RECEIV_SERVER,
	_SIM_URC_END,
}URC_SIM_TypeDef;




/*===================== extern ====================*/
extern const sCommand_Sim_Struct aSimL506Step[];
extern const sCommand_Sim_Struct aSimUrc[];

extern uint8_t aSimStepBlockInit[13];
extern uint8_t aSimStepBlockNework[10];
#ifdef USING_TRANSPARENT_MODE
    extern uint8_t aSimStepBlockConnect[7];
    extern uint8_t aSIM_STEP_PUBLISH_FB[2];
    extern uint8_t aSIM_STEP_PUBLISH[2];
#else
    extern uint8_t aSimStepBlockConnect[11];
    extern uint8_t aSIM_STEP_PUBLISH_FB[4];
    extern uint8_t aSIM_STEP_PUBLISH[4];
#endif
extern uint8_t aSIM_STEP_INFOR_CSQ[3];
extern uint8_t aSimStepBlockDisConnect[4];
extern uint8_t aSimStepBlockGetClock[3];

extern uint8_t aSimStepBlockHttpInit[19];
extern uint8_t aSimStepBlockHttpRead[2];
extern uint8_t aSimStepBlockSMS[4]; 
extern uint8_t aSimStepBlockFileClose[2];

extern uint8_t aSimStepBlockLocation[2]; 
extern uint8_t aSimStepBlockDelLocation[1];

/*================== Function ==========================*/
uint8_t     EC200U_Check_Step_Long_Timeout (uint8_t sim_step);
sCommand_Sim_Struct * EC200U_Get_AT_Cmd (uint8_t step);
uint32_t    EC200U_Get_Delay_2_AT (uint8_t Step);
uint8_t     EC200U_Is_Step_Check_URC (uint8_t sim_step);
uint8_t     EC200U_Check_Step_Skip_Error (uint8_t step);

#endif /* SIM_MODULE_SIML506_SIML506_H_ */
