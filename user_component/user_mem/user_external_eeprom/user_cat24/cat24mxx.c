

#include "cat24mxx.h"
#include "i2c.h"


/*
    - Neu ghi vao cuoi cua Page. Data tiep tuc ghi vao dau cua page do?
    - Khi doc thi se tu dong tang len vao k bi gioi han bo end page (ma la end mem)
*/

sFROM_Manager_Struct    sFROM_Manager;
// Chua co check trang thai cho EEPROM moi khi doc ghi

/* return 1 is OK or 0 ERROR */

uint8_t CAT24Mxx_Write_Array (uint16_t writeAddr, uint8_t *dataBuff, uint16_t dataLength)
{
    uint8_t reVal       = 0U;
    uint8_t addrA16bit  = 0U;
    uint8_t catBuff[I2C_CAT24Mxx_MAX_BUFF];
    uint16_t index;

    sFROM_Manager.Count_retry_u8 = NUMBER_RETRY_R_W;
    /* check input parameters */
    if ((NULL == dataBuff) || (0U == dataLength))
    {
        sFROM_Manager.status_u8 = 1U;
        return 0U;
    }
    /* khi so luong lon, dung IC nho tiep theo */
    if (((uint32_t) writeAddr & 0x00010000) == 0x00010000)
    {
        addrA16bit = I2C_CAT24Mxx_ADDR_7BIT | 0x02;
        writeAddr &= 0x0000FFFF;
    } else
    {
        addrA16bit = I2C_CAT24Mxx_ADDR_7BIT;
    }
    //
    do
    {
        if (HAL_I2C_Mem_Write(&I2C_CAT24, addrA16bit, writeAddr, I2C_MEMADD_SIZE_16BIT, dataBuff, dataLength, 1000) == HAL_OK)
        {
            HAL_Delay(2);
            if (HAL_I2C_Mem_Read(&I2C_CAT24, addrA16bit, writeAddr, I2C_MEMADD_SIZE_16BIT, catBuff, dataLength, 1000) == HAL_OK)
            {
                for (index = 0; index < dataLength; index++)
                {
                    if (catBuff[index] != *(dataBuff + index))
                    {
                        sFROM_Manager.Count_retry_u8--;
                        reVal = 0U;
                        break;
                    }
                    else
                    {
                        if (1U != reVal)
                        {
                            reVal = 1U;
                        }
                    }
                }

                if (0U != reVal)
                {
                    sFROM_Manager.Count_retry_u8 = 0;
                }
            }
            else
            {
                sFROM_Manager.Count_retry_u8--;
            }
        }
        else
        {
            sFROM_Manager.Count_retry_u8--;
        }
    } while (sFROM_Manager.Count_retry_u8 > 0);

    if (0U == reVal)
    {
        sFROM_Manager.status_u8 = 1U;
    }
    else
    {
        sFROM_Manager.status_u8 = 0;
        sFROM_Manager.Count_err_u8 = 0;
    }
    return reVal;
}



/* return 1 is SUCCESS or 0 is error */
uint8_t CAT24Mxx_Read_Array(uint16_t readAddr, uint8_t *dataBuff, uint16_t dataLength)
{
    uint8_t reVal       = 0U;
    uint8_t addrA16bit  = 0U;

    sFROM_Manager.Count_retry_u8 = NUMBER_RETRY_R_W;

    if (((uint32_t) readAddr & 0x00010000) == 0x00010000)
    {
        addrA16bit = I2C_CAT24Mxx_ADDR_7BIT|0x02;
        readAddr &= 0x0000FFFF;
    }
    else
    {
        addrA16bit = I2C_CAT24Mxx_ADDR_7BIT;
    }

    do
    {
        if (HAL_I2C_Mem_Read(&I2C_CAT24, addrA16bit, readAddr, I2C_MEMADD_SIZE_16BIT, dataBuff, dataLength, 1000) == HAL_OK)
        {
            sFROM_Manager.status_u8 = 0U;
            sFROM_Manager.Count_retry_u8 = 0U;
            sFROM_Manager.Count_err_u8 = 0U;
            reVal = 1U;
        }
        else
        {
            sFROM_Manager.Count_retry_u8--;
            sFROM_Manager.status_u8 = 1U; /* ERROR notif */
        }
//        /* Wait */    /*20/10*/
//        osDelay(10);
    } while (sFROM_Manager.Count_retry_u8 > 0);

    return reVal;
}

/*
return: 1 - OK, 0 - Error
*/

uint16_t  addr_start = 800;
int32_t TestEEPROM(void)
{
  uint8_t Data[64] = {0};
  uint16_t i = 0;
  uint16_t  addr_start = 0;

  for(i=0;i<64;i++)
	Data[i] = 0xA5;
  
  for(i=CAT24_START_LOG;i< CAT24_END_LOG/64; i++)
  {
	 HAL_Delay(2);
     if (CAT24Mxx_Write_Array(addr_start + i * 64, &Data[0], 64) != 1)
		return i + 1;
  }
  
  HAL_Delay(2000);
  /* Clean all */
  for (i = 0; i < 64; i++)
      Data[i] = 0xFF;
  for (i = CAT24_START_LOG; i < CAT24_END_LOG / 64; i++)
  {
      HAL_Delay(2);
      if (CAT24Mxx_Write_Array(addr_start + i * 64, &Data[0], 64) != 1)
          return i+1;
  }
  if (sFROM_Manager.status_u8 == 1)
    return -1; // loi

  // OK
    return 0;
}


/* */
uint8_t CAT24Mxx_Erase(void)
{
    uint8_t Data[64] = {0};
    uint16_t i = 0;
  
    /* Clean all */
    for (i = 0; i < 64; i++)
        Data[i] = 0xFF;
    
    for (i = CAT24_START_LOG; i < CAT24_END_LOG / 64; i++)
    {
        HAL_Delay(2);
        if (CAT24Mxx_Write_Array(i * 64, &Data[0], 64) != 1)
            return i+1;
    }
    
    return 0;
}
/* This function use to write a byte */
uint8_t CAT24Mxx_Write_Byte(uint16_t writeAddr, uint8_t *dataBuff)
{
    return CAT24Mxx_Write_Array (writeAddr,dataBuff,1);
}
/* This function uses to write a word */
uint8_t CAT24Mxx_Write_Word(uint16_t writeAddr, uint16_t *dataBuff)
{
    return CAT24Mxx_Write_Array (writeAddr,(uint8_t *)dataBuff,2);
}
/* This function uses to write 4 word */
uint8_t CAT24Mxx_Write_DoubleWord(uint16_t writeAddr, uint32_t *dataBuff)
{
  return CAT24Mxx_Write_Array (writeAddr,(uint8_t *)dataBuff,4);
}
/* This function uses to write 8 word */
uint8_t CAT24Mxx_Write_QuadWord(uint16_t writeAddr, uint64_t *dataBuff)
{
    return CAT24Mxx_Write_Array (writeAddr,(uint8_t *)dataBuff,8);
}
/* Read 1 byte from the EEROM memory */
uint8_t CAT24Mxx_Read_Byte(uint16_t writeAddr, uint8_t *dataBuff)
{
    return CAT24Mxx_Read_Array (writeAddr,dataBuff,1);
}
/* Read 2 bytes from the EEROM memory */
uint8_t CAT24Mxx_Read_Word(uint16_t writeAddr, uint16_t *dataBuff)
{
    return CAT24Mxx_Read_Array (writeAddr,(uint8_t *)dataBuff,2);
}
/* Read 4 bytes from the EEROM memory */
uint8_t CAT24Mxx_Read_DoubleWord(uint16_t writeAddr, uint32_t *dataBuff)
{
  return CAT24Mxx_Read_Array (writeAddr,(uint8_t *)dataBuff,4);
}
/* Read 8 bytes from the EEROM memory */
uint8_t CAT24Mxx_Read_QuadWord(uint16_t writeAddr, uint64_t *dataBuff)
{
  return CAT24Mxx_Read_Array (writeAddr,(uint8_t *)dataBuff,8);
}
/* This function uses to check the CAT24 busy or idle */
uint8_t CAT24Mxx_Check_Busy(uint8_t  addrA16bit, uint8_t Retry, uint8_t Timeout)
{
    if (HAL_I2C_IsDeviceReady(&I2C_CAT24, addrA16bit, Retry, Timeout) == HAL_OK)
    {
        return 1;
    }
    else
    {
        sFROM_Manager.Count_err_u8++;/* save error memory */
        return 0;
    }
}


