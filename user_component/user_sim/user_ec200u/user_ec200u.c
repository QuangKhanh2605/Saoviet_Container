/*
 * siml506.c
 *
 *  Created on: Feb 16, 2022
 *      Author: chien
 */


#include "string.h"

#include "user_ec200u.h"
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
static uint8_t _CbSYS_FILE_OPEN_1(sData *uart_string);
static uint8_t _CbSYS_FILE_OPEN_2(sData *uart_string);
static uint8_t _CbSYS_FILE_CLOSE_1(sData *uart_string);
static uint8_t _CbSYS_FILE_SEEK_1(sData *uart_string);
static uint8_t _CbSYS_FILE_POSITION_1(sData *uart_string);
static uint8_t _CbSYS_FILE_READ_1(sData *uart_string);
static uint8_t _CbSYS_FILE_READ_2(sData *uart_string);

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

static uint8_t _CbHTTP_GET_REQUEST(sData *uart_string);
static uint8_t _CbHTTP_STORE_FILE1(sData *uart_string);

static uint8_t _CbHTTP_READ_1(sData *uart_string);
static uint8_t _CbHTTP_READ_2(sData *uart_string);

//GPS callback
static uint8_t _Cb_GPS_POSITION_INF(sData *uart_string);

/*============= Var struct ====================*/
//Block step control SIM
uint8_t aSimStepBlockInit[13] = 
{
	_SIM_AT_ECHO,               //0
    _SIM_AT_ECHO,               //1
	_SIM_AT_AT,                 //2
	_SIM_NET_CEREG,             //3
	_SIM_AT_BAUD_RATE,          //4
	_SIM_AT_GET_ID,             //5
	_SIM_AT_CHECK_RSSI,         //6
	_SIM_AT_GET_IMEI,           //7
    _SIM_AT_DTR_MODE,           //8
	_SIM_AT_CHECK_SIM,          //9
    _SIM_CALLING_LINE,          //10
    _SIM_SMS_FORMAT_TEXT,       //11    
    _SIM_SMS_NEW_MESS_IND,      //12
};

  


uint8_t aSimStepBlockNework[10] = 
{
	_SIM_NET_CHECK_ATTACH,      //0
    _SIM_SMS_READ_UNREAD,       //1
    _SIM_SMS_DEL_ALL,           //2
    _SIM_AT_CHECK_RSSI,         //3
	_SIM_NET_GET_RTC,           //4
	_SIM_AT_SET_APN_1,          //5
    _SIM_AT_SET_APN_2,          //6
    _SIM_GPS_TURN_ON,           //7
    _SIM_AT_CHECK_RSSI,         //8
    _SIM_GPS_POSTION_INF,       //9
    
//    _SIM_GPS_CFG_TOKEN,         //7
//    _SIM_GPS_QUERY_TOKEN,       //8
//    _SIM_GPS_CFG_FORMAT,        //9
//    _SIM_GPS_GET_LOCATION,      //10    
//    _SIM_GPS_TURN_OFF,          //11   
};
   

uint8_t aSimStepBlockLocation[2] = 
{
    _SIM_AT_CHECK_RSSI,
    _SIM_GPS_POSTION_INF,
//    _SIM_GPS_GET_LOCATION,
};

uint8_t aSimStepBlockDelLocation[1] = 
{
    _SIM_GPS_DEL_CUR_DATA,
};


#ifdef USING_TRANSPARENT_MODE
    uint8_t aSimStepBlockConnect[7] = 
    {
        _SIM_TCP_IP_SIM,     //0
        _SIM_TCP_CONNECT_1,  //1
        _SIM_TCP_CONNECT_2,  //2
        
        _SIM_MQTT_CONN_1,        //3
        _SIM_MQTT_CONN_2,        //4

        _SIM_MQTT_SUB_1,         //5
        _SIM_MQTT_SUB_2,         //6
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
    uint8_t aSimStepBlockConnect[11] = 
    {
        _SIM_TCP_CONNECT_1,     //0
        _SIM_TCP_CONNECT_2,     //1
        _SIM_TCP_IP_SIM,        //2
        
        _SIM_MQTT_CONN_1,       //3
        _SIM_TCP_SEND_CMD_1,    //4
        _SIM_TCP_SEND_CMD_2,    //5
        _SIM_MQTT_CONN_2,       //6

        _SIM_MQTT_SUB_1,        //7
        _SIM_TCP_SEND_CMD_1,    //8
        _SIM_TCP_SEND_CMD_2,    //9
        _SIM_MQTT_SUB_2,        //10
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
    _SIM_TCP_IP_SIM,
	_SIM_TCP_CLOSE_1,
    _SIM_TCP_CLOSE_2,
};

uint8_t aSimStepBlockGetClock[3] = 
{
    _SIM_TCP_OUT_DATAMODE,
    _SIM_NET_GET_RTC,
    _SIM_TCP_IN_DATAMODE,
};

uint8_t aSimStepBlockHttpInit[19] = 
{
    _SIM_NET_CHECK_ATTACH,      //0
    _SIM_AT_CHECK_RSSI,         //1
    _SIM_AT_SET_APN_1,          //2
    _SIM_AT_SET_APN_2,          //3
//    _SIM_AT_APN_AUTH_1,
//    _SIM_AT_APN_AUTH_2,
    _SIM_AT_TEST_0,             //4
    _SIM_AT_ACTIVE_APN,         //5
    
    _SIM_HTTP_CFG_CONTEXT,      //6
    _SIM_HTTP_CFG_HEADER2,      //7
    _SIM_HTTP_SET_URL_1,        //8
    _SIM_HTTP_SET_URL_2,        //9
    _SIM_HTTP_SET_URL_3,        //10
         
    _SIM_HTTP_REQUEST_GET,      //11
    _SIM_HTTP_STORE_FILE1,      //12
    _SIM_HTTP_STORE_FILE2,      //13
        
    _SIM_SYS_FILE_LIST,         //14
    _SIM_SYS_FILE_OPEN1,        //15
    _SIM_SYS_FILE_OPEN2,        //16
    
    _SIM_SYS_FILE_SEEK1,        //17
    _SIM_SYS_FILE_SEEK2,        //18
};




uint8_t aSimStepBlockHttpRead[2] = 
{
    _SIM_SYS_FILE_READ1,
    _SIM_SYS_FILE_READ2,

//    _SIM_SYS_FILE_POS1,
//    _SIM_SYS_FILE_POS2,
};



//Read SMS
uint8_t aSimStepBlockSMS[4] = 
{
    _SIM_TCP_OUT_DATAMODE,
    _SIM_SMS_READ_UNREAD,
    _SIM_SMS_DEL_ALL,
    _SIM_TCP_IN_DATAMODE,
};  

uint8_t aSimStepBlockFileClose[2] = 
{
    _SIM_SYS_FILE_CLOSE1,
    _SIM_SYS_FILE_CLOSE2,
};
    


/*========= Struct List AT Sim ==================*/
static char DataConack_8a[] = { MY_CONNACK, 0x02, 0x00 };		 //20020000
static char DataSuback_8a[] = { MY_SUBACK, 0x04, 0x00};          //9004000a00
static char DataPuback_8a[] = { MY_PUBACK, 0x02, 0x00};	         //40020001  , 0x1A 

    
const sCommand_Sim_Struct aSimEC200UStep[] =
{
    //Cmd Pre Init
	{	_SIM_AT_AT,				    at_callback_success,        at_callback_failure,	    "OK",                   "AT\r"              },
	{	_SIM_AT_ECHO,				at_callback_success,        at_callback_failure,        "OK", 		            "ATE0\r" 	 	    },
    //Cmd Init
    {   _SIM_AT_BAUD_RATE, 			at_callback_success,        at_callback_failure,        "OK",                   "AT+IPR=115200\r"	},
    {	_SIM_AT_GET_ID,			    _CbAT_SIM_ID,               at_callback_failure,        "CCID: ",               "AT+QCCID\r"     	},
	{	_SIM_AT_CHECK_RSSI,			_CbAT_CHECK_RSSI,           at_callback_failure,        "+CSQ: ",               "AT+CSQ\r\n" 	 	},
	{   _SIM_AT_GET_IMEI, 		    _CbAT_GET_IMEI,             at_callback_failure,        "OK",                   "AT+CGSN=1\r"		},
	{	_SIM_AT_CHECK_SIM,			_CbAT_CHECK_SIM,            at_callback_failure,        "OK",                   "AT+CPIN?\r"  	    },
    {	_SIM_AT_SET_APN_1,      	_CbAT_SET_APN,              at_callback_failure,        NULL,                   "AT+CGDCONT=1,\"IP\",\""		}, 
	{	_SIM_AT_SET_APN_2,          at_callback_success,        at_callback_failure,        "OK",                   "\"\r"         		}, 
    {	_SIM_AT_APN_AUTH_1,	        _CbAT_APN_AUTH,             at_callback_failure,        NULL,                   "AT+QICSGP=1,1,\""	},      // "AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r" 
    {   _SIM_AT_APN_AUTH_2,         at_callback_success,        at_callback_failure,        "OK",                   ",1\r" 	        },
    
    {   _SIM_AT_DTR_MODE, 	        at_callback_success,        at_callback_failure,        "OK",                   "AT&D1\r"			},
    {   _SIM_AT_TEST_0, 		    at_callback_success,        at_callback_failure,        "OK",                   "AT+CGATT=1\r"	   	},
    {   _SIM_AT_GET_IP_APN,         _CbAT_APN_ACTIVE,           at_callback_failure,        "+QIACT: 1,",           "AT+QIACT?\r"	},
    {   _SIM_AT_ACTIVE_APN, 	    at_callback_success,        at_callback_failure,        "OK",                   "AT+QIACT=1\r"  	},
   //SMS
    {   _SIM_SMS_FORMAT_TEXT,	    at_callback_success,        at_callback_failure,        "OK",                   "AT+CMGF=1\r"       },  
    {   _SIM_SMS_NEW_MESS_IND,	    at_callback_success,        at_callback_failure,        "OK",                   "AT+CNMI=2,1\r"     },
    {   _SIM_SMS_READ_UNREAD,  	    at_callback_success,        at_callback_failure,        "\r\nOK",               "AT+CMGL=\"REC UNREAD\"\r"      },
    {   _SIM_SMS_READ_ALL,  	    at_callback_success,        at_callback_failure,        "\r\nOK",               "AT+CMGL=\"ALL\"\r" },
    {   _SIM_SMS_DEL_ALL,	        at_callback_success,        at_callback_failure,        "OK",                   "AT+CMGD=1,4\r"     },
    //Calling
    {   _SIM_CALLING_LINE, 		    at_callback_success,        at_callback_failure,        "OK",                   "AT+CLIP=1\r"       },
    //Cmd Check Network
	{	_SIM_NET_CEREG, 			at_callback_success,        at_callback_failure,        "OK",	                "AT+CEREG=0\r"		},
	{	_SIM_NET_CHECK_ATTACH,	    _Cb_AT_CHECK_ATTACH,        at_callback_failure,        "OK",                   "AT+CGATT?\r"		},
    //Cmd clock
	{	_SIM_NET_GET_RTC, 			_CbAT_GET_CLOCK,            at_callback_failure,        "+CCLK:",	            "AT+CCLK?\r"		},
    //Cmd Data mode and command mode
#ifdef USING_TRANSPARENT_MODE
    {   _SIM_TCP_OUT_DATAMODE, 	    _CbAT_NON_TRANPARENT_MODE,  at_callback_failure,        NULL,                   "+++"			  	}, 
    {   _SIM_TCP_IN_DATAMODE,       _CbAT_TRANPARENT_MODE,      at_callback_failure,        "CONNECT",              "ATO\r"		        },
#else
    {   _SIM_TCP_OUT_DATAMODE, 	    _CbAT_NON_TRANPARENT_MODE,  at_callback_failure,        NULL, 			        NULL	            }, 
    {   _SIM_TCP_IN_DATAMODE,       _CbAT_TRANPARENT_MODE,      at_callback_failure,        NULL,		            NULL     	        },
#endif   
    //Cmd TCP 
    {   _SIM_TCP_IP_SIM,	        _CbAT_GET_IP_SIM,           at_callback_failure,        "OK",                   "AT+QISTATE?\r"  },         
	{	_SIM_TCP_CLOSE_1,	 	    _CbAT_TCP_CLOSE_1,          at_callback_failure,        NULL,	                "AT+QICLOSE="  	},
    {	_SIM_TCP_CLOSE_2,	 	    at_callback_success,        at_callback_failure,        "OK",	                "\r"  	}, 
  
    {	_SIM_TCP_CONNECT_1,	        _CbAT_OPEN_TCP_1,           at_callback_failure,        NULL,                   "AT+QIOPEN=1,"     	},
#ifdef USING_TRANSPARENT_MODE
    {	_SIM_TCP_CONNECT_2,	        _CbAT_OPEN_TCP_2,           at_callback_failure,        "CONNECT",              ",0,2\r"            },   // Tranparent; AT+QIOPEN=1,0,"TCP","220.180.239.212",8009,0,2
#else
    {	_SIM_TCP_CONNECT_2,	        _CbAT_OPEN_TCP_2,           at_callback_failure,        "+QIOPEN:",             ",0,1\r"            },   // Push mode: AT+QIOPEN=1,0,"TCP","220.180.239.212",8009,0,1 
#endif        
    {   _SIM_TCP_SEND_CMD_1, 	    _CbAT_TCP_SEND_1,           at_callback_failure,        NULL,                   "AT+QISEND="      	},
    {   _SIM_TCP_SEND_CMD_2,        _CbAT_TCP_SEND_2,           at_callback_failure,        ">",                    "\r"	 		   	},
    {   _SIM_TCP_SEND_CMD_3, 	    _Cb_MQTT_PUBLISH_2,         at_callback_failure,        "SEND OK",	            NULL            	},
    
    //Mqtt AT
	{   _SIM_MQTT_CONN_1, 		    _Cb_MQTT_CONNECT_1,         at_callback_failure,        NULL,	                NULL   	            },
	{   _SIM_MQTT_CONN_2, 		    _Cb_MQTT_CONNECT_2,         at_callback_failure,        DataConack_8a,          NULL	    		},  //4,"20020000"

	{   _SIM_MQTT_SUB_1, 		    _Cb_MQTT_SUBCRIBE_1,        at_callback_failure,        NULL,	                NULL                },
	{   _SIM_MQTT_SUB_2, 		    _Cb_MQTT_SUBCRIBE_2,        at_callback_failure,        DataSuback_8a,          NULL       			},

	{   _SIM_MQTT_PUB_1, 		    _Cb_MQTT_PUBLISH_1,         at_callback_failure,        NULL,				    NULL             	},
	{   _SIM_MQTT_PUB_2, 		    _Cb_MQTT_PUBLISH_2,         at_callback_failure,        NULL,	   			    NULL  		     	},  //"4,40020001"  "SEND OK"
    
	{   _SIM_MQTT_PUB_FB_1, 	    _Cb_MQTT_PUBLISH_1,         at_callback_failure,        NULL,				    NULL             	},
	{   _SIM_MQTT_PUB_FB_2, 	    _Cb_MQTT_PUBLISH_2,         at_callback_failure,        DataPuback_8a,          NULL       	        },  // 4,"40020001"
    //Ping
    {   _SIM_TCP_PING_TEST1,        _CbAT_PING_TEST,            at_callback_failure,        NULL,                   "AT+QPING=1,\""    	},    
    {   _SIM_TCP_PING_TEST2,        at_callback_success,        at_callback_failure,        "+QPING: 0,4",          "\",1,4\r"        	},
    
    // File	 
    {   _SIM_SYS_FILE_DEL1,         _CbSYS_DEL_FILE,            at_callback_failure,        NULL,                   "AT+QFDEL=\""     	},
    {   _SIM_SYS_FILE_DEL2,         at_callback_success,        at_callback_failure,        "OK",                   "\"\r"          	},
    
    {   _SIM_SYS_FILE_DEL_ALL,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QFDEL=\"*\"\r"  }, 
    {   _SIM_SYS_FILE_LIST,         at_callback_success,        at_callback_failure,        "OK",                   "AT+QFLST=\"*\"\r"  },    
    
    {   _SIM_SYS_FILE_OPEN1,        _CbSYS_FILE_OPEN_1,         at_callback_failure,        NULL,                   "AT+QFOPEN=\""  },    
    {   _SIM_SYS_FILE_OPEN2,        _CbSYS_FILE_OPEN_2,         at_callback_failure,        "+QFOPEN:",              "\",2\r"  },                //mode 2: readonly
        
    {   _SIM_SYS_FILE_CLOSE1,       _CbSYS_FILE_CLOSE_1,        at_callback_failure,        NULL,                   "AT+QFCLOSE="  },    
    {   _SIM_SYS_FILE_CLOSE2,       at_callback_success,        at_callback_failure,        "OK",                   "\r"  },   
    
    {   _SIM_SYS_FILE_SEEK1,        _CbSYS_FILE_SEEK_1,         at_callback_failure,        NULL,                   "AT+QFSEEK="  },    
    {   _SIM_SYS_FILE_SEEK2,        at_callback_success,        at_callback_failure,        "OK",                   ",0,0\r"  },   
    
    {   _SIM_SYS_FILE_POS1,        _CbSYS_FILE_POSITION_1,     at_callback_failure,        NULL,                    "AT+QFPOSITION="  },   
    {   _SIM_SYS_FILE_POS2,        at_callback_success,        at_callback_failure,        "OK",                    "\r"  },   
    
    {   _SIM_SYS_FILE_READ1,        _CbSYS_FILE_READ_1,         at_callback_failure,        NULL,                   "AT+QFREAD="  },    
    {   _SIM_SYS_FILE_READ2,        _CbSYS_FILE_READ_2,         at_callback_failure,        "\r\nOK\r\n",           ",1024\r"  },   
    // FTP		
    {   _SIM_FTP_SERVER_1, 	        _CbFTP_SEND_IP,             at_callback_failure,        NULL,                   "AT+CFTPSERV=\""   	},
    {   _SIM_FTP_SERVER_2, 	        at_callback_success,        at_callback_failure,        "OK",                   "\"\r"            	},
    {   _SIM_FTP_PORT,              at_callback_success,        at_callback_failure,        "OK",                   "AT+CFTPPORT=21\r" 	},    
    
    {   _SIM_FTP_USERNAME, 	        at_callback_success,        at_callback_failure,        "OK",                   "AT+CFTPUN=\"admin\"\r"		},
    {   _SIM_FTP_PASSWORD, 	        at_callback_success,        at_callback_failure,        "OK",                   "AT+CFTPPW=\"admin\"\r"		},
    {   _SIM_FTP_TYPE, 		        at_callback_success,        at_callback_failure,        "OK",                   "AT+CFTPTYPE=\"I\"\r"		},
    
    {   _SIM_FTP_LIST_FILE,         at_callback_success,        at_callback_failure,        "+CFTPLIST:SUCCESS",    "AT+CFTPLIST=\"/\"\r"   	},
    {   _SIM_FTP_GET_FILE_1,        at_callback_success,        at_callback_failure,        "+CFTPGET:",            "AT+CFTPGET=\"/Test/FIRM_HCMCHES2.bin\",10\r"  	},
    
    {   _SIM_FTP_GET_FILE1,         _CbFTP_GET_FILE1,           at_callback_failure,        NULL,                   "AT+CFTPGETFILE=\"/"       	},
    {   _SIM_FTP_GET_FILE2,         _CbFTP_GET_FILE2,           at_callback_failure,        NULL,                   "\",\""           	},
    {   _SIM_FTP_GET_FILE3,         _CbFTP_GET_FILE3,           at_callback_failure,        "+CFTPGETFILE:SUCCESS", "\",0\r"          	},
    
    {   _SIM_FTP_READ_1,            _CbFTP_READ_1,              at_callback_failure,        NULL,                   "AT+CFTPRDFILE=\"" 	},    
    {   _SIM_FTP_READ_2,            _CbFTP_READ_2,              at_callback_failure,        "+CFTPRDFILE:SUCCESS",  "\r"               	},
    
    // HTTP	
    {   _SIM_HTTP_CFG_CONTEXT,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QHTTPCFG=\"contextid\",1\r"   	},
    {   _SIM_HTTP_CFG_HEADER1,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QHTTPCFG=\"requestheader\",0\r"  	},
    {   _SIM_HTTP_CFG_HEADER2,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QHTTPCFG=\"responseheader\",0\r"   	},
    {   _SIM_HTTP_CFG_SSL,          at_callback_success,        at_callback_failure,        "OK",                   "AT+QHTTPCFG=\"sslctxid\",1\r"     	},
    
    {   _SIM_HTTP_SET_URL_1,        _CbHTTP_SETURL_1,           at_callback_failure,        NULL,                   "AT+QHTTPURL="    	},
    {   _SIM_HTTP_SET_URL_2,        _CbHTTP_SETURL_2,           at_callback_failure,        "CONNECT",              ",60\r"             },
    {   _SIM_HTTP_SET_URL_3,        at_callback_success,        at_callback_failure,        "OK",                   NULL                },
    
    {   _SIM_HTTP_REQUEST_GET,      _CbHTTP_GET_REQUEST,        at_callback_failure,        "+QHTTPGET:",           "AT+QHTTPGET=80\r"  	},  
    {   _SIM_HTTP_STORE_FILE1,      _CbHTTP_STORE_FILE1,        at_callback_failure,        NULL,                   "AT+QHTTPREADFILE=\"UFS:"  	},
    {   _SIM_HTTP_STORE_FILE2,      at_callback_success,        at_callback_failure,        "+QHTTPREADFILE: 0",    "\",80\r"  	},
    
    {   _SIM_HTTP_READ_1,           _CbHTTP_READ_1,             at_callback_failure,        NULL,                   "AT$HTTPREAD=0,"    },
    {   _SIM_HTTP_READ_2,           _CbHTTP_READ_2,             at_callback_failure,        "\r\nOK\r\n",           "\r",               },
    //GPS: GLOCA
    {   _SIM_GPS_CFG_TOKEN,         at_callback_success,        at_callback_failure,        "OK",                   "AT+QLBSCFG=\"token\",\"V7Sk323082828KtB\"\r"    	},
    {   _SIM_GPS_CFG_FORMAT,        at_callback_success,        at_callback_failure,        "OK",                   "AT+QLBSCFG=\"latorder\",1\r"   },
    {   _SIM_GPS_QUERY_TOKEN,       at_callback_success,        at_callback_failure,        "OK",                   "AT+QLBSCFG=\"token\"\r"    	},
    {   _SIM_GPS_GET_LOCATION,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QLBS=0\r"       }, 
    //GPS: GNSS
    {   _SIM_GPS_CFG_OUT_PORT,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QGPSCFG=\"outport\",\"usbnmea\"\r"  }, // "AT+QGPSCFG=\"outport\",\"usbnmea\"\r"
    {   _SIM_GPS_CFG_AUTO_RUN,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QGPSCFG=\"autogps\",1\r"   }, 
    {   _SIM_GPS_DEL_CUR_DATA,      at_callback_success,        at_callback_failure,        "OK",                   "AT+QGPSDEL=1\r"   },
    {   _SIM_GPS_TURN_ON,           at_callback_success,        at_callback_failure,        "OK",                   "AT+QGPS=1\r"    	},
    {   _SIM_GPS_POSTION_INF,       _Cb_GPS_POSITION_INF,       at_callback_failure,        "OK",                   "AT+QGPSLOC=2\r"    },
    {   _SIM_GPS_TURN_OFF,          at_callback_success,        at_callback_failure,        "OK",                   "AT+QGPSEND\r"       },   
    
	// ...
	{	_SIM_END, 			        at_callback_success,        at_callback_failure,        NULL,		             NULL,	            },
};

      
const sCommand_Sim_Struct aSimUrc[] =
{
	{   _SIM_URC_RESET_SIM900,		_CbURC_RESET_SIM900,        NULL,       "NORMAL POWER DOWN",        NULL    },// OK
	{   _SIM_URC_SIM_LOST, 			_CbURC_SIM_LOST,            NULL,       "SIM CRASH",	            NULL 	},
	{   _SIM_URC_SIM_REMOVE, 	    _CbURC_SIM_LOST,            NULL,       "SIM REMOVED",		        NULL	},
	{   _SIM_URC_CLOSED,			_CbURC_CLOSED,              NULL,       "+QIURC: \"closed\"",       NULL	},   
	{   _SIM_URC_PDP_DEACT, 		_CbURC_CLOSED,              NULL,       "+NETWORK DISCONNECTED",    NULL	},   
	{   _SIM_URC_CALL_READY, 		_CbURC_CALL_READY,          NULL,       "Call Ready",		        NULL    },
    {   _SIM_URC_CALLING, 		    _CbURC_CALLING,             NULL,       "+CLIP:",                   NULL	},
    {   _SIM_URC_SMS_CONTENT, 		_CbURC_SMS_CONTENT,         NULL,       "+CMGL:",                   NULL	},
    {   _SIM_URC_SMS_INDICATION1, 	_CbURC_SMS_IND,             NULL,       "+CMTI:",                   NULL	},
    {   _SIM_URC_SMS_INDICATION2, 	_CbURC_SMS_IND,             NULL,       "+CMT:",                    NULL	},
    
	{   _SIM_URC_ERROR, 			_CbURC_ERROR,               NULL,       "ERROR",                    NULL	},
    {   _SIM_URC_RECEIV_SERVER, 	_Cb_RECEIV_SERVER,          NULL,       "AMI/",			            NULL    },     
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
    sData   strCheck = {(uint8_t*) aSimEC200UStep[_SIM_AT_CHECK_RSSI].at_response, strlen(aSimEC200UStep[_SIM_AT_CHECK_RSSI].at_response)};
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
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sAPN.Data_a8, sSimInfor.sAPN.Length_u16, 1000);
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "\",\"", 3, 1000);
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sAPNUsername.Data_a8, sSimInfor.sAPNUsername.Length_u16,1000);
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "\",\"", 3, 1000);
    Sim_Common_Send_AT_Cmd(&uart_sim, sSimInfor.sAPNPassword.Data_a8, sSimInfor.sAPNPassword.Length_u16,1000);
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t*) "\"", 1, 1000);
    
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
        sSimCommon.NetReady_u8      = TRUE;
        return 1;
	}

	return 0;
}


static uint8_t _CbAT_GET_CLOCK (sData *uart_string)
{
	sData  strCheck = {(uint8_t*) aSimEC200UStep[_SIM_NET_GET_RTC].at_response, strlen(aSimEC200UStep[_SIM_NET_GET_RTC].at_response)};
    //Call Func Get sTime BTS
    Sim_Common_Get_Stime_BTS (&strCheck, uart_string);
        
    return 1;
}



static uint8_t _CbAT_CHECK_SIM(sData *uart_string)
{
	sSimCommon.CallReady_u8     = TRUE;
    
    sSimCommon.GroupStepID_u8   = _GR_CHECK_ATTACH;
    
    return 1;
}





static uint8_t _CbAT_GET_IP_SIM(sData *uart_string)
{
    
    return 1;
}

static uint8_t _CbAT_TCP_CLOSE_1(sData *uart_string)
{
    uint8_t aCID = CID_SERVER;
    
    Sim_Common_Send_AT_Cmd(&uart_sim, &aCID, 1, 1000);
    
    return 1;
}



static uint8_t _CbAT_OPEN_TCP_1(sData *uart_string)
{
    uint8_t TypeConnnect = 0;
    
    uint8_t aTEMP_STR[10] = {"0,\"TCP\",\""};

    aTEMP_STR[0] = CID_SERVER;
    
    Sim_Common_Send_AT_Cmd(&uart_sim, &aTEMP_STR[0], 9, 1000);
    
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
    
    Posfix = Find_String_V2((sData*) &StConnectOk, uart_string);
	if (Posfix >= 0)
    {
        sSimCommon.GroupStepID_u8 = _GR_SEND_MQTT;
        return 1;
    }
#else
    sData StConnectOk = {(uint8_t*)"+QIOPEN:", 8};    // +QIOPEN: 0,0   //check so '0' phia sau

    Posfix = Find_String_V2((sData*) &StConnectOk, uart_string);
	if (Posfix >= 0)
    {
        Posfix += StConnectOk.Length_u16;
        
        if (*(uart_string->Data_a8 + Posfix + 3) == '0')
        {
            sSimCommon.GroupStepID_u8 = _GR_SEND_MQTT;
            return 1;
        }
    }
#endif
    
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
    sSimCommon.PowerStatus_u8   = _POWER_CONN_MQTT;
    sSimCommon.ServerReady_u8   = TRUE;
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
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *)&sFileSys.aNAME[0], sFileSys.LengthName_u8, 1000);
    
    return 1;
}


static uint8_t _CbSYS_FILE_OPEN_1(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *)&sFileSys.aNAME[0], sFileSys.LengthName_u8, 1000);

    return 1;
}

static uint8_t _CbSYS_FILE_OPEN_2(sData *uart_string)
{
    //get file handle: +QFOPEN: <filehandle>   +QFOPEN: 20
    int Pos_Str = -1;
    uint16_t i = 0;
    uint16_t Temp = 0;
    
    sData strCheck = {(uint8_t*) aSimEC200UStep[_SIM_SYS_FILE_OPEN2].at_response, strlen(aSimEC200UStep[_SIM_SYS_FILE_OPEN2].at_response)};
    
	Pos_Str = Find_String_V2((sData*) &strCheck, uart_string);
    if (Pos_Str >= 0)
    {
        Pos_Str+= strCheck.Length_u16 + 1;
        for (i = Pos_Str; i < uart_string->Length_u16; i++)
        {
            if (*(uart_string->Data_a8 + i) == 0x0D)
                break;
                  
            if ( (*(uart_string->Data_a8 + i) < 0x30) || (*(uart_string->Data_a8 + i) > 0x39) )
            {
                return 0;
            }
            
             Temp = Temp * 10 + *(uart_string->Data_a8 + i) - 0x30;          
        }
        //Get File Handle
        sFileSys.Handle_u32 = Temp;
        //Convert to string
        Reset_Buff(&sFileSys.strHandle);
        Convert_Uint64_To_StringDec (&sFileSys.strHandle, (uint64_t) (sFileSys.Handle_u32), 0);
    }
        
    return 1;
}


static uint8_t _CbSYS_FILE_CLOSE_1(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, sFileSys.strHandle.Data_a8, sFileSys.strHandle.Length_u16, 1000);

    return 1;
}

static uint8_t _CbSYS_FILE_SEEK_1(sData *uart_string)
{
    //<filehandle>
    Sim_Common_Send_AT_Cmd(&uart_sim, sFileSys.strHandle.Data_a8, sFileSys.strHandle.Length_u16, 1000);
    
    return 1;
}

static uint8_t _CbSYS_FILE_POSITION_1(sData *uart_string)
{
    //<filehandle>
    Sim_Common_Send_AT_Cmd(&uart_sim, sFileSys.strHandle.Data_a8, sFileSys.strHandle.Length_u16, 1000);
    
    return 1;
}
 

static uint8_t _CbSYS_FILE_READ_1(sData *uart_string)
{
    //<filehandle>[,<length>]
    Sim_Common_Send_AT_Cmd(&uart_sim, sFileSys.strHandle.Data_a8, sFileSys.strHandle.Length_u16, 1000);

    return 1;
}

/*
    Header fw: SVMM_LO_GW:SV_1_1_1,0x08004000@  
*/
static uint8_t _CbSYS_FILE_READ_2(sData *uart_string)
{
    /*
        Xu ly lay data fw ow day:
            + Lay data fw sau: CONNECT 1024\r\n va truoc \r\nOK\r\n
            + Kiem tra du 1024 byte?
            + Kiem tra header FW SV
            + Tinh crc va kiem tra
    */  
    int             Pos = 0;
    sData           strConnect      = {(uint8_t*)"\r\nCONNECT ", 10};
    sData           strHeaderFw;
    
    sData           strFix;
    uint16_t        i = 0, j = 0;
    uint16_t        NumbyteRecei = 0;
    uint8_t         aTEMP_DATA[8] = {0};
                   
    Pos = Find_String_V2((sData*) &strConnect, uart_string);
	if (Pos >= 0)
	{
        Pos += strConnect.Length_u16;
        //Get Number recv string
        for (i = Pos; i < uart_string->Length_u16; i++)
        {
            if ((*(uart_string->Data_a8 + i) < 0x30) || (*(uart_string->Data_a8 + i) > 0x39))
            {
                break;
            }
            
            NumbyteRecei = NumbyteRecei * 10 + *(uart_string->Data_a8 + i) - 0x30;
        }
        //phia tren i da tro den \r. can + 2 de tro den dau fw
        Pos = i + 2;   
        
        strFix.Data_a8 = uart_string->Data_a8 + Pos;
        strFix.Length_u16 = uart_string->Length_u16 - Pos;
        //check num byte recev
        if ( (NumbyteRecei == 0) || (NumbyteRecei > strFix.Length_u16 - 6) )    //6 byte cuoi \r\nOK\r\n
        {
            sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
            return 0;
        }
        strFix.Length_u16 = NumbyteRecei;  //lay length dung bang length thuc te
            
        sSimFwUpdate.CountByteTotal_u32 += NumbyteRecei; 
        
        //Neu la Pack dau tien, Check 32 byte header xem co chu: ModemFirmware:SV1.1,0x0800C000
        strHeaderFw.Data_a8 = sSimCommFuncCallBack->pSim_Common_Get_Fw_Version()->Data_a8;
        strHeaderFw.Length_u16 = LENGTH_HEADER_FW;
        
        if (sSimFwUpdate.CountPacket_u8 == 0)
        {
            Pos = Find_String_V2( (sData*)&strHeaderFw, &strFix );  
            if (Pos >= 0)
                sSimFwUpdate.IsFirmSaoViet_u8 = 1;
            //Lay byte cuoi cung ra: byte 32 cua Packdau tien. Sau header 31 byte
            sSimFwUpdate.LastCrcFile_u8 = *(strFix.Data_a8 + 31);  
            //Check Get Fw Sao Viet
            if (sSimFwUpdate.IsFirmSaoViet_u8 == 0)    //neu khong co header SV code thi return luon
            {
                sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                return 0;
            }
            //Fix string data
            strFix.Data_a8 += 32;
            strFix.Length_u16 -= 32;
        }
    
        //Cacal Crc
        for (i = 0; i < strFix.Length_u16; i++)
            sSimFwUpdate.LasCrcCal_u8 += *(strFix.Data_a8 + i);
                  
        //Increase Count Packet
        sSimFwUpdate.CountPacket_u8++;
        
        //Ghi data vao. Bu bao cho thanh boi cua FLASH_BYTE_WRTIE
        if ((strFix.Length_u16 % FLASH_BYTE_WRTIE) != 0)
            strFix.Length_u16 += ( FLASH_BYTE_WRTIE - (strFix.Length_u16 % FLASH_BYTE_WRTIE) );
        //write fw to Mem: Onchip flash or External mem
        for (i = 0; i < (strFix.Length_u16 / FLASH_BYTE_WRTIE); i++)
        {
            //Phai chia ra buffer rieng moi k loi
            for (j = 0; j < FLASH_BYTE_WRTIE; j++)
                aTEMP_DATA[j] = *(strFix.Data_a8 + i * FLASH_BYTE_WRTIE + j); 
            //
            if(OnchipFlashWriteData(sSimFwUpdate.AddSave_u32, &aTEMP_DATA[0], FLASH_BYTE_WRTIE) != HAL_OK)
            {
                sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                return 0;
            } else
                sSimFwUpdate.AddSave_u32 += FLASH_BYTE_WRTIE;
        }          
             
        //kiem tra xem nhan du byte chua de bao ok
        if (sSimFwUpdate.CountByteTotal_u32 >= sSimFwUpdate.FirmLength_u32)  //Binh thuong la dau == //Test de >=           
        {
            if (sSimFwUpdate.LastCrcFile_u8 == sSimFwUpdate.LasCrcCal_u8)
            {
                sSimFwUpdate.CountByteTotal_u32 += (FLASH_BYTE_WRTIE - sSimFwUpdate.CountByteTotal_u32 % FLASH_BYTE_WRTIE);
                
                sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_OK );
                return 1;
            } else
            {
                sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                return 0;
            }
        }
        
        //Continue
        sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_READ_OK );
        return 1;
    }
    //Set Fail Setup Http
    sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_SETUP;
    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
    
    return 0;
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
    Sim_Common_Send_AT_Cmd(&uart_sim, sFileSys.strHandle.Data_a8, sFileSys.strHandle.Length_u16, 1000);

    return 1;
}

static uint8_t _CbFTP_GET_FILE3(sData *uart_string)
{
//    int16_t     index =0;
//    int16_t     Pos_Str = -1;
//    uint8_t     statusM = 0;
//    uint8_t     dataMemory[8] = {0};
//    uint8_t     count = 0;
//    sData    strCheck = {(uint8_t*) aSimEC200UStep[SIM_CMD_FTP_GET_FILE3].at_response, strlen(aSimEC200UStep[SIM_CMD_FTP_GET_FILE3].at_response)};
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
    uint8_t aSTR_LENGTH_URL[5] = {0};
    sData strLenURL = {&aSTR_LENGTH_URL[0], 0};
    
    Convert_Uint64_To_StringDec (&strLenURL, (uint64_t) (sSimFwUpdate.StrURL_HTTP.Length_u16), 0);
    
    Sim_Common_Send_AT_Cmd(&uart_sim, strLenURL.Data_a8, strLenURL.Length_u16,1000);

    return 1;
}

static uint8_t _CbHTTP_SETURL_2(sData *uart_string)
{ 
    Sim_Common_Send_AT_Cmd(&uart_sim,sSimFwUpdate.StrURL_HTTP.Data_a8,sSimFwUpdate.StrURL_HTTP.Length_u16,1000);
    
    return 1;
}

//+QHTTPGET: 0,200,601710   : lay length tu phan hoi nay
static uint8_t _CbHTTP_GET_REQUEST(sData *uart_string)
{
    int16_t     index =0;
    int16_t     Pos_Str = -1;
    uint8_t     statusM = 0;
    uint8_t     dataMemory[8] = {0};
    uint8_t     count = 0;
    sData       strCheck = {(uint8_t*) aSimEC200UStep[_SIM_HTTP_REQUEST_GET].at_response, strlen(aSimEC200UStep[_SIM_HTTP_REQUEST_GET].at_response)};
    
	Pos_Str = Find_String_V2((sData*) &strCheck, uart_string);
    
    if (Pos_Str >= 0)
    {
        Pos_Str += strlen(aSimEC200UStep[_SIM_HTTP_REQUEST_GET].at_response) + 1;   
        //tro den err va check '0'
        if (*(uart_string->Data_a8 + Pos_Str) != '0' )
        {
            return 0;
        }
        
        Pos_Str += 2;  //qua dau ',' dau tien
        
        //Lay length fw
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

static uint8_t _CbHTTP_STORE_FILE1(sData *uart_string)
{
    Sim_Common_Send_AT_Cmd(&uart_sim, (uint8_t *)&sFileSys.aNAME[0], sFileSys.LengthName_u8, 1000);
    
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

// +QGPSLOC: <UTC>,<latitude>,<longitude>,
static uint8_t _Cb_GPS_POSITION_INF(sData *uart_string)
{
    sData strCheck = {(uint8_t *) "+QGPSLOC: ", 10};
    int PosFind = -1;
    uint8_t Result = false, CountPh = 0;
    uint16_t PosStart = 0, i = 0;
    uint16_t PosEnd = 0;
    
    PosFind = Find_String_V2((sData*) &strCheck, uart_string);
    if (PosFind >= 0)
    {
        PosFind += strCheck.Length_u16;
        //Tim kiem day phay tiep theo: la vi tri bat dau
        for (i = PosFind; i < uart_string->Length_u16; i++)
        {
            if (*(uart_string->Data_a8 + i) == ',')
            {
                CountPh++;
               
                if (CountPh == 1)
                {
                    //Vi tri dau phay dau tien: start
                    PosStart = i + 1;
                } else if (CountPh == 3)
                {
                    //Vi tri dau phay thu 3: Stop va ket thuc
                    PosEnd = i;
                    Result = true;
                    break;
                }
            }
        }
                    
        if (Result == true)
        {
            sSimCommon.sGPS.LengData_u8 = PosEnd - PosStart;
            
            if (sSimCommon.sGPS.LengData_u8 < MAX_LENGTH_GPS)
            {
                sSimCommon.sGPS.Status_u8 = true;
                
                for (i = 0; i < sSimCommon.sGPS.LengData_u8; i++)
                {
                    sSimCommon.sGPS.aPOS_INFOR[i] = *(uart_string->Data_a8 + PosStart + i) ;
                }
                //Get OK
                UTIL_Printf_Str (DBLEVEL_M, "u_ec200u: gps get data OK!\r\n" );
                
                //Push block clear data cu
                sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_GPS_OK );   
            
                return 1;
            } else
            {
                UTIL_Log_Str (DBLEVEL_M, "u_ec200u: over size data!\r\n" );
                sSimCommon.sGPS.LengData_u8 = 0;
            }
        }
    }
    
    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_GPS_ERROR ); 
    
    UTIL_Log_Str ( DBLEVEL_M, "u_ec200u: gps get data error!\r\n" );
    
    return 1;
}


/*
    Func: Check Step Long timeout
        + Step CONNECT TCP 
        + STEP CONN MQTT
        + STEP PUB, SUB
        + STEP In HTTP process
*/

uint8_t EC200U_Check_Step_Long_Timeout (uint8_t sim_step)
{
    if ((sim_step == _SIM_TCP_CONNECT_2) || (sim_step == _SIM_MQTT_CONN_2) || (sim_step == _SIM_MQTT_SUB_2)
        || ((sim_step >= _SIM_MQTT_PUB_2) && (sim_step <= _SIM_END)))
        return 1;
    
    return 0;    
}


sCommand_Sim_Struct * EC200U_Get_AT_Cmd (uint8_t step)
{
    return (sCommand_Sim_Struct *) &aSimEC200UStep[step];
}


uint32_t EC200U_Get_Delay_2_AT (uint8_t Step)
{
    uint32_t Time = 0;
    
    switch (Step)
    {
        case _SIM_TCP_OUT_DATAMODE:
            Time = 1000;
            break;
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
uint8_t EC200U_Is_Step_Check_URC (uint8_t sim_step)
{
    if ((sim_step != _SIM_FTP_READ_1) && (sim_step != _SIM_FTP_READ_2)
        && (sim_step != _SIM_HTTP_READ_1) && (sim_step != _SIM_HTTP_READ_2)  
        && (sim_step != _SIM_SYS_FILE_READ1) && (sim_step != _SIM_SYS_FILE_READ2))  
    {
        return TRUE;
    }
    
    return FALSE;
}


uint8_t EC200U_Check_Step_Skip_Error (uint8_t step)
{
    if ( step == _SIM_GPS_POSTION_INF )
    {
        sSimCommon.sGPS.Status_u8 = error;
        sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_GPS_ERROR ); 
    }
    
    if ( ( step == _SIM_GPS_POSTION_INF) || (step == _SIM_GPS_TURN_ON) || (step == _SIM_GPS_DEL_CUR_DATA) )
        return true;
    
    return false;
}






