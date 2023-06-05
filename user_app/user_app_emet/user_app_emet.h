



#ifndef USER_APP_EMET_H_
#define USER_APP_EMET_H_


#include "user_util.h"
#include "event_driven.h"

#include "user_external_mem.h"
#include "user_internal_mem.h"


#define  USING_APP_EMET

/*============= Define =====================*/


/*================ var struct =================*/

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


extern sEvent_struct sEmetAppEvent [];

/*================ Function =================*/
uint8_t     _Cb_Log_TSVH (uint8_t event);
uint8_t     _Cb_Scan_Alarm (uint8_t event);

//Function handler
uint8_t     Modem_Check_Alarm (void);
uint8_t     Modem_Packet_TSVH (uint8_t *pData);
uint8_t     Modem_Packet_Event (uint8_t *pData);

uint8_t     Modem_Handler_Alarm (uint8_t Index);

void        Modem_Log_Data_Event (StructManageRecordFlash *sRecordEvent);
void        Modem_Log_Data_TSVH (StructManageRecordFlash *sRecordTSVH);



void        _rREQUEST_SETTING(sData *str_Receiv,int16_t Pos);





#endif