
/*
    8/2021
    Thu vien xu ly string
*/
#ifndef USER_STRING_H
#define USER_STRING_H

#include "user_util.h"

#include "stdlib.h"


/*======================== Structs ======================*/


/*======================== FUNCTION ======================*/
/*
 * 		Find String
 */
int16_t         Find_String(uint8_t *s, uint8_t Length, uint8_t *Master, int16_t Area) ;
int16_t         Find_String_V2(sData *sTemp_Receiv,sData *sResponding);
int16_t         RC(int16_t Result,uint8_t Off_Set);
int16_t         Find_str_fix(int16_t pos_fix,sData *sTemp_Receiv,sData *sResponding,int16_t Pos_Res, int16_t *Pos_sharp);
uint8_t	        Copy_String(sData *sTarget, sData *sCopy, uint8_t Length_Copy, uint8_t Length_Max);
uint8_t	        Copy_String_section(sData *sTarget, sData *sCopy, int Pos_Target, int Pos_Copy, int Length_Copy, int Length_Max);
uint8_t         Cut_String(sData *str_in,int *Pos_find,sData *Path);
uint8_t         Cut_Value(sData *str_in,int *Pos_find,uint32_t *Value);
uint8_t	        Copy_String_2 (sData *sTarget, sData *sCopy);
uint8_t	        Copy_String_STime (sData *sTarget, ST_TIME_FORMAT Stime);

uint8_t	        Copy_String_toTaget (sData *sTarget, uint16_t Pos, sData *sCopy);
uint8_t         Compare_String (sData Str1, sData Str2);
		//2021
void            ConvertStringsTime_tosTime (uint8_t* Buff, ST_TIME_FORMAT* Stime);
void            Copy_STime_fromsTime (ST_TIME_FORMAT *sTimeTaget, uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec);
int8_t          Compare_sTime (ST_TIME_FORMAT sTime1, ST_TIME_FORMAT sTime2, uint32_t *ForTime);

#endif /*  */

