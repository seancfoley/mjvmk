#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "psTypes.h"

/*
 * It is assumed that all lists begin with a pointer to a linkedListStruct and
 * not with a linkedListStruct itself.  When passing in an entire list to any
 * of these functions in the API, pass in the address of this pointer.
 *
 * The linked lists are circular, meaning that the pointer to the list will point
 * to the first entry on the list.  The first entry on the list will point forward
 * to the next entry and backwards to the previous entry.  The advantage of the circular
 * doubly linked list is that adding/removing from the front/back of the list are all 
 * constant time operations.
 *
 * The only operation that is not constant time is the doubleListAddOneSorted operation.
 */


typedef struct linkedListStruct linkedListStruct, *LINKED_LIST;

struct linkedListStruct {
    UINT32 data;
    LINKED_LIST pNext;
    LINKED_LIST pPrevious; /* ignored in singly linked lists */
};



/* functions for circular doubly linked lists */

void doubleListAddOneSorted(LINKED_LIST *ppDestList, LINKED_LIST pOneToAdd, BOOLEAN (* comparator)(LINKED_LIST, LINKED_LIST));
LINKED_LIST doubleListRemoveOneFromFront(LINKED_LIST *ppList);
void doubleListMoveAllToEnd(LINKED_LIST *ppDestList, LINKED_LIST *ppSourceList);
void doubleListAddOneToEnd(LINKED_LIST *ppDestList, LINKED_LIST pSource);
LINKED_LIST doubleListRotateOneFrontToEnd(LINKED_LIST *ppList);

/* when calling this you should be certain that that what you're removing is actually on the list */
void doubleListRemoveOne(LINKED_LIST *ppList, LINKED_LIST pOneToRemove);

#define doubleListMoveOneFrontToEnd(ppDestList, ppSourceList)               \
            doubleListAddOneToEnd((LINKED_LIST *) ppDestList,               \
                doubleListRemoveOneFromFront((LINKED_LIST *) ppSourceList))


#define ITERATE_LIST(pList) {                   \
    LINKED_LIST pFirstOnList = (pList);         \
    do {

#define END_ITERATE_LIST(pList)                 \
        (pList) = (pList)->pNext;               \
    } while(pFirstOnList != (pList));           \
}


#endif