//#include "stdafx.h"
#pragma once

#include <iostream>


template <class INF, class LISTTYPE>
class Iterator;

// Шаблонный класс для работы с однонаправленным списком
template <typename INF>
class TList
{
	friend	class Iterator <INF, TList<INF> >;

	// тип данных, который описывает элемент списка
	// Информационная часть (полезная нагрузка) + указатель на следующий элемент (когда он равен NULL, то это последний элемент)

	template <typename ElemType> struct TElem
	{
		ElemType Inf;
		TElem* Next;

	};

public:	typedef TElem<INF> ListElem;

protected:

	// Описываем указатели на первый и последний элементы, которые недоступны пользователю
	TElem<INF>* Head, * Tail;

	//-----------------------------------------------------------------------

	//-----------------------------------------------------------------------

public:

	typedef Iterator <INF, TList<INF> > IteratorTList;

	TList()
	{
		Head = Tail = NULL;
	}

	~TList()
	{
		DelAllElem();
	}

	//-----------------------------------------------------------------------

	// Вставка нового элемента после последнего. Элемент задается своей информационной частью

	void AddAfterTail(INF Inf)
	{
		ListElem* Cur = new TElem<INF>;

		Cur->Inf = Inf;
		Cur->Next = NULL;
		if (Head != NULL)
			Tail->Next = Cur;
		else
			Head = Cur;
		Tail = Cur;
	}

	// Вставка нового элемента после последнего. Элемент задается своей информационной частью
	// Для корректной работы в многопоточной среде применяются interlocked-функции

	void AddAfterTailInterlocked(INF Inf)
	{
		ListElem* Cur = new TElem<INF>;

		Cur->Inf = Inf;
		Cur->Next = NULL;
		if (Head != NULL)
			InterlockedExchange((LONG*)(&(Tail->Next)), (LONG)Cur);
		else
			InterlockedExchange((LONG*)(&Head), (LONG)Cur);

		InterlockedExchange((LONG*)(&Tail), (LONG)Cur);

	}



	//-----------------------------------------------------------------------

	// Поиск элемента. Элемент задается своей информационной частью

	bool SearchElement(INF Inf, IteratorTList& It)
	{
		for (It = *this; !It && !(It == Inf); ++It);
		return It.GetCurInf(Inf);

	}

	//-----------------------------------------------------------------------

	// Проверка существования элементов в списке

	bool IsNotEmpty()
	{
		return (Head != NULL);
	}

	//-----------------------------------------------------------------------

	// Удаление всех элементов из списка

	void DelAllElem()
	{
		ListElem* Cur = Head;

		while (Head != NULL)
		{
			Cur = Head;
			Head = Head->Next;
			delete Cur;
		}

		Tail = NULL;
	}

	friend std::ostream& operator << (std::ostream& os, TList& List);

};


//-----------------------------------------------------------------------


template <class INF>
std::ostream& operator << (std::ostream& os, TList<INF>& List)
{
	INF* Val = NULL;

	for (TList::IteratorTList It = List; !It; ++It)
	{
		It.GetCurInf(Val);
		os << *Val << std::endl;
	}

	return os;
}


//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

template <class INF, class LISTTYPE>
class Iterator
{
	
	TList<INF>** Head, ** Tail, * CurPos;


public:

	Iterator()
	{
		Head = Tail = NULL; //конструктор
		CurPos = NULL;
	}
	Iterator(Iterator& It)
	{
		*this = It;
	}

	Iterator(LISTTYPE& List)
	{
		*this = List; //конструктор, но...
	}

	//---------------------------------------------------

	Iterator& operator = (Iterator& It)
	{
		Head = It.Head;
		Tail = It.Tail;
		CurPos = It.CurPos;
		return *this;
	}

	Iterator& operator = (LISTTYPE& List)
	{
		Head = &(List.Head);
		Tail = &(List.Tail);
		CurPos = List.Head;
		return *this;
	}

	Iterator& operator = (INF& Val)
	{
		if (CurPos)
			CurPos->Inf = Val;

		return *this;
	}

	//---------------------------------------------------

	Iterator& operator ++()
	{
		if (CurPos)
			CurPos = CurPos->Next;

		return *this;
	}

	//---------------------------------------------------

	bool operator !()
	{
		return CurPos != NULL; //если NULL, то фалсе
	}

	bool operator == (INF Val)
	{
		return CurPos ? CurPos->Inf == Val : false;
	}

	//---------------------------------------------------

	void SetStart()
	{
		CurPos = *Head;
	}

	bool IsNotEmpty()
	{
		return (*Head != NULL);
	}

	//---------------------------------------------------

	bool GetCurInf(INF& Inf)
	{
		if (CurPos)
		{
			Inf = CurPos->Inf;
			return true;
		}
		return false;
	}

	bool GetCurInf(INF*& Inf)
	{
		if (CurPos)
		{
			Inf = &(CurPos->Inf);
			return true;
		}
		return false;
	}

};


