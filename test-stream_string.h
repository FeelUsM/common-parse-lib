//(c) FeelUs
#ifndef MYSTREAM_H
#define MYSTREAM_H

//#include <deque>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <memory>
#include <iterator>
#include <list>
using std::list;
using std::pair;

#include <iostream>
using std::cerr;
using std::endl;
using std::hex;

template<typename X> inline
void my_assert(bool b, X x){
	if(!b)	throw x;
}

#define DEBUG_MES(mes)	(cerr << mes << endl)

namespace str {

const char * test_string="test string";

class stream_string;
class _stream_string_iterator
{
		//private TYPEDEFS
	typedef const char		ch_t;
	typedef _stream_string_iterator 	my_t;
	typedef typename list<float>::iterator 	super_iterator;

		//DATA
	ch_t * pointer;//==0 <=> atend
	ch_t * endbuf;//==0 <=> не связан ни с каким буфером
	
public:
		//CONSTRUCTION, DESTRUCTION
	explicit 
	_stream_string_iterator(super_iterator sit)	
		: pointer(test_string)
		, endbuf(test_string+10){
		cerr<<"конструируем str_iterator"
			<<"[" <<hex<<this <<"]"
			<<"("<<hex<<(int)pointer<<","<<hex<<(int)endbuf<<")"
			<<"от super_iterator'а по буферу[" <<hex<<&*sit <<"]" 
			<<endl;
	}

	_stream_string_iterator()				: pointer(0), endbuf(0)	{
		cerr<<"конструируем по умолчанию str_iterator[" <<hex<<this <<"]"
			<<"("<<hex<<(int)pointer<<","<<hex<<(int)endbuf<<")"
			<<endl;
	}
	~_stream_string_iterator()	{
		cerr<< "разрушаем str_iterator "
			<<"[" <<hex<<this <<"]"
			<<"(" <<hex<<(int)pointer <<"," <<hex<<(int)endbuf <<")"
			<< endl;
		if(pointer)	{
			cerr << "и он валидный" << endl;
			pointer=0;
			endbuf=0;
		}
	}
		
		//COPYING
	_stream_string_iterator(const my_t & r)	: pointer(r.pointer), endbuf(r.endbuf)	{
		cerr<<"конструируем итератор"
			<<"[" <<hex<<this <<"]"
			<<" от итератора"
			<<"[" <<hex<<&r <<"]"
			<<endl;
	}
	my_t & operator=(const my_t & r){
		cerr<<"присваиваем в итератор"
			<<"[" <<hex<<this <<"]"
			<<" от итератора"
			<<"[" <<hex<<&r <<"]"
			<<endl;
		//можно оптимизировать
		this->~my_t();
		pointer=r.pointer;
		endbuf=r.endbuf;
		return *this;
	}

};

// ----****----		
// ----****---- CONTEINER stream_string ----****----
// ----****----
class stream_string
{
public:
	typedef _stream_string_iterator			iterator;
private:
	typedef stream_string	my_t;
	list<float> _bufs;
	iterator _iterator;//internal iterator
public:	
	stream_string()
	{
		cerr << "начали конструировать stream_string" << endl;
		cerr<< "internal _iterator"
			<<"["<<hex<<&_iterator<<"]"
			<<endl;

		_bufs.push_back(17);
		cerr<<"в конструкторе stream_string создали первый буфер" 
			<<"["<<hex<<&*_bufs.begin()<<"]"
			<<endl;
		_iterator = iterator(_bufs.begin());//он сам сконструируется от итераора на буфер
		cerr << "stream_string сконструирован" << endl;
	}
	
	~stream_string()	{
		_iterator.~iterator();
		cerr<< "internal _iterator"
			<<"["<<hex<<&_iterator<<"]"
			<<" разрушен"
			<<endl;
		my_assert(_bufs.empty(),"при деструктировании остались не удаленные буфера");
		//если потом начнут разрушаться итераторы - это пиздец
		cerr<<"закончили разрушать поток"
			<<endl;
	}

};//CLASS basic_stream_string

}//namespace str
#endif //MYSTREAM_H
