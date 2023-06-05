#ifndef CUSTOMER_
#define CUSTOMER_	CUSTOMER_H

#include "main.h"



/*     Chu ki wake up
      - Chu ki nay se duoc set timer danh thuc MCU day.
      - Don vi (s)
*/
#define DEFAULT_FREQ_WAKEUP                             2     //don vi min

/* So lan thiet bi day roi moi gui ban tin */    //tu thong so nay va chu ki wakeup se tinh ra duoc chu ki cua ban tin
#define DEFAULT_NUMBER_WAKE_UP                          1

#define MAX_NUMBER_WAKE_UP                              10
#define MAX_NUNBER_SEND_INIT                            1
#define MAX_MESS_IN_PACKET                              1       //1 ban tin 31 byte. Max 512 byte - 45 byte header

/*--- ID cua thiet bi -------*/
#define DEVICE_ID                                       "SVTHGS23000001"  


#define DEFAULT_FREQ_SEND_ONLINE                        2







#endif
//End
