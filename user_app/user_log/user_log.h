
#ifndef USER_LOG_H_
#define USER_LOG_H_

#include "user_util.h"

/*===========Var struct=============*/
typedef enum
{
    _RQ_RECORD_TSVH,
    _RQ_RECORD_EVENT,
    _RQ_RECORD_LOG,
    _RQ_RECORD_GPS,
}sTypeRequestReadOldRecord;

/*===========Extern Var struct=============*/


/*==================Function==================*/

uint8_t LOG_Save_Record (uint8_t *pData, uint16_t Length);






#endif /* USER_UART_H_ */
