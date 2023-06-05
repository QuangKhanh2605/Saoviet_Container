/*
    8/2021
    Thu vien xu ly ADC
*/

#ifndef USER_ADC_H
#define USER_ADC_H	

#include "user_util.h"

#if defined (STM32L433xx) 
    #include "stm32l4xx_hal_adc_ex.h"
#elseif defined (STM32L072xx) 
    #include "stm32l0xx_hal_adc_ex.h"
#endif
    
#include "adc.h"

/*===================== Define ===================== */
#if defined (STM32L072xx) || defined (STM32L151xC) 
    #define hadc1               hadc
#endif

/* Internal voltage reference, parameter VREFINT_CAL*/
#if defined (STM32L433xx) 
    #define VREFINT_CAL         ((uint16_t*) ((uint32_t) 0x1FFF75AA)) 
#endif

#if defined (STM32L151xC) 
    #define VREFINT_CAL         ((uint16_t*) ((uint32_t) 0x1FF800F8))  
#endif

#if defined (STM32L433xx) 
    #define VREFINT_CAL         ((uint16_t*) ((uint32_t) 0x1FF80078))
    /* Internal temperature sensor, parameter TS_CAL1: TS ADC raw data acquired at
     *a temperature of 110 DegC (+-5 DegC), VDDA = 3.3 V (+-10 mV). */
    #define TEMP30_CAL_ADDR     ((uint16_t*) ((uint32_t) 0x1FF8007A))
    /* Internal temperature sensor, parameter TS_CAL2: TS ADC raw data acquired at
     *a temperature of  30 DegC (+-5 DegC), VDDA = 3.3 V (+-10 mV). */
    #define TEMP110_CAL_ADDR    ((uint16_t*) ((uint32_t) 0x1FF8007E))
#endif


#define VDDA_VREFINT_CAL        ((uint32_t) 3000)        
#define BAT_CR2032              ((uint32_t) 3000)  

#define LEVEL_MAX_BAT           100

/*===================== Struct var ===================== */
typedef struct
{
    uint32_t    mVol_u32;
    uint16_t    Level_u16;
}Struct_Battery_Status;

extern uint8_t      AdcInitialized;
/*===================== Extern struct var ===================== */


/*===================== Funcion ===================== */
uint16_t        HW_AdcReadChannel(uint32_t Channel);
uint32_t        HW_GetBatteryLevel (void);
uint32_t        Get_Value_ADC (uint32_t Channel);
void            ADC_Desequence_Powerhungry_Channels (void);
void            STM32_MX_ADC1_Init(void);
uint16_t        Get_Level_Voltage (uint32_t Vol_mV, uint32_t VMAX, uint32_t VMIN);


#endif  


