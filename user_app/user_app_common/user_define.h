/*
 * user_define.h
 *
 *  Created on: Dec 31, 2021
 *      Author: Chien
 */

#ifndef INC_USER_DEFINE_H_
#define INC_USER_DEFINE_H_

#include "user_app_comm.h"

/* 
    - Using lib app sim
    #define INC_APP_SIM_FEATURE
*/
#define INC_APP_SIM_FEATURE
#ifdef INC_APP_SIM_FEATURE
    #include "user_app_sim.h"
#endif 

/* 
    - Using lib app water meter
    #define INC_APP_WM_FEATURE
*/
//    #define INC_APP_WM_FEATURE
#ifdef INC_APP_WM_FEATURE
    #include "user_app_wm.h"
#endif 

/* 
    - Using lib app Emeter
    #define INC_APP_EMET_FEATURE 
*/

#ifdef INC_APP_EMET_FEATURE
    #include "user_app_emet.h"
#endif 

/* 
    - Using lib app Lora
    #define INC_APP_LORA_FEATURE 
*/
//#define INC_APP_LORA_FEATURE 

#ifdef INC_APP_LORA_FEATURE
    #include "user_app_lora.h"
#endif 


/* 
    - Using lib app Lora
    #define INC_APP_TEMH_FEATURE 
*/
#define INC_APP_TEMH_FEATURE 

#ifdef INC_APP_TEMH_FEATURE
    #include "user_app_inverter.h"
#endif 


#include "user_app_mem.h"

#include "user_modem.h"
#include "user_modem_init.h"

#include "user_uart.h"
#include "event_driven.h"
#include "queue_p.h"

#include "user_at_serial.h"

#include "user_time.h"
#include "user_obis_sv.h"

  

/*========================== User define =================================*/
/*     Chu ki wake up
      - Chu ki nay se duoc set timer danh thuc MCU day.
      - Don vi (s)
*/
#define DEFAULT_FREQ_WAKEUP                             2     //don vi min

/* So lan thiet bi day roi moi gui ban tin */    //tu thong so nay va chu ki wakeup se tinh ra duoc chu ki cua ban tin
#define DEFAULT_NUMBER_WAKE_UP                          1

#define MAX_NUMBER_WAKE_UP                              10
#define MAX_NUNBER_SEND_INIT                            1
#define MAX_MESS_IN_PACKET                              1       //1 ban tin 31 byte. Max 512 byte - 45 byte header

/*--- ID cua thiet bi -------*/
#define DEVICE_ID                                       "SVWMCI23000002"  


#define DEFAULT_FREQ_SEND_ONLINE                        2






#endif /* INC_USER_DEFINE_H_ */
