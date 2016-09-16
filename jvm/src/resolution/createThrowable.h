#ifndef CREATETHROWABLE_H
#define CREATETHROWABLE_H

#include "elements/base.h"
#include "interpret/returnCodes.h"
#include "frame/throw.h"

OBJECT_INSTANCE getThrowableErrorObject(JAVA_THREAD pThread, RETURN_CODE returnCode, INSTRUCTION pc, FRAME fp);
OBJECT_INSTANCE getThrowableExceptionObject(JAVA_THREAD pThread, EXCEPTION_CODE exceptionCode, INSTRUCTION pc, FRAME fp);

#endif
