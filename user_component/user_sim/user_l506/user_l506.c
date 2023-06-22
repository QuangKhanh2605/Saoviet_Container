/*
 * siml506.c
 *
 *  Created on: Feb 16, 2022
 *      Author: chien
 */


#include "string.h"

#include "user_l506.h"
#include "user_string.h"

     
/* ========= handler Sim driver ============*/
/* define static functions */
static uint8_t at_callback_success(sData *uart_string);
static uint8_t at_callback_failure(uint8_t Type);

/*========== Callback AT ===================*/
static uint8_t _CbAT_SIM_ID(sData *uart_string);
static uint8_t _CbAT_CHECK_RSSI(sData *uart_string);
static uint8_t _CbAT_GET_IMEI(sData *uart_string);
static uint8_t _CbAT_SET_APN (sData *uart_string);
static uint8_t _CbAT_APN_AUTH (sData *uart_string);
static uint8_t _CbAT_APN_ACTIVE (sData *uart_string);

static uint8_t _Cb_AT_CHECK_ATTACH(sData *str_Receive);

static uint8_t _CbAT_GET_CLOCK(sData *uart_string);
static uint8_t _CbAT_CHECK_SIM(sData *uart_string);
static uint8_t _CbAT_GET_IP_SIM(sData *uart_string);
static uint8_t _CbAT_TCP_CLOSE_1(sData *uart_string);
static uint8_t _CbAT_NON_TRANPARENT_MODE(sData *uart_string);
static uint8_t _CbAT_TRANPARENT_MODE(sData *uart_string);
static uint8_t _CbAT_OPEN_TCP_1 (sData *uart_string);
static uint8_t _CbAT_OPEN_TCP_2 (sData *uart_string);
static uint8_t _CbAT_TCP_SEND_1 (sData *uart_string);
static uint8_t _CbAT_TCP_SEND_2 (sData *uart_string);

/*========== Mqtt Callback ==============*/
static uint8_t _Cb_MQTT_CONNECT_1(sData *str_Receiv);
static uint8_t _Cb_MQTT_CONNECT_2(sData *str_Receiv);
static uint8_t _Cb_MQTT_SUBCRIBE_1(sData *str_Receiv);
static uint8_t _Cb_MQTT_SUBCRIBE_2(sData *str_Receiv);
static uint8_t _Cb_MQTT_PUBLISH_1(sData *str_Receiv);
static uint8_t _Cb_MQTT_PUBLISH_2(sData *str_Receiv);

/*========== Callback URC ===================*/
static uint8_t _CbURC_ERROR (sData *uart_string);
static uint8_t _CbURC_CALL_READY (sData *uart_string);
static uint8_t _CbURC_CLOSED (sData *uart_string);
static uint8_t _CbURC_SIM_LOST (sData *uart_string);
static uint8_t _CbURC_RESET_SIM900 (sData *uart_string);
static uint8_t _CbAT_PING_TEST (sData *uart_string);
static uint8_t _Cb_RECEIV_SERVER (sData *uart_string);
static uint8_t _CbURC_SMS_IND (sData *uart_string);
static uint8_t _CbURC_SMS_CONTENT (sData *uart_string);
static uint8_t _CbURC_CALLING (sData *uart_string);

//File sys callback
static uint8_t _CbSYS_DEL_FILE(sData *uart_string);
//FTP Callback
static uint8_t _CbFTP_SEND_IP(sData *uart_string);
static uint8_t _CbFTP_GET_FILE1(sData *uart_string);
static uint8_t _CbFTP_GET_FILE2(sData *uart_string);
static uint8_t _CbFTP_GET_FILE3(sData *uart_string);
static uint8_t _CbFTP_READ_1(sData *uart_string);
static uint8_t _CbFTP_READ_2(sData *uart_string);
//Callback HTTP
static uint8_t _CbHTTP_SETURL_1(sData *uart_string);
static uint8_t _CbHTTP_SETURL_2(sData *uart_string);
static uint8_t _CbHTTP_FIRM_LENGTH(sData *uart_string);
static uint8_t _CbHTTP_READ_1(sData *uart_string);
static uint8_t _CbHTTP_READ_2(sData *uart_string);

/*============= Var struct ====================*/
//Block step control SIM
uint8_t aSimStepBlockInit[15] = {
	_SIM_AT_ECHO,
    _SIM_AT_ECHO,
	_SIM_AT_AT,
	_SIM_AT_MAN_LOG,
	_SIM_NET_RA_AUTO,
	_SIM_NET_CEREG,
	_SIM_AT_BAUD_RATE,
	_SIM_AT_GET_ID,
	_SIM_AT_CHECK_RSSI,
	_SIM_AT_GET_IMEI,
    _SIM_AT_DTR_MODE,
	_SIM_AT_CHECK_SIM, 
    _SIM_CALLING_LINE,
    _SIM_SMS_FORMAT_TEXT,
    _SIM_SMS_NEW_MESS_IND,
};

  

uint8_t aSimStepBlockNework[10] = 
{
	_SIM_NET_CHECK_ATTACH,
    _SIM_SMS_READ_UNREAD,
    _SIM_SMS_DEL_ALL,
    _SIM_AT_CHECK_RSSI,
	_SIM_NET_SYN_TIME_ZONE,
	_SIM_NET_GET_RTC,
	_SIM_AT_SET_APN_1,
    _SIM_AT_SET_APN_2,
	_SIM_AT_APN_AUTH_1,
    _SIM_AT_APN_AUTH_2,
};




#ifdef USING_TRANSPARENT_MODE
    uint8_t aSimStepBlockConnect[13] = 
    {
        _SIM_TCP_CLOSE_1,
        _SIM_TCP_CLOSE_2,
        _SIM_TCP_NETCLOSE,
        _SIM_TCP_TRANS_SETUP, //1
        _SIM_TCP_TRANS,       //2
        _SIM_TCP_NETOPEN,     //3
        _SIM_TCP_GET_IP_SIM,      //4
        _SIM_TCP_CONNECT_1,   //5
        _SIM_TCP_CONNECT_2,   //6
        
        _SIM_MQTT_CONN_1,        //7
        _SIM_MQTT_CONN_2,        //8

        _SIM_MQTT_SUB_1,         //9
        _SIM_MQTT_SUB_2,         //10
    };

    uint8_t aSIM_STEP_PUBLISH[2] =
    {
        _SIM_MQTT_PUB_1,
        _SIM_MQTT_PUB_2,
    };

    uint8_t aSIM_STEP_PUBLISH_FB[2] =
    {
        _SIM_MQTT_PUB_FB_1,
        _SIM_MQTT_PUB_FB_2,
    };
#else
    uint8_t aSimStepBlockConnect[17] = 
    {
        _SIM_TCP_CLOSE_1,
        _SIM_TCP_CLOSE_2,
        _SIM_TCP_NETCLOSE,
        _SIM_TCP_TRANS_SETUP, //1
        _SIM_TCP_TRANS,       //2
        _SIM_TCP_NETOPEN,     //3
        _SIM_TCP_GET_IP_SIM,      //4
        _SIM_TCP_CONNECT_1,   //5
        _SIM_TCP_CONNECT_2,   //6
        
        _SIM_MQTT_CONN_1,        //7
        _SIM_TCP_SEND_CMD_1,
        _SIM_TCP_SEND_CMD_2,
        _SIM_MQTT_CONN_2,        //8

        _SIM_MQTT_SUB_1,         //9
        _SIM_TCP_SEND_CMD_1,
        _SIM_TCP_SEND_CMD_2,
        _SIM_MQTT_SUB_2,         //10
    };

    
    uint8_t aSIM_STEP_PUBLISH[4] =
    {
        _SIM_MQTT_PUB_1,
        _SIM_TCP_SEND_CMD_1,
        _SIM_TCP_SEND_CMD_2,
        _SIM_TCP_SEND_CMD_3,
    };

    uint8_t aSIM_STEP_PUBLISH_FB[4] =
    {
        _SIM_MQTT_PUB_FB_1,
        _SIM_TCP_SEND_CMD_1,
        _SIM_TCP_SEND_CMD_2,
        _SIM_MQTT_PUB_FB_2,
    };
#endif

uint8_t  aSIM_STEP_INFOR_CSQ[3] = 
{
    _SIM_TCP_OUT_DATAMODE,
    _SIM_AT_CHECK_RSSI,
    _SIM_TCP_IN_DATAMODE,
};


uint8_t aSimStepBlockDisConnect[4] = 
{
    _SIM_TCP_OUT_DATAMODE,
	_SIM_TCP_CLOSE_1,
    _SIM_TCP_CLOSE_2,
	_SIM_TCP_NETCLOSE,
};

uint8_t aSimStepBlockGetClock[3] = 
{
    _SIM_TCP_OUT_DATAMODE,
    _SIM_NET_GET_RTC,
    _SIM_TCP_IN_DATAMODE,
};

uint8_t aSimStepBlockHttpInit[19] = 
{
    _SIM_NET_CHECK_ATTACH,
    _SIM_AT_CHECK_RSSI,
    _SIM_AT_SET_APN_1,
    _SIM_AT_SET_APN_2,
    _SIM_AT_APN_AUTH_1,
    _SIM_AT_APN_AUTH_2,
    _SIM_AT_TEST_0,
    _SIM_AT_GET_IP_APN,
    _SIM_AT_ACTIVE_APN,
    _SIM_AT_GET_IP_APN,
    
    _SIM_TCP_PING_TEST1,
    _SIM_TCP_PING_TEST2,
    _SIM_HTTP_CLOSE,
    _SIM_HTTP_OPEN,
    _SIM_HTTP_SET_URL_1,
    _SIM_HTTP_SET_URL_2,
    _SIM_HTTP_SAVE_LOCAL,
    _SIM_HTTP_REQUEST_GET,
    _SIM_HTTP_LENGTH,
};

uint8_t aSimStepBlockHttpRead[2] = 
{
    _SIM_HTTP_READ_1,
    _SIM_HTTP_READ_2,
};



//Read SMS
uint8_t aSimStepBlockSMS[4] = 
{
    _SIM_TCP_OUT_DATAMODE,
    _SIM_SMS_READ_UNREAD,
    _SIM_SMS_DEL_ALL,
    _SIM_TCP_IN_DATAMODE,
};  


/*========= Struct List AT Sim ==================*/
static char DataConack_8a[] = { MY_CONNACK, 0x02, 0x00 };		 //20020000
static char DataSuback_8a[] = { MY_SUBACK, 0x04, 0x00};          //9004000a00
static char DataPuback_8a[] = { MY_PUBACK, 0x02, 0x00};	 //40020001  , 0x1A 

    
const sCommand_Sim_Struct aSimL506Step[] =
{
    //Cmd Pre Init
	{	_SIM_AT_AT,			    at_callback_success,        at_callback_failure,            "OK", 			    "AT\r" 	        },
	{	_SIM_AT_ECHO,			at_callback_success,        at_callback_failure,            "OK", 			    "ATE0\r" 	 	},
	{   _SIM_AT_MAN_LOG,    	at_callback_success,        at_callback_failure,            "OK",	            "AT+MLOGK=5\r"  },
    //Cmd Init
    {   _SIM_AT_BAUD_RATE, 		at_callback_success,        at_callback_failure,            "OK",			    "AT+IPR=115200\r"	},
    {	_SIM_AT_GET_ID,			_CbAT_SIM_ID,               at_callback_failure,            "CCID: ", 	        "AT+CICCID\r" 	},
	{	_SIM_AT_CHECK_RSSI,		_CbAT_CHECK_RSSI,           at_callback_failure,            "+CSQ: ", 		    "AT+CSQ\r"	 	},
	{   _SIM_AT_GET_IMEI, 		_CbAT_GET_IMEI,             at_callback_failure,            "OK", 		        "AT+CGSN\r"		},
	{	_SIM_AT_CHECK_SIM,		_CbAT_CHECK_SIM,            at_callback_failure,            "OK", 			    "AT+CPIN?\r"   	},
    {	_SIM_AT_SET_APN_1,      _CbAT_SET_APN,              at_callback_failure,            NULL,	            "AT+CGDCONT=1,\"IP\",\""		},// AT+CGDCONT=1,"IP","V3G2057"  - "AT+CGDCONT=1,\"IP\",\"",19}
	{	_SIM_AT_SET_APN_2,      at_callback_success,        at_callback_failure,            "OK",	            "\"\r"         }, 
    {	_SIM_AT_APN_AUTH_1,	    _CbAT_APN_AUTH,             at_callback_failure,            NULL,			    "AT$QCPDPP=1,",	},// AT$QCPDPP=1,0 - AT$QCPDPP=1,",12}
    {   _SIM_AT_APN_AUTH_2,     at_callback_success,        at_callback_failure,            "OK",               "\r" 	        },
    
    {   _SIM_AT_DTR_MODE, 	    at_callback_success,        at_callback_failure,            "OK",               "AT&D1\r"	    },
    {   _SIM_AT_TEST_0, 		at_callback_success,        at_callback_failure,            "OK",               "AT+CGATT=1\r" 	},
    {   _SIM_AT_GET_IP_APN,     _CbAT_APN_ACTIVE,           at_callback_failure,            "+CGPADDR: 1,",     "AT+CGPADDR=1\r"	},
    {   _SIM_AT_ACTIVE_APN, 	at_callback_success,        at_callback_failure,            "OK",               "AT+CGACT=1,1\r"	},
   //SMS
    {   _SIM_SMS_FORMAT_TEXT,	at_callback_success,        at_callback_failure,            "OK",               "AT+CMGF=1\r"	},  
    {   _SIM_SMS_NEW_MESS_IND,	at_callback_success,        at_callback_failure,            "OK",               "AT+CNMI=2,1\r"	    },
    {   _SIM_SMS_READ_UNREAD,  	at_callback_success,        at_callback_failure,            "\r\nOK",           "AT+CMGL=\"REC UNREAD\"\r"	    },
    {   _SIM_SMS_READ_ALL,  	at_callback_success,        at_callback_failure,            "\r\nOK",           "AT+CMGL=\"ALL\"\r"	        },
    {   _SIM_SMS_DEL_ALL,	    at_callback_success,        at_callback_failure,            "OK",               "AT+CMGD=1,4\r"	},
    //Calling
    {   _SIM_CALLING_LINE, 		at_callback_success,        at_callback_failure,            "OK",               "AT+CLIP=1\r"   },
    //Cmd Set 2G 3G 4G
	{	_SIM_NET_RA_AUTO,       at_callback_success,        at_callback_failure,            "OK",			    "AT+CNMP=2\r"	},
	{   _SIM_NET_RA_2G,         at_callback_success,        at_callback_failure,            "OK",			    "AT+CNMP=13\r"  },
	{   _SIM_NET_RA_3G,         at_callback_success,        at_callback_failure,            "OK",			    "AT+CNMP=14\r"  },
	{  	_SIM_NET_RA_4G,         at_callback_success,        at_callback_failure,            "OK",			    "AT+CNMP=38\r"  },

	{	_SIM_NET_CHECK_RA,      at_callback_success,        at_callback_failure,            "+CNMP:",       	"AT+CNMP?\r"    },
	{	_SIM_NET_CHECK_BAND,    at_callback_success,        at_callback_failure,            "+CNBP:",           "AT+CNBP?\r"    },
	{	_SIM_NET_CHECK_CSPI,    at_callback_success,        at_callback_failure,            "+CPSI:",           "AT+CPSI?\r"    },
    //Cmd Check Network
	{	_SIM_NET_CEREG, 	    at_callback_success,        at_callback_failure,            "OK",			    "AT+CEREG=0\r"	},
	{	_SIM_NET_CHECK_ATTACH,	_Cb_AT_CHECK_ATTACH,        at_callback_failure,            "OK",	            "AT+CGATT?\r"	},
    //Cmd Config
	{	_SIM_NET_SYN_TIME_ZONE, at_callback_success,        at_callback_failure,            "OK",			    "AT+MTZ=1\r"	},
	{	_SIM_NET_GET_RTC, 		_CbAT_GET_CLOCK,            at_callback_failure,            "+CCLK:",		    "AT+CCLK?\r"	},
    //Cmd Data mode and command mode
#ifdef USING_TRANSPARENT_MODE
    {   _SIM_TCP_OUT_DATAMODE, 	_CbAT_NON_TRANPARENT_MODE,  at_callback_failure,            NULL,               "+++" 			}, 
    {   _SIM_TCP_TRANS, 		_CbAT_TRANPARENT_MODE,      at_callback_failure,            "OK",               "AT+CIPMODE=2\r" 	 	}, 
    {   _SIM_TCP_IN_DATAMODE,   _CbAT_TRANPARENT_MODE,      at_callback_failure,            "CONNECT",          "ATO\r"		    },
#else
    {   _SIM_TCP_OUT_DATAMODE, 	_CbAT_NON_TRANPARENT_MODE,  at_callback_failure,            NULL, 			    NULL            }, 
    {   _SIM_TCP_TRANS, 		_CbAT_TRANPARENT_MODE,      at_callback_failure,            "OK",               "AT+CIPMODE=0\r", 		}, 
    {   _SIM_TCP_IN_DATAMODE,   _CbAT_TRANPARENT_MODE,      at_callback_failure,            NULL,		        NULL            },
#endif   
    
    {   _SIM_TCP_TRANS_SETUP,   at_callback_success,        at_callback_failure,            "OK",               "AT+MCIPCFGPL=0,1,0,0,0\r",    	}, 
    //Cmd TCP 
    {   _SIM_TCP_GET_IP_SIM,	_CbAT_GET_IP_SIM,           at_callback_failure,            "+IPADDR:",         "AT+IPADDR\r"   },         
    {	_SIM_TCP_NETOPEN,	    at_callback_success,        at_callback_failure,            "+NETOPEN:SUCCESS", "AT+NETOPEN\r"	},
    {   _SIM_TCP_NETCLOSE,      at_callback_success,        at_callback_failure,            "OK",               "AT+NETCLOSE\r"	},

    {	_SIM_TCP_CLOSE_1,	 	_CbAT_TCP_CLOSE_1,          at_callback_failure,            NULL,	            "AT+CIPCLOSE="  	},
    {	_SIM_TCP_CLOSE_2,	 	at_callback_success,        at_callback_failure,            "OK",	            "\r"  	}, 

    {	_SIM_TCP_CONNECT_1,	    _CbAT_OPEN_TCP_1,           at_callback_failure,            NULL,               "AT+CIPOPEN="  	},
#ifdef USING_TRANSPARENT_MODE
    {	_SIM_TCP_CONNECT_2,	    _CbAT_OPEN_TCP_2,           at_callback_failure,            "CONNECT",          ",0\r"          }, 
#else
    {	_SIM_TCP_CONNECT_2,	    _CbAT_OPEN_TCP_2,           at_callback_failure,            "+CIPOPEN:",        ",0\r"          },
#endif        
    {   _SIM_TCP_SEND_CMD_1, 	_CbAT_TCP_SEND_1,           at_callback_failure,            NULL,               "AT+CIPSEND="  	},
    {   _SIM_TCP_SEND_CMD_2, 	_CbAT_TCP_SEND_2,           at_callback_failure,            ">",                 "\r"	 		},
    {   _SIM_TCP_SEND_CMD_3, 	_Cb_MQTT_PUBLISH_2,         at_callback_failure,            "+CIPSEND:SUCCESS",	 NULL	        },
    
    //Mqtt AT
	{   _SIM_MQTT_CONN_1, 		_Cb_MQTT_CONNECT_1,         at_callback_failure,            NULL,	            NULL            },
	{   _SIM_MQTT_CONN_2, 		_Cb_MQTT_CONNECT_2,         at_callback_failure,            DataConack_8a,      NULL       	    },  //4,"20020000"

	{   _SIM_MQTT_SUB_1, 		_Cb_MQTT_SUBCRIBE_1,        at_callback_failure,            NULL,	            NULL        	},
	{   _SIM_MQTT_SUB_2, 		_Cb_MQTT_SUBCRIBE_2,        at_callback_failure,            DataSuback_8a,      NULL  			},

	{   _SIM_MQTT_PUB_1, 		_Cb_MQTT_PUBLISH_1,         at_callback_failure,            NULL,				NULL           	},
	{   _SIM_MQTT_PUB_2, 		_Cb_MQTT_PUBLISH_2,         at_callback_failure,            NULL,	   			NULL        	},  //"4,40020001"  "SEND OK"

	{   _SIM_MQTT_PUB_FB_1,     _Cb_MQTT_PUBLISH_1,         at_callback_failure,            NULL,				NULL          	},
	{   _SIM_MQTT_PUB_FB_2, 	_Cb_MQTT_PUBLISH_2,         at_callback_failure,            DataPuback_8a,      NULL   			},  // 4,"40020001"
    //Ping
    {   _SIM_TCP_PING_TEST1,    _CbAT_PING_TEST,            at_callback_failure,            NULL,               "AT+MPING=\""  	},    
    {   _SIM_TCP_PING_TEST2,    at_callback_success,        at_callback_failure,            "+MPING:3",         "\",1,4\r"     	},
    
    // File	 
    {   _SIM_SYS_DEL_FILE_1,    _CbSYS_DEL_FILE,            at_callback_failure,            NULL,               "AT+MFSDEL=\"" 	},
    {   _SIM_SYS_DEL_FILE_2,    at_callback_success,        at_callback_failure,            "OK",               "\"\r"        	},
    {   _SIM_SYS_LIST_FILE,     at_callback_success,        at_callback_failure,            "OK",               "AT+MFSLS\r"   	},    
    // FTP		
    {   _SIM_FTP_SERVER_1, 	    _CbFTP_SEND_IP,             at_callback_failure,            NULL,               "AT+CFTPSERV=\""  	},
    {   _SIM_FTP_SERVER_2, 	    at_callback_success,        at_callback_failure,            "OK",               "\"\r" 		  	    },
    {   _SIM_FTP_PORT,          at_callback_success,        at_callback_failure,            "OK",               "AT+CFTPPORT=21\r"	},    
        
    {   _SIM_FTP_USERNAME, 	    at_callback_success,        at_callback_failure,            "OK",               "AT+CFTPUN=\"admin\"\r" 	},
    {   _SIM_FTP_PASSWORD, 	    at_callback_success,        at_callback_failure,            "OK",               "AT+CFTPPW=\"admin\"\r"		},
    {   _SIM_FTP_TYPE, 		    at_callback_success,        at_callback_failure,            "OK",               "AT+CFTPTYPE=\"I\"\r"   	},
    
    {   _SIM_FTP_LIST_FILE,     at_callback_success,        at_callback_failure,            "+CFTPLIST:SUCCESS", "AT+CFTPLIST=\"/\"\r"   	},
    {   _SIM_FTP_GET_FILE_1,    at_callback_success,        at_callback_failure,            "+CFTPGET:",        "AT+CFTPGET=\"/Test/FIRM_HCMCHES2.bin\",10\r"  	},
    
    {   _SIM_FTP_GET_FILE1,     _CbFTP_GET_FILE1,           at_callback_failure,            NULL,               "AT+CFTPGETFILE=\"/"      	},
    {   _SIM_FTP_GET_FILE2,     _CbFTP_GET_FILE2,           at_callback_failure,            NULL,               "\",\""            	},
    {   _SIM_FTP_GET_FILE3,     _CbFTP_GET_FILE3,           at_callback_failure,            "+CFTPGETFILE:SUCCESS",     "\",0\r"	},
    
    {   _SIM_FTP_READ_1,        _CbFTP_READ_1,              at_callback_failure,            NULL,               "AT+CFTPRDFILE=\""	},    
    {   _SIM_FTP_READ_2,        _CbFTP_READ_2,              at_callback_failure,            "+CFTPRDFILE:SUCCESS",      "\r"        },
    
    // HTTP	
    {   _SIM_HTTP_OPEN,         at_callback_success,        at_callback_failure,            "OK",               "AT$HTTPOPEN\r"     },
    {   _SIM_HTTP_CLOSE,        at_callback_success,        at_callback_failure,            "OK",               "AT$HTTPCLOSE\r"  	},
    {   _SIM_HTTP_SAVE_LOCAL,   at_callback_success,        at_callback_failure,            "OK",               "AT$HTTPTYPE=1\r" 	},
    
    {   _SIM_HTTP_SET_URL_1,    _CbHTTP_SETURL_1,           at_callback_failure,            NULL,               "AT$HTTPPARA="    	},
    {   _SIM_HTTP_SET_URL_2,    _CbHTTP_SETURL_2,           at_callback_failure,            "OK",               "\r"                },

    {   _SIM_HTTP_REQUEST_GET,  at_callback_success,        at_callback_failure,           "OK",               "AT$HTTPACTION=0,1\r"   },
    {   _SIM_HTTP_LENGTH,       _CbHTTP_FIRM_LENGTH,        at_callback_failure,            "$HTTPREAD:1",      "AT$HTTPREAD=1\r"     	},    
    
    {   _SIM_HTTP_READ_1,       _CbHTTP_READ_1,             at_callback_failure,            NULL,               "AT$HTTPREAD=0,"  	},
    {   _SIM_HTTP_READ_2,       _CbHTTP_READ_2,             at_callback_failure,            "\r\nOK\r\n",       "\r"               	},
	// ...
	{	_SIM_END, 			    at_callback_success,        at_callback_failure,            NULL,		        NULL,	            },
};

      
const sCommand_Sim_Struct aSimUrc[] =
{
	{_SIM_URC_RESET_SIM900,		_CbURC_RESET_SIM900,        NULL,       "NORMAL POWER DOWN",        NULL 	},// OK
	{_SIM_URC_SIM_LOST, 		_CbURC_SIM_LOST,            NULL,       "SIM CRASH",		 		NULL    },
	{_SIM_URC_SIM_REMOVE, 		_CbURC_SIM_LOST,            NULL,       "SIM REMOVED",		 		NULL	},
	{_SIM_URC_CLOSED,			_CbURC_CLOSED,              NULL,       "+SERVER DISCONNECTED:", 	NULL	},   //+SERVER DISCONNECTED:0
	{_SIM_URC_PDP_DEACT, 		_CbURC_CLOSED,              NULL,       "+NETWORK DISCONNECTED",    NULL	},   //+NETWORK DISCONNECTED:0
	{_SIM_URC_CALL_READY, 		_CbURC_CALL_READY,          NULL,       "Call Ready",				NULL	},
    {_SIM_URC_CALLING, 		    _CbURC_CALLING,             NULL,       "+CLIP:",                   NULL    },
    {_SIM_URC_SMS_CONTENT, 		_CbURC_SMS_CONTENT,         NULL,       "+CMGL:",                   NULL	},
    {_SIM_URC_SMS_INDICATION1, 	_CbURC_SMS_IND,             NULL,       "+CMTI:",                   NULL	},
    {_SIM_URC_SMS_INDICATION2, 	_CbURC_SMS_IND,             NULL,       "+CMT:",                    NULL	},
    
	{_SIM_URC_ERROR, 			_CbURC_ERROR,               NULL,       "ERROR",					NULL	},
    {_SIM_URC_RECEIV_SERVER, 	_Cb_RECEIV_SERVER,          NULL,       "AMI/",			            NULL	},     
};


/*=============== Call back =================*/
static uint8_t at_callback_success(sData *uart_string)
{
    
        
	return 1;
}


/*
    Func: Cb Failure AT cmd
    return 
            1: 
*/
static uint8_t at_callback_failure(uint8_t Type)
{
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_AT_FAILURE); 
 
	return 1;
}



/*==============Call back at sim ===============*/

static uint8_t _CbAT_SIM_ID (sData *uart_string)
{
    //Call func Get CSQ
	Sim_Common_Get_Sim_ID (uart_string);
        
    return 1;
}



static uint8_t _CbAT_CHECK_RSSI(sData *uart_string)
{
    sData   strCheck = {(uint8_t*) aSimL506Step[_SIM_AT_CHECK_RSSI].at_response, strlen(aSimL506Step[_SIM_AT_CHECK_RSSI].at_response)};
    //Call func Get CSQ
	Sim_Common_Get_CSQ (&strCheck, uart_string);
    
    return 1;
}



static uint8_t _CbAT_GET_IMEI(sData *uart_string)
{
    Sim_Common_Get_Imei(uart_string);
    
    return 1;
}


static uint8_t _CbAT_SET_APN (sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sAPN.Data_a8, sSimInfor.sAPN.Length_u16, 1000);
    
    return 1;
}

static uint8_t _CbAT_APN_AUTH (sData *uart_string)
{
    if ((sSimInfor.sAPNUsername.Length_u16 == 0) && (sSimInfor.sAPNPassword.Length_u16 == 0))
    {
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "0", 1, 1000);
    } else
    {
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "3,\"", 3, 1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sAPNUsername.Data_a8, sSimInfor.sAPNUsername.Length_u16,1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "\",\"", 3, 1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sAPNPassword.Data_a8, sSimInfor.sAPNPassword.Length_u16,1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "\"", 1, 1000);
    }
    
    return 1;
}


static uint8_t _CbAT_APN_ACTIVE (sData *uart_string)
{

    return 1;
}




static uint8_t _CbAT_TRANPARENT_MODE(sData *uart_string)
{
    sSimCommon.DataMode_u8      = 1;
    sSimCommon.GroupStepID_u8   = _GR_INIT_TCP;
    
    return 1;
}

static uint8_t _CbAT_NON_TRANPARENT_MODE(sData *uart_string)
{
	sSimCommon.DataMode_u8      = 0;
    
    return 1;
}


static uint8_t _Cb_AT_CHECK_ATTACH(sData *str_Receive)
{
	char *p = NULL;

	p = strstr((char*) str_Receive->Data_a8, "+CGATT: 1");
	if (p != NULL)
	{
		sSimCommon.GroupStepID_u8 = _GR_SETTING;
        return 1;
	}

	return 0;
}


static uint8_t _CbAT_GET_CLOCK (sData *uart_string)
{
	sData  strCheck = {(uint8_t*) aSimL506Step[_SIM_NET_GET_RTC].at_response, strlen(aSimL506Step[_SIM_NET_GET_RTC].at_response)};
    //Call Func Get sTime BTS
    Sim_Common_Get_Stime_BTS (&strCheck, uart_string);
        
    return 1;
}



static uint8_t _CbAT_CHECK_SIM(sData *uart_string)
{
	sSimCommon.CallReady_u8 	    = 1;
    
    sSimCommon.GroupStepID_u8 = _GR_CHECK_ATTACH;
    
    return 1;
}

static uint8_t _CbAT_TCP_CLOSE_1(sData *uart_string)
{
    uint8_t aCID = CID_SERVER;
    
    Sim_Common_Send_AT_Cmd(&uart_sim, &aCID, 1, 1000);
    
    return 1;
}
static uint8_t _CbAT_GET_IP_SIM(sData *uart_string)
{
    
    return 1;
}



static uint8_t _CbAT_OPEN_TCP_1(sData *uart_string)
{
    uint8_t TypeConnnect = 0;
    
    uint8_t aTEMP_STR[10] = {"0,\"TCP\",\""};

    aTEMP_STR[0] = CID_SERVER;
    
    HAL_UART_Transmit(&uart_sim, &aTEMP_STR[0], 9, 1000);
    
    TypeConnnect = sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_GET_CONN);
        
    if (TypeConnnect == MODE_CONNECT_DATA_MAIN)
    {
        Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sServer.sIPMain.Data_a8, sSimInfor.sServer.sIPMain.Length_u16, 1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *) "\",", 2, 1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sServer.sPortMain.Data_a8, sSimInfor.sServer.sPortMain.Length_u16, 1000);
    } else 
    {
        Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sServer.sIPBackUp.Data_a8, sSimInfor.sServer.sIPBackUp.Length_u16, 1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *) "\",", 2, 1000);
        Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sServer.sPortBackUp.Data_a8, sSimInfor.sServer.sPortBackUp.Length_u16, 1000);
    }
      
    return 1;
}



static uint8_t _CbAT_OPEN_TCP_2 (sData *uart_string)
{
    int Posfix = 0;
    
#ifdef USING_TRANSPARENT_MODE
    sData StConnectOk = {(uint8_t*)"CONNECT \r\n",10};
#else
    sData StConnectOk = {(uint8_t*)"+CIPOPEN:SUCCESS", 16}; 
#endif
    Posfix = Find_String_V2((sData*) &StConnectOk, uart_string);
	if (Posfix >= 0)
    {
        sSimCommon.GroupStepID_u8 = _GR_SEND_MQTT;
        return 1;
    }

    return 0;
}

static uint8_t _CbAT_TCP_SEND_1 (sData *uart_string)
{
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_TCP_SEND_1);
    
    return 1;
}


static uint8_t _CbAT_TCP_SEND_2 (sData *uart_string)
{
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_TCP_SEND_2);
    
    return 1;
}


static uint8_t _Cb_MQTT_CONNECT_1(sData *str_Receiv)
{
    //Gui sang sim chuoi mqtt connect
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_CONN_MQTT_1);
        
	return 1;
}

static uint8_t _Cb_MQTT_CONNECT_2(sData *str_Receiv)
{
    //Neu connect OK:
//    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_CONN_MQTT_2);
        
	return 1;
}

static uint8_t _Cb_MQTT_SUBCRIBE_1(sData *str_Receiv)
{
    //Truyen chuoi Subcribe
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_SUB_MQTT_1);
        
	return 1;
}

static uint8_t _Cb_MQTT_SUBCRIBE_2(sData *str_Receiv)
{
    sSimCommon.PowerStatus_u8 = _POWER_CONN_MQTT;
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_SUB_MQTT_2);
      
	return 1;
}

static uint8_t _Cb_MQTT_PUBLISH_1(sData *str_Receiv)
{
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_PUB_MQTT_1);
    
	return 1;
}

/*
    Fun: Handler Publish success
        TSVH:   + Cong IndexSend vï¿½ CountPacket
                + Neu nhu IndexSend == IndexSave thï¿½ clear Mess
        Other Mess:
                + Clear Mess

*/
static uint8_t _Cb_MQTT_PUBLISH_2(sData *str_Receiv)
{
    //Neu Publish OK ->Clear Mark_Mess_Pending
    sSimCommon.PowerStatus_u8 = _POWER_CONN_MQTT;
    sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_PUB_MQTT_2);
    
	return 1;
}


static uint8_t _CbAT_PING_TEST (sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimFwUpdate.StrURL_FirmIP.Data_a8, sSimFwUpdate.StrURL_FirmIP.Length_u16, 1000);
    
    return 1;
}


/*=============== Call back URC =================*/
static uint8_t _CbURC_RESET_SIM900 (sData *uart_string)
{     
    Sim_Common_URC_Reset();
    
    return 1;
}


static uint8_t _CbURC_SIM_LOST (sData *uart_string)
{
    Sim_Common_URC_Lost_Card();
    
    return 1;
}


//+SERVER DISCONNECTED:0
static uint8_t _CbURC_CLOSED (sData *uart_string)
{    
    //Kiem tra xem disconnect cua server chính k
    int Pos_Str = -1;
    
    sData strCheck = {(uint8_t*) aSimUrc[_SIM_URC_CLOSED].at_response, strlen(aSimUrc[_SIM_URC_CLOSED].at_response)};
    
	Pos_Str = Find_String_V2((sData*) &strCheck, uart_string);
    if (Pos_Str >= 0)
    {
        Pos_Str += strlen(aSimUrc[_SIM_URC_CLOSED].at_response);
        //check vi tri tiep theo la CID disconnect
        if (*(uart_string->Data_a8 + Pos_Str + 1) == CID_SERVER)
        {
            Sim_Common_URC_Closed();
        }
    }
 
    return 1;
}

static uint8_t _CbURC_CALL_READY (sData *uart_string)
{
    sSimCommon.CallReady_u8 = TRUE;
    
    return 1;
}


static uint8_t _CbURC_CALLING (sData *uart_string)
{
    Sim_Common_URC_Calling();
    return 1;
}


static uint8_t _CbURC_SMS_CONTENT (sData *uart_string)
{
    Sim_Common_URC_Sms_Content();
    
    return 1;
}

static uint8_t _CbURC_SMS_IND (sData *uart_string)
{
    Sim_Common_URC_Sms_Indication();
    
    return 1;
}




static uint8_t _CbURC_ERROR (sData *uart_string)
{
    Sim_Common_URC_Error();
 
    return 1;
}
  
      
      
static uint8_t _Cb_RECEIV_SERVER(sData *str_Receive)
{
    Sim_Common_URC_Recv_Server();
    
    return 1;
}

 

/*================== Call back File SYS, FTP va HTTP====================*/
//File sys callback
static uint8_t _CbSYS_DEL_FILE(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *)&sFileSys.aNAME[1], sFileSys.LengthName_u8 - 1, 1000);
    
    return 1;
}


//FTP Callback
static uint8_t _CbFTP_SEND_IP(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimFwUpdate.StrURL_FirmIP.Data_a8, sSimFwUpdate.StrURL_FirmIP.Length_u16, 1000);
    
    return 1;
}



static uint8_t _CbFTP_GET_FILE1(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimFwUpdate.StrURL_FirmPath.Data_a8, sSimFwUpdate.StrURL_FirmPath.Length_u16, 1000);
    
    if (sSimFwUpdate.StrURL_FirmPath.Length_u16 != 0) 
        Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "/",1,100);
    
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimFwUpdate.StrURL_FirmName.Data_a8, sSimFwUpdate.StrURL_FirmName.Length_u16, 1000); 

    return 1;
}

static uint8_t _CbFTP_GET_FILE2(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *)&sFileSys.aNAME[1], sFileSys.LengthName_u8 - 1, 1000);

    return 1;
}

static uint8_t _CbFTP_GET_FILE3(sData *uart_string)
{
//    int16_t     index =0;
//    int16_t     Pos_Str = -1;
//    uint8_t     statusM = 0;
//    uint8_t     dataMemory[8] = {0};
//    uint8_t     count = 0;
//    sData    strCheck = {(uint8_t*) aSimL506Step[_SIM_FTP_GET_FILE3].at_response, strlen(aSimL506Step[_SIM_FTP_GET_FILE3].at_response)};
//    
//	Pos_Str = Find_String_V2((sData*) &strCheck, uart_string);
//    if (Pos_Str >= 0)
//    {
//        for (index = Pos_Str; index < uart_string->Length_u16; index++)
//        {
//            if (*(uart_string->Data_a8 + index) == ',')
//                statusM = 1;
//            
//            if (*(uart_string->Data_a8 + (index + 1)) == '\r')
//            {
//                statusM = 0;
//                break;
//            }
//            if (statusM == 1)
//            {
//                dataMemory[count] = *(uart_string->Data_a8 + (index + 1));
//                count++;
//            }
//            if (count == 8)
//                break;
//        }
//        sSimFwUpdate.FirmLength_u32 = Convert_String_To_Dec(dataMemory, count);
//    }
    
    return 1;
}



static uint8_t _CbFTP_READ_1(sData *uart_string)
{
//    uint8_t     aNumbyte[10] = {0};
//    uint8_t     aLengread[10] = {0};
//    sData    strPos      = {&aNumbyte[0], 0};
//    sData    strlenread  = {&aLengread[0], 0};
//    //chuyen offset ra string de truyen vao sim
//    Convert_Uint64_To_StringDec(&strPos, sSimFwUpdate.CountByteTotal_u32, 0);
//    
//    if (sSimFwUpdate.FirmLength_u32 >= (sSimFwUpdate.CountByteTotal_u32 + 1024))
//        Convert_Uint64_To_StringDec(&strlenread, 1024, 0);
//    else 
//        Convert_Uint64_To_StringDec(&strlenread, (sSimFwUpdate.FirmLength_u32 - sSimFwUpdate.CountByteTotal_u32), 0);
//      
//    Sim_Common_Send_AT_Cmd(&uart_sim,(uint8_t*) &aFIRM_FILE_NAME[0], LENGTH_FIRM_NAME, 100);
//    Sim_Common_Send_AT_Cmd(&uart_sim,(uint8_t*) "\",", 2,100);
//    Sim_Common_Send_AT_Cmd(&uart_sim,(uint8_t*) strPos.Data_a8, strPos.Length_u16,100);
//    Sim_Common_Send_AT_Cmd(&uart_sim,(uint8_t*) ",", 1,100);
//    Sim_Common_Send_AT_Cmd(&uart_sim,(uint8_t*) strlenread.Data_a8, strlenread.Length_u16,100);
    
    return 1;
}

static uint8_t _CbFTP_READ_2(sData *uart_string)
{
    //Tuong tu nhu Http Read
    
    return 1;
}

//Callback HTTP

static uint8_t _CbHTTP_SETURL_1(sData *uart_string)
{   
    Sim_Common_Send_AT_Cmd(&uart_sim,sSimFwUpdate.StrURL_HTTP.Data_a8,sSimFwUpdate.StrURL_HTTP.Length_u16,1000);
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) ",", 1,1000);
    Sim_Common_Send_AT_Cmd(&uart_sim,sSimFwUpdate.StrURL_FirmPort.Data_a8,sSimFwUpdate.StrURL_FirmPort.Length_u16,1000);
    
    return 1;
}

static uint8_t _CbHTTP_SETURL_2(sData *uart_string)
{
    return 1;
}


static uint8_t _CbHTTP_FIRM_LENGTH(sData *uart_string)
{
    int16_t     index =0;
    int16_t     Pos_Str = -1;
    uint8_t     statusM = 0;
    uint8_t     dataMemory[8] = {0};
    uint8_t     count = 0;
    sData       strCheck = {(uint8_t*) aSimL506Step[_SIM_HTTP_LENGTH].at_response, strlen(aSimL506Step[_SIM_HTTP_LENGTH].at_response)};
    
	Pos_Str = Find_String_V2((sData*) &strCheck, uart_string);
    if (Pos_Str >= 0)
    {
        for (index = Pos_Str; index < uart_string->Length_u16; index++)
        {
            if (*(uart_string->Data_a8 + index) == ',')
                statusM = 1;

            if (*(uart_string->Data_a8 + (index + 1)) == '\r')
            {
                statusM = 0;
                break;
            }
            if (statusM == 1)
            {
                dataMemory[count] = *(uart_string->Data_a8 + (index + 1));
                count++;
            }
            if (count == 8)
                break;
        }
        sSimFwUpdate.FirmLength_u32 = Convert_String_To_Dec(dataMemory, count);
    }      

    sSimCommon.GroupStepID_u8 = _GR_READ_HTTP; 
    //Init Struct Update Fw
    Sim_Common_Init_UpdateFw();
    
    return 1;
}

static uint8_t _CbHTTP_READ_1(sData *uart_string)
{
    uint8_t  aOFFSET[10] = {0};
    sData    strOffset = {&aOFFSET[0], 0};
    //chuyen offset ra string de truyen vao sim
    Convert_Uint64_To_StringDec(&strOffset, sSimFwUpdate.AddOffset_u32, 0);

    Sim_Common_Send_AT_Cmd(&uart_sim, strOffset.Data_a8, strOffset.Length_u16,1000);

    return 1;
}



static uint8_t _CbHTTP_READ_2(sData *uart_string)
{
    return Sim_Common_Http_Read_Data (uart_string);
}



/*
    Func: Check Step Long timeout
        + Step CONNECT TCP 
        + STEP CONN MQTT
        + STEP PUB, SUB
        + STEP In HTTP process
*/

uint8_t L506_Check_Step_Long_Timeout (uint8_t sim_step)
{
    if ((sim_step == _SIM_TCP_CONNECT_2) || (sim_step == _SIM_MQTT_CONN_2) || (sim_step == _SIM_MQTT_SUB_2)
        || ((sim_step >= _SIM_MQTT_PUB_2) && (sim_step <= _SIM_HTTP_LENGTH)))
        return 1;
    
    return 0;    
}


sCommand_Sim_Struct * L506_Get_AT_Cmd (uint8_t step)
{
    return (sCommand_Sim_Struct *) &aSimL506Step[step];
}


uint32_t L506_Get_Delay_2_AT (uint8_t Step)
{
    uint32_t Time = 0;
    
    switch (Step)
    {
        case _SIM_TCP_OUT_DATAMODE:
            Time = 1000;
            break;
        case _SIM_TCP_TRANS:
        case _SIM_HTTP_READ_2:
        case _SIM_FTP_READ_2:
            Time = 100;
            break;
        default:
            break;
    }
    
    return Time;
}
                          
/*
    Func: Check step allow check URC
        + 1: Allow
        + 0: Not allow
*/
uint8_t L506_Is_Step_Check_URC (uint8_t sim_step)
{
    if ((sim_step != _SIM_FTP_READ_1) && (sim_step != _SIM_FTP_READ_2)
        && (sim_step != _SIM_HTTP_READ_1) && (sim_step != _SIM_HTTP_READ_2)) 
    {
        return TRUE;
    }
    
    return FALSE;
}


uint8_t L506_Check_Step_Skip_Error (uint8_t step)
{
    
    return false;
}
