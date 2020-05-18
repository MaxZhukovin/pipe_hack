
#ifndef PER_PIPE_STRUCT_INCLUDE
#define PER_PIPE_STRUCT_INCLUDE

#define WRONG_PASSWORD 101
#define WRONG_LOGIN 102
#define OK_PASSWORD 103

#include <list>
#include "iostream"
#include "list_maker.h"
#include <cmath>  
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
	bool check_account(String &input, l_p* &account)
	{

		switch (password_is_ok(input, account))
		{
		case WRONG_PASSWORD:
			//std::cout << "wrong password -> " << input_l_p << endl;
			return 0;

		case WRONG_LOGIN:
			//std::cout << "wrong login -> " << input_l_p << endl;
			return 0;

		case OK_PASSWORD:
			//std::cout << "password is ok -> " << input_l_p << endl;
			return 1;

		}

		

		
		return 0;
	}


private:

	int password_is_ok(String &input_l_p, l_p* &account) {

		String input_login, input_password;
		get_lp(input_l_p, input_login, input_password);

		for (auto &i : list_lp)
		{
			if (i.Login == input_login) {
			
				if (i.Password == input_password) {
					i.delay = 0;

					account = &(*i);
					return OK_PASSWORD;
				}
				else {

					account = &(*i);

					i.delay = compute_delay(i.delay);

					
					return WRONG_PASSWORD;
				}
			}
	
		}
		return WRONG_LOGIN;
	}

	//parsing
	bool get_lp(String &str, String &login, String &password) {


		size_t pos = 0;
		pos = str.find(":");
		login = str.substr(0, pos);
		password = str.substr(pos+1, str.length());


		return true;
	}

	unsigned compute_delay(unsigned delay) {

				   //10	  20	30     35	 40
		int cur[] = {100, 600,	1600,  2500, 3000	};
		int add[] = {10,  50,   100,   180,	 100	};

		int size = sizeof(cur) / sizeof(*cur);
		for (int i = 0; i < size; i++){
		
			if (delay < cur[i])
				return delay + add[i];
		}

		return 3000;
	}

};
#endif