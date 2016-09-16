#ifndef RUN_H
#define RUN_H

#include "thread/thread.h"
#include "interpret/returnCodes.h"


/* We must push the internal frame for clinit, 
 * then push the arguments as they appear in the above structure.
 */
#define initializeClassAtStartUp(pJavaThread, pClassInstance, pStackLimit, failureRoutine) {        \
    FRAME pNewFrame;                                                                                \
                                                                                                    \
    pushFrame(pNewFrame, pInternalClinitMethod, getJavaThreadSP(pJavaThread),                       \
        getJavaThreadFP(pJavaThread), getJavaThreadPC(pJavaThread), pStackLimit, failureRoutine);   \
    setJavaThreadRegisters(pJavaThread, pNewFrame, pInternalClinitMethod);                          \
    pushJavaThreadOperandStackObject(pJavaThread, NULL);                                            \
    pushJavaThreadOperandStackObject(pJavaThread, getClassObjectFromClassInstance(pClassInstance)); \
    pushJavaThreadOperandStackInt(pJavaThread, CLINIT_INITIAL_STAGE);                               \
}


RETURN_CODE initializeVM();


#endif