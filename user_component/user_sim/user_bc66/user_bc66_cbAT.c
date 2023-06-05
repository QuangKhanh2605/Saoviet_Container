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
	fClearSimStepQueue();
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
				fPushBlockSimStepToQueue(&aSIM_STEP_CONTROL[10], sizeof(aSIM_STEP_CONTROL) - 10);
				break;
			case GR_CLOSE_TCP:
				sSimVar.CountSoftReset_u8 = MAX_SOFT_RESET;
				fevent_active(sEventSim, EVENT_SIM_HARD_RESET);
				break;
			default:
				break;
		}
	} else
	{
        sSimVar.CountHardReset_u8++;
        if (sSimVar.CountHardReset_u8 < MAX_HARD_RESET)   
            fevent_active(sEventSim, EVENT_SIM_HARD_RESET);  //Reset bằng chân phần cứng + Power off
        else
        {
            sSimVar.IsFinishTransPacket_u8 = TRUE;
            sSimVar.BC66ActiveMode_u8 = MODE_SIM_POWER_OFF;
        }
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
	sSimVar.SimInserted_u8 	= TRUE;

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
			break;
		case 0x01:
			sSimInfor.RSSI_u8 = 111;
			break;
		case 0x31:
			sSimInfor.RSSI_u8 = 52;
			break;
        case 0x99:
        	sSimInfor.RSSI_u8 = 0;
			break;
		default:
          //công thuc này can xem lai
			sSimInfor.RSSI_u8 = 114 - ((sSimInfor.RSSI_u8 / 16) * 10 + sSimInfor.RSSI_u8 % 16) * 2;
			break;
	}

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
//ST_TIME_FORMAT  sRTC_Get;
uint8_t _Cb_AT_GET_CLOCK(sData *str_Receiv)
{
	char			*Pos;
	uint16_t        Temp_Get = 0;
	uint16_t        Temp_year = 0;
	ST_TIME_FORMAT  sRTC_Get = {0};
	uint8_t         count = 0;
	uint8_t         aRTCtemp[8];
    
	sSimVar.GroupStepID_u8 = GR_SEND_MQTT;
	Pos = strstr ((char*) str_Receiv->Data_a8, "+CCLK:");

	if ((Pos != NULL) && (strlen (Pos) >= (14 + strlen(aSimStep[_AT_GET_RTC].at_response) - 1)))
	{
		Pos += strlen(aSimStep[_AT_GET_RTC].at_response) - 1 + 1;   //dau khoang trong va nhay ++ de tro vao year

		//tim vi tri bat dau so decima
		Temp_year = *(Pos);
		while ((0x30 > Temp_year) || (Temp_year > 0x39))
		{
			Pos++;
			Temp_year = *(Pos);
		}

		//bat dau vi tri co so: year month date hour min sec
		while (count < 6)
		{
			Temp_Get  = 0;
			Temp_year = *(Pos);
			while ((0x30 <=  Temp_year) && (Temp_year <= 0x39))
			{
				Temp_Get = Temp_Get* 10 + Temp_year - 0x30;
				Pos++;
				Temp_year = *(Pos);
			}

			aRTCtemp[count++] = Temp_Get % 100;               //nam chi lay 2 chu so sau

			if (Pos == NULL)
			{
				HAL_UART_Transmit(&UART_SIM, (uint8_t *) "Qua buff\r\n", 10, 1000);
				break;  //return 1;
			}

			Pos++;
		}
		sRTC_Get.year   = aRTCtemp[0];
		sRTC_Get.month  = aRTCtemp[1];
		sRTC_Get.date   = aRTCtemp[2];
		sRTC_Get.hour   = aRTCtemp[3];
		sRTC_Get.min    = aRTCtemp[4];
		sRTC_Get.sec    = aRTCtemp[5];
		if (sRTC_Get.year < 20)
			return 1;

		Convert_sTime_ToGMT(&sRTC_Get, 7);  //Do thoi gian da la gio dia phuong roi. Co nhung module sim thoi gian la gio GMT0
		//Convert lai day. 1/1/2012 la chu nhat. Thu 2 - cn: 2-8
		sRTC_Get.day = ((HW_RTC_GetCalendarValue_Second(sRTC_Get, 1) / SECONDS_IN_1DAY) + 6) % 7 + 1;
		UTIL_Set_RTC(sRTC_Get);
	}

	return 1;
}


uint8_t _Cb_AT_TCP_CONNECT_1(sData *str_Receiv)
{
	uint8_t aTemp[40] = {0};
	sData   Protocol = {(uint8_t*) "\"TCP\",\"", 7};
	uint8_t length = 0, i = 0;

	//them pay load vao day
	aTemp[length++] = '1' ;            //cortex
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
	aTemp[length++] = ',';  //direct mode
	aTemp[length++] = '0';

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
	char *strMQTTConnect = "AT+QISENDEX=0,42,102800044D51545404C2003C000E5356574D4E423230303030313432000561646D696E000561646D696E";

	HAL_UART_Transmit(&UART_SIM, (uint8_t *) strMQTTConnect, strlen (strMQTTConnect), 1000);
	return 1;
}

uint8_t _Cb_MQTT_CONNECT_2(sData *str_Receiv)
{

	return 1;
}
uint8_t _Cb_MQTT_SUBCRIBE_1(sData *str_Receiv)
{
	char *strMQTTSubcribe = "AT+QISENDEX=0,39,8225000A0014414D492F5356574D4E4232303030303134322F23000009414D492F7354696D6500";

	HAL_UART_Transmit(&UART_SIM, (uint8_t *) strMQTTSubcribe, strlen (strMQTTSubcribe), 1000);
	return 1;
}

uint8_t _Cb_MQTT_SUBCRIBE_2(sData *str_Receiv)
{

	return 1;
}

uint8_t _Cb_MQTT_PUBLISH_1(sData *str_Receiv)
{
	char *strMQTTPublish = "AT+QISENDEX=0,62,303C00224443552F574D53562F574D45312F53696D49442F5356574D4E423230303030313432011438393834303438303030303639303537393936380017";

	HAL_UART_Transmit(&UART_SIM, (uint8_t *) strMQTTPublish, strlen (strMQTTPublish), 1000);
	return 1;
}

uint8_t _Cb_MQTT_PUBLISH_2(sData *str_Receiv)
{
	sSimVar.GroupStepID_u8 = GR_CLOSE_TCP;

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






