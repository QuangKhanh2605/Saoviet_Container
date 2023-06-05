
/*
    8/2021
    Thu vien xu ly Time va Timer
*/
#ifndef USER_TIME_H
#define USER_TIME_H

#include "user_util.h"

#include "user_rtc.h"

/*======================== Structs ======================*/


extern RTC_HandleTypeDef hrtc;
/*========================Extern Structs ======================*/
extern const uint16_t   days[4][12];

/*======================== FUNCTION ======================*/
void            Get_RTC(void);
uint8_t         Set_RTC(ST_TIME_FORMAT sRTC_Var);
uint8_t         Check_update_Time(ST_TIME_FORMAT *sRTC_temp);
//
void            Convert_sTime_ToGMT (ST_TIME_FORMAT* sRTC_Check, uint8_t GMT);
void            Epoch_to_date_time(ST_TIME_FORMAT* date_time, uint32_t meterTS, uint8_t type);
uint32_t        HW_RTC_GetCalendarValue_Second( ST_TIME_FORMAT sTimeRTC, uint8_t type);

void 		Set_Alarm_Defaut_Time (uint8_t time_hour,uint8_t time_min, uint8_t time_sec);
void 		Set_Alarm_For_Time (uint8_t time_hour,uint8_t time_min, uint8_t time_sec);
void 		Set_Alarm_DCU (uint8_t time_hour,uint8_t time_min, uint8_t time_sec);


#endif /*  */

