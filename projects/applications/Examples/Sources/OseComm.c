
#include <stdio.h>
#include "lib.h"

void OnOseComInfo(int status, int errorsuccess, int progress, const char *info)
{
    switch (status)
    {
    case STAT_GET_TIME_DATE:
        break;
    case STAT_SET_TIME_DATE:
        break;
    case STAT_GET_OS_VERSION:
        break;
    case STAT_GET_APPL_VERSION:
        break;
    case STAT_XMIT_FILE_TO_PC:
        // if successful then delete the file
        if (errorsuccess == SUCC_COMPLETE)
            remove(info);    // Delete the transfered file
        break;
    case STAT_RECV_FILE_FROM_PC:
        break;
    case STAT_LIST_FILES:
        break;
    }
}

void main(void)
{
    for (;;)
    {
        if (IsCharging())
        {
            printf("\nStart OseComm");
            if (OseComm( -1L, CONNECTION_ABORT, "OseComm Example", OnOseComInfo) == COMM_OK)
                printf("\nOseComm Success");
            else
                printf("\nOseComm aborted");
        }
        Idle();
    }
}
