#include <iostream>
#include "test-stream_string.h"
using std::cout;
using namespace str;
int main()
{
	cout <<"================" << endl;
	try{
		//stream_string<basic_simple_buffer<char,200>>::iterator x;
		cerr << "до создания str" <<endl; 
		stream_string<basic_simple_buffer<char,200>> str;
	}
	catch(const char * mes){
		cerr << "ошибка: " << mes << endl;
		return -1;
	}
	catch(...){
		cerr << "неизвестная ошибка" << endl;
	}
}
