/*
    8/2021
    Thu vien xu ly string
*/

#include "user_time.h"
#include "user_string.h"

/*======================== Init Structs ======================*/
/*
     Ham tra ve vi tri cua xau S trong chuoi Master
     Length : do dai cua xau s
     Master : Mang Dich Can tim kiem
     Area   : khoang tim kiem xau S trong mang Dich (bat dau tu vtri 0)
     Ham  = -1 : neu ko tim thay chuoi
    		= i  : (0-32768) tim dc chuoi, i la vi tri chuoi
*/
int16_t Find_String (uint8_t *s, uint8_t Length, uint8_t *Master, int16_t Area) 
{
	int16_t     i,j;
	uint8_t     Check = 1;
    
	if (Area <  Length) 
        return -1;
	for (i = 0; i <= Area - Length; ++i) 
    {
		if (*s == *(Master +i))  // tim dc ky tu dau giong
        {
			Check = 1;
			for (j = 1; j < Length; ++j)
				if (*(s+j) != *(Master + i + j)) 
                    Check = 0;
            
			if (Check == 1) 
                return (i+1);
		}
	}
    
	return -1;
}

/* TESTED
 * Ham tra ve vi tri cua xau sTemp_Receiv trong chuoi sResponding
 * Ham  = -1 : neu ko tim thay chuoi
 * 		= i  : (0-32768) tim dc chuoi, i la vi tri chuoi
 * sTemp_Receiv : cau truc du lieu CHUAN can so sanh
 * 		- Dang du lieu vao "abc...|xy|zt" = tim 2 chuoi "abc...xy" va "abc...zt"
 * 		- Dang 2 : "!abc12"  : ham se tra ve TRUE (>=0) khi khong tim thay chuoi "abc12"
 * sResponding : chuoi nhan duoc qua UART
 * trong sTemp_Receiv chua ky tu "|" - phia sau la so sanh OR 
 * 	Note :
 * 		- do dai ham se ko phai la Do dai thuc su cua ham
 * 	ex : find s "CREG: 0,|1|5" = find 2 string "CREG: 0,1" va "CREG: 0,5"
 * 	Except :
 * 		- Khong tim duoc 2 doan ky tu trong sTemp_Receiv khac nhau hoan toan "|error|CPIN"
 */

int16_t Find_String_V2 (sData *sTemp_Receiv, sData *sResponding)
{
    int16_t     i = 0,j = 0;
    int16_t     Pos_Sharp_first = 0, Pos_Sharp = 0;
    uint8_t     Check = 1;
	uint8_t     Check_sharp = 0;
	uint8_t	    Off_Set = 0;
    
	if (*(sTemp_Receiv->Data_a8) == '!') 
        Off_Set = 1;

	for (i = 0; i < sResponding->Length_u16; ++i) 
    {
		if (*(sTemp_Receiv->Data_a8 + Off_Set) == *(sResponding->Data_a8 +i))  // tim dc ky tu dau giong
        {
			Check = 1;
			j = 1 + Off_Set;
			Check_sharp = 0;
			Pos_Sharp = 0;
			while (j < sTemp_Receiv->Length_u16) 
            {
				if (*(sTemp_Receiv->Data_a8+j) == '|') 
                {
					if (Pos_Sharp == 0)  // Luu lai vi tri '|' dau tien
                    {
						Check_sharp = 1;
						Pos_Sharp_first = i + j;
					}
					Check = Find_str_fix (j + 1 ,sTemp_Receiv,sResponding, Pos_Sharp_first, &Pos_Sharp);
					if (Check == 1) 
                        return RC(i,Off_Set);
				}
				if ((Check_sharp == 0) && (*(sTemp_Receiv->Data_a8+j) != *(sResponding->Data_a8 + i + j)))
				{
					Check = 0;
					break;
				}
				if (Check_sharp == 0) 
                    j++;
				else 
                {
					if (j < Pos_Sharp) 
                        j = Pos_Sharp;
					else 
                        j++;
				}
			}
			if (Check == 1) 
                return RC(i, Off_Set);
		}
	}
    
	return RC (-1, Off_Set);
}



int16_t RC(int16_t Result,uint8_t Off_Set)
{
	if (Off_Set == 0) 
        return Result;
    
	if (Result >= 0) 
        return -1;
	else 
        return 0;
}

/*
 * return 	-1 	: neu ko tim thay chuoi
 * 			1	: Neu chuoi dung
 */

int16_t Find_str_fix (int16_t pos_fix, sData *sTemp_Receiv, sData *sResponding, int16_t Pos_Res, int16_t *Pos_sharp)
{
	 int16_t    Result,count;
     
	 Result = 1,count = 0;
	 while ((pos_fix + count) < sTemp_Receiv->Length_u16) 
     {
		  if (*(sTemp_Receiv->Data_a8+pos_fix+count) == '|') 
          {
			   *Pos_sharp = pos_fix + count;
			   return Result;
		  }
		  if ((Pos_Res + count) > sResponding->Length_u16)  // Vuot qua xau dich
          { 
			   *Pos_sharp = sResponding->Length_u16 + 1;
			   Result = -1;
		  }
		  if (*(sTemp_Receiv->Data_a8+pos_fix+count) != *(sResponding->Data_a8 + Pos_Res + count)) 
              Result = -1;
		  count++;
	 }

	 return Result;
}

/*
 * 		COPY STRING
 * Param :
 * 		- sData *sTarget : Chuoi su dung
 * 		- sData *sCopy	: chuoi bi Copy
 * 		- Length_Copy			: so Ky tu Copy tu *sCopy (=255 - Copy all)
 * 		- Length_Max			: Do dai toi da co the Copy
 * Return :
 * 		0 - FALSE 	: 	ERROR :tran chuoi copy vao,
 * 		1 - True 	:	Copy thanh con
 */
uint8_t	Copy_String (sData *sTarget, sData *sCopy, uint8_t Length_Copy, uint8_t Length_Max)
{
	uint8_t var;
	if (Length_Copy == 255) 
        Length_Copy = sCopy->Length_u16;
    
	if ((sTarget->Length_u16 + Length_Copy) >= Length_Max) 	
		return 0;

	for (var = 0; var < Length_Copy; ++var) 
		*(sTarget->Data_a8 +sTarget->Length_u16 + var) = *(sCopy->Data_a8 + var);
    
	sTarget->Length_u16 +=  Length_Copy;
    
	return 1;
}

uint8_t	Copy_String_section (sData *sTarget, sData *sCopy, int Pos_Target, int Pos_Copy, int Length_Copy, int Length_Max)
{	
	uint16_t var;
	if ((sTarget->Length_u16 + Length_Copy) >= Length_Max) 
		return 0;

	for (var = 0; var < Length_Copy; ++var) 
		*(sTarget->Data_a8 + var + Pos_Target) = *(sCopy->Data_a8 + var + Pos_Copy);

	sTarget->Length_u16 +=  Length_Copy;
	return 1;
}

/*
 * Param :
 * 		str_in : chuoi dau vao can chia
 * 		Pos_find : bi tri bat dau tim trong str_in, va vi tri tra ve sau khi tim
 * 		Path : chuoi sau khi da cat ra
 * 	Return :
 * 		> 0 : OK
 * 		0 : Not find
 */

uint8_t Cut_String(sData *str_in,int *Pos_find,sData *Path)
{
	int Pos_Copy;
	int Length_Copy;
    
	Pos_Copy = *Pos_find;
	Pos_Copy++;
	Length_Copy = *(str_in->Data_a8 + Pos_Copy);
	Pos_Copy++;	
	if (Copy_String_section (Path, str_in, 0, Pos_Copy, Length_Copy, 20) == FALSE)   //LENGTH_FTP
        return 0;
	Pos_Copy += Path->Length_u16;
	if (Pos_Copy > str_in->Length_u16) 
        return 0;	
	*Pos_find = Pos_Copy;
    
	return 1;
}

/*
 * return
 *   0 : false
 *   >0 : OK
 */

uint8_t Cut_Value(sData *str_in, int *Pos_find, uint32_t *Value)
{	
    int Pos_Copy;
    int Length_Copy;
	// Find PATH
	Pos_Copy = *Pos_find;
	Pos_Copy++;
	*Value = 0;
	Length_Copy = *(str_in->Data_a8 + Pos_Copy);
	if (Length_Copy > 4) 
        return 0;
	while (Length_Copy > 0) 
    {
		Length_Copy--;
		Pos_Copy++;
		*Value = *Value << 8;
		*Value |= *(str_in->Data_a8 + Pos_Copy);
	}
	*Pos_find = Pos_Copy;
	return 1;
}


uint8_t	Copy_String_2 (sData *sTarget, sData *sCopy)
{
	uint16_t var;
    
	for (var = 0; var < sCopy->Length_u16; ++var) 
		*(sTarget->Data_a8 +sTarget->Length_u16 + var) = *(sCopy->Data_a8 + var);

	sTarget->Length_u16 +=  sCopy->Length_u16;
    
	return 1;
}

//copy 1 chuoi str vao 1 vi tri o trong chuoi dich. Pos <= length taget
uint8_t	Copy_String_toTaget (sData *sTarget, uint16_t Pos, sData *sCopy)
{
	uint16_t var;
    
    if (Pos > sTarget->Length_u16) 
        return 0;
    //dich chuoi taget ra sau "length vi tri" can copy vao
    for(var = sTarget->Length_u16; var > Pos; var--)
        *(sTarget->Data_a8 + var + sCopy->Length_u16 - 1) = *(sTarget->Data_a8 + var - 1);

    sTarget->Length_u16 += sCopy->Length_u16;
    //ghi gi� tri chuoi copy vao vi tri pos
	for (var = 0; var < sCopy->Length_u16; ++var) 
		*(sTarget->Data_a8 +Pos + var) = *(sCopy->Data_a8 + var);
    
	return 1;
}


uint8_t	Copy_String_STime (sData *sTarget, ST_TIME_FORMAT Stime)
{
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.year/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.year%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.month/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.month%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.date/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.date%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.hour/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.hour%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.min/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.min%10 + 0x30);
    
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) ((Stime.sec/10)%10 + 0x30);
    *(sTarget->Data_a8 +sTarget->Length_u16++) = (uint8_t) (Stime.sec%10 + 0x30);
    
	return 1;
}


uint8_t Compare_String (sData Str1, sData Str2)
{
    uint16_t  i = 0;
    
    if (Str1.Length_u16 != Str2.Length_u16) 
        return 0;
    
    for (i = 0; i < Str1.Length_u16; i++)
        if (*(Str1.Data_a8 + i) != *(Str2.Data_a8 + i)) 
            return 0;
    
    return 1;
}


void ConvertStringsTime_tosTime (uint8_t* Buff, ST_TIME_FORMAT* Stime)
{
    //convert thoi gian
    Stime->year  = (Buff[0] - 0x30) * 10 + (Buff[1] - 0x30);
    Stime->month = (Buff[2] - 0x30) * 10 + (Buff[3] - 0x30);
    Stime->date  = (Buff[4] - 0x30) * 10 + (Buff[5]- 0x30); 
    
    Stime->hour  = (Buff[6] - 0x30) * 10 + (Buff[7] - 0x30);
    Stime->min   = (Buff[8] - 0x30) * 10 + (Buff[9] - 0x30);
    Stime->sec   = (Buff[10] - 0x30) * 10 + (Buff[11]- 0x30); 
}


void Copy_STime_fromsTime (ST_TIME_FORMAT *sTimeTaget, uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t min, uint8_t sec)
{
    sTimeTaget->year = year;
    sTimeTaget->month = month;
    sTimeTaget->date = date;
    sTimeTaget->hour = hour;
    sTimeTaget->min = min;
    sTimeTaget->sec = sec;
}



//Return 1: Stime1 > STime 2   : Fortime = 0
//Reutrn 0: Stime 1 < sTime 2 : ForTime = Stime2 - Stime 1 (sec)

int8_t Compare_sTime (ST_TIME_FORMAT sTime1, ST_TIME_FORMAT sTime2, uint32_t* ForTime)    
{
    uint32_t Sec1 = 0;
    uint32_t Sec2 = 0;
    
    Sec1 = HW_RTC_GetCalendarValue_Second(sTime1 ,0);   
    Sec2 = HW_RTC_GetCalendarValue_Second(sTime2, 0);
    
    if (Sec1 >= Sec2) 
    {
        *ForTime = 0;
        return 1;
    } else
    {
        *ForTime = Sec2 - Sec1;
    }
    
    return 0;
}



//them dau . thap phan vao trong chuoi Decimal cua 1 so




