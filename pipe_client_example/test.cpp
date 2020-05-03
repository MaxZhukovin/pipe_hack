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
	unsigned StartVal, EndVal, Divider;

	SetConsoleOutputCP(1251);

	printf("������� ��������� ��������: ");
	scanf("%u", &StartVal);

	CPipeClient<unsigned> PC;
	char* ServerName = new char[MAX_PATH], * PipeName = new char[MAX_PATH];
	printf("������� ��� ������� (. - ��� ���������� ����������): ");
	scanf("%s", ServerName);
	strcat(strcat(strcpy(PipeName, PIPE_NAME_PREFIX), ServerName), PIPE_NAME);

	if (PC.ConnectPipe(PipeName))
	{

		if (PC.WriteMessage(StartVal))
		{
			
			if (!PC.WriteMessage(StartVal))
				printf("������ ������ � ����������� �����!\n");


			unsigned Message = 0;
			if (PC.ReadMessage(Message))			
				std::cout << Message << std::endl;
			else
				printf("������ ������!\n");
					
		}
		else
			printf("������ ������ � ����������� �����!\n");
	}
	else
		printf("������ ���������� � �������� (��� ������: %u)! ������� ����� ������� ��� ������\n", GetLastError());

	delete[]ServerName;
	delete[]PipeName;
	return 0;
}
