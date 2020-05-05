#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"
#include "list.h"

#define MAX_PIPE_INST	2

class server {

private:
	HANDLE hEvents[MAX_PIPE_INST];

	PerPipeStruct processing[MAX_PIPE_INST];
	CPipeServer<char[]> Pipes[MAX_PIPE_INST];

	DWORD PipeNumber, NBytesRead;

	
	int PipesConnect = 0;

	bool is_init;

public:

	server() {
		is_init = false;
		PipeNumber = 0;
		NBytesRead = 0;
	}

	bool init(char PIPE_NAME[]) {

		if (is_init)
			return false;

		for (int i = 0; i < MAX_PIPE_INST; i++) {

			hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
			Pipes[i].CreatePipeAndWaitClient(PIPE_NAME, hEvents[i]);

			if (Pipes[i].GetState() == PIPE_ERROR) {

				for (i--; i >= 0; i--)
					CloseHandle(hEvents[i]);

				return false;
			}
		}

		is_init = true;
		return true;
	}

	~server() {

		for (int i = 0; i < MAX_PIPE_INST; i++)
			CloseHandle(hEvents[i]);
	}

	void run_kernel( bool(*should_i_continue)(void), void(*pipe_error)(void) ) {
	
		if (!is_init)
			return;

		do {

			PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, 1000) - WAIT_OBJECT_0;

			if (PipeNumber > MAX_PIPE_INST)
				continue;

			check_task_list();

			if (!Pipes[PipeNumber].GetIOComplete())
				Pipes[PipeNumber].GetPendingResult(NBytesRead);

			if (!Pipes[PipeNumber].GetIOComplete())
				continue;


			switch (Pipes[PipeNumber].GetState()) {

				case PIPE_ERROR:

					pipe_error();
					break;

				case PIPE_CONNECTED:
					
					if (!pipe_connected())
						pipe_error();

					break;

				case PIPE_LOST_CONNECT:

					pipe_lost_connect();
					break;

			}

			if (PipesConnect == 0) {
				/*if (!should_i_continue())
					break;*/
			}

		} while (1);
	}

private: 


	bool pipe_connected() {

		if (Pipes[PipeNumber].GetOperState() == PIPE_JUST_CONNECTED) 
			PipesConnect++;

		char inputMessage[100] = { 0 };

		if (!Pipes[PipeNumber].ReadMessage(inputMessage, sizeof(inputMessage))) 
			return false;

		if (Pipes[PipeNumber].GetOperState() != PIPE_READ_SUCCESS)
			return false;
			
		if (!processing[PipeNumber].check_value(inputMessage)) {
			Pipes[PipeNumber].set_waiting();
			return true;
		}
				
		if (!Pipes[PipeNumber].WriteMessage((char*)"1"))
			return false;

		pipe_lost_connect();
		return true;
	}

	void pipe_lost_connect() {

		if (PipesConnect > 0)
			PipesConnect--;

		Pipes[PipeNumber].DisconnectClient();
		Pipes[PipeNumber].WaitClient();

	}

	void check_task_list() {
		for (int i = 0; i < MAX_PIPE_INST; i++){
		
			if (Pipes[PipeNumber].GetState() != PIPE_WAIT_SENDING)
				continue;


			if (Pipes[PipeNumber].elapsed_time() > processing[i].get_delay())
			{
				Pipes[PipeNumber].WriteMessage((char*)"0");
				pipe_lost_connect();
			}

		}
		 
	}

};
