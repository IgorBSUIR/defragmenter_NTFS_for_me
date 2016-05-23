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
	
	getPathToFile(L"F:\\*");
	std::cout<<GetLastError();
	getchar();
	return 0;
}

