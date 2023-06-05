
/*
    - Thiet bi doc level chat long
        + Day ket noi:
            ++ Red:    Vcc 12-36V
            ++ Green:  Gnd
            ++ Yellow: RS485B
            ++ Blue: RS485A
*/      

#include "liquid_level_S_YW_01B.h"
#include "user_modbus_rtu.h"



/*========== Var struct =============*/


/*======================== Funcion ======================*/


/*
    FuncTest: Master Read
*/

void MeaLevel_Read_Value (uint8_t SlaveID, void (*pFuncResetRecvData) (void)) 
{
    uint8_t aFrame[48] = {0};
    sData   strFrame = {(uint8_t *) &aFrame[0], 0};
    
    ModRTU_Master_Read_Frame(&strFrame, SlaveID, FUN_READ_BYTE, REGIS_SLAVE_ID, NUM_REGISTER_READ);

    HAL_GPIO_WritePin(DE_GPIO_PORT, DE_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
    // Send
    pFuncResetRecvData();
    
    HAL_UART_Transmit(&UART_485, strFrame.Data_a8, strFrame.Length_u16, 1000); 
    //Dua DE ve Receive
    HAL_GPIO_WritePin(DE_GPIO_PORT, DE_GPIO_PIN, GPIO_PIN_RESET);
}

uint8_t MeaLevel_Extract_Data (uint8_t SlaveID, structFloatValue *pFloatValue, uint8_t *pSource, uint16_t Length)
{
    uint16_t crc_calcu = 0;
    uint8_t Crc_Cut[2] = {0};
    uint16_t Pos = 0;
    uint8_t FuncCode = 0;
    uint8_t LengthData = 0;
    uint8_t SlaveIDGet = 0;
    uint16_t TempValue = 0;
    
    if (Length < 4)
        return FALSE;
    
    crc_calcu = ModRTU_CRC(pSource, Length - 2);
    
    Crc_Cut[0] = (uint8_t) (crc_calcu & 0x00FF);
    Crc_Cut[1] = (uint8_t) ( (crc_calcu >> 8) & 0x00FF );
    
    if ( (Crc_Cut[0] != *(pSource + Length - 2)) || (Crc_Cut[1] != *(pSource + Length - 1)) )
        return FALSE;
    
    SlaveIDGet = *(pSource + Pos++);
    FuncCode = *(pSource + Pos++);
    
    LengthData = *(pSource + Pos++);
    //check frame
    if ( (SlaveIDGet != SlaveID) || (FuncCode != FUN_READ_BYTE) || (LengthData != (NUM_REGISTER_READ * 2)))
        return FALSE;
       
    //Get data begin Unit: skip 2byte slaveID, 2byte baurate
    UTIL_MEM_set (pFloatValue, 0x00, sizeof(structFloatValue));
                  
    pFloatValue->Unit_u16 = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    Pos += 2;
    pFloatValue->Decimal_u16 = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    pFloatValue->Value_i16 = (int16_t) TempValue;
    Pos += 2;
    TempValue = *(pSource + 4 + Pos) * 256 + *(pSource + 4 + Pos + 1);
    pFloatValue->ZeroPoint_i16 = (int16_t ) TempValue;
    Pos += 2;
    
    return TRUE;
}

