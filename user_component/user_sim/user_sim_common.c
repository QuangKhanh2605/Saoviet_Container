

#include "user_sim_common.h"

#include "user_string.h"
#include "user_time.h"

/*=========== Var =====================*/
sFuncCallbackHandlerSim *sSimCommFuncCallBack = NULL;

/*=========== Buff constain Information====================*/
uint8_t     aSIM_ID[SIM_ID_LENGTH] = {"123456789"};
uint8_t     aSERIAL_MODULESIM[SIM_ID_LENGTH];
uint8_t	    aAPN[BUFF_LENGTH_SETUP] = "SV123";  
uint8_t	    aAPN_DIAL_STRING[BUFF_LENGTH_SETUP] = "12345";
uint8_t	    aAPN_USERNAME[BUFF_LENGTH_SETUP] = "xyz";  
uint8_t	    aAPN_PASSWORD[BUFF_LENGTH_SETUP] = "1111";

/*=== Server Infor Buffer*/
uint8_t     aIP_SERVER_MAIN[MAX_IP_LENGTH]  = "124.158.5.154";  //124.158.5.154 port 1883
uint8_t     aPORT_SERVER_MAIN[MAX_PORT_LENGTH] = "1883";
uint8_t     aUSER_MQTT_MAIN[MAX_USER_PASS_LENGTH] = "syswmsv";
uint8_t     aPASS_MQTT_MAIN[MAX_USER_PASS_LENGTH] = "sv@wmsv568";

uint8_t     aIP_SERVER_BACKUP[MAX_IP_LENGTH] = "124.158.5.154";  
uint8_t     aPORT_SERVER_BACKUP[MAX_PORT_LENGTH] = "7525";
uint8_t     aUSER_MQTT_BACKUP[MAX_USER_PASS_LENGTH] = "syswmsv";
uint8_t     aPASS_MQTT_BACKUP[MAX_USER_PASS_LENGTH] = "sv@wmsv568";


//Buff FTP file
uint8_t		aFIRM_IP[LENGTH_FTP]={"124.158.5.154"};
uint8_t		aFIRM_PORT[LENGTH_FTP]={"66"};
uint8_t		aFIRM_PATH[LENGTH_FTP] = {'T','e', 's', 't'};
uint8_t		aFIRM_NAME[LENGTH_FTP] = {'F', 'I', 'R', 'M', '_', 'H', 'C', 'M', 'C', 'H', 'E', 'S', '2', '.','b', 'i', 'n'};
uint8_t	    aFIRM_HTTP_URL[LENGTH_HTTP_URL]={"http://172.16.11.141:8909/FIRM_HCMCHES1_4.bin"};


/*============== Struct var ==========*/

Struct_Sim_Information	sSimInfor = 
{
    .sSIMid = {&aSIM_ID[0], 14}, 
    .RSSI_u8 = 0,
    .Ber_u8  = 0,
    .sImei   = {&aSERIAL_MODULESIM[0], 0},
    
    .sAPN    = {&aAPN[0], 5},
    .sAPNDialString = {&aAPN_DIAL_STRING[0], 5},
    .sAPNUsername   = {&aAPN_USERNAME[0], 3},
    .sAPNPassword   = {&aAPN_PASSWORD[0], 4},
    
    //Server Infor
    .sServer.sIPMain            = {&aIP_SERVER_MAIN[0], 13},
    .sServer.sPortMain          = {&aPORT_SERVER_MAIN[0], 4},
    .sServer.sUserNameMQTTMain  = {&aUSER_MQTT_MAIN[0], 7},
    .sServer.sPasswordMQTTMain  = {&aPASS_MQTT_MAIN[0], 10},
    
    .sServer.sIPBackUp            = {&aIP_SERVER_BACKUP[0], 24},
    .sServer.sPortBackUp          = {&aPORT_SERVER_BACKUP[0], 4},
    .sServer.sUserNameMQTTBackUp  = {&aUSER_MQTT_BACKUP[0], 7},
    .sServer.sPasswordMQTTBackUp  = {&aPASS_MQTT_BACKUP[0], 10},
    
    .sServer.KeepAlive_u32  = 60,
};


Struct_Sim_Variable	    sSimCommon;


StructSimUpdateFirmware    sSimFwUpdate = 
{
    .StrURL_FirmIP      = {&aFIRM_IP[0], 13},
    .StrURL_FirmPort    = {&aFIRM_PORT[0], 2},
    .StrURL_FirmPath    = {&aFIRM_PATH[0], 4},
    .StrURL_FirmName    = {&aFIRM_NAME[0], 17},
    .StrURL_HTTP        = {&aFIRM_HTTP_URL[0], 50},
};

SFileSystemInfor         sFileSys = 
{
    .aNAME = {"/SVWM_Firmware.bin"},
    .LengthName_u8 = 18,
    
    .strHandle = {&sFileSys.aTEMP_STR_HANDLE[0], 0},
    .Handle_u32 = 1,
};


/*=========== Function =====================*/

/*
    Func: Get Serial SIM card 
*/
void Sim_Common_Get_Sim_ID (sData *strSoure)
{
	uint16_t 		Pos,var;
	uint8_t			aRes_Data[SIM_ID_LENGTH] = {0};
    sData	        sRes = {&aRes_Data[0], 0};
    uint8_t         MarkCompare = 0;    
    
	for (Pos = 1; Pos < strSoure->Length_u16; ++Pos) 
		if ((strSoure->Data_a8[Pos-1] == '8') && (strSoure->Data_a8[Pos] == '9')) 
            break;
    
	for (var = (Pos-1); var < strSoure->Length_u16; var++) 
		if ((strSoure->Data_a8[var] < '0') || (strSoure->Data_a8[var] > '9')) 
            break;
    
	sRes.Data_a8 = &strSoure->Data_a8[Pos-1];
	sRes.Length_u16 = var - Pos + 1;
    
	if (sRes.Length_u16 > SIM_ID_LENGTH) 
        sRes.Length_u16 = SIM_ID_LENGTH;
    
	if (sRes.Length_u16 != sSimInfor.sSIMid.Length_u16)
    { 
        MarkCompare = 1;
	} else
    {
        //Compare new Sim to Mark Send Sim ID
        for (var = 0; var < sRes.Length_u16; ++var) 
        {
            if (sSimInfor.sSIMid.Data_a8[var] != sRes.Data_a8[var])
            {
                MarkCompare = 1;   //Co su khac nhau
                break;
            }
        } 
    }
    //If Mark == 1: Is new sim ID
    if (MarkCompare == 1)
    {
        for (var = 0; var < sRes.Length_u16; ++var) 
            sSimInfor.sSIMid.Data_a8[var] = sRes.Data_a8[var];

        sSimInfor.sSIMid.Length_u16 = sRes.Length_u16;
        //Call Func Handler from SIM
        
        if (sSimCommFuncCallBack->pSim_Common_Handler_AT != NULL)
            sSimCommFuncCallBack->pSim_Common_Handler_AT (_SIM_COMM_EVENT_SIM_CARD);
    }
}

/*
    Func: Get CSQ
        + Convert to Dec (RSSI)
        + Convert to %
*/

void Sim_Common_Get_CSQ (sData *strCheck, sData *strSoure)
{
	int Pos_Str = -1;
        
	/* ghi lai RSSI va BER */
	Pos_Str = Find_String_V2(strCheck, strSoure);
	if (Pos_Str >= 0) 
    {
        Pos_Str += strCheck->Length_u16;
		sSimInfor.RSSI_u8 = *(strSoure->Data_a8 + Pos_Str) - 0x30;
		sSimInfor.Ber_u8  = *(strSoure->Data_a8 + Pos_Str + 2) - 0x30;  
		if (*(strSoure->Data_a8 + Pos_Str + 2) == 0x2C) 
        {
			sSimInfor.RSSI_u8 = ((*(strSoure->Data_a8 + Pos_Str))<<4) + \
															 ((*(strSoure->Data_a8 + Pos_Str + 1)) & 0x0F);
			sSimInfor.Ber_u8 = *(strSoure->Data_a8 + Pos_Str + 3) - 0x30;           // ber 0->7
		}
	}
	/* convert signal strength */
	switch (sSimInfor.RSSI_u8) 
    {
		case 0x00:
			sSimInfor.RSSI_u8 = 113;
			break;
		case 0x01:
			sSimInfor.RSSI_u8 = 111;	
			break;
		case 0x31:
			sSimInfor.RSSI_u8 = 51;
			break;
        case 0x99:
			sSimInfor.RSSI_u8 = 0;
			break;
		default: //2 den 30/ 2 per step: 0x19 ->19  sau do - 2 = 17
			sSimInfor.RSSI_u8 = 113 - ( ( (sSimInfor.RSSI_u8) / 16 ) * 10 + sSimInfor.RSSI_u8 % 16 ) * 2;
			break;
	}
}


/*
    Func: Get Imei Module SIM
*/

void Sim_Common_Get_Imei(sData *strSoure)
{
    uint16_t i = 0;
    
    for (i = 0; i < strSoure->Length_u16; i++)
        if ((*(strSoure->Data_a8 + i) >= 0x30) && (*(strSoure->Data_a8 + i) <= 0x39)) 
            break;
    
    if (i < strSoure->Length_u16)
    {
        Reset_Buff(&sSimInfor.sImei);
        while ((*(strSoure->Data_a8 + i) >= 0x30) && (*(strSoure->Data_a8 + i) <= 0x39)) 
        {
            *(sSimInfor.sImei.Data_a8 + sSimInfor.sImei.Length_u16++) = *(strSoure->Data_a8 + i);
            i++;
            if (sSimInfor.sImei.Length_u16 >= SIM_ID_LENGTH) 
                break;
        }
    }
}


/*
    Func: Get sTime from BTS
*/

void Sim_Common_Get_Stime_BTS (sData *strCheck, sData *strSoure)
{
	int             Pos_Fix = 0;
	uint16_t        Temp_Get = 0;
    uint16_t        Temp_year = 0;
    uint8_t         count = 0;
    uint8_t         aRTCtemp[8] = {0};

	Pos_Fix = Find_String_V2(strCheck, strSoure);
    
    if (strSoure->Length_u16 > (Pos_Fix + strCheck->Length_u16 + 14))
	{
        Pos_Fix += strCheck->Length_u16 + 1;   //dau khoang trong va nhay ++ de tro vao year
        
        //tim vi tri bat dau so decima
        Temp_year = *(strSoure->Data_a8 + Pos_Fix);
        while ((0x30 > Temp_year) || (Temp_year > 0x39))
        {
            Pos_Fix++;
            Temp_year = *(strSoure->Data_a8 + Pos_Fix);
        }
        
        //bat dau vi tri co so: year month date hour min sec
        while (count < 6)
        {
            Temp_Get  = 0;
            Temp_year = *(strSoure->Data_a8 + Pos_Fix);
            while ((0x30 <=  Temp_year) && (Temp_year <= 0x39))
            {
                Temp_Get = Temp_Get* 10 + Temp_year - 0x30;
                Pos_Fix++;
                Temp_year = *(strSoure->Data_a8 + Pos_Fix);
            }
                    
            aRTCtemp[count++] = Temp_Get;               //nam chi lay 2 chu so sau
            
            if (Pos_Fix >= strSoure->Length_u16) 
                return;
            
            Pos_Fix++; // dau /
        }
        
        sSimInfor.sTime.year   = aRTCtemp[0] % 100;
        sSimInfor.sTime.month  = aRTCtemp[1];
        sSimInfor.sTime.date   = aRTCtemp[2];
        sSimInfor.sTime.hour   = aRTCtemp[3];  
        sSimInfor.sTime.min    = aRTCtemp[4];
        sSimInfor.sTime.sec    = aRTCtemp[5];
        
        if (Check_update_Time (&sSimInfor.sTime) == 0)
            return;

        //Convert lai day. 1/1/2012 la chu nhat. Thu 2 - cn: 2-8
        sSimInfor.sTime.day = ((HW_RTC_GetCalendarValue_Second (sSimInfor.sTime, 1) / SECONDS_IN_1DAY) + 6) % 7 + 1;
       
        sSimCommFuncCallBack->pSim_Common_Handler_AT(_SIM_COMM_EVENT_GET_STIME);
    }
}


/*================================ Func HTTP ===========================*/

uint8_t Sim_Common_Http_Read_Data (sData *uart_string)
{
    int             Pos = 0;
    sData           strHeader      = {(uint8_t*)"$HTTPREAD:", 10};
    sData           strStartHttp   = {(uint8_t*)"\r\n", 2};
    sData           StrHeaderSVFirm = {(uint8_t*)"ModemFirmware:", 14};
    
    sData           strFix;
    uint16_t        i = 0, j = 0;
    uint8_t         aNumbyte[10] = {0};
    uint8_t         lengthnum = 0;
    uint16_t        NumbyteRecei = 0;
    uint8_t         aTEMP_DATA[8] = {0};
                   
    Pos = Find_String_V2((sData*) &strHeader, uart_string);
	if (Pos >= 0)
	{
        //Get Number recv string
        for (i = Pos; i < uart_string->Length_u16; i++)
        {
            if (*(uart_string->Data_a8 + i) == ',')
            {
                i++;
                while ((*(uart_string->Data_a8 + i) >= 0x30) && (*(uart_string->Data_a8 + i) <= 0x39))
                {
                    aNumbyte[lengthnum++] = *(uart_string->Data_a8 + i) - 0x30;
                    i++;
                }
                
                break;
            }
        }
        //convert ra so byte nhan dc
        for (i = 0; i < lengthnum; i++)
             NumbyteRecei = NumbyteRecei *10 + aNumbyte[i];
        
        if (NumbyteRecei == 0) 
        {
            sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
            return 0;
        }
        
        strFix.Data_a8      = uart_string->Data_a8 + Pos;
        strFix.Length_u16   = uart_string->Length_u16 - Pos;
        
        Pos = Find_String_V2((sData*)&strStartHttp, &strFix);  //check vi tri bat dau cua toan bo data http
        if (Pos >= 0)
        {   
            Pos  += strStartHttp.Length_u16;
            //
            strFix.Data_a8 = strFix.Data_a8 + Pos;
            if (NumbyteRecei > (strFix.Length_u16 - Pos - 6))
            {
                sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                return 0;
            }
            strFix.Length_u16 = NumbyteRecei;
            //
            sSimFwUpdate.CountByteTotal_u32 += NumbyteRecei;                          // tong so data ma ban tin nhan doc ra cua http
            sSimFwUpdate.AddOffset_u32 = sSimFwUpdate.CountByteTotal_u32;
            //Neu la Pack dau tien, Check 32 byte header xem co chu: ModemFirmware:SV1.1,0x0800C000
            if (sSimFwUpdate.CountPacket_u8 == 0)
            {
                Pos = Find_String_V2((sData*)&StrHeaderSVFirm, &strFix);  
                if (Pos >= 0)
                    sSimFwUpdate.IsFirmSaoViet_u8 = 1;
                //Lay byte cuoi cung ra: byte 32 cua Packdau tien. Sau header 31 byte
                sSimFwUpdate.LastCrcFile_u8 = *(strFix.Data_a8 + 31);  
                //Check Get Fw Sao Viet
                if (sSimFwUpdate.IsFirmSaoViet_u8 == 0)    //neu khong co header SV code thi return luon
                {
                    sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                    return 0;
                }
                //Fix string data
                strFix.Data_a8 += 32;
                strFix.Length_u16 -= 32;
            }
        
            //Cacal Crc
            for (i = 0; i < strFix.Length_u16; i++)
                sSimFwUpdate.LasCrcCal_u8 += *(strFix.Data_a8 + i);
                      
            //Increase Count Packet
            sSimFwUpdate.CountPacket_u8++;
            
            //Ghi data vao. Bu bao cho thanh boi cua FLASH_BYTE_WRTIE
            if ((strFix.Length_u16 % FLASH_BYTE_WRTIE) != 0)
                strFix.Length_u16 += ( FLASH_BYTE_WRTIE - (strFix.Length_u16 % FLASH_BYTE_WRTIE) );
            //write fw to Mem: Onchip flash or External mem
            for (i = 0; i < (strFix.Length_u16 / FLASH_BYTE_WRTIE); i++)
            {
                //Phai chia ra buffer rieng moi k loi
                for (j = 0; j < FLASH_BYTE_WRTIE; j++)
                    aTEMP_DATA[j] = *(strFix.Data_a8 + i * FLASH_BYTE_WRTIE + j); 
                //
                if(OnchipFlashWriteData(sSimFwUpdate.AddSave_u32, &aTEMP_DATA[0], FLASH_BYTE_WRTIE) != HAL_OK)
                {
                    sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                    return 0;
                } else
                    sSimFwUpdate.AddSave_u32 += FLASH_BYTE_WRTIE;
            }          
                 
            //kiem tra xem nhan du byte chua de bao ok
            if (sSimFwUpdate.CountByteTotal_u32 >= sSimFwUpdate.FirmLength_u32)  //Binh thuong la dau == //Test de >=           
            {
                if (sSimFwUpdate.LastCrcFile_u8 == sSimFwUpdate.LasCrcCal_u8)
                {
                    sSimFwUpdate.CountByteTotal_u32 += (FLASH_BYTE_WRTIE - sSimFwUpdate.CountByteTotal_u32 % FLASH_BYTE_WRTIE);
                    
                    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_OK );
                    return 1;
                } else
                {
                    sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
                    return 0;
                }
            }
            
            //Continue
            sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_READ_OK );
            return 1;
        }
    }
    //Set Fail Setup Http
    sSimFwUpdate.UpdateFailStatus_u8 = SEND_UPDATE_FIRMWARE_FAIL_SETUP;
    sSimCommFuncCallBack->pSim_Common_Handler_AT ( _SIM_COMM_EVENT_HTTP_UPDATE_FAIL ); 
    
    return 0;
}




/*================================ Func URC ===========================*/
/*
    Func: Call Handler URC Reset SIM
*/
void Sim_Common_URC_Reset (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_RESET_SIM);
}

/*
    Func: Call Handler URC LOST SIM
*/
void Sim_Common_URC_Lost_Card (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_SIM_LOST);
}

/*
    Func: Call Handler URC LOST SIM
*/
void Sim_Common_URC_Closed (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_CLOSED);  
}


/*
    Func: Call Handler URC Error
*/
void Sim_Common_URC_Error (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_ERROR);
}


/*
    Func: Call Handler URC Receive server
*/
void Sim_Common_URC_Recv_Server (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_RECEIV_SERVER);
}

/*
    Func: Call Handler URC calling
*/
void Sim_Common_URC_Calling (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_CALLING);   
}

void Sim_Common_URC_Sms_Content (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_SMS_CONTENT);
}

void Sim_Common_URC_Sms_Indication (void)
{
    sSimCommFuncCallBack->pSim_Common_Handler_URC(_SIM_COMM_URC_SMS_IND);
}



/*================================ Func Process ===========================*/


/*
    Func: Config 2 var to Erase
        + Add new Fw
        + MaxSize Page new FW

        - 2 information get by App Config
*/

void Sim_Common_Config_UpdateFw (uint32_t AddNewFw, uint16_t MAX_SIZE)
{
    sSimFwUpdate.AddSave_u32					= AddNewFw;
    sSimFwUpdate.MaxPage_u16					= MAX_SIZE;
}

/*
    Func: Init Struct Update Fw variable
*/

void Sim_Common_Init_Struct_UpdateFw (void)
{
	sSimFwUpdate.CountByteTotal_u32 			= 0;
	sSimFwUpdate.Pending_u8 					= 0;
    //
    sSimFwUpdate.AddOffset_u32                  = 0;
    sSimFwUpdate.CountPacket_u8                 = 0;
    sSimFwUpdate.LastCrcFile_u8                 = 0;
    sSimFwUpdate.LasCrcCal_u8                   = 0;
    sSimFwUpdate.IsFirmSaoViet_u8               = FALSE;
}

/*
    Func: Init Update fw before send AT+HTTP Read
        + Init Struct var
        + Erase area New Fw
*/

void Sim_Common_Init_UpdateFw (void)
{    
    sSimCommFuncCallBack->pSim_Common_Config_Update();
        
    Sim_Common_Init_Struct_UpdateFw();
    Sim_Common_Init_Struct_FileSys();
    //Erase new firmware area
    Erase_Firmware(sSimFwUpdate.AddSave_u32, sSimFwUpdate.MaxPage_u16); 
}


/*
    Init:
        + Group begin Pree Init
        + Count Soft = 0
        + Finish = FALSE
        ...
*/
void Sim_Common_Init_Var_Default (void)
{
	sSimCommon.GroupStepID_u8           = _GR_PREE_INIT;
    sSimCommon.CallReady_u8             = FALSE;

    sSimCommon.PowerStatus_u8           = _POWER_INIT;
    
    sSimCommon.RxPinReady_u8            = FALSE;
    sSimCommon.TxPinReady_u8            = FALSE;
    sSimCommon.NetReady_u8              = FALSE;
    sSimCommon.ServerReady_u8           = FALSE;
}



//
void Sim_Common_Send_AT_Cmd (UART_HandleTypeDef *huart, uint8_t *string, uint16_t length, uint32_t TimeOut) 
{
    UTIL_Printf( DBLEVEL_M, string, length );
	HAL_UART_Transmit(huart, string, length, TimeOut);
}




void Sim_Common_Init_Struct_FileSys (void)
{
    sFileSys.Handle_u32 = 1;
    UTIL_MEM_set ( sFileSys.aTEMP_STR_HANDLE, 0, sizeof (sFileSys.aTEMP_STR_HANDLE) );
    sFileSys.strHandle.Length_u16 = 0;
}




void Sim_Common_Default_Struct_GPS (void)
{
    sSimCommon.sGPS.Status_u8   = FALSE;
//    sSimCommon.sGPS.LengData_u8 = 0;
//    
//    UTIL_MEM_set( sSimCommon.sGPS.aPOS_INFOR, 0, sizeof (sSimCommon.sGPS.aPOS_INFOR) );
}


