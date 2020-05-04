#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <iostream>

#include "PipeClient.h"

#define	PIPE_NAME_PREFIX	"\\\\"
#define	PIPE_NAME			"\\\\.\\pipe\\pipe_example"


int main()
{
	
	SetConsoleOutputCP(1251);

	CPipeClient<char[]> PC;
	char* ServerName = new char[MAX_PATH], * PipeName = new char[MAX_PATH],* StartVal = new char[100];

	printf("Введите начальное значение: ");
	std::cin >> StartVal;

	printf("Введите имя сервера (. - для локального компьютера): ");
	std::cin >> ServerName;


	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);


	if (PC.ConnectPipe(PipeName))
	{

		if (PC.WriteMessage(StartVal))
		{
			char Message[100] = { 0 };

			if (PC.ReadMessage(Message, sizeof(Message)))
				std::cout << Message << std::endl;
			else
				printf("Ошибка чтения!\n");
					
		}
		else
			printf("Ошибка записи в именованный канал!\n");
	}
	else
		printf("Ошибка соединения с сервером (код ошибки: %u)! Нажмите любую клавишу для выхода\n", GetLastError());

	delete[]ServerName;
	delete[]PipeName;
	return 0;
}
