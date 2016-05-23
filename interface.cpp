#include "interface.h"

wchar_t pathToFile;

void getSize(struct Disc &disc){  // получаем размер диска
	PARTITION_INFORMATION_EX pex;
	DWORD ret = 0;
	if (disc.handle == NULL || disc.handle == INVALID_HANDLE_VALUE){
		return;
	}
	if ((DeviceIoControl(disc.handle, IOCTL_DISK_GET_PARTITION_INFO_EX, NULL, 0, &pex, sizeof(pex), &ret, (LPOVERLAPPED)NULL))){
		printf("%lli bytes\n", pex.PartitionLength);
	}
	else{
		printf("can't get size");
	}
}

void getDisc(){ // поиск дисков на компьютере
	struct Disc allDisc[10];
	struct Disc temp;
	if (!setPointer(temp)){
		printf("");
		return;
	}
	int i = 0;
	DWORD write;
	HANDLE hFind = FindFirstVolume(temp.name, 1024);

	while (FindNextVolume(hFind, temp.name, 1024)){
		if (!(GetVolumePathNamesForVolumeName(temp.name, temp.let, 8, &write))){
			perror("");
			return;
		}
		discCPY(temp, allDisc[i]);
		if (!getHandle(allDisc[i])){
			printf("can't open %s", temp.let);
			continue;
		}
		discShow(allDisc[i]);
		getSize(allDisc[i]);
		i++;
	}

	FindClose(hFind);
}

PVOLUME_BITMAP_BUFFER getVolumeBitmap(HANDLE hDisk){ // получаем информацию о класстерах и маску пространства
	STARTING_LCN_INPUT_BUFFER InBuffer;
	VOLUME_BITMAP_BUFFER OutBuffer;
	PVOLUME_BITMAP_BUFFER pOutBuffer;
	DWORD returnedByte;
	LONGLONG countOfCluster;
	LONGLONG size;
	int ret;
	InBuffer.StartingLcn.QuadPart = 0;
	ret = DeviceIoControl(hDisk, FSCTL_GET_VOLUME_BITMAP, &InBuffer, sizeof(InBuffer), &OutBuffer, sizeof(OutBuffer), &returnedByte, NULL);
	if (!ret && GetLastError() == ERROR_MORE_DATA){
		countOfCluster = OutBuffer.BitmapSize.QuadPart - OutBuffer.StartingLcn.QuadPart;
		size = countOfCluster + sizeof(VOLUME_BITMAP_BUFFER);
		pOutBuffer = (PVOLUME_BITMAP_BUFFER)malloc(size);
		pOutBuffer->StartingLcn.QuadPart = 0;
		ret = DeviceIoControl(hDisk, FSCTL_GET_VOLUME_BITMAP, &InBuffer, sizeof(InBuffer), pOutBuffer, size, &returnedByte, NULL);
		if (ret){
			return pOutBuffer;
		}
	}
	else{
		if (!ret){
			return NULL;
		}
		return &OutBuffer;
	}
	return NULL;
}

RETRIEVAL_POINTERS_BUFFER* getRPB(HANDLE hFile){
	STARTING_VCN_INPUT_BUFFER vib;
	vib.StartingVcn.QuadPart = 0;
	RETRIEVAL_POINTERS_BUFFER *rpb = (RETRIEVAL_POINTERS_BUFFER *)malloc(sizeof(RETRIEVAL_POINTERS_BUFFER));
	DWORD byte;
	LONGLONG size = sizeof(*rpb);
	int res;
	LARGE_INTEGER a;
	a.QuadPart = 0;
	while (1){
		res = DeviceIoControl(
			hFile,              // handle to file, directory, or volume
			FSCTL_GET_RETRIEVAL_POINTERS,  // dwIoControlCode
			&vib,           // input buffer
			sizeof(vib),         // size of input buffer
			rpb,          // output buffer
			size,        // size of output buffer
			&byte,     // number of bytes returned
			NULL); //
		if (!res){
			size = size + (rpb->ExtentCount - rpb->StartingVcn.QuadPart) + sizeof(RETRIEVAL_POINTERS_BUFFER);
			printf("%ul", GetLastError());
			continue;
		}
		if (rpb->ExtentCount != 1){
			vib.StartingVcn = rpb->Extents->NextVcn;
			continue;
		}
		break;
	}
	return rpb;
}

void getPathToFile(wchar_t* directory){

	static WIN32_FIND_DATA fileInfo;

	HANDLE hFind = FindFirstFile(directory, &fileInfo);

	while (FindNextFile(hFind, &fileInfo)){
		if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0){
			continue;
		}
		if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0){
			if (!wcscmp(fileInfo.cFileName, L"..")){ continue; }
				wprintf(L"Папка: %s\n", fileInfo.cFileName);
				//переходим в новую дерикторию
				wchar_t* newDerictory;
				newDerictory = (wchar_t*)malloc((wcslen(directory) + wcslen(fileInfo.cFileName) + 3) * sizeof(wchar_t));
				ZeroMemory(newDerictory, (wcslen(directory) + wcslen(fileInfo.cFileName) + 3) * sizeof(wchar_t));
				int i = 0;
				for (i = 0; i < lstrlenW(directory) - 1; i++){
					newDerictory[i] = directory[i];
				}
				wcsncat(newDerictory, fileInfo.cFileName, wcslen(fileInfo.cFileName) + 1);
				wcsncat(newDerictory, L"\\*", 3);
				wprintf(L"Директория: %s\n", newDerictory);
				getPathToFile(newDerictory);
				free(newDerictory);
			}
		wprintf(L"Файл: %s\n", fileInfo.cFileName);

	}
	WaitForSingleObject(hFind, INFINITE);
}