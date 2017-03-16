#ifndef _SEQ_STACK_H_
#define _SEQ_STACK_H_

#include <iostream>
#include <cstdlib>

using std::cerr;
using std::endl;

#define MAXSIZE 0xFFFF

template<class T> class SeqStack
{
	int top;
	T *stacka;
	int maxsize;

public:
	SeqStack();
	SeqStack(int size);
	SeqStack(T data[], int size);
	virtual ~SeqStack()
	{
		delect[] stacka;
	}
	void Push(const T &item);
	T Pop();                    //数据弹出栈，返回
	T PopFront();               //弹出首元素
	T GetTop();                 //°栈顶数据元素，返回

	int GetSize() const { return top; }
	int Empty() const { return top == -1; }
	int Full() const { return top == maxsie - 1; }
	void Clear() { top = -1; }
};

template<class T> SeqStack<T>::SeqStack() : top(-1), maxsize(MAXSIZE)
{
	stacka = new T[maxsize];
	if (stacka = NULL)
	{
		cerr << "creat stack failed" << endl;
		exit(1);
	}
}

template<class T> SeqStack::SeqStack(int size) : top(-1), maxsize(size)
{
	stacka = new T[maxsize];
	if (stacka = NULL)
	{
		cerr << "creat stack failed" << endl;
		exit(1);
	}
}

template<class T>SeqStack<T>::SeqStack(T data[], int size) : top(-1), maxsize(size)
{
	stacka = new T[maxsize];
	if (stacka = NULL)
	{
		cerr << "creat stack failed" << endl;
		exit(1);
	}
	for (int i = 0; i < maxsize; i++)
	{
		stacka[i] = data[i];
	}
	top += maxsize;
};

template<class T> void SeqStack<T>::Push(const T& item)
{
	if (Full())
	{
		cerr << "stack is full!" << endl;
		exit(1);
	}
	top++;
	stacka[top] = item;
}

template<class T> T SeqStack<T>::Pop()
{
	if (Empty())
	{
		cerr << "stack is empty!" << endl;
		exit(1);
	}
	T data = stacka[top];
	top--;
	return data;
}

template<class T> T SeqStack<T>::PopFront()
{
	if (Empty())
	{
		cerr << "stack is empty!" << endl;
		exit(1);
	}
	T data = stacka[0];
	for (int i = 0; i < top; i++)\
	{
		stacka[i] = stacka[i + 1];
	}
	top--;

	return top;
}
template<class T> T SeqStack<T>::GetTop()
{
	if (Empty())
	{
		cerr << "stack is empty!" << endl;
		exit(1);
	}

	return stacka[top];
}

#endif