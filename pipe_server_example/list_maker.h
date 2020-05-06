#pragma once
#include <fstream>
#include <list>


using namespace std;

//логин + пароль
struct l_p {

public:
	string Login;
	string Password;
	unsigned delay;

public:
	l_p(string login, string password) {
		Login = login;
		Password = password;
		delay = 0;
	}

};

list <l_p> Get_list(char* path) {

	std::ifstream infile;
	infile.open(path, std::fstream::in | std::fstream::out | std::fstream::app);

	unsigned max_password_length;
	unsigned max_login_length;
		
	list<l_p> list;

	infile >> max_password_length >> max_login_length;


	while (1)
	{
		string a, b;
		if (!(infile >> a >> b)) { break; } // error
				
		list.push_back(l_p(a,b));
	}


	return list;
}



