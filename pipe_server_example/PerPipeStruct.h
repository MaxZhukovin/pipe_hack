
#ifndef PER_PIPE_STRUCT_INCLUDE
#define PER_PIPE_STRUCT_INCLUDE


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
	PerPipeStruct(list<l_p> &&input_list) {
		list_lp = input_list;
	}

	unsigned check_value(char* input_l_p)
	{

		unsigned delay;
		if (!password_is_ok(input_l_p, delay))
			return delay;
		

		std::cout << "password is ok -> " << input_l_p <<endl;
		return 0;
	}


private:

	bool password_is_ok(char* input_l_p, unsigned &delay) {

		string input_login, input_password;
		get_lp(input_l_p, input_login, input_password);

		for (auto &i : list_lp)
		{
			if (i.Login == input_login) {
			
				if (i.Password == input_password)
					return true;
				else {
					//wrong password
					delay = compute_delay(i.delay);
					i.delay = delay;
					return false;
				}
			}
	
		}

		//wrong login
		delay = 1000;
		return false;
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