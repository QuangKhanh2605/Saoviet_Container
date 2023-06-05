


#ifndef USER_MODEM_INIT_H
#define USER_MODEM_INIT_H

#include "user_util.h"




/*================ Func ================== */
void 		Init_Memory_Infor(void);
void        Modem_Init_Feature (void);

void 		Init_DCU_ID(void);
void 		Save_DCU_ID(void);

void        Init_Timer_Send (void);
void        Init_Index_Packet (void);

void        Save_Freq_Send_Data (void);

#endif