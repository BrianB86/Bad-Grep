#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include "sorted-list.c"
#include "sorted-list.h"
#include "indexer.h"
/*
 * Only used when SLInsert returns a 0
 * 
 */

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
/* Second compare method for filestats structs. Made so I can use
 * SLCustomSearch and pass in a pointer to this to search for filenames.
 */ 
int compareStatsByFilename(void* p1, void* p2)
{
	StatsPtr firstStats = (StatsPtr)p1;
	StatsPtr secondStats = (StatsPtr)p2;
	return(strcmp(firstStats->filename, secondStats->filename));
}

/* Pass in a pointer to a char array.
 * Return a pointer to a word struct storing this char array.
 * Initialize the filestats list to empty list.
 */
 
WordPtr createWordStruct(char* word)
{
	WordPtr newWord = (WordPtr)malloc(sizeof(struct Word));
	newWord->wordArray = (char*)malloc(strlen(word)*sizeof(char) + 1);
	strcpy(newWord->wordArray, word);
	newWord->filestats = SLCreate(*compareStatsByFrequency);
	return newWord;
}

/* Pass in a pointer to a char array.
 * Return a pointer to a stats struct initialized to freq. 1.
 */
 
StatsPtr createStatsStruct(char* filename)
{
	StatsPtr newStats = (StatsPtr)malloc(sizeof(struct Stats));
	newStats->filename = (char*)malloc(sizeof(char)*(strlen(filename) + 1));
	strcpy(newStats->filename, filename);
	newStats->frequency = 1;
	return newStats;
}
/* Destroy a stats struct. Free all associated memory.
 * 
 */ 
void destroyStatsStruct(StatsPtr statsStruct)
{
	if(statsStruct == NULL)
	{
		return;
	}
	else
	{
		free(statsStruct->filename);
		free(statsStruct);
		return;
	}
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
			}
			SLDestroy(wordStruct->filestats);
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

/* Extension of SLSearch, because I don't want to be messy and creating
 * structs to search for words inside of other methods. Keep that here.
 */ 
int SLSearchWord(SortedListPtr head, char* word)
{
	WordPtr target = createWordStruct(word);
	void *castToSearch = (void*)target;
	int result = SLSearch(head,castToSearch);
	destroyWordStruct(target);
	return result;
}
/* Uses SLCustomSearch to search the list of filestats structs for the
 * given filename.
 */ 
int SLSearchFile(SortedListPtr head, char* filename)
{
	StatsPtr newStats = createStatsStruct(filename);
	int result = SLCustomSearch(head,newStats, *compareStatsByFilename);
	destroyStatsStruct(newStats);
	return result;
}

/* Pass in a pointer to a char array for the word and one for the name
 * of the file the word was found in. Search the given list for the word.
 * If it is found, search the filestats list for the filename. If it is 
 * not found, then add it to the list. If it was found, search filestats
 * for the filename. Increment the frequency by one.
 */
 
int incrementFrequency(SortedListPtr head, char* word, char* filename)
{
	int result = SLSearchWord(head, word);
	if(result == -1) //Word not found. Add it to the list.
	{
		//Create the new word struct.
		WordPtr newWord = createWordStruct(word);
		StatsPtr newStats = createStatsStruct(filename);
		//Add this new word struct to the list.
		int addWord = SLInsert(head, newWord);
		//Add the new stats to this new word.
		SLInsert(newWord->filestats, newStats);
		if(addWord == 1) //Success.
		{
			printf("Added %s to the word list.\n",word);
			return 1;
		}
		else //Failure to add.
		{
			printf("Unable to insert %s to the word list.\n",word);
			return -1;
		}
	}
	else //Word found. Increment frequency for the filename.
	{
		//Get the node the word is stored in.
		SLNode wordNode = SLGetIndex(head,result);
		WordPtr wordData = (WordPtr)wordNode->value;
		//Find if the file is already in this word's stats list.
		int filestatsResult = SLSearchFile(wordData->filestats, filename);
		if(filestatsResult == -1) //Given filename not found in stats for word.
		{
			//Add the file to the list.
			StatsPtr newStats = createStatsStruct(filename);
			int insert = SLInsert(wordData->filestats, newStats);
			if(insert == 0)//need to force it in manually
			{
				SLInsertIndex(wordData->filestats, newStats);
				return 1;
			}
		}
		else //Given filename found in the stats for the word. Increment.
		{
			//Get the node at the index that the filename stats were found at.
			SLNode statsNode = SLGetIndex(wordData->filestats, filestatsResult);
			StatsPtr stats = (StatsPtr)statsNode->value;
			stats->frequency = stats->frequency + 1;
			//after adding, need to re-sort the array
			wordData->filestats = mergeHelper(wordData->filestats);
			return 1;
		}
	}
	return 0;
}
/* Given a file pointer, return information about how large the file is.
 * 
 */ 
int filesize(FILE *fp){
	int size;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return size;
}
/* Given a file pointer, read in the file and return its information
 * as a char array.
 */ 
char* readFile(FILE *fp){
	//Find the size of the file.
	int size = filesize(fp);
	//Malloc out that much memory.
	char *inputArray = malloc((sizeof(char)*size)+1);
	int curChar;
	int i = 0;
	//Loop over every character in the file, add it to the array we malloc'ed.
	while(curChar != EOF)
	{
		curChar = fgetc(fp);
		curChar = tolower(curChar);
		inputArray[i] = curChar;
		i++;
	}
	inputArray[i-1]='\0';
	//Close the file pointer.
	fclose(fp);
	//Return the data that was in the file.
	return inputArray;
}
/* Given a list of indexes so far, the data for a file, and the name of that file
 * tokenize the file and attempt to add every token to the given list.
 */ 
void parseFileData(SortedListPtr list, char* filedata, char* filename)
{
	static char *delims = " -=+_~!@#$%^&*()[];':',./<>?";
	char *result = NULL;
	result = strtok(filedata, delims);
	//Tokenize the array until there are no more tokens left.
	while(result != NULL)
	{
		//Result will store the latest token.
		//Attempt call incrementFrequency passing this token.
		incrementFrequency(list, result, filename);
		result = strtok(NULL, delims);
	}
	return;
}
/* Given a directory or filename.
 * If a directory, go through the directory and all subdirectories and call
 * readFile to get the file information into a char array. Then, call parseFileData
 * to return a list 
 */ 
SortedListPtr dirwalk(SortedListPtr newList, const char* dir_name)
{
    DIR* dir;
    //Attempt to open the directory.
    //printf("Attempting to open directory/file %s\n",dir_name);
    dir = opendir(dir_name);
    if (dir == NULL) { //If this returns null, it is most likely a file.
		printf("Selected dir_name is NOT a folder.\n");
		FILE *fp = fopen(dir_name, "r");
		if(fp == NULL) //If it is neither a directory, nor a file, then it is an error.
		{
			printf("Unable to open %s\n",dir_name);
			return newList;
		}
		else
		{
			//dir_name is the NAME OF A FILE.
			//readFile to get the file information.
			//parseFileData to tokenize it and add the data to the list.
			char *input = readFile(fp);
			char *directoryName = (char*)malloc(sizeof(char)*(strlen(dir_name) + 1));
			strcpy(directoryName, dir_name);
			parseFileData(newList, input, directoryName);
			free(input);
			free(directoryName);
			//fclose(fp);
		}
		return newList;
    }
    while (1) {
        struct dirent *dirInfo;
        const char *nextDirName;
        //Get next entry from the directory.
        dirInfo = readdir (dir);
        if (dirInfo == NULL)
        {
            break;
        }
        nextDirName = dirInfo->d_name;
        //Check for subdirectories.
        if (dirInfo->d_type == DT_DIR) {
			//printf("%s is a folder.\n", nextDirName);
            if (strcmp (nextDirName, "..") != 0 && strcmp (nextDirName, ".") != 0) 
            {
				//Build the path to the next subdirectory.
                int path_length;
                char path[4096];
                //Save the path into the char array called path.
                path_length = snprintf (path, 4096, "%s/%s", dir_name, nextDirName);
                //printf ("FOLDER: %s\n", path);
                if (path_length >= 4096)
                {
                    printf("Path length has grown too long. Error!\n");
                    return newList;
                }
                //Call recursively for next path
                
                dirwalk(newList, path);
            }
        }
        else
        {
			printf("File %s found, attempting to open.\n", dir_name);
			//nextDirName is the NAME OF A FILE.
			//readFile to get the file information.
			//parseFileData to tokenize it and add the data to the list.
			if (strcmp (nextDirName, "..") != 0 && strcmp (nextDirName, ".") != 0) 
            {
				char path[4096];
                //Save the path into the char array called path.
                int path_length = snprintf (path, 4096, "%s/%s", dir_name, nextDirName);
                if (path_length >= 4096)
                {
                    printf("Path length has grown too long. Error!\n");
                    return newList;
                }
				FILE *fp = fopen(path,"r");
				char *input = readFile(fp);
				printf("Input: %s\n",input);
				char *directoryName = (char*)malloc(sizeof(char)*strlen(nextDirName) + 1);
				strcpy(directoryName, nextDirName);
				parseFileData(newList,input,directoryName);
				free(directoryName);
				free(input);
				//fclose(fp);
			}
		}
    }
    //Close the directory and return.
    closedir(dir);
    return newList;
}
/* Given the head of a linked list containing index data and an output file
 * write out all the information to the given file.
 */ 
void exportData(SortedListPtr head, char* outputFile)
{
	FILE *fp = fopen(outputFile, "w");
	if(fp == NULL)
	{
		printf("Unable to write to file %s.\n", outputFile);
		return;
	}
	SortedListIteratorPtr outerIter = SLCreateIterator(head);
	do
	{
		//Grab the current word struct.
		WordPtr curWord = (WordPtr)outerIter->curNode->value;
		//Write out the word to the file.
		fprintf(fp,"%s %s\n","<List>", curWord->wordArray);
		int numStats = 0;
		SortedListIteratorPtr statsIter = SLCreateIterator(curWord->filestats);
		do
		{
			//Get the current stats struct.
			StatsPtr curStats = (StatsPtr)statsIter->curNode->value;
			//Start writing out all the statistics.
			fprintf(fp, "%s %d ", curStats->filename, curStats->frequency);
			//They said they only want 5 stats per line.
			if(numStats == 5)
			{
				fprintf(fp, "\n");
				numStats = 0;
			}
			else
			{
				numStats++;
			}
		} while(SLNextItem(statsIter) != NULL);
		SLDestroyIterator(statsIter);
		fprintf(fp, "\n</List>\n");
	} while(SLNextItem(outerIter) != NULL);
	SLDestroyIterator(outerIter);
	fclose(fp);
}
int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("Requires execution in following format: ./indexer <output file> <input file or directory>\n");
		return 1;
	}
	SortedListPtr newList = SLCreate(*compareWordStruct);
	newList = dirwalk(newList, argv[2]);
	printf("List built, begin parsing.\n");
	//parseFileData(newList, input, argv[2]);
	printf("Parsing complete, begin export.\n");
	if(newList->headOfList == NULL) //Fix for Empty directories.
	{
		printf("Empty input and output list.\n");
		SLDestroy(newList);
		return 1;
	}
	exportData(newList, argv[1]);
	destroyAllWordStructs(newList);
	SLDestroy(newList);
	return 1;
}
