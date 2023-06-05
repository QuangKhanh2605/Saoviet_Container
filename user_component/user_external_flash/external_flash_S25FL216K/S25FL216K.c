
#include "S25FL216K.h"
#include "stdlib.h"
#include "user_time.h"


extern uint32_t         RtCountSystick_u32;
/* =============================================================================================== */
/*--------------------------------------- 
Config driver SPI:
CLK = 24MHz
FullDuplex
*/

/***************************************************
Chip Select
*/
void Flash_S25FL_ChipSelect(uint8_t State)
{
	uint16_t	i = 0;
	/* Set High or low the chip select line on PA.4 pin */
	for (i = 0; i< 1000; i++);   // ??? có nên dùng
	if (State == LOW)
		HAL_GPIO_WritePin(S25FL_SC_PORT, S25FL_SC_PIN, GPIO_PIN_RESET);  
	else
		HAL_GPIO_WritePin(S25FL_SC_PORT, S25FL_SC_PIN, GPIO_PIN_SET);
	// delay
	for(i = 0; i < 1000; i++);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
uint32_t Flash_S25FL_Connect(void)
{	
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    
    /* Select the FLASH: Chip Select low */
    Flash_S25FL_ChipSelect(LOW);	   
    
    /* Send "RDID " instruction */
    Flash_S25FL_Send_Byte(S25FL_JEDEC);
    
    /* Read a byte from the FLASH */			
    Temp0 = Flash_S25FL_Send_Byte(DUMMY_BYTE);
    
    /* Read a byte from the FLASH */			
    Temp1 = Flash_S25FL_Send_Byte(DUMMY_BYTE);
    
    /* Read a byte from the FLASH */			
    Temp2 = Flash_S25FL_Send_Byte(DUMMY_BYTE);
    
    /* Deselect the FLASH: Chip Select high */
    Flash_S25FL_ChipSelect(HIGH);
    
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
	
    return Temp;
}

/*******************************************************************************
* Function Name  : Flash_S25FL_BufferRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read 
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH (not limit)
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Flash_S25FL_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	uint8_t mAdress[3];
	uint16_t mNumByteToRead = NumByteToRead;
	uint8_t Retry = 0;
	
	mAdress[0] = (uint8_t)((ReadAddr & 0xFF0000) >> 16);
	mAdress[1] = (uint8_t)((ReadAddr & 0x00FF00) >> 8);
	mAdress[2] = (uint8_t)(ReadAddr & 0x0000FF);
	
	do
	{
		if(Flash_S25FL_WaitForWriteEnd(TIMEOUT_WR_ENABLE) == 1)
		{
            /* Select the FLASH: Chip Select low */
            Flash_S25FL_ChipSelect(LOW);
            
            /* Send "Read from Memory " instruction */
            Flash_S25FL_Send_Byte(S25FL_READ);	 
            // Send addr
            /* Send ReadAddr high nibble address byte to read from */
            Flash_S25FL_Send_Byte(mAdress[0]);
            /* Send ReadAddr medium nibble address byte to read from */
            Flash_S25FL_Send_Byte(mAdress[1]);
            /* Send ReadAddr low nibble address byte to read from */
            Flash_S25FL_Send_Byte(mAdress[2]);   
			
            while (mNumByteToRead--) /* while there is data to be read */
            {
                /* Read a byte from the FLASH */			
                *pBuffer = Flash_S25FL_Send_Byte(DUMMY_BYTE);
                /* Point to the next location where the byte read will be saved */
                pBuffer++;
            }
            
            /* Deselect the FLASH: Chip Select high */
            Flash_S25FL_ChipSelect(HIGH);
            return 1;
        }
		Retry++;
	} while (Retry<3);
    
	return 0;
}	

/*******************************************************************************
* Function Name  : Flash_S25FL_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void Flash_S25FL_StartReadSequence(uint32_t ReadAddr)
{
    /* Select the FLASH: Chip Select low */
    Flash_S25FL_ChipSelect(LOW);	   
    
    /* Send "Read from Memory " instruction */
    Flash_S25FL_Send_Byte(S25FL_READ);	 
    
    /* Send the 24-bit address of the address to read from -----------------------*/  
    /* Send ReadAddr high nibble address byte */
    Flash_S25FL_Send_Byte((ReadAddr & 0xFF0000) >> 16);
    /* Send ReadAddr medium nibble address byte */
    Flash_S25FL_Send_Byte((ReadAddr& 0xFF00) >> 8);  
    /* Send ReadAddr low nibble address byte */
    Flash_S25FL_Send_Byte(ReadAddr & 0xFF);  
	/* Not Deselect Chip */
}

//
/*******************************************************************************
* Function Name  : Flash_S25FL_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
uint8_t Flash_S25FL_ReadByte(void)
{
    return (Flash_S25FL_Send_Byte(DUMMY_BYTE));
}

/*******************************************************************************
* Function Name  : Flash_S25FL_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be 
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH. ( < Chip Size )
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Flash_S25FL_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	uint8_t Retry = 0;
	uint16_t mNumByteToWrite = NumByteToWrite;
    
	Addr = WriteAddr % S25FL_PAGE_SIZE;     /* Kiem tra truong hop dia chi bat dau ko phai la boi so cua PAGE_SIZE */
	count = S25FL_PAGE_SIZE - Addr;			 /* tinh khoang trong con lai	cua 1 page de ghi du lieu */
	NumOfPage =  NumByteToWrite / S25FL_PAGE_SIZE;
	NumOfSingle = NumByteToWrite % S25FL_PAGE_SIZE;
    
	do
	{
		if (Flash_S25FL_WaitForWriteEnd(TIMEOUT_WR_ENABLE) == 1)
		{
			if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned   - Dia chi la boi so cua PAGE_SIZE */
			{
				if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
				{
                    if (Flash_S25FL_PageWrite(pBuffer, WriteAddr, NumByteToWrite) != 1)
                        return 0;
				} else /* NumByteToWrite > SPI_FLASH_PageSize */ 
				{
					while(NumOfPage--)
					{
						if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, S25FL_PAGE_SIZE) != 1) 
                            return 0;
						WriteAddr +=  S25FL_PAGE_SIZE;
						pBuffer += S25FL_PAGE_SIZE;  
					}
					// Truyen con tro pBuffer va thay doi gia tri cua no
					// Truong hop so byte la boi cua PAGE_SIZE -> loi (pBuffer tro toi NULL)
					if(NumOfSingle != 0)
						if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, NumOfSingle) != 1) 
                            return 0;
				}
			} else /* WriteAddr is not SPI_FLASH_PageSize aligned  - Dia chi ko phai la boi so cua PAGE_SIZE */ 
			{
				if(NumOfPage== 0) /* NumByteToWrite < SPI_FLASH_PageSize */
				{
                    if(NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize  - so khoang trong ko du, phai can page moi */
                    {
                        temp = NumOfSingle - count;  // tinh khoang data can luu o page moi
                        
                        if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, count) != 1) 
                            return 0;
                        WriteAddr += count;
                        pBuffer += count; 
                        
                        if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, temp) != 1) 
                            return 0;
                    } else
                    {
                        if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, NumByteToWrite) != 1) 
                            return 0;
                    }
				} else /* NumByteToWrite > SPI_FLASH_PageSize */
				{
                    mNumByteToWrite -= count;
                    NumOfPage =  mNumByteToWrite / S25FL_PAGE_SIZE;
                    NumOfSingle = mNumByteToWrite % S25FL_PAGE_SIZE;	
                    
                    if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, count) != 1) 
                        return 0;
                    WriteAddr +=  count;
                    pBuffer += count;  
                    
                    while(NumOfPage--)
                    {
                        if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, S25FL_PAGE_SIZE) != 1) 
                            return 0;
                        WriteAddr += S25FL_PAGE_SIZE;
                        pBuffer += S25FL_PAGE_SIZE;  
                    }
                    
                    if(NumOfSingle != 0)
                        if(Flash_S25FL_PageWrite(pBuffer, WriteAddr, NumOfSingle) != 1) 
                            return 0;
				}
			}
			Retry = 3;
			return 1;       // ok
		}
		Retry++;
	} while (Retry<3);
    
	return 0; // Error
}

/*******************************************************************************
* Function Name  : Flash_S25FL_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be 
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value. 
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Flash_S25FL_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    //	uint8_t Retry = 0;
	uint8_t mTempReadback[256];
	
    //	do{
    /* Enable the write access to the FLASH */
    Flash_S25FL_Enable(); // Check enable thanh cong
    if(Flash_S25FL_CheckWriteEnable(TIMEOUT_WR_ENABLE) == 1)
    {		
        /* Select the FLASH: Chip Select low */
        Flash_S25FL_ChipSelect(LOW);	   
        /* Send "Write to Memory " instruction */    
        Flash_S25FL_Send_Byte(S25FL_WRITE);	
        /* Send WriteAddr high nibble address byte to write to */
        Flash_S25FL_Send_Byte((WriteAddr & 0xFF0000) >> 16);
        /* Send WriteAddr medium nibble address byte to write to */
        Flash_S25FL_Send_Byte((WriteAddr & 0xFF00) >> 8);  
        /* Send WriteAddr low nibble address byte to write to */
        Flash_S25FL_Send_Byte(WriteAddr & 0xFF);             
        
        /* while there is data to be written on the FLASH */
        HAL_SPI_TransmitReceive(&hspi1,pBuffer,mTempReadback,NumByteToWrite,10);
        
        //            /* Deselect the FLASH: Chip Select high */
        //            Retry = 3;
    } else
        return 0;
    
    Flash_S25FL_ChipSelect(HIGH);
    //		Retry++;
    //	}while(Retry<3);
    //    
    //    if(Flash_S25FL_CheckWriteEnable(TIMEOUT_WR_ENABLE) != 1)
    //		return 0;
    /* Wait the end of Flash writing */
    if(Flash_S25FL_WaitForWriteEnd(TIMEOUT_WR_PAGE) != 1)
        return 0;
    else
        return 1;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Flash_S25FL_Erase_Sector(uint32_t SectorAddr)
{
	uint8_t Retry = 0;
    /* Send write enable instruction */
	do{	
        Flash_S25FL_Enable();
		if(Flash_S25FL_CheckWriteEnable(TIMEOUT_WR_ENABLE) == 1)
		{
            /* Sector Erase */ 
            /* Select the FLASH: Chip Select low */
            Flash_S25FL_ChipSelect(LOW);		
            /* Send Sector Erase instruction  */
            Flash_S25FL_Send_Byte(S25FL_SECTOR_ERASE);
            /* Send SectorAddr high nibble address byte */
            Flash_S25FL_Send_Byte((SectorAddr & 0xFF0000) >> 16);
            /* Send SectorAddr medium nibble address byte */
            Flash_S25FL_Send_Byte((SectorAddr & 0xFF00) >> 8);
            /* Send SectorAddr low nibble address byte */
            Flash_S25FL_Send_Byte(SectorAddr & 0xFF);
            /* Deselect the FLASH: Chip Select high */
            Flash_S25FL_ChipSelect(HIGH);
            /* Wait the end of Flash writing */
            if(Flash_S25FL_WaitForWriteEnd(TIMEOUT_ER_SECTOR) == 1)
                return 1;
            else
                return 0;
		}
		Retry++;
	}while(Retry < 3);
    
	return 0;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Flash_S25FL_Erase_Chip(void)
{ 	
	uint8_t Retry = 0;
    /* Send write enable instruction */
    do{
		Flash_S25FL_Enable();	  // ktra trang thai cho phep viet truoc khi thuc hien lenh xoa
		if(Flash_S25FL_CheckWriteEnable(TIMEOUT_WR_ENABLE) == 1)
        {
            /* Bulk Erase */ 
            /* Select the FLASH: Chip Select low */
            Flash_S25FL_ChipSelect(LOW);		
            /* Send Bulk Erase instruction  */
            Flash_S25FL_Send_Byte(S25FL_CHIP_ERASE);
            /* Deselect the FLASH: Chip Select high */
            Flash_S25FL_ChipSelect(HIGH);	
            /* Wait the end of Flash writing */
            if (Flash_S25FL_WaitForWriteEnd(TIMEOUT_ER_CHIP) == 1)
                return 1;
            else
                return 0;
        }
        Retry++;
	} while(Retry < 3);
    
	return 0;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte 
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
uint8_t Flash_S25FL_Send_Byte(uint8_t byte)
{
	uint8_t retVal=0;
	
	HAL_SPI_TransmitReceive(&hspi1,&byte,&retVal,1,1000);
	
	return retVal;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/* Chua co check trang thai khi ghi thanh cong */
void Flash_S25FL_Enable(void)
{
    /* Select the FLASH: Chip Select low */
    Flash_S25FL_ChipSelect(LOW);	
    
    /* Send "Write Enable" instruction */
    Flash_S25FL_Send_Byte(S25FL_WREN);
    
    /* Deselect the FLASH: Chip Select high */
    Flash_S25FL_ChipSelect(HIGH);	
}

/*******************************************************************************
* Function Name  : Flash_S25FL_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the  
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t Flash_S25FL_WaitForWriteEnd(uint32_t timeout) 
{
    uint8_t   FLASH_Status = 0;
    uint32_t	Start_check = 0;
    
    Start_check = RtCountSystick_u32;
    
    /* Select the FLASH: Chip Select low */
    Flash_S25FL_ChipSelect(LOW);
  	
    /* Send "Read Status Register" instruction */
    Flash_S25FL_Send_Byte(S25FL_READ_STATUS);
    
    /* Loop as long as the memory is busy with a write cycle */ 		
    do
    { 
        /* Send a dummy byte to generate the clock needed by the FLASH 
        and put the value of the status register in FLASH_Status variable */
        FLASH_Status = Flash_S25FL_Send_Byte(DUMMY_BYTE); 
		
        if (Check_Time_Out(Start_check,timeout) == 1)
        {
            Flash_S25FL_ChipSelect(HIGH);
            return 0;
        }
        if ((FLASH_Status & WIP_Flag) != 0)                    //Bit so 0: WIP   ; Bit so 1: WEL:  1: cho phep program,,,, 0 K cho phep
            HAL_Delay(1);
    } while ((FLASH_Status & WIP_Flag) != 0);     /* Write in progress */  
    
    /* Deselect the FLASH: Chip Select high */
    Flash_S25FL_ChipSelect(HIGH);
    
    return 1;
}
/******************************************

******************************************/

uint8_t Flash_S25FL_CheckWriteEnable(uint32_t timeout)
{
    uint8_t     FLASH_Status = 0;
	uint32_t	Start_check = 0;
	
	Start_check = RtCountSystick_u32;
    
    /* Select the FLASH: Chip Select low */
    Flash_S25FL_ChipSelect(LOW);
  	
    /* Send "Read Status Register" instruction */
    Flash_S25FL_Send_Byte(S25FL_READ_STATUS);
    
    /* Loop as long as the memory is busy with a write cycle */ 		
    do
    { 
        /* Send a dummy byte to generate the clock needed by the FLASH 
        and put the value of the status register in FLASH_Status variable */
        FLASH_Status = Flash_S25FL_Send_Byte(DUMMY_BYTE); 
		
        if (Check_Time_Out(Start_check,timeout) == 1)
        {
            Flash_S25FL_ChipSelect(HIGH);
            return 0;
        }
        if((FLASH_Status & WEL_Flag) != WEL_Flag)   
            HAL_Delay(1);
    }while((FLASH_Status & WEL_Flag) != WEL_Flag); /* Write in progress */     //Neu 1: cho phep write    0: k cho phep
    /* Deselect the FLASH: Chip Select high */
    Flash_S25FL_ChipSelect(HIGH);
    
    return 1;
}


/*
- SoftReset hay HardReset (trong OTP mode) ddeeeu khong clear RS mà chi dua ve trang thai ready (xoa WEL)
*/
uint8_t Flash_S25FL_SoftReset(void)
{
    uint8_t Retry = 0;
    /* Send write enable instruction */
    do{
		Flash_S25FL_Enable();	  // ktra trang thai cho phep viet truoc khi thuc hien lenh xoa
		if(Flash_S25FL_CheckWriteEnable(TIMEOUT_WR_ENABLE) == 1)
        {
            /* Select the FLASH: Chip Select low */
            Flash_S25FL_ChipSelect(LOW);	
            
            /* Send "Write Enable" instruction */
            Flash_S25FL_Send_Byte(S25FL_RSTEN);  
            
            /* Deselect the FLASH: Chip Select high */
            Flash_S25FL_ChipSelect(HIGH);	
            
            /* Select the FLASH: Chip Select low */
            Flash_S25FL_ChipSelect(LOW);	
            
            /* Send "Write Enable" instruction */
            Flash_S25FL_Send_Byte(S25FL_RST);
            
            /* Deselect the FLASH: Chip Select high */
            Flash_S25FL_ChipSelect(HIGH);
            
            if (Flash_S25FL_WaitForWriteEnd(TIMEOUT_WR_ENABLE) == 1)
                return 1;
            else
                return 0;
        }
        Retry++;
        HAL_Delay(20);
	}while(Retry<3);
    
	return 0;
}


uint8_t Flash_S25FL_Read_StatusRegis (void)
{
    uint8_t     FLASH_Status = 0;
    /* Select the FLASH: Chip Select low */
    Flash_S25FL_ChipSelect(LOW);
    
    /* Send "Read Status Register" instruction */
    Flash_S25FL_Send_Byte(S25FL_READ_STATUS);
    
    FLASH_Status = Flash_S25FL_Send_Byte(DUMMY_BYTE); 
    
    Flash_S25FL_ChipSelect(HIGH);
    
    return FLASH_Status;
}


void S25FL_Write_Disable(void)
{
    Flash_S25FL_ChipSelect(LOW);		
    /* Send Bulk Erase instruction  */
    Flash_S25FL_Send_Byte(S25FL_WRITE_DISABLE);
    /* Deselect the FLASH: Chip Select high */
    Flash_S25FL_ChipSelect(HIGH);	
}




/******************* THE END ********************* */
