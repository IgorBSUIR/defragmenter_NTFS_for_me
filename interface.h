#if !defined INT_111
#define INT_111
#include "struct.h"

#define BUFFER_SIZE 1024*1024

void getSize(struct Disc&);
PVOLUME_BITMAP_BUFFER getVolumeBitmap(HANDLE);
RETRIEVAL_POINTERS_BUFFER* getRPB(HANDLE);
void getPathToFile(wchar_t*);
void getDisc();
#endif