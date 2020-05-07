
#ifndef PER_PIPE_STRUCT_INCLUDE
#define PER_PIPE_STRUCT_INCLUDE

#define WRONG_PASSWORD 101
#define WRONG_LOGIN 102
#define OK_PASSWORD 103

#include <list>
#include "iostream"
#include "list_maker.h"
#define	NELEM	3

using namespace std;


class PerPipeStruct
{

private:

	list<l_p>	list_lp;	//list for login and password 

public:
	PerPipeStruct(list<l_p> &input_list) {
		list_lp = input_list;
	}

	//return delay + mess_for_sending
	unsigned check_account(char* input_l_p, char* mess_for_sending)
	{

		unsigned delay;
		switch (password_is_ok(input_l_p, delay))
		{
		case WRONG_PASSWORD:
			//std::cout << "wrong password -> " << input_l_p << endl;
			*mess_for_sending = '0';
			break;

		case WRONG_LOGIN:
			//std::cout << "wrong login -> " << input_l_p << endl;
			*mess_for_sending = '2';
			break;

		case OK_PASSWORD:
			//std::cout << "password is ok -> " << input_l_p << endl;
			*mess_for_sending = '1';	
			break;
		}

		

		
		return delay;
	}


private:

	int password_is_ok(char* input_l_p, unsigned &delay) {

		string input_login, input_password;
		get_lp(input_l_p, input_login, input_password);

		for (auto &i : list_lp)
		{
			if (i.Login == input_login) {
			
				if (i.Password == input_password) {
					delay = i.delay;
					i.delay = 0;
					return OK_PASSWORD;
				}
				else {
					//wrong password
					delay = compute_delay(i.delay);
					i.delay = delay;
					return WRONG_PASSWORD;
				}
			}
	
		}

		//wrong login
		delay = 1000;
		return WRONG_LOGIN;
	}


	//parsing
	bool get_lp(char* str, string &login, string &password) {


		std::string s = str;

		size_t pos = 0;
		pos = s.find(":");
		login = s.substr(0, pos);
		password = s.substr(pos+1, s.length());


		return true;
	}

	unsigned compute_delay(unsigned delay) {

		return delay+1000;
	}


};
#endif