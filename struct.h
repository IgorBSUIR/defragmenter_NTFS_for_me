#if  !defined STRUCT_22
#define STRUCT_22
#include <Windows.h>
#include <stdio.h>
#include "interface.h"
struct Disc{
	HANDLE handle;
	wchar_t *name;
	wchar_t *let;
};

bool setPointer(struct Disc&);

bool discCPY(struct Disc&, struct Disc&);

bool getHandle(struct Disc&);
#endif