#ifndef PER_PIPE_STRUCT_INCLUDE
#define PER_PIPE_STRUCT_INCLUDE

#include "list.h"
#include "iostream"

#define	NELEM	3

using namespace std;

template <class T> 
class PerPipeStruct
{

private:

	T ControlVals[NELEM];
	unsigned ControlInd;
	TList<T> List;

public:

	PerPipeStruct()
	{
		ClearData();
	}

	void ReadVal(T Val)
	{
		if (ControlInd < NELEM)
			ControlVals[ControlInd++] = Val;
		else
			List.AddAfterTail(Val);
		std::cout << "value is -> " << Val <<endl;
	}

	void ClearData()
	{
		for (ControlInd = 0; ControlInd < NELEM; ControlInd++)
			ControlVals[ControlInd] = 0;
		ControlInd = 0;
		List.DelAllElem();
	}

};
#endif