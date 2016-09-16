

#ifdef INSIDE_INSTRUCTION_LOOP


//TODO a GC opcode would allow for a java thread dedicated to garbage collection which
//would make the THREADED_GC parameter universal


START_INSTRUCTION_HANDLER(FIRST_OPCODE)
    /* we've arrived back at the first opcode, meaning that the thread has terminated */

    deAllocateStack(&getCurrentJavaThread()->pStackBase);

    /* mark the protection being waited on */
    if(getCurrentThread()->pKillProtectionMonitor != NULL) {
        memoryFree(getCurrentThread()->pKillProtectionMonitor);
    }

    //TODO PROCESS MODEL: in the process model, we need to clean up everything related to this thread
    //if it is the final thread in a particular process...
    removeJavaThread();
    
    /* with kernel threads, we will never arrive here */
    generatedRegisterSwitch();
END_HANDLER




START_INSTRUCTION_HANDLER(INIT_OPCODE) /* we've completed executing an init required by the Class.newInstance native method */
    
    if(getResetSP(getFP(), FALSE) != getSP()) { /* something is on the stack, it must be a thrown object */

        /* the spec is a little unclear as far as this method is concerned...*/
        throwException(EXCEPTION_CODE_INSTANTIATION);
        instructionBreak();
    }
    else {
        popCurrentFrame();
        incrementSwitchTimeCounter();
    }
END_HANDLER




/* 
 * Here is the somewhat complex class initialization process as described in 2.17.5 of VM spec.
 *
 * It is necessarily complex due to the numerous locking issues as well as the fact that class
 * initialization is an internal disruption of the instruction execution process.
 *
 */
START_INSTRUCTION_HANDLER(CLINIT_OPCODE)
    /* we are executing a class initialization */

    /* 
     * When arriving here, we know a clinitDataStruct is sitting on the operand stack indicating
     * the state of the class initialization.
     */


    pClinitData = (CLINIT_DATA) (getResetSP(getFP(), FALSE) + 1);
    pClassInstance = getClassInstanceFromClassObject(extractStackFieldObject(pClinitData->classInstanceObject));
    
    LOG_CLASS_REF("executing clinit instruction", pClassInstance->pRepresentativeClassDef->key);
    

    /* Note that if an exception was thrown, the class object instance is above the stack, but it
     * won't be garbage collected because class objects are only explicitly collected.
     */
    switch(extractStackFieldInt(pClinitData->clinitStageCode)) {

        case CLINIT_INITIAL_STAGE:
            LOG_CLASS_REF("initializing class instance", pClassInstance->pRepresentativeClassDef->key);
            if((pMonitor = getClassMonitor(pClassInstance)) == NULL) {
                if((ret = memoryCallocMonitor(getClassMonitorPtr(pClassInstance))) != SUCCESS) {
                    popCurrentFrame();
                    throwErrorAndBreak(ret);
                }
                pMonitor = getClassMonitor(pClassInstance);
            }
            monitorEnter(pMonitor);
#if !PREEMPTIVE_THREADS
            if(monitorNotOwnedByCurrentJavaThread(getClassMonitor(pClassInstance))) {
                insertStackFieldInt(pClinitData->clinitStageCode, CLINIT_ENTERED_MONITOR);
                generatedRegisterSwitch(); /* switch java threads */
                instructionBreak();
            }
#endif
            

        case CLINIT_ENTERED_MONITOR:
            pMonitor = getClassMonitor(pClassInstance);

acquiredMonitor:            
            if(classIsInitialized(pClassInstance, getCurrentJavaThread())) {
                popCurrentFrame();
                exitSynchronizedMonitor(pMonitor);
                instructionBreak();
            }
            else if(classInitializationHasStarted(pClassInstance)) {
                monitorWait(pMonitor, NULL);
                insertStackFieldInt(pClinitData->clinitStageCode, CLINIT_ENTERED_MONITOR);
                generatedRegisterSwitch(); /* switch java threads */
                instructionBreak();
                goto acquiredMonitor; /* we will reach if and only if we have kernel level java thread switching,
                                       * otherwise the above macro will have caused a break out of the switch statement 
                                       */
            }
            else if(classInitializationFailed(pClassInstance)) {
                popCurrentFrame();
                exitSynchronizedMonitor(pMonitor);
                throwErrorAndBreak(ERROR_CODE_NO_CLASS_DEF_FOUND);
            }
            else { /* mark the current thread as the initializing thread */
                setClassInitializingThread(pClassInstance, getCurrentJavaThread());
                exitOrThrowFromBelow(pMonitor);
            }


            pClassDef = getParent(pClassInstance->pRepresentativeClassDef);
            if(pClassDef != NULL) {

                /* initialize the super class if necessary */
                if(!classIsInitialized(getClassInstanceFromClassDef(pClassDef), getCurrentJavaThread())) {
                    insertStackFieldInt(pClinitData->clinitStageCode, CLINIT_COMPLETED_SUPER_CLINIT);
                    initializeClass(getClassInstanceFromClassDef(pClassDef)); /* will break out of the switch here */
                }
            }
            

        case CLINIT_COMPLETED_SUPER_CLINIT:
            if(extractStackFieldObject(pClinitData->thrownThrowableObject) != NULL) {
                /* the super class clinit threw something */
                goto clinitFailed;
            }

            LOG_CLASS_REF("calling class initializer", pClassInstance->pRepresentativeClassDef->key);
            
            /* proceed with initialization */
            if(classHasInitializer(pClassInstance->pRepresentativeClassDef)) {
                insertStackFieldInt(pClinitData->clinitStageCode, CLINIT_COMPLETED_CLINIT);
                pushStackFrame(getClinitMethod(pClassInstance->pRepresentativeClassDef));
                instructionBreak();
            }

        case CLINIT_COMPLETED_CLINIT:
            LOG_CLASS_REF("<clinit> method completed", pClassInstance->pRepresentativeClassDef->key);
            if(extractStackFieldObject(pClinitData->thrownThrowableObject) != NULL) {
                /* the clinit threw something */
                goto clinitFailed;
            }

            /* no need to check the return code since the monitor has been allocated already */
            pMonitor = getClassMonitor(pClassInstance);
            monitorEnter(pMonitor);
#if !PREEMPTIVE_THREADS
            if(monitorNotOwnedByCurrentJavaThread(getClassMonitor(pClassInstance))) {
                insertStackFieldInt(pClinitData->clinitStageCode, CLINIT_ENTERED_MONITOR_UPON_COMPLETION);
                generatedRegisterSwitch(); /* switch java threads */
                instructionBreak();
            }
#endif
            

        case CLINIT_ENTERED_MONITOR_UPON_COMPLETION:
            pMonitor = getClassMonitor(pClassInstance);
            setClassInitializingThread(pClassInstance, NULL);
            setClassInitStatus(pClassInstance, CLASS_INIT_STATUS_INITIALIZED);
            popCurrentFrame();
            if(monitorNotOwnedByCurrentJavaThread(pMonitor)) {
                throwExceptionAndBreak(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
            }
            monitorNotifyAll(pMonitor);
            monitorExit(pMonitor);
            LOG_CLASS_REF("Completed class initialization", pClassInstance->pRepresentativeClassDef->key);
            instructionBreak();

            /* we're done! */


clinitFailed:
        case CLINIT_FAILED:
            /* no need to check the return code since the monitor has been allocated already */
            pMonitor = getClassMonitor(pClassInstance);
            monitorEnter(pMonitor);
#if !PREEMPTIVE_THREADS
            if(monitorNotOwnedByCurrentJavaThread(getClassMonitor(pClassInstance))) {
                insertStackFieldInt(pClinitData->clinitStageCode, CLINIT_ENTERED_MONITOR_UPON_FAILURE);
                generatedRegisterSwitch(); /* switch java threads */
                instructionBreak();
            }
#endif

        case CLINIT_ENTERED_MONITOR_UPON_FAILURE:
            pObject = extractStackFieldObject(pClinitData->thrownThrowableObject);
            pMonitor = getClassMonitor(pClassInstance);
            setClassInitStatus(pClassInstance, CLASS_INIT_STATUS_ERRONEOUS);
            setClassInitializingThread(pClassInstance, NULL);      
            popCurrentFrame();
            if(monitorNotOwnedByCurrentJavaThread(pMonitor)) {
                throwExceptionAndBreak(EXCEPTION_CODE_ILLEGAL_MONITOR_STATE);
            }
            monitorNotifyAll(pMonitor);
            monitorExit(pMonitor);
            if(getClassKey(pObject->header.pClassDef).namePackageKey != javaLangExceptionInInitializerErrorKey.namePackageKey) {
                pObject = getThrowableErrorObject(getCurrentJavaThread(), ERROR_CODE_EXCEPTION_IN_INITIALIZER, getFramePreviousPC(getFP()), getFramePreviousFP(getFP()));
                
                /* if we indeed managed to create an ExceptionInInitializerError, then set its field to the thrown exception */
                if(getClassKey(pObject->header.pClassDef).namePackageKey == javaLangExceptionInInitializerErrorKey.namePackageKey) {
                    insertObjectFieldObject(getExceptionInInitializerErrorInstanceFromObjectInstance(pObject)->throwable, extractStackFieldObject(pClinitData->thrownThrowableObject));
                }
            }
            throwThrowableObject(&pObject, getFP(), getPC());
            instructionBreak();
    }
    /* Note that the breaks above such as instructionBreak only get us to here and not outside the instruction
     * since they are all encased inside a switch statement, but that's OK since there's nothing here anyway 
     */
END_HANDLER





START_INSTRUCTION_HANDLER(IDLE_OPCODE)
#if PREEMPTIVE_THREADS
    idleFunction();
#else
    scheduledRegisterSwitch();
    if(getCurrentThread() != pIdleThread) { 
        instructionBreak(); /* the attempt succeeded, so we vamoose */
    }

    garbageCollect();

    /* since the garbage collection took a bit of time, we try switching out again */
    scheduledRegisterSwitch();
    if(getCurrentThread() != pIdleThread) { 
        instructionBreak(); /* the attempt succeeded, so we vamoose */
    }

    psSleep(); /* see idleFunction for notes regarding the platform specific sleep function */
#endif
END_HANDLER



START_INSTRUCTION_HANDLER(STRING_CHAR_AT_QUICK)
    index = getOperandStackInt(0);
    pObject = getOperandStackObject(1);
    if(pObject == NULL) {
        throwExceptionAndBreak(EXCEPTION_CODE_NULL_POINTER);
    }
    pStringInstance = getStringInstanceFromObjectInstance(pObject);
    value1 = extractPrimitiveFieldInt(pStringInstance->offset);
    if(index < 0 || index >= (INT32) extractPrimitiveFieldInt(pStringInstance->length)) {
        throwExceptionAndBreak(EXCEPTION_CODE_STRING_INDEX_OUT_OF_BOUNDS);
    }
    setOperandStackInt(1, 
        ((CHAR_ARRAY_INSTANCE) extractObjectFieldObject(pStringInstance->pCharArray))->values[value1 + index]);
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER


/* this very quick replacement for the String.charAt() method that does a quick copy without any bounds or null pointer checking */
START_INSTRUCTION_HANDLER(STRING_CHAR_AT_INTERNAL_QUICK)
    setOperandStackInt(1, 
        ((CHAR_ARRAY_INSTANCE) extractObjectFieldObject(getStringInstanceFromObjectInstance(getOperandStackObject(1))->pCharArray))->values[extractPrimitiveFieldInt(getStringInstanceFromObjectInstance(getOperandStackObject(1))->offset) + getOperandStackInt(0)]);
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER

START_INSTRUCTION_HANDLER(VECTOR_ELEMENT_AT_INTERNAL_QUICK)
    setOperandStackObject(1, 
        extractObjectFieldObject(((OBJECT_ARRAY_INSTANCE) extractObjectFieldObject(getVectorInstanceFromObjectInstance(getOperandStackObject(1))->objectArray))->values[getOperandStackInt(0)]));
    decrementOperandStack();
    incrementProgramCounterBy(3);
END_HANDLER






#endif








    

