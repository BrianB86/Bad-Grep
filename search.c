#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "search.h"
#include "indexer.h"
#include "sorted-list.c"

int compareStrings(void *p1, void *p2)
{
	char *s1 = p1;
	char *s2 = p2;

	return strcmp(s1, s2);
}
/* Two methods to compare stats structs against each other, because
 * the list needs to be sorted by frequency in descending order, and
 * I want to be able to search by filename using SLCustomSearch.
 */ 
  
int compareStatsByFrequency(void* p1, void* p2)
{
	StatsPtr firstStats = (StatsPtr)p1;
	StatsPtr secondStats = (StatsPtr)p2;
	return (secondStats->frequency)-(firstStats->frequency);
}
/* Pass in a pointer to a char array.
 * Return a pointer to a word struct storing this char array.
 * Initialize the filestats list to empty list.
 */
 
WordPtr createWordStruct(char* word)
{
	WordPtr newWord = (WordPtr)malloc(sizeof(struct Word));
	newWord->wordArray = (char*)malloc(strlen(word) + 1);
	strcpy(newWord->wordArray, word);
	//newWord->filestats = SLCreate(*compareStatsByFrequency);
	return newWord;
}
void destroyStatsStruct(StatsPtr statsStruct)
{
	if(statsStruct == NULL)
	{
		return;
	}
	else
	{
		//printf("Freeing file: %s\n", statsStruct->filename);
		free(statsStruct->filename);
		//free(statsStruct->frequency);
		free(statsStruct);
		return;
	}
}
/* Given a sorted list storing strings, go through and free the value.
 * 
 */ 
void destroyAllStrings(SortedListPtr list)
{
	SortedListIteratorPtr listIter = SLCreateIterator(list);
	do
	{
		free(listIter->curNode->value);
	}while(SLNextItem(listIter) != NULL);
	SLDestroyIterator(listIter);
	return;
}
/* Destroy a word struct. Free all associated memory.
 * 
 */ 
void destroyWordStruct(WordPtr wordStruct)
{
	if(wordStruct == NULL)
	{
		return;
	}
	else
	{
		//printf("Freeing wordStruct for %s\n", wordStruct->wordArray);
		//Loop over filestats array and free all values
		if(wordStruct->filestats != NULL)
		{
			if(wordStruct->filestats->headOfList != NULL) //Free the filestats data too
			{
				SortedListIteratorPtr filestatsIter = SLCreateIterator(wordStruct->filestats);
				do
				{
					SLNode curNode = filestatsIter->curNode;
					StatsPtr value = (StatsPtr)curNode->value;
					destroyStatsStruct(value);
				}while(SLNextItem(filestatsIter) != NULL);
				SLDestroyIterator(filestatsIter);
				SLDestroy(wordStruct->filestats);
			}
		}
		free(wordStruct->wordArray);
		free(wordStruct);
		return;
	}
}
/* Given an index, go through and free all memory for word structs.
 * 
 */ 
void destroyAllWordStructs(SortedListPtr index)
{
	if(index == NULL)
	{
		return;
	}
	if(index->headOfList == NULL)
	{
		return;
	}
	SortedListIteratorPtr wordIter = SLCreateIterator(index);
	do
	{
		WordPtr curWordStruct = (WordPtr)wordIter->curNode->value;
		destroyWordStruct(curWordStruct);
	}while(SLNextItem(wordIter) != NULL);
	SLDestroyIterator(wordIter);
	return;
}

SortedListPtr searchOr(SortedListPtr indexList, SortedListPtr inputList)
{
	//need a result list
	SortedListPtr result = SLCreate(*compareStrings);
	//need to iterate through the inputList to check the words in the indexList
	SortedListIteratorPtr inputIterator = SLCreateIterator(inputList);
	do
	{
		SLNode inputListPtr = inputIterator->curNode;
		char* inputListWord = (char*)inputListPtr->value;
		//need to iterate through the indexListIterator for each item of inputList
		SortedListIteratorPtr indexListIterator = SLCreateIterator(indexList);
		do
		{
			//compare the word in the indexed list to the input list to see if they match
			SLNode indexListPtr = indexListIterator->curNode;
			WordPtr indexWord = (WordPtr) indexListPtr->value;
			int compareWords = strcmp(indexWord->wordArray, inputListWord);
			if(compareWords == 0)
			{
				//extract the filelist for the word
				SortedListPtr filePtr = indexWord->filestats;
				SortedListIteratorPtr fileIterator = SLCreateIterator(filePtr);
				//add the word to the result list
				do
				{
					SLNode fileHead = (SLNode)filePtr->headOfList;
					StatsPtr fileNamesPtr = (StatsPtr)fileHead->value;
					char* filenameToAdd = fileNamesPtr->filename;
					SLInsert(result, filenameToAdd);
				}while (SLNextItem(fileIterator) != NULL);
				SLDestroyIterator(fileIterator);
			} 
		} while (SLNextItem(indexListIterator) != NULL);
		SLDestroyIterator(indexListIterator);
	} while(SLNextItem(inputIterator) != NULL);
	SLDestroyIterator(inputIterator);
	return result;
}
/* Extension of SLSearch, because I don't want to be messy and creating
 * structs to search for words inside of other methods. Keep that here.
 */ 
int SLSearchWord(SortedListPtr head, char* word)
{
	WordPtr target = createWordStruct(word);
	void *castToSearch = (void*)target;
	int result = SLSearch(head,castToSearch);
	target->filestats = NULL;
	destroyWordStruct(target);
	return result;
}

SortedListPtr inputToLinkedList(char* tokenInput)
{
	//create new list
	SortedListPtr inputList = SLCreate(*compareStrings);
	//add words to linked list until the tokenstream is null
	while(tokenInput != NULL)
	{
		SLInsert(inputList, tokenInput);
		tokenInput = strtok(NULL, " ");
	}
	return inputList;
}
/* Given two lists of file names, return a list of file names that
 * is in both lists.
 */ 
SortedListPtr findCommonElements(SortedListPtr firstList, SortedListPtr secondList)
{
	SortedListPtr commonElements = SLCreate(*compareStrings);
	SortedListIteratorPtr outsideIter = SLCreateIterator(firstList);
	do
	{
		//printf("Grabbing outside node.\n");
		SLNode outsideNode = outsideIter->curNode;
		SortedListIteratorPtr insideIter = SLCreateIterator(secondList);
		do
		{
			//For every element in the second list, compare it to whatever my outsideIter is pointing to.
			SLNode insideNode = insideIter->curNode;
			//printf("Attempting to compare file stats by file name\n");
			//int compareVal = compareStatsByFilename(outsideNode->value, insideNode->value);
			if(outsideNode == NULL || insideNode == NULL || outsideNode->value == NULL || insideNode->value == NULL)
			{
				SLDestroyIterator(insideIter);
				SLDestroyIterator(outsideIter);
				SLDestroy(commonElements);
				//printf("Returning from findcommonelements because something is null.\n");
				return NULL;
			}
			int compareVal = strcmp(outsideNode->value, insideNode->value);
			//printf("Compareval = %d\n",compareVal);
			if(compareVal == 0) //Same file name, add it to the results list
			{
				//Theyre the same, so I can just add the outside one
				char* outsideStats = outsideNode->value;
				//printf("Found common element: %s\n", outsideStats);
				SLInsert(commonElements, outsideStats);
			}
		}while(SLNextItem(insideIter) != NULL);
		SLDestroyIterator(insideIter);		
	}while(SLNextItem(outsideIter) != NULL);
	SLDestroyIterator(outsideIter);
	return commonElements;
}
/* Insert all file names from a given word's node into a given list
 * 
 */ 
void insertAllFilenames(SortedListPtr list, SLNode word)
{
	WordPtr actualWord = (WordPtr)word->value;
	SortedListIteratorPtr filenameIter = SLCreateIterator(actualWord->filestats);
	do
	{
		StatsPtr curStat = filenameIter->curNode->value;
		char *word = curStat->filename;
		SLInsert(list, word);
	}while(SLNextItem(filenameIter) != NULL);
	SLDestroyIterator(filenameIter);
}
/* Perform logical and search
 * 
 * 
 */ 
SortedListPtr searchLogicalAnd(SortedListPtr indexHead, SortedListPtr words)
{
	SortedListPtr result = SLCreate(*compareStrings);
	SortedListPtr nextResult = NULL;
	SortedListPtr testResult = NULL;
	//Check if the first word exists
	SortedListIteratorPtr wordIter = SLCreateIterator(words);
	char *curWord = (char*)wordIter->curNode->value;
	int wordSearch = SLSearchWord(indexHead, curWord);
	//If the word is not in the list then just screw it, they get nothing.
	if(wordSearch == -1)
	{
		//printf("Returning becasue wordSearch=-1\n");
		SLDestroyIterator(wordIter);
		SLDestroy(result);
		return NULL;
	}
	//First word is in the list, so just copy everything from its filestats to results
	SLNode firstwordNode = SLGetIndex(indexHead, wordSearch);
	//printf("Copying all first word's stuff into new list\n");
	insertAllFilenames(result, firstwordNode);
	//Now hop into a while loop and check over and over.
	int i = 0;
	while(SLNextItem(wordIter) != NULL)
	{
		SortedListPtr nextWordList = SLCreate(*compareStrings);
		curWord = (char*)wordIter->curNode->value;
		int searchVal = SLSearchWord(indexHead, curWord);
		if(searchVal == -1) //Word not in the list, return nothing immediately.
		{
			//destroyAllStrings(result);
			//printf("Returning because searchVal = -1 inside while.\n");
			SLDestroy(nextWordList);
			//SLDestroy(result);
			if(nextResult != NULL)
			{
				SLDestroy(nextResult);
			}
			SLDestroyIterator(wordIter);
			return NULL;
		}
		else
		{
			SLNode nextNode = SLGetIndex(indexHead, searchVal);
			//printf("Copying another word's file stats shit\n");
			insertAllFilenames(nextWordList, nextNode);
			//printf("Attempting to find common elements.\n");
			if(i == 0)
			{
				testResult = findCommonElements(result, nextWordList);
				if(testResult != NULL)
				{
					nextResult = testResult;
				}
				else
				{
					if(nextResult != NULL)
					{
						SLDestroy(nextResult);
					}
					SLDestroyIterator(wordIter);
					SLDestroy(nextWordList);
					return NULL;
				}
				SLDestroy(result);
			}
			else
			{
				testResult = findCommonElements(nextResult, nextWordList);
				if(testResult != NULL)
				{
					nextResult = testResult;
				}
				else
				{
					if(nextResult != NULL)
					{
						SLDestroy(nextResult);
					}
					SLDestroyIterator(wordIter);
					SLDestroy(nextWordList);
					return NULL;
				}
			}
		}
		//destroyAllStrings(nextWordList);
		SLDestroy(nextWordList);
		i++;
	}
	SLDestroyIterator(wordIter);
	if(i == 0)
	{
		return result;
	}
	else
	{
		return nextResult;
	}
}
void takeInput(SortedListPtr indexList)
{
	int maxInputSize = 128;

	char line[maxInputSize];
	char *p;
	int done = 0;
	while(done == 0)
	{
		p = fgets(line, maxInputSize, stdin); 
		if (p == NULL)
		{
			return;
		}
		else
		{
			int last = strlen(line);
			if(line[last-1] == '\n')
			{
				line[last-1] = '\0';
			}
			printf("Input: %s\n", line);
		}
		int quit = strcmp("q",line);
		if(quit == 0)
		{
			done = 1;
			printf("Gracefully shutting down...\n");
		}
		else
		{
			//check to see if the operation is sa or so
			char* subbuff = strtok(p, " ");
			
			char* newsa = "sa";
			char* newso = "so";
			int sa = strcmp(newsa, subbuff);
			int so = strcmp(newso, subbuff);
			if((sa == 0) || (so == 0))
			{

				//need to add all words given into a linked list
				subbuff = strtok(NULL, " ");
				
				if(subbuff != NULL)
				{
					SortedListPtr inputList = inputToLinkedList(subbuff);
					if(sa == 0)
					{

							//do search and
							SortedListPtr SAList = searchLogicalAnd(indexList, inputList);
							if(SAList == NULL)
							{
								printf("No files found\n");
							}
							else
							{
								printList(SAList);
								/*if(SAList->headOfList != NULL)
								{
									//destroyAllStrings(SAList);
									SLDestroy(SAList);
								}*/
								SLDestroy(SAList);
							}
							
					}
					else
					{
						//do search or
						SortedListPtr SOList = searchOr(indexList, inputList);
						if(SOList->headOfList == NULL)
						{
							printf("No files found\n");
							SLDestroy(SOList);
						}
						else
						{
							printList(SOList);
							SLDestroy(SOList);
						}
					}
					//destroyAllStrings(inputList);
					SLDestroy(inputList);
				}
				//no words given
				else 
				{
					printf("No words added\n");
				}
				//check if it is searchand or searchor

			}
			//not an argument
			else
			{
				printf("Not an argument\n");
			}	
		}
	}
}


int SLInsertIndex(SortedListPtr list, void *newObj)
{
		//Create new node.
		SLNode newNode = SLCreateNode(newObj);
		//Set the next of the new node, to be the head of the current list.
		newNode->next = list->headOfList;
		list->headOfList = newNode;
		//Sort the list
		list = mergeHelper(list);
		//printList(list);
		return 1;

}

/* Compares two word structs. Does this by casting the void* to WordPtr
 * then using strcmp to compare the values of the strings stored by 
 * each word struct.
 */

int compareWordStruct(void* p1, void* p2)
{
	WordPtr firstWord = (WordPtr)p1;
	WordPtr secondWord = (WordPtr)p2;
	//printf("Comparing %s and %s.\n",firstWord->wordArray, secondWord->wordArray);
	return strcmp(firstWord->wordArray,secondWord->wordArray);
}

/* Pass in a pointer to a char array.
 * Return a pointer to a stats struct initialized to freq. 1.
 */
 
StatsPtr createStatsStruct(char* filename)
{
	StatsPtr newStats = (StatsPtr)malloc(sizeof(struct Stats));
	newStats->filename = (char*)malloc(strlen(filename) + 1);
	strcpy(newStats->filename, filename);
	//newStats->filename = *filename;
	newStats->frequency = 1;
	return newStats;
}

/* Given a line of file stats, create a Sorted List of file stats
 * and return it
 * 
 */
 SortedListPtr buildFileStatsList(char *line)
 {
	 SortedListPtr newList = SLCreate(*compareStatsByFrequency);
	 //tokenize by spaces
	 char *result = NULL;
	 result = strtok(line, " ");
	 //tokenize until there are no more spaces
	 char *filename = NULL;
	 while(result != NULL)
	 {
		 int testNum = atoi(result);
		 if(testNum == 0) //It must be a file name then
		 {
			 filename = result;
		 }
		 else
		 {
			 if(filename != NULL)
			 {
				 StatsPtr newStatsStruct = createStatsStruct(filename);
				 newStatsStruct->frequency = testNum;
				 int insert = SLInsert(newList, newStatsStruct);
				 if(insert == 0) //Need to force it in the list
				 {
					 SLInsertIndex(newList, newStatsStruct);
				 }
			 }
		 }
		 result = strtok(NULL, " ");
	 }
	 return newList;
 }

/* Given a word line, extract the word from the line.
 * 
 */ 
WordPtr extractWord(char *line)
{
	//I just want to take every character in the line EXCEPT the first 7
	//The first 7 will be <List>  
	char *newWord = (char*)malloc((strlen(line) - 6));
	strcpy(newWord,&line[7]);
	WordPtr newWordStruct = createWordStruct(newWord);
	//printf("Extracted word %s\n",newWord);
	free(newWord);
	return newWordStruct;
}


/* Given a line of input from the file, determine the type of 
 * information stored on that line.
 * 
 */ 

enum lineType classifyLine(char *line)
{
	//Look at the first character. If it is <, then it is either
	//the beginning of a new word definition or the end of one.
	char firstChar = line[0];
	if(firstChar == '<')
	{
		char nextChar = line[1];
		if(nextChar != '/') //This line is a definition of a new word
		{
			return WordLine;
		}
		else
		{
			return Throwaway;
		}
	}
	return FileStatsLine;
}
/* Given the name of a file exported from indexer, parse it back into
 * the linked list form.
 * 
 */ 
SortedListPtr parseFileToList(char *filename)
{
	FILE *fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("Unable to open file %s\n",filename);
		return NULL;
	}
	SortedListPtr newList = SLCreate(*compareWordStruct);
	WordPtr curWord = NULL;
	char line[128];
	while(fgets(line,128,fp) != NULL)
	{
		//Trim the newline character.
		if(strlen(line) > 0)
		{
			line[strlen(line)-1] = '\0';
			//printf("Input trimmed.\n");
		}
		//Classify the type of line.
		enum lineType curLine = classifyLine(line);
		//printf("Line type: %d\n",curLine);
		switch(curLine)
		{
			case WordLine:
				//Extract the word from the line.
				curWord = extractWord(line);
				//printf("Inserting new word into list.\n");
				SLInsert(newList, curWord);
				break;
			case FileStatsLine:
				//Parse the line
				//Add the file stats to curWord's filestatslist
				curWord->filestats = buildFileStatsList(line);
				break;
			case Throwaway:
				//Do nothing I guess.
				break;
		}
	}
	fclose(fp);
	return newList;
}

int main(int argc, char* argv[])
{
	SortedListPtr newList = parseFileToList(argv[1]);
	if(newList == NULL)
	{
		printf("No indexer file to parse\n");
		return 0;
	}
	takeInput(newList);
	destroyAllWordStructs(newList);
	SLDestroy(newList);
	return 0;
}
