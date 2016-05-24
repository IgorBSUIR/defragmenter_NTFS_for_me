#include "users.h"
#include "interface.h"
#include <Windows.h>
#include <stdio.h>
#include <locale.h>
#include <iostream>

int main(){
	setlocale(0, "RUS");
	if (!isUserAdmin())
	{
		printf("run the program as administrator\nCompleted the program\n");
		system("pause");
		return 0;
	}
	char y;
	Disc *allDisc = getDisc();
	getInterface(allDisc);
	printf("Choose disk: ");
	y = getchar();
	system("CLS");
	int choose = y - '0' - 1;
	printf("1) Analisis Disc\n2) Defragmentation\nChoose work: ");
	fflush(stdin);
	y = getchar();
	switch (y){
	case '1':
		wchar_t disc[5];
		wcscpy(disc, allDisc[choose].let);
		wcscat(disc, L"\*");
		startAnalisis(disc);
		break;
	}
	//startAnalisis(L"D:\\Фильмы\\*");
	//printf("%lu", GetLastError());
	system("pause");
	return 0;
}