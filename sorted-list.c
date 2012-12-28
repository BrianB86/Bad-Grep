/*
 * sorted-list.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sorted-list.h"



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

SortedListPtr SLCreate(CompareFuncT cf)
{
	SortedListPtr slNew = malloc(sizeof(struct SortedList));
	slNew->headOfList = NULL;
	slNew->cf = cf;
	return slNew;
}

/* SLCreateNode creates a new, empty sorted list node. Caller must provide data to be stored.
 * If it succeeds, it returns the newly created node. Otherwise, it returns null.
 * 
 */

SLNode SLCreateNode(void* newObj)
{
	SLNode newNode = malloc(sizeof(struct SortedListNode));
	newNode->value = newObj;
	newNode->next = NULL;
	return newNode;
}


/* SLDestroyNode will destroy an individual node, whereas SLDestroy will destroy an entire list.
 * 
 * 
 * 
 */
void SLDestroyNode(SLNode node)
{
	if(node != NULL)
	{
		free(node);
	}
	return;
}

/*
 * SLDestroy destroys a list, freeing all dynamically allocated memory.
 * Ojects should NOT be deallocated, however.  That is the responsibility
 * of the user of the list.
 *
 * You need to fill in this function as part of your implementation.
 */
 
void SLRecursiveDestroy(SLNode node)
{
	if(node->next != NULL)
	{
		SLRecursiveDestroy(node->next);
	}
	SLDestroyNode(node);
	return;
}

void SLDestroy(SortedListPtr list)
{
	if(list == NULL)
	{
		return;
	}
	SLNode temp = list->headOfList;
	if(temp == NULL)
	{
		free(list);
		return;
	}
	if(temp->next == NULL)
	{
//		free(temp->value);
		free(temp);
		free(list);
		return;
	}
	SLNode cur = NULL;
	while(temp != NULL)
	{
		cur = temp->next;
		free(temp);
		temp = cur;
	}
	free(list);
	printf("-----\n");
	return;
}

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

int SLInsert(SortedListPtr list, void *newObj)
{
	CompareFuncT cf = list->cf;
	if(list->headOfList == NULL)
	{
		SLNode newNode = SLCreateNode(newObj);
		list->headOfList = newNode;
		return 1;
	}		
	//Create new node.
	SLNode newNode = SLCreateNode(newObj);
	/*//set it to point to the head
	newNode->next = list->headOfList;
	list->headOfList = newNode;
	list = mergeHelper(list);
	return 1;*/
	if(cf(newObj, list->headOfList->value) < 0) //New obj comes before headoflist's value.
	{
		newNode->next = list->headOfList;
		list->headOfList = newNode;
		return 1;
	}
	else
	{
		SortedListIteratorPtr prevIter = SLCreateIterator(list);
		SortedListIteratorPtr nextIter = SLCreateIterator(list);
		if(SLNextItem(nextIter) != NULL)
		{
			while(cf(newObj, nextIter->curNode->value) > 0) //Loop while the current node's value comes before the one im trying to insert
			{
				if(SLNextItem(nextIter) == NULL)
				{
					break;
				}
				SLNextItem(prevIter); //Should never be null before next iter.
			}
			if(nextIter->curNode->next == NULL) //Add it to the end of the list
			{
				nextIter->curNode->next = newNode;
			}
			else
			{
				newNode->next = prevIter->curNode->next;
				prevIter->curNode->next = newNode;
			}
		}
		else
		{
			nextIter->curNode->next = newNode;
		}
		SLDestroyIterator(prevIter);
		SLDestroyIterator(nextIter);
	}
	return 1;
}

/* SLSearch takes the head of a linked list, and a target to search for
 * if the target is in the list, it will return the index of the node it was found at.
 * Otherwise, returns -1 if not found.
 * 
 */

int SLSearch(SortedListPtr list, void *targObj)
{
	//Create a new iterator from the head of the list.
	SortedListIteratorPtr iterator = SLCreateIterator(list);
	CompareFuncT cf = list->cf;
	//If the head of the list contains the target value, return 0.
	if(iterator->curNode == NULL)
	{
		SLDestroyIterator(iterator);
		return -1;
	}
	if(iterator->curNode->value == NULL)
	{
		SLDestroyIterator(iterator);
		return -1;
	}
	if(cf(iterator->curNode->value,targObj) == 0)
	{
		SLDestroyIterator(iterator);
		return 0;
	}
	//Otherwise, begin looping through the list.
	int done = 0;
	int index = 0;
	while(done == 0)
	{
		//Get the next item in the list
		index++;
		//Check if I'm at the end of the list
		if(SLNextItem(iterator) == NULL)
		{
			done = 1;
		}
		else
		{
			int result = cf(iterator->curNode->value,targObj);
			if(result == 0)
			{
				SLDestroyIterator(iterator);
				return index;
			}
		}
	}
	SLDestroyIterator(iterator);
	return -1;
	//Otherwise, until SLNextItem returns NULL, check each item to see if it matches targObj.
	//If matches, return index it was found at.
}

int SLCustomSearch(SortedListPtr list, void *targObj, CompareFuncT cf)
{
	//Create a new iterator from the head of the list.
	SortedListIteratorPtr iterator = SLCreateIterator(list);
	//If the head of the list contains the target value, return 0.
	if(cf(iterator->curNode->value,targObj) == 0)
	{
		SLDestroyIterator(iterator);
		return 0;
	}
	//Otherwise, begin looping through the list.
	int done = 0;
	int index = 0;
	while(done == 0)
	{
		//Get the next item in the list
		index++;
		//Check if I'm at the end of the list
		if(SLNextItem(iterator) == NULL)
		{
			done = 1;
		}
		else
		{
			int result = cf(iterator->curNode->value,targObj);
			if(result == 0)
			{
				SLDestroyIterator(iterator);
				return index;
			}
		}
	}
	SLDestroyIterator(iterator);
	return -1;
	//Otherwise, until SLNextItem returns NULL, check each item to see if it matches targObj.
	//If matches, return index it was found at.
}

/* SLRemoveIndex will go through the linked list, and remove the node at the given index.
 * It will return the head of the modified list.
 *
 */

SortedListPtr SLRemoveIndex(SortedListPtr list, int index)
{
	int curIndex = 0;
	if(index == 0)
	{
		SLNode temp = list->headOfList;
		list->headOfList = list->headOfList->next;
		free(temp);
        return list;
	}
	SLNode head = list->headOfList;
	if(head->next != NULL)
	{
		SortedListIteratorPtr leadingPtr = SLCreateIterator(list);
		if(SLNextItem(leadingPtr) == NULL)
		{
			return list;
		}
		SortedListIteratorPtr laggingPtr = SLCreateIterator(list);
		//Loop until index
		while(curIndex<index-1)
		{
			SLNextItem(leadingPtr);
			SLNextItem(laggingPtr);
			curIndex++;
			if(leadingPtr == NULL || laggingPtr == NULL)
			{
				return list;
			}
		}
		laggingPtr->curNode->next = leadingPtr->curNode->next;
		SLDestroyNode(leadingPtr->curNode);
		SLDestroyIterator(leadingPtr);
		SLDestroyIterator(laggingPtr);
		return list;
	}
	else
	{
		return NULL;
	}
	//Set next of (index-1)->(index+1)
	//return head of list
}

/*
 * SLRemove removes a given object from a sorted list.  Sorted ordering
 * should be maintained.
 *
 * If the function succeeds, it returns 1.  Else, it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

int SLRemove(SortedListPtr list, void *newObj)
{
	//call SLSearch for newObj
	int result = SLSearch(list, newObj);
    if(result == -1)
	{
       	return -1;
    }
	else
	{
		SLRemoveIndex(list,result);
		return 1;
	}
}

/*
 * SLCreateIterator creates an iterator object that will allow the caller
 * to "walk" through the list from beginning to the end using SLNextItem.
 *
 * If the function succeeds, it returns a non-NULL SortedListIterT object.
 * Else, it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

SortedListIteratorPtr SLCreateIterator(SortedListPtr list)
{
    
    if(list != NULL)
    {
		SortedListIteratorPtr iter = malloc(sizeof(struct SortedListIterator));
        iter->curNode = list->headOfList;
        return iter;
    }
    else
    {
        return NULL;
    }
    //Pass in head of a linked list.
	//Create iterator
	//Return it.
}

/*
 * SLDestroyIterator destroys an iterator object that was created using
 * SLCreateIterator().  Note that this function should destroy the
 * iterator but should NOT affectt the original list used to create
 * the iterator in any way.
 *
 * You need to fill in this function as part of your implementation.
 */

void SLDestroyIterator(SortedListIteratorPtr iter)
{
	//free(iter->curNode);
	free(iter);
}

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

void *SLNextItem(SortedListIteratorPtr iter)
{
	//make sure iter->curNode != null so i dont try to say NULL->curNode
	//Set iterator to point to SortedListIteratorPtr->curNode->next
	if(iter->curNode == NULL)
	{
		return NULL;
	}
	if(iter->curNode->next != NULL)
	{
		iter->curNode = iter->curNode->next;
		return iter;
	}
	else
	{
		return NULL;
	}
}
/* Given an index of a node in the linked list
 * Return that node.
 */
SLNode SLGetIndex(SortedListPtr head, int index)
{
	SortedListIteratorPtr newIterator = SLCreateIterator(head);
	int curIndex = 0;
	while(curIndex < index)
	{
		SLNextItem(newIterator);
		curIndex++;
	}
	SLNode ret = newIterator->curNode;
	SLDestroyIterator(newIterator);
	return ret;
}

void printList(SortedListPtr head)
{
	//Please be aware, this is only going to work for sorted
	//lists of type CHAR*. Other typed lists probably wont work,
	//this is only for testing purposes.
	SortedListIteratorPtr newIterator = SLCreateIterator(head);
	printf("List:\n");
	int done = 0;
	if(head == NULL)
	{
		done = 1;
	}
	else if(head->headOfList == NULL)
	{
		done = 1;
	}
	while(done == 0)
	{
		char *val = (char*)newIterator->curNode->value;
		printf("%s\n", val);
		if(SLNextItem(newIterator) == NULL)
		{
			printf("Reached end of linked list.\n");
			done = 1;
		}
	}
	SLDestroyIterator(newIterator);
	return;
}
SLNode merge(SLNode head_one, SLNode head_two, CompareFuncT cf)
{
	SLNode head_three = NULL;

	if(head_one == NULL)
	{
		return head_two;
	}

	if(head_two == NULL)
	{
		return head_one;
	}

	if(cf(head_one->value, head_two->value) < 0)
	{
		head_three = head_one;
		head_three->next = merge(head_one->next, head_two, cf);
	} 
	else
	{
		head_three = head_two;
		head_three->next = merge(head_one, head_two->next, cf);
	}
	return head_three;
}
//add a helper method that takes in the SortedListPtr, then passes the head of that list to mergesort, change return type to SLNode
SLNode mergesort(SLNode head, CompareFuncT cf)
{
	if(head == NULL)
	{
		return head;
	}
	SLNode head_one = NULL;
	SLNode head_two = NULL;
	

	if((head == NULL) || (head->next == NULL))
	{
		return head;
	}

	head_one = head;
	head_two = head->next;

	while((head_two != NULL) && (head_two->next != NULL)) 
	{
		head = head->next;
		head_two = head->next->next;
	}
	
	head_two = head->next;
	head->next = NULL;
	
	return merge(mergesort(head_one, cf), mergesort(head_two, cf), cf);
}

SortedListPtr mergeHelper(SortedListPtr list)
{
	SLNode head = list->headOfList;
	list->headOfList = mergesort(head, list->cf);
	return list;
}
