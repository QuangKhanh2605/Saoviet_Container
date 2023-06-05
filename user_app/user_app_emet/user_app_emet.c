

#include "user_app_emet.h"
#include "user_define.h"

#include "e_measure.h"
#include "temp_humid.h"
#include "relay.h"

/*================ Define =================*/

extern struct_Metering_Data        sMetering_Data;
extern sTemp_Humid_Struct          sTemp_Humid;
extern sRelay_struct               sRelay[RELAY_NUMBER];
extern struct_Metering_Threshold   sMetering_Threshold;
/*================ Struct =================*/
sEvent_struct sEmetAppEvent [] =
{
    { 0, 		0, 0, 0, 	    _Cb_Log_TSVH },
    { 1,	    0, 0, 1000,     _Cb_Scan_Alarm },
};


/*================ Struct =================*/

uint8_t _Cb_Log_TSVH(uint8_t event)
{
	//Log Data meter to flash
    PrintDebug(&uart_debug, (uint8_t*) "=Log TSVH data=\r\n", 17, 1000);
    //
    Get_RTC();

    if (sRTC.year < 20)
    {
        return 0;
    }
    //Record TSVH
    Modem_Log_Data_TSVH(&sRecTSVH);
    //Mark flash mess after sleep
#ifdef USING_APP_SIM
    sMQTT.aMARK_MESS_PENDING[DATA_TSVH_OPERA] = TRUE;
    sMQTT.aMARK_MESS_PENDING[SEND_EVENT_MESS] = TRUE;
#endif

	return 1;
}



uint8_t _Cb_Scan_Alarm(uint8_t event)
{
    Get_RTC();

    if (sRTC.year < 20)
    {
        return 0;
    }
    /*
        Cacul and check alarm: Vbat, Threshold flow, peak, Dir change
        If have Alarm mess -> Send immediately
    */
    if (Modem_Check_Alarm() == 1)
    {
        //Dong goi Event Mess
        Modem_Log_Data_Event (&sRecEvent);

    #ifdef USING_APP_SIM
        fevent_active(sEventApp, EVENT_SIM_SEND_MESS);
        sMQTT.aMARK_MESS_PENDING[SEND_EVENT_MESS] = TRUE;
    #endif
    }


	return 1;
}

/*
    - Function tinh toan va check alarm
        + Co the doc ra tu record truoc va lay lai duoc chi so o phia truoc + thoi gian: Sau do tính
        + Neu nhu thiet bi duoc cam moi hoan toan:
                *Index Record  = 0
                *Page cuoi cung se bi xoa roi. Nen se check ra firstbyte = 0 va length = 0.
                * Khi do check alarm se khong bao alarm
*/

uint8_t Modem_Check_Alarm (void)
{


    return 0;
}

/*
    Func: handler when check Alarm true
        + Neu truoc do da canh bao -> Sau MAX_ALARM_CONTINUOUS lan ms dc canh bao lai
        + Neu truoc do chua co canh bao: Mark alarm
*/
uint8_t Modem_Handler_Alarm (uint8_t Index)
{


    return 0;
}






uint8_t Modem_Packet_TSVH (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;
    //----------------------- start send data below --------------------- //
    //----------sTime--------------------
    pData[length++] = 0x01;   // sTime
    pData[length++] = 0x06;
    pData[length++] = sRTC.year;
    pData[length++] = sRTC.month;
    pData[length++] = sRTC.date;
    pData[length++] = sRTC.hour;
    pData[length++] = sRTC.min;
    pData[length++] = sRTC.sec;
    //----------Dien ap pha A-------------------
    pData[length++] = 0x21;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.RMSVol_PA_x100 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.RMSVol_PA_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------Dien ap pha B-------------------
    pData[length++] = 0x22;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.RMSVol_PB_x100 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.RMSVol_PB_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------Dien ap pha C-------------------
    pData[length++] = 0x23;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.RMSVol_PC_x100 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.RMSVol_PC_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------Dong dien Pha A-------------------
    pData[length++] = 0x25;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.RMSCur_PA_x100 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.RMSCur_PA_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------Dong dien Pha B-------------------
    pData[length++] = 0x26;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.RMSCur_PB_x100 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.RMSCur_PB_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------Dong dien Pha C-------------------
    pData[length++] = 0x27;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.RMSCur_PC_x100 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.RMSCur_PC_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------He so cong suat A-------------------
    pData[length++] = 0x2A;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.PF_PA_x1000 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.PF_PA_x1000 & 0xFF;

    pData[length++] = 0xFD;
    //----------He so cong suat B-------------------
    pData[length++] = 0x2B;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.PF_PB_x1000 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.PF_PB_x1000 & 0xFF;

    pData[length++] = 0xFD;
    //----------He so cong suat C-------------------
    pData[length++] = 0x2C;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.PF_PC_x1000 >> 8) & 0xFF;
    pData[length++] = sMetering_Data.PF_PC_x1000 & 0xFF;

    pData[length++] = 0xFD;

    //----------Cong suat pha A-------------------
    pData[length++] = 0x2E;
    pData[length++] = 0x04;

    pData[length++] = (sMetering_Data.ActPower_PA_x100>> 24) & 0xFF;
    pData[length++] = (sMetering_Data.ActPower_PA_x100>> 16) & 0xFF;
    pData[length++] = (sMetering_Data.ActPower_PA_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.ActPower_PA_x100 & 0xFF;

    pData[length++] = 0xFE;

    //----------Cong suat pha B-------------------
    pData[length++] = 0x2F;
    pData[length++] = 0x04;

    pData[length++] = (sMetering_Data.ActPower_PB_x100>> 24) & 0xFF;
    pData[length++] = (sMetering_Data.ActPower_PB_x100>> 16) & 0xFF;
    pData[length++] = (sMetering_Data.ActPower_PB_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.ActPower_PB_x100 & 0xFF;

    pData[length++] = 0xFE;

    //----------Cong suat pha C-------------------
    pData[length++] = 0x30;
    pData[length++] = 0x04;

    pData[length++] = (sMetering_Data.ActPower_PC_x100>> 24) & 0xFF;
    pData[length++] = (sMetering_Data.ActPower_PC_x100>> 16) & 0xFF;
    pData[length++] = (sMetering_Data.ActPower_PC_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.ActPower_PC_x100 & 0xFF;

    pData[length++] = 0xFE;


    //----------Cong suat phan khang pha A -------------------
    pData[length++] = 0x36;
    pData[length++] = 0x04;

    pData[length++] = (sMetering_Data.ReActPower_PA_x100>> 24) & 0xFF;
    pData[length++] = (sMetering_Data.ReActPower_PA_x100>> 16) & 0xFF;
    pData[length++] = (sMetering_Data.ReActPower_PA_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.ReActPower_PA_x100 & 0xFF;

    pData[length++] = 0xFE;

    //----------Cong suat phan khang pha B -------------------
    pData[length++] = 0x37;
    pData[length++] = 0x04;

    pData[length++] = (sMetering_Data.ReActPower_PB_x100>> 24) & 0xFF;
    pData[length++] = (sMetering_Data.ReActPower_PB_x100>> 16) & 0xFF;
    pData[length++] = (sMetering_Data.ReActPower_PB_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.ReActPower_PB_x100 & 0xFF;

    pData[length++] = 0xFE;

    //----------Cong suat phan khang pha C -------------------
    pData[length++] = 0x38;
    pData[length++] = 0x04;

    pData[length++] = (sMetering_Data.ReActPower_PC_x100>> 24) & 0xFF;
    pData[length++] = (sMetering_Data.ReActPower_PC_x100>> 16) & 0xFF;
    pData[length++] = (sMetering_Data.ReActPower_PC_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.ReActPower_PC_x100 & 0xFF;

    pData[length++] = 0xFE;

    //---------- Goc lech pha A -------------------
    pData[length++] = 0x39;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.Phase_Agnle_PA_x10>> 8) & 0xFF;
    pData[length++] = sMetering_Data.Phase_Agnle_PA_x10 & 0xFF;

    pData[length++] = 0xFF;

    //---------- Goc lech pha B -------------------
    pData[length++] = 0x3A;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.Phase_Agnle_PB_x10>> 8) & 0xFF;
    pData[length++] = sMetering_Data.Phase_Agnle_PB_x10 & 0xFF;

    pData[length++] = 0xFF;

    //---------- Goc lech pha C -------------------
    pData[length++] = 0x3B;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.Phase_Agnle_PC_x10>> 8) & 0xFF;
    pData[length++] = sMetering_Data.Phase_Agnle_PC_x10 & 0xFF;

    pData[length++] = 0xFF;

    //---------- Tan so -------------------
    pData[length++] = 0x28;
    pData[length++] = 0x02;

    pData[length++] = (sMetering_Data.Frequency_x100>> 8) & 0xFF;
    pData[length++] = sMetering_Data.Frequency_x100 & 0xFF;

    pData[length++] = 0xFE;

    //----------Nhien do -------------------
    pData[length++] = 0x70;
    pData[length++] = 0x02;

    pData[length++] = (sTemp_Humid.temp_x100 >> 8) & 0xFF;
    pData[length++] = sTemp_Humid.temp_x100 & 0xFF;

    pData[length++] = 0xFE;
    //----------Do am -------------------
    pData[length++] = 0x72;
    pData[length++] = 0x02;

    pData[length++] = (sTemp_Humid.humid_x1 >> 8) & 0xFF;
    pData[length++] = sTemp_Humid.humid_x1 & 0xFF;

    pData[length++] = 0x00;

    //---------- Relay 1 -------------------
    pData[length++] = 0x90;
    pData[length++] = 0x01;

    pData[length++] = sRelay[RELAY_1].status & 0xFF;

    //---------- Relay 2 -------------------
    pData[length++] = 0x91;
    pData[length++] = 0x01;

    pData[length++] = sRelay[RELAY_2].status & 0xFF;

    //---------- Relay 3 -------------------
    pData[length++] = 0x92;
    pData[length++] = 0x01;

    pData[length++] = sRelay[RELAY_3].status & 0xFF;

    //---------- Relay 4 -------------------
    pData[length++] = 0x93;
    pData[length++] = 0x01;

    pData[length++] = sRelay[RELAY_4].status & 0xFF;

    //---------- Relay 5 -------------------
    pData[length++] = 0x94;
    pData[length++] = 0x01;

    pData[length++] = sRelay[RELAY_5].status & 0xFF;

    //---------- Mode  -------------------
    pData[length++] = 0x03;
    pData[length++] = 0x01;

    pData[length++] = 01;

    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;


    return length;
}


uint8_t Modem_Packet_Event (uint8_t *pData)
{
    uint8_t     length = 0;
    uint16_t	i = 0;
    uint8_t     TempCrc = 0;



    // caculator crc
    length++;
	for (i = 0; i < (length - 1); i++)
		TempCrc ^= pData[i];

    pData[length-1] = TempCrc;

    return length;
}


/*
    Func: Log Data Event
        + Packet Event Data
        + Save to Flash or ExMem
*/

void Modem_Log_Data_Event (StructManageRecordFlash *sRecordEvent)
{
    uint8_t     aMessData[SIZE_DATA_RECORD] = {0};
    uint8_t     Length = 0;

    if (sRTC.year <= 20)
        return;

    Length = Modem_Packet_Event (&aMessData[0]);
	//Luu vao Flash
#ifdef MEMORY_ON_FLASH
    Flash_Save_Record (sRecordEvent, &aMessData[0], Length);
#else
    ExMem_Save_Record (sRecordEvent, &aMessData[0], Length);
#endif

}

/*
    Func: Log Data TSVH
        + Packet Data TSVH
        + Save to Flash or ExMem
*/

void Modem_Log_Data_TSVH (StructManageRecordFlash *sRecordTSVH)
{
    uint8_t     aMessData[SIZE_DATA_RECORD] = {0};
    uint8_t     Length = 0;

    if (sRTC.year <= 20)
        return;

    Length = Modem_Packet_TSVH (&aMessData[0]);

    //Luu vao Flash
#ifdef MEMORY_ON_FLASH
    Flash_Save_Record (sRecordTSVH, &aMessData[0], Length);
#else
    ExMem_Save_Record (sRecordTSVH, &aMessData[0], Length);
#endif
}


//
//void Init_Threshold_Measure (void)
//{
//#ifdef MEMORY_ON_FLASH
//    uint8_t 	temp = 0xFF;
//	uint8_t 	Length = 0;
//	uint8_t		Buff_temp[16] = {0};
//
//	// Doc gia tri cau hinh threshold. Dung de check cac alarm
//    temp = *(__IO uint8_t*) ADDR_COFI_HIGH_PEAK;    //2 byte
//	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
//    {
//        Length = *(__IO uint8_t*)(ADDR_COFI_HIGH_PEAK + 1);
//        if (Length == 2)
//        {
//            OnchipFlashReadData(ADDR_COFI_HIGH_PEAK + 2, &Buff_temp[0], 2);
//            sMeterThreshold.PeakHigh = (Buff_temp[0] << 8) | Buff_temp[1];
//        } else  //Cho bang mac dinh
//            sMeterThreshold.PeakHigh = 0xFFFF;
//    } else
//    {
//        sMeterThreshold.PeakHigh = 0xFFFF;       //mac dinh se set cao nhat de khong bi canh bao
//        Buff_temp[0] = 0xAA;
//        Buff_temp[1] = 2;
//        Buff_temp[2] = (sMeterThreshold.PeakHigh >> 8) & 0xFF;
//        Buff_temp[3] = sMeterThreshold.PeakHigh & 0xFF;
//
//        OnchipFlashPageErase(ADDR_COFI_HIGH_PEAK);
//        OnchipFlashWriteData(ADDR_COFI_HIGH_PEAK, &Buff_temp[0], 16);
//    }
//
//    // Peak low
//    temp = *(__IO uint8_t*) ADDR_COFI_LOW_PEAK;    //2 byte
//	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
//    {
//        Length = *(__IO uint8_t*)(ADDR_COFI_LOW_PEAK + 1);
//        if (Length == 2)
//        {
//            OnchipFlashReadData(ADDR_COFI_LOW_PEAK + 2, &Buff_temp[0], 2);
//            sMeterThreshold.PeakLow = (Buff_temp[0] << 8) | Buff_temp[1];
//        } else
//            sMeterThreshold.PeakLow = 0;
//    } else
//    {
//        sMeterThreshold.PeakLow = 0;       //mac dinh se set cao nhat de khong bi canh bao
//        Buff_temp[0] = 0xAA;
//        Buff_temp[1] = 2;
//        Buff_temp[2] = (sMeterThreshold.PeakLow >> 8) & 0xFF;
//        Buff_temp[3] = sMeterThreshold.PeakLow & 0xFF;
//
//        OnchipFlashPageErase(ADDR_COFI_LOW_PEAK);
//        OnchipFlashWriteData(ADDR_COFI_LOW_PEAK, &Buff_temp[0], 16);
//    }
//
//    // flow high
//    temp = *(__IO uint8_t*) ADDR_COFI_HIGH_FLOW;    //2 byte
//	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
//    {
//        Length = *(__IO uint8_t*)(ADDR_COFI_HIGH_FLOW+1);
//        if (Length == 2)
//        {
//            OnchipFlashReadData(ADDR_COFI_HIGH_FLOW+2, &Buff_temp[0], 2);
//            sMeterThreshold.FlowHigh = (Buff_temp[0] << 8) | Buff_temp[1];
//        } else
//            sMeterThreshold.FlowHigh = 0xFFFF;
//    } else
//    {
//        sMeterThreshold.FlowHigh = 0xFFFF;       //mac dinh se set cao nhat de khong bi canh bao
//        Buff_temp[0] = 0xAA;
//        Buff_temp[1] = 2;
//        Buff_temp[2] = (sMeterThreshold.FlowHigh >> 8) & 0xFF;
//        Buff_temp[3] = sMeterThreshold.FlowHigh & 0xFF;
//
//        OnchipFlashPageErase(ADDR_COFI_HIGH_FLOW);
//        OnchipFlashWriteData(ADDR_COFI_HIGH_FLOW, &Buff_temp[0], 16);
//    }
//
//    // flow low
//    temp = *(__IO uint8_t*) ADDR_COFI_LOW_FLOW;    //2 byte
//	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
//    {
//        Length = *(__IO uint8_t*)(ADDR_COFI_LOW_FLOW+1);
//        if (Length == 2)
//        {
//            OnchipFlashReadData(ADDR_COFI_LOW_FLOW+2, &Buff_temp[0], 2);
//            sMeterThreshold.FlowLow = (Buff_temp[0] << 8) | Buff_temp[1];
//        } else
//            sMeterThreshold.FlowLow = 0;
//    } else
//    {
//        sMeterThreshold.FlowLow = 0;       //mac dinh 0
//        Buff_temp[0] = 0xAA;
//        Buff_temp[1] = 2;
//        Buff_temp[2] = (sMeterThreshold.FlowLow >> 8) & 0xFF;
//        Buff_temp[3] = sMeterThreshold.FlowLow & 0xFF;
//
//        OnchipFlashPageErase(ADDR_COFI_LOW_FLOW);
//        OnchipFlashWriteData(ADDR_COFI_LOW_FLOW, &Buff_temp[0], 16);
//    }
//    //Gia tri threshold dien ap
//
//    temp = *(__IO uint8_t*) ADDR_COFI_LOW_BATERY;    //2 byte
//	if (temp != FLASH_BYTE_EMPTY)   //  Doc cau hinh so lan retry send cua 1 ban tin
//    {
//        Length = *(__IO uint8_t*)(ADDR_COFI_LOW_BATERY+1);
//        if (Length == 1)
//        {
//            sMeterThreshold.LowBatery = *(__IO uint8_t*)(ADDR_COFI_LOW_BATERY+2);
//        } else
//            sMeterThreshold.LowBatery = 10;
//    } else
//    {
//        sMeterThreshold.LowBatery = 10;       //mac dinh 10%
//        Buff_temp[0] = 0xAA;
//        Buff_temp[1] = 2;
//        Buff_temp[2] = (sMeterThreshold.LowBatery >> 8) & 0xFF;
//        Buff_temp[3] = sMeterThreshold.LowBatery & 0xFF;
//
//        OnchipFlashPageErase(ADDR_COFI_LOW_BATERY);
//        OnchipFlashWriteData(ADDR_COFI_LOW_BATERY, &Buff_temp[0], 16);
//    }
//#else
//    uint8_t aTEMP[40] = {0};
//
//    //Peak High
//    UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//    CAT24Mxx_Read_Array(CAT_ADDR_COFI_HIGH_PEAK, aTEMP, 40);
//    if ((aTEMP[0] != CAT_BYTE_EMPTY) && (aTEMP[1] != CAT_BYTE_EMPTY))
//    {
//        sMeterThreshold.PeakHigh = (aTEMP[2] << 8) | aTEMP[3];
//    } else
//    {
//        UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//        aTEMP[0] = 0xAA;
//        aTEMP[1] = 2;
//        aTEMP[2] = (sMeterThreshold.PeakHigh >> 8) & 0xFF;
//        aTEMP[3] = sMeterThreshold.PeakHigh & 0xFF;
//
//        CAT24Mxx_Write_Array(CAT_ADDR_COFI_HIGH_PEAK, &aTEMP[0], 4);
//    }
//    //Peak low
//    UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//    CAT24Mxx_Read_Array(CAT_ADDR_COFI_LOW_PEAK, aTEMP, 40);
//    if ((aTEMP[0] != CAT_BYTE_EMPTY) && (aTEMP[1] != CAT_BYTE_EMPTY))
//    {
//        sMeterThreshold.PeakLow = (aTEMP[2] << 8) | aTEMP[3];
//    } else
//    {
//        UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//        aTEMP[0] = 0xAA;
//        aTEMP[1] = 2;
//        aTEMP[2] = (sMeterThreshold.PeakLow >> 8) & 0xFF;
//        aTEMP[3] = sMeterThreshold.PeakLow & 0xFF;
//
//        CAT24Mxx_Write_Array(CAT_ADDR_COFI_LOW_PEAK, &aTEMP[0], 4);
//    }
//    //Flow High
//    UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//    CAT24Mxx_Read_Array(CAT_ADDR_COFI_HIGH_FLOW, aTEMP, 40);
//    if ((aTEMP[0] != CAT_BYTE_EMPTY) && (aTEMP[1] != CAT_BYTE_EMPTY))
//    {
//        sMeterThreshold.FlowHigh = (aTEMP[2] << 8) | aTEMP[3];
//    } else
//    {
//        UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//        aTEMP[0] = 0xAA;
//        aTEMP[1] = 2;
//        aTEMP[2] = (sMeterThreshold.FlowHigh >> 8) & 0xFF;
//        aTEMP[3] = sMeterThreshold.FlowHigh & 0xFF;
//
//        CAT24Mxx_Write_Array(CAT_ADDR_COFI_HIGH_FLOW, &aTEMP[0], 4);
//    }
//    //Flow low
//    UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//    CAT24Mxx_Read_Array(CAT_ADDR_COFI_LOW_FLOW, aTEMP, 40);
//    if ((aTEMP[0] != CAT_BYTE_EMPTY) && (aTEMP[1] != CAT_BYTE_EMPTY))
//    {
//        sMeterThreshold.FlowLow = (aTEMP[2] << 8) | aTEMP[3];
//    } else
//    {
//        UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//        aTEMP[0] = 0xAA;
//        aTEMP[1] = 2;
//        aTEMP[2] = (sMeterThreshold.FlowLow >> 8) & 0xFF;
//        aTEMP[3] = sMeterThreshold.FlowLow & 0xFF;
//
//        CAT24Mxx_Write_Array(CAT_ADDR_COFI_LOW_FLOW, &aTEMP[0], 4);
//    }
//    //Battery low
//    UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//    CAT24Mxx_Read_Array(CAT_ADDR_COFI_LOW_BATERY, aTEMP, 40);
//    if ((aTEMP[0] != CAT_BYTE_EMPTY) && (aTEMP[1] != CAT_BYTE_EMPTY))
//    {
//        sMeterThreshold.LowBatery = (aTEMP[2] << 8) | aTEMP[3];
//    } else
//    {
//        UTIL_MEM_set(aTEMP, 0xFF, sizeof(aTEMP));
//        aTEMP[0] = 0xAA;
//        aTEMP[1] = 2;
//        aTEMP[2] = (sMeterThreshold.LowBatery >> 8) & 0xFF;
//        aTEMP[3] = sMeterThreshold.LowBatery & 0xFF;
//
//        CAT24Mxx_Write_Array(CAT_ADDR_COFI_LOW_BATERY, &aTEMP[0], 4);
//    }
//#endif
//
//}

void _rREQUEST_SETTING(sData *str_Receiv,int16_t Pos)
{
    uint8_t   length = sMark_MessageRecei_Type[REQUEST_SETTING].sKind.Length_u16;
    uint16_t  PosFix = length + Pos;
    uint8_t   TempLeng;
    uint8_t   ObisConfig = 0;
    uint8_t   i = 0;
    uint32_t  TempData_u32 = 0;
    uint8_t   Threshold_type = 0;
    uint8_t   aTempData[50] = {0};
//    uint32_t  aScale[6] = {1, 10, 100, 1000, 10000, 100000};
//    uint32_t  Scale = 0;

    //chay tu tren xuong duoi de check tat ca cac obis cau hinh
    while ((PosFix + 4) <= str_Receiv->Length_u16)   //vi byte cuoi la crc
    {
        TempLeng = 0;
        ObisConfig = *(str_Receiv->Data_a8 + PosFix++);
        switch(ObisConfig)   //luon luon co 1 byte crc phía sau
        {
            case 0xD0:  // obis|length|scale|data
                    TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                    Threshold_type = 1;
                break;
            case 0xD1:
                    TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                    Threshold_type = 2;
            break;
            case 0x21:
                TempData_u32 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                        TempData_u32 = (TempData_u32 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                PosFix++;// get scale (ignore)
                switch(Threshold_type)
                {
                  case 1:
                  sMetering_Threshold.Vol_PB_1_x100 = TempData_u32;
                  break;
                  case 2:
                    sMetering_Threshold.Vol_PA_1_x100 = TempData_u32;
                  break;
                  default:
                    return;
                  break;
                }
                e_measure_storage_setting_vol();

                DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 2, 1);  //
                SysApp_Set_Next_TxTimer();

                Threshold_type = 0;
            break;
          case 0x2A:
                TempData_u32 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                        TempData_u32 = (TempData_u32 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                PosFix++;// get scale (ignore)
                switch(Threshold_type)
                {
                  case 1:
                  sMetering_Threshold.PF_PA_2_x1000 = TempData_u32;
                  break;
                  case 2:
                    sMetering_Threshold.PF_PA_1_x1000 = TempData_u32;
                  break;
                  default:
                    return;
                  break;
                }
                e_measure_storage_setting_pf();

                DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 2, 1);
                SysApp_Set_Next_TxTimer();

                Threshold_type = 0;
          break;
         case 0x3C:
                TempData_u32 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                        TempData_u32 = (TempData_u32 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                PosFix++;// get scale (ignore)
                switch(Threshold_type)
                {
                  case 1:
                  sMetering_Threshold.CT_value_T = TempData_u32;
                  break;
                  case 2:
                    sMetering_Threshold.CT_value_M = TempData_u32;
                  break;
                  default:
                    return;
                  break;
                }
                e_measure_storage_setting_ct();

                DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 2, 1);
                SysApp_Set_Next_TxTimer();

                Threshold_type = 0;
        break;
        case 0x9A:
                TempData_u32 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                        TempData_u32 = (TempData_u32 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                PosFix++;// get scale (ignore)

                sMetering_Threshold.Time_on_x1000 = TempData_u32;
                e_measure_storage_setting_time();

                DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 2, 1);
                SysApp_Set_Next_TxTimer();
          break;
        case 0x9B:
                TempData_u32 = 0;
                TempLeng = *(str_Receiv->Data_a8 + PosFix++);
                for (i = 0; i < TempLeng; i++)
                        TempData_u32 = (TempData_u32 << 8 ) | *(str_Receiv->Data_a8 + PosFix++);
                PosFix++;// get scale (ignore)

                sMetering_Threshold.Time_off_x1000 = TempData_u32;
                e_measure_storage_setting_time();

                DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)"\r\nOK", 2, 1);
                SysApp_Set_Next_TxTimer();
          break;
        case 0xFF:
                i = 0;
                aTempData[i++] = 0xD0;
                aTempData[i++] = 0x0F;
                aTempData[i++] = 0x21;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.Vol_PB_1_x100 >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.Vol_PB_1_x100 & 0xFF;
                aTempData[i++] = 0xFE;
                aTempData[i++] = 0x2A;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.PF_PA_2_x1000 >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.PF_PA_2_x1000 & 0xFF;
                aTempData[i++] = 0xFD;
                aTempData[i++] = 0x3C;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.CT_value_T >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.CT_value_T & 0xFF;
                aTempData[i++] = 0x00;

                aTempData[i++] = 0xD1;
                aTempData[i++] = 0x0F;
                aTempData[i++] = 0x21;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.Vol_PA_1_x100 >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.Vol_PA_1_x100 & 0xFF;
                aTempData[i++] = 0xFE;
                aTempData[i++] = 0x2A;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.PF_PA_1_x1000 >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.PF_PA_1_x1000 & 0xFF;
                aTempData[i++] = 0xFD;
                aTempData[i++] = 0x3C;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.CT_value_M >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.CT_value_M & 0xFF;
                aTempData[i++] = 0x00;

                aTempData[i++] = 0x9A;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.Time_on_x1000/1000 >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.Time_on_x1000/1000 & 0xFF;
                aTempData[i++] = 0x00;
                aTempData[i++] = 0x9B;
                aTempData[i++] = 0x02;
                aTempData[i++] = (sMetering_Threshold.Time_off_x1000/1000 >> 8) & 0xFF;
                aTempData[i++] = sMetering_Threshold.Time_off_x1000/1000 & 0xFF;
                aTempData[i++] = 0x00;

                DCU_Respond(_AT_REQUEST_SERVER, (uint8_t *)aTempData, i, 1);
                SysApp_Set_Next_TxTimer();
        break;
          default:
            return;
        }
    }
}




