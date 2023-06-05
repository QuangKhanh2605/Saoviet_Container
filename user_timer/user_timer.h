/*
    8/2021
    Thu vien xu ly Uart
*/
#ifndef USER_TIMER_H
#define USER_TIMER_H

#include <stdbool.h>


#include "user_util.h"

/*======================== Define ======================*/

    
/*======================== Structs var======================*/

/**
  * @brief Timer mode
  */
typedef enum {
  UTIL_TIMER_ONESHOT  = 0, /*!<One-shot timer. */
  UTIL_TIMER_PERIODIC = 1  /*!<Periodic timer. */
} UTIL_TIMER_Mode_t;


/**
  * @brief Timer status
  */
typedef enum {
  UTIL_TIMER_OK            = 0,  /*!<Operation terminated successfully.*/
  UTIL_TIMER_INVALID_PARAM = 1,  /*!<Invalid Parameter.                */
  UTIL_TIMER_HW_ERROR      = 2,  /*!<Hardware Error.                   */
  UTIL_TIMER_UNKNOWN_ERROR = 3   /*!<Unknown Error.                    */
} UTIL_TIMER_Status_t;

/**
  * @brief Timer object description
  */
typedef struct TimerEvent_s
{
    uint32_t Timestamp;           /*!<Expiring timer value in ticks from TimerContext */
    uint32_t ReloadValue;         /*!<Reload Value when Timer is restarted            */
    uint8_t IsPending;            /*!<Is the timer waiting for an event               */
    uint8_t IsRunning;            /*!<Is the timer running                            */
    uint8_t IsReloadStopped;      /*!<Is the reload stopped                           */
    UTIL_TIMER_Mode_t Mode;       /*!<Timer type : one-shot/continuous                */
    void ( *Callback )( void *);  /*!<callback function                               */
    void *argument;               /*!<callback argument                               */
	struct TimerEvent_s *Next;    /*!<Pointer to the next Timer object.               */
} UTIL_TIMER_Object_t;

/**
  * @brief Timer driver definition
  */
typedef struct
{
    UTIL_TIMER_Status_t   (* InitTimer )( void );                  /*!< Initialisation of the low layer timer    */
    UTIL_TIMER_Status_t   (* DeInitTimer )( void );                /*!< Un-Initialisation of the low layer timer */

    UTIL_TIMER_Status_t   (* StartTimerEvt )( uint32_t timeout );  /*!< Start the low layer timer */
    UTIL_TIMER_Status_t   (* StopTimerEvt )( void);                /*!< Stop the low layer timer */

    uint32_t              (* SetTimerContext)( void );             /*!< Set the timer context */
    uint32_t              (* GetTimerContext)( void );             /*!< Get the timer context */

    uint32_t              (* GetTimerElapsedTime)( void );         /*!< Get elapsed time */
    uint32_t              (* GetTimerValue)( void );               /*!< Get timer value */
    uint32_t              (* GetMinimumTimeout)( void );           /*!< Get Minimum timeout */

    uint32_t              (* ms2Tick)( uint32_t timeMicroSec );    /*!< convert ms to tick */
    uint32_t              (* Tick2ms)( uint32_t tick );            /*!< convert tick into ms */
} UTIL_TIMER_Driver_s;

/**
  * @brief Timer value on 32 bits
  */
typedef uint32_t UTIL_TIMER_Time_t;


/*======================== External Var struct ======================*/
extern const UTIL_TIMER_Driver_s UTIL_TimerDriver;


/*======================== Function ======================*/

UTIL_TIMER_Status_t 	UTIL_TIMER_Init(void);
UTIL_TIMER_Status_t 	UTIL_TIMER_DeInit(void);
UTIL_TIMER_Status_t 	UTIL_TIMER_Create( UTIL_TIMER_Object_t *TimerObject, uint32_t PeriodValue,  \
														UTIL_TIMER_Mode_t Mode, void ( *Callback )( void *), void *Argument);

UTIL_TIMER_Status_t 	UTIL_TIMER_Start( UTIL_TIMER_Object_t *TimerObject);
UTIL_TIMER_Status_t 	UTIL_TIMER_StartWithPeriod( UTIL_TIMER_Object_t *TimerObject, uint32_t PeriodValue);
UTIL_TIMER_Status_t 	UTIL_TIMER_Stop( UTIL_TIMER_Object_t *TimerObject );
UTIL_TIMER_Status_t 	UTIL_TIMER_SetPeriod(UTIL_TIMER_Object_t *TimerObject, uint32_t NewPeriodValue);
UTIL_TIMER_Status_t 	UTIL_TIMER_SetReloadMode(UTIL_TIMER_Object_t *TimerObject, UTIL_TIMER_Mode_t ReloadMode);
UTIL_TIMER_Status_t 	UTIL_TIMER_GetRemainingTime(UTIL_TIMER_Object_t *TimerObject, uint32_t *ElapsedTime);
uint32_t 				UTIL_TIMER_IsRunning( UTIL_TIMER_Object_t *TimerObject );
uint32_t 				UTIL_TIMER_GetFirstRemainingTime(void);
void 					UTIL_TIMER_IRQ_Handler( void );
UTIL_TIMER_Time_t 		UTIL_TIMER_GetCurrentTime(void);
UTIL_TIMER_Time_t 		UTIL_TIMER_GetElapsedTime(UTIL_TIMER_Time_t past );
bool 					TimerExists( UTIL_TIMER_Object_t *TimerObject );
void 					TimerSetTimeout( UTIL_TIMER_Object_t *TimerObject );
void 					TimerInsertTimer( UTIL_TIMER_Object_t *TimerObject);
void 					TimerInsertNewHeadTimer( UTIL_TIMER_Object_t *TimerObject );

void 					UTIL_Set_RTC (ST_TIME_FORMAT sTime);

#endif
