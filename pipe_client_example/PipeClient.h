#ifndef PIPE_CLIENT_INCLUDE
#define PIPE_CLIENT_INCLUDE

#include <windows.h>

#include <chrono>
#include <thread>


#define	PIPE_READ_SUCCESS			12
#define	PIPE_READ_PART				13

template <class T> 
class CPipeClient
{

private:
	OVERLAPPED Overl;
	HANDLE hPipe;

public:

	//------------------------------------------------------------------
	CPipeClient()
	{
		hPipe = INVALID_HANDLE_VALUE;
		Overl.hEvent = NULL;
	}

	~CPipeClient()
	{
		if (hPipe != INVALID_HANDLE_VALUE)
			CloseHandle(hPipe);
	}

	//------------------------------------------------------------------
	bool ConnectPipe(char* PipeName, bool WaitInfinite = false)
	{
		/*
		����������� ���� ������� �����������
		*/
		do
		{
			/*
			������� ����������� � ������
			*/

			hPipe = CreateFile(PipeName,
				GENERIC_READ |
				GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
			);

			if (!IsPipeConnected())
			{
				if (GetLastError() == ERROR_PIPE_BUSY)
				{
					/*
					���� ����������� �� ��������� �� ������� ��������� �������, �� ���������� �������� ���
					������������ (� ������ ����������� ��������   �����) � ������� �� ��������� �����������
					*/

					if (!WaitNamedPipe(PipeName, WaitInfinite ? NMPWAIT_WAIT_FOREVER : NMPWAIT_USE_DEFAULT_WAIT))
						return false;
				}
				else
					// ������ ������ ��� �����������, �������������, ��� �� �������
					return false;
			}
			else
				// ������� �����������
				break;
		} while (1);

		return true;
	}

	bool InitMessageMode()
	{
		if (IsPipeConnected())
		{
			DWORD Mode = PIPE_READMODE_MESSAGE;
			return SetNamedPipeHandleState(hPipe, &Mode, NULL, NULL) == TRUE;
		}
		return false;
	}

	//------------------------------------------------------------------
	bool WriteMessage(char* Message)
	{
		if (IsPipeConnected())
			return WriteFile(hPipe, Message, strlen(Message), NULL, NULL) == TRUE;

		return false;
	}

	bool IsPipeConnected()
	{
		return hPipe != INVALID_HANDLE_VALUE;
	}

	bool ReadMessage(char* Message, size_t n)
	{
		if (IsPipeConnected())
		{

			if (ReadFile(hPipe, Message, n, NULL, &Overl) == TRUE)		
				return true;
			else
				printf("ReadMessage() failed with error %d\n", GetLastError());

		}
		return false;
	}

};

#endif