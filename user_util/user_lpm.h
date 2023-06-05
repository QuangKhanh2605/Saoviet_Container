/*
 * user_lpm.h
 *
 *  Created on: Dec 14, 2021
 *      Author: Chien
 */

#ifndef INC_USER_LPM_H_
#define INC_USER_LPM_H_


#include "user_util.h"

/*======================== Structs var======================*/

#define 	LPM_TRUE	1
#define 	LPM_FALSE	0

typedef enum
{
  UTIL_LPM_ENABLE = 0,
  UTIL_LPM_DISABLE,
} UTIL_LPM_State_t;

typedef enum
{
  UTIL_LPM_SLEEPMODE,
  UTIL_LPM_STOPMODE,
  UTIL_LPM_OFFMODE,
} UTIL_LPM_Mode_t;


struct UTIL_LPM_Driver_s
{
  void (*EnterSleepMode) ( void ); /*!<function to enter the sleep mode */
  void (*ExitSleepMode) ( void );  /*!<function to exit the sleep mode  */
  void (*EnterStopMode) ( void );  /*!<function to enter the stop mode  */
  void (*ExitStopMode) ( void );   /*!<function to exit the stop mode   */
  void (*EnterStandbyMode) ( void );  /*!<function to enter the standby mode  */
  void (*ExitStandbyMode) ( void );   /*!<function to exit the standby mode   */
};



typedef struct 
{
    void (*LPM_Deinit_Peripheral) ( void );
    void (*LPM_Init_Peripheral) ( void );
    void (*LPM_Deinit_Before_IRQ_Handle) ( void );
    void (*LPM_Init_Before_IRQ_Handle) ( void );
}StructpFuncLPM;


extern StructpFuncLPM    *sLPMFeatureHandler;


/*======================== Function======================*/
void 	UTIL_LPM_Init(void);
void 	UTIL_LPM_DeInit(void);
void 	UTIL_LPM_SetStopMode(UTIL_LPM_State_t state);
UTIL_LPM_Mode_t 	UTIL_LPM_GetMode(void);

void 	PWR_EnterStopMode(void);
void 	PWR_ExitStopMode(void);
void 	PWR_EnterSleepMode(void);
void 	PWR_ExitSleepMode(void);
void    PWR_ExitStandbyMode( void);
void    PWR_EnterStandbyMode( void);

void 	UTIL_LPM_EnterLowPower(void);
void 	UTIL_LPM_Dis_Debug_STlink (void);

void    UTIL_LPM_EnterStandbyMode(void);


#endif /* INC_USER_LPM_H_ */
