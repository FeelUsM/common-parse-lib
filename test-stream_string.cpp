#include <iostream>
#include "test-stream_string.h"
using std::cout;
using namespace str;


void foo()
{
	
		//stream_string<basic_simple_buffer<char,200>>::iterator x;
		cerr << "до создания str" <<endl; 
		stream_string str;
}

class X{
	public:
	void foo()
	{
		my_assert(false,"строка из класса");
	}
	~X()
	{
		foo();
	}
};
void test_try(){
	//throw "строка";
	//my_assert(false,"строка");
	X x;
}

int main()
{
	cout <<"================" << endl;
	try{
		//foo();
		test_try();
	}
	catch(char * mes){
		cerr << "ошибка: " << mes << endl;
		return -1;
	}
	catch(...){
		cerr << "неизвестная ошибка" << endl;
	}
}
