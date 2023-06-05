/*
 * user_bc66_cbAT.c
 *
 *  Created on: 6 Jan 2022
 *      Author: Chien
 */

#include "string.h"
#include "stdint.h"

#include "user_define.h"
#include "user_sim.h"
#include "user_timer.h"
#include "user_mqtt.h"

/*================Func===================*/

uint8_t _Cb_AT_Success(sData *str_Receiv)
{
	return 1;
}

/*
 * 	Func: Handler timeout AT
 *		  Khi thuc hien 1 lenh bi Timeout: |K tra ve resp |Tra ve va chua dung voi str mong muốn
 * */

uint8_t _Cb_AT_Failure(uint8_t Type)
{
	//Tang soft Reset
	sSimVar.CountSoftReset_u8++;
	qQueue_Clear(&qSimStep);
    fevent_disable(sEventSim, EVENT_SIM_SEND_MESS);
	//Nếu retry lai -> Gr_Step_nao thi Push lên các step cân thiết
	if (sSimVar.CountSoftReset_u8 < MAX_SOFT_RESET)
	{
		switch (sSimVar.GroupStepID_u8)
		{
			case GR_PREE_INIT:
			case GR_CHECK_ATTACH:
			case GR_SETTING:
				fPushBlockSimStepToQueue(aSIM_STEP_CONTROL, sizeof(aSIM_STEP_CONTROL));
				break;
			case GR_INIT_TCP:
			case GR_SEND_MQTT:
				fPushBlockSimStepToQueue(aSIM_STEP_CLOSE_TCP, sizeof(aSIM_STEP_CLOSE_TCP));
				fPushBlockSimStepToQueue(&aSIM_STEP_CONTROL[17], sizeof(aSIM_STEP_CONTROL) - 17);
				break;
			case GR_CLOSE_TCP:
				sSimVar.CountSoftReset_u8 = MAX_SOFT_RESET;
                sSimVar.CountHardReset_u8 = MAX_HARD_RESET;
				fevent_active(sEventSim, EVENT_SIM_HARD_RESET);
				break;
			default:
				break;
		}
	} else
	{
        sSimVar.CountHardReset_u8++;
        sSimVar.CountSoftReset_u8 = 0;
        fevent_active(sEventSim, EVENT_SIM_HARD_RESET);
	}

	return 1;
}

uint8_t _Cb_AT_SET_BAND_3 (sData *str_Receiv)
{
	sSimVar.GroupStepID_u8 = GR_CHECK_ATTACH;

	return 1;
}

uint8_t _Cb_AT_CHECK_SIM(sData *str_Receiv)
{
	sSimVar.CallReady_u8 	= TRUE;

	return 1;
}

uint8_t _Cb_AT_CHECK_RSSI(sData *str_Receiv)           //+CSQ: 99,99  +CSQ: 0,0  +CSQ: 99,99
{
	char *p;

	p = strstr((char*) str_Receiv->Data_a8, aSimStep[_AT_CHECK_RSSI].at_response);
	if (p != NULL)
    {
		sSimInfor.RSSI_u8 = *(p + 5) - 0x30;
		sSimInfor.Ber_u8  = *(p + 7) - 0x30;

		if(*(p + 7) == 0x2C)
        {
			sSimInfor.RSSI_u8 = ((*(p + 5)) << 4) + ((*(p + 6)) & 0x0F);
			sSimInfor.Ber_u8 = *(p + 8) - 0x30;  // ber 0->7
		}
	}
    
	/* convert signal strength */
	switch(sSimInfor.RSSI_u8)
    {
		case 0x00:
			sSimInfor.RSSI_u8 = 115;
            sSimInfor.RSSILevel_u8 = 0;
			break;
		case 0x01:
            sSimInfor.RSSILevel_u8 = sSimInfor.RSSI_u8;
			sSimInfor.RSSI_u8 = 111;
			break;
		case 0x31:
            sSimInfor.RSSILevel_u8 = sSimInfor.RSSI_u8;
			sSimInfor.RSSI_u8 = 52;
			break;
        case 0x99:
            sSimInfor.RSSILevel_u8 = 0;
        	sSimInfor.RSSI_u8 = 0;
			break;
		default:
          //công thuc này can xem lai
            sSimInfor.RSSILevel_u8 = sSimInfor.RSSI_u8;
			sSimInfor.RSSI_u8 = 114 - ((sSimInfor.RSSI_u8 / 16) * 10 + sSimInfor.RSSI_u8 % 16) * 2;
			break;
	}
    //Convert CSQ to level: 1 dai tu  0 -> 0x20.
    if (sSimInfor.RSSILevel_u8 > MAX_CSQ)   
        sSimInfor.RSSILevel_u8 = MAX_LEVEL_CSQ;
    else
        sSimInfor.RSSILevel_u8 = ( ( (uint32_t) (sSimInfor.RSSILevel_u8 - MIN_CSQ) * MAX_LEVEL_CSQ ) / (MAX_CSQ-MIN_CSQ) ); 

	return 1;
}


uint8_t _Cb_AT_CHECK_CREG(sData *str_Receiv)
{
	//Check lai CREG

	return 1;
}



uint8_t _Cb_AT_SIM_ID(sData *str_Receiv)
{
	sData			sRes;
	uint8_t 		Pos,var;
	uint8_t			aTEMP_SIM_ID[MAX_LENGTH_SIM_ID] = {0};

	sRes.Data_a8 	= aTEMP_SIM_ID;
	sRes.Length_u16 = 0;

	for (Pos = 1; Pos < str_Receiv->Length_u16; ++Pos)
    {
		if ((str_Receiv->Data_a8[Pos-1] == '8') && (str_Receiv->Data_a8[Pos] == '9'))
			break;
	}
	for (var = Pos - 1; var < str_Receiv->Length_u16; var++)
    {
		if ((str_Receiv->Data_a8[var] < '0') || (str_Receiv->Data_a8[var] > '9'))
			break;
	}

	sRes.Data_a8 = &str_Receiv->Data_a8[Pos-1];
	sRes.Length_u16 = var - Pos + 1;

	if(sRes.Length_u16 > MAX_LENGTH_SIM_ID)
		sRes.Length_u16 = MAX_LENGTH_SIM_ID;

	for (var = 0; var < sRes.Length_u16; ++var)
		sSimInfor.sSIMid.Data_a8[var] = sRes.Data_a8[var];

	sSimInfor.sSIMid.Length_u16 = sRes.Length_u16;

	return 1;
}


//+CCLK: 2020/02/24,04:03:29GMT+7  +CCLK: 2020/3/26,3:43:33GMT+7

uint8_t _Cb_AT_GET_CLOCK(sData *str_Receiv)
{
	int 			Pos;
	uint16_t        Temp_Get = 0;
	uint16_t        Temp_year = 0;
	uint8_t         count = 0;
	uint8_t         aRTCtemp[8];
    static uint8_t  MarkFirstGetTime = 0;
    sData   strCheck = {(uint8_t*) aSimStep[_AT_GET_RTC].at_response, strlen(aSimStep[_AT_GET_RTC].at_response)};
    
    sSimVar.GroupStepID_u8 = GR_SEND_MQTT;

    Pos = Find_String_V2(&strCheck, str_Receiv);
	if ((Pos >= 0) && (str_Receiv->Length_u16 > (14 + strlen(aSimStep[_AT_GET_RTC].at_response) + Pos - 1)))
	{
		Pos += strlen(aSimStep[_AT_GET_RTC].at_response) - 1 + 1;   //dau khoang trong va nhay ++ de tro vao year

		//tim vi tri bat dau so decima
		Temp_year = *(str_Receiv->Data_a8 + Pos);
		while ((0x30 > Temp_year) || (Temp_year > 0x39))
		{
			Pos++;
			Temp_year = *(str_Receiv->Data_a8 + Pos);
		}

		//bat dau vi tri co so: year month date hour min sec
		while (count < 6)
		{
			Temp_Get  = 0;
			Temp_year = *(str_Receiv->Data_a8 + Pos);
			while ((0x30 <=  Temp_year) && (Temp_year <= 0x39))
			{
				Temp_Get = Temp_Get* 10 + Temp_year - 0x30;
				Pos++;
				Temp_year = *(str_Receiv->Data_a8 + Pos);
			}

			aRTCtemp[count++] = Temp_Get % 100;               //nam chi lay 2 chu so sau

			if (Pos >= str_Receiv->Length_u16)
			{
				HAL_UART_Transmit(&UART_SERIAL, (uint8_t *) "Qua buff\r\n", 10, 1000);
				break;  //return 1;
			}

			Pos++;
		}
        //Get Temp RTC
		sRTCSet.year   = aRTCtemp[0];
		sRTCSet.month  = aRTCtemp[1];
		sRTCSet.date   = aRTCtemp[2];
		sRTCSet.hour   = aRTCtemp[3];
		sRTCSet.min    = aRTCtemp[4];
		sRTCSet.sec    = aRTCtemp[5];
        
		if (sRTCSet.year < 20)
			return 1;

		Convert_sTime_ToGMT(&sRTCSet, 7);  //Do thoi gian da la gio dia phuong roi. Co nhung module sim thoi gian la gio GMT0
		//Convert lai day. 1/1/2012 la chu nhat. Thu 2 - cn: 2-8
		sRTCSet.day = ((HW_RTC_GetCalendarValue_Second(sRTCSet, 1) / SECONDS_IN_1DAY) + 6) % 7 + 1;
        //Active Event Set RTC
        fevent_active(sEventIdle, EVENT_SET_RTC);
        //active event log mess first
        if (MarkFirstGetTime == 0)
        {
            fevent_active(sEventIdle, EVENT_IDLE);
            fevent_active(sEventIdle, EVENT_LOG_TSVH);
            MarkFirstGetTime = 1;
        }
	}

	return 1;
}


uint8_t _Cb_AT_TCP_CONNECT_1(sData *str_Receiv)
{
	uint8_t aTemp[40] = {0};
	sData   Protocol = {(uint8_t*) "\"TCP\",\"", 7};
	uint8_t length = 0, i = 0;

	//them pay load vao day
	aTemp[length++] = '0' ;            //cortex
	aTemp[length++] = ',';              //Id cua Socket

	aTemp[length++] = '0' ;            //Id cua Socket + 0x30 ;
	aTemp[length++] = ',';             //Id cua Socket

	for(i = 0; i < Protocol.Length_u16;i ++)
	  aTemp[length++] = *(Protocol.Data_a8 + i);


	for(i = 0; i < sServerInfor.sIP.Length_u16;i ++)
		aTemp[length++] = *(sServerInfor.sIP.Data_a8 + i);

	aTemp[length++]  = (uint8_t ) '"';
	aTemp[length++]  = (uint8_t ) ',';

	for(i = 0; i < sServerInfor.sPort.Length_u16;i ++)
		aTemp[length++] = *(sServerInfor.sPort.Data_a8 + i);

	aTemp[length++] = ',';  //local Port
	aTemp[length++] = '0';
	aTemp[length++] = ',';  //direct mode
	aTemp[length++] = '1';

	HAL_UART_Transmit(&UART_SIM, &aTemp[0], length, 1000);

	return 1;
}



uint8_t _Cb_AT_TCP_CONNECT_2(sData *str_Receiv)
{
	char *Pos = 0;

	Pos = strstr((char*) str_Receiv->Data_a8, "+QIOPEN: 0,0");

	if (Pos != NULL)
		return 1;

	return 0;
}


uint8_t _Cb_MQTT_CONNECT_1(sData *str_Receiv)
{
    //Gui sang sim chuoi mqtt connect
	mConnect_MQTT();

	return 1;
}

uint8_t _Cb_MQTT_CONNECT_2(sData *str_Receiv)
{
    //Neu connect OK: ->
    
	return 1;
}

uint8_t _Cb_MQTT_SUBCRIBE_1(sData *str_Receiv)
{
    //Truyen chuoi Subcribe
	mSubcribe_MQTT ();
        
	return 1;
}

uint8_t _Cb_MQTT_SUBCRIBE_2(sData *str_Receiv)
{
    //
    mSet_default_MQTT(); 

    //Subcribe OK: -> active event Send Messange MQTT
    fevent_active(sEventSim, EVENT_SIM_SEND_MESS);
      
	return 1;
}

uint8_t _Cb_MQTT_PUBLISH_1(sData *str_Receiv)
{
	//Thuc hien truyen buff MQTT publish da dong goi
    mPublish_MQTT();
    
	return 1;
}

/*
    Fun: Handler Publish success
        TSVH:   + Cong IndexSend và CountPacket
                + Neu nhu IndexSend == IndexSave thì clear Mess
        Other Mess:
                + Clear Mess

*/
uint8_t _Cb_MQTT_PUBLISH_2(sData *str_Receiv)
{
    //Neu Publish OK ->Clear Mark_Mess_Pending
    sMQTT.Status_u8 = TRUE;   //Set status ve true
       
    switch (sMQTT.MessType_u8)
    {
        case DATA_TSVH:
                sManFlash.IndexSend_u16 = (sManFlash.IndexSend_u16 + sManFlash.CountMessPacket_u16) % MAX_MESS_SAVE;
                Save_Index_Send_Save(ADDR_INDEX_TSVH_SEND, sManFlash.IndexSend_u16);
                
                if (sManFlash.IndexSend_u16 == sManFlash.IndexSave_u16)
                    sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE;  
            
                break;
          case SEND_EVENT_MESS:
                sEventMess.IndexSend_u16 = (sEventMess.IndexSend_u16 + sEventMess.CountMessPacket_u16) % MAX_MESS_EVENT_SAVE;
                Save_Index_Send_Save(ADDR_INDEX_EVENT_SEND, sEventMess.IndexSend_u16);
                
                if (sEventMess.IndexSend_u16 == sEventMess.IndexSave_u16)
                    sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE; 
            
                break;
          case SEND_SAVE_BOX_OK:
                sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE; 
                fevent_active(sEventIdle, EVENT_SAVE_BOX);
                break;
          default:
                sMQTT.aMARK_MESS_PENDING[sMQTT.MessType_u8] = FALSE;  
                break;
    }
            
	sSimVar.GroupStepID_u8 = GR_CLOSE_TCP;
    sSimVar.CountPowerOff_u8 = 0;  //Reset flag poweroff
    sSimVar.LastConnectedServer_u8 = TRUE;  //Connected to server
    
	return 1;
}


uint8_t _Cb_MQTT_CONNECT_0(sData *str_Receiv)
{

	return 1;
}



uint8_t _Cb_AT_CHECK_ATTACH(sData *str_Receive)
{
	char *p;

	p = strstr((char*) str_Receive->Data_a8, "+CGATT: 1");
	if (p != NULL)
	{
		sSimVar.GroupStepID_u8 = GR_SETTING;
        return 1;
	}

	return 0;
}

uint8_t _CB_AT_DIS_URC(sData *str_Receive)
{
	sSimVar.GroupStepID_u8 = GR_INIT_TCP;

	return 1;
}

uint8_t _Cb_AT_Power_Off(sData *str_Receive)
{
	//Set Flag Finish Tx Packet
	sSimVar.IsFinishTransPacket_u8 = TRUE;
	sSimVar.BC66ActiveMode_u8 = MODE_SIM_POWER_OFF;

	return 1;
}

uint8_t _Cb_AT_PSM_Success(sData *str_Receive)
{
	sSimVar.IsFinishTransPacket_u8 = TRUE;
	sSimVar.BC66ActiveMode_u8 = MODE_SIM_DEEPSLEEP;

	return 1;
}


/*========================Func Cb URC BC66========================*/


uint8_t _Cb_URC_ERROR(sData *str_Receive)
{
	//Neu dang cho response AT -> active event timeout
	if (Is_SIM_Waitting_Response() == TRUE)
	{
		PrintDebug(&UART_SERIAL, (uint8_t*) "Response Error!\r\n", 17, 1000);
		fevent_active(sEventSim, EVENT_SIM_AT_SEND_TIMEOUT);
	}

	return 1;
}


uint8_t _Cb_URC_WAKEUP(sData *str_Receive)
{
    sSimVar.IsWakeupBc66_u8 = TRUE;

	return 1;
}


uint8_t aPAY_LOAD[256] = {0};
uint8_t _Cb_RECEIV_SERVER(sData *str_Receive)
{
    //Find Pos AMI/ again
    int     PosFind = 0;
    sData   strCheck = {(uint8_t*) aSimUrc[SIM_URC_RECEIV_SERVER].at_response, strlen(aSimUrc[SIM_URC_RECEIV_SERVER].at_response)};
//    uint8_t aPAY_LOAD[64] = {0};
    sData   sPayload = {&aPAY_LOAD[0], 0};
    uint8_t var = 0;
    //
    PosFind = Find_String_V2(&strCheck, str_Receive);
    if (PosFind >= 0)
    {
        //Convert data from string hex to hex
        UTIL_MEM_set(&aPAY_LOAD[0], 0, sizeof (aPAY_LOAD));
        Convert_String_Hex_To_Hex (str_Receive, PosFind, &sPayload, sizeof (aPAY_LOAD));
        //
        for (var = REQUEST_RESET; var < END_MQTT_RECEI; ++var)
        {
            PosFind = Find_String_V2((sData*) &sMark_MessageRecei_Type[var].sKind, &sPayload);
            
            if ((PosFind >= 0) && (sMark_MessageRecei_Type[var].CallBack != NULL))
                sMark_MessageRecei_Type[var].CallBack(&sPayload, PosFind);
        }
    }

	return 1;
}

 

