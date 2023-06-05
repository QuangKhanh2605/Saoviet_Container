/*
 8/2021
 Thu vien xu ly ADC
 */

#include "user_adc.h"

/*===================== Var, struct===================== */
uint8_t AdcInitialized = 0;
/*===================== Funcion ===================== */
/*
    Function Read ADC value
    Input: 
        - Channel: kenh da chon doi voi chan ADC
        - rank: Khai bao o trong cube
    Return: Gia tri ADC
*/


void STM32_MX_ADC1_Init(void)
{
    /* USER CODE BEGIN ADC1_Init 0 */    
    ADC_ChannelConfTypeDef sConfig = {0};
    /** Common config
    */
    hadc1.Instance = ADC1;
    
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;

#if defined (STM32L433xx) 
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
#endif

#if defined (STM32L072xx) 
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5; 
    hadc1.Init.LowPowerFrequencyMode = ENABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
#endif
   
#if defined (STM32L151xC) 

    hadc1.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
    hadc1.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
    hadc1.Init.NbrOfConversion = 1;  
#endif
  
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
 
#if defined (STM32L072xx) 
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel to be converted. 
    */
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
#endif
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

  
  

uint16_t HW_AdcReadChannel (uint32_t Channel)
{
    ADC_ChannelConfTypeDef  sConfig;
    uint16_t adcData = 0;

	STM32_MX_ADC1_Init();

    #if defined (STM32L433xx) || defined (STM32L151xC) 
  
        #if defined (STM32L433xx)
            /*calibrate ADC if any calibraiton hardware*/
            HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED );           
            /*
                * Configure Regular Channel 
            */
        #endif
        sConfig.Channel = Channel;
        sConfig.Rank = ADC_REGULAR_RANK_1;
        #if defined (STM32L433xx)
            sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
        
            sConfig.SingleDiff = ADC_SINGLE_ENDED;
            sConfig.OffsetNumber = ADC_OFFSET_NONE;
            sConfig.Offset = 0;
        #endif
 
        #if defined (STM32L151xC)
            sConfig.SamplingTime = ADC_SAMPLETIME_384CYCLES;
        #endif
        if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
        {
            Error_Handler();
        }
  
        /* Start the conversion process */
        HAL_ADC_Start(&hadc1);
          
         /* Wait for the end of conversion */
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY ); 
        /* Get the converted value of regular channel */
        adcData = HAL_ADC_GetValue(&hadc1);
        
        HAL_ADC_Stop (&hadc1);
        #if defined (STM32L433xx)   
            ADC_Disable( &hadc1);
        #endif
        
    #endif
               
    #if defined (STM32L072xx) 
        
         /* wait the the Vrefint used by adc is set */
        while (__HAL_PWR_GET_FLAG(PWR_FLAG_VREFINTRDY) == RESET) {};
          
        __HAL_RCC_ADC1_CLK_ENABLE() ;
        
        /*calibrate ADC if any calibraiton hardware*/
        HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED );
        
        /* Deselects all channels*/
        sConfig.Channel = ADC_CHANNEL_MASK;
        sConfig.Rank = ADC_RANK_NONE; 
        HAL_ADC_ConfigChannel( &hadc1, &sConfig);
          
        /* configure adc channel */
        sConfig.Channel = Channel;
        sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
        HAL_ADC_ConfigChannel( &hadc1, &sConfig);

        /* Start the conversion process */
        HAL_ADC_Start( &hadc1);
          
        /* Wait for the end of conversion */
        HAL_ADC_PollForConversion( &hadc1, HAL_MAX_DELAY );
          
        HAL_ADC_Stop(&hadc1) ;   /* it calls also ADC_Disable() */
        /* Get the converted value of regular channel */
        adcData = HAL_ADC_GetValue ( &hadc1);

        HAL_ADC_DeInit(&hadc1);

        __HAL_RCC_ADC1_CLK_DISABLE() ;
    #endif
    
    return adcData;
}

void ADC_Desequence_Powerhungry_Channels(void)
{
#if defined (STM32L072xx) || defined (STM32L082xx)
     ADC_ChannelConfTypeDef sConfig;
      
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_RANK_NONE; 
    HAL_ADC_ConfigChannel (&hadc1, &sConfig);

    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = ADC_RANK_NONE; 
    HAL_ADC_ConfigChannel (&hadc1, &sConfig);
#endif
}


/*
    Function Doc Gia tri PIN (mV)
    Input:
        - Channel
        - rank
    return: Gia tri PIN (mv)
*/

uint32_t HW_GetBatteryLevel (void)
{
	uint16_t measuredLevel = 0;
	uint32_t batteryLevelmV = 0;

	measuredLevel = HW_AdcReadChannel( ADC_CHANNEL_VREFINT);

	if (measuredLevel == 0)
	{
		batteryLevelmV = 0;
	} else
	{
		batteryLevelmV = (((uint32_t) VDDA_VREFINT_CAL * (*VREFINT_CAL)) / measuredLevel);
	}

	return batteryLevelmV;
}


uint32_t Get_Value_ADC (uint32_t Channel)
{
	uint32_t Bat_Voltage_u32 = 0;
	uint32_t Value_ADC_u32 = 0;
	uint32_t Temp_mV = 0;
    
    Value_ADC_u32 = HW_AdcReadChannel(Channel);
	//lay dien ap pin ve day la dien ap tham chieu
	Bat_Voltage_u32 = HW_GetBatteryLevel();
	//quy doi sang mV cho gia tri ADC doc ve
	Temp_mV = (Bat_Voltage_u32 * Value_ADC_u32) / 4095;

	return Temp_mV;
}

uint16_t Get_Level_Voltage (uint32_t Vol_mV, uint32_t VMAX, uint32_t VMIN)
{
    uint16_t batteryLevel = 0;
    
    if (Vol_mV > VMAX)
        batteryLevel = LEVEL_MAX_BAT;
    else if (Vol_mV < VMIN)
        batteryLevel = 0;
	else
        batteryLevel = ( ( (uint32_t) (Vol_mV - VMIN) * LEVEL_MAX_BAT ) / (VMAX - VMIN) ); 

	return batteryLevel;
}
   











