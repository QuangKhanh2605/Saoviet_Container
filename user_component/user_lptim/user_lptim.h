


#ifndef _USER_LPTIM_H
#define _USER_LPTIM_H 	

#include "user_util.h"
#include "lptim.h"


/*================Define=====================*/

#ifndef LPTIM_ENCODER_MODE
	#define LPTIM_ENCODER_MODE
#endif

/*================Var struct=====================*/
typedef enum
{
    _DIRECT_FORWARD = 1,
    _DIRECT_REVERSE,
    _DIRECT_END,
}Kind_Direct_WM;


typedef struct
{
    uint16_t    FlowHigh;
    uint16_t    FlowLow;
    
    uint16_t    PeakHigh;
    uint16_t    PeakLow;
    
    uint8_t     LowBatery;
    
    int16_t     LevelHigh;
    int16_t     LevelLow;
}struct_ThresholdConfig;


typedef struct
{
	uint32_t	Number_u32;
    uint32_t	NumberStartInit_u32;
    uint8_t     Direct_u8;              //0: quay thuan, 1: quay nguoc
    int32_t	    Flow_i32;               //luu luong    
    int32_t	    Quantity_i32;           //San luong 
    
    uint16_t	NumStartReverse_u32;   //Num start Reverse
    uint16_t	NumEndReverse_u32;     //Num end Reverse    

}Struct_Pulse;


/*================Extern var struct=====================*/
extern void (*pLPTIM_Func_Direct_Change_Handler) (uint8_t Type);


/*================Function=====================*/
void        LPTIME_Encoder_Get_Pulse (Struct_Pulse *sPulse);
void        BSP_sensor_Read (uint16_t *sensor_data);

uint16_t    LPTIM_Check_Pulse_Period (uint16_t Last_Pulse , uint16_t First_Pulse);
void        LPTIM_Check_DIR_PIN (Struct_Pulse *sPulse);

void        LPTIM_Init_Feature (void);

#endif
