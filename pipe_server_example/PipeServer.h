#ifndef PIPE_SERVER_INCLUDE
#define PIPE_SERVER_INCLUDE

#include <windows.h>

#define	PIPE_ERROR					-1
#define PIPE_NOT_CONNECTED			1
#define	PIPE_CONNECTED				2
#define	PIPE_LOST_CONNECT			3

#define	PIPE_OPERATION_ERROR		-2
#define	PIPE_JUST_CONNECTED			10
#define	PIPE_NO_OPERATION			11

#define	PIPE_READ_SUCCESS			12
#define	PIPE_READ_PART				13
#define PIPE_WAIT_SENDING			14

#define DEF_BUF_SIZE				4096
#define DEF_WAIT_TIME				20000

#define buf_size					4096

//------------------------------------------------------------------

template <class T> 
class CPipeServer
{

private:
	//-----------------------
	HANDLE hPipe;
	OVERLAPPED Overl;
	DWORD PipeState;

	//-----------------------
	bool fPendingIOComplete;
	bool CanCloseFlag;

	//-----------------------
	DWORD PipeCurOperState;

	//-----------------------
	PSECURITY_DESCRIPTOR pSD;

	//-----------------------
	unsigned arrival_time;
	l_p* account;

private:

	void CheckError()
	{
		DWORD Error = GetLastError();

		switch (Error)
		{
			// Асинхронная операция в процессе выполнения
		case ERROR_IO_PENDING:			PipeCurOperState = PIPE_NO_OPERATION;
			fPendingIOComplete = false;

			break;

			// Клиент отключился от именованного канала
		case ERROR_BROKEN_PIPE:			PipeState = PIPE_LOST_CONNECT;
			fPendingIOComplete = true;
			break;

			// Произошла ошибка при работе с каналом 
		default:						PipeCurOperState = PIPE_OPERATION_ERROR;
			fPendingIOComplete = true;
			break;

		}
	}

	void ClearOVERL()
	{
		Overl.Offset = Overl.OffsetHigh = Overl.Internal = Overl.InternalHigh = 0;
	}


public:

	CPipeServer()
	{
		hPipe = INVALID_HANDLE_VALUE;
		PipeState = PIPE_NOT_CONNECTED;
		PipeCurOperState = PIPE_NO_OPERATION;
		ClearOVERL();
		fPendingIOComplete = true;
		Overl.hEvent = NULL;
		CanCloseFlag = false;
		pSD = NULL;

		arrival_time = 0;
		account = NULL;
	}

	~CPipeServer()
	{
		if (hPipe != INVALID_HANDLE_VALUE){
			// Отмена всех асинхронных операций
			CancelIo(hPipe);
			CloseHandle(hPipe);
		}
		/*
		Освобождение в случае необходимости памяти, выделенной с помощью функции HeapAlloc под дескриптор безопасности, которое осуществляется с помощью функции HeapFree, прототип которой описан в файле заголовков winbase.h следующим образом:
		BOOL HeapFree(
			HANDLE hHeap,
			DWORD dwFlags,
			LPVOID lpMem
		);
		*/
		if (pSD)
			HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pSD);
	}

	//-------------------------------------------------------------
	DWORD CreatePipeAndWaitClient(char* PipeName, HANDLE hEvent, bool ByteMode = true, DWORD BufSize = DEF_BUF_SIZE, DWORD DefWaitTime = DEF_WAIT_TIME)
	{
		if (!hEvent)
			return PipeState;

		ClearOVERL();
		Overl.hEvent = hEvent;
		/*
		Выделение памяти под дескриптор безопасности с помощью функции HeapAlloc, прототип
		которой описан в файле заголовков winbase.h следующим образом:
		LPVOID HeapAlloc(
			HANDLE hHeap,
			DWORD dwFlags,
			SIZE_T dwBytes
		);
		*/

		if (!pSD)
			pSD = HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, SECURITY_DESCRIPTOR_MIN_LENGTH);

		if (pSD && InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION) && SetSecurityDescriptorDacl(pSD, TRUE, NULL, FALSE)){
		
			/*
			Если инициализация и заполнение данными DACL дескриптора безопасности прошли успешно, то
			производится описание и соответствующая инициализация экземпляра класса SECURITY_ATTRIBUTES,
			который будет задействован при вызове функции CreateNamedPipe
			*/

			SECURITY_ATTRIBUTES sa = { sizeof(sa), pSD, true };

			hPipe = CreateNamedPipe(PipeName,
				PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
				ByteMode ? PIPE_TYPE_BYTE | PIPE_READMODE_BYTE : PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES,
				BufSize,
				BufSize,
				DefWaitTime,
				&sa
			);

			PipeState = WaitClient();
		}
		

		return PipeState;
	}

	DWORD WaitClient()
	{
		if (IsOpen())
		{
			// Операций в канале нет
			PipeCurOperState = PIPE_NO_OPERATION;
			ClearOVERL();

			// Асинхронное ожидание подключения клиента к каналу
			if (ConnectNamedPipe(hPipe, &Overl) == FALSE)
			{
				switch (GetLastError())
				{
					// Выполняется ожидание подключения клиента к каналу в асинхронном режиме
				case ERROR_IO_PENDING:		fPendingIOComplete = false;
					return PipeState;

					// Клиент подключился к каналу																
				case ERROR_PIPE_CONNECTED:
					// Перевод события в свободное состояние
					SetEvent(Overl.hEvent);
					PipeState = PIPE_CONNECTED;
					CanCloseFlag = false;
					fPendingIOComplete = true;
					return PipeState;

				}
			}
			/*
			Ожидание подключения клиента к каналу в асинхронном режиме завершено неудачно
			*/
			CanCloseFlag = false;
			fPendingIOComplete = true;
		}
		PipeState = PIPE_ERROR;

		return PipeState;
	}

	DWORD DisconnectClient()
	{
		if (IsOpen() && DisconnectNamedPipe(hPipe))
		{
			PipeState = PIPE_NOT_CONNECTED;
			return WaitClient();
		}
		else
		{
			fPendingIOComplete = true;
			return (PipeState = PIPE_ERROR);
		}
	}
	
	//-------------------------------------------------------------
	bool ReadMessage(string &Message)
	{

		if (IsOpen())
		{

			if (get_message(Message)){
				CanCloseFlag = true;

				PipeCurOperState = PIPE_READ_SUCCESS; 
				fPendingIOComplete = false;
				arrival_time = GetTickCount();

				return true;
			}
			else
				CheckError();
		}

		return false;
	}
private:
	bool get_message(string &msg) {

		
		DWORD read_bytes = 0;
		char input[buf_size + 5] = {0}; //with zero symbol

		if (ReadFile(hPipe, input, buf_size + 4, &read_bytes, &Overl) != TRUE)
			return false;

		int mess_size = int((unsigned char)(input[0])  |
							(unsigned char)(input[1]) << 8 |
							(unsigned char)(input[2]) << 16 |
							(unsigned char)(input[3]) <<24);
		
		//---------------------------------------

		msg = input+4; //without length

		if (mess_size + 4 <= read_bytes)
			return true;


		//if read part
		while (read_bytes < mess_size + 4) {

			*input = { 0 };
			DWORD next_bytes = 0;
			if (ReadFile(hPipe, input, mess_size + 4 - read_bytes, &next_bytes, &Overl) != TRUE)
				return false;

			read_bytes += next_bytes;
			msg += input;
		}
	

		return true;
	}



public:
	bool GetPendingResult(DWORD& NBytesRead)
	{
		if (IsOpen() && GetOverlappedResult(hPipe, &Overl, &NBytesRead, FALSE) == TRUE)
		{
			if (PipeState == PIPE_NOT_CONNECTED)
			{
				PipeState = PIPE_CONNECTED;
				PipeCurOperState = PIPE_JUST_CONNECTED;
			}
			else
				CanCloseFlag = true;


			fPendingIOComplete = true;
			return true;
		}
		else
			CheckError();

		return false;
	}
	
	//-------------------------------------------------------------
	bool IsOpen()
	{
		return hPipe != INVALID_HANDLE_VALUE;
	}

	DWORD GetState()
	{
		return PipeState;
	}

	DWORD GetOperState()
	{
		return PipeCurOperState;
	}

	bool GetIOComplete()
	{
		return fPendingIOComplete;
	}

	bool CanClose()
	{
		return CanCloseFlag;
	}

	//-------------------------------------------------------------
	bool WriteMessage(char* Message)
	{

		if (WriteFile(hPipe, Message, strlen(Message), NULL, &Overl) == 0)
			return false;

		return true;
	}

	void set_waiting(l_p* account) {
		this->PipeState = PIPE_WAIT_SENDING;
		this->account = account;
	}
	
	bool ready_to_send() {

		if (!arrival_time || !account)
			return true;

		if (GetTickCount() - arrival_time > account->delay) {

			arrival_time = 0;
			account = NULL;
			return true;
		}
		return false;
	}
};

#endif
