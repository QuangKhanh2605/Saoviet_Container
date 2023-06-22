
#include "user_app_inverter.h"
#include "user_define.h"

#include "user_modbus_rtu.h"
#include "user_sv_inverter.h"

/*================ Define =================*/

/*============= Function static  ===========*/
static uint8_t _Write_Read_IVT(uint8_t event);
static uint8_t _Config_IVT(uint8_t event);
static uint8_t _Entry_IVT(uint8_t event);
static uint8_t _Cb_Temh_Log_TSVH (uint8_t event);
static uint8_t _Flash_Mem_IVT(uint8_t event);
static uint8_t _Once_Handle_IVT(uint8_t event);
/*================ Struct =================*/
sEvent_struct         sEventAppIVT [] =
{
  { _EVENT_ENTRY_IVT,           0, 0, 0,          _Entry_IVT},    
  
  { _EVENT_ONCE_HANDLE,         1, 0, 0,          _Once_Handle_IVT},
  
  { _EVENT_WRITE_FLASH,         1, 0, 10,         _Flash_Mem_IVT},
  
  { _EVENT_CONFIG_IVT,          1, 0, 10, 	      _Config_IVT},
    
  { _EVENT_WRITE_READ_IVT,  	0, 0, 100,        _Write_Read_IVT},
	
  { _EVENT_TEMH_LOG_TSVH, 	    0, 0, 10000, 	  _Cb_Temh_Log_TSVH},
};      

Struct_Config_IVT      sConfig_IVT[] =
{
  { _ENUM_FREQUENCY_SETTING,           0, 1001, 0},
  
  { _ENUM_START_STOP_SETTING,          0, 1002, 3},
  
  { _ENUM_ACCELERATION_TIME,           0, 1004, 100},
  
  { _ENUM_DECELERATION_TIME,           0, 1005, 100},
};

Struct_Battery_Status    sBattery;
Struct_Battery_Status    sVout;

struct_ThresholdConfig   sMeterThreshold =
{
    .FlowHigh = 0xFFFF,
    .FlowLow = 0,
    .PeakHigh = 0xFFFF,
    .PeakLow = 0,
    .LowBatery = 10,
    .LevelHigh = 4000,
    .LevelLow  = 10,
};

uint8_t aMARK_ALARM[10];
uint8_t aMARK_ALARM_PENDING[10];

uint8_t UnLock_run_stop_485_IVT = 1;

uint8_t run_stop_485_IVT=0xFF;
uint8_t permit_run_stop_485_IVT = 0;

uint8_t config_IVT_u8=0;
uint8_t connect_IVT_u8=0;

uint32_t ID_server_u32=0;

uint16_t response_IVT_u16 = 0;

uint8_t aRead_IVT_Receive[6];
uint8_t aWrite_IVT_Receive[6];
uint8_t aRun_IVT_Receive[6];
uint8_t aConfig_IVT_Flash[NUMBER_BUFFER_CONFIG];

Param_Read_485_Inverter             sRead_485_Inverter;
Param_Run_Stop_485_Inverter         sRun_Stop_485_Inverter;
Param_Check_Config_485_Inverter     sCheck_Config_485_Inverter;

IVT_Receive            sFre_IVT_Receive=
{
  .Status_Receive_u8 = 0,
};

ADDR_485_Receive       sRead_485_IVT=
{
  .Status_Receive_u8 = 0,
  .aData = aRead_IVT_Receive,
};

ADDR_485_Receive       sConfig_485_IVT=
{
  .Status_Receive_u8 = 0,
  .aData = aWrite_IVT_Receive,
};

ADDR_485_Receive        sRun_485_IVT=
{
  .Status_Receive_u8 = 0,
  .aData = aRun_IVT_Receive,
};

STempHumiVariable         sTempHumi = 
{
    .aSlaveID = {SLAVE_ID_DEFAULT, SLAVE_ID_DEFAULT},
    .NumSlave_u8 = NUM_SLAVE_DEFAULT,
    .CountFailGetTemh_u8 = 1,
};

uint8_t aUART_485_DATA [64];
sData   sUart485 = {(uint8_t *) &aUART_485_DATA[0], 0};

static GPIO_TypeDef*  LED_PORT[3] = {LED_1_GPIO_Port, LED_2_GPIO_Port, LED_3_GPIO_Port};
static const uint16_t LED_PIN[3] = {LED_1_Pin, LED_2_Pin, LED_3_Pin};

/*================ Struct =================*/

static uint8_t _Entry_IVT (uint8_t event)
{
    fevent_active(sEventAppIVT, _EVENT_TEMH_LOG_TSVH);
    return 1;
}

/*
    Function thuc hien 1 lan
*/
static uint8_t _Once_Handle_IVT(uint8_t event)
{
//  OnchipFlashPageErase(ADDR_RUN_STOP_INVERTER);
//  OnchipFlashPageErase(ADDR_CONFIG_INVERTER);
//  OnchipFlashPageErase(ADDR_READ_INVERTER);
    Setup_Addr_Register_Read_IVT();
    Setup_Addr_Register_Write_IVT();
    Setup_Addr_Register_Run_Stop_IVT();
    return 1;
}

/*
    Doc gia tri va thong tin thanh ghi Run/Stop Inverter
*/
uint8_t Setup_Addr_Register_Run_Stop_IVT(void)
{
    if(*(__IO uint8_t*)(ADDR_RUN_STOP_INVERTER) == BYTE_TEMP_FIRST)
    {
        sRun_Stop_485_Inverter.Addr_IVT = Read_Flash_U16(ADDR_RUN_STOP_INVERTER+2);
        sRun_Stop_485_Inverter.Stop_IVT = Read_Flash_U16(ADDR_RUN_STOP_INVERTER+4);
        sRun_Stop_485_Inverter.Run_IVT  = Read_Flash_U16(ADDR_RUN_STOP_INVERTER+6);
    }
    else
    {
        sRun_Stop_485_Inverter.Addr_IVT = DEF_ADDR_REGISTER_RUN_STOP;
        sRun_Stop_485_Inverter.Stop_IVT = DEF_INFOR_STOP;
        sRun_Stop_485_Inverter.Run_IVT  = DEF_INFOR_RUN;
    }
    
    if(sRun_Stop_485_Inverter.Run_IVT != sRun_Stop_485_Inverter.Stop_IVT )
    {
        permit_run_stop_485_IVT = 1;
    }
    
    return 1;
}

/*
    Doc gia tri thanh ghi Read Inverter
*/
uint8_t Setup_Addr_Register_Read_IVT(void)
{
    if(*(__IO uint8_t*)(ADDR_READ_INVERTER) == BYTE_TEMP_FIRST)
    {
        sRead_485_Inverter.ID_Iverter     = *(__IO uint8_t*)(ADDR_READ_INVERTER +2);
        sRead_485_Inverter.Scale_Inverter = *(__IO uint8_t*)(ADDR_READ_INVERTER +3);
        sRead_485_Inverter.Set_Freq_IVT   = Read_Flash_U16(ADDR_READ_INVERTER +4);
        sRead_485_Inverter.Out_Freq_IVT   = Read_Flash_U16(ADDR_READ_INVERTER +6);
    }
    else
    {
        sRead_485_Inverter.ID_Iverter     = DEF_ID_INVERTER;
        sRead_485_Inverter.Scale_Inverter = SCALE_IVT;
        sRead_485_Inverter.Set_Freq_IVT   = DEF_ADDR_REGISTER_SETUP_FREQ;
        sRead_485_Inverter.Out_Freq_IVT   = DEF_ADDR_REGISTER_OUT_FREQ;
    }

    return 1;
}

/*
    Doc gia tri thanh ghi Write Inverter
*/
uint8_t Setup_Addr_Register_Write_IVT(void)
{
    if(*(__IO uint8_t*)(ADDR_CONFIG_INVERTER) != BYTE_TEMP_FIRST)
    {
        UTIL_MEM_set(aConfig_IVT_Flash, 0x00, NUMBER_BUFFER_CONFIG);
        uint8_t j = 0;
        for(uint8_t i=0; i < _ENUM_FINISH_CONFIG;i++)
        {
            aConfig_IVT_Flash[j]   = sConfig_IVT[i].address_register >> 8;
            aConfig_IVT_Flash[j+1] = sConfig_IVT[i].address_register;
            aConfig_IVT_Flash[j+2] = sConfig_IVT[i].infor_register >> 8;
            aConfig_IVT_Flash[j+3] = sConfig_IVT[i].infor_register;
            j+= 4;
        }
        if(j%4 !=0)
        {
            j+= j%4;
        }
        Save_Array(ADDR_CONFIG_INVERTER, aConfig_IVT_Flash, j);
    }
    
    sCheck_Config_485_Inverter.Addr_IVT = Read_Flash_U16(ADDR_CONFIG_INVERTER+2);
    sCheck_Config_485_Inverter.Infor_IVT = Read_Flash_U16(ADDR_CONFIG_INVERTER+4);
    
    return 1;
}

/*
    Function xu ly chuoi nhan duoc va nap vao Flash
*/
static uint8_t _Flash_Mem_IVT(uint8_t event)
{
  if(sRun_485_IVT.Status_Receive_u8 == 1)
  {
    sRun_485_IVT.Status_Receive_u8 = 0;
    Save_Array(ADDR_RUN_STOP_INVERTER, sRun_485_IVT.aData, 8);
    Setup_Addr_Register_Run_Stop_IVT();
    DCU_Respond(2, sRun_485_IVT.aData, 6, 0);
  }
  
  if(sRead_485_IVT.Status_Receive_u8 == 1)
  {
    sRead_485_IVT.Status_Receive_u8 = 0 ;
    Save_Array(ADDR_READ_INVERTER, sRead_485_IVT.aData, 8);
    Setup_Addr_Register_Read_IVT();
    DCU_Respond(2, sRead_485_IVT.aData, 6, 0);
  }
  
  if(sConfig_485_IVT.Status_Receive_u8 == 1)
  {
    sConfig_485_IVT.Status_Receive_u8 = 0;
    if((sConfig_485_IVT.aData[0] <= sConfig_485_IVT.aData[1]) && sConfig_485_IVT.aData[1] < NUMBER_CONFIG)
    {
        UTIL_MEM_set(aConfig_IVT_Flash, 0x00, NUMBER_BUFFER_CONFIG);
        uint8_t j=0;
        uint32_t address = ADDR_CONFIG_INVERTER + 2;
        for(uint8_t i=1 ; i <= sConfig_485_IVT.aData[1]; i++)
        {
            if(i == sConfig_485_IVT.aData[0])
            {
                aConfig_IVT_Flash[j]  =sConfig_485_IVT.aData[2];
                aConfig_IVT_Flash[j+1]=sConfig_485_IVT.aData[3];
                aConfig_IVT_Flash[j+2]=sConfig_485_IVT.aData[4];
                aConfig_IVT_Flash[j+3]=sConfig_485_IVT.aData[5];
                j+= 4;
                address+= 4;
            }
            else
            {
                aConfig_IVT_Flash[j]  =*(__IO uint8_t*)(address);
                aConfig_IVT_Flash[j+1]=*(__IO uint8_t*)(address+1);
                aConfig_IVT_Flash[j+2]=*(__IO uint8_t*)(address+2);
                aConfig_IVT_Flash[j+3]=*(__IO uint8_t*)(address+3);
                j=j+4;
                address+= 4;
            }
        }
        if(j%4 !=0)
        {
            j+= j%4;
        }
        Save_Array(ADDR_CONFIG_INVERTER, aConfig_IVT_Flash, j);
        DCU_Respond(2, aConfig_IVT_Flash, j, 0);
    }
  }
  
    fevent_enable(sEventAppIVT, _EVENT_WRITE_FLASH);
    return 1;
}

/*
    Function doc va ghi gia tri vao Inverter
*/
static uint8_t _Write_Read_IVT(uint8_t event)
{ 
  static uint8_t step=0;
  static uint32_t gettick=0;
  static uint8_t count_disconnect=0;
  static uint8_t read_IVT=0;
  
    switch(step)
    {
        case 0:
          if(sFre_IVT_Receive.Status_Receive_u8 == 1)
          {
              uint16_t fre_send_IVT=0;
              fre_send_IVT = Calculator_Scale_IVT(sFre_IVT_Receive);
              Send_Command_IVT(sRead_485_Inverter.ID_Iverter, 0x06, sRead_485_Inverter.Set_Freq_IVT, fre_send_IVT,&AppIVT_Clear_Before_Recv);
              step = 1;
              gettick = HAL_GetTick();
              
              if(UnLock_run_stop_485_IVT == 1)
              {
                run_stop_485_IVT = sRun_Stop_485_Inverter.Run_IVT;
              }
              else
              {
                run_stop_485_IVT = sRun_Stop_485_Inverter.Stop_IVT;
              }
          }
          else if(run_stop_485_IVT != 0xFF)
          {
            if(permit_run_stop_485_IVT == 1)
            {
                Send_Command_IVT(sRead_485_Inverter.ID_Iverter, 0x06, sRun_Stop_485_Inverter.Addr_IVT, run_stop_485_IVT,&AppIVT_Clear_Before_Recv);
                step = 1;
                gettick = HAL_GetTick();
            }
            else
            {
                run_stop_485_IVT = 0xFF;
            }
          }
          else
          {
            if(HAL_GetTick() - gettick > TIME_READ_FREQUENCY )
            {
              if(read_IVT == 0)
              {
                Send_Command_IVT(sRead_485_Inverter.ID_Iverter, 0x03, sRead_485_Inverter.Out_Freq_IVT, 1,&AppIVT_Clear_Before_Recv);
              }
              else
              {
                Send_Command_IVT(sRead_485_Inverter.ID_Iverter, 0x03, sCheck_Config_485_Inverter.Addr_IVT, 1,&AppIVT_Clear_Before_Recv);
              }
              step = 1;
              gettick = HAL_GetTick();
            }
          }
          break;
          
        case 1:
          if(HAL_GetTick() - gettick < TIME_RESEND_IVT)
          {
            if(Check_CountBuffer_Complete_Uart() == 1)
            {
                uint16_t CRC_Check = ModRTU_CRC(sUart485.Data_a8, sUart485.Length_u16-2);
                uint16_t CRC_Receive = sUart485.Data_a8[sUart485.Length_u16-1] << 8 | sUart485.Data_a8[sUart485.Length_u16-2];
                
                if(sUart485.Data_a8[1] != 0x86 && sUart485.Data_a8[1] != 0x83)
                {
                    if(CRC_Check == CRC_Receive)
                    {
                        if(sUart485.Data_a8[1] == 0x03)
                        {
                          if(read_IVT == 0)
                          {
                            response_IVT_u16 = sUart485.Data_a8[sUart485.Length_u16-4] << 8 | sUart485.Data_a8[sUart485.Length_u16-3];
                            read_IVT = 1;
                          }
                          else
                          {
                            read_IVT = sUart485.Data_a8[sUart485.Length_u16-4] << 8 | sUart485.Data_a8[sUart485.Length_u16-3];
                            if(read_IVT != sCheck_Config_485_Inverter.Infor_IVT)
                            {
                                config_IVT_u8 = 1;
                            }
                            read_IVT = 0;
                          }
                        }
                        else if(sUart485.Data_a8[1] == 0x06)
                        {
                          if((sUart485.Data_a8[sUart485.Length_u16-6] << 8 | sUart485.Data_a8[sUart485.Length_u16-5]) == sRead_485_Inverter.Set_Freq_IVT)
                          {
                            sFre_IVT_Receive.Status_Receive_u8 = 0;
                          }
                          else if((sUart485.Data_a8[sUart485.Length_u16-6] << 8 | sUart485.Data_a8[sUart485.Length_u16-5]) == sRun_Stop_485_Inverter.Addr_IVT)
                          {
                            run_stop_485_IVT = 0xFF;
                          }
                        }
                        connect_IVT_u8 = 1;
                        count_disconnect = 0;
                    }
                }
                else
                {
                    sFre_IVT_Receive.Status_Receive_u8 = 0;
                }
                step = 0;
            }
          }
          else
          {
              MX_USART1_UART_Init();
              Init_Uart_485_Rx_IT();
              step = 0;
              count_disconnect++;
          }
          break;
          
        default: 
          step = 0;
          break;
    }
    
    if(count_disconnect >= 3)
    {
      connect_IVT_u8 = 0;
      response_IVT_u16 = 0;
    }
    
    fevent_enable(sEventAppIVT, event);
    return 1;
}

/*
    Cau hinh Inverter
*/
static uint8_t _Config_IVT(uint8_t event)
{ 
  static uint8_t handle_once=0;
  static uint32_t gettick_config=0;
  if(config_IVT_u8 == 1)
  {
    if(sRead_485_IVT.Status_Receive_u8 == 0 && sConfig_485_IVT.Status_Receive_u8 == 0)
    {
        if(handle_once == 0)
        {
            gettick_config = HAL_GetTick();
            handle_once = 1;
        }
        
        if(HAL_GetTick() - gettick_config < TIME_SEND_OFF_IVT)
        {
          connect_IVT_u8 =0;
          fevent_disable(sEventAppIVT, _EVENT_WRITE_READ_IVT);
          if(Task_ConfigIVT(sRead_485_Inverter.ID_Iverter) == 1)
          {
            sCheck_Config_485_Inverter.Addr_IVT  =  Read_Flash_U16(ADDR_CONFIG_INVERTER+2);
            sCheck_Config_485_Inverter.Infor_IVT =  Read_Flash_U16(ADDR_CONFIG_INVERTER+4);
            
            config_IVT_u8 = 0;
            connect_IVT_u8 = 1;
            UTIL_Printf_Str( DBLEVEL_M, "u_app_IVT200: Success Config IVT\r\n" );
          }
        }
        else
        {
            handle_once = 0;
            Send_Command_IVT(sRead_485_Inverter.ID_Iverter, 0x06, sRun_Stop_485_Inverter.Addr_IVT, sRun_Stop_485_Inverter.Stop_IVT,&AppIVT_Clear_Before_Recv);
            gettick_config = HAL_GetTick();
        }
    }
  }
  else
  {
    handle_once=0;
    fevent_active(sEventAppIVT, _EVENT_WRITE_READ_IVT);
  }
  fevent_enable(sEventAppIVT, event);
  return 1;
}

/*
    Function Handle Config Inverter
*/
uint8_t Task_ConfigIVT(uint8_t ID_Slave)
{
    static uint8_t step=0;
    static uint8_t handle_once=0;
    static uint16_t addr_register = 0;
    static uint16_t infor_register = 0;
    uint32_t address = ADDR_CONFIG_INVERTER + 2;
    
    if(step == (*(__IO uint8_t*)(ADDR_CONFIG_INVERTER + 1) - 2)/4)
    {
        step = 0;
        handle_once = 0;
        return 1;
    }
    else
    {
      if(handle_once == 0)
      {
        handle_once = 1;
        addr_register  = Read_Flash_U16(address + step*4);                  
        infor_register = Read_Flash_U16(address + step*4 + 2);
      }                              
      if(Check_Complete_Command_IVT(ID_Slave, addr_register , infor_register) == 1) 
      {
        handle_once = 0;
        step++;
      }
    }
    
    return 0;
}

/*================ Function Handler =================*/

uint8_t AppIVT_Task(void)
{
	uint8_t i = 0;
	uint8_t Result = false;

	for (i = 0; i < _EVENT_END_TEMP_HUMI; i++)
	{
		if (sEventAppIVT[i].e_status == 1)
		{
            Result = true;

			if ((sEventAppIVT[i].e_systick == 0) ||
					((HAL_GetTick() - sEventAppIVT[i].e_systick)  >=  sEventAppIVT[i].e_period))
			{
                sEventAppIVT[i].e_status = 0;  //Disable event
				sEventAppIVT[i].e_systick = HAL_GetTick();
				sEventAppIVT[i].e_function_handler(i);
			}
		}
	}
    
	return Result;
}

/*
  @brief  Gui lenh toi IVT va doi phan hoi 
  @retval (0) Chua hoan thanh
  @retval (1) Da hoan thanh 
*/
uint8_t Check_Complete_Command_IVT(uint8_t Addr_Slave, uint16_t Addr_Register, uint16_t Infor_Register)
{
  static uint8_t step=0;
  uint8_t answer=0;
  static uint32_t gettick = 0;

  switch (step)
  {
  case 0:
    Send_Command_IVT(Addr_Slave, 0x06, Addr_Register, Infor_Register ,&AppIVT_Clear_Before_Recv);
    gettick = HAL_GetTick();
    step++;
    break;
    
  case 1:
    if(HAL_GetTick() - gettick < TIME_RESEND_IVT)
    {
      if(Check_CountBuffer_Complete_Uart() == 1)
      {
        uint16_t CRC_Receive = ModRTU_CRC(sUart485.Data_a8, sUart485.Length_u16-2);
        uint16_t CRC_Check   = sUart485.Data_a8[sUart485.Length_u16-1]<<8 | sUart485.Data_a8[sUart485.Length_u16-2];
        uint16_t Check_Addr_Register = sUart485.Data_a8[sUart485.Length_u16-6]<<8 | sUart485.Data_a8[sUart485.Length_u16-5];
        uint16_t Check_Infor_Register= sUart485.Data_a8[sUart485.Length_u16-4]<<8 | sUart485.Data_a8[sUart485.Length_u16-3];
        if(CRC_Check == CRC_Receive)
        {
          if(Addr_Slave == sUart485.Data_a8[0])
          {
            if(sUart485.Data_a8[1] == 0x06)
            {
                if(Addr_Register == Check_Addr_Register)
                {
                  if(Infor_Register == Check_Infor_Register)
                  {
                    answer = 1;
                    step = 0;
                  }
                }
            }
          }
        }
      }
    }
    else
    {
      MX_USART1_UART_Init();
      Init_Uart_485_Rx_IT();
      step = 0;
    }
    break;
    
  default:
    step=0;
    break;
  }
  return answer;
}

/*
    Doc gia tri Uint16 tu Flash
*/
uint16_t Read_Flash_U16(uint32_t address)
{
    uint32_t result = 0 ;
    result = *(__IO uint8_t*)(address );
    result = result << 8 | *(__IO uint8_t*)(address +1);    
    return result;
}

/*
    Chuyen Scale sang Uint
*/
uint32_t Calculator_Scale(uint8_t Scale)
{
    /*
    Scale
    00->1 
    FF->0.1 
    FE->0.01 
    FD->0.001  
    FC->0.0001  
    FB->0.00001 
    */
  
    if(Scale == 0x00) return 1;
    else if(Scale == 0xFF) return 10;
    else if(Scale == 0xFE) return 100;
    else if(Scale == 0xFD) return 1000;
    else if(Scale == 0xFC) return 10000;
    else if(Scale == 0xFB) return 100000;
    
    return 1;
}

/*
    @brief  Tinh tan so theo Scale
    @retval Tan so nap vao bien tan
*/
uint16_t Calculator_Scale_IVT(IVT_Receive  sFre_IVT_Receive)
{
      /*
    Scale
    00->1 
    FF->0.1 
    FE->0.01 
    FD->0.001  
    FC->0.0001  
    FB->0.00001 
    */
  uint32_t Stamp_Scale = sFre_IVT_Receive.Pre_IVT * Calculator_Scale(sRead_485_Inverter.Scale_Inverter);
  return Stamp_Scale/Calculator_Scale(sFre_IVT_Receive.Scale_IVT);
}

/*
	@brief  Kiem tra da nhan xong Uart hay chua 
	@param  sUart Struct uart muon kiem tra
	@return (0) chua nhan xong
	@return (1) da nhan xong
*/
uint8_t Check_CountBuffer_Complete_Uart(void)
{
      static uint16_t check_countBuffer_uart=0;
      static uint32_t Get_systick_countBuffer_uart=0;
      static uint16_t State_systick_countBuffer_uart=0;
      uint8_t answer=0;
      
      if(sUart485.Length_u16 > 0)
      {
              if(State_systick_countBuffer_uart == 0 )
              {
                      Get_systick_countBuffer_uart = HAL_GetTick();
                      check_countBuffer_uart = sUart485.Length_u16;
                      State_systick_countBuffer_uart = 1;
              }
              else
              {
                      if(HAL_GetTick() - Get_systick_countBuffer_uart > 5)	
                      {
                              if(check_countBuffer_uart == sUart485.Length_u16)
                              {
                                      //Get_systick_countBuffer_uart = HAL_GetTick();
                                      answer = 1;
                              }
                              else
                              {
                                      Get_systick_countBuffer_uart = HAL_GetTick();
                                      check_countBuffer_uart = sUart485.Length_u16;
                              }
                      }
                      else
                      {
                              if(check_countBuffer_uart != sUart485.Length_u16)
                              {
                                      Get_systick_countBuffer_uart = HAL_GetTick();
                                      check_countBuffer_uart = sUart485.Length_u16;
                              }
                      }
              }
      }
      else
      {
              State_systick_countBuffer_uart = 0;
      }
      return answer;
}

uint16_t AppTemH_Get_VBAT_mV(void)
{
    //Get batterry
    sBattery.mVol_u32 = Get_Value_ADC (ADC_CHANNEL_14);
    sBattery.mVol_u32 *= 3;   //Phan ap chia 2
    sBattery.Level_u16 = Get_Level_Voltage (sBattery.mVol_u32, VDD_BAT, VDD_MIN); 

	return sBattery.Level_u16;
}

uint16_t AppTemH_Get_Vout_mV(void)
{
    AdcInitialized = 0; 
    sVout.mVol_u32 = Get_Value_ADC(ADC_CHANNEL_7); 
    sVout.mVol_u32 = sVout.mVol_u32 * 247 / 47 + 600;  //qua 2 diode
    sVout.Level_u16 = Get_Level_Voltage (sVout.mVol_u32, VDD_OUT_MAX, VDD_OUT_MIN); 
   
	return sVout.Level_u16;
}



uint8_t AppIVT_Packet_TSVH (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;   
    //----------------------- start send data below --------------------- //
    //----------sTime--------------------
    pData[length++] = OBIS_TIME_DEVICE;   // sTime
    pData[length++] = 0x06;
    pData[length++] = sRTC.year;
    pData[length++] = sRTC.month;
    pData[length++] = sRTC.date;
    pData[length++] = sRTC.hour;
    pData[length++] = sRTC.min;
    pData[length++] = sRTC.sec;
    
    pData[length++] = OBIS_EMET_FREQ; 
    pData[length++] = 0x02;
    pData[length++] = (response_IVT_u16 >> 8) & 0xFF;
    pData[length++] = response_IVT_u16 & 0xFF;
    pData[length++] = sRead_485_Inverter.Scale_Inverter;
    
    
        //----------trang thai ket noi --------------------
    pData[length++] = OBIS_ENVI_CONNECT_STATUS; 
    pData[length++] = 0x01;
    pData[length++] = connect_IVT_u8 & 0xFF;
    
//     pData[length++] = OBIS_ENVI_ID_SERVER; 
//    pData[length++] = 0x04;
//    pData[length++] = (ID_server_u32 >> 24) & 0xFF;
//    pData[length++] = (ID_server_u32 >> 16) & 0xFF;
//    pData[length++] = (ID_server_u32 >> 8) & 0xFF;
//    pData[length++] = ID_server_u32 & 0xFF;
//    pData[length++] = 0x00;
       
//    if (sTempHumi.Status_u8[0] == true)
//    {        
//        //----------Nhiet do--------------------
//        pData[length++] = OBIS_ENVI_TEMP_1; // Dien ap pin   
//        pData[length++] = 0x02;
//        pData[length++] = (sTempHumi.sTemperature[0].Val_i16)>>8;
//        pData[length++] = (sTempHumi.sTemperature[0].Val_i16);
//        pData[length++] = sTempHumi.sTemperature[0].Scale_u8;
//        
//        //----------do am--------------------
//        pData[length++] = OBIS_ENVI_HUMI_1; // Dien ap pin
//        pData[length++] = 0x02;
//        pData[length++] = (sTempHumi.sHumidity[0].Val_i16)>>8;
//        pData[length++] = (sTempHumi.sHumidity[0].Val_i16);
//        pData[length++] = sTempHumi.sHumidity[0].Scale_u8;
//    }
//        
//    if (sTempHumi.Status_u8[1] == true)
//    {
//        //----------Nhiet do--------------------
//        pData[length++] = OBIS_ENVI_TEMP_2; // Dien ap pin   
//        pData[length++] = 0x02;
//        pData[length++] = (sTempHumi.sTemperature[1].Val_i16)>>8;
//        pData[length++] = (sTempHumi.sTemperature[1].Val_i16);
//        pData[length++] = sTempHumi.sTemperature[1].Scale_u8;
//        
//        //----------do am--------------------
//        pData[length++] = OBIS_ENVI_HUMI_2; // Dien ap pin
//        pData[length++] = 0x02;
//        pData[length++] = (sTempHumi.sHumidity[1].Val_i16)>>8;
//        pData[length++] = (sTempHumi.sHumidity[1].Val_i16);
//        pData[length++] = sTempHumi.sHumidity[1].Scale_u8;
//    }
//    
//    //----------Dong dien --------------------
//    pData[length++] = OBIS_EMET_CUR; // Dien ap pin
//    pData[length++] = 0x02;
//    pData[length++] = (sTempHumi.sCurrent.Val_i16)>>8;
//    pData[length++] = (sTempHumi.sCurrent.Val_i16);
//    pData[length++] = 0xFD;
//    
//    //----------Toa do GPS --------------------
//#ifdef USING_APP_SIM
////    if (sSimCommon.sGPS.Status_u8 == true)
////    {
//        if ( (sSimCommon.sGPS.LengData_u8 != 0) && (sSimCommon.sGPS.LengData_u8 < 30) )
//        {
//            pData[length++] = OBIS_GPS_LOC;  
//            pData[length++] = sSimCommon.sGPS.LengData_u8;
//            
//            for (i = 0; i < sSimCommon.sGPS.LengData_u8; i++)
//                pData[length++] = sSimCommon.sGPS.aPOS_INFOR[i]; 
//        }
////    }
//#endif
//
//    //----------Dien ap Pin--------------------
//    pData[length++] = OBIS_DEV_VOL1; // Dien ap pin
//    pData[length++] = 0x02;
//    pData[length++] = (sBattery.mVol_u32)>>8;
//    pData[length++] = (sBattery.mVol_u32);
//    pData[length++] = 0xFD;
//    
//    //----------Cuong do song--------------------
//#ifdef USING_APP_SIM
//    pData[length++] = OBIS_RSSI_1; 
//    pData[length++] = 0x01;
//    pData[length++] = sSimInfor.RSSI_u8;
//    pData[length++] = 0x00;   
//#endif
//    //----------Dien ap Pin--------------------
//    pData[length++] = OBIS_DEV_VOL2; // Dien ap pin
//    pData[length++] = 0x02;
//    pData[length++] = (sVout.mVol_u32)>>8;
//    pData[length++] = (sVout.mVol_u32);
//    pData[length++] = 0xFD;
//    
    //----------Tan suat--------------------
    pData[length++] = OBIS_FREQ_SEND; 
    pData[length++] = 0x02;
    pData[length++] = (sFreqInfor.FreqSendUnitMin_u32 >> 8) & 0xFF;
    pData[length++] = sFreqInfor.FreqSendUnitMin_u32 & 0xFF;
    pData[length++] = 0x00;

    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;
    
    
    return length;
}


uint8_t Modem_Packet_Event (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;
    
    //----------------------- start send data below ---------------------
    pData[length++] = 0x01;   // sTime
    pData[length++] = 0x06;
    pData[length++] = sRTC.year;
    pData[length++] = sRTC.month;
    pData[length++] = sRTC.date;
    pData[length++] = sRTC.hour;
    pData[length++] = sRTC.min;
    pData[length++] = sRTC.sec;
    
    //Chac chan vao day da co 1 alarm
    for (i = 0; i < _ALARM_END;i++)
    {
        if (aMARK_ALARM[i] == 1)
        {
            //Obis
            pData[length++] = i;
            //Length +Data + scale
            switch (i)
            {
                case _ALARM_FLOW_LOW:
                case _ALARM_FLOW_HIGH:
                    break;
                case _ALARM_PEAK_LOW:
                case _ALARM_PEAK_HIGH:
                    break;
                case _ALARM_VBAT_LOW:
                    break;
                case _ALARM_DIR_CHANGE:
                    break;
                case _ALARM_DETECT_CUT:
                    pData[length++] = 1;     //length
                    pData[length++] = sModem.DetectCutStatus_u8; 
                    pData[length++] = 0x00;
                    break;
                default:
                    break;   
            }
        }
        
        //Reset Alarm
        aMARK_ALARM[i] = 0;
    }  
            
    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;
    
    return length;
}


int32_t Modem_Cacul_Quantity (uint32_t PulseCur, uint32_t PulseOld)
{
    int32_t Qual = 0;
    
    if (PulseCur >=  PulseOld)
	{
		if ((PulseCur - PulseOld) < 0xFFFFFFAF)   //khong the quay nguoc dc 0x50 xung trong 10p va khong the quay tien 0xFFAF
			Qual = (PulseCur - PulseOld);  //binh thuong
		else
			Qual = - (0xFFFFFFFF - PulseCur + PulseOld + 1); //quay nguoc
	} else
	{
		if ((PulseOld - PulseCur) > 0x50)   //khong the quay nguoc dc 0x50 xung trong 10p
			Qual = (0xFFFFFFFF - PulseOld + PulseCur + 1);  //binh thuong
        else
			Qual = - (PulseOld - PulseCur);  // quay ngu?c
	}
    
    return Qual;
}





/*
    Func: Log Data Event
        + Packet Event Data
        + Save to Flash or ExMem
*/

void AppTemH_Log_Data_Event (void)
{
    uint8_t     aMessData[64] = {0};
    uint8_t     Length = 0;
    
    if (sRTC.year <= 20)
        return;
    
    Length = Modem_Packet_Event (&aMessData[0]);
    AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_EVENT_A, &aMessData[0], Length);
}

/*
    Func: Log Data TSVH
        + Packet Data TSVH
        + Save to Flash or ExMem
*/

void AppIVT_Log_Data_TSVH (void)
{
    uint8_t     aMessData[256] = {0};
    uint8_t     Length = 0;
    
//    if (sRTC.year <= 20)
//        return;
    
    Length = AppIVT_Packet_TSVH (&aMessData[0]);
    AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_TSVH_A, &aMessData[0], Length);
}



void AppIVT_Init_Thresh_Measure (void)
{
    uint8_t 	temp = 0xFF;
	uint8_t		Buff_temp[16] = {0};
      
	// Doc gia tri cau hinh threshold. Dung de check cac alarm
    temp = *(__IO uint8_t*) ADDR_THRESH_MEAS;    //2 byte
	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
    {
        OnchipFlashReadData(ADDR_THRESH_MEAS, &Buff_temp[0], 16);
        //
        sMeterThreshold.FlowHigh    = (Buff_temp[2] << 8) | Buff_temp[3];
        sMeterThreshold.FlowLow     = (Buff_temp[4] << 8) | Buff_temp[5];
        
        sMeterThreshold.PeakHigh    = (Buff_temp[6] << 8) | Buff_temp[7];
        sMeterThreshold.PeakLow     = (Buff_temp[8] << 8) | Buff_temp[9];
        
        sMeterThreshold.LowBatery   = Buff_temp[10];
        
        sMeterThreshold.LevelHigh   = (Buff_temp[11] << 8) | Buff_temp[12];
        sMeterThreshold.LevelLow    = (Buff_temp[13] << 8) | Buff_temp[14];
    } else
    {
        AppIVT_Save_Thresh_Measure();
    }    
}



void AppIVT_Save_Thresh_Measure (void)
{
    uint8_t aTEMP_THRESH[24] = {0};
    
    aTEMP_THRESH[0] = BYTE_TEMP_FIRST;
    aTEMP_THRESH[1] = 13;

    aTEMP_THRESH[2] = (sMeterThreshold.FlowHigh >> 8) & 0xFF;
    aTEMP_THRESH[3] = sMeterThreshold.FlowHigh & 0xFF;
    
    aTEMP_THRESH[4] = (sMeterThreshold.FlowLow >> 8) & 0xFF;
    aTEMP_THRESH[5] = sMeterThreshold.FlowLow & 0xFF;
     
    aTEMP_THRESH[6] = (sMeterThreshold.PeakHigh >> 8) & 0xFF;  
    aTEMP_THRESH[7] = sMeterThreshold.PeakHigh & 0xFF;
    
    aTEMP_THRESH[8] = (sMeterThreshold.PeakLow >> 8) & 0xFF;
    aTEMP_THRESH[9] = sMeterThreshold.PeakLow & 0xFF;
    
    aTEMP_THRESH[10] = sMeterThreshold.LowBatery;
    
    aTEMP_THRESH[11] = (sMeterThreshold.LevelHigh >> 8) & 0xFF;
    aTEMP_THRESH[12] = sMeterThreshold.LevelHigh & 0xFF;
    
    aTEMP_THRESH[13] = (sMeterThreshold.LevelLow >> 8) & 0xFF;
    aTEMP_THRESH[14] = sMeterThreshold.LevelLow & 0xFF;


    OnchipFlashPageErase(ADDR_THRESH_MEAS);
    OnchipFlashWriteData(ADDR_THRESH_MEAS, &aTEMP_THRESH[0], 16);
}

	 
void AppIVT_Init_Slave_ID (void)
{
    uint8_t 	temp = 0xFF;
    //Doc List Slave ra
    temp = *(__IO uint8_t*) ADDR_SLAVE_ID;  
	if (temp != FLASH_BYTE_EMPTY) 
	{
        sTempHumi.NumSlave_u8 = *(__IO uint8_t*) (ADDR_SLAVE_ID + 2); 
        
        if (sTempHumi.NumSlave_u8 >= MAX_SLAVE) 
            sTempHumi.NumSlave_u8 = MAX_SLAVE;
        
        OnchipFlashReadData (ADDR_SLAVE_ID + 3, &sTempHumi.aSlaveID[0], sTempHumi.NumSlave_u8);
    } else
    {
        AppIVT_Save_Slave_ID();
    }  
    
}

void AppIVT_Save_Slave_ID (void)
{
    uint8_t aTEMP[64] = {0};
    uint8_t Count = 0;
    
    aTEMP[Count++] = BYTE_TEMP_FIRST;
    aTEMP[Count++] = sTempHumi.NumSlave_u8 + 1;
    aTEMP[Count++] = sTempHumi.NumSlave_u8;
        
    for (uint8_t i = 0; i < sTempHumi.NumSlave_u8; i++)
    {
        aTEMP[Count++] = sTempHumi.aSlaveID[i];
    }
    
    OnchipFlashPageErase(ADDR_SLAVE_ID);
    OnchipFlashWriteData(ADDR_SLAVE_ID, &aTEMP[0], 64);
}


void AppIVT_Init (void)
{
    //AppIVT_Init_Thresh_Measure();
    //AppIVT_Init_Slave_ID();
    //On power to 485 and tempetature 
    V12_IN_ON;
}

void Send_Command_IVT (uint8_t SlaveID, uint8_t Func_Code, uint16_t Addr_Register, uint16_t Infor_Register, void (*pFuncResetRecvData) (void)) 
{
  if(SlaveID >= 1 && SlaveID<=127)
  {
    uint8_t aFrame[48] = {0};
    sData   strFrame = {(uint8_t *) &aFrame[0], 0};
    
    ModRTU_Master_Read_Frame(&strFrame, SlaveID, Func_Code, Addr_Register, Infor_Register);

    HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_SET);  
    HAL_Delay(10);
    // Send
    pFuncResetRecvData();
    
    HAL_UART_Transmit(&UART_485, strFrame.Data_a8, strFrame.Length_u16, 1000); 
    
    //Dua DE ve Receive
    HAL_GPIO_WritePin(RS485_TXDE_GPIO_Port, RS485_TXDE_Pin, GPIO_PIN_RESET);
  }
}

void AppIVT_Clear_Before_Recv (void)
{
    Reset_Buff(&sUart485);
}

/*
    Func: Check thu thap Temh co day du k?
        + Neu qua 10 lan: Reset cac node temp humi: Poweroff -> On again
*/

uint8_t AppTemh_Check_Status_TempH (void)
{
    uint8_t i = 0;
    uint8_t Result = true;
    
    //Check status
    for ( i = 0; i < sTempHumi.NumSlave_u8; i++)
    {
        if (sTempHumi.Status_u8[i] == false)
            break;         
    }
    //Neu tat ca deu true: i = sTempHumi.NumSlave_u8
    if (i == sTempHumi.NumSlave_u8)
    {
        //tat ca status deu true:
        sTempHumi.CountFailGetTemh_u8 = 0;
    } else
    {
        sTempHumi.CountFailGetTemh_u8++;
        if (sTempHumi.CountFailGetTemh_u8 >= MAX_GET_TEMH_FAIL)
        {
            sTempHumi.CountFailGetTemh_u8 = 0;

            Result = false;
        }
    }
    
    return Result;
}




void LED_Toggle (Led_TypeDef Led)
{
    HAL_GPIO_TogglePin(LED_PORT[Led], LED_PIN[Led]);
}


void LED_On (Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_SET);
}


void LED_Off (Led_TypeDef Led)
{
    HAL_GPIO_WritePin(LED_PORT[Led], LED_PIN[Led], GPIO_PIN_RESET);
}

static uint8_t _Cb_Temh_Log_TSVH (uint8_t event)
{
	//Log Data meter to flash
    UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: packet tsvh!\r\n" );
    //
    Get_RTC();
         
//    AppTemH_Get_VBAT_mV();
//    UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: dien ap pin trong: " );
//    UTIL_Printf_Dec(DBLEVEL_M, sBattery.mVol_u32);
//    UTIL_Printf_Str(DBLEVEL_M, " mV\r\n" );
//    
//    AppTemH_Get_Vout_mV();
//    //Print to debug
//    UTIL_Printf_Str(DBLEVEL_M, "u_app_temh: dien ap pin ngoai: " );
//    UTIL_Printf_Dec(DBLEVEL_M, sVout.mVol_u32);
//    UTIL_Printf_Str(DBLEVEL_M, " mV\r\n" );
// 
//    //Record TSVH
//    
//    //Cat chuoi data GPS luu lai: gui di
//    if (sAppSimVar.sDataGPS.Length_u16 > MAX_BYTE_CUT_GPS)
//    {
//        AppMem_Push_Mess_To_Queue_Write(_FLASH_TYPE_DATA_GPS_A, sAppSimVar.sDataGPS.Data_a8, sAppSimVar.sDataGPS.Length_u16);
//        sAppSimVar.sDataGPS.Length_u16 = 0;
//    }
    
    AppIVT_Log_Data_TSVH();    
        
    return 1;
}

