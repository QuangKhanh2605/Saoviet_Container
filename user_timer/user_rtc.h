/*
    8/2021
    Thu vien cau hinh rtc
*/
#ifndef USER_RTC_H
#define USER_RTC_H

#include "user_util.h"
#include "user_timer.h"

/*======================== Define ======================*/
#ifndef RTC_N_PREDIV_S
    #define RTC_N_PREDIV_S 			10
    #define RTC_PREDIV_S 			((1 << RTC_N_PREDIV_S) - 1)
    #define RTC_PREDIV_A 			((1 << (15-RTC_N_PREDIV_S)) - 1)
#endif

#define MIN_ALARM_DELAY               3 /* in ticks */

/* RTC Ticks/ms conversion */
#define RTC_BKP_SECONDS    		RTC_BKP_DR0
#define RTC_BKP_SUBSECONDS 		RTC_BKP_DR1
#define RTC_BKP_MSBTICKS   		RTC_BKP_DR2

/* Sub-second mask definition  */
#define RTC_ALARMSUBSECONDMASK    RTC_N_PREDIV_S<<RTC_ALRMASSR_MASKSS_Pos

/* RTC Time base in us */
#define USEC_NUMBER             1000000
#define MSEC_NUMBER             (USEC_NUMBER / 1000)

#define COMMON_FACTOR        	3
#define CONV_NUMER              (MSEC_NUMBER >> COMMON_FACTOR)
#define CONV_DENOM              (1 << (RTC_N_PREDIV_S - COMMON_FACTOR))

/*======================== Define ======================*/
#ifndef DAYS_IN_LEAP_YEAR
    #define DAYS_IN_LEAP_YEAR                       ( ( uint32_t )  366U )
    #define DAYS_IN_YEAR                            ( ( uint32_t )  365U )
    #define SECONDS_IN_1DAY                         ( ( uint32_t )86400U )
    #define SECONDS_IN_1HOUR                        ( ( uint32_t ) 3600U )
    #define SECONDS_IN_1MINUTE                      ( ( uint32_t )   60U )
    #define MINUTES_IN_1HOUR                        ( ( uint32_t )   60U )
    #define HOURS_IN_1DAY                           ( ( uint32_t )   24U )
    
    #define DIVC( X, N )                            ( ( ( X ) + ( N ) -1 ) / ( N ) )
#endif

#define DAYS_IN_MONTH_CORRECTION_NORM           ((uint32_t) 0x99AAA0 )
#define DAYS_IN_MONTH_CORRECTION_LEAP           ((uint32_t) 0x445550 )

/*======================== Structs var======================*/

typedef struct
{
	uint32_t Rtc_Time; /* Reference time */
	RTC_TimeTypeDef RTC_Calndr_Time; /* Reference time in calendar format */
	RTC_DateTypeDef RTC_Calndr_Date; /* Reference date in calendar format */
} RtcTimerContext_t;

/*======================== External Var struct ======================*/


/*======================== Function ======================*/
/*!
 * @brief Init RTC hardware
 * @param none
 * @retval none
 */
UTIL_TIMER_Status_t RTC_IF_Init(void);

/*!
 * @brief Set the alarm
 * @note The alarm is set at timeout from timer Reference (TimerContext)
 * @param timeout Duration of the Timer in ticks
 */
UTIL_TIMER_Status_t RTC_IF_StartTimer(uint32_t timeout);

/*!
 * @brief Stop the Alarm
 * @param none
 * @retval none
 */
UTIL_TIMER_Status_t RTC_IF_StopTimer(void);

/*!
 * @brief set timer Reference (TimerContext)
 * @param none
 * @retval  Timer Reference Value in  Ticks
 */
uint32_t RTC_IF_SetTimerContext(void);

/*!
 * @brief Get the RTC timer Reference
 * @param none
 * @retval Timer Value in  Ticks
 */
uint32_t RTC_IF_GetTimerContext(void);

/*!
 * @brief Get the timer elapsed time since timer Reference (TimerContext) was set
 * @param none
 * @retval RTC Elapsed time in ticks
 */
uint32_t RTC_IF_GetTimerElapsedTime(void);

/*!
 * @brief Get the timer value
 * @param none
 * @retval RTC Timer value in ticks
 */
uint32_t RTC_IF_GetTimerValue(void);

/*!
 * @brief Return the minimum timeout in ticks the RTC is able to handle
 * @param none
 * @retval minimum value for a timeout in ticks
 */
uint32_t RTC_IF_GetMinimumTimeout(void);

/*!
 * @brief a delay of delay ms by polling RTC
 * @param delay in ms
 * @retval none
 */
void RTC_IF_DelayMs(uint32_t delay);

/*!
 * @brief converts time in ms to time in ticks
 * @param [IN] time in milliseconds
 * @retval returns time in timer ticks
 */
uint32_t RTC_IF_Convert_ms2Tick(uint32_t timeMilliSec);

/*!
 * @brief converts time in ticks to time in ms
 * @param [IN] time in timer ticks
 * @retval returns time in timer milliseconds
 */
uint32_t RTC_IF_Convert_Tick2ms(uint32_t tick);

/*!
 * @brief Get rtc time
 * @param [OUT] subSeconds in ticks
 * @retval returns time seconds
 */
uint32_t RTC_IF_GetTime(uint16_t *subSeconds);

/*!
 * @brief write seconds in backUp register
 * @Note Used to store seconds difference between RTC time and Unix time
 * @param [IN] time in seconds
 * @retval None
 */
void RTC_IF_BkUp_Write_Seconds(uint32_t Seconds);

/*!
 * @brief reads seconds from backUp register
 * @Note Used to store seconds difference between RTC time and Unix time
 * @param [IN] None
 * @retval Time in seconds
 */
uint32_t RTC_IF_BkUp_Read_Seconds(void);

/*!
 * @brief writes SubSeconds in backUp register
 * @Note Used to store SubSeconds difference between RTC time and Unix time
 * @param [IN] time in SubSeconds
 * @retval None
 */
void RTC_IF_BkUp_Write_SubSeconds(uint32_t SubSeconds);

/*!
 * @brief reads SubSeconds from backUp register
 * @Note Used to store SubSeconds difference between RTC time and Unix time
 * @param [IN] None
 * @retval Time in SubSeconds
 */
uint32_t RTC_IF_BkUp_Read_SubSeconds(void);


uint32_t 	HAL_GetTick(void);
void 		HAL_Delay(__IO uint32_t Delay);

extern void (*pModemProcessIRQTimer) (void);

#endif



