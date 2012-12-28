#ifndef SORTED_LIST_H
#define SORTED_LIST_H
/*
 * sorted-list.h
 */

/*
 * When your sorted list is used to store objects of some type, since the
 * type is opaque to you, you will need a comparator function to order
 * the objects in your sorted list.
 *
 * You can expect a comparator function to return -1 if the 1st object is
 * smaller, 0 if the two objects are equal, and 1 if the 2nd object is
 * smaller.
 *
 * Note that you are not expected to implement any comparator functions.
 * You will be given a comparator function when a new sorted list is
 * created.
 */

typedef int (*CompareFuncT)(void *, void *);

/*Node type for the sorted list.
 * 
 */
struct SortedListNode
{
	void* value;
	struct SortedListNode* next;
}; 
typedef struct SortedListNode* SLNode;

/*
 * Sorted list type.  You need to fill in the type as part of your implementation.
 */
struct SortedList
{
	SLNode headOfList;
    CompareFuncT cf;
};
typedef struct SortedList* SortedListPtr;

/*
 * Iterator type for user to "walk" through the list item by item, from
 * beginning to end.  You need to fill in the type as part of your implementation.
 */
struct SortedListIterator
{
	SLNode curNode;
};
typedef struct SortedListIterator* SortedListIteratorPtr;



/*
 * SLCreate creates a new, empty sorted list.  The caller must provide
 * a comparator function that can be used to order objects that will be
 * kept in the list.
 * 
 * If the function succeeds, it returns a (non-NULL) SortedListT object.
 * Else, it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

SortedListPtr SLCreate(CompareFuncT cf);

/* SLCreateNode creates a new, empty sorted list node. Caller must provide data to be stored.
 * If it succeeds, it returns the newly created node. Otherwise, it returns null.
 * 
 */
SLNode SLCreateNode(void* newObj);

/* SLDestroyNode destroys an individual node, whereas SLDestroy will destroy an entire list and all memory associated.
 * 
 * 
 */
 void SLDestroyNode(SLNode node);

/* SLRecursiveDestroy recursively destroys an entire list.
 * 
 */
 void SLRecursiveDestroy(SLNode node);

/*
 * SLDestroy destroys a list, freeing all dynamically allocated memory.
 * Ojects should NOT be deallocated, however.  That is the responsibility
 * of the user of the list.
 *
 * You need to fill in this function as part of your implementation.
 */
void SLDestroy(SortedListPtr list);


/*
 * SLInsert inserts a given object into a sorted list, maintaining sorted
 * order of all objects in the list.  If the new object is equal to a subset
 * of existing objects in the list, then the subset can be kept in any
 * order.
 *
 * If the function succeeds, it returns 1.  Else, it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

int SLInsert(SortedListPtr list, void *newObj);

/* SLSearch takes the head of a linked list, and a target to search for
 * if the target is in the list, it will return the index of the node it was found at.
 * Otherwise, returns -1 if not found.
 * 
 */
 
int SLSearch(SortedListPtr list, void *targObj);
int SLCustomSearch(SortedListPtr list, void *targObj, CompareFuncT cf);

/* SLRemoveIndex will go through the linked list, and remove the node at the given index.
 * It will return the head of the modified list.
 *
 */
 
SortedListPtr SLRemoveIndex(SortedListPtr list, int index);
 
/*
 * SLRemove removes a given object from a sorted list.  Sorted ordering
 * should be maintained.
 *
 * If the function succeeds, it returns 1.  Else, it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

int SLRemove(SortedListPtr list, void *newObj);


/*
 * SLCreateIterator creates an iterator object that will allow the caller
 * to "walk" through the list from beginning to the end using SLNextItem.
 *
 * If the function succeeds, it returns a non-NULL SortedListIterT object.
 * Else, it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

SortedListIteratorPtr SLCreateIterator(SortedListPtr list);


/*
 * SLDestroyIterator destroys an iterator object that was created using
 * SLCreateIterator().  Note that this function should destroy the
 * iterator but should NOT affectt the original list used to create
 * the iterator in any way.
 *
 * You need to fill in this function as part of your implementation.
 */

void SLDestroyIterator(SortedListIteratorPtr iter);


/*
 * SLNextItem returns the next object in the list encapsulated by the
 * given iterator.  It should return a NULL when the end of the list
 * has been reached.
 *
 * One complication you MUST consider/address is what happens if a
 * sorted list encapsulated within an iterator is modified while that
 * iterator is active.  For example, what if an iterator is "pointing"
 * to some object in the list as the next one to be returned but that
 * object is removed from the list using SLRemove() before SLNextItem()
 * is called.
 *
 * You need to fill in this function as part of your implementation.
 */

void *SLNextItem(SortedListIteratorPtr iter);
SLNode SLGetIndex(SortedListPtr head, int index);
SLNode merge(SLNode head_one, SLNode head_two, CompareFuncT cf);
SLNode mergesort(SLNode head, CompareFuncT cf);
SortedListPtr mergeHelper(SortedListPtr list);
void printList(SortedListPtr head);
#endif
