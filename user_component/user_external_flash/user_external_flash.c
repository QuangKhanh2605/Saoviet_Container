
/*
    8/2021
    Thu vien: Quan ly external flash 
*/


#include "user_external_flash.h"



/*===================Var struct======================================*/
uint32_t        pos_DCU_u32;
//Vi tri luu firmware
uint32_t        pos_FirmWrite_u32;
uint32_t        pos_FirmRead_u32;

uint8_t         ReadBackBuff[METER_LOG_MESSAGE_SIZE];

Struct_Manage_External_Flash    sExFlash;

Manage_Flash_Struct				Manage_Flash_TSVH = 
{
    .AddStartA_u32  = ADDR_METER_A_START,
    .AddStopA_u32   = ADDR_METER_A_STOP,
    .AddStartB_u32  = ADDR_METER_B_START,
    .AddStopB_u32   = ADDR_METER_B_STOP,
    .AddOffsetAB_u32 = ADDR_OFFSET_METER_INFO,

    .PosMessPartA_u32 = ADDR_METER_A_START,
    .PosMessPartB_u32 = ADDR_METER_B_START, 
};
    
Manage_Flash_Struct				Manage_Flash_Lpf = 
{
    .AddStartA_u32  = ADDR_LPF_A_START,
    .AddStopA_u32   = ADDR_LPF_A_STOP,
    .AddStartB_u32  = ADDR_LPF_B_START,
    .AddStopB_u32   = ADDR_LPF_B_STOP,
    .AddOffsetAB_u32 = ADDR_OFFSET_METER_LPF,

    .PosMessPartA_u32 = ADDR_LPF_A_START,
    .PosMessPartB_u32 = ADDR_LPF_B_START, 
};

Manage_Flash_Struct				Manage_Flash_Bill = 
{
    .AddStartA_u32  = ADDR_BILL_A_START,
    .AddStopA_u32   = ADDR_BILL_A_STOP,
    .AddStartB_u32  = ADDR_BILL_B_START,
    .AddStopB_u32   = ADDR_BILL_B_STOP,
    .AddOffsetAB_u32 = ADDR_OFFSET_METER_BILL,

    .PosMessPartA_u32 = ADDR_BILL_A_START,
    .PosMessPartB_u32 = ADDR_BILL_B_START, 
};  

//--------------------Khai bao them cai queue trong Readmeter
Meter_Flash_Queue_Struct		sQMeterFlashInfo, *ptrsQMeterFlashInfo;
Meter_Flash_Queue_Struct		sQMeterSIM_Data, *ptrsQMeterSIM_Data;

Meter_Flash_Queue_Struct		sQFlashSIM_Info1, *ptrsQFlashSIM_Info1;
Meter_Flash_Queue_Struct		sQFlashSIM_Info2, *ptrsQFlashSIM_Info2;

/*============Ðoc old packet flash - 23/12/21===================*/
struct_Manage_Read_Old_Packet    sOldPacket[3] =
{
    {ADDR_METER_A_STOP, ADDR_METER_A_START},
    {ADDR_BILL_A_STOP, ADDR_BILL_A_START},
    {ADDR_LPF_A_STOP, ADDR_LPF_A_START},    
};



/*===================Function======================================*/

uint8_t Init_External_Flash(void)
{
    uint8_t Result = 0;
    
    Result |= Check_Flash_Pos(&Manage_Flash_TSVH);
    Result |= Check_Flash_Pos(&Manage_Flash_Lpf);
    Result |= Check_Flash_Pos(&Manage_Flash_Bill);
    Result |= Check_Flash_1Part(&pos_DCU_u32, ADDR_BASE_LOG_DCU, ADDR_TOP_LOG_DCU);
        
    //Init Struct Mess Fail
//    eInit_Total_Mess_Fail();
    
    return Result;
}
         



/*******************************************************************************

* Function Name  : Flash_Check_Message
* Description    : Check position Log Meter and Log GSM 
* Output         : pos_MeterInfor_To_Send_u32, pos_MeterInfor_Sent_u32

*******************************************************************************/

uint8_t Check_Flash_Pos(Manage_Flash_Struct *ManageType)
{
	uint8_t 	read_data[64] = {0};
	uint8_t		i = 0, j = 0;
	uint8_t		sent_buff_flag = 1;
	uint8_t		sent_buff_empty_flag = 0;
	uint8_t		sent_buff_full_flag = 0;
	uint32_t	writing_address=0;
	uint8_t     Result = 0;  
    
	/* Check TSVH to Sent */
	writing_address                 = ManageType->AddStartA_u32;     //Vi tri bat dau check 
    ManageType->PosMessPartA_u32    = ManageType->AddStartA_u32;    //Gan Pos check duoc o vi tri start
	ManageType->BuffA_Writting_S_ui16 = 1;  //Sector check duoc = 1
	while (writing_address < ManageType->AddStopA_u32)
	{
        UTIL_MEM_set(&read_data[0], 0, sizeof(read_data));
		if (Flash_S25FL_BufferRead(read_data, writing_address, 64) == 1)
		{
			j = 0;
			for (i = 0; i < 64; i++)
				if (read_data[i]==0xFF)
					j++;
            
			if (j == 64)
			{
				ManageType->PosMessPartA_u32 = writing_address;
				break;
			}

			writing_address = writing_address + METER_LOG_MESSAGE_SIZE;	
			
			if (deFLASH_S25FL_PAGE_OF_ADDRESS(writing_address) % 16 == 0) 		 	//***********NOTE**********
			{
				writing_address = writing_address + S25FL_PAGE_SIZE;	 		    //***********NOTE**********
				ManageType->BuffA_Writting_S_ui16++;
			}
			
			if (writing_address > ManageType->AddStopA_u32)
			{
				ManageType->PosMessPartA_u32 = ManageType->AddStartA_u32;
				if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartA_u32) != 1)
				{
					Result = 0x02; // Loi Init Flash, bao loi len Server
					break;
				}
				ManageType->BuffA_Writting_S_ui16 = 1;
				break;
			}
		} else
		{
			Result = 0x02; // Loi Init Flash, bao loi len Server
			break;
		}
	}
	
	//Check log sent status	
	writing_address                     = ManageType->AddStartB_u32;
	ManageType->PosMessPartB_u32        = ManageType->AddStartB_u32;
	ManageType->BuffB_Writting_S_ui16   = 1;
    
	while (writing_address < ManageType->AddStopB_u32)
	{
		read_data[0] = 0;read_data[1] = 0;read_data[2] = 0;read_data[3] = 0;
		if (Flash_S25FL_BufferRead(read_data, writing_address, 4) == 1)
		{
			if ((read_data[0] == 0xFF) && (read_data[1] == 0xFF) && (read_data[2] == 0xFF) && (read_data[3] == 0xFF))
			{
				sent_buff_full_flag = 1;
				if (sent_buff_flag == 1)
				{
					ManageType->PosMessPartB_u32 = writing_address;
					sent_buff_flag = 0;
				}
			} else
			{
				sent_buff_empty_flag = 1;
				sent_buff_flag = 1;
			}
			
			writing_address = writing_address + METER_LOG_MESSAGE_SIZE;	
			
			if (deFLASH_S25FL_PAGE_OF_ADDRESS(writing_address) % 16 == 0) 		 	//***********NOTE**********
				writing_address = writing_address + S25FL_PAGE_SIZE;	 		//***********NOTE**********		
		} else
		{
			Result = 0x02; // Loi Init Flash, bao loi len Server
			break;
		}
	}
	//Neu tat ca con trong thi Vi tri cua dau Sector to Sent + offset
	if (sent_buff_empty_flag == 0)  
	{
		read_data[0] = 0;read_data[1] = 0;read_data[2] = 0;read_data[3] = 0;
		if (Flash_S25FL_BufferRead(read_data, (ManageType->BuffA_Writting_S_ui16 * S25FL_SECTOR_SIZE + ManageType->AddStartA_u32), 4) == 1)
		{
			if ((read_data[0] != 0xFF) && (read_data[1] != 0xFF) && (read_data[2] != 0xFF) && (read_data[3] != 0xFF))
				ManageType->PosMessPartB_u32 = ManageType->BuffA_Writting_S_ui16 * S25FL_SECTOR_SIZE + ManageType->AddStartB_u32;       
        } else
			Result = 0x02; // Loi Init Flash, bao loi len Server
	}
	//neu nhu full phan B thi Vi tri sent la dau tien phan B. Sau do can clear sector first
	if (sent_buff_full_flag == 0)  
		if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartB_u32) != 1)
			Result = 0x02; // Loi Init Flash, bao loi len Server
	
	ManageType->BuffB_Writting_S_ui16 = (ManageType->PosMessPartB_u32 - ManageType->AddStartB_u32) / S25FL_SECTOR_SIZE + 1;
    
    return Result;
}



/*
    - Function Check Pos Data chi luu trong 1 phan (A). Không chia 2 phan A B
    //Check log DCU
*/
uint8_t Check_Flash_1Part (uint32_t *PosResult, uint32_t ADD_START, uint32_t ADD_STOP)
{
    uint8_t 	read_data[64] = {0};
	uint32_t	writing_address=0;
	uint8_t     Result = 0;
    
	writing_address = ADD_START;
	while (writing_address < ADD_STOP)
	{
        UTIL_MEM_set(&read_data[0], 0, sizeof(read_data));
		if (Flash_S25FL_BufferRead(read_data,writing_address, 5) == 1)
		{
			if ((read_data[0] == 0xFF) && (read_data[1] == 0xFF))
			{
				*PosResult = writing_address;
				break;
			}

			writing_address = writing_address + DCU_LOG_MESSAGE_SIZE;
			
			if (writing_address > ADD_STOP)
			{
				pos_DCU_u32 = ADD_START;
                
				if (Flash_S25FL_Erase_Sector(pos_DCU_u32) != 1)
					Result = 0x02;        // Loi Init Flash, bao loi len Server
				break;
			}
		} else
		{
			Result = 0x02;              // Loi Init Flash, bao loi len Server
			break;
		}
 	}
    
    return Result;
}

/*
    - Function: Log cac ban tin phan A B cua ca data luu 2 phan
    - Input:    + PosA, PosB, ADD_Start_A, ADD_Stop_A, 
                + buff, length data
                + type: luu vao A or B
*/
uint8_t Flash_Log_Message(Manage_Flash_Struct *ManageType, uint8_t *Buff, uint16_t length, uint16_t TypePartAB)   
{
    uint8_t     crc = 0;     //1: Luu fail, 0 OK
	uint16_t    i = 0;
	
	//Reset Buff doc lai
    for (i = 0; i < METER_LOG_MESSAGE_SIZE; i++)
        ReadBackBuff[i] = 0xFF;
    
    if (TypePartAB == _PART_A)   //Luu phan A
    {
        /* Write message */
        if (Flash_S25FL_BufferWrite(Buff, ManageType->PosMessPartA_u32, length) != 1) 
            return 0;
        /* Read message to check */
        Flash_S25FL_BufferRead(&ReadBackBuff[0], ManageType->PosMessPartA_u32, length);
        /* Check readback data*/
        for (i = 0; i < length; i++)
            if (ReadBackBuff[i] != *(Buff + i))
                return 0;
        /* Compare Check Sum Byte */
        crc = 0;
        for (i = 0; i < length - 1; i++)
            crc += ReadBackBuff[i];         
        //Check crc
        if (crc != ReadBackBuff[length - 1])
            return 0;
        /* Caculator next Message */
        ManageType->PosMessPartA_u32 += METER_LOG_MESSAGE_SIZE;	

        /* bo qua page cuoi cua moi sector */
        if (deFLASH_S25FL_PAGE_OF_ADDRESS(ManageType->PosMessPartA_u32) % 16 == 0) 		    //***********NOTE**********
        {
            ManageType->PosMessPartA_u32 = ManageType->PosMessPartA_u32 + S25FL_PAGE_SIZE;	                    //***********NOTE**********
            if (ManageType->PosMessPartA_u32 >= ManageType->AddStopA_u32) 
            {
                ManageType->PosMessPartA_u32 = ManageType->AddStartA_u32;   
                ManageType->BuffA_Writting_S_ui16 = 1;
                if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartA_u32) != 1)  //Clear sector phia truoc phan A
                    return 0;
                
                HAL_Delay(50);
                
                if (Flash_S25FL_Erase_Sector(ManageType->AddStartA_u32 + ManageType->AddOffsetAB_u32) != 1) //Clear sector phia truoc phan B
                    return 0;
                
                if (ManageType->BuffB_Writting_S_ui16 == 1)      //B = 1 tuc la bang voi A thi công 1 cho phan B nhu o duoi
                {
                    ManageType->BuffB_Writting_S_ui16 = 2;
                    ManageType->PosMessPartB_u32 = ManageType->AddStartA_u32 + ManageType->AddOffsetAB_u32 + S25FL_SECTOR_SIZE;
                    ManageType->BuffA_Change_Sector_ui8 = 1;
                }					
            } else
            {
                ManageType->BuffA_Writting_S_ui16++;
                if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartA_u32) != 1)    //Clear sector phia truoc phan A
                    return 0;
                
                HAL_Delay(50);
                
                if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartA_u32 + ManageType->AddOffsetAB_u32) != 1)  //Clear sector phía truoc phan B
                    return 0;
                
                if (ManageType->BuffB_Writting_S_ui16 == ManageType->BuffA_Writting_S_ui16)
                {
                    ManageType->BuffB_Writting_S_ui16 = ManageType->BuffA_Writting_S_ui16 + 1;
                    ManageType->PosMessPartB_u32 = ManageType->PosMessPartA_u32 + ManageType->AddOffsetAB_u32 + S25FL_SECTOR_SIZE;
                    if (ManageType->PosMessPartB_u32 > (ManageType->AddStopA_u32 + ManageType->AddOffsetAB_u32))
                    {
                        ManageType->PosMessPartB_u32 = ManageType->AddStartA_u32 + ManageType->AddOffsetAB_u32;
                        ManageType->BuffB_Writting_S_ui16 = 1;
                    }
                    ManageType->BuffA_Change_Sector_ui8 = 1;
                }	
            }
        }
    }
    
    if (TypePartAB == _PART_B) //Luu phan B
    {
        /* Write message */
		if (Flash_S25FL_BufferWrite(Buff, ManageType->PosMessPartB_u32, length) != 1)
			return 0;
		/* Caculator next Message */
		ManageType->PosMessPartB_u32 = ManageType->PosMessPartB_u32 + METER_LOG_MESSAGE_SIZE;
		
		/* bo qua page cuoi cua moi sector */
		if (deFLASH_S25FL_PAGE_OF_ADDRESS(ManageType->PosMessPartB_u32) % 16 == 0)            //***********NOTE**********
		{
			ManageType->PosMessPartB_u32 = ManageType->PosMessPartB_u32 + S25FL_PAGE_SIZE; 		 //***********NOTE**********
			ManageType->BuffB_Writting_S_ui16++;
			if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartB_u32) != 1) 
                return 0;
		}
				
		if (ManageType->PosMessPartB_u32 >= (ManageType->AddStopA_u32 + ManageType->AddOffsetAB_u32))
		{
			ManageType->PosMessPartB_u32 = (ManageType->AddStartA_u32 + ManageType->AddOffsetAB_u32);
			ManageType->BuffB_Writting_S_ui16 = 1;
			if (Flash_S25FL_Erase_Sector(ManageType->PosMessPartB_u32) != 1) 
                return 0;
		}
    }
    
    return 1;
}




/*
    - Function Luu ban tin chi luu trong 1 phan: Khong chia A B
    - Input: + Pos
             + Add_Start, ADD_Stop, SizeLog  
             + Buff, length data
*/
uint8_t Flash_Log_Message_1Part (uint32_t *PosTaget, uint32_t ADD_START, uint32_t ADD_STOP, uint16_t SIZE_LOG, uint8_t *Buff, uint16_t length)   
{
    uint8_t     crc = 0;    
	uint16_t    i = 0;
    //Reset Buff doc lai
    for (i = 0; i < METER_LOG_MESSAGE_SIZE; i++)
        ReadBackBuff[i] = 0xFF;
    /* Write message */
    if (Flash_S25FL_BufferWrite(Buff, *PosTaget, length) != 1)
        return 0;
    /* Read message to check */
    Flash_S25FL_BufferRead(&ReadBackBuff[0], *PosTaget, length);
    
    for (i = 0; i < length; i++)
        if (ReadBackBuff[i] != *(Buff + i))
            return 0;
    /* Compare Check Sum Byte */
    crc = 0;
    for (i = 0; i < length - 1; i++)
        crc += ReadBackBuff[i];         /* caculator */

    if (crc != ReadBackBuff[length - 1])
        return 0;
    
    /* Caculator next Message */
    *PosTaget = *PosTaget + SIZE_LOG;
    
    if (*PosTaget >= ADD_STOP)
        *PosTaget = ADD_START;
    
    /* Erase Sector if page start of Sector*/
    if (*PosTaget % S25FL_SECTOR_SIZE == 0)
        if (Flash_S25FL_Erase_Sector(*PosTaget) != 1) 
            return 0;
    
    return 1;
}

/*
    - Function Log firmware vao external flash
    - Input: + Buff, length data
             + Type: 0: First data firm ; 1: Datafirst before; 2: crc 
*/

uint8_t Flash_Log_Firm (uint8_t *Buff, uint16_t length, uint8_t type, uint8_t *CrcResult)   
{
    uint8_t     crc = 0;    
	uint16_t    i = 0;
    
    //Reset Buff doc lai
    for (i = 0; i < METER_LOG_MESSAGE_SIZE; i++)
        ReadBackBuff[i] = 0xFF;
    
    if (type < 2)	//Firmware Packet Data
	{
		/* Write message */
		if (Flash_S25FL_BufferWrite(Buff, pos_FirmWrite_u32, length) != 1)
			return 0;
        //doc ra lai de check cac byte ghi vao
        Flash_S25FL_BufferRead(&ReadBackBuff[0], pos_FirmWrite_u32, length);
		
		/* Check readback data*/
		for (i = 0; i < length; i++)
			if (ReadBackBuff[i] != *(Buff + i))
				return 0;
        //Generate checksum byte       
        crc = 0;
        if (type == 0)    //Packet dau tien tinh crc tu byte 32
        {
            for (i = 32; i < length; i++)
                crc += ReadBackBuff[i];  /* caculator */
        } else                          //cac Pack sau tinh crc theo bình thuong
            for (i = 0; i < length; i++)
                crc += ReadBackBuff[i];  /* caculator */
		/* Caculator next Message */
		pos_FirmWrite_u32 = pos_FirmWrite_u32 + LENGTH_FIRM_LOG;          //1024 byte nen k can nhay page cuoi cua sector
        if (deFLASH_S25FL_IS_NEW_SECTOR(pos_FirmWrite_u32) == 0) 		 //***********NOTE**********
		{
            if (Flash_S25FL_Erase_Sector(pos_FirmWrite_u32) != 1) 
              return 0;
        }
        *CrcResult = crc;
	}
	
    if (type == 2)	//CRC Firmware Packet 
	{
        if (Flash_S25FL_Erase_Sector(ADDR_CRC_FIRM) != 1) 
              return 0;
		/* Write message */
		if (Flash_S25FL_BufferWrite(Buff, ADDR_CRC_FIRM, length) != 1)
			return 0;
        //doc ra lai de check cac byte ghi vao
        Flash_S25FL_BufferRead(&ReadBackBuff[0], ADDR_CRC_FIRM, length);
		
		/* Check readback data*/
		for (i=0;i<length;i++)
			if (ReadBackBuff[i] != *(Buff + i))
				return 0;
 	}
    
    return 1;
}




uint8_t ExFlash_Save_Data_To_Sector (uint32_t Add, uint8_t *pData, uint16_t Length)
{
    uint16_t i = 0;
    
    if (Flash_S25FL_Erase_Sector(Add) != 1) 
        return 0;
    
    HAL_Delay (10);  //Thu de test
    
    UTIL_MEM_set (ReadBackBuff, 0xFF, Length );
    /* Write message */
    if (Flash_S25FL_BufferWrite(pData, Add, Length) != 1) 
        return 0;
    /* Read message to check */
    Flash_S25FL_BufferRead(&ReadBackBuff[0], Add, Length);
    
    /* Check readback data*/
    for (i = 0; i < Length; i++)
        if (ReadBackBuff[i] != *(pData + i))
            return 0;
    
    return 1;
}




void ExFlash_Erase (void)
{
    // Xoa Flash, Reset
    Flash_S25FL_Erase_Chip();
    
    __disable_irq();
    NVIC_SystemReset();
}

























