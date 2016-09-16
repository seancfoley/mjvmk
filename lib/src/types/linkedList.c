#include "linkedList.h"


/*
 * Will insert a single entry into the list in the proper place
 * pOneToAdd must point to a single list entry
 * comparator(a, b) returns true if a<b, false if a>=b
 * 
 * The list is maintained in ascending order, and when two entries
 * are equal they are listed in FIFO order, ie the first added will be 
 * lower on the list.
 *
 *
 * returns a pointer to the list
 *
 * This is the only lined list function that is not constant time.
 */
void doubleListAddOneSorted(LINKED_LIST *ppDestList, LINKED_LIST pOneToAdd, 
                             BOOLEAN (* comparator)(LINKED_LIST, LINKED_LIST))
{
    LINKED_LIST pTemp;

    if(pOneToAdd == NULL) { /* nothing to add */
        return;
    }
    if(*ppDestList == NULL) { /* just move the list over */
        *ppDestList = pOneToAdd->pNext = pOneToAdd->pPrevious = pOneToAdd;
        return;
    }
    
    pTemp = *ppDestList;

    if(comparator(pOneToAdd, pTemp)) { /* goes to front of list */
        pOneToAdd->pNext = pTemp;
        pOneToAdd->pPrevious = pTemp->pPrevious;
        pTemp->pPrevious = pOneToAdd;
        *ppDestList = pOneToAdd->pPrevious->pNext = pOneToAdd;
    }
    else { /* slot the entry into the appropriate slot */

        /* pTemp is current (first) entry (ie first struct) */
        LINKED_LIST pFirst = pTemp;

        do {
            if(pTemp->pNext == pFirst || comparator(pOneToAdd, pTemp->pNext)) { 
                /* pOneToAdd comes before pTemp->pNext or we're at the list end */
                break;
            }
            pTemp = pTemp->pNext;
        } while(TRUE);
        pOneToAdd->pNext = pTemp->pNext;
        pOneToAdd->pPrevious = pTemp;
        pOneToAdd->pNext->pPrevious = pOneToAdd;
        pTemp->pNext = pOneToAdd;
   }
   return;
}


void doubleListAddOneToEnd(LINKED_LIST *ppDestList, LINKED_LIST pSource)
{
    if(*ppDestList == NULL) { /* just create the one element list */
        *ppDestList = pSource->pNext = pSource->pPrevious = pSource;
    }
    else { /* tack the element onto the  end of the dest list */
        LINKED_LIST pTemp = *ppDestList;

        pSource->pNext = pTemp;
        pSource->pPrevious = pTemp->pPrevious;
        pTemp->pPrevious = pSource;
        pSource->pPrevious->pNext = pSource;
    }
    return;
}


/*
 * Removes an element from the list, we do not know where in the list it is located,
 * or even if it is in the list at all.
 *
 * Returns pOneToRemove.
 */
void doubleListRemoveOne(LINKED_LIST *ppList, LINKED_LIST pOneToRemove)
{
    LINKED_LIST pTemp = pOneToRemove->pNext;

    if(*ppList == pOneToRemove) { /* removing from front of the list */
        if(pOneToRemove == pTemp) {
            *ppList = NULL;
        }
        else {
            *ppList = pOneToRemove->pPrevious->pNext = pTemp;
            pTemp->pPrevious = pOneToRemove->pPrevious;
        }
    }
    else {
        pOneToRemove->pPrevious->pNext = pTemp;
        pTemp->pPrevious = pOneToRemove->pPrevious;
    }
}


/*
 * returns a pointer to the one removed
 */
LINKED_LIST doubleListRemoveOneFromFront(LINKED_LIST *ppList)
{
    LINKED_LIST pRetList = *ppList;

    if(pRetList != NULL) {
        LINKED_LIST pTemp = pRetList->pNext;

        if(pTemp == pRetList) {
            *ppList = NULL;
        }
        else {
            pTemp->pPrevious = pRetList->pPrevious;
            *ppList = pRetList->pPrevious->pNext = pTemp;
        }
    }
    return pRetList;
}


/* 
 * rotates an entry from the front of a list to the end, returns the new entry at the front
 */ 
LINKED_LIST doubleListRotateOneFrontToEnd(LINKED_LIST *ppList)
{
    LINKED_LIST pRemoved = *ppList;
    
    if(pRemoved) {
        return (*ppList = pRemoved->pNext);
    }
    return NULL;
}


/*
 * moves everything from the source list to the end of the dest list
 */
void doubleListMoveAllToEnd(LINKED_LIST *ppDestList, LINKED_LIST *ppSourceList)
{
    LINKED_LIST pSource = *ppSourceList;
    LINKED_LIST pDest = *ppDestList;
        
    if(pSource == NULL) { /* no threads to move */
        return;
    }
    if(pDest == NULL) { /* just move the list over */
        *ppDestList = *ppSourceList;
    }
    else {
        LINKED_LIST pEndOfSourceList = pSource->pPrevious;
        LINKED_LIST pEndOfDestList = pDest->pPrevious;
        
        pEndOfSourceList->pNext = pDest;
        pSource->pPrevious = pEndOfDestList;
        pDest->pPrevious = pEndOfSourceList;
        pEndOfDestList->pNext = pSource;
    }
    *ppSourceList = NULL;
    
}



