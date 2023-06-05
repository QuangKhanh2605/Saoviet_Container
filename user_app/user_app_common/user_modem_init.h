


#ifndef USER_MODEM_INIT_H
#define USER_MODEM_INIT_H

#include "user_util.h"




/*================ Func ================== */
void 		Init_Memory_Infor(void);

void 		Init_DCU_ID(void);
void 		Save_DCU_ID(void);

void        Init_Timer_Send (void);
void        Init_Index_Packet (void);

void        Save_Freq_Send_Data (void);

uint16_t    mInit_Index_Record (uint32_t Addr, uint16_t MaxRecord);
void        mSave_Index_Record (uint32_t Addr, uint16_t Val);


#endif