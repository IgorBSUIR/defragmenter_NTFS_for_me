#include "interface.h"

wchar_t* pathToFile = NULL;
bool workThread;
HANDLE mutex1;
HANDLE mutex2;
ULONG size;

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

Disc* getDisc(){ // поиск дисков на компьютере
	struct Disc* allDisc=(Disc*)malloc(10*sizeof(Disc));
	struct Disc temp;
	if (!setPointer(temp)){
		printf("");
		return NULL;
	}
	int i = 0;
	DWORD write;
	HANDLE hFind = FindFirstVolume(temp.name, 1024);

	while (FindNextVolume(hFind, temp.name, 1024)){
		if (!(GetVolumePathNamesForVolumeName(temp.name, temp.let, 8, &write))){
			perror("");
			continue;
		}
		discCPY(temp, allDisc[i]);
		if (!getHandle(allDisc[i])){
			printf("can't open %s", temp.let);
			continue;
		}
		i++;
		allDisc[i].name = NULL;
	}
	FindClose(hFind);
	return allDisc;
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
	RETRIEVAL_POINTERS_BUFFER *rpb;
	
	DWORD byte;
	LONGLONG size1 = sizeof(RETRIEVAL_POINTERS_BUFFER)+(size / 4096) * sizeof(rpb->Extents);
	rpb = (RETRIEVAL_POINTERS_BUFFER *)malloc(size1);
	ZeroMemory(rpb, sizeof(RETRIEVAL_POINTERS_BUFFER));
	int res;

	while (1){
		res = DeviceIoControl(
			hFile,              // handle to file, directory, or volume
			FSCTL_GET_RETRIEVAL_POINTERS,  // dwIoControlCode
			&vib,           // input buffer
			sizeof(vib),         // size of input buffer
			rpb,          // output buffer
			size1,        // size of output buffer
			&byte,     // number of bytes returned
			NULL); //
		if (res == ERROR_MORE_DATA){
			printf("%lu", GetLastError());
			size1 = size1 + (rpb->ExtentCount - rpb->StartingVcn.QuadPart) + sizeof(RETRIEVAL_POINTERS_BUFFER);
			rpb = (RETRIEVAL_POINTERS_BUFFER *)malloc(size1);
			//printf("%ul", GetLastError());
			continue;
		}
		if (!res){
			free(rpb);
			return NULL;
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
			if (!wcscmp(fileInfo.cFileName, L"..")){ 
				continue; 
			}
			//переходим в новую дерикторию
			wchar_t* newDerictory;
			newDerictory = (wchar_t*)malloc((wcslen(directory) + wcslen(fileInfo.cFileName) + 3) * sizeof(wchar_t));
			ZeroMemory(newDerictory, (wcslen(directory) + wcslen(fileInfo.cFileName) + 3) * sizeof(wchar_t));
			wcsncat(newDerictory, directory, wcslen(directory) - 1);
			wcsncat(newDerictory, fileInfo.cFileName, wcslen(fileInfo.cFileName) + 1);
			wcsncat(newDerictory, L"\\*", 3);
			//wprintf(L"Директория: %s\n", newDerictory);
			//ReleaseSemaphore(mutex2, 1, NULL);
			getPathToFile(newDerictory);
			free(newDerictory);
			continue;
		}
		WaitForSingleObject(mutex2, INFINITE);
		if (pathToFile != NULL){
			free(pathToFile);
			pathToFile = NULL;
		}
		pathToFile = (wchar_t*)malloc((wcslen(directory) + wcslen(fileInfo.cFileName)) * sizeof(wchar_t));
		ZeroMemory(pathToFile, (wcslen(directory) + wcslen(fileInfo.cFileName)) * sizeof(wchar_t));
		wcsncat(pathToFile, directory, wcslen(directory)-1);
		wcsncat(pathToFile, fileInfo.cFileName, wcslen(fileInfo.cFileName) + 1); 
		size = fileInfo.nFileSizeLow;
		ReleaseSemaphore(mutex1, 1, NULL);
	}
	printf("\n%lu\n", GetLastError());
}

unsigned int _stdcall getFiles(PVOID param)
{
	wchar_t* directory = (wchar_t*)param;
	getPathToFile(directory);
	workThread = false;
	ReleaseSemaphore(mutex1, 1, NULL);
	_endthreadex(0);
	return 0;
}

void analysis(){
	HANDLE hFile;
	long long int allFiles = 0;
	long long int fragmentsFiles=0;
	while (workThread){
		WaitForSingleObject(mutex1, INFINITE);
		hFile = CreateFile(pathToFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile == INVALID_HANDLE_VALUE){
			printf("can't open: ");
			wprintf(L"%s\n", pathToFile);
			ReleaseSemaphore(mutex2, 1, NULL);
			continue;
		}
		RETRIEVAL_POINTERS_BUFFER* RPB = getRPB(hFile);
		if (RPB == NULL){
			CloseHandle(hFile);
			allFiles += 1;
			ReleaseSemaphore(mutex2, 1, NULL);
			continue;
		}
		if (RPB->ExtentCount > 1){
			wprintf(L"FRA: %s\n", pathToFile);
			fragmentsFiles += 1;
		}
		free(RPB);
		CloseHandle(hFile);
		allFiles += 1;
		ReleaseSemaphore(mutex2,1, NULL);
	}
	printf("ALL: %lld\n", allFiles);
	printf("FRAG: %lld\n", fragmentsFiles);
	double result = fragmentsFiles;
	result /= allFiles;
	result *= 100;
	if (result < 1){
		result>0.5 ? 1 : 0;
	}
	printf("fragmentation: %.0f\n", result);
}

void startAnalisis(wchar_t* disc){
	startThread(disc);
	analysis();
	CloseHandle(mutex1);
	CloseHandle(mutex2);
}

void startThread(wchar_t* disc){
	mutex1 = CreateSemaphore(NULL, 0, 1, L"sem1");
	mutex2 = CreateSemaphore(NULL, 1, 1, L"sem2");
	workThread = true;
	_beginthreadex(NULL, 0, getFiles, (PVOID)disc, 0, 0);
}

void getInterface(Disc *drive){
	for (int i = 0; (drive + i)->name; i++){
		printf("%d) ", i + 1);
		wprintf(L"%s: ", (drive+i)->let);
		getSize(drive[i]);
	}
}