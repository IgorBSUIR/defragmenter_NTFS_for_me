#if !defined INT_111
#define INT_111
#include "struct.h"
#include <process.h>

#define BUFFER_SIZE 1024*1024

void getSize(struct Disc&);
PVOLUME_BITMAP_BUFFER getVolumeBitmap(HANDLE);
RETRIEVAL_POINTERS_BUFFER* getRPB(HANDLE);
void getPathToFile(wchar_t*);
Disc* getDisc();
void startThread(wchar_t*);
void startAnalisis(wchar_t*);
unsigned int _stdcall getFiles(PVOID param);
void analysis();
void getInterface(Disc*);
#endif