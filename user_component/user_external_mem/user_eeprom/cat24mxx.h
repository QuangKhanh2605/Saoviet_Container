

#ifndef CAT24MXX_H
#define CAT24MXX_H _CAT24MXX_H

#include "user_util.h"

#define CAT24_START_LOG             0
#define CAT24_END_LOG               16384 /* Cat24 128kbs */

#define I2C_CAT24                   hi2c1

#define NUMBER_RETRY_BUSY           20
#define TIME_RETRY_BUSY             10
#define NUMBER_RETRY_R_W            20

/* ========================================= Drivers ===================================== */
#define I2C_CAT24Mxx_ADDR_7BIT          0xA0        /* Don't care operation bit (R/W bit) - 1010A1A2A30b */
#define I2C_CAT24Mxx_SIZE               0x20000     /* Bytes */
#define I2C_CAT24Mxx_MAX_BUFF           64          /* Bytes */

/* ===================================== Struct =========================================== */
typedef struct
{
    uint8_t status_u8;
    uint8_t Count_err_u8;
    uint8_t Count_retry_u8;
}sFROM_Manager_Struct;


extern sFROM_Manager_Struct	sFROM_Manager;



/* ===================================== Function =========================================== */
uint8_t CAT24Mxx_Write_Array (uint16_t writeAddr, uint8_t* dataBuff, uint16_t dataLength);
uint8_t CAT24Mxx_Read_Array (uint16_t readAddr, uint8_t* dataBuff, uint16_t dataLength);
int32_t TestEEPROM(void); /* Test EEPROM */
uint8_t CAT24Mxx_Erase (void);
uint8_t CAT24Mxx_Write_Byte (uint16_t writeAddr, uint8_t *dataBuff);
uint8_t CAT24Mxx_Write_Word (uint16_t writeAddr, uint16_t *dataBuff);
uint8_t CAT24Mxx_Write_DoubleWord (uint16_t writeAddr, uint32_t *dataBuff);
uint8_t CAT24Mxx_Write_QuadWord (uint16_t writeAddr, uint64_t *dataBuff);
uint8_t CAT24Mxx_Read_Byte (uint16_t writeAddr, uint8_t *dataBuff);
uint8_t CAT24Mxx_Read_Word (uint16_t writeAddr, uint16_t *dataBuff);
uint8_t CAT24Mxx_Read_DoubleWord (uint16_t writeAddr, uint32_t *dataBuff);
uint8_t CAT24Mxx_Read_QuadWord (uint16_t writeAddr, uint64_t *dataBuff);
uint8_t CAT24Mxx_Check_Busy(uint8_t  addrA16bit, uint8_t Retry, uint8_t Timeout);

#endif
