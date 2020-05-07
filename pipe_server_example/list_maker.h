#pragma once
#include <fstream>
#include <list>
#include <iostream>

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

bool Get_list(char* path, list <l_p> &list) {

	std::ifstream infile;

	infile.open(path, std::fstream::in);
	if (!infile.is_open())
		return false;

	unsigned max_password_length;
	unsigned max_login_length;
		

	infile >> max_login_length >> max_password_length;


	while (1)
	{
		string a, b;
		if (!(infile >> a >> b)) {break;} 
			
		if (a.size() > max_login_length || b.size() > max_password_length)
			return false;

		list.push_back(l_p(a,b));
	}

	infile.close();
	return true;
}



