#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "indexer.h"
#include "sorted-list.c"

unsigned long maxSize; //maximum size of data structure allocated as defined by user
unsigned long indexerListSize; //size of current indexList
FILE* fp; //current position of filename
int gCounter = 0;

enum lineType 
{
	WordLine, FileStatsLine, Throwaway
};
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
char* extractWord(char* line)
{
	//I just want to take every character in the line EXCEPT the first 7
	//The first 7 will be <List>  
	char *newWord = (char*)malloc((strlen(line) - 6));
	strcpy(newWord,&line[7]);
	int len = strlen(newWord);
	newWord[len-1] = '\0';
	//free(newWord);
	return newWord;
}

struct finalCache
{
	char* wordArray;
	long int lineOfStats;
	int counter;
};
typedef struct finalCache* cachePtr;

int compareStrings(void *p1, void *p2)
{
	char *s1 = p1;
	char *s2 = p2;

	return strcmp(s1, s2);
}

int compareCacheStruct(void* p1, void* p2)
{
	cachePtr firstWord = (cachePtr)p1;
	cachePtr secondWord = (cachePtr)p2;
	//printf("Comparing %s and %s.\n",firstWord->wordArray, secondWord->wordArray);
	return strcmp(firstWord->wordArray,secondWord->wordArray);
}

cachePtr createCacheStruct(char* word)
{
	cachePtr newCache = (cachePtr)malloc(sizeof(struct finalCache));
	newCache->wordArray = (char*)malloc(strlen(word) + 1);
	strcpy(newCache->wordArray, word);
	newCache->counter = gCounter;
	gCounter++;
	//newWord->filestats = SLCreate(*compareStatsByFrequency);
	return newCache;
}

void destroyCacheStruct(cachePtr cacheStruct)
{
	if(cacheStruct == NULL)
	{
		return;
	}
	free(cacheStruct->wordArray);
	free(cacheStruct);
}
SortedListPtr inputToLinkedList(char* tokenInput)
{
	//create new list
	SortedListPtr inputList = SLCreate(*compareStrings);
	//add words to linked list until the tokenstream is null
	while(tokenInput != NULL)
	{
		//printf("Inserting %s to the input list.\n", tokenInput);
		SLInsert(inputList, tokenInput);
		tokenInput = strtok(NULL, " ");
	}
	return inputList;
}

SortedListPtr readFileStats(cachePtr word)
{
	SortedListPtr result = SLCreate(*compareStrings);
	//Step one, store the current location of the buffer so I can go back.
	long int curBytes = ftell(fp);
	//Seek to the location of the filestats, relative to the beginning of the file.
	fseek(fp,word->lineOfStats, SEEK_SET);
	enum lineType curLine = FileStatsLine;
	char line[256];
	while(fgets(line, 256, fp) != NULL && classifyLine(line) == FileStatsLine)
	{
		//Get the next line
		//fgets(line, 256, fp);
		//printf("Got a filestats line for %s, here it is: %s\n",word->wordArray, line);
		curLine = classifyLine(line);
		if(curLine == FileStatsLine)
		{
			//printf("CurLine = %d\n", curLine);
			char* filename = strtok(line, " \n");
			while(filename != NULL)
			{
				int testNum = atoi(filename);
				if(testNum == 0) //It must be a file name then
				{
					//printf("Adding %s to result list\n", filename);
					char* newWord = malloc(sizeof(char)*strlen(filename) + 1);
					strcpy(newWord, filename);
					SLInsert(result, newWord);
					//printf("Result list is now: \n");
					//printList(result);
				}
				filename = strtok(NULL, "  \n");
			}
			//printf("Done adding files to list.\n");
		}	
	}
	//Reset the file buffer back to where it was
	fseek(fp, curBytes, SEEK_SET);
	//printf("Returning:\n");
	//printList(result);
	return result;
}


int SLSearchWordFromCache(SortedListPtr head, char* word)
{
	cachePtr target = createCacheStruct(word);
	void *castToSearch = (void*)target;
	int result = SLSearch(head,castToSearch);
	//target->counter = NULL;
	//target->lineOfStats = NULL;
	destroyCacheStruct(target);
	return result;
}
void updateCache(SortedListPtr curCache)
{
	int done = 0;
	cachePtr lastCacheStruct;
	int hasFileStatsBeenSet = 0;
	char line[256];
	while(done == 0)
	{
		long int curLine = ftell(fp);
		if(fgets(line, 256, fp) == NULL)
		{
			//If I've reached the end of the file and still have leftover memory, then
			//I need to just return.
			if(indexerListSize < maxSize)
			{
				//At this point, updateCache should never need to be called again. The entire index is in memory.
				return;
			}
			else
			{
				//Otherwise, fseek to the beginning
				fseek(fp, 0, SEEK_SET);
				continue;
			}
			
		}
		enum lineType curLineType = classifyLine(line);
		char* word;
		switch(curLineType)
		{
			case WordLine:
				//Extract the word
				word = extractWord(line);
				if(indexerListSize + (strlen(word) + sizeof(struct finalCache)) > maxSize)
				{
					//Too much memory. Abandon ship!
					free(word);
					fseek(fp, curLine, SEEK_SET);
					return;
				}
				else
				{
					lastCacheStruct = createCacheStruct(word);
					hasFileStatsBeenSet = 0;
				//	printf("Adding %s to the cache\n", lastCacheStruct->wordArray);
					SLInsert(curCache, lastCacheStruct);
					indexerListSize += (strlen(word) + sizeof(struct finalCache));
					free(word);
					break;
				}
			case FileStatsLine:
				if(hasFileStatsBeenSet == 0)
				{
					lastCacheStruct->lineOfStats = curLine;
					hasFileStatsBeenSet = 1;
				}
				break;
			case Throwaway:
				//do nothing
				break;
		}
	}
}
//finds the relatively last thing added to the list
int findSmallestCounter(SortedListPtr list)
{
	int smallestCounter;
	int curIndex = 0;
	int smallestCounterIndex = 0;
	SortedListIteratorPtr indexListIterator = SLCreateIterator(list);
	SLNode curNodeSmallest = indexListIterator->curNode;
	cachePtr smallestCounterCache = (cachePtr)curNodeSmallest->value;
	smallestCounter = smallestCounterCache->counter;
	do //iterates through the list
	{
		SLNode indexListPtr = indexListIterator->curNode;
		cachePtr indexCache = (cachePtr) indexListPtr->value;
		if(smallestCounter > indexCache->counter)
		{ //if the smallestCounter is larger than the counter found, swap the values
			smallestCounter = indexCache->counter;
			smallestCounterIndex = curIndex;
		}
		curIndex++;
	} while(SLNextItem(indexListIterator) != NULL);
	SLDestroyIterator(indexListIterator);
	return smallestCounterIndex;
}
//finds the most recent thing added to the list
int findLargestCounter(SortedListPtr list)
{
	int largestCounter = gCounter;
	SortedListIteratorPtr indexListIterator = SLCreateIterator(list);
	int index = 0;
	//gets index of largest counter
	do
	{
		SLNode indexListPtr = indexListIterator->curNode;
		cachePtr indexCache = (cachePtr) indexListPtr->value;
		if(largestCounter == indexCache->counter)
		{
			break;
		}
		index++;                                                                                                                                                                                                                                                                                                                                                                                                                                       
	} while(SLNextItem(indexListIterator) != NULL);
	SLDestroyIterator(indexListIterator);
	return index;
}
int firstInFirstOut(SortedListPtr list)
{
	int smallestCounterIndex = findSmallestCounter(list);
	SLNode indexCache = SLGetIndex(list, smallestCounterIndex);
	cachePtr cacheToRemove = (cachePtr)indexCache->value;
//	printf("Removed %s from list.\n", cacheToRemove->wordArray);
	int sizeOfFilename = strlen(cacheToRemove->wordArray);
	destroyCacheStruct(cacheToRemove);
	SLRemoveIndex(list, smallestCounterIndex);
	return sizeOfFilename;
}
void recache(SortedListPtr indexList)
{
	 //printCache(indexList);
	 int sizeOfRemoved = firstInFirstOut(indexList);
	 indexerListSize = indexerListSize - ((sizeof(struct finalCache)) + sizeOfRemoved);
    
    //method to add next line
    //TODO get brian's stuff
   // printf("indexerListSize is %d\n", indexerListSize);
    //printf("maxSize is %d\n", maxSize);
    updateCache(indexList);
}

int getWordToCache(SortedListPtr cache, char* word)
{
	//Check if the word is in the cache.
	int result = SLSearchWordFromCache(cache, word);
	//printf("Result of word being in the cache = %d\n", result);
	int answer = 0;
	if(result == -1) //The word is NOT in the cache.
	{
		int done = 0;
		while(done == 0)
		{
			int firstIndex = findSmallestCounter(cache);
			int lastIndex = findLargestCounter(cache);
			SLNode firstIndexNode = SLGetIndex(cache, firstIndex);
			SLNode lastIndexNode = SLGetIndex(cache, lastIndex);
			cachePtr firstIndexCache = (cachePtr)firstIndexNode->value;
			cachePtr lastIndexCache = (cachePtr)lastIndexNode->value;
			int firstCompareVal = strcmp(firstIndexCache->wordArray, word);
			int secondCompareVal = strcmp(lastIndexCache->wordArray, word);
			if(firstCompareVal <= -1 && secondCompareVal >= 1) //Not possible for the word to be in the cache.
			{
				done = 1;
				answer = -1;
			}
			else //Still possible for the word to be in the index. Update cache.
			{
				recache(cache);
				result = SLSearchWordFromCache(cache, word);
				if(result != -1) //Word IS in the cache
				{
					done = 1;
					answer = result;
				}
			}
		}
	}
	else //The word is already in the cache.
	{
		answer = result;
		//printf("Answer = 1. Word is in the cache.\n");
	}
	return answer;
}

void addAllToList(SortedListPtr dest, SortedListPtr src)
{
	SortedListIteratorPtr srcIter = SLCreateIterator(src);
	do
	{
		SLNode curNode = srcIter->curNode;
		char* wordToAdd = (char*)curNode->value;
		int inList = SLSearch(dest, wordToAdd);
		if(inList == -1)
		{
			SLInsert(dest, wordToAdd);
		}
		else //I don't need it again, so free associated memory..?
		{
			free(wordToAdd);
		}
	}while(SLNextItem(srcIter) != NULL);
	SLDestroyIterator(srcIter);
}
SortedListPtr searchCacheOr(SortedListPtr cache, SortedListPtr input)
{
	SortedListPtr resultList = SLCreate(*compareStrings);
	//printf("List created\n.");
	SortedListIteratorPtr inputIter = SLCreateIterator(input);
	do
	{
		//For everything in the input list, check the cache for that word.
		char* curInputWord = (char*)inputIter->curNode->value;
		//printf("Attempting to get word to cache.\n");
		int wordInCache = getWordToCache(cache, curInputWord);
		//printf("%d\n", wordInCache);
		if(wordInCache == -1)
		{
			//Not possible for word to be in list.
		}
		else
		{
			//printf("Searching for the word in the cache struct.\n");
		//	printf("wordInCache is %d\n", wordInCache);
			int wordToFindIndex = SLSearchWordFromCache(cache, curInputWord);
			SLNode wordToFindNode = SLGetIndex(cache, wordToFindIndex);
			cachePtr wordToFind = (cachePtr)wordToFindNode->value;
			//printf("attempting to read file stats for the word\n");
			SortedListPtr filenamesForWord = readFileStats(wordToFind);
			//printf("Received filenames for word, here is the list: \n");
			//printList(filenamesForWord);
			addAllToList(resultList, filenamesForWord);
			SLDestroy(filenamesForWord);
		}	
	}while(SLNextItem(inputIter) != NULL);
	SLDestroyIterator(inputIter);
	//printf("Final result of search or list: \n");
	//printList(resultList);
	return resultList;
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
				char* outsideFileName = malloc(sizeof(char)*strlen(outsideStats) + 1);
				strcpy(outsideFileName, outsideStats);
				//printf("Found common element: %s\n", outsideStats);
				SLInsert(commonElements, outsideFileName);
			}
		}while(SLNextItem(insideIter) != NULL);
		SLDestroyIterator(insideIter);		
	}while(SLNextItem(outsideIter) != NULL);
	SLDestroyIterator(outsideIter);
	destroyAllStrings(firstList);
	destroyAllStrings(secondList);
	SLDestroy(firstList);
	SLDestroy(secondList);
	return commonElements;
}
SortedListPtr searchCacheAnd(SortedListPtr cache, SortedListPtr words)
{
	SortedListPtr result = SLCreate(*compareStrings);
	SortedListPtr nextResult = NULL;
	SortedListPtr testResult = NULL;
	//Check if the first word exists
	SortedListIteratorPtr wordIter = SLCreateIterator(words);
	char *curWord = (char*)wordIter->curNode->value;
	int wordSearch = getWordToCache(cache, curWord);
	//If the word is not in the list then just screw it, they get nothing.
	if(wordSearch == -1)
	{
		//printf("Returning becasue wordSearch=-1\n");
		SLDestroyIterator(wordIter);
		SLDestroy(result);
		return NULL;
	}
	//First word is in the list, so just copy everything from its filestats to results
	SLNode firstwordNode = SLGetIndex(cache, wordSearch);
	//printf("Copying all first word's stuff into new list\n");
	cachePtr firstWordCache  = (cachePtr)firstwordNode->value;
	SortedListPtr firstWordsFiles = readFileStats(firstWordCache);
	addAllToList(result, firstWordsFiles);
	//Now hop into a while loop and check over and over.
	int i = 0;
	while(SLNextItem(wordIter) != NULL)
	{
		SortedListPtr nextWordList = SLCreate(*compareStrings);
		curWord = (char*)wordIter->curNode->value;
		int searchVal = getWordToCache(cache, curWord);
		if(searchVal == -1) //Word not in the list, return nothing immediately.
		{
			//destroyAllStrings(result);
			//printf("Returning because searchVal = -1 inside while.\n");
			SLDestroy(nextWordList);
			SLDestroy(result);
			destroyAllStrings(firstWordsFiles);
			SLDestroy(firstWordsFiles);
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
			SLNode nextNode = SLGetIndex(cache, searchVal);
			cachePtr nextCache = (cachePtr)nextNode->value;
			SLDestroy(nextWordList);
			nextWordList = readFileStats(nextCache);
			//insertAllFilenames(nextWordList, nextNode);
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
					destroyAllStrings(nextWordList);
					SLDestroy(nextWordList);
					destroyAllStrings(firstWordsFiles);
					SLDestroy(firstWordsFiles);
					return NULL;
				}
				//SLDestroy(result);
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
					destroyAllStrings(nextWordList);
					SLDestroy(nextWordList);
					//destroyAllStrings(firstWordsFiles);
					SLDestroy(firstWordsFiles);
					return NULL;
				}
			}
		}
		//destroyAllStrings(nextWordList);
		//SLDestroy(nextWordList);
		i++;
	}
	SLDestroyIterator(wordIter);
	if(i == 0)
	{
		//destroyAllStrings(firstWordsFiles);
		SLDestroy(firstWordsFiles);
		return result;
	}
	else
	{
	 //   destroyAllStrings(firstWordsFiles);
		SLDestroy(firstWordsFiles);
		return nextResult;
	}
}

void takeInput(SortedListPtr indexList)
{
    printf("Done populating cache\n");
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
							SortedListPtr SAList = searchCacheAnd(indexList, inputList);
							if(SAList == NULL || SAList->headOfList == NULL)
							{
								printf("No files found\n");
							}
							else
							{
								printList(SAList);
								if(SAList->headOfList != NULL)
								{
								//	destroyAllStrings(SAList);
								}
								SLDestroy(SAList);
							}
							
					}
					else
					{
						//do search or
						//printf("Doing search or\n");
						SortedListPtr SOList = searchCacheOr(indexList, inputList);
						if(SOList->headOfList == NULL)
						{
							printf("No files found\n");
						}
						else
						{
							printf("Results for search or: \n");
							printList(SOList);
							destroyAllStrings(SOList);
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
void destroyAllCacheStructs(SortedListPtr cache)
{
	SortedListIteratorPtr iter = SLCreateIterator(cache);
	do
	{
		SLNode curNode = iter->curNode;
		cachePtr curcachestruct = (cachePtr)curNode->value;
		destroyCacheStruct(curcachestruct);
	}while(SLNextItem(iter) != NULL);
	SLDestroyIterator(iter);
}
void printCache(SortedListPtr cache)
{
	SortedListIteratorPtr newIter = SLCreateIterator(cache);
	printf("List:\n");
	do
	{
		SLNode curNode = newIter->curNode;
		cachePtr curCache = (cachePtr)curNode->value;
		printf("%s\n",curCache->wordArray);
	}while(SLNextItem(newIter)!= NULL);
	SLDestroyIterator(newIter);
}
int main(int argc, char* argv[])
{
		if(strcasecmp("-m", argv[1]) == 0)
		{
		fp = fopen(argv[3],"r");
		char* userInput = argv[2];
		char* suffixSize = malloc(sizeof(char)*2 + 1);
		int len = (strlen(userInput) -2);
		strcpy(suffixSize, &userInput[len]);
		printf("suffixSize: %s \n",suffixSize);
		char* memAmt = malloc(sizeof(char)*(strlen(userInput)-3) + 2);
		memcpy(memAmt,userInput,len);
		int amount = atoi(memAmt);
		free(memAmt);
		printf("amount %d\n",amount);
		int memSize = suffixSize[0];
		printf("Beginning populating cache\n");
	
		SortedListPtr newList = SLCreate(*compareCacheStruct);
		switch(memSize)
		{
			case 'k':
			case 'K':
			printf("I found k\n");	
			maxSize = (unsigned long )(amount * 1024);
			printf("MaxSize: %lu \n",maxSize);
			updateCache(newList);
			printCache(newList);
			if(newList == NULL)
			{
				printf("No indexer file to parse\n");
				return 0;
			}
			takeInput(newList);
			free(suffixSize);
			fclose(fp);
			destroyAllCacheStructs(newList);
			SLDestroy(newList);
			return 0;
			break;
			
			case 'm':
			case 'M':
			printf("I found M\n");
			maxSize = (unsigned long)(amount* 1048576);
			printf("MaxSize: %lu \n",maxSize);
			updateCache(newList);
			//printCache(newList);
			if(newList == NULL)
			{
				printf("No indexer file to parse\n");
				return 0;
			}
			takeInput(newList);
			free(suffixSize);
			fclose(fp);
			destroyAllCacheStructs(newList);
			SLDestroy(newList);
			return 0;
			break;
			
			case 'g':
			case 'G':
			printf("I found g\n");
			maxSize = (unsigned long)(amount* 1073741824);
			printf("MaxSize: %lu \n",maxSize);
			updateCache(newList);
			//printCache(newList);
			if(newList == NULL)
			{
				printf("No indexer file to parse\n");
				return 0;
			}
			takeInput(newList);
			free(suffixSize);
			fclose(fp);
			destroyAllCacheStructs(newList);
			SLDestroy(newList);
			return 0;
			break;
			
			default:
				printf("Please enter a valid size ex: KB,MB,GB\n");
		}
		if(maxSize == 0)
		{
			printf("why are you putting %s in cache?\n", argv[2]);
			return 0;
		}
		//free(suffixSize);
		//SLDestroy(newList);
		//fclose(fp);
	}
	else if(argc < 1)
	{
		printf("Wrong amount of arguments");
	}
	else
	{
		char buff[50];
		char* command = "./oldsearch";
		char* indexFile = sprintf(buff, "%s %s", command, argv[1]);
		system(buff);
	}
	
	return 0;
}
