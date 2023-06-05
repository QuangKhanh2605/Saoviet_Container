/*
 * user_bc66.c
 *
 *  Created on: Dec 31, 2021
 *      Author: Chien
 */


#include "user_define.h"
#include "event_driven.h"
#include "user_bc66_cbAT.h"

/*===============define======================*/

/*=====================Var struct============================*/
static UTIL_TIMER_Object_t TimerControlBC66;

static void OnTimerControlBC66Event(void *context);

/*====Khi them hay bot step -> Sua So phan tu 34 kia -> De push dung step*/
uint8_t aSIM_STEP_CONTROL[42] =   //32
{
	//Pre Init
	_AT_CHECK_AT,			//0
	_AT_ECHO_CMD,		    //1
	_AT_ECHO_CMD,		    //2
	_AT_DIS_DEEP_SLEEP,		//3
	_AT_BAUD_RATE,			//4
	_AT_SIM_ID,				//5
	_AT_CHECK_SIM,			//6
	_AT_CHECK_RSSI,			//7
	_AT_DEFINE_APN,			//8
	_AT_SET_BAND_3,			//9
    _AT_RQ_MANUFAC,      //Test
	//Check Attach
	_AT_CHECK_ATTACH,		//10
	//Setting
//	_AT_CHECK_QENG,               //Test
    _AT_CONFIG_MODULE,
    _AT_CONFIG_MODULE1,
    _AT_CONFIG_MODULE2,
    _AT_CONFIG_MODULE3,
    _AT_CONFIG_MODULE4,
    _AT_CONFIG_MODULE5,
    _AT_SET_URC_PSM,
    
	_AT_EN_WAKEUP_URC,		//11
	_AT_TCP_CONFIG_DATA,	//12
	_AT_TCP_CONFIG_VIEW,	//13
	_AT_TCP_CONFIG_SHOW,	//14
	_AT_DISABLE_eDRX,		//15
	_AT_DIS_CONNECT_URC,	//16
	_AT_SAVE_SETTING,       //17   //BC660 k dung  lenh nay

	//Init TCP/IP
	_AT_CHECK_ATTACH,		//18
	_AT_DIS_DEEP_SLEEP,		//19
	_AT_GET_RTC,			//20
	//Open TCP socket
	_AT_TCP_OPEN_1,			//21
	_AT_TCP_OPEN_2,			//22

	_AT_MQTT_CONN_1,		//23
	_AT_MQTT_CONN_2,		//24

	_AT_MQTT_SUB_1,			//25
	_AT_MQTT_SUB_2,			//26

	_AT_MQTT_PUB_1,			//27
	_AT_MQTT_PUB_2,			//28

	_AT_TCP_CLOSE,			//29
    _AT_SET_CREG,
	_AT_EN_DEEP_SLEEP,		//30
	_AT_ENSABLE_PSM,		//31
    _AT_CHECK_CREG,
};


uint8_t aSIM_STEP_POWER_OFF[1] =
{
	_AT_POWER_OFF,
};

uint8_t aSIM_STEP_CLOSE_TCP[1] =
{
	_AT_TCP_CLOSE,
};


const sCommand_Sim_Struct aSimStep[] =
{
	{_AT_TEMP,		        "AT\r", 		 	"OK", 			_Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_CHECK_AT, 		    "AT\r", 			"OK", 			_Cb_AT_Success,   	_Cb_AT_Failure	},
	// pre Setup
    {_AT_SIM_ID, 		    "AT+QCCID\r", 	    "+QCCID:",		_Cb_AT_SIM_ID,   	_Cb_AT_Failure	},
	{_AT_BAUD_RATE, 		"AT+IPR=115200\r",  "OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_ECHO_CMD, 	        "ATE1\r",			"OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_NO_ECHO_CMD, 	    "ATE0\r",			"OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_CHECK_SIM,         "AT+CPIN?\r", 		"OK",           _Cb_AT_CHECK_SIM,   _Cb_AT_Failure	},
	{_AT_CHECK_RSSI, 		"AT+CSQ\r", 		"CSQ:",         _Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_DEFINE_APN, 		"AT+QCGDEFCONT=\"IP\",\"nbiot\"\r",		"OK", _Cb_AT_Success,   _Cb_AT_Failure	},	//v-internet    nbiot
    {_AT_RQ_MANUFAC, 	    "AT+CGMR\r", 	    "OK",		    _Cb_AT_Success,   	_Cb_AT_Failure	},
    
	//Attach
    {_AT_SAVE_SETTING, 		"AT&W\r",           "OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_POWER_OFF, 		"AT+QPOWD=0\r",	    "OK",			_Cb_AT_Power_Off,   _Cb_AT_Failure	},
	{_AT_CHECK_QENG, 		"AT+QENG=0\r",		"OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_CHECK_ATTACH, 		"AT+CGATT?\r",	    "OK",			_Cb_AT_CHECK_ATTACH,   	_Cb_AT_Failure	},
	{_AT_SET_BAND_3, 		"AT+QBAND=1,3\r",	"OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_SET_BAND_ALL, 		"AT+QBAND=0\r",	    "OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_CONNECT_URC, 	    "AT+CSCON=1\r",	    "OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
	{_AT_DIS_CONNECT_URC,   "AT+CSCON=0\r",	    "OK",			_CB_AT_DIS_URC,   	_Cb_AT_Failure	},
    
	{_AT_GET_RTC, 			"AT+CCLK?\r",		"OK", 		    _Cb_AT_GET_CLOCK,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE, 	"AT+QCFG=\"combinedattach\"\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE1, 	"AT+QCFG=\"autopdn\"\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE2, 	"AT+QCFG=\"ripin\"\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE3, 	"AT+QCFG=\"initlocktime\"\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE4, 	"AT+QCFG=\"dsevent\",1\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE5, 	"AT+QEDRXCFG=0\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CONFIG_MODULE6, 	"AT+QCFG=\"activetimer\"\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
    
    {_AT_SET_URC_PSM, 	"AT+QNBIOTEVENT=1,1\r",		"OK", 		    _Cb_AT_Success,   _Cb_AT_Failure	},
      
    //Lowpower
	{_AT_DIS_DEEP_SLEEP,  	"AT+QSCLK=0\r",	    "OK",			_Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_EN_DEEP_SLEEP,     "AT+QSCLK=1\r",		"OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_EN_LIGHT_SLEEP,    "AT+QSCLK=2\r",		"OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
    
    {_AT_DISABLE_PSM,  	    "AT+CPSMS=0\r",	    "OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_ENSABLE_PSM,  	    "AT+CPSMS=1,\"00111100\",\"00000001\"\r",	"OK",   _Cb_AT_PSM_Success,   _Cb_AT_Failure	}, //3h v� 4s
    {_AT_EN_WAKEUP_URC,     "AT+QATWAKEUP=1\r", "OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_ENABLE_eDRX,    	"AT+CEDRXS=1,5,\"0101\"\r",  "OK",  _Cb_AT_Success,   	_Cb_AT_Failure	},    //ENABLE eDRX
    {_AT_DISABLE_eDRX,      "AT+CEDRXS=0\r",	"OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_SOFT_RESET,        "AT+QRST=1\r",      "RDY",          _Cb_AT_Success,   	_Cb_AT_Failure	},
    
    {_AT_SET_CREG, 		    "AT+CEREG=5\r",     "OK",           _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_CHECK_CREG, 		"AT+CEREG?\r",      "OK",           _Cb_AT_Success,   _Cb_AT_Failure	},
    
    // Setup GPRS
    {_AT_TCP_CONFIG_DATA,	"AT+QICFG=\"dataformat\",1,1\r",    "OK", _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_TCP_CONFIG_VIEW,	"AT+QICFG=\"viewmode\",1\r",	    "OK", _Cb_AT_Success,   _Cb_AT_Failure	},
    {_AT_TCP_CONFIG_SHOW,	"AT+QICFG=\"showlength\",1\r",	    "OK", _Cb_AT_Success,   _Cb_AT_Failure	},

    {_AT_TCP_ACESS_MODE,	"AT+QISWTMD=0,0\r",	 "OK",          _Cb_AT_Success,   	_Cb_AT_Failure	},
    {_AT_TCP_PUSH_MODE,	    "AT+QISWTMD=0,1\r",  "OK",          _Cb_AT_Success,   	_Cb_AT_Failure	},
    //TCP Socket
    {_AT_TCP_OPEN_1, 		"AT+QIOPEN=",	    NULL,           _Cb_AT_TCP_CONNECT_1,   _Cb_AT_Failure	},
    {_AT_TCP_OPEN_2, 		"\r",  				"+QIOPEN: 0",   _Cb_AT_TCP_CONNECT_2,   _Cb_AT_Failure	},
    
	{_AT_MQTT_CONN_1, 		NULL,				NULL,           _Cb_MQTT_CONNECT_1,   _Cb_AT_Failure	},
	{_AT_MQTT_CONN_2, 		"\r",	    		"4,20020000",   _Cb_MQTT_CONNECT_2,   _Cb_AT_Failure	},

	{_AT_MQTT_SUB_1, 		NULL,				NULL,           _Cb_MQTT_SUBCRIBE_1,   _Cb_AT_Failure	},
	{_AT_MQTT_SUB_2, 		"\r",	    		"6,9004000A0000",  _Cb_MQTT_SUBCRIBE_2,   _Cb_AT_Failure	},

	{_AT_MQTT_PUB_1, 		NULL,				NULL,           _Cb_MQTT_PUBLISH_1,   _Cb_AT_Failure	},
	{_AT_MQTT_PUB_2, 		"\r",	   			"OK" ,   		_Cb_MQTT_PUBLISH_2,   _Cb_AT_Failure	},  //"4,40030001"  "SEND OK"

	{_AT_MQTT_PUB_FB_1, 	NULL,				NULL,           _Cb_MQTT_PUBLISH_1,   _Cb_AT_Failure	},
	{_AT_MQTT_PUB_FB_2, 	"\r",	   			"4,40030001",   _Cb_MQTT_PUBLISH_2,   _Cb_AT_Failure	},  //  "SEND OK"

    {_AT_TCP_CLOSE, 	    "AT+QICLOSE=0\r",   "OK",           _Cb_AT_Success,   	_Cb_AT_Failure	},
    
	{SIM_CMD_END, 		    NULL,		        NULL,	        _Cb_AT_Success,   	_Cb_AT_Failure},
};



const sCommand_Sim_Struct aSimUrc[] =
{
	{SIM_URC_RESET_SIM900,		NULL, 	"NORMAL POWER DOWN", 		_Cb_AT_Success,   	NULL	},// OK
	{SIM_URC_ALREADY_CONNECT, 	NULL, 	"ALREADY CONNECT",   		_Cb_AT_Success,  	NULL	},
	{SIM_URC_SIM_LOST, 			NULL, 	"CPIN: NOT",		 		_Cb_AT_Success,   	NULL	},
	{SIM_URC_CLOSED,			NULL, 	"+SERVER DISCONNECTED", 	_Cb_AT_Success,   	NULL	},
	{SIM_URC_CALL_READY, 		NULL,	"+CPIN: READY",				_Cb_AT_CHECK_SIM,   NULL	},
	{SIM_URC_ERROR, 			NULL,	"ERROR",					_Cb_URC_ERROR,   	NULL	},
	{SIM_URC_WAKEUP,	 		NULL,	"+QATWAKEUP",				_Cb_URC_WAKEUP,   	NULL	},
	{SIM_URC_DEEP_SLEEP, 		NULL,	"ENTER DEEPSLEEP",			_Cb_AT_Success,   	NULL	},
};



/*====================Function call back============================*/

/*====================Function handler============================*/
uint8_t BC66_ON(void)
{
	static uint8_t Step_On_BC66 = 0;

	switch(Step_On_BC66)
	{
		case 0:
			PrintDebug(&UART_SERIAL, (uint8_t*) "POWER ON SIM\r\n", 14, 1000);
			BC66_PWKEY_PIN_OFF;
			BC66_RESET_PIN_OFF;
			sSimVar.EventMarkToControl_u8 = EVENT_SIM_TURN_ON;
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 1000);
			UTIL_TIMER_Start(&TimerControlBC66);   
			break;
		case 1:
			BC66_PWKEY_PIN_ON;
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 1000);
			UTIL_TIMER_Start(&TimerControlBC66);
			break;
		case 2:
			BC66_PWKEY_PIN_OFF;
            UTIL_TIMER_SetPeriod(&TimerControlBC66, 2000);
			UTIL_TIMER_Start(&TimerControlBC66);
            break;
		default:
			Step_On_BC66 = 0;
			return 1;
	}

	Step_On_BC66++;

	return 0;
}


uint8_t BC66_Hard_Reset(void)
{
	static uint8_t Step_Reset_BC66 = 0;

	switch(Step_Reset_BC66)
	{
		case 0:
			PrintDebug(&UART_SERIAL, (uint8_t*) "Hard Reset SIM\r\n", 16, 1000);
			BC66_RESET_PIN_OFF;
			sSimVar.EventMarkToControl_u8 = EVENT_SIM_HARD_RESET;
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 500);
			UTIL_TIMER_Start(&TimerControlBC66);
			break;
		case 1:
			BC66_RESET_PIN_ON;
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 500);
			UTIL_TIMER_Start(&TimerControlBC66);
			break;
		case 2:
			BC66_RESET_PIN_OFF;
			UTIL_TIMER_SetPeriod(&TimerControlBC66, 2000);
			UTIL_TIMER_Start(&TimerControlBC66);
			break;
		default:
			Step_Reset_BC66 = 0;
			return 1;
	}

	Step_Reset_BC66++;

	return 0;
}



/*
 * Func: Danh thuc Bc66 bang chan Phan cung: PSM wakeup
 * Output: 0: Fail
 * 		   1: Wakeup OK: Check phản hồi về OK
 * 		   2: Pending
 * */
uint8_t BC66_Wake_Up(void)
{
	static uint8_t Step_Wakeup_BC66 = 0;

    fevent_enable(sEventSim, EVENT_SIM_WAKEUP);
	switch(Step_Wakeup_BC66)
	{
		case 0:
			PrintDebug(&UART_SERIAL, (uint8_t*) "WAKE UP BC66\r\n", 14, 1000);
			sSimVar.IsWakeupBc66_u8 = FALSE;
			BC66_WAKEUP_PIN_ON;
			sEventSim[EVENT_SIM_WAKEUP].e_period = 50;
			break;
		case 1:
			BC66_WAKEUP_PIN_OFF;
			sEventSim[EVENT_SIM_WAKEUP].e_period = 50;
			break;
		case 2:
			BC66_WAKEUP_PIN_ON;
			sEventSim[EVENT_SIM_WAKEUP].e_period = 50;
			break;
		case 3:
			BC66_WAKEUP_PIN_OFF;
			sEventSim[EVENT_SIM_WAKEUP].e_period = 50;
			break;
		case 4:
            BC66_WAKEUP_PIN_ON;
		case 5:
		case 6:
		case 7:
			//Cho day 2000 s de check xem wakeup OK không
			if (sSimVar.IsWakeupBc66_u8 == TRUE)
				Step_Wakeup_BC66 += 4;

			sEventSim[EVENT_SIM_WAKEUP].e_period = 2000;   //4 lân & 500ms
			break;
		default:
            fevent_disable(sEventSim, EVENT_SIM_WAKEUP);
			Step_Wakeup_BC66 = 0;
			return sSimVar.IsWakeupBc66_u8;   //Return gia tri wakeup Flag
	}

	Step_Wakeup_BC66++;

	return 2;
}


void OnTimerControlBC66Event(void *context)
{
	//active event sleep hoac powon hoac wakeup
	fevent_active(sEventSim, sSimVar.EventMarkToControl_u8);
}


void Init_Timer_PowerOn (void)
{
	UTIL_TIMER_Create(&TimerControlBC66, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerControlBC66Event, NULL);
}

