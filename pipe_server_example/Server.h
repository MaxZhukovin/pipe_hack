#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>

#include "PipeServer.h"
#include "PerPipeStruct.h"
#include "list_maker.h"

#define MAX_PIPE_INST	2

class server {

private:
	HANDLE hEvents[MAX_PIPE_INST];
	CPipeServer<char[]> Pipes[MAX_PIPE_INST];

	PerPipeStruct processing;
	
	DWORD PipeNumber, NBytesRead;
	
	int PipesConnect;

	bool is_init;

	bool protected_mode;

public:

	server(list<l_p> &input_list, bool protected_mode):
		processing(input_list)
	{
		is_init = false;
		PipeNumber = 0;
		NBytesRead = 0;
		PipesConnect = 0;
		this->protected_mode = protected_mode;
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

		if (is_init) {

			for (int i = 0; i < MAX_PIPE_INST; i++)
				CloseHandle(hEvents[i]);
		}
	}

	void run_kernel( bool(*should_i_continue)(void), void(*pipe_error)(void)) {
	
		if (!is_init)
			return;

		unsigned waiting_for_connect = INFINITE;
		if (protected_mode)
			waiting_for_connect = 100;

		do {

			PipeNumber = WaitForMultipleObjects(MAX_PIPE_INST, hEvents, FALSE, waiting_for_connect) - WAIT_OBJECT_0;

			if (protected_mode)
				check_awaiting();

			if (PipeNumber > MAX_PIPE_INST)
				continue;

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

					pipe_disconnect(PipeNumber);
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

		string inputMessage;

		if (!Pipes[PipeNumber].ReadMessage(inputMessage)) 
			return false;

		if (Pipes[PipeNumber].GetOperState() != PIPE_READ_SUCCESS)
			return false;
			

		l_p* account = NULL;
		bool res = processing.check_account(inputMessage, account);

		if (!res)
			return process_wrong_account(account, PipeNumber);
				
		return send((char*)"1", PipeNumber);
		
	}

	bool process_wrong_account(l_p* account, unsigned PipeNumber) {
		if(!account)
			return send((char*)"2", PipeNumber);

		if (!protected_mode) {
			return send((char*)"0", PipeNumber);
		}

		//set waiting (for protected_mode)
		Pipes[PipeNumber].set_waiting(account);
		ResetEvent(hEvents[PipeNumber]);
		return true;
	}

	bool send(char* message, unsigned PipeNumber) {
		if (!Pipes[PipeNumber].WriteMessage(message))
			return false;

		pipe_disconnect(PipeNumber);
		return true;
	}



	void pipe_disconnect(unsigned num_of_pipe) {

		if (PipesConnect > 0)
			PipesConnect--;

		Pipes[num_of_pipe].DisconnectClient();
		Pipes[num_of_pipe].WaitClient();
	
	}

	void check_awaiting() {
		for (int i = 0; i < MAX_PIPE_INST; i++){
		
			if (Pipes[i].GetState() != PIPE_WAIT_SENDING)
				continue;

			if (Pipes[i].ready_to_send()) {
				send((char*)"0", i);
				pipe_disconnect(i);
			}

		}
		 
	}

};
