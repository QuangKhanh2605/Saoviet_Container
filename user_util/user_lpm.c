/*
 * user_lp.c
 *
 *  Created on: Dec 14, 2021
 *      Author: Chien
 */

#include "user_lpm.h"

/**
 * @brief macro used to initialized the critical section
 */
#ifndef UTIL_LPM_INIT_CRITICAL_SECTION
#define UTIL_LPM_INIT_CRITICAL_SECTION( )
#endif

/**
 * @brief macro used to enter the critical section
 */
#ifndef UTIL_LPM_ENTER_CRITICAL_SECTION
#define UTIL_LPM_ENTER_CRITICAL_SECTION( )    UTILS_ENTER_CRITICAL_SECTION( )
#endif

/**
 * @brief macro used to exit the critical section
 */
#ifndef UTIL_LPM_EXIT_CRITICAL_SECTION
#define UTIL_LPM_EXIT_CRITICAL_SECTION( )     UTILS_EXIT_CRITICAL_SECTION( )
#endif


extern void SystemClock_Config(void);
/*===================Varialble, struct=================================*/

static uint8_t StopModeDisable = LPM_FALSE;

const struct UTIL_LPM_Driver_s UTIL_PowerDriver =
{
	PWR_EnterSleepMode,
	PWR_ExitSleepMode,

	PWR_EnterStopMode,
	PWR_ExitStopMode,
    
    PWR_EnterStandbyMode,
    PWR_ExitStandbyMode,
};


StructpFuncLPM    *sLPMFeatureHandler = NULL;


/*=======================================Function===========================================*/
void UTIL_LPM_Init(void)
{
	StopModeDisable = LPM_TRUE;
	UTIL_LPM_Dis_Debug_STlink();
}

void UTIL_LPM_DeInit(void)
{

}

void UTIL_LPM_SetStopMode(UTIL_LPM_State_t state)
{
	StopModeDisable = state;
}

UTIL_LPM_Mode_t UTIL_LPM_GetMode(void)
{
	UTIL_LPM_Mode_t mode_selected;

	if (StopModeDisable != LPM_FALSE)
		mode_selected = UTIL_LPM_SLEEPMODE;
	else
		mode_selected = UTIL_LPM_STOPMODE;

	return mode_selected;
}

void UTIL_LPM_EnterLowPower(void)
{    
	if (StopModeDisable != LPM_FALSE)
	{
		UTIL_PowerDriver.EnterSleepMode();
		UTIL_PowerDriver.ExitSleepMode();
	} else
	{
        UTIL_Printf_Str (DBLEVEL_M, "u_lpm: go to stopmode\r\n" );
        
        if (sLPMFeatureHandler->LPM_Deinit_Peripheral != NULL)
            sLPMFeatureHandler->LPM_Deinit_Peripheral();
        
        //Con lora them 1 cai nua: de init IO radio truoc khi thuc hien ngat
        UTILS_ENTER_CRITICAL_SECTION();   
        
		UTIL_PowerDriver.EnterStopMode();
		UTIL_PowerDriver.ExitStopMode();
        
        //Con lora them 1 cai nua: de init IO radio truoc khi thuc hien ngat
        UTILS_EXIT_CRITICAL_SECTION();   
        
        if (sLPMFeatureHandler->LPM_Init_Peripheral != NULL)
            sLPMFeatureHandler->LPM_Init_Peripheral();       
	}
}

void UTIL_LPM_EnterStandbyMode(void)
{
    UTIL_PowerDriver.EnterStopMode();
    UTIL_PowerDriver.ExitStopMode();
}



//Ham xu ly lpm trong chip
void PWR_EnterStopMode(void)
{    
    UTILS_ENTER_CRITICAL_SECTION();
    
    if (sLPMFeatureHandler->LPM_Deinit_Before_IRQ_Handle!= NULL)
        sLPMFeatureHandler->LPM_Deinit_Before_IRQ_Handle();  
        
#if defined (STM32L072xx) || defined (STM32L082xx) || defined (STM32L151xC) 
    /*clear wake up flag*/
    SET_BIT(PWR->CR, PWR_CR_CWUF);

    HAL_PWREx_EnableUltraLowPower();

    HAL_PWREx_EnableFastWakeUp(); 

    UTILS_EXIT_CRITICAL_SECTION();
    /* Enter Stop Mode */
    HAL_PWR_EnterSTOPMode ( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
#endif
    
#if defined (STM32L433xx) 
	/*clear wake up flag*/
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	UTILS_EXIT_CRITICAL_SECTION();
	/* Enter Stop Mode */
	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);  // HAL_PWREx_EnterSTOP2Mode
#endif
    
}


void PWR_ExitStopMode(void)
{
	/* Disable IRQ while the MCU is not running on HSI */
	UTILS_ENTER_CRITICAL_SECTION();

#if defined (STM32L072xx) || defined (STM32L082xx) || defined (STM32L151xC) 
    HAL_PWREx_DisableUltraLowPower();
	
    HAL_PWREx_DisableFastWakeUp();
#endif
  
#ifdef DEVICE_TYPE_GATEWAY
	/* After wake-up from STOP reconfigure the system clock */
	/* Enable HSI */
	__HAL_RCC_HSI_ENABLE();

	/* Wait till HSI is ready */
	while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET)
	{
	}

	/* Enable PLL */
	__HAL_RCC_PLL_ENABLE();
	/* Wait till PLL is ready */
	while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source */
	__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);

	/* Wait till PLL is used as system clock source */
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK)
	{

	}
#else
    
	/* After wake-up from STOP reconfigure the system clock */
	/* Enable HSI */
	__HAL_RCC_MSI_ENABLE();

	/* Wait till HSI is ready */
	while (__HAL_RCC_GET_FLAG(RCC_FLAG_MSIRDY) == RESET)
	{
	}

	/* Select PLL as system clock source */
	__HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_MSI);

	/* Wait till PLL is used as system clock source */
	while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_MSI)
	{

	}
    
#endif
         
    if (sLPMFeatureHandler->LPM_Init_Before_IRQ_Handle!= NULL)
        sLPMFeatureHandler->LPM_Init_Before_IRQ_Handle();  
        
	UTILS_EXIT_CRITICAL_SECTION();  
}


void PWR_EnterStandbyMode( void)
{
    UTILS_ENTER_CRITICAL_SECTION();

    if (sLPMFeatureHandler->LPM_Deinit_Peripheral != NULL)
        sLPMFeatureHandler->LPM_Deinit_Peripheral();

    UTILS_EXIT_CRITICAL_SECTION();

    /* Enter Standby Mode */
    HAL_PWR_EnterSTANDBYMode ();
}


void PWR_ExitStandbyMode( void)
{
    UTILS_ENTER_CRITICAL_SECTION();

    SystemClock_Config();
      
    if (sLPMFeatureHandler->LPM_Init_Peripheral != NULL)
        sLPMFeatureHandler->LPM_Init_Peripheral();
    
    UTILS_EXIT_CRITICAL_SECTION();
}



void PWR_EnterSleepMode(void)
{
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

void PWR_ExitSleepMode(void)
{
	HAL_ResumeTick();
}

void UTIL_LPM_Dis_Debug_STlink (void)
{
	HAL_DBGMCU_DisableDBGSleepMode( );
	HAL_DBGMCU_DisableDBGStopMode( );
	HAL_DBGMCU_DisableDBGStandbyMode( );
}


