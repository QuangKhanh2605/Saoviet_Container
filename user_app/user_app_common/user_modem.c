/*
    8/2021
    Thu vien Quan ly Modem
*/

#include "user_modem.h"

#include "user_log.h"

#include "user_rtc.h"
#include "onchipflash.h"
#include "user_adc.h"

/*===================Struct, Var=========================*/
ST_TIME_FORMAT          sRTCSet = {0};

/*================ struct Feature=====================*/
uint8_t aDCU_ID[MAX_DCU_ID_LENGTH] = DEVICE_ID;

#ifdef MEMORY_ON_FLASH
    sRecordMemoryManager     sRecTSVH = 
    {
        .AddAStart_u32      = ADDR_MESS_A_START,   
        .AddAStop_u32       = ADDR_MESS_A_STOP,
        
        .Offset_u32         = 0,
        
        .AddBStart_u32      = ADDR_MESS_A_START,   
        .AddBStop_u32       = ADDR_MESS_A_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_TSVH_SEND,
        .AddIndexSave_u32   = ADDR_INDEX_TSVH_SAVE,
        
        .MaxRecord_u16      = FLASH_MAX_MESS_TSVH_SAVE, 
        .SizeRecord_u16     = SIZE_DATA_TSVH, 
    };
    
    sRecordMemoryManager     sRecEvent = 
    {
        .AddAStart_u32      = ADDR_EVENT_START,
        .AddAStop_u32       = ADDR_EVENT_STOP,
            
        .Offset_u32         = 0,
        
        .AddBStart_u32      = ADDR_EVENT_START,   
        .AddBStop_u32       = ADDR_MESS_A_STOP,
        
        .AddIndexSend_u32   = ADDR_INDEX_EVENT_SEND,
        .AddIndexSave_u32   = ADDR_INDEX_EVENT_SAVE,
        
        .MaxRecord_u16      = FLASH_MAX_MESS_EVENT_SAVE,
        .SizeRecord_u16     = SIZE_DATA_EVENT,
    };
    
    //Struct Log Ðata
    sRecordMemoryManager     sRecLog =    
    {
        .AddAStart_u32       = ADDR_LOG_START,   
        .AddAStop_u32        = ADDR_LOG_STOP,
        
        .Offset_u32         = 0,
        
        .AddBStart_u32       = ADDR_LOG_START,   
        .AddBStop_u32        = ADDR_LOG_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_LOG_SEND,   
        .AddIndexSave_u32   = ADDR_INDEX_LOG_SAVE,
        
        .MaxRecord_u16      = FLASH_MAX_MESS_LOG_SAVE,     
        .SizeRecord_u16     = SIZE_DATA_LOG,
    };
#endif
    
#ifdef EX_MEM_EEPROM
    sRecordMemoryManager     sRecTSVH = 
    {
        .AddAStart_u32      = CAT_ADDR_MESS_A_START,                      
        .AddAStop_u32       = CAT_ADDR_MESS_A_STOP,
        
        .Offset_u32         = 0,
        
        .AddBStart_u32      = CAT_ADDR_MESS_A_START,                      
        .AddBStop_u32       = CAT_ADDR_MESS_A_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_TSVH_SEND,     //luu trong flash noi      
        .AddIndexSave_u32   = ADDR_INDEX_TSVH_SAVE,     //luu trong flash noi  
            
        .MaxRecord_u16      = CAT_MAX_MESS_TSVH_SAVE,  
        .SizeRecord_u16     = CAT_SIZE_DATA_TSVH, 
    };
    // Struct Event Data
    sRecordMemoryManager     sRecEvent = 
    {
        .AddAStart_u32      = CAT_ADDR_EVENT_START,
        .AddAStop_u32       = CAT_ADDR_EVENT_STOP,
        
        .Offset_u32         = 0,
        
        .AddBStart_u32      = CAT_ADDR_EVENT_START,
        .AddBStop_u32       = CAT_ADDR_EVENT_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_EVENT_SEND,    //luu trong flash noi  
        .AddIndexSave_u32   = ADDR_INDEX_EVENT_SAVE,    //luu trong flash noi  
        
        .MaxRecord_u16      = CAT_MAX_MESS_EVENT_SAVE,
        .SizeRecord_u16     = CAT_SIZE_DATA_EVENT, 
    };
    //Struct Log Ðata: luu trong onflash nhung index luu trong exmem
    sRecordMemoryManager     sRecLog = 
    {
        .AddAStart_u32      = CAT_ADDR_LOG_START,   
        .AddAStop_u32       = CAT_ADDR_LOG_STOP,
        
        .Offset_u32         = 0,
        
        .AddBStart_u32      = CAT_ADDR_LOG_START,   
        .AddBStop_u32       = CAT_ADDR_LOG_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_LOG_SEND,     
        .AddIndexSave_u32   = ADDR_INDEX_LOG_SAVE,
        
        .MaxRecord_u16      = CAT_MAX_MESS_LOG_SAVE,
        .SizeRecord_u16     = CAT_SIZE_DATA_LOG,
    };
    
    //Struct GPS Data
    sRecordMemoryManager     sRecGPS =    
    {
        .AddAStart_u32      = CAT_ADDR_GPS_START,   
        .AddAStop_u32       = CAT_ADDR_GPS_STOP,
        
        .Offset_u32         = 0,
        
        .AddBStart_u32      = CAT_ADDR_GPS_START,   
        .AddBStop_u32       = CAT_ADDR_GPS_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_GPS_SEND,   
        .AddIndexSave_u32   = ADDR_INDEX_GPS_SAVE,
        
        .SizeRecord_u16     = CAT_SIZE_DATA_GPS,
        .MaxRecord_u16      = CAT_MAX_MESS_GPS_SAVE,     
        
        .IndexSend_u16      = 0,
        .IndexSave_u16      = 0,
    };
#endif

#ifdef EX_MEM_FLASH
     
    sRecordMemoryManager     sRecTSVH = 
    {
        .AddA_u32           = FLASH_ADDR_TSVH_A_START,
        .AddB_u32           = FLASH_ADDR_TSVH_B_START,
        
        .Offset_u32         = FLASH_ADDR_TSVH_OFFSET,
            
        .AddAStart_u32      = FLASH_ADDR_TSVH_A_START,   
        .AddAStop_u32       = FLASH_ADDR_TSVH_A_STOP,
        
        .AddBStart_u32      = FLASH_ADDR_TSVH_B_START,   
        .AddBStop_u32       = FLASH_ADDR_TSVH_B_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_TSVH_SEND,     //luu trong flash noi      
        .AddIndexSave_u32   = ADDR_INDEX_TSVH_SAVE,     //luu trong flash noi 
        
        .SizeRecord_u16     = FLASH_SIZE_DATA_TSVH, 
        .MaxRecord_u16      = FLASH_MAX_RECORD_TSVH, 
        
        .IndexSend_u16      = 0,
        .IndexSave_u16      = 0,
    };
    
    sRecordMemoryManager     sRecEvent = 
    {
        .AddA_u32           = FLASH_ADDR_EVENT_A_START,
        .AddB_u32           = FLASH_ADDR_EVENT_B_START,
        
        .Offset_u32         = FLASH_ADDR_EVENT_OFFSET,
            
        .AddAStart_u32      = FLASH_ADDR_EVENT_A_START,   
        .AddAStop_u32       = FLASH_ADDR_EVENT_A_STOP,
        
        .AddBStart_u32      = FLASH_ADDR_EVENT_B_START,   
        .AddBStop_u32       = FLASH_ADDR_EVENT_B_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_EVENT_SEND,    //luu trong flash noi  
        .AddIndexSave_u32   = ADDR_INDEX_EVENT_SAVE,    //luu trong flash noi  
        
        .SizeRecord_u16     = FLASH_SIZE_DATA_EVENT, 
        .MaxRecord_u16      = FLASH_MAX_RECORD_EVENT, 
        
        .IndexSend_u16      = 0,
        .IndexSave_u16      = 0,
    };
    
    sRecordMemoryManager     sRecLog = 
    {
        .AddA_u32           = FLASH_ADDR_LOG_START,
        .AddB_u32           = FLASH_ADDR_LOG_START,
        
        .Offset_u32         = 0,
            
        .AddAStart_u32      = FLASH_ADDR_LOG_START,   
        .AddAStop_u32       = FLASH_ADDR_LOG_STOP,
        
        .AddBStart_u32      = FLASH_ADDR_LOG_START,   
        .AddBStop_u32       = FLASH_ADDR_LOG_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_LOG_SEND,     
        .AddIndexSave_u32   = ADDR_INDEX_LOG_SAVE,
        
        .SizeRecord_u16     = FLASH_SIZE_DATA_LOG, 
        .MaxRecord_u16      = FLASH_MAX_RECORD_LOG, 
        
        .IndexSend_u16      = 0,
        .IndexSave_u16      = 0,
    };

    //Struct GPS Data
    sRecordMemoryManager     sRecGPS =    
    {
        .AddAStart_u32      = FLASH_ADDR_GPS_A_START,   
        .AddAStop_u32       = FLASH_ADDR_GPS_A_STOP,
        
        .Offset_u32         = FLASH_ADDR_GPS_OFFSET,
        
        .AddBStart_u32      = FLASH_ADDR_GPS_B_START,   
        .AddBStop_u32       = FLASH_ADDR_GPS_B_STOP,
            
        .AddIndexSend_u32   = ADDR_INDEX_GPS_SEND,   
        .AddIndexSave_u32   = ADDR_INDEX_GPS_SAVE,
        
        .SizeRecord_u16     = FLASH_SIZE_DATA_GPS,
        .MaxRecord_u16      = FLASH_MAX_RECORD_GPS,     
        
        .IndexSend_u16      = 0,
        .IndexSave_u16      = 0,
    };
    
#endif 
      
uint8_t aRESPONDSE_AT [LEGNTH_AT_SERIAL];
uint8_t aAT_CMD [LEGNTH_AT_SERIAL];
uint8_t aALRM_EMERGENCY [MAX_LENGTH_ALAMR_EMER];
  
Struct_Modem_Variable	sModem = 
{
    .sDCU_id    = {&aDCU_ID[0], 14},
    .strATResp  = {&aRESPONDSE_AT[0], 0},
    .strATCmd   = {&aAT_CMD[0], 0},
    .strAlarmEmer = {&aALRM_EMERGENCY[0], 0},
};

   
SModemFreqActionInformation     sFreqInfor = 
{
    .NumWakeup_u8           = DEFAULT_NUMBER_WAKE_UP,  
    .FreqWakeup_u32         = DEFAULT_FREQ_WAKEUP,
        
    .FreqSendOnline_u32     = DEFAULT_FREQ_SEND_ONLINE,
    .FreqSendUnitMin_u32    = 2,
}; 

StructpFuncLPM sModemLPMHandler = 
{
    .LPM_Deinit_Peripheral = Modem_Deinit_Peripheral,  
    .LPM_Init_Peripheral = Modem_Init_Peripheral,
    .LPM_Deinit_Before_IRQ_Handle = Modem_Deinit_Before_IRQ_Handle,
    .LPM_Init_Before_IRQ_Handle = Modem_Init_Before_IRQ_Handle,
};
    
/*===================Function=========================*/    
uint8_t Reset_Chip (void)
{
//    if (sModem.IsOverFivePulse_u8 == TRUE)
//    {
    	Modem_Log_And_Send_Emergency((uint8_t *) "Modem: Reset Chip\r\n", 19);
//		//Get Pulse
//    	LPTIM_Counter_Get_Pulse();
//		Modem_Check_Add_5Pulse();
//    	//Save Mark Soft Reset MCU value 0xA5A5A5A5 to Add RAM: 0x2000FC40
//    	sModem.MarkResetChip_u8 = 0xA5A5A5A5;
//
//    	__asm("ldr r0, = pMarkSoftResetMCU");
//		__asm("ldr r1, [r0, 0]");
//		__asm("ldr r2, [r1, 0]");
//
//		__asm("ldr r0, = 0x2000F840");
//		__asm("str r2, [r0]");
//
//		//Save Pulse to Add RAM: 0x2000FC4A
//		__asm("ldr r0, = pPulseLPTIM");
//		__asm("ldr r1, [r0, 0]");
//		__asm("ldr r2, [r1, 0]");
//
//		__asm("ldr r0, = 0x2000F84A");
//		__asm("str r2, [r0]");
//
    #ifdef USING_APP_SIM
        //Power off module sim
        SIM_PW_OFF1;
    #endif 
        __disable_irq();
        //Reset
        NVIC_SystemReset(); // Reset MCU
//    } else
//    {
//    	UTIL_Log_Str(DBLEVEL_L, (uint8_t *) "Xung chua vuot qua 5\r\n";
//    }
    
//    return 0;
}


void Reset_Chip_Immediately (void)
{
	Modem_Log_And_Send_Emergency((uint8_t *) "Modem: Reset Chip Immediately\r\n", 31);
//	//Get Pulse
//	LPTIM_Counter_Get_Pulse();
//	Modem_Check_Add_5Pulse();
//	//Save Mark Soft Reset MCU value 0xA5A5A5A5 to Add RAM: 0x2000FC40
//	sModem.MarkResetChip_u8 = 0xA5A5A5A5;
//
//	__asm("ldr r0, = pMarkSoftResetMCU");
//	__asm("ldr r1, [r0, 0]");
//	__asm("ldr r2, [r1, 0]");
//
//	__asm("ldr r0, = 0x2000F840");
//	__asm("str r2, [r0]");
//
//	//Save Pulse to Add RAM: 0x2000FC4A
//	__asm("ldr r0, = pPulseLPTIM");
//	__asm("ldr r1, [r0, 0]");
//	__asm("ldr r2, [r1, 0]");
//
//	__asm("ldr r0, = 0x2000F84A");
//	__asm("str r2, [r0]");

#ifdef USING_APP_SIM
    //Power off module sim
    SIM_PW_OFF1;
#endif
        
	__disable_irq();
	//Reset
	NVIC_SystemReset(); // Reset MCU
}



/*
    Func: vao che do luu kho
        + Power off module SIM
        + Go to standby mode
        + Wait wake up later
*/
void Modem_Save_Box (void)
{
//    //Power off module SIM
//    BC660_POW_PIN_OFF;
//    
//    /* Disable the Alarm A interrupt */
//    __HAL_RTC_ALARMA_DISABLE(&hrtc);
//    /* Disable the Alarm A interrupt */
//    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A );
//    /* Clear RTC Alarm Flag */
//    __HAL_RTC_ALARM_CLEAR_FLAG( &hrtc, RTC_FLAG_ALRAF);
//    /* Clear the EXTI's line Flag for RTC Alarm */  
//    __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
//            
//    HAL_LPTIM_Encoder_Stop_IT(&hlptim1);
//        
//    UTIL_LPM_EnterStandbyMode();
//
//    //Reset
//    __disable_irq();
//       
//    NVIC_SystemReset(); // Reset MCU
}



void Modem_Log_And_Send_Emergency (uint8_t *pData, uint16_t length)
{
    UTIL_Log(DBLEVEL_M, pData, length);
    //Copy vao buff send len server
    for (uint16_t i = 0; i < length; i++)
    {
        if (sModem.strAlarmEmer.Length_u16 >= MAX_LENGTH_ALAMR_EMER)
            break;
        *(sModem.strAlarmEmer.Data_a8 + sModem.strAlarmEmer.Length_u16++) = *(pData + i);
    }
#ifdef USING_APP_SIM
    sMQTT.aMARK_MESS_PENDING[SEND_ALARM_EMERGENCY] = TRUE;
#endif
}

                        
/*============ Func Modem App ======================*/    
void Modem_Get_Config_UpdateFw (uint32_t *AddNewFw, uint16_t *MAX_PAGE)
{
    *AddNewFw = ADDR_UPDATE_PROGRAM; 
    *MAX_PAGE = FIRMWARE_SIZE_PAGE;
}

/*
    Func: Resp to user
        + PortNo: serial or server
        + Type: Server ACK (1) or Server Not ACK  (0)
*/
void DCU_Respond(uint8_t portNo, uint8_t *data, uint16_t length, uint8_t Type)
{
    uint16_t i = 0;
    
    switch(portNo)   
    {
        case _AT_REQUEST_SERIAL:
            HAL_UART_Transmit(&uart_debug, data,length, 1000);
            break;
        case _AT_REQUEST_SERVER:
        case _AT_REQUEST_LORA: 
            UTIL_MEM_set (aRESPONDSE_AT, 0, sizeof ( aRESPONDSE_AT ));
            sModem.strATResp.Length_u16 = 0;
            //Dong goi Cmd
            aRESPONDSE_AT[sModem.strATResp.Length_u16++] = '(';
            
            for (i = 0; i < sModem.strATCmd.Length_u16; i++)
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = *(sModem.strATCmd.Data_a8 + i);
            
            aRESPONDSE_AT[sModem.strATResp.Length_u16++] = ')';
            //Dong goi phan hoi
            aRESPONDSE_AT[sModem.strATResp.Length_u16++] = '(';
            for (i = 0; i < length; i++)
                aRESPONDSE_AT[sModem.strATResp.Length_u16++] = *(data + i);
            
            aRESPONDSE_AT[sModem.strATResp.Length_u16++] = ')';
            
            sModem.strATCmd.Length_u16 = 0;  //Reset buff AT cmd
            
        #ifdef USING_APP_SIM  
            if (Type == 0)
                sMQTT.aMARK_MESS_PENDING[SEND_RESPOND_SERVER] = TRUE;
            else
                sMQTT.aMARK_MESS_PENDING[SEND_RESPOND_SERVER_ACK] = TRUE;
        #endif
        
            break;
        default:
            break;
    }
}



/*============ Func Pointer LPM lib ================*/

void Modem_Deinit_Peripheral (void)
{
    MX_GPIO_DeInit();

	ADC_Desequence_Powerhungry_Channels();
    
    HAL_ADC_DeInit(&hadc);
       
    HAL_UART_DeInit(&uart_sim);
    
    DeInit_Uart_Sim_Rx_IT();
}


void Modem_Init_Peripheral (void)
{
    Modem_Init_Gpio_Again();  //Only init GPIO pin deinit
    //Init ADC again
    AdcInitialized = 0; 
    
    MX_USART2_UART_Init();
    Init_Uart_Sim_Rx_IT();
}

void Modem_Init_Before_IRQ_Handle (void)
{
#ifdef USING_APP_LORA
    AppLora_Init_IO_Radio();
#endif
}


void Modem_Deinit_Before_IRQ_Handle (void)
{
#ifdef USING_APP_LORA
    AppLora_Deinit_IO_Radio();
#endif
}




void Modem_Uart_DeInit(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure ={0};
//    
//    __HAL_RCC_USART1_CLK_ENABLE();
//    __HAL_RCC_USART2_CLK_ENABLE();
//    __HAL_RCC_USART3_CLK_ENABLE();
//
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    __HAL_RCC_GPIOC_CLK_ENABLE();
//    
//    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//
//    GPIO_InitStructure.Pin =  GPIO_PIN_2|GPIO_PIN_3;
//    HAL_GPIO_Init( GPIOA, &GPIO_InitStructure );
//    
//    GPIO_InitStructure.Pin =  GPIO_PIN_9|GPIO_PIN_10;  
//    HAL_GPIO_Init( GPIOA, &GPIO_InitStructure );
//    
//    GPIO_InitStructure.Pin =  GPIO_PIN_10|GPIO_PIN_11;  
//    HAL_GPIO_Init( GPIOC, &GPIO_InitStructure );
//    
//    __HAL_RCC_USART1_CLK_DISABLE();
//    __HAL_RCC_USART2_CLK_DISABLE();
//    __HAL_RCC_USART3_CLK_DISABLE();
}





void MX_GPIO_DeInit(void)
{
//    GPIO_InitTypeDef GPIO_InitStructure ={0};
//    
//	__HAL_RCC_GPIOB_CLK_ENABLE();
//	
//	GPIO_InitStructure.Mode   = GPIO_MODE_ANALOG;
//	GPIO_InitStructure.Pull   = GPIO_NOPULL;
//    
//    //Chan output       
//    GPIO_InitStructure.Pin    = LED_2_Pin|LED_1_Pin | SIM_RESET_Pin|SIM_PWR_KEY_Pin;              
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
//             
//    //Cac chan Input          
//    GPIO_InitStructure.Pin    = BUTTON_INPUT_Pin|SIM_RI_Pin ;                        
//	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure); 
}




/*
    Func: Cau hinh lai nhung chan deinit truoc khi ngu
    */
void Modem_Init_Gpio_Again(void)
{
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//    /* GPIO Ports Clock Enable */
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//
//    /*Configure GPIO pin Output Level */
//    HAL_GPIO_WritePin(GPIOB, LED_2_Pin|LED_1_Pin|SIM_RESET_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_RESET);
//
//    /*Configure GPIO pins : PBPin PBPin */
//    GPIO_InitStruct.Pin = BUTTON_INPUT_Pin|SIM_RI_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    /*Configure GPIO pins : PBPin PBPin */
//    GPIO_InitStruct.Pin = LED_2_Pin|LED_1_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    /*Configure GPIO pins : PBPin PBPin */
//    GPIO_InitStruct.Pin = SIM_RESET_Pin|SIM_PWR_KEY_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

    