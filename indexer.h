#include "sorted-list.h"

struct Word {
	char* wordArray;
	SortedListPtr filestats;
};
typedef struct Word* WordPtr;

struct Stats {
	char* filename;
	int frequency;
};
typedef struct Stats* StatsPtr;

