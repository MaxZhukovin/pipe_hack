#ifndef UNICODE  
typedef std::string String;
#else
typedef std::wstring String;
#endif

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

#define BUF_SIZE					4096

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
	DWORD waiting_bytes;
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
			// ����������� �������� � �������� ����������
		case ERROR_IO_PENDING:			PipeCurOperState = PIPE_NO_OPERATION;
			fPendingIOComplete = false;

			break;

			// ������ ���������� �� ������������ ������
		case ERROR_BROKEN_PIPE:			PipeState = PIPE_LOST_CONNECT;
			fPendingIOComplete = true;
			break;

			// ��������� ������ ��� ������ � ������� 
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

		waiting_bytes = 0;

		arrival_time = 0;
		account = NULL;
	}

	~CPipeServer()
	{
		if (hPipe != INVALID_HANDLE_VALUE){
			// ������ ���� ����������� ��������
			CancelIo(hPipe);
			CloseHandle(hPipe);
		}
		/*
		������������ � ������ ������������� ������, ���������� � ������� ������� HeapAlloc ��� ���������� ������������, ������� �������������� � ������� ������� HeapFree, �������� ������� ������ � ����� ���������� winbase.h ��������� �������:
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
		��������� ������ ��� ���������� ������������ � ������� ������� HeapAlloc, ��������
		������� ������ � ����� ���������� winbase.h ��������� �������:
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
			���� ������������� � ���������� ������� DACL ����������� ������������ ������ �������, ��
			������������ �������� � ��������������� ������������� ���������� ������ SECURITY_ATTRIBUTES,
			������� ����� ������������ ��� ������ ������� CreateNamedPipe
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
			// �������� � ������ ���
			PipeCurOperState = PIPE_NO_OPERATION;
			ClearOVERL();

			// ����������� �������� ����������� ������� � ������
			if (ConnectNamedPipe(hPipe, &Overl) == FALSE)
			{
				switch (GetLastError())
				{
					// ����������� �������� ����������� ������� � ������ � ����������� ������
				case ERROR_IO_PENDING:		fPendingIOComplete = false;
					return PipeState;

					// ������ ����������� � ������																
				case ERROR_PIPE_CONNECTED:
					// ������� ������� � ��������� ���������
					SetEvent(Overl.hEvent);
					PipeState = PIPE_CONNECTED;
					CanCloseFlag = false;
					fPendingIOComplete = true;
					return PipeState;

				}
			}
			/*
			�������� ����������� ������� � ������ � ����������� ������ ��������� ��������
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
	bool ReadMessage(String &Message)
	{

		if (IsOpen())
		{

			if (get_message(Message)){

				CanCloseFlag = true;			
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


	bool get_message(String &msg) {

		
		DWORD read_bytes = 0;
		TCHAR input[BUF_SIZE +1] = {0}; //with zero symbol

		if (ReadFile(hPipe, input, BUF_SIZE, &read_bytes, &Overl) != TRUE)
			return false;

		if (PipeCurOperState != PIPE_READ_PART)
		{ 
			unsigned mess_size = *(unsigned*)input;
			if (mess_size + 4  > BUF_SIZE)
			{
				this->waiting_bytes = mess_size + 4 - BUF_SIZE;
				PipeCurOperState = PIPE_READ_PART;
			}else
				PipeCurOperState = PIPE_READ_SUCCESS;
			
			msg += input + 4; //without length	
			return true;
		}

		if (BUF_SIZE >= waiting_bytes) {
			waiting_bytes = 0;
			PipeCurOperState = PIPE_READ_SUCCESS;
		}
		else
			this->waiting_bytes -= BUF_SIZE;


		msg += input;
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
	bool WriteMessage(String Message)
	{

		if (WriteFile(hPipe, Message.c_str(), Message.length(), NULL, &Overl) == 0)
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
