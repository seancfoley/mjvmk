#ifndef METHODINVOKEFILTER_H
#define METHODINVOKEFILTER_H


#include "elements/base.h"
#include "interpret/returnCodes.h"


RETURN_CODE initializeFilteredMethods();

/* do not use this method, use the macro below */
BOOLEAN substituteMethodInvoke(NamePackageKey classKey, NameTypeKey methodKey, INSTRUCTION instruction);

/* bitwise "or" of all class keys corresponding to classes with filtered methods */
extern NamePackageKey classesOrFilter;
extern NamePackageKey classesAndFilter;

/* use this macro to do method to bytecode substitutions, it "returns" TRUE if a substitution was made */
#define checkAndSubstituteMethod(namePackageKey, methodKey, instruction)    \
    ((classesOrFilter & namePackageKey) == namePackageKey                   \
     && (classesAndFilter | namePackageKey) == namePackageKey               \
     && substituteMethodInvoke(namePackageKey, methodKey, instruction))                 



#endif

