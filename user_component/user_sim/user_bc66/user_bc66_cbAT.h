/*
 * user_bc66_cbAT.h
 *
 *  Created on: 6 Jan 2022
 *      Author: Chien
 */

#ifndef USER_BC66_CBAT_H_
#define USER_BC66_CBAT_H_

#include "user_util.h"
#include "user_define.h"



/*================Func======================*/

uint8_t _Cb_AT_Success(sData *str_Receiv);
uint8_t _Cb_AT_Failure(uint8_t Type);

uint8_t _Cb_AT_CHECK_SIM(sData *str_Receiv);
uint8_t _Cb_AT_CHECK_RSSI(sData *str_Receiv);
uint8_t _Cb_AT_SIM_ID(sData *str_Receiv);

uint8_t _Cb_AT_SET_BAND_3 (sData *str_Receiv);
uint8_t _Cb_AT_CHECK_CREG(sData *str_Receiv);
uint8_t _CB_AT_DIS_URC(sData *str_Receive);

uint8_t _Cb_AT_GET_CLOCK(sData *str_Receiv);
uint8_t _Cb_AT_TCP_CONNECT_1(sData *str_Receiv);
uint8_t _Cb_AT_TCP_CONNECT_2(sData *str_Receiv);
uint8_t _Cb_AT_CHECK_ATTACH(sData *str_Receive);

uint8_t _Cb_AT_Power_Off(sData *str_Receive);
uint8_t _Cb_AT_PSM_Success(sData *str_Receive);

uint8_t _Cb_MQTT_CONNECT_1(sData *str_Receiv);
uint8_t _Cb_MQTT_CONNECT_2(sData *str_Receiv);
uint8_t _Cb_MQTT_SUBCRIBE_1(sData *str_Receiv);
uint8_t _Cb_MQTT_SUBCRIBE_2(sData *str_Receiv);
uint8_t _Cb_MQTT_PUBLISH_1(sData *str_Receiv);
uint8_t _Cb_MQTT_PUBLISH_2(sData *str_Receiv);

/*========================Func Cb URC BC66========================*/
uint8_t _Cb_URC_ERROR(sData *str_Receive);
uint8_t _Cb_URC_WAKEUP(sData *str_Receive);


#endif /* USER_BC66_CBAT_H_ */
