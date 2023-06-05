
#ifndef _USER_LIQUID_LEVEL_
#define _USER_LIQUID_LEVEL_	_USER_LIQUID_LEVEL_H



#include "user_util.h"



/*======================== Define ======================*/
#define DE_GPIO_PORT    RS485_TXDE_GPIO_Port  
#define DE_GPIO_PIN     RS485_TXDE_Pin     
#define UART_485	    huart3


#define RS485_POWER_PORT        GPS_RESET_GPIO_Port     //thay doi chan nay thanh chan on/off 485
#define RS485_POWER_PIN         GPS_RESET_Pin    

#define RS485_ON         HAL_GPIO_WritePin(RS485_POWER_PORT, RS485_POWER_PIN, GPIO_PIN_RESET);
#define RS485_OFF         HAL_GPIO_WritePin(RS485_POWER_PORT, RS485_POWER_PIN, GPIO_PIN_SET);


/*=== Register S_YW_01B ===*/
#define REGIS_SLAVE_ID	    0x0000
#define REGIS_BAURATE	    0x0001
#define REGIS_UNIT  	    0x0002
#define REGIS_DECIMAL	    0x0003
#define REGIS_VALUE 	    0x0004
#define REGIS_ZERO_POINT    0x0005


#define NUM_REGISTER_READ   6

typedef struct
{
    uint16_t        Unit_u16;
    uint16_t        Decimal_u16;
    int16_t         Value_i16;
    int16_t         ZeroPoint_i16;
}structFloatValue;



/*======================== Funcion ======================*/


void        MeaLevel_Read_Value (uint8_t SlaveID, void (*pFuncResetRecvData) (void)) ;
uint8_t     MeaLevel_Extract_Data (uint8_t SlaveID, structFloatValue *pFloatValue, uint8_t *pSource, uint16_t Length);

#endif






