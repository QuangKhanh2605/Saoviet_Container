/*
    8/2021
    Thu vien Quan ly onchip Flash
*/

#include "onchipflash.h"


/*===================Struct, Var=========================*/

/*===================Function=========================*/
/*
 *	So luong bien truyen vao Phai la Boi cua 8
 */
HAL_StatusTypeDef	OnchipFlashWriteData (uint32_t andress, uint8_t	*data_address, uint32_t data_length)
{
	uint8_t		*temp_data_address;
	uint16_t	i=0;
	uint64_t	writeval=0,writetime=0;
    uint32_t    WriteData = 0;
	uint32_t	temp_write_address;
	HAL_StatusTypeDef   status = HAL_ERROR;

	temp_data_address = data_address;
	temp_write_address = andress;

	if ((data_length % FLASH_BYTE_WRTIE) == 0)
	{
		//Calculate number of word to write
		writetime = data_length / FLASH_BYTE_WRTIE;
		//Unlock flash
		HAL_FLASH_Unlock();
		//Wait for last operation to be completed
        FLASH_WaitForLastOperation(1000);
		//Change bytes order then write
		for (i = 0; i < writetime; i++)
		{
			#if (FLASH_BYTE_WRTIE == 8)
                writeval = *(__IO uint64_t*)(temp_data_address);

                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, temp_write_address, writeval);
            #else
                WriteData = *(__IO uint32_t*)(temp_data_address);

                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, temp_write_address, WriteData);
            #endif

			if (status == HAL_ERROR)
				break;

			temp_write_address = temp_write_address + FLASH_BYTE_WRTIE;
			temp_data_address = temp_data_address + FLASH_BYTE_WRTIE;
		}

		//Lock flash
		HAL_FLASH_Lock();
	}

	return status;
}

HAL_StatusTypeDef	OnchipFlashCopy (uint32_t sourceandress, uint32_t destinationandress, uint32_t data_length)
{
	uint32_t	i=0,writetime=0;
	uint64_t	writeval=0;
    uint32_t    WriteData = 0;
	uint32_t	temp_sourceandress,temp_destinationandress;
	HAL_StatusTypeDef status = HAL_ERROR;

	temp_sourceandress = sourceandress;
	temp_destinationandress = destinationandress;

	if ((data_length % FLASH_BYTE_WRTIE) == 0)
	{
		//Calculate number of word to write
		writetime = data_length / FLASH_BYTE_WRTIE;
		//Unlock flash
		HAL_FLASH_Unlock();
		//Wait for last operation to be completed
        FLASH_WaitForLastOperation(1000);
		//Change bytes order then write
		for (i = 0; i < writetime; i++)
		{
			#if (FLASH_BYTE_WRTIE == 8)
                writeval = *(__IO uint64_t*)(temp_sourceandress);

                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, temp_destinationandress, writeval);
            #else
                WriteData = *(__IO uint32_t*)(temp_sourceandress);

                status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, temp_destinationandress, WriteData);
            #endif

			if (status == HAL_ERROR)
				break;

			temp_sourceandress = temp_sourceandress + FLASH_BYTE_WRTIE;
			temp_destinationandress = temp_destinationandress + FLASH_BYTE_WRTIE;
		}

		//Lock flash
		HAL_FLASH_Lock();
	}

	return status;
}




HAL_StatusTypeDef OnchipFlashPageErase (uint32_t pageaddress)
{
	HAL_StatusTypeDef status = HAL_ERROR;

    //Unlock flash
    status = HAL_FLASH_Unlock();
    //Wait for last operation to be completed
    FLASH_WaitForLastOperation(1000);

    #if (FLASH_BYTE_WRTIE == 8)
        //If the previous operation is completed, proceed to erase the page
        if (status == HAL_OK)
            FLASH_PageErase((pageaddress & 0x00FFFFFF)/2048, FLASH_BANK_1);

        //Wait for last operation to be completed
        status = FLASH_WaitForLastOperation(1000);

        //Disable the PER Bit
        CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
    #else
        //If the previous operation is completed, proceed to erase the page
        if (status == HAL_OK)
            FLASH_PageErase(pageaddress);

        //Wait for last operation to be completed
        status = FLASH_WaitForLastOperation(1000);

        //Disable the PER Bit
        CLEAR_BIT(FLASH->PECR, FLASH_PECR_ERASE);
        CLEAR_BIT(FLASH->PECR, FLASH_PECR_PROG);
    #endif

    HAL_FLASH_Lock();

	return status;
}

void Erase_Firmware(uint32_t Add_Flash_update, uint32_t Total_page)
{
	uint32_t var;

	for (var = 0; var < Total_page; var++)
		OnchipFlashPageErase(Add_Flash_update + var * FLASH_PAGE_SIZE);
}

void OnchipFlashReadData (uint32_t address, uint8_t *destination, uint16_t length_inByte)
{
	uint16_t    i=0;
	uint32_t    temp_address;
	uint8_t     *temp_destination;

	temp_address = address;
	temp_destination = destination;

	for (i = 0; i < length_inByte; i++)
	{
		*temp_destination = *(__IO uint8_t*)temp_address;
		temp_destination++;
		temp_address++;
	}
}


/*=========================Function xu ly onchip flash DCU===========*/
/*
    Function luu Serial Modem:
    Input: Add, string serial.
*/

uint8_t Save_Array(uint32_t ADD, uint8_t* Buff, uint16_t length)
{
    uint16_t i = 0;
    uint8_t aTemp[256] = {0};

    OnchipFlashPageErase(ADD);

    aTemp[0] = BYTE_TEMP_FIRST;
    aTemp[1] = length;
    for(i = 0; i < length; i++)
        aTemp[i+2] = *(Buff + i);

    length += 2;
    length = length + ( FLASH_BYTE_WRTIE - (length % FLASH_BYTE_WRTIE) );

    if (OnchipFlashWriteData(ADD, &aTemp[0], length) != HAL_OK)
        return 0;

    return 1;
}

uint8_t Save_Array_without_erase (uint32_t ADD, uint8_t* Buff, uint16_t length)
{
    uint16_t i = 0;
    uint8_t aTemp[256] = {0};

    aTemp[0] = BYTE_TEMP_FIRST;
    aTemp[1] = length;
    for(i = 0; i < length; i++)
        aTemp[i+2] = *(Buff + i);

    length += 2;
    length = length + ( FLASH_BYTE_WRTIE - (length % FLASH_BYTE_WRTIE) );

    if (OnchipFlashWriteData(ADD, &aTemp[0], length) != HAL_OK)
        return 0;

    return 1;
}


uint8_t OnchipFlash_Write_Buff (uint32_t Addr, uint8_t *pData, uint16_t Length)
{
    Length = Length + FLASH_BYTE_WRTIE - Length % FLASH_BYTE_WRTIE;
    
    if (OnchipFlashWriteData(Addr, pData, Length) != HAL_OK)
        return 0;

    return 1;
}
