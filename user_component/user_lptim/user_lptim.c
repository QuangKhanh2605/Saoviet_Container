


#include "user_lptim.h"
#include "lptim.h"

#include "main.h"

/*================Function=====================*/

void LPTIM_Init_Feature (void)
{
#ifdef LPTIM_ENCODER_MODE
    HAL_LPTIM_Encoder_Start_IT(&hlptim1, 0xFFFF);
#else  
    HAL_LPTIM_Counter_Start(&hlptim1, 0xFFFF);
#endif
}



/*
    Feature LpTimer.
*/

void LPTIME_Encoder_Get_Pulse (Struct_Pulse *sPulse)
{
    uint16_t LPTRegisValue_u32 = 0;
    static uint16_t LastLPTRegisValue_u32 = 0;
    
	BSP_sensor_Read(&LPTRegisValue_u32);

	if (LPTRegisValue_u32 >=  LastLPTRegisValue_u32)
	{
		if ((LPTRegisValue_u32 - LastLPTRegisValue_u32) < 0xFFAF)   //khong the quay nguoc dc 0x50 xung trong 10p va khong the quay tien 0xFFAF
		{
			sPulse->Number_u32 		+= (LPTRegisValue_u32 - LastLPTRegisValue_u32);  //binh thuong
			LastLPTRegisValue_u32   = LPTRegisValue_u32;
		} else
		{
			sPulse->Number_u32 		-= (0xFFFF - LPTRegisValue_u32 + LastLPTRegisValue_u32 + 1); //quay nguoc
			LastLPTRegisValue_u32   = LPTRegisValue_u32;
		}
	} else
	{
		if ((LastLPTRegisValue_u32 - LPTRegisValue_u32) > 0x50)   //khong the quay nguoc dc 0x50 xung trong 10p
		{
			sPulse->Number_u32  += (0xFFFF - LastLPTRegisValue_u32 + LPTRegisValue_u32 + 1);  //binh thuong
			LastLPTRegisValue_u32		= LPTRegisValue_u32;
		} else
		{
			sPulse->Number_u32  -= (LastLPTRegisValue_u32 - LPTRegisValue_u32);  //binh thuong
			LastLPTRegisValue_u32		= LPTRegisValue_u32;
		}
	}
}

/*
    Func: Get Value from register
*/
void BSP_sensor_Read (uint16_t *sensor_data)
{
	*(sensor_data) = HAL_LPTIM_ReadCounter(&hlptim1);
}


/*
    Func: Callback Lptim Interupt HAL
*/
void HAL_LPTIM_DirectionUpCallback(LPTIM_HandleTypeDef *hlptim)
{
    UTIL_Printf( (uint8_t*) "=Event: Direct Change -> Forward=\r\n", 35 );
    pLPTIM_Func_Direct_Change_Handler (_DIRECT_FORWARD);
}


void HAL_LPTIM_DirectionDownCallback(LPTIM_HandleTypeDef *hlptim)
{
    UTIL_Printf( (uint8_t*) "=Event: Direct Change -> Reverse=\r\n", 35 );
    pLPTIM_Func_Direct_Change_Handler (_DIRECT_REVERSE);
}

                     

void LPTIM_Counter_Get_Pulse (Struct_Pulse *sPulse)
{
    uint16_t LPTRegisValue_u32 = 0;
    static uint16_t LastLPTRegisValue_u32 = 0;
    
	BSP_sensor_Read(&LPTRegisValue_u32);

    if (sPulse->Direct_u8 == 1)
    {
        sPulse->Number_u32 = sPulse->Number_u32 + LPTIM_Check_Pulse_Period( sPulse->NumStartReverse_u32, LastLPTRegisValue_u32 )  \
                                        - LPTIM_Check_Pulse_Period( LPTRegisValue_u32, sPulse->NumStartReverse_u32 );
        
        sPulse->NumStartReverse_u32 = LPTRegisValue_u32;
    } else if (sPulse->Direct_u8 == 2)
    {
        sPulse->Number_u32 = sPulse->Number_u32 + LPTIM_Check_Pulse_Period (sPulse->NumStartReverse_u32, LastLPTRegisValue_u32)   \
                                        - LPTIM_Check_Pulse_Period (sPulse->NumEndReverse_u32, sPulse->NumStartReverse_u32)  \
                                        + LPTIM_Check_Pulse_Period (LPTRegisValue_u32, sPulse->NumEndReverse_u32);
        
        sPulse->NumStartReverse_u32 = LPTRegisValue_u32;
        sPulse->NumEndReverse_u32 = LPTRegisValue_u32;
        
        sPulse->Direct_u8 = 0;
    } else
    {
        sPulse->Number_u32 = sPulse->Number_u32 + LPTIM_Check_Pulse_Period(LPTRegisValue_u32, LastLPTRegisValue_u32);
    }
    
    LastLPTRegisValue_u32      = LPTRegisValue_u32;
    
    LPTIM_Check_DIR_PIN (sPulse);
}


void LPTIM_Check_DIR_PIN (Struct_Pulse *sPulse)
{
    //check lai chan DIR
    if (HAL_GPIO_ReadPin (CYBLE_DIR_GPIO_Port, CYBLE_DIR_Pin) == 0)   //quay nguoc
    {
        if (sPulse->Direct_u8 != 1)   //neu nhu Flag bao khong phai quay nguoc thi can Set lai    
        {
            sPulse->NumStartReverse_u32 = HAL_LPTIM_ReadCounter(&hlptim1);  
            sPulse->Direct_u8  = 1;
        }
    } else
    {
        if (sPulse->Direct_u8 == 1)     //neu nhu luc truoc no co phat hien quay nguoc thi ghi lai 
        {
            sPulse->NumEndReverse_u32 = HAL_LPTIM_ReadCounter(&hlptim1);
            sPulse->Direct_u8  = 2;
        }
    }
}

uint16_t LPTIM_Check_Pulse_Period (uint16_t Last_Pulse , uint16_t First_Pulse)
{
    if (Last_Pulse < First_Pulse)
		return (0xFFFF - First_Pulse + Last_Pulse + 1);
	return (Last_Pulse - First_Pulse);
}














