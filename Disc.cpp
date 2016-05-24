#include "struct.h"

bool setPointer(struct Disc& temp){   // выделение памяти
	if (!(temp.name = (wchar_t*)malloc(1024)) || !(temp.let = (wchar_t*)malloc(8)))
	{
		perror("");
		return false;
	}
	return true;
}

bool discCPY(Disc &source, Disc &dest){// копируем информацию о дисках
	if (!setPointer(dest)){
		perror("");
		return false;
	}
	wcscpy(dest.let, source.let);
	wcscpy(dest.name, source.name);
	dest.handle = source.handle;
	return true;
}

bool getHandle(Disc& disc){ // создаём HANDLE диска
	wchar_t diskName[30] = L"\\\\.\\";
	wcscat(diskName, disc.let);
	diskName[6] = L'\0';
	disc.handle = CreateFile(diskName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (disc.handle == NULL || disc.handle == INVALID_HANDLE_VALUE){
		printf("Error: %ul\n", GetLastError());
		return false;
	}
	return true;
}

