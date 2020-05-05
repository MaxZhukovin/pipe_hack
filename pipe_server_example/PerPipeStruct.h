#ifndef PER_PIPE_STRUCT_INCLUDE
#define PER_PIPE_STRUCT_INCLUDE

#include <list>
#include "iostream"

#define	NELEM	3

using namespace std;


class PerPipeStruct
{

private:

	unsigned delay; //for current login
	//list<l_p>		//list for login and password 

public:


	bool check_value(char* Val)
	{

		//parsing input string and checking password

		//if password is not ok
		if (*Val == '1') {

			delay = 3000;
			return false;
		}
		
		//password is ok 
		std::cout << "value is -> " << Val <<endl;
		return true;
	}


	unsigned get_delay() {
		return delay;
	}



};
#endif