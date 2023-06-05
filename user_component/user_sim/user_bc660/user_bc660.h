
/*
 * user_bc66.h
 *
 *  Created on: Dec 31, 2021
 *      Author: Chien
 */

#ifndef USER_BC660_H_
#define USER_BC660_H_


#include "user_util.h"
#include "main.h"

/*=============Define====================*/
#define BC660_POW_PIN_ON  		    HAL_GPIO_WritePin(ON_OFF_SIM_GPIO_Port,ON_OFF_SIM_Pin,GPIO_PIN_RESET)
#define BC660_POW_PIN_OFF      	    HAL_GPIO_WritePin(ON_OFF_SIM_GPIO_Port,ON_OFF_SIM_Pin,GPIO_PIN_SET)

// ********** PWKEY
#define BC660_PWKEY_PIN_ON 		  	HAL_GPIO_WritePin(BOOT_GPIO_Port, BOOT_Pin, GPIO_PIN_SET)
#define BC660_PWKEY_PIN_OFF  	   	HAL_GPIO_WritePin(BOOT_GPIO_Port, BOOT_Pin, GPIO_PIN_RESET)


#define BC660_RESET_PIN_ON  		HAL_GPIO_WritePin(RESET_BC66_GPIO_Port, RESET_BC66_Pin, GPIO_PIN_SET)
#define BC660_RESET_PIN_OFF  	   	HAL_GPIO_WritePin(RESET_BC66_GPIO_Port, RESET_BC66_Pin, GPIO_PIN_RESET)

#define BC660_WAKEUP_PIN_ON			HAL_GPIO_WritePin(PSM_WK_GPIO_Port, PSM_WK_Pin, GPIO_PIN_SET);
#define BC660_WAKEUP_PIN_OFF		HAL_GPIO_WritePin(PSM_WK_GPIO_Port, PSM_WK_Pin, GPIO_PIN_RESET);


/*===================var struct enum===========*/

typedef enum
{
	_AT_TEMP,
	_AT_CHECK_AT,
    
	_AT_SIM_ID,
	_AT_BAUD_RATE,
	_AT_DISPLAY_CMD,
    _AT_CHECK_SIM,
    _AT_CHECK_RSSI,
	_AT_DEFINE_APN,
    
    _AT_SAVE_SETTING,
	_AT_POWER_OFF,
    
	_AT_CHECK_ATTACH,
	_AT_SET_BAND_3,
	_AT_SET_BAND_ALL,
	_AT_CONNECT_URC,
	_AT_DIS_CONNECT_URC,
	_AT_GET_RTC,
    
	_AT_DIS_DEEP_SLEEP,
    _AT_EN_DEEP_SLEEP,
    _AT_EN_LIGHT_SLEEP,
  
    _AT_DISABLE_PSM,
    _AT_ENSABLE_PSM,
    _AT_EN_WAKEUP_URC,
    _AT_ENABLE_eDRX,
    _AT_DISABLE_eDRX,
    
    _AT_SOFT_RESET,
    
	_AT_TCP_CONFIG_DATA,
    _AT_TCP_CONFIG_VIEW,
    _AT_TCP_CONFIG_SHOW,
    
    _AT_CHECK_CREG,
      
    _AT_TCP_ACESS_MODE,
    _AT_TCP_PUSH_MODE,
    
    _AT_TCP_OPEN_1,
    _AT_TCP_OPEN_2,
    
	_AT_MQTT_CONN_1,
	_AT_MQTT_CONN_2,

	_AT_MQTT_SUB_1,
	_AT_MQTT_SUB_2,

	_AT_MQTT_PUB_1,
	_AT_MQTT_PUB_2,

    _AT_MQTT_PUB_FB_1,
    _AT_MQTT_PUB_FB_2,
    
    _AT_TCP_CLOSE,
    
	SIM_CMD_END, // don't use
    
}AT_SIM_TypeDef;


typedef enum
{   
    SIM_URC_RESET_SIM900,
    SIM_URC_ALREADY_CONNECT,
    SIM_URC_SIM_LOST,
    SIM_URC_CLOSED,
    SIM_URC_CALL_READY,
    SIM_URC_ERROR,
    SIM_URC_WAKEUP,
    SIM_URC_DEEP_SLEEP,
    SIM_URC_RECEIV_SERVER,
    
	SIM_URC_END,
}Kind_URC_BC66;


typedef uint8_t (*CB_AT_Success) (sData *str_Receive);
typedef uint8_t (*CB_AT_Failure) (uint8_t Type);
typedef struct
{
	uint8_t at_name;
	char	*at_string;
	char 	*at_response;
	CB_AT_Success	callback_success;
	CB_AT_Failure	callback_failure;
}sCommand_Sim_Struct; // it is not a declaration, it is a new type of data

extern const sCommand_Sim_Struct aSimStep[];
extern const sCommand_Sim_Struct aSimUrc[];

extern uint8_t aSIM_STEP_CONTROL[28];
extern uint8_t aSIM_STEP_POWER_OFF[1];
extern uint8_t aSIM_STEP_CLOSE_TCP[1];
extern uint8_t aSIM_STEP_PUBLISH[2];
extern uint8_t aSIM_STEP_PUBLISH_FB[2];
extern uint8_t aSIM_STEP_LOW_POWER[2];


/*================Function===============*/
uint8_t 	BC660_ON(void);
uint8_t 	BC660_Hard_Reset(void);
uint8_t 	BC660_Wake_Up(void);
void        Init_Timer_PowerOn (void);

#endif /* USER_BC660_H_ */
