#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"

#define MAX_PIPE_INST	2

class server {

private:
	HANDLE hEvents[MAX_PIPE_INST];

	PerPipeStruct<unsigned> PipeInfo[MAX_PIPE_INST];
	CPipeServer<char[]> Pipes[MAX_PIPE_INST];

	char answer;
	DWORD PipeNumber, NBytesRead;

	char inputMessage[100] = {0};
	int PipesConnect = 0;

public:
	server() = delete;

	server(char PIPE_NAME[]) {

		answer = '\0';
		PipeNumber = 0; 
		NBytesRead = 0;
		//inputMessage ;

		for (int i = 0; i < MAX_PIPE_INST; i++) {

			hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
			Pipes[i].CreatePipeAndWaitClient(PIPE_NAME, hEvents[i]);

			if (Pipes[i].GetState() == PIPE_ERROR) {

				for (i--; i >= 0; i--)
					CloseHandle(hEvents[i]);

				cout << "Ошибка создания экземпляров именованного канала " << PIPE_NAME << endl;
			}
		}
	}

	~server() {

		for (int i = 0; i < MAX_PIPE_INST; i++)
			CloseHandle(hEvents[i]);
	}

	void run_kernel() {
		cout << "Ожидание подключения клиентов..." << endl;

		do {
			PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, 500) - WAIT_OBJECT_0;

			if (PipeNumber > MAX_PIPE_INST)
				continue;

			if (!Pipes[PipeNumber].GetIOComplete())
				Pipes[PipeNumber].GetPendingResult(NBytesRead);

			if (!Pipes[PipeNumber].GetIOComplete())
				break;

			switch (Pipes[PipeNumber].GetState()) {

				case PIPE_ERROR:
					pipe_error();
					break;

				case PIPE_CONNECTED:
					pipe_connected();
					break;

				case PIPE_LOST_CONNECT:

					pipe_lost_connect();
					break;
			}




			/*if (!should_i_continue())
				break;*/

		} while (1);
	}

private: 

	bool should_i_continue() {
		if (PipesConnect == 0) {

			cout << "Все клиенты отключены! Продолжить работу (Y или y - да / любая другая клавиша - нет)? ";
			cin >> answer;

			if (answer != 'Y' && answer != 'y')
				return 0;

			cout << "Ожидание подключения клиентов..." << endl;
		}

		return 1;
	}

	void pipe_error() {
		cout << "Ошибка при работе с каналом! Производится принудительное отсоединение клиента (код ошибки: " << GetLastError() << ")!" << endl;

	}

	void pipe_connected() {
		if (Pipes[PipeNumber].GetOperState() == PIPE_JUST_CONNECTED) {
			cout << "Just connected" << endl;
			PipesConnect++;
		}

		if (Pipes[PipeNumber].ReadMessage(inputMessage, sizeof(inputMessage))) {

			switch (Pipes[PipeNumber].GetOperState()) {

			case PIPE_READ_PART:
				Pipes[PipeNumber].ReadMessage(inputMessage, sizeof(inputMessage));
				cout << "Reading data part" << endl;
				break;

			case PIPE_READ_SUCCESS:
				cout << "Sending data" << endl;
				
				Pipes[PipeNumber].WriteMessage(inputMessage);
				break;

			case PIPE_OPERATION_ERROR:
				cout << "Ошибка при чтении данных из канала (код ошибки: " << GetLastError() << ")!" << endl;
				break;

			}

		}
	}

	bool pipe_lost_connect() {
		cout << "Lost connect" << endl;
		PipeInfo[PipeNumber].ClearData();

		if (PipesConnect > 0)
			PipesConnect--;

		Pipes[PipeNumber].DisconnectClient();
		Pipes[PipeNumber].WaitClient();

		if (Pipes[PipeNumber].CanClose() == false) {
			cout << "В канал не было передано никаких данных со стороны клиента. Повторить попытку чтения данных Y или y - да / любая другая клавиша - нет)?" << endl;
			cin >> answer;

			if (answer == 'Y' || answer == 'y')
				return true;
		}

		return false;
	}

};
