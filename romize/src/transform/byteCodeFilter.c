
#include "interpret/instructions.h"
#include "instructionDescs.h"
#include "methodInvokeFilter.h"
#include "byteCodeFilter.h"

#if BYTECODE_FILTERING
                
/* each of these macros evaluates to the replaced opcode instruction length  
 */
#define reassignByteCode(pByteCode, oldInstruction, newInstruction)                     \
    (((PBYTE) pByteCode)[0] = (UINT8) (newInstruction),                                 \
    getInstructionLength(oldInstruction))

#define reassignWideByteCode(pByteCode, oldInstruction, newInstruction)                 \
    (((PBYTE) pByteCode)[0] = NOP,                                                      \
    ((PBYTE) pByteCode)[1] = (newInstruction),                                          \
    getWideInstructionLength(oldInstruction) + 1)

#define SWITCH_INSTRUCTION(pCode, oldInstruction, newInstruction)                       \
        case oldInstruction:                                                            \
            pCode += reassignByteCode(pCode, oldInstruction, newInstruction);           \
            break;

#define SWITCH_WIDE_INSTRUCTION(pCode, oldInstruction, newInstruction)                  \
        case oldInstruction:                                                            \
            pCode += reassignWideByteCode(pCode, oldInstruction, newInstruction);       \
            break;


static void filterMethod(METHOD_DEF pMethod) 
{
    PBYTE pByteCode = pMethod->pCode;
    PBYTE pLastByteCode = pByteCode + pMethod->byteCodeCount - 1;
    
    while(pByteCode <= pLastByteCode) {
        switch(pByteCode[0]) {
            
            
            SWITCH_INSTRUCTION(pByteCode, ACONST_NULL, ACONST_NULL)
            
            

            case INVOKESTATIC:
            case INVOKEVIRTUAL:
            case INVOKESPECIAL:
            case INVOKEINTERFACE: {
                    checkAndSubstituteMethod(pMethod->base.pOwningClass->header.key.namePackageKey, pMethod->base.nameType.nameTypeKey, pByteCode);
                }
                break;

                
            

            case WIDE:
                switch (pByteCode[1]) {
                    SWITCH_WIDE_INSTRUCTION(pByteCode, ALOAD, ALOAD)
                    
                    default:
                        pByteCode += getWideInstructionLength(pByteCode[0]) + 1;
                        break;
                }
                break;
            
                /* the length of the switch instructions need to
                 * be computed at runtime since they are variable length
                 */
            case TABLESWITCH: {
                    INT8 padding = 3 - ((pByteCode - pMethod->pCode) % 4);
                    INT32 low = getU4(pByteCode + (padding + 5));
                    INT32 high = getU4(pByteCode + (padding + 9));
                    pByteCode += padding + (4 * (high - low + 1)) + 13;
                }
                break;

            case LOOKUPSWITCH: {
                    INT8 padding = 3 - ((pByteCode - pMethod->pCode) % 4);
                    INT32 numPairs = getU4(pByteCode + (padding + 5));
                    pByteCode += padding + (8 * numPairs) + 9;
                }    
                break;

            default:
                pByteCode += getInstructionLength(pByteCode[0]);
                break;
            
        }       
    }
    return;
}

static void filterMethodList(LOADED_CLASS_DEF pClassDef, METHOD_LIST pList)
{
    UINT16 i;
    UINT16 count = pList->length;
    METHOD_DEF *ppMethod = pList->ppMethods;
    METHOD_DEF pMethod;

    for(i=0; i<count; i++) {
        pMethod = ppMethod[i];
        if(!isMethodFiltered(pMethod)) {
            filterMethod(pMethod);
            setMethodFiltered(pMethod);
        }
    }
    return;
}

void filterClass(LOADED_CLASS_DEF pClassDef) 
{
    if(getInstanceMethodList(pClassDef) != NULL) {
        filterMethodList(pClassDef, getInstanceMethodList(pClassDef));
    }
    if(getStaticMethodList(pClassDef) != NULL) {
        filterMethodList(pClassDef, getStaticMethodList(pClassDef));
    }
    return;
}

#endif


