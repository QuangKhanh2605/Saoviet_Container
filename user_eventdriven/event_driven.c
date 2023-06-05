/*
 * event_driven.c
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */

#include "gpio.h"
#include "event_driven.h"


/*
 * Func: Active 1 event lên.
 *
 * Note: cho thực hiện luôn
 * */

uint8_t fevent_active(sEvent_struct *event_struct, uint8_t event_name)
{
	// check input data
	event_struct[event_name].e_status = 1;
	event_struct[event_name].e_systick = 0;

	// push to queue
	return 1;
}

/*
 * Func: Enable 1 event lên.
 *
 * Note: excute event sau khi check Period. (vi set lai systick)
 * */

uint8_t fevent_enable(sEvent_struct *event_struct, uint8_t event_name)
{
	// check input data
	event_struct[event_name].e_status = 1;
	event_struct[event_name].e_systick = HAL_GetTick();

	// push to queue
	return 1;
}

/*
 * Func: Disable 1 event.
 * */

uint8_t fevent_disable(sEvent_struct *event_struct, uint8_t event_name)
{
	// check input data
	event_struct[event_name].e_status = 0;

	// reject cauz status = 0 (don't need status variable)
	return 1;
}

