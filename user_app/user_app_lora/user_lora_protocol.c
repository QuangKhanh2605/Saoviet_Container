

#include "user_lora_protocol.h"
#include "user_message.h"

#include "user_define.h"

#include "stm32_mem.h"
#include "user_time.h"

#include "user_app_lora.h"
#include "sys_app.h"

/*================ Var struct ==================*/

StructControlLora        sCtrlLora;

/*================ Func ==================*/

uint8_t Lora_Pro_Check_Rx (eKindDevice Type, uint8_t *pData, uint8_t Length, uint8_t NoProcess, LoRaFrame_t *sFrameRx)
{
    uint8_t Result = FALSE;
    uint32_t DevAddrRx = 0;
    uint16_t FcntRx = 0;
    uint8_t i = 0;
    sData  Atcmd;
    //Frame Header   
    sFrameRx->Header.Value = *(pData);
    sFrameRx->FCtrl.Value = *(pData + 1);
    
    DevAddrRx = DevAddrRx << 8 | *(pData + 2);
    DevAddrRx = DevAddrRx << 8 | *(pData + 3);
    DevAddrRx = DevAddrRx << 8 | *(pData + 4);
    DevAddrRx = DevAddrRx << 8 | *(pData + 5);
    
    sFrameRx->DevAddr = DevAddrRx;
    
    FcntRx = FcntRx << 8 | *(pData + 6);
    FcntRx = FcntRx << 8 | *(pData + 7);
    
    sFrameRx->FCnt = FcntRx;
    
    //Decode Rx data
    switch (Type)
    {
        case __DEVICE_NODE:
            //Check DevAddr
            for (i = 0; i < 4; i++)
            {
                if ( *(pData + 2 + i) != *(sModem.sDCU_id.Data_a8 + i))
                {
                    APP_LOG(TS_OFF, VLEVEL_L, "user_lora_protocol: devaddr error\r\n");
                    return 0;
                }
            }
            //Set ACK if confirm Mess recv
            if (sFrameRx->Header.Bits.Mtype_u8 == FRAME_TYPE_DATA_CONFIRMED_DOWN )
                sCtrlLora.CtrlACK_u8 = TRUE;
            //
            switch (sFrameRx->Header.Bits.DataDevType)
            {
                case _DATA_CONFIG:   
                    //Get config: tu pos 8
                    if (Length > 8)
                        Lora_Pro_Extract_Rx(Type, _DATA_CONFIG, pData + 8, Length - 8);
                    break;
                case _DATA_AT_CMD:
                    //Get at cmd
                    if (Length > 8)
                    {
                        APP_LOG( TS_OFF, VLEVEL_L, "user_protocol: at cmd request: %s", (pData + 8) );
                        Atcmd.Data_a8 = pData + 8;
                        Atcmd.Length_u16 = Length - 9;
                    }
                    Check_AT_User(&Atcmd, _AT_REQUEST_LORA);
                    
                    fevent_active(sEventAppLora, _EVENT_LORA_TX); 
                    break;
                default:
                    break;
            }
            
            Result = TRUE;
            
            break;
        case __DEVICE_GW:    
            //Check DevAdr 'S' va Fcnt OK?
            if (*(pData + 2) != 'S')
            {
                APP_LOG(TS_OFF, VLEVEL_L, "user_lora_protocol: devaddr error\r\n");
                return 0;
            }
            //Set ACK if confirm Mess recv
            if (sFrameRx->Header.Bits.Mtype_u8 == FRAME_TYPE_DATA_CONFIRMED_UP)
            {
                sCtrlLora.CtrlACK_u8 = TRUE;
                Result = TRUE;
            }
            
            if (sFrameRx->FCtrl.Bits.rStime_u8 == TRUE )
            {
                //Co yeu cau set sTime
                sCtrlLora.CtrlStime_u8 = TRUE;
                Result = TRUE;
            }
            //Neu có Payload data wm: Test Printf ra uart
            if ( (sFrameRx->Header.Bits.DataDevType == _DATA_TSVH)
                || (sFrameRx->Header.Bits.DataDevType == _DATA_INTAN)
                || (sFrameRx->Header.Bits.DataDevType == _DATA_EVENT)
                || (sFrameRx->Header.Bits.DataDevType == _DATA_RESPONSE) )  
            {
                if (Length > 8)
                {
                    Lora_Pro_Extract_Rx (Type, sFrameRx->Header.Bits.DataDevType, pData + 8, Length - 8);
                }
            } 
            //Co request tu server xuong: true -> Active event TX now.
            if (shLora.rAtcmd_u8 == TRUE)
            {
                Result = TRUE; 
            }
            break;
        default:
            break;
    }
    
    return Result;
}


                                      
/*
    Func: Extract Data Master
        + TSVH, Event: Save to memory
        + RESPONSE: Send Resp to Server.
*/
      


uint8_t Lora_Pro_Extract_Rx (eKindDevice Type, uint8_t DataType, uint8_t *pData, uint8_t Length)
{
    uint8_t Obis = 0;
    uint8_t Pos = 0;
    uint8_t lendata = 0;
    uint16_t i = 0;
    
    APP_LOG(TS_OFF, VLEVEL_L, "user_lora_protocol: rx payload: ");
    
    switch (Type)
    {
        case __DEVICE_NODE:
            for ( i = 0; i < Length; i++)    
                APP_LOG(TS_OFF, VLEVEL_L, "%02X", pData[i]);
            
            while (Pos < (Length - 1))  //bo crc
            {
                Obis = *(pData + Pos++);
                
                switch (Obis)
                {
                    case OBIS_TIME_DEVICE:
                        //Config sTime
                        lendata = *(pData + Pos++);
                        
                        if ( (Pos + lendata) <= Length )
                        {
                            sRTCSet.year    = *(pData + Pos++);
                            sRTCSet.month   = *(pData + Pos++);
                            sRTCSet.date    = *(pData + Pos++);
                            
                            sRTCSet.hour    = *(pData + Pos++);
                            sRTCSet.min     = *(pData + Pos++);
                            sRTCSet.sec     = *(pData + Pos++);
                            
                            fevent_enable(sEventAppMain, _EVENT_SET_RTC);
                        }
                        break;
                    case OBIS_FREQ_SEND:
                        
                        break;
                    default:
                        break;
                }
            }
            break;
            
        case __DEVICE_GW:
            switch (DataType)
            {
                case _DATA_TSVH:  
                    //bo crc mess va byte index length truoc khi luu
                    for ( i = 0; i < Length; i++)     
                        APP_LOG(TS_OFF, VLEVEL_L, "%02X", pData[i]);
                    
                    Lora_Pro_Save_Record_TSVH(pData, Length);
                    
                #ifdef DEVICE_TYPE_GATEWAY
                    sMQTT.aMARK_MESS_PENDING[DATA_TSVH_OPERA] = TRUE;
                #endif
                    
                    break;
                case _DATA_INTAN:
                    for ( i = 0; i < Length; i++)     
                        APP_LOG(TS_OFF, VLEVEL_L, "%02X", pData[i]);
                    
                    Reset_Buff(&sLoraVar.sIntanData);
                    for ( i = 0; i < Length; i++) 
                        *(sLoraVar.sIntanData.Data_a8 + sLoraVar.sIntanData.Length_u16++) = *(pData + i);
                    
                    #ifdef DEVICE_TYPE_GATEWAY
                        sMQTT.aMARK_MESS_PENDING[DATA_INTAN_TSVH] = TRUE;
                    #endif
                        
                    break;   
                case _DATA_EVENT:
                    for ( i = 0; i < Length; i++)     //bo crc mess
                        APP_LOG(TS_OFF, VLEVEL_L, "%02X", pData[i]);
                    
                    Lora_Pro_Save_Record_EVENT(pData, Length);
                #ifdef DEVICE_TYPE_GATEWAY
                    sMQTT.aMARK_MESS_PENDING[SEND_EVENT_MESS] = TRUE;
                #endif
                    
                    break;
                case _DATA_RESPONSE:
                    APP_LOG(TS_OFF, VLEVEL_L, "%s", pData);
                    //forward to server
                    UTIL_MEM_set (aRESPONDSE_AT, 0, sizeof ( aRESPONDSE_AT ));
                    sModem.strATResp.Length_u16 = 0;
                    //Dong goi Cmd
                    aRESPONDSE_AT[sModem.strATResp.Length_u16++] = '(';
                    
                    for (i = 0; i < sModem.strATCmd.Length_u16; i++)
                        aRESPONDSE_AT[sModem.strATResp.Length_u16++] = *(sModem.strATCmd.Data_a8 + i);
                    
                    aRESPONDSE_AT[sModem.strATResp.Length_u16++] = ')';
                    //Dong goi phan hoi
                    aRESPONDSE_AT[sModem.strATResp.Length_u16++] = '(';
                    for (i = 0; i < Length; i++)
                        aRESPONDSE_AT[sModem.strATResp.Length_u16++] = *(pData + i);
                    
                    aRESPONDSE_AT[sModem.strATResp.Length_u16++] = ')';
                    
                    sModem.strATCmd.Length_u16 = 0; 
                #ifdef DEVICE_TYPE_GATEWAY
                    sMQTT.aMARK_MESS_PENDING[SEND_RESPOND_FROM_RF] = TRUE;
                #endif
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
        
    return 1;
}

/*
    Func: Save Rec TSVH
        + Data: Bo qua crc: add obis and
        + Ghep them voi cac obis khac tu GW: CSQ2, GPS... : CHo nhung cai nay vao ban tin mping
*/


void Lora_Pro_Save_Record_TSVH (uint8_t *pdata, uint8_t Length)
{
    //Luu vao Flash
    #ifdef MEMORY_ON_FLASH
        Flash_Save_Record (&sRecTSVH, pdata, Length);
    #else       
        ExMem_Save_Record (&sRecTSVH, pdata, Length);
    #endif
}

void Lora_Pro_Save_Record_EVENT (uint8_t *pdata, uint8_t Length)
{
    //Luu vao Flash
    #ifdef MEMORY_ON_FLASH
        Flash_Save_Record (&sRecEvent, pdata, Length);
    #else       
        ExMem_Save_Record (&sRecEvent, pdata, Length);
    #endif
}
                  
 
uint8_t Lora_Pro_Master_Packet_Payload (uint8_t *pData, uint8_t *length)
{
    uint8_t TypeData_u8 = _DATA_NONE;
    uint8_t Index = *length;
    
    //Payload Req sTime
    if (sCtrlLora.CtrlStime_u8 == TRUE)
    {
        Get_RTC();
        Lora_Pro_Packet_sTime(pData, length);
        TypeData_u8 = _DATA_CONFIG;
    } 
   
    if ( (shLora.rAtcmd_u8 == TRUE) && (TypeData_u8 == _DATA_NONE) )
    {
        TypeData_u8 = _DATA_AT_CMD;
        
        for (uint8_t i = 0; i < shLora.LengthConfig_u8; i++)
            *(pData + Index++) = shLora.aDATA_CONFIG[i];
        
        *(pData + Index++) = 0x01;  //Crc Temp
        *length = Index;
        //clear rq atcmd data
        shLora.rAtcmd_u8 = FALSE;
        shLora.LengthConfig_u8 = 0;
    } else if (TypeData_u8 != _DATA_NONE)
    {
        //Set flag Fpending to notify slave: Rx next mess
        
    }
        
    return TypeData_u8;
}

void Lora_Pro_Packet_sTime (uint8_t *pData, uint8_t *length)
{
    uint8_t crc = 0;
    uint8_t index = *length;
    uint8_t i = 0;
    
    pData[index++] = 0x01;
    pData[index++] = 0x06;
    
    pData[index++] = sRTC.year;  
    pData[index++] = sRTC.month;
    pData[index++] = sRTC.date;
    pData[index++] = sRTC.hour;
    pData[index++] = sRTC.min;
    pData[index++] = sRTC.sec;
    
    for (i = (*length); i < index; i++)
        crc ^= pData[i];
    
    pData[index++] = crc; 
    
    *length = index;
}




/*
    Func: Packet Header Mess:

    return:
            1: Have ACK hoac Request
            0: not ACK Request, Only Payload Data
*/

uint8_t Lora_Pro_Packet_Header (LoRaFrame_t *pFrame, uint8_t MessType, uint8_t DataType)
{
    //Header
    pFrame->Header.Bits.Mtype_u8 = MessType;
    pFrame->Header.Bits.DataDevType = DataType;
    //Ctrl
#ifdef DEVICE_TYPE_GATEWAY
    pFrame->FCtrl.Bits.Ack_u8   = sCtrlLora.CtrlACK_u8;
    pFrame->FCtrl.Bits.rAdr_u8  = FALSE;
    pFrame->FCtrl.Bits.rAtr_u8  = FALSE;
    pFrame->FCtrl.Bits.rStime_u8 = FALSE;
    pFrame->FCtrl.Bits.rIncTxPower_u8 = FALSE;
    pFrame->FCtrl.Bits.rDecTxPower_u8 = FALSE;
    pFrame->FCtrl.Bits.NotUsed_u8     = 0;
        
    pFrame->DevAddr = sLoraVar.sFrameRx.DevAddr;
#else
    uint32_t DCUID_u32 = 0;
    uint8_t i = 0;
    
    pFrame->FCtrl.Bits.Ack_u8 = sCtrlLora.CtrlACK_u8;
    pFrame->FCtrl.Bits.rAdr_u8 = sCtrlLora.CtrlAdr_u8;
    pFrame->FCtrl.Bits.rAtr_u8 = sCtrlLora.CtrlAtr_u8;
    pFrame->FCtrl.Bits.rStime_u8 = sCtrlLora.CtrlStime_u8;  
    pFrame->FCtrl.Bits.rIncTxPower_u8 = FALSE;
    pFrame->FCtrl.Bits.rDecTxPower_u8 = FALSE;
    pFrame->FCtrl.Bits.NotUsed_u8     = 0;
    
    for (i = 0; i < 4; i++)
    {
        DCUID_u32 = (DCUID_u32 << 8) + *(sModem.sDCU_id.Data_a8 + i);
    }
    //DevAddr and Fcnt
    pFrame->DevAddr = DCUID_u32;
#endif
    pFrame->FCnt    = sCtrlLora.MessCount_u16;
    //mem set value ctrlLora
    UTIL_MEM_set ((StructControlLora *) &sCtrlLora, 0, sizeof(sCtrlLora) );
    
    //Kiem tra bit ACK
    if ((pFrame->FCtrl.Value & 0x3F) != 0)
    {
        return 1;
    }
    
    return 0;
}












