#define _CRT_SECURE_NO_WARNINGS


#include "list_maker.h"
#include "Server.h"
using namespace std;

bool should_i_continue() {
	
		cout << "Все клиенты отключены! Продолжить работу (Y или y - да / любая другая клавиша - нет)? ";
		
		char answer;
		cin >> answer;

		if (answer != 'Y' && answer != 'y')
			return 0;

		cout << "Ожидание подключения клиентов..." << endl;
	

	return 1;
}

void pipe_error() {
	cout << "Ошибка при работе с каналом! (код ошибки: " << GetLastError() << ")!" << endl;
}


int main()
{

	char PIPE_NAME[] = "\\\\.\\pipe\\pipe_example";
	char file_name[] = "C:\\Users\\HP\\Desktop\\l_p.txt";

	SetConsoleOutputCP(1251);

	list<l_p> list;
	if (!Get_list(file_name, list)) {
		cout << "Ошибка при работе с файлом " << endl;
		return 0;
	}


	server server(list, true);

	if (!server.init(PIPE_NAME))
		cout << "Ошибка создания экземпляров именованного канала " << PIPE_NAME << endl;


	cout << "Ожидание подключения клиентов..." << endl;
	server.run_kernel(should_i_continue, pipe_error);

	return 0;
}
