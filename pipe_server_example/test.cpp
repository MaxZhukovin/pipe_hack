#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"

#define MAX_PIPE_INST	2

using namespace std;
int main()
{


	char PIPE_NAME[] = "\\\\.\\pipe\\pipe_example";
	HANDLE hEvents[MAX_PIPE_INST];
	PerPipeStruct<unsigned> PipeInfo[MAX_PIPE_INST];
	CPipeServer<unsigned> Pipes[MAX_PIPE_INST];
	char* FName = new char[MAX_PATH], answer;
	ofstream file;
	DWORD PipeNumber, NBytesRead;
	unsigned Message;
	int PipesConnect = 0;
	unsigned mes = 6;

	SetConsoleOutputCP(1251);
	
	for (int i = 0; i < MAX_PIPE_INST; i++){
		hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
		Pipes[i].CreatePipeAndWaitClient(PIPE_NAME, hEvents[i]);

		if (Pipes[i].GetState() == PIPE_ERROR){
			file.close();
			delete[]FName;

			for (i--; i >= 0; i--)
				CloseHandle(hEvents[i]);

			cout << "Ошибка создания экземпляров именованного канала " << PIPE_NAME << endl;
			return 0;
		}
	}

	cout << "Ожидание подключения клиентов..." << endl;

	do{
	
		PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, INFINITE) - WAIT_OBJECT_0;

		if (PipeNumber > MAX_PIPE_INST)
			break;

		if (!Pipes[PipeNumber].GetIOComplete())
			Pipes[PipeNumber].GetPendingResult(NBytesRead);

		if (!Pipes[PipeNumber].GetIOComplete())
			break;

		switch (Pipes[PipeNumber].GetState()){
						
			case PIPE_ERROR:
					
				cout << "Ошибка при работе с каналом! Производится принудительное отсоединение клиента (код ошибки: " << GetLastError() << ")!" << endl;


			case PIPE_CONNECTED:	

				if (Pipes[PipeNumber].GetOperState() == PIPE_JUST_CONNECTED){
					cout << "Just connected" << endl;
					PipesConnect++;
				}

				if (Pipes[PipeNumber].ReadMessage(Message)){

					switch (Pipes[PipeNumber].GetOperState()){
										   
						case PIPE_READ_PART:		
							Pipes[PipeNumber].ReadMessage(Message);
							cout << "Reading data part" << endl;
							break;

						case PIPE_READ_SUCCESS:		
							cout << "Sending data" << endl;			
							Pipes[PipeNumber].WriteMessage(Message);
							break;

						case PIPE_OPERATION_ERROR:	
							cout << "Ошибка при чтении данных из канала (код ошибки: " << GetLastError() << ")!" << endl;
							break;

					}

					break;
				}


			case PIPE_LOST_CONNECT:		

				cout << "Lost connect" << endl;
				PipeInfo[PipeNumber].ClearData();
					
				if (PipesConnect > 0)
					PipesConnect--;

				Pipes[PipeNumber].DisconnectClient();
				Pipes[PipeNumber].WaitClient();

				if (Pipes[PipeNumber].CanClose() == false){
						cout << "В канал не было передано никаких данных со стороны клиента. Повторить попытку чтения данныхY или y - да / любая другая клавиша - нет)?" << endl;
						cin >> answer;

						if (answer == 'Y' || answer == 'y')
							continue;
				}

				break;
		}
				
			


		if (PipesConnect == 0){
			
			cout << "Все клиенты отключены! Продолжить работу (Y или y - да / любая другая клавиша - нет)? ";
			cin >> answer;

			if (answer != 'Y' && answer != 'y')
				break;

			cout << "Ожидание подключения клиентов..." << endl;
		}

	} while (1);


	for (int i = 0; i < MAX_PIPE_INST; i++)
		CloseHandle(hEvents[i]);

	file.close();
	delete[]FName;

	return 0;
}
