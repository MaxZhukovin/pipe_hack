
#include"Server.h"
using namespace std;

bool should_i_continue() {
	
		cout << "��� ������� ���������! ���������� ������ (Y ��� y - �� / ����� ������ ������� - ���)? ";
		
		char answer;
		cin >> answer;

		if (answer != 'Y' && answer != 'y')
			return 0;

		cout << "�������� ����������� ��������..." << endl;
	

	return 1;
}

void pipe_error() {
	cout << "������ ��� ������ � �������! (��� ������: " << GetLastError() << ")!" << endl;
}


int main()
{


	char PIPE_NAME[] = "\\\\.\\pipe\\pipe_example";

	SetConsoleOutputCP(1251);



	server server;

	if (!server.init(PIPE_NAME))
		cout << "������ �������� ����������� ������������ ������ " << PIPE_NAME << endl;


	cout << "�������� ����������� ��������..." << endl;
	server.run_kernel(should_i_continue, pipe_error);

	return 0;
}
