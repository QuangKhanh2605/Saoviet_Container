/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_AT_SERIAL_H
#define USER_AT_SERIAL_H

#include "user_util.h"
#include "usart.h"

/*======================== Define ======================*/

    
/*======================== Structs var======================*/
typedef enum
{
	_SET_DEV_SERIAL,
	_QUERY_DEV_SERIAL,
    _QUERY_SIM_ID,
    
	_SET_SERVER_MAIN,
	_QUERY_SERVER_MAIN,
    
    _SET_SERVER_BACKUP,
    _QUERY_SERVER_BACKUP,
    
    _REQ_SWITCH_SERVER,
    
    _SET_DUTY_READ_DATA,
    _QUERY_DUTY_READ_DATA,
    
    _SET_FREQ_ONLINE,
    _QUERY_FREQ_ONLINE,
    
    _SET_DEFAULT_PULSE,
    _SET_SAVE_BOX,
    _SET_NUM_WAKEUP,
    _READ_OLD_DATA,
    
    _SET_FLOW_THRESH,
    _QUERY_FLOW_THRESH,
    _SET_QUAN_THRESH,
    _QUERY_QUAN_THRESH,
    _SET_BAT_THRESH,
    _QUERY_BAT_THRESH,
    _SET_PULSE_VALUE,
    _QUERY_PULSE_VALUE,
    
    _SET_RTC_TIME,
    _SET_RTC_DATE,
    _SET_RTC,
    _QUERY_RTC,
    _SET_REQUEST_STIME,
    
    _RESET_MODEM,
    _ERASE_MEMORY,
    _QUERY_FIRM_VER,
    _QUERY_UP_FIRM_HTTP,
    _QUERY_UP_FIRM_FTP,
    
    _QUERY_INDEX_LOG,
    _TEST_LOG,
    _QUERY_READ_ALL_LOG,
    _QUERY_READ_LAST_LOG,
    _QUERY_LAST_FOR_LOG,
    
    _QUERY_LAST_FOR_OPERA,
    _QUERY_LAST_FOR_EVENT,
    
    _SET_CALIB_LEVEL,
    _QUERY_CALIB_LEVEL,
    
    _SET_LEVEL_THRESH,
    _QUERY_LEVEL_THRESH,
    
    _QUERY_GPS_LOC,
    _SET_LIST_ID_SLAVE,
    _QUERY_LIST_ID_SLAVE,
    
    _END_AT_CMD,
}Type_Command_AT_PC;



typedef void (*_func_callback_f)(sData *str_Receiv, uint16_t Pos);

typedef struct {
	int 			    idStep;
	_func_callback_f	CallBack;
	sData	    sTempReceiver;				// temp Receive <- Sim900
} struct_CheckList_AT;

/*======================== External Var struct ======================*/
extern const struct_CheckList_AT CheckList_AT_CONFIG[];
extern uint8_t      aResPondAT[256];
extern uint8_t 		PortConfig;
extern uint8_t      aReceiAT[256];
extern uint16_t     LengthRecei;
extern uint16_t     LengthResPond;

/*======================== Function ======================*/
void        _fSET_DEV_SERIAL(sData *str_Receiv, uint16_t Pos);
void        _fQUERY_DEV_SERIAL(sData *str_Receiv, uint16_t Pos);
void        _fQUERY_SIM_ID(sData *strRecei, uint16_t Pos);

void 		_fSET_SERVER_MAIN_INFO (sData *strRecei, uint16_t Pos);
void 		_fQUERY_SERVER_MAIN_INFO (sData *strRecei, uint16_t Pos);

void       	_fSET_DUTY_READ_DATA(sData *str_Receiv, uint16_t Pos);
void        _fQUERY_DUTY_READ_DA(sData *str_Receiv, uint16_t Pos);

void        _fSET_FREQ_ONLINE(sData *strRecei, uint16_t Pos);
void        _fQUERY_FREQ_ONLINE(sData *strRecei, uint16_t Pos);

void        _fSET_DATAPULSE_DEFAUL(sData *str_Receiv, uint16_t Pos);
void        _fSET_NUM_WAKEUP_DEV(sData *str_Receiv, uint16_t Pos);
void        _fSET_SAVE_BOX(sData *str_Receiv, uint16_t Pos);
void 		_fREAD_OLD_DATA(sData *strRecei, uint16_t Pos);

void        _fSET_FLOW_THRESH (sData *str_Receiv, uint16_t Pos);
void        _fQUERY_FLOW_THRESH (sData *str_Receiv, uint16_t Pos);
void        _fSET_QUAN_THRESH (sData *str_Receiv, uint16_t Pos);
void        _fQUERY_QUAN_THRESH (sData *str_Receiv, uint16_t Pos);
void        _fSET_BAT_THRESH (sData *str_Receiv, uint16_t Pos);
void        _fQUERY_BAT_THRESH (sData *str_Receiv, uint16_t Pos);
void        _fSET_PULSE_VALUE (sData *str_Receiv, uint16_t Pos);
void        _fQUERY_PULSE_VALUE (sData *str_Receiv, uint16_t Pos);

void        _fSET_RTC_TIME (sData *str_Receiv, uint16_t Pos);
void        _fSET_RTC_DATE (sData *str_Receiv, uint16_t Pos);
void        _fSET_RTC (sData *str_Receiv, uint16_t Pos);
void        _fQUERY_RTC (sData *str_Receiv, uint16_t Pos);

void        _fRESET_MODEM (sData *str_Receiv, uint16_t Pos);
void        _fERASE_MEMORY (sData *str_Receiv, uint16_t Pos);

void        _fQUERY_FIRM_VER (sData *strRecei, uint16_t Pos);
void        _fGET_URLHTTP_FIRM (sData *strRecei, uint16_t Pos);
void        _fGET_URLFTP_FIRM (sData *strRecei, uint16_t Pos);

void        _fGET_LAST_LOG (sData *strRecei, uint16_t Pos);
void        _fQUERY_ALL_LOG (sData *strRecei, uint16_t Pos);
void        _fQUERY_INDEX_LOG (sData *strRecei, uint16_t Pos);
void        _fGET_LAST_FOR_LOG (sData *strRecei, uint16_t Pos);
void        _fTEST_LOG (sData *strRecei, uint16_t Pos);

void        _fGET_LAST_FOR_OPERA (sData *strRecei, uint16_t Pos);
void        _fGET_LAST_FOR_EVENT (sData *strRecei, uint16_t Pos);

void        _fSET_CALIB_LEVEL (sData *strRecei, uint16_t Pos);
void        _fQUERY_CALIB_LEVEL (sData *strRecei, uint16_t Pos);

void        _fSET_LEVEL_THRESH (sData *strRecei, uint16_t Pos);
void        _fQUERY_LEVEL_THRESH (sData *strRecei, uint16_t Pos);

uint8_t     Check_AT_User(sData *StrUartRecei, uint8_t Type);

void        _fSET_SERVER_BACKUP_INFO (sData *strRecei, uint16_t Pos);
void        _fQUERY_SERVER_BACKUP_INFO (sData *strRecei, uint16_t Pos);
void        _fREQ_SWITCH_SERVER (sData *strRecei, uint16_t Pos);
void        _fSET_RF_RQ_STIME (sData *str_Receiv, uint16_t Pos);
void        _fQUERY_GPS_LOC (sData *strRecei, uint16_t Pos);


uint16_t    SERIAL_Get_Num (sData *strRecei, uint16_t Pos);
void        _fQUERY_LIST_ID_SLAVE (sData *str_Receiv, uint16_t Pos);
void        _fSET_LIST_ID_SLAVE (sData *str_Receiv, uint16_t Pos);

#endif
