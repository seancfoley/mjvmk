
#include "elements/base.h"
#include "interpret/returnCodes.h"
#include "string/javaString.h"
#include "string/stringPool.h"
#include "class/table.h"
#include "resolution/file/fileAccess.h"
#include "resolution/file/memClassFile.h"
#include "resolution/load/preload.h"
#include "resolution/load/load.h"
#include "natives/initializeNatives.h"
#include "thread/schedule.h"
#include "thread/thread.h"
#include "run.h"

struct tester *pTester1;

int test(int argc, char *argv[])
{
    RETURN_CODE ret;
    

    ret = initializeVM();
    if(ret != SUCCESS) {
        return -1;
    }

    /* tests go here */

    return ret;
}


