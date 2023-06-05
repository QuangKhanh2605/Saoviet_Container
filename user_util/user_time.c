
/*
    8/2021
    Thu vien xu ly Time va Timer
*/
#include "user_time.h"

/*===================Struct, Var=========================*/
static uint8_t         DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static uint8_t         DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const uint16_t days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

extern RTC_AlarmTypeDef sAlarm;
/*===================Function=========================*/
void Get_RTC(void)
{
    RTC_TimeTypeDef     sRTCTime;
    RTC_DateTypeDef     sRTCDate;

    HAL_RTC_GetTime(&hrtc, &sRTCTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sRTCDate, RTC_FORMAT_BIN);
    
	sRTC.sec 	= sRTCTime.Seconds;
	sRTC.min 	= sRTCTime.Minutes;
	sRTC.hour 	= sRTCTime.Hours;
	sRTC.day 	= sRTCDate.WeekDay;
	sRTC.date 	= sRTCDate.Date;
	sRTC.month 	= sRTCDate.Month;
	sRTC.year 	= sRTCDate.Year;
}


uint8_t Set_RTC(ST_TIME_FORMAT sRTC_Var)
{
    RTC_TimeTypeDef 		sRTCTime_Temp = {0};
    RTC_DateTypeDef 		sRTCDate_Temp = {0};

    if (Check_update_Time(&sRTC_Var) == 1)
    {
        sRTCTime_Temp.SubSeconds     = sRTC_Var.SubSeconds;
        sRTCTime_Temp.Seconds 	     = sRTC_Var.sec;
        sRTCTime_Temp.Minutes        = sRTC_Var.min;
        sRTCTime_Temp.Hours 		 = sRTC_Var.hour;

        sRTCDate_Temp.WeekDay 	     = sRTC_Var.day;
        sRTCDate_Temp.Date 		     = sRTC_Var.date;
        sRTCDate_Temp.Month 		 = sRTC_Var.month;
        sRTCDate_Temp.Year 		     = sRTC_Var.year;

        HAL_RTC_SetTime(&hrtc, &sRTCTime_Temp, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &sRTCDate_Temp, RTC_FORMAT_BIN);
        
        return 1;
    }
    
    return 0;
}

/*
 *  return 1 - TRUE : if data correct
 */
uint8_t Check_update_Time(ST_TIME_FORMAT *sRTC_temp)
{
	if (sRTC_temp->sec > 60) 
        return 0;
	if (sRTC_temp->min > 60) 
        return 0;
	if (sRTC_temp->hour > 23) 
        return 0;
	if (sRTC_temp->day > 8)  
        return 0;
	if (sRTC_temp->date > 31) 
        return 0;
	if (sRTC_temp->month > 12) 
        return 0;
	if (sRTC_temp->year > 99) 
        return 0;
    if (sRTC_temp->year < 20) 
        return 0;
    
	return 1;
}


//bat dau tu hour
void Convert_sTime_ToGMT (ST_TIME_FORMAT* sRTC_Check, uint8_t GMT)
{      
    sRTC_Check->hour = sRTC_Check->hour + GMT;
      
    if ((sRTC_Check->hour >= 24) && (sRTC_Check->hour < 48))
    {
        sRTC_Check->hour = sRTC_Check->hour % 24;
        //c�ng ngay them 1
        if ((sRTC_Check->year % 4) == 0)
        {
            sRTC_Check->date += 1;
            if (sRTC_Check->date > DaysInMonthLeapYear[sRTC_Check->month - 1])
            {
                sRTC_Check->date = 1;
                sRTC_Check->month += 1;
                //
                if (sRTC_Check->month > 12)
                {
                    sRTC_Check->month = 1;
                    sRTC_Check->year  = (sRTC_Check->year + 1) % 100;
                }
            }   
        } else
        {
            sRTC_Check->date += 1;
            if (sRTC_Check->date > DaysInMonth[sRTC_Check->month - 1])
            {
                sRTC_Check->date = 1;
                sRTC_Check->month += 1;
                //
                if (sRTC_Check->month > 12)
                {
                    sRTC_Check->month = 1;
                    sRTC_Check->year  = (sRTC_Check->year + 1) % 100;
                }
            } 
        }
    }  
}

/*
    -function chung:
        + khac nhau moc thoi gian danh cho tung loai cong to
    - type: 1: CPC
            2: ELster
            3: Genius
*/

void Epoch_to_date_time(ST_TIME_FORMAT* date_time, uint32_t meterTS, uint8_t type)
{
	unsigned int    years;
	unsigned int    year;
	unsigned int    month;
	uint32_t	    epoch;
	
    switch (type)
    {
        case 1:
            epoch = meterTS + 378691200;   //from 2000. Do CPC moc thoi gian tu 1.1.2012. n�n cong them cho ve nam 2000
            break;
        case 2:
            epoch = meterTS - 946684800;   //from 2000.moc tu 1976
            break; 
        case 3:
            epoch = meterTS - 126230400;     //1/1/1996 - 1/1/2000: 1461 ngay
            break;     
        default: 
            epoch = meterTS;
          break;
    }
	
    date_time->sec = epoch%60; epoch /= 60;
    date_time->min = epoch%60; epoch /= 60;
    date_time->hour= epoch%24; epoch /= 24;

    years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    for (year = 3; year > 0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->date  = epoch-days[year][month]+1;
}

uint32_t HW_RTC_GetCalendarValue_Second( ST_TIME_FORMAT sTimeRTC, uint8_t type)
{
  uint32_t correction;
  uint32_t seconds;
  
 //anh l�y bat d�u tu d�y
  /* calculte amount of elapsed days since 01/01/2000 */
  seconds= DIVC( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* sTimeRTC.year , 4);

  correction = ( (sTimeRTC.year % 4) == 0 ) ? DAYS_IN_MONTH_CORRECTION_LEAP : DAYS_IN_MONTH_CORRECTION_NORM ;
 
  seconds +=( DIVC( (sTimeRTC.month-1)*(30+31) ,2 ) - (((correction>> ((sTimeRTC.month-1)*2) )&0x3)));

  seconds += (sTimeRTC.date -1);
  
  /* convert from days to seconds */
  seconds *= SECONDS_IN_1DAY; 

  seconds += ( ( uint32_t )sTimeRTC.sec + 
             ( ( uint32_t )sTimeRTC.min * SECONDS_IN_1MINUTE ) +
             ( ( uint32_t )sTimeRTC.hour * SECONDS_IN_1HOUR ) ) ;

    switch(type)
    {
        case 1:
            seconds -= 378691200;   //Do moc CPC 2012
            break; 
        case 2: 
            seconds += 946684800;
            break;
        case 3:
            seconds += 126230400;
            break;     
        default: 
            break;
    }
    
    return seconds;   
}



void Set_Alarm_Defaut_Time (uint8_t time_hour, uint8_t time_min, uint8_t time_sec)
{
    if (time_hour >= 24)
    	time_hour = time_hour % 24;

    if (time_min >= 60)
    	time_min = time_min % 60;

    if (time_sec >= 60)
		time_sec = time_sec % 60;

	sAlarm.AlarmTime.Seconds = time_sec;
	sAlarm.AlarmTime.Minutes = time_min;
	sAlarm.AlarmTime.Hours   = time_hour;

	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}


void Set_Alarm_For_Time (uint8_t time_hour,uint8_t time_min, uint8_t time_sec)
{
    uint16_t Temp_Sec = 0;
    uint16_t Temp_min = 0;

	Get_RTC();

    Temp_Sec = sRTC.sec + time_sec;
    Temp_min = sRTC.min + time_min;

	sAlarm.AlarmTime.Seconds  = Temp_Sec%60;
	sAlarm.AlarmTime.Minutes  = (Temp_min+(Temp_Sec/60))%60;
	sAlarm.AlarmTime.Hours=((sRTC.hour+time_hour)+((Temp_min+(Temp_Sec/60))/60))%24;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}



void Set_Alarm_DCU (uint8_t time_hour,uint8_t time_min, uint8_t time_sec)
{
    uint8_t Temp = 0;
    uint8_t Temp_hour = 0;
    uint8_t Temp_min = 0;
    uint8_t Temp_Sec = 0;

    Get_RTC();

    if (time_hour == 0)
    {
        if (time_min != 0)
        {
            Temp = time_min - (sRTC.min % time_min);
            if (Temp < 2)
                Temp += time_min;

            Temp_Sec  = time_sec % 60;
            Temp_min  = (sRTC.min + Temp) % 60;
            Temp_hour = (sRTC.hour + (sRTC.min + Temp) / 60) % 24;

            Set_Alarm_Defaut_Time ( Temp_hour, Temp_min, Temp_Sec);   //neu so phut la 01 00 thi hen 4 tieng.
        }
    } else
    {
        if (sRTC.min >= 57)
            Temp = time_hour + 1;
        else
        	Temp = time_hour;

        Temp_hour = (sRTC.hour + Temp) %24;

        Set_Alarm_Defaut_Time (Temp_hour, 0, 0);
    }
}
