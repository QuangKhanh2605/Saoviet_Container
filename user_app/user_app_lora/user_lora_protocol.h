
#ifndef USER_LORA_PROTOCOL_H
#define USER_LORA_PROTOCOL_H

#include "user_util.h"



/*=========== Struct var ===============*/

typedef enum
{
    __DEVICE_GW = 0,
    __DEVICE_NODE,
}eKindDevice;

typedef union uLoRaHeader
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sHeaderBits
    {
        /* Message Type */
        uint8_t Mtype_u8        : 3;
        /* Data Device Type */
        uint8_t DataDevType     : 3;
        /* not used*/
        uint8_t NotUsed_u8      : 2;      
    }Bits;
}LoRaHeader_t;


typedef union uLoRaFCtrl
{
    uint8_t Value;
    
    struct sFCtrlBits
    {
        /* Ack bit */
        uint8_t Ack_u8          : 1;
        /* ADR Request: Data rate */
        uint8_t rAdr_u8         : 1;
        /* ATR Request: Tx power */
        uint8_t rAtr_u8         : 1;
        /* Request sTime*/
        uint8_t rStime_u8       : 1;
        /* Increase Tx Power*/
        uint8_t rIncTxPower_u8  : 1;
        /* Decrease Tx Power*/
        uint8_t rDecTxPower_u8  : 1;
        /* Not Used */
        uint8_t NotUsed_u8      : 2;
    }Bits;
} LoraFCtrl_t;


typedef struct sLoRaFrame
{
    /* Header frame */
    LoRaHeader_t Header;
    /* Frame control field */
    LoraFCtrl_t FCtrl;
    /* Device address */
    uint32_t DevAddr;
    /* Frame counter */
    uint16_t FCnt;
}LoRaFrame_t;



typedef enum eLoRaMessType
{
    /* LoRaMAC join request frame */
    FRAME_TYPE_JOIN_REQ              = 0x00,
    /* LoRaMAC join accept frame */
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    /* LoRaMAC unconfirmed up-link frame  */
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    /* LoRaMAC unconfirmed down-link frame */
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    /* LoRaMAC confirmed up-link frame */
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    /* LoRaMAC confirmed down-link frame */
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    /* LoRaMAC proprietary frame */
    FRAME_TYPE_PROPRIETARY           = 0x07,
}LoRaMessType_t;



typedef struct
{
    uint8_t CtrlACK_u8;
    uint8_t CtrlAdr_u8;
    uint8_t CtrlAtr_u8;
    uint8_t CtrlStime_u8;
    
    uint8_t CtrlResp_u8;
    uint16_t MessCount_u16;
    
}StructControlLora;

extern StructControlLora        sCtrlLora;



/*========== Func ==========================*/
uint8_t Lora_Pro_Check_Rx (eKindDevice Type, uint8_t *pData, uint8_t Length, uint8_t NoProcess, LoRaFrame_t *sFrameRx);
uint8_t Lora_Pro_Extract_Rx (eKindDevice Type, uint8_t DataType, uint8_t *pData, uint8_t Length);

uint8_t Lora_Pro_Master_Packet_Payload (uint8_t *pData, uint8_t *length);
void    Lora_Pro_Packet_sTime (uint8_t *pData, uint8_t *length);

uint8_t Lora_Pro_Packet_Header (LoRaFrame_t *pFrame, uint8_t MessType, uint8_t DataType);

void    Lora_Pro_Save_Record_TSVH (uint8_t *pdata, uint8_t Length);
void    Lora_Pro_Save_Record_EVENT (uint8_t *pdata, uint8_t Length);

#endif


