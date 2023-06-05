/*
 * user_define.h
 *
 *  Created on: Dec 31, 2021
 *      Author: Chien
 */

#ifndef INC_USER_DEFINE_H_
#define INC_USER_DEFINE_H_


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
    #include "user_app_temh.h"
#endif 



#include "Customer_Config.h"

#include "user_app_main.h"
#include "user_modem.h"
#include "user_modem_init.h"

#include "user_uart.h"
#include "event_driven.h"
#include "queue_p.h"

#include "user_at_serial.h"

#include "user_time.h"
#include "user_external_mem.h"
#include "user_internal_mem.h"

#include "user_obis_sv.h"




/*========================== User define =================================*/





#endif /* INC_USER_DEFINE_H_ */
