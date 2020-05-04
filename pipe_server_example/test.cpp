
#include"Server.h"
using namespace std;
int main()
{


	char PIPE_NAME[] = "\\\\.\\pipe\\pipe_example";

	SetConsoleOutputCP(1251);



	server server(PIPE_NAME);

	server.run_kernel();

	return 0;
}
