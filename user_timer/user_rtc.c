/*
 8/2021
 Thu vien: cau hinh rtc
 */
#include "user_rtc.h"
#include "rtc.h"
#include "user_timer.h"

/*======================== Structs var======================*/

#ifdef USER_RTC_DEBUG
	#define RTC_IF_DBG_PRINTF(...)
#else
#define RTC_IF_DBG_PRINTF(...)
#endif

extern RTC_HandleTypeDef hrtc;

static bool RTC_Initalized = false;

static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static RTC_AlarmTypeDef RTC_AlarmStructure;      //Alarm đang được set
static RtcTimerContext_t RtcTimerContext; 		 //neu get-> là thời gian trước hẹn giờ | sau đó set ->Thơi gian hien tai

/*Timer driver*/
const UTIL_TIMER_Driver_s UTIL_TimerDriver =
{
    RTC_IF_Init,
    NULL,

    RTC_IF_StartTimer,				//start
    RTC_IF_StopTimer,

    RTC_IF_SetTimerContext,		  	//Set moc thoi gian set alarm
    RTC_IF_GetTimerContext,         //Get Context luc set ALarm

    RTC_IF_GetTimerElapsedTime, 	//Lay thoi gian troi qua: sRTC - RtcTimerContext (Ghi lai sRTC lúc set alarm)
    RTC_IF_GetTimerValue,           //Lay thoi gian RTC quy đôi sang ms.
    RTC_IF_GetMinimumTimeout,       //3tick: danh de xu ly setup timer

    RTC_IF_Convert_ms2Tick,         //Convert ms sang tick
    RTC_IF_Convert_Tick2ms,
};


///*System Time driver*/
//const UTIL_SYSTIM_Driver_s UTIL_SYSTIMDriver =
//{
//    RTC_IF_BkUp_Write_Seconds,
//    RTC_IF_BkUp_Read_Seconds,
//    RTC_IF_BkUp_Write_SubSeconds,
//    RTC_IF_BkUp_Read_SubSeconds,
//    RTC_IF_GetTime,
//};


//Khai bao nguyen mau cac ham static

static void RTC_StartWakeUpAlarm(uint32_t timeoutValue);
static uint32_t RTC_GetCalendarValue(RTC_DateTypeDef *RTC_DateStruct, RTC_TimeTypeDef *RTC_TimeStruct);
static void User_RTC_Init(void);

void (*pModemProcessIRQTimer) (void);

/*======================== Function ======================*/
static void User_RTC_Init(void)
{

	RTC_AlarmTypeDef sAlarm = { 0 };

	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = RTC_PREDIV_A;
	hrtc.Init.SynchPrediv = RTC_PREDIV_S;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
#if defined (STM32L072xx) 
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
#endif
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}

	/** Enable the Alarm A
	 */
	sAlarm.AlarmTime.Hours = 0;
	sAlarm.AlarmTime.Minutes = 0;
	sAlarm.AlarmTime.Seconds = 0;
	sAlarm.AlarmTime.SubSeconds = 0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = 1;
	sAlarm.Alarm = RTC_ALARM_A;
	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

UTIL_TIMER_Status_t RTC_IF_Init(void)
{
	if (RTC_Initalized == false)
	{
		RTC_IF_DBG_PRINTF("RTC_init\n\r");

		User_RTC_Init();

		/** Configure the Alarm A */
		HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
		/*Enable Direct Read of the calendar registers (not through Shadow) */
		HAL_RTCEx_EnableBypassShadow(&hrtc);

		RTC_IF_SetTimerContext();

		RTC_Initalized = true;
	}

	return UTIL_TIMER_OK;
}

UTIL_TIMER_Status_t RTC_IF_StartTimer(uint32_t timeout)
{
	/* we don't go in Low Power mode for timeout below MIN_ALARM_DELAY */
//  if ((MIN_ALARM_DELAY + McuWakeUpTimeCal) < ((timeout - RTC_IF_GetTimerElapsedTime())))
//  {
//    UTIL_LPM_SetStopMode((1 << CFG_LPM_RTC_Id), UTIL_LPM_ENABLE);
//  }
//  else
//  {
//    UTIL_LPM_SetStopMode((1 << CFG_LPM_RTC_Id), UTIL_LPM_DISABLE);
//  }
//
//  /*In case stop mode is required */
//  if (UTIL_LPM_GetMode() == UTIL_LPM_STOPMODE)
//  {
//    timeout = timeout -  McuWakeUpTimeCal;
//  }
	RTC_StartWakeUpAlarm(timeout);

	return UTIL_TIMER_OK;
}

UTIL_TIMER_Status_t RTC_IF_StopTimer(void)
{
	/* Clear RTC Alarm Flag */
	__HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
	/* Disable the Alarm A interrupt */
	HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

	return UTIL_TIMER_OK;
}

uint32_t RTC_IF_SetTimerContext(void)
{
	/*store time context*/
	RtcTimerContext.Rtc_Time = RTC_GetCalendarValue(&RtcTimerContext.RTC_Calndr_Date,
			&RtcTimerContext.RTC_Calndr_Time);
	RTC_IF_DBG_PRINTF("RTC_IF_SetTimerContext=%d\n\r", RtcTimerContext.Rtc_Time);
	/*return time context*/
	return (uint32_t) RtcTimerContext.Rtc_Time;
}

uint32_t RTC_IF_GetTimerContext(void)
{
	RTC_IF_DBG_PRINTF("RTC_IF_GetTimerContext=%d\n\r", RtcTimerContext.Rtc_Time);
	/*return time context*/
	return (uint32_t) RtcTimerContext.Rtc_Time;
}

uint32_t RTC_IF_GetTimerElapsedTime(void)
{
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	uint32_t CalendarValue = RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct);

	return ((uint32_t) (CalendarValue - RtcTimerContext.Rtc_Time));
}

uint32_t RTC_IF_GetTimerValue(void)
{
	uint32_t CalendarValue = 0;
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;

	if (RTC_Initalized == true)
	{
		CalendarValue = (uint32_t) RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct);
	}
	return CalendarValue;
}

uint32_t RTC_IF_GetMinimumTimeout(void)
{
	return (MIN_ALARM_DELAY);
}

uint32_t RTC_IF_Convert_ms2Tick(uint32_t timeMilliSec)
{
	return (uint32_t) ((((uint64_t) timeMilliSec) * CONV_DENOM) / CONV_NUMER);
}

uint32_t RTC_IF_Convert_Tick2ms(uint32_t tick)
{
	return (((uint64_t) (tick) * CONV_NUMER) / CONV_DENOM);
}

void RTC_IF_DelayMs(uint32_t delay)
{
	uint32_t delayValue = 0;
	uint32_t timeout = 0;

	delayValue = RTC_IF_Convert_ms2Tick(delay);

	/* Wait delay ms */
	timeout = RTC_IF_GetTimerValue();
	while (((RTC_IF_GetTimerValue() - timeout)) < delayValue)
	{
		__NOP();
	}
}

/**
 * @brief  Alarm A callback.
 * @param  RtcHandle: RTC handle
 * @retval None
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *RtcHandle)
{
    pModemProcessIRQTimer();
//	UTIL_TIMER_IRQ_Handler();
}

uint32_t RTC_IF_GetTime(uint16_t *mSeconds)
{
	RTC_TimeTypeDef RTC_TimeStruct;
	RTC_DateTypeDef RTC_DateStruct;
	uint32_t ticks;

	uint64_t calendarValue = RTC_GetCalendarValue(&RTC_DateStruct, &RTC_TimeStruct);

	uint32_t seconds = (uint32_t) (calendarValue >> RTC_N_PREDIV_S);

	ticks = (uint32_t) calendarValue & RTC_PREDIV_S;

	*mSeconds = RTC_IF_Convert_Tick2ms(ticks);

	return seconds;
}

void RTC_IF_BkUp_Write_Seconds(uint32_t Seconds)
{
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SECONDS, Seconds);
}

void RTC_IF_BkUp_Write_SubSeconds(uint32_t SubSeconds)
{
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SUBSECONDS, SubSeconds);
}

uint32_t RTC_IF_BkUp_Read_Seconds(void)
{
	return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_SECONDS);
}

uint32_t RTC_IF_BkUp_Read_SubSeconds(void)
{
	return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_SUBSECONDS);
}

/* Private functions ---------------------------------------------------------*/

/*!
 * @brief start wake up alarm
 * @note  alarm in RtcTimerContext.Rtc_Time + timeoutValue
 * @param timeoutValue in ticks
 * @retval none
 */
static void RTC_StartWakeUpAlarm(uint32_t timeoutValue)
{
	uint16_t rtcAlarmSubSeconds = 0;
	uint16_t rtcAlarmSeconds = 0;
	uint16_t rtcAlarmMinutes = 0;
	uint16_t rtcAlarmHours = 0;
	uint16_t rtcAlarmDays = 0;
	RTC_TimeTypeDef RTC_TimeStruct = RtcTimerContext.RTC_Calndr_Time;
	RTC_DateTypeDef RTC_DateStruct = RtcTimerContext.RTC_Calndr_Date;

	RTC_IF_StopTimer();

	/*reverse counter */
	rtcAlarmSubSeconds = RTC_PREDIV_S - RTC_TimeStruct.SubSeconds;
	rtcAlarmSubSeconds += (timeoutValue & RTC_PREDIV_S);
	/* convert timeout  to seconds */
	timeoutValue >>= RTC_N_PREDIV_S; /* convert timeout  in seconds */

	/*convert microsecs to RTC format and add to 'Now' */
	rtcAlarmDays = RTC_DateStruct.Date;
	while (timeoutValue >= SECONDS_IN_1DAY )
	{
		timeoutValue -= SECONDS_IN_1DAY;
		rtcAlarmDays++;
	}

	/* calc hours */
	rtcAlarmHours = RTC_TimeStruct.Hours;
	while (timeoutValue >= SECONDS_IN_1HOUR )
	{
		timeoutValue -= SECONDS_IN_1HOUR;
		rtcAlarmHours++;
	}

	/* calc minutes */
	rtcAlarmMinutes = RTC_TimeStruct.Minutes;
	while (timeoutValue >= SECONDS_IN_1MINUTE )
	{
		timeoutValue -= SECONDS_IN_1MINUTE;
		rtcAlarmMinutes++;
	}

	/* calc seconds */
	rtcAlarmSeconds = RTC_TimeStruct.Seconds + timeoutValue;

	/***** correct for modulo********/
	while (rtcAlarmSubSeconds >= (RTC_PREDIV_S + 1))
	{
		rtcAlarmSubSeconds -= (RTC_PREDIV_S + 1);
		rtcAlarmSeconds++;
	}

	while (rtcAlarmSeconds >= SECONDS_IN_1MINUTE )
	{
		rtcAlarmSeconds -= SECONDS_IN_1MINUTE;
		rtcAlarmMinutes++;
	}

	while (rtcAlarmMinutes >= MINUTES_IN_1HOUR )
	{
		rtcAlarmMinutes -= MINUTES_IN_1HOUR;
		rtcAlarmHours++;
	}

	while (rtcAlarmHours >= HOURS_IN_1DAY )
	{
		rtcAlarmHours -= HOURS_IN_1DAY;
		rtcAlarmDays++;
	}

	if (RTC_DateStruct.Year % 4 == 0)
	{
		if (rtcAlarmDays > DaysInMonthLeapYear[RTC_DateStruct.Month - 1])
		{
			rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[RTC_DateStruct.Month - 1];
		}
	} else
	{
		if (rtcAlarmDays > DaysInMonth[RTC_DateStruct.Month - 1])
		{
			rtcAlarmDays = rtcAlarmDays % DaysInMonth[RTC_DateStruct.Month - 1];
		}
	}

	/* Set RTC_AlarmStructure with calculated values*/
	RTC_AlarmStructure.AlarmTime.SubSeconds = RTC_PREDIV_S - rtcAlarmSubSeconds;
	RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK;
	RTC_AlarmStructure.AlarmTime.Seconds = rtcAlarmSeconds;
	RTC_AlarmStructure.AlarmTime.Minutes = rtcAlarmMinutes;
	RTC_AlarmStructure.AlarmTime.Hours = rtcAlarmHours;
	RTC_AlarmStructure.AlarmDateWeekDay = (uint8_t) rtcAlarmDays;
	RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_TimeStruct.TimeFormat;
	RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;   // RTC_ALARMMASK_DATEWEEKDAY  ; Old: RTC_ALARMMASK_NONE
	RTC_AlarmStructure.Alarm = RTC_ALARM_A;
	RTC_AlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	RTC_AlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

	/* Set RTC_Alarm */
	HAL_RTC_SetAlarm_IT(&hrtc, &RTC_AlarmStructure, RTC_FORMAT_BIN);
}

/*!
 * @brief get current time from calendar in ticks
 * @param pointer to RTC_DateStruct
 * @param pointer to RTC_TimeStruct
 * @retval time in ticks
 */
static uint32_t RTC_GetCalendarValue(RTC_DateTypeDef *RTC_DateStruct,
		RTC_TimeTypeDef *RTC_TimeStruct)
{
	uint32_t calendarValue = 0;
	uint32_t correction;

	/* Get Time and Date*/
	HAL_RTC_GetDate(&hrtc, RTC_DateStruct, RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc, RTC_TimeStruct, RTC_FORMAT_BIN);
	/* calculte amount of elapsed days since 01/01/2000 */
	calendarValue = DIVC((DAYS_IN_YEAR * 3 + DAYS_IN_LEAP_YEAR) * RTC_DateStruct->Year, 4);

	correction = ((RTC_DateStruct->Year % 4) == 0) ?
	DAYS_IN_MONTH_CORRECTION_LEAP :
														DAYS_IN_MONTH_CORRECTION_NORM;

	calendarValue += (DIVC((RTC_DateStruct->Month - 1) * (30 + 31),
			2) - (((correction >> ((RTC_DateStruct->Month - 1) * 2)) & 0x3)));

	calendarValue += (RTC_DateStruct->Date - 1);

	/* convert from days to seconds */
	calendarValue *= SECONDS_IN_1DAY;

	calendarValue += ((uint32_t) RTC_TimeStruct->Seconds
			+ ((uint32_t) RTC_TimeStruct->Minutes * SECONDS_IN_1MINUTE )
			+ ((uint32_t) RTC_TimeStruct->Hours * SECONDS_IN_1HOUR ));

	calendarValue = (calendarValue << RTC_N_PREDIV_S) + (RTC_PREDIV_S - RTC_TimeStruct->SubSeconds);

	return (calendarValue);
}


//Dung tick bang rtc. disable ngat systick
//
//HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
//{
//  /*Don't enable SysTick if TIMER_IF is based on other counters (e.g. RTC) */
//	return HAL_OK;
//}
//
//
//uint32_t HAL_GetTick(void)
//{
//	return RTC_IF_GetTimerValue();
//}
//
//void HAL_Delay(__IO uint32_t Delay)
//{
//	RTC_IF_DelayMs(Delay);   /* based on RTC */
//}
//







